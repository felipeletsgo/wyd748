package game

import (
	"bytes"
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

func criticalEntryFixture(t *testing.T) (*World, *Player, *charStateMemoryStore, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, _ := newEnterWorldPlayer(t, 742, 1000)
	w.clock = newFakeClock(time.Date(2026, time.October, 4, 12, 0, 0, 0, time.UTC))
	p.Account.Name, p.Account.Chars[0].Name, p.Account.Chars[0].UID = "EntryAccount", "EntryCharacter", "entry-character"
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("entry fixture could not claim account")
	}
	fallback := &charStateMemoryStore{}
	w.store = fallback
	w.items = map[uint16]model.ItemDef{4300: premiumTestDef(4300, fairySlot)}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	return w, p, fallback, backend, queue, release
}

func assertCriticalEntryPending(t *testing.T, w *World, p *Player, before *model.Account, fallback *charStateMemoryStore) {
	t.Helper()
	if p.Char != nil || p.CharSlot != -1 || p.ID != 0 || p.InWorld || p.Session.QueuedPacketsForTest() != 0 ||
		!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 || fallback.syncSaves != 0 ||
		!w.criticalPlayerReserved(p) {
		t.Fatal("pending entry selected/published/mutated a live character, saved inline or released ownership")
	}
}

func assertCriticalEntryPackets(t *testing.T, p *Player) {
	t.Helper()
	ch := p.Char
	projected := *ch
	projected.Equip = clientEquipProjection(ch)
	want := [][]byte{
		wire.EnterWorld(p.ID, uint16(p.CharSlot), projected),
		wire.CreateMobWithGuildRank(p.ID, ch.Name, ch.X, ch.Y, bodyMesh(ch), bodyAncient(ch), wireScoreState(ch), ch.Affects[:], 2, ch.GuildID, ch.GuildRank, ch.CP),
		wire.WarInfo(), playerScorePacket(p), playerAffectsPacket(p), wire.UpdateCarry(p.ID, ch.Inv[:], ch.Gold),
		wire.UpdateEtc(p.ID, *ch), wire.SelfEquip(p.ID, projected.Equip[:]), wire.HpMp(p.ID, wireScoreState(ch)),
		wire.ActionStop(p.ID, ch.X, ch.Y), wire.SetShortSkill(p.ID, ch.ShortSkill),
	}
	for sequence, expected := range want {
		got, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(got) {
			t.Fatalf("entry frame %d missing", sequence)
		}
		header, expectedHeader := wire.ParseHeader(got), wire.ParseHeader(expected)
		if header.Type != expectedHeader.Type || header.ID != expectedHeader.ID || int(header.Size) != len(expected) ||
			!bytes.Equal(got[12:], expected[12:]) {
			t.Fatalf("entry frame %d differs in opcode, recipient, size or payload", sequence)
		}
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("unexpected extra entry publication")
	}
}

func TestCriticalEntryDetachedPreparationAndFullPublication(t *testing.T) {
	for _, slot := range []int{0, 1} {
		t.Run([]string{"first-slot", "second-slot"}[slot], func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalEntryFixture(t)
			p.Account.Chars = append(p.Account.Chars, cloneCharacterState(&p.Account.Chars[0]))
			p.Account.Chars[1].Name, p.Account.Chars[1].UID = "SecondCharacter", "entry-second"
			selected := &p.Account.Chars[slot]
			selected.Equip[9] = model.Item{Index: 1234, UID: "unsupported-item"}
			selected.Equip[fairySlot] = model.Item{Index: 4300, UID: "fresh-fairy"}
			p.Account.Chars[1-slot].Inv[5] = model.Item{Index: 4300, UID: "expired-inactive", ActivatedUnix: w.now().Add(-time.Hour).Unix(), ExpiresUnix: w.now().Add(-time.Minute).Unix()}
			p.Account.Cargo[4] = p.Account.Chars[1-slot].Inv[5]
			p.Account.Cargo[4].UID = "expired-cargo"
			fallback.state = &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 23},
				Affects: []model.PersistedAffect{{Type: 2, Value: 5, ExpiresUnix: w.now().Add(time.Hour).Unix()}}}
			before := accountStateSnapshot(p.Account)
			account, score := p.Account, selected.Score
			w.onEnterWorld(p.Session, enterWorldPacket(slot))
			proposal := awaitCriticalWrite(t, backend.entered)
			if len(proposal.accounts) != 1 || proposal.accounts[0] == account || proposal.character != nil || proposal.instances != nil {
				t.Fatal("entry did not submit one detached account-only preparation")
			}
			expected := accountPersistenceSnapshot(before)
			expected.Chars[slot].Equip[9], expected.Chars[slot].Inv[0] = model.Item{}, before.Chars[slot].Equip[9]
			expected.Chars[1-slot].Inv[5], expected.Cargo[4] = model.Item{}, model.Item{}
			fairy := proposal.accounts[0].Chars[slot].Equip[fairySlot]
			if fairy.ActivatedUnix <= 0 || fairy.ExpiresUnix-fairy.ActivatedUnix != int64(w.timedItemDuration(fairy, fairySlot)/time.Second) {
				t.Fatal("entry proposal did not bind the initial timed deadline")
			}
			expected.Chars[slot].Equip[fairySlot] = fairy
			expected.Chars[slot].Affects[0] = model.Affect{Type: 2, ClientType: 2, Value: 5,
				ExpiresAt: time.Unix(fallback.state.Affects[0].ExpiresUnix, 0), NextTick: w.now().Add(8 * time.Second)}
			if !reflect.DeepEqual(proposal.accounts[0], expected) {
				t.Fatal("entry proposal differs outside migration, expiry or activation")
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.onEnterWorld(p.Session, enterWorldPacket(slot))
				w.onEnterWorld(p.Session, enterWorldPacket(1-slot))
				assertCriticalEntryPending(t, w, p, before, fallback)
				if fallback.loads != 1 || len(queue.pending) != 1 || len(backend.entered) != 0 ||
					!w.criticalCharacterReserved(before.Chars[0].UID) || !w.criticalCharacterReserved(before.Chars[1].UID) {
					t.Fatalf("%s entry repeated preparation or lost sibling ownership", phase)
				}
			}
			w.processCriticalCompletions(1)
			if !p.InWorld || p.Account != account || p.Char != &account.Chars[slot] || p.Char.Score != score ||
				p.CharSlot != slot || w.playersByID[p.ID] != p || w.playersByCharacterUID[p.Char.UID] != p ||
				len(queue.pending) != 0 || w.criticalPlayerReserved(p) || fallback.loads != 1 || fallback.saves != 0 ||
				p.Char.Equip[fairySlot] != fairy || p.Char.Inv[0] != before.Chars[slot].Equip[9] ||
				p.Account.Chars[1-slot].Inv[5].Index != 0 || p.Account.Cargo[4].Index != 0 || p.SpecialCoins["fame"] != 23 {
				t.Fatal("entry failed to install exactly the committed items/sidecar and selected identity")
			}
			fallback.state.SpecialCoins["fame"] = 999
			if p.SpecialCoins["fame"] != 23 || activePlayerAffectAt(p.Char, 2, w.now()) == nil {
				t.Fatal("entry lost the loaded affect or aliased sidecar coins")
			}
			assertCriticalEntryPackets(t, p)
		})
	}
}

func TestCriticalEntryRejectionsLeaveSelectionUntouched(t *testing.T) {
	for _, scenario := range []string{"missing-item-uid", "invalid-deadline", "sidecar-load", "queue-full", "stopping", "unclaimed", "closed", "invalid-slot", "poisoned"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, fallback, backend, queue, _ := criticalEntryFixture(t)
			p.Account.Chars[0].Equip[fairySlot] = model.Item{Index: 4300, UID: "fresh-fairy"}
			requestedSlot := 0
			wantPanel := true
			switch scenario {
			case "missing-item-uid":
				p.Account.Chars[0].Equip[fairySlot].UID = ""
			case "invalid-deadline":
				p.Account.Chars[0].Equip[fairySlot].ExpiresUnix = w.now().Add(time.Hour).Unix()
			case "sidecar-load":
				fallback.loadErr = errors.New("sidecar unavailable")
			case "queue-full":
				for _, name := range []string{"OtherOne", "OtherTwo"} {
					if _, err := queue.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}); err != nil {
						t.Fatal(err)
					}
				}
				awaitCriticalWrite(t, backend.entered)
			case "stopping":
				queue.closeAdmission()
			case "unclaimed":
				delete(w.accountSessions, accountSessionKey(p.Account.Name))
			case "closed":
				p.Session.Close()
				wantPanel = false
			case "invalid-slot":
				requestedSlot, wantPanel = 4, false
			case "poisoned":
				p.PersistencePoisoned, wantPanel = true, false
			}
			before := accountStateSnapshot(p.Account)
			pending := len(queue.pending)
			w.onEnterWorld(p.Session, enterWorldPacket(requestedSlot))
			if p.InWorld || p.Char != nil || p.CharSlot != -1 || p.ID != 0 || p.criticalLifecycle != nil ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 || fallback.syncSaves != 0 ||
				len(queue.pending) != pending || w.criticalAccountReserved(p.Account.Name) {
				t.Fatal("rejected entry mutated, persisted or installed a lifecycle/reservation")
			}
			if wantPanel {
				assertCargoErrorPanel(t, p)
			} else if p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("invalid/closed/poisoned entry published packets")
			}
		})
	}
}

func TestCriticalEntryUnknownCommitOrChangedCharacterDoesNotPublish(t *testing.T) {
	for _, scenario := range []string{"unknown-error", "panic", "selected-uid", "selected-name", "sibling-uid", "character-list", "session-generation"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalEntryFixture(t)
			p.Account.Chars = append(p.Account.Chars, model.Char{Name: "Sibling", UID: "entry-sibling"})
			p.Account.Chars[0].Equip[fairySlot] = model.Item{Index: 4300, UID: "fresh-fairy"}
			w.onEnterWorld(p.Session, enterWorldPacket(0))
			awaitCriticalWrite(t, backend.entered)
			switch scenario {
			case "unknown-error":
				backend.failure = errors.New("unknown commit")
			case "panic":
				backend.panicAfterWrite = true
			case "selected-uid":
				p.Account.Chars[0].UID = "replacement"
			case "selected-name":
				p.Account.Chars[0].Name = "Replacement"
			case "sibling-uid":
				p.Account.Chars[1].UID = "replacement-sibling"
			case "character-list":
				p.Account.Chars = p.Account.Chars[:1]
			case "session-generation":
				p.criticalLifecycle = &criticalPlayerLifecycle{}
			}
			beforeDelivery := accountStateSnapshot(p.Account)
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if !w.shuttingDown || !p.PersistencePoisoned || !p.Session.IsClosed() || w.shutdownErr == nil ||
				p.InWorld || p.Char != nil || p.ID != 0 || p.CharSlot != -1 || p.Session.QueuedPacketsForTest() != 0 ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), beforeDelivery) || len(queue.pending) != 1 ||
				!w.criticalAccountReserved(p.Account.Name) || fallback.saves != 0 {
				t.Fatal("uncertain/stale entry applied, published, released ownership or escaped containment")
			}
			done := make(chan error, 1)
			w.runShutdown(done)
			if err := <-done; err == nil || fallback.saves != 0 || fallback.syncSaves != 0 || len(backend.entered) != 0 {
				t.Fatal("uncertain entry was replayed or overwritten by terminal persistence")
			}
		})
	}
}

func TestCriticalEntryNoPreparationWriteAndStorageFallbacks(t *testing.T) {
	for _, scenario := range []string{"unchanged", "existing-deadline", "carry-full", "carry-and-cargo-full"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalEntryFixture(t)
			ch := &p.Account.Chars[0]
			if scenario == "existing-deadline" {
				ch.Equip[fairySlot] = model.Item{Index: 4300, UID: "existing-fairy", ActivatedUnix: w.now().Add(-time.Hour).Unix(), ExpiresUnix: w.now().Add(time.Hour).Unix()}
			}
			if scenario == "carry-full" || scenario == "carry-and-cargo-full" {
				ch.Equip[9] = model.Item{Index: 1234, UID: "unsupported-item"}
				for slot := 0; slot < model.PlayerCarrySlots; slot++ {
					ch.Inv[slot] = model.Item{Index: 1235}
				}
				if scenario == "carry-and-cargo-full" {
					for slot := 0; slot < model.PlayerCargoSlots; slot++ {
						p.Account.Cargo[slot] = model.Item{Index: 1235}
					}
				}
			}
			before := accountStateSnapshot(p.Account)
			w.onEnterWorld(p.Session, enterWorldPacket(0))
			if scenario == "carry-full" {
				write := awaitCriticalWrite(t, backend.entered)
				if write.accounts[0].Chars[0].Equip[9].Index != 0 || write.accounts[0].Cargo[0] != before.Chars[0].Equip[9] {
					t.Fatal("full Carry did not plan migration into visible Cargo")
				}
				assertCriticalEntryPending(t, w, p, before, fallback)
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
			} else if len(queue.pending) != 0 || len(backend.entered) != 0 || p.Char == nil ||
				!reflect.DeepEqual(snapshotTimedAccountItems(p.Account), snapshotTimedAccountItems(before)) {
				t.Fatal("unchanged/full-storage entry queued or altered item identity/deadlines")
			}
			if !p.InWorld || fallback.loads != 1 || fallback.saves != 0 || fallback.syncSaves != 0 {
				t.Fatal("prepared entry failed or fell back to an inline write")
			}
			if scenario == "carry-and-cargo-full" && clientEquipProjection(p.Char)[9].Index != 0 {
				t.Fatal("unmigrated unsupported equipment reached client projection")
			}
			assertCriticalEntryPackets(t, p)
		})
	}
}

func TestCriticalEntryHelperRejectsMalformedSelection(t *testing.T) {
	w, p, _, _, _, _ := criticalEntryFixture(t)
	for _, slot := range []int{-1, 1, 4} {
		if err := w.prepareCriticalCharacterEntry(p, slot); !errors.Is(err, errCriticalPersistenceIdentity) {
			t.Fatalf("invalid entry slot %d: %v", slot, err)
		}
	}
	if err := w.prepareCriticalCharacterEntry(nil, 0); !errors.Is(err, errCriticalPersistenceIdentity) {
		t.Fatalf("nil participant: %v", err)
	}
}

func TestCriticalEntryActualDispatchKeepsWorldResponsive(t *testing.T) {
	w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, nil, func(w *World, p *Player) {
		w.unregisterPlayerSpatial(p)
		w.unindexPlayerCharacter(p)
		delete(w.playersByID, p.ID)
		p.InWorld = false
		resetCharacterRuntime(p)
		w.items = map[uint16]model.ItemDef{4300: premiumTestDef(4300, fairySlot)}
		p.Account.Chars[0].Equip[fairySlot] = model.Item{Index: 4300, UID: "loop-entry-fairy"}
	})
	request := func() {
		select {
		case w.commands <- command{s: p.Session, pkt: enterWorldPacket(0)}:
		case <-time.After(time.Second):
			t.Fatal("World did not accept character-entry intention")
		}
	}
	request()
	write := awaitCriticalWrite(t, backend.entered)
	if write.accounts[0].Chars[0].Equip[fairySlot].ActivatedUnix == 0 {
		t.Fatal("dispatcher did not submit entry preparation")
	}
	for range 3 {
		request()
		deliverCriticalLoopTick(t, ticks)
		overview := readCriticalLoopOverview(t, w)
		progress := false
		for _, player := range overview.Players {
			if player.UID == unrelatedUID && player.HP > 500 {
				progress = true
			}
		}
		if overview.State != "running" || !progress || p.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("held entry blocked unrelated commands/ticks or published early")
		}
	}
	if len(backend.entered) != 0 {
		t.Fatal("repeated pending entry submitted another operation")
	}
	release()
	deadline := time.Now().Add(2 * time.Second)
	for p.Session.QueuedPacketsForTest() == 0 && time.Now().Before(deadline) {
		readCriticalLoopOverview(t, w)
	}
	stop()
	if !p.InWorld || p.Char == nil || p.Char.Equip[fairySlot].ActivatedUnix == 0 || len(queue.pending) != 0 ||
		w.criticalPlayerReserved(p) || w.mobTickCounter < 3 || w.shuttingDown {
		t.Fatal("actor did not publish committed entry once while keeping unrelated ticks responsive")
	}
}

// One worker executes these independent gates in order. Tests release each
// transaction explicitly without mutating worker-owned configuration mid-call.
type criticalEntrySequencedStore struct {
	*criticalTestStore
	gates []<-chan struct{}
	next  int
}

func (s *criticalEntrySequencedStore) SaveAccount(account *model.Account) error {
	if s.next >= len(s.gates) {
		return errors.New("unexpected extra character-entry write")
	}
	gate := s.gates[s.next]
	s.next++
	s.entered <- criticalWrite{accounts: []*model.Account{account}}
	<-gate
	return nil
}

func TestCriticalEntryExpiryDuringWriteRequiresAnotherCommit(t *testing.T) {
	w, p, fallback, backend, queue, releaseFirst := criticalEntryFixture(t)
	secondGate, releaseSecond := criticalGate(t)
	t.Cleanup(releaseSecond)
	queue.store = &criticalEntrySequencedStore{criticalTestStore: backend, gates: []<-chan struct{}{backend.release, secondGate}}
	ch := &p.Account.Chars[0]
	ch.Equip[9] = model.Item{Index: 1234, UID: "migrated-item"}
	ch.Equip[fairySlot] = model.Item{Index: 4300, UID: "expiring-fairy",
		ActivatedUnix: w.now().Add(-time.Hour).Unix(), ExpiresUnix: w.now().Add(time.Second).Unix()}
	fallback.state = &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 23},
		Affects: []model.PersistedAffect{{Type: 2, Value: 5, ExpiresUnix: w.now().Add(time.Second).Unix()}}}
	before := accountStateSnapshot(p.Account)
	w.onEnterWorld(p.Session, enterWorldPacket(0))
	first := awaitCriticalWrite(t, backend.entered)
	assertCriticalEntryPending(t, w, p, before, fallback)
	w.clock.(*fakeClock).Advance(2 * time.Second)
	releaseFirst()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	second := awaitCriticalWrite(t, backend.entered)
	if first.accounts[0].Chars[0].Equip[fairySlot] != before.Chars[0].Equip[fairySlot] ||
		second.accounts[0].Chars[0].Equip[fairySlot].Index != 0 || ch.Equip[fairySlot] != before.Chars[0].Equip[fairySlot] ||
		ch.Equip[9].Index != 0 || ch.Inv[0] != before.Chars[0].Equip[9] || p.InWorld || p.Char != nil ||
		p.Session.QueuedPacketsForTest() != 0 || !w.criticalPlayerReserved(p) || fallback.loads != 1 || fallback.saves != 0 {
		t.Fatal("entry published or deleted an item before the distinct expiry transaction committed")
	}
	for _, phase := range []string{"held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			releaseSecond()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.onEnterWorld(p.Session, enterWorldPacket(0))
		if p.Char != nil || p.Session.QueuedPacketsForTest() != 0 || fallback.loads != 1 || len(backend.entered) != 0 {
			t.Fatalf("%s expiry follow-up repeated preparation or published early", phase)
		}
	}
	w.processCriticalCompletions(1)
	if !p.InWorld || ch.Equip[fairySlot].Index != 0 || p.SpecialCoins["fame"] != 23 ||
		ch.Affects != [16]model.Affect{} || fallback.loads != 1 || len(queue.pending) != 0 || w.shuttingDown {
		t.Fatal("entry did not use committed removals and discard the affect that expired during preparation")
	}
	assertCriticalEntryPackets(t, p)
}

func TestCriticalEntryDisconnectAndShutdownPreserveCommittedPreparation(t *testing.T) {
	for _, scenario := range []string{"disconnect", "shutdown", "disconnect-and-shutdown"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, fallback, backend, queue, releaseFirst := criticalEntryFixture(t)
			secondGate, releaseSecond := criticalGate(t)
			t.Cleanup(releaseSecond)
			queue.store = &criticalEntrySequencedStore{criticalTestStore: backend, gates: []<-chan struct{}{backend.release, secondGate}}
			terminal := &criticalPurchaseTerminalStore{craftStore: &fallback.craftStore}
			// No sidecar is loaded in this scenario; capture only terminal writes.
			w.store = terminal
			p.Account.Chars[0].Equip[fairySlot] = model.Item{Index: 4300, UID: "entry-lifecycle-fairy"}
			before := accountStateSnapshot(p.Account)
			w.onEnterWorld(p.Session, enterWorldPacket(0))
			first := awaitCriticalWrite(t, backend.entered)
			committed := accountPersistenceSnapshot(before)
			committed.Chars[0].Equip[fairySlot] = first.accounts[0].Chars[0].Equip[fairySlot]
			var done chan error
			if scenario != "shutdown" {
				w.handle(command{s: p.Session})
			} else {
				done = make(chan error, 1)
				w.runShutdown(done)
			}
			if w.players[p.Session] != p || w.claimAccountSession(gameNet.NewTestSession(99, 8), p.Account.Name) ||
				fallback.saves != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Char != nil || p.InWorld {
				t.Fatal("held entry lost account ownership or persisted/published uncommitted preparation")
			}
			releaseFirst()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if p.InWorld || p.Char != nil || p.ID != 0 || p.Session.QueuedPacketsForTest() != 0 ||
				!reflect.DeepEqual(accountPersistenceSnapshot(p.Account), committed) {
				t.Fatal("closed/stopping entry published or lost its confirmed preparation")
			}
			if scenario == "shutdown" {
				if err := <-done; err != nil || !w.shutdownFinished || fallback.saves != 0 ||
					len(backend.entered) != 0 || len(queue.pending) != 0 || terminal.flushes != 1 {
					t.Fatalf("shutdown failed to drain the durable selection write without replay: %v", err)
				}
				return
			}
			final := awaitCriticalWrite(t, backend.entered)
			if final.character != nil || !reflect.DeepEqual(final.accounts[0], committed) ||
				w.players[p.Session] != p || !w.criticalPlayerReserved(p) || fallback.saves != 0 {
				t.Fatal("selection disconnect did not retain a detached final snapshot with committed deadlines")
			}
			if scenario == "disconnect-and-shutdown" {
				done = make(chan error, 1)
				w.runShutdown(done)
			}
			releaseSecond()
			awaitReadyCriticalResults(t, queue, 1)
			w.handle(command{s: p.Session})
			if w.claimAccountSession(gameNet.NewTestSession(100, 8), p.Account.Name) || len(backend.entered) != 0 {
				t.Fatal("unacknowledged final entry snapshot allowed readmission or repeated persistence")
			}
			w.processCriticalCompletions(1)
			if scenario == "disconnect-and-shutdown" {
				if err := <-done; err != nil || fallback.saves != 1 || terminal.flushes != 1 ||
					!reflect.DeepEqual(terminal.account, committed) || p.Char != nil || p.InWorld {
					t.Fatalf("terminal write lost the prepared items or selected the character: %v", err)
				}
			} else if w.players[p.Session] != nil || len(w.criticalDisconnects) != 0 || fallback.saves != 0 ||
				!w.claimAccountSession(gameNet.NewTestSession(101, 8), p.Account.Name) {
				t.Fatal("final acknowledgement did not release selection ownership exactly once")
			}
			if len(queue.pending) != 0 || len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("entry lifecycle left pending work or published to a closed transport")
			}
		})
	}
}

func TestCriticalEntryAllocatesIDAndPositionAfterPreparation(t *testing.T) {
	for _, full := range []bool{false, true} {
		t.Run(map[bool]string{false: "contended-position", true: "world-full"}[full], func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalEntryFixture(t)
			p.Account.Chars[0].Equip[fairySlot] = model.Item{Index: 4300, UID: "entry-allocation-fairy"}
			w.onEnterWorld(p.Session, enterWorldPacket(0))
			write := awaitCriticalWrite(t, backend.entered)
			_, x, y := playerHomeCitySpawn(&p.Account.Chars[0])
			limit := uint16(2)
			if full {
				limit = 1000
			}
			for id := uint16(1); id < limit; id++ {
				session := gameNet.NewTestSession(int64(1000+id), 32)
				otherChar := cloneCharacterState(&p.Account.Chars[0])
				otherChar.Name, otherChar.UID, otherChar.X, otherChar.Y = "Other", "", x, y
				occupant := &Player{Session: session, ID: id, InWorld: true, X: x, Y: y,
					Char: &otherChar, Visible: make(map[uint16]struct{})}
				w.players[session], w.playersByID[id] = occupant, occupant
				w.updatePlayerSpatial(occupant)
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if len(queue.pending) != 0 || p.Account.Chars[0].Equip[fairySlot] != write.accounts[0].Chars[0].Equip[fairySlot] || fallback.saves != 0 {
				t.Fatal("publication allocation lost confirmed equipment or wrote inline")
			}
			if full {
				if p.InWorld || p.Char != nil || p.ID != 0 || p.CharSlot != -1 || w.playersByID[999].Session == p.Session {
					t.Fatal("delayed entry overwrote an occupied ID when the world filled during preparation")
				}
				assertCargoErrorPanel(t, p)
			} else if !p.InWorld || p.ID != 2 || (p.X == x && p.Y == y) || w.playersByID[1].Session == p.Session {
				t.Fatal("delayed entry used a stale ID or occupied spawn position")
			}
		})
	}
}
