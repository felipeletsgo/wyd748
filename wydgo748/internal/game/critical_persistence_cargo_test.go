package game

import (
	"errors"
	"fmt"
	"reflect"
	"sync"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

func criticalCargoFixture(t *testing.T) (*World, *Player, *cargoGoldContractStore, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, fallback := cargoGoldContractWorld(t)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("fixture account admission failed")
	}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	return w, p, fallback, backend, queue, release
}

func cargoDirection(deposit bool) string {
	if deposit {
		return "deposit"
	}
	return "withdraw"
}

func assertCriticalCargoUnchanged(t *testing.T, p *Player, before *model.Account, fallback *cargoGoldContractStore) {
	t.Helper()
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 {
		t.Fatal("pending/rejected transfer changed the live account or saved inline")
	}
}

func assertCargoErrorPanel(t *testing.T, p *Player) {
	t.Helper()
	packet, ok := p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpMessagePanel ||
		p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("rejection must send only an error panel, never transfer confirmation")
	}
}

func TestCriticalCargoGoldProposesBeforeCommitAndConfirmsAfterAcknowledgement(t *testing.T) {
	for _, deposit := range []bool{true, false} {
		for _, secondSlot := range []bool{false, true} {
			name := cargoDirection(deposit) + "/first-slot"
			if secondSlot {
				name = cargoDirection(deposit) + "/second-slot"
			}
			t.Run(name, func(t *testing.T) {
				w, p, fallback, backend, queue, release := criticalCargoFixture(t)
				if secondSlot {
					oldUID := p.Char.UID
					p.Account.Chars = append(p.Account.Chars, cloneCharacterState(p.Char))
					p.CharSlot, p.Char = 1, &p.Account.Chars[1]
					p.Char.UID += "-second"
					delete(w.playersByCharacterUID, oldUID)
					w.indexPlayerCharacter(p)
				}
				account, character, score := p.Account, p.Char, p.Char.Score
				before := accountStateSnapshot(account)
				gold, cargo := uint32(1200), uint32(300)
				if deposit {
					gold, cargo = 800, 700
				}
				expected := accountPersistenceSnapshot(account)
				expected.Chars[p.CharSlot].Gold, expected.CargoGold = gold, cargo
				request := cargoGoldRequest(deposit, 200)
				w.onCargoGold(p.Session, request, deposit)
				write := awaitCriticalWrite(t, backend.entered)
				if len(write.accounts) != 1 || write.character != nil || write.instances != nil ||
					write.accounts[0] == account || &write.accounts[0].Chars[p.CharSlot] == character ||
					!reflect.DeepEqual(write.accounts[0], expected) {
					t.Fatal("worker did not receive the exact detached account proposal for the selected slot")
				}
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, queue, 1)
					}
					w.onCargoGold(p.Session, request, deposit)
					w.onCargoGold(p.Session, cargoGoldRequest(!deposit, 100), !deposit)
					assertCriticalCargoUnchanged(t, p, before, fallback)
					if p.Session.QueuedPacketsForTest() != 0 || !w.criticalPlayerReserved(p) ||
						len(queue.pending) != 1 || len(backend.entered) != 0 {
						t.Fatalf("%s confirmed, duplicated or released the pending transfer", phase)
					}
				}
				w.processCriticalCompletions(1)
				expectedLive := accountStateSnapshot(before)
				expectedLive.Chars[p.CharSlot].Gold, expectedLive.CargoGold = gold, cargo
				if p.Account != account || p.Char != character || p.Char.Score != score ||
					!reflect.DeepEqual(accountStateSnapshot(account), expectedLive) ||
					w.criticalPlayerReserved(p) || len(queue.pending) != 0 || fallback.saves != 0 {
					t.Fatal("acknowledgement replaced identity/non-balance state or did not install committed balances")
				}
				assertCargoGoldPublication(t, p, deposit, 200, gold, cargo)
				// No wire transaction ID exists. Once acknowledged, the identical
				// intention is a new transfer using the newly committed balances.
				w.onCargoGold(p.Session, request, deposit)
				write = awaitCriticalWrite(t, backend.entered)
				if deposit {
					gold, cargo = 600, 900
				} else {
					gold, cargo = 1400, 100
				}
				if write.accounts[0].Chars[p.CharSlot].Gold != gold || write.accounts[0].CargoGold != cargo ||
					p.Char.Gold == gold || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("post-acknowledgement repetition was deduplicated or applied before its own commit")
				}
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				assertCargoGoldPublication(t, p, deposit, 200, gold, cargo)
			})
		}
	}
}

func TestCriticalCargoGoldAdmissionRejectionDoesNotMutateOrFallback(t *testing.T) {
	for _, deposit := range []bool{true, false} {
		for _, reason := range []string{"full", "stopping", "account-owner", "character-index", "slot", "slot-pointer",
			"closed", "poisoned", "world-stopping"} {
			t.Run(cargoDirection(deposit)+"/"+reason, func(t *testing.T) {
				w, p, fallback, backend, queue, _ := criticalCargoFixture(t)
				wantPanel := true
				switch reason {
				case "full":
					for _, name := range []string{"occupied-a", "occupied-b"} {
						if _, err := queue.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}); err != nil {
							t.Fatal(err)
						}
					}
					awaitCriticalWrite(t, backend.entered)
				case "stopping":
					queue.closeAdmission()
				case "account-owner":
					delete(w.accountSessions, accountSessionKey(p.Account.Name))
				case "character-index":
					delete(w.playersByCharacterUID, p.Char.UID)
				case "slot":
					p.CharSlot = len(p.Account.Chars)
				case "slot-pointer":
					copy := cloneCharacterState(p.Char)
					p.Char = &copy
				case "closed":
					p.Session.Close()
					wantPanel = false
				case "poisoned":
					p.PersistencePoisoned = true
					wantPanel = false
				case "world-stopping":
					w.shuttingDown = true
					wantPanel = false
				}
				before, character, pending, lastID := accountStateSnapshot(p.Account), cloneCharacterState(p.Char), len(queue.pending), queue.lastID
				w.onCargoGold(p.Session, cargoGoldRequest(deposit, 200), deposit)
				assertCriticalCargoUnchanged(t, p, before, fallback)
				if !reflect.DeepEqual(*p.Char, character) || len(queue.pending) != pending || queue.lastID != lastID ||
					p.criticalLifecycle != nil || w.criticalPlayerWriteReserved(p) {
					t.Fatal("rejected admission mutated a detached pointer, installed a token/fence or accepted a write")
				}
				if wantPanel {
					assertCargoErrorPanel(t, p)
				} else if p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("closed/poisoned/stopping participant received a response")
				}
			})
		}
	}
}

func TestCriticalCargoGoldDomainRejections(t *testing.T) {
	for _, deposit := range []bool{true, false} {
		for _, reason := range []string{"zero", "over-limit", "insufficient", "destination-limit", "dead", "out-of-world",
			"hidden-banker", "distant-banker", "truncated", "oversized"} {
			t.Run(cargoDirection(deposit)+"/"+reason, func(t *testing.T) {
				w, p, fallback, backend, queue, _ := criticalCargoFixture(t)
				request, wantPanel := cargoGoldRequest(deposit, 200), false
				switch reason {
				case "zero":
					request = cargoGoldRequest(deposit, 0)
				case "over-limit":
					request = cargoGoldRequest(deposit, maxCharacterGold+1)
				case "insufficient":
					request, wantPanel = cargoGoldRequest(deposit, 1001), true
				case "destination-limit":
					if deposit {
						p.Account.CargoGold = maxCharacterGold - 199
					} else {
						p.Char.Gold = maxCharacterGold - 199
					}
					wantPanel = true
				case "dead":
					setPlayerCurHP(p.Char, 0)
				case "out-of-world":
					p.InWorld = false
				case "hidden-banker":
					delete(p.Visible, p.CargoNPC)
				case "distant-banker":
					p.X += npcInteractionRange + 20
				case "truncated":
					request = request[:15]
				case "oversized":
					request = append(request, 0)
				}
				before := accountStateSnapshot(p.Account)
				w.onCargoGold(p.Session, request, deposit)
				assertCriticalCargoUnchanged(t, p, before, fallback)
				if len(queue.pending) != 0 || queue.lastID != 0 || len(backend.entered) != 0 || p.criticalLifecycle != nil {
					t.Fatal("invalid request reached persistence admission")
				}
				if wantPanel {
					assertCargoErrorPanel(t, p)
				} else if p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("invalid request received transfer confirmation")
				}
			})
		}
	}
}

func TestCriticalCargoGoldRetriesRejectedAdmissionOnlyAfterCapacityReturns(t *testing.T) {
	w, p, fallback, backend, queue, release := criticalCargoFixture(t)
	for _, name := range []string{"occupied-a", "occupied-b"} {
		if _, err := w.criticalWrites.submit(criticalWrite{accounts: []*model.Account{{Name: name}}},
			func(criticalPersistenceResult) error { return nil }); err != nil {
			t.Fatal(err)
		}
	}
	awaitCriticalWrite(t, backend.entered)
	before := accountStateSnapshot(p.Account)
	w.onCargoGold(p.Session, cargoGoldRequest(true, 200), true)
	assertCriticalCargoUnchanged(t, p, before, fallback)
	assertCargoErrorPanel(t, p)
	if p.criticalLifecycle != nil || w.criticalPlayerReserved(p) || len(queue.pending) != 2 {
		t.Fatal("full admission retained the rejected request as pending work")
	}
	release()
	awaitCriticalWrite(t, backend.entered)
	awaitReadyCriticalResults(t, queue, 2)
	w.processCriticalCompletions(2)
	if len(queue.pending) != 0 || w.shuttingDown || len(backend.entered) != 0 {
		t.Fatal("unrelated accepted work did not release capacity normally")
	}
	w.onCargoGold(p.Session, cargoGoldRequest(true, 200), true)
	write := awaitCriticalWrite(t, backend.entered)
	assertCriticalCargoUnchanged(t, p, before, fallback)
	if write.accounts[0].Chars[p.CharSlot].Gold != 800 || write.accounts[0].CargoGold != 700 ||
		p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("new admission did not plan exactly one transfer after the rejection")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	assertCargoGoldPublication(t, p, true, 200, 800, 700)
}

func TestCriticalCargoGoldReservationPrecedesBankerContextMutation(t *testing.T) {
	w, p, fallback, backend, queue, release := criticalCargoFixture(t)
	before := accountStateSnapshot(p.Account)
	w.onCargoGold(p.Session, cargoGoldRequest(true, 200), true)
	awaitCriticalWrite(t, backend.entered)
	p.CargoNPC = 0
	for _, phase := range []string{"held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.onCargoGold(p.Session, cargoGoldRequest(false, 200), false)
		assertCriticalCargoUnchanged(t, p, before, fallback)
		if p.CargoNPC != 0 || p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 || len(backend.entered) != 0 {
			t.Fatalf("%s reserved request mutated banker context before checking ownership", phase)
		}
	}
	w.processCriticalCompletions(1)
	assertCargoGoldPublication(t, p, true, 200, 800, 700)
}

func TestCriticalCargoGoldUnknownOutcomeDoesNotApplyConfirmReplayOrOverwrite(t *testing.T) {
	for _, deposit := range []bool{true, false} {
		for _, reason := range []string{"error", "panic", "identity"} {
			t.Run(cargoDirection(deposit)+"/"+reason, func(t *testing.T) {
				w, p, fallback, backend, queue, release := criticalCargoFixture(t)
				if reason == "error" {
					backend.failure = errors.New("commit acknowledgement lost")
				}
				backend.panicAfterWrite = reason == "panic"
				before := accountStateSnapshot(p.Account)
				w.onCargoGold(p.Session, cargoGoldRequest(deposit, 200), deposit)
				awaitCriticalWrite(t, backend.entered)
				// A physical disconnect must not supersede the unknown economic
				// outcome with a final snapshot of pre-transfer balances.
				w.handle(command{s: p.Session})
				if reason == "identity" {
					p.criticalLifecycle = &criticalPlayerLifecycle{}
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				w.onCargoGold(p.Session, cargoGoldRequest(deposit, 200), deposit)
				w.resumeCriticalDisconnects(8)
				w.processCriticalCompletions(1)
				assertCriticalCargoUnchanged(t, p, before, fallback)
				if !w.shuttingDown || !p.PersistencePoisoned || !p.Session.IsClosed() || w.shutdownErr == nil ||
					!w.criticalPlayerReserved(p) || len(queue.pending) != 1 || len(backend.entered) != 0 ||
					w.players[p.Session] != p || len(w.criticalDisconnects) != 1 || p.Session.QueuedPacketsForTest() != 0 ||
					w.claimAccountSession(gameNet.NewTestSession(3, 8), p.Account.Name) {
					t.Fatal("uncertain outcome applied, confirmed, replayed or released the aggregate")
				}
				if reason == "identity" && !errors.Is(w.shutdownErr, errCriticalPersistenceIdentity) ||
					reason != "identity" && !errors.Is(w.shutdownErr, errCriticalPersistenceIndeterminate) {
					t.Fatalf("maintenance lost the correct outcome classification: %v", w.shutdownErr)
				}
				done := make(chan error, 1)
				w.runShutdown(done)
				if err := <-done; err == nil || fallback.saves != 0 {
					t.Fatal("terminal shutdown overwrote an unknown transfer")
				}
			})
		}
	}
}

func TestCriticalCargoGoldAcceptsExactBalanceCaps(t *testing.T) {
	for _, deposit := range []bool{true, false} {
		for _, amount := range []uint32{1, maxCharacterGold} {
			t.Run(cargoDirection(deposit)+"/"+fmt.Sprint(amount), func(t *testing.T) {
				w, p, fallback, backend, queue, release := criticalCargoFixture(t)
				gold, cargo := uint32(0), maxCharacterGold
				if deposit {
					p.Char.Gold, p.Account.CargoGold = amount, maxCharacterGold-amount
				} else {
					p.Account.CargoGold, p.Char.Gold = amount, maxCharacterGold-amount
					gold, cargo = maxCharacterGold, 0
				}
				before := accountStateSnapshot(p.Account)
				w.onCargoGold(p.Session, cargoGoldRequest(deposit, amount), deposit)
				write := awaitCriticalWrite(t, backend.entered)
				assertCriticalCargoUnchanged(t, p, before, fallback)
				if write.accounts[0].Chars[p.CharSlot].Gold != gold || write.accounts[0].CargoGold != cargo {
					t.Fatal("exact-cap transfer overflowed or failed to conserve balances")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if p.Char.Gold != gold || p.Account.CargoGold != cargo {
					t.Fatal("exact-cap transfer did not install the committed balances")
				}
				assertCargoGoldPublication(t, p, deposit, amount, gold, cargo)
			})
		}
	}
}

type criticalCargoDisconnectStore struct {
	*criticalTestStore
	finalRelease <-chan struct{}
}

func (s *criticalCargoDisconnectStore) SavePlayerState(_ *model.GuildRegistry, account *model.Account,
	uid string, state *model.CharState) error {
	s.entered <- criticalWrite{accounts: []*model.Account{account}, character: &criticalCharacterState{uid: uid, state: state}}
	<-s.finalRelease
	return nil
}

func TestCriticalCargoGoldDisconnectSettlesBeforeAtomicFinalSaveAndReadmission(t *testing.T) {
	for _, deposit := range []bool{true, false} {
		t.Run(cargoDirection(deposit), func(t *testing.T) {
			gate, release := criticalGate(t)
			finalGate, releaseFinal := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			backend := &criticalCargoDisconnectStore{criticalTestStore: base, finalRelease: finalGate}
			queue := newCriticalTestQueue(t, backend, 1, 2)
			t.Cleanup(release)
			t.Cleanup(releaseFinal)
			w, p, fallback := cargoGoldContractWorld(t)
			if !w.claimAccountSession(p.Session, p.Account.Name) {
				t.Fatal("fixture account admission failed")
			}
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			before := accountStateSnapshot(p.Account)
			session, uid, id, key := p.Session, p.Char.UID, p.ID, accountSessionKey(p.Account.Name)
			reconnecting := gameNet.NewTestSession(3, 8)
			w.onCargoGold(session, cargoGoldRequest(deposit, 200), deposit)
			awaitCriticalWrite(t, base.entered)
			w.handle(command{s: session})
			w.handle(command{s: session})
			for _, phase := range []string{"transfer-held", "transfer-completed-unacknowledged"} {
				if phase == "transfer-completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				assertCriticalCargoUnchanged(t, p, before, fallback)
				if !session.IsClosed() || w.players[session] != p || w.playersByID[id] != p ||
					w.playersByCharacterUID[uid] != p || w.accountSessions[key] != session ||
					len(w.criticalDisconnects) != 1 || len(queue.pending) != 1 || len(base.entered) != 0 ||
					w.claimAccountSession(reconnecting, p.Account.Name) {
					t.Fatalf("%s disconnected transfer lost ownership or submitted stale final balances", phase)
				}
			}
			w.processCriticalCompletions(1)
			final := awaitCriticalWrite(t, base.entered)
			gold, cargo := uint32(1200), uint32(300)
			if deposit {
				gold, cargo = 800, 700
			}
			if p.Char.Gold != gold || p.Account.CargoGold != cargo || final.accounts[0] == p.Account ||
				final.accounts[0].Chars[p.CharSlot].Gold != gold || final.accounts[0].CargoGold != cargo ||
				final.character == nil || final.character.uid != uid ||
				!reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) {
				t.Fatal("atomic final account/sidecar snapshot omitted the committed transfer")
			}
			for _, phase := range []string{"final-held", "final-completed-unacknowledged"} {
				if phase == "final-completed-unacknowledged" {
					releaseFinal()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.handle(command{s: session})
				w.resumeCriticalDisconnects(8)
				if len(queue.pending) != 1 || len(base.entered) != 0 || fallback.saves != 0 ||
					w.players[session] != p || w.accountSessions[key] != session ||
					session.QueuedPacketsForTest() != 0 || w.claimAccountSession(reconnecting, p.Account.Name) {
					t.Fatalf("%s final save was replayed or identity released before acknowledgement", phase)
				}
			}
			w.processCriticalCompletions(1)
			w.handle(command{s: session})
			if w.players[session] != nil || w.playersByID[id] != nil || w.playersByCharacterUID[uid] != nil ||
				w.accountSessions[key] != nil || len(w.criticalDisconnects) != 0 || len(queue.pending) != 0 ||
				len(base.entered) != 0 || fallback.saves != 0 || session.QueuedPacketsForTest() != 0 ||
				!w.claimAccountSession(reconnecting, p.Account.Name) || w.shuttingDown {
				t.Fatal("acknowledged disconnect did not clean up exactly once before readmission")
			}
		})
	}
}

type criticalCargoTerminalStore struct {
	*cargoGoldContractStore
	flushes int
}

func (s *criticalCargoTerminalStore) Flush() { s.flushes++ }

func TestCriticalCargoGoldShutdownPersistsCommittedBalancesAfterDrain(t *testing.T) {
	for _, deposit := range []bool{true, false} {
		t.Run(cargoDirection(deposit), func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalCargoFixture(t)
			terminal := &criticalCargoTerminalStore{cargoGoldContractStore: fallback}
			w.store = terminal
			w.onCargoGold(p.Session, cargoGoldRequest(deposit, 200), deposit)
			awaitCriticalWrite(t, backend.entered)
			first, second := make(chan error, 1), make(chan error, 1)
			w.runShutdown(first)
			w.runShutdown(second)
			w.handle(command{s: p.Session})
			if fallback.saves != 0 || terminal.flushes != 0 || !queue.stopping || w.shutdownFinished ||
				p.Char.Gold != 1000 || p.Account.CargoGold != 500 || !p.Session.IsClosed() {
				t.Fatal("shutdown persisted/released proposed balances while the transfer was held")
			}
			for _, done := range []chan error{first, second} {
				select {
				case <-done:
					t.Fatal("shutdown reported completion before transfer acknowledgement")
				default:
				}
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			for _, done := range []chan error{first, second} {
				select {
				case err := <-done:
					if err != nil {
						t.Fatal(err)
					}
				default:
					t.Fatal("shutdown did not return after the accepted transfer settled")
				}
			}
			gold, cargo := uint32(1200), uint32(300)
			if deposit {
				gold, cargo = 800, 700
			}
			if fallback.saves != 1 || terminal.flushes != 1 || fallback.gold != gold || fallback.cargo != cargo ||
				p.Char.Gold != gold || p.Account.CargoGold != cargo || !w.shutdownFinished || len(queue.pending) != 0 ||
				p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("terminal snapshot omitted the committed balances or published to a closed transport")
			}
			third := make(chan error, 1)
			w.runShutdown(third)
			if err := <-third; err != nil || fallback.saves != 1 || terminal.flushes != 1 {
				t.Fatal("repeated shutdown replayed the terminal snapshot")
			}
		})
	}
}

func criticalCargoLoopFixture(t *testing.T, failure error) (*World, *Player, string, *criticalTestStore, *criticalPersistence, chan time.Time, func(), func()) {
	t.Helper()
	return criticalEconomyLoopFixture(t, failure, nil)
}

// configure runs before the actor starts; related economy tests must not mutate
// maps or player state after the fixture returns a live World.
func criticalEconomyLoopFixture(t *testing.T, failure error, configure func(*World, *Player)) (*World, *Player, string, *criticalTestStore, *criticalPersistence, chan time.Time, func(), func()) {
	t.Helper()
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate, failure: failure}
	w, err := NewWorld(backend, nil, nil, model.Catalog{}, [model.MaxCarry]int{}, model.VolatileCatalog{},
		model.CharacterTemplateFile{}, model.TerrainMap{})
	if err != nil {
		t.Fatal(err)
	}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	p, _ := networkedTestPlayer(1, "CargoOwner", 2100, 2100)
	unrelated, _ := networkedTestPlayer(2, "Unrelated", 2110, 2100)
	for _, player := range []*Player{p, unrelated} {
		w.players[player.Session], w.playersByID[player.ID] = player, player
		w.indexPlayerCharacter(player)
		w.updatePlayerSpatial(player)
		if !w.claimAccountSession(player.Session, player.Account.Name) {
			t.Fatal("fixture account admission failed")
		}
		player.NextCPRecovery = w.now().Add(time.Hour)
		player.NextRegen = w.now().Add(time.Hour)
	}
	banker := &Mob{ID: 1102, X: 2102, Y: 2100, Def: &model.NPCDef{
		Name: "Cargo", Tipo: model.TipoNPC, Score: &model.Score{Merchant: 2},
	}}
	w.registerMobSpatial(banker)
	p.show(banker.ID)
	p.CargoNPC = banker.ID
	p.Char.Gold, p.Account.CargoGold = 1000, 500
	setPlayerCurHP(unrelated.Char, 500)
	unrelated.NextRegen = w.now().Add(-time.Second)
	unrelatedUID := unrelated.Char.UID
	// This fixture covers detached economy intentions, not pending autosave.
	w.nextAutoSave = w.now().Add(time.Hour)
	if configure != nil {
		configure(w, p)
	}
	ticks, stop, stopped := make(chan time.Time), make(chan struct{}), make(chan struct{})
	go func() {
		defer close(stopped)
		w.run(ticks, stop)
	}()
	var once sync.Once
	stopLoop := func() {
		once.Do(func() { close(stop) })
		select {
		case <-stopped:
		case <-time.After(2 * time.Second):
			t.Error("World loop did not stop")
		}
	}
	t.Cleanup(stopLoop)
	return w, p, unrelatedUID, backend, queue, ticks, release, stopLoop
}

func deliverCargoLoopRequest(t *testing.T, w *World, p *Player, deposit bool) {
	t.Helper()
	select {
	case w.commands <- command{s: p.Session, pkt: cargoGoldRequest(deposit, 200)}:
	case <-time.After(time.Second):
		t.Fatal("World did not accept the cargo intention")
	}
}

func TestCriticalCargoGoldActualDispatchKeepsWorldResponsiveDuringCommit(t *testing.T) {
	for _, deposit := range []bool{true, false} {
		t.Run(cargoDirection(deposit), func(t *testing.T) {
			w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalCargoLoopFixture(t, nil)
			deliverCargoLoopRequest(t, w, p, deposit)
			write := awaitCriticalWrite(t, backend.entered)
			gold, cargo := uint32(1200), uint32(300)
			if deposit {
				gold, cargo = 800, 700
			}
			if write.accounts[0].Chars[0].Gold != gold || write.accounts[0].CargoGold != cargo {
				t.Fatal("actual dispatcher did not submit the proposed transfer")
			}
			for range 3 {
				deliverCriticalLoopPing(t, w, p)
				deliverCriticalLoopTick(t, ticks)
				overview := readCriticalLoopOverview(t, w)
				regenerated := false
				for _, player := range overview.Players {
					if player.UID == unrelatedUID && player.HP > 500 {
						regenerated = true
					}
				}
				if overview.State != "running" || !regenerated || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("held transfer blocked unrelated ticks/commands or published before commit")
				}
			}
			readCriticalLoopOverview(t, w)
			if len(backend.entered) != 0 {
				t.Fatal("repeated packet dispatch submitted another pending transfer")
			}
			release()
			deadline := time.Now().Add(2 * time.Second)
			for p.Session.QueuedPacketsForTest() < 3 && time.Now().Before(deadline) {
				readCriticalLoopOverview(t, w)
			}
			stop()
			// Join World before reading mutable balances, tokens and maps.
			if p.Char.Gold != gold || p.Account.CargoGold != cargo || len(queue.pending) != 0 ||
				w.criticalPlayerReserved(p) || w.mobTickCounter < 3 || len(backend.entered) != 0 || w.shuttingDown {
				t.Fatal("actual loop failed to settle the transfer once and continue unrelated ticks")
			}
			assertCargoGoldPublication(t, p, deposit, 200, gold, cargo)
		})
	}
}

func TestCriticalCargoGoldActualDispatchUnknownOutcomeEntersMaintenance(t *testing.T) {
	w, p, _, backend, queue, ticks, release, stop := criticalCargoLoopFixture(t, errors.New("commit acknowledgement lost"))
	deliverCargoLoopRequest(t, w, p, true)
	awaitCriticalWrite(t, backend.entered)
	release()
	deadline := time.Now().Add(2 * time.Second)
	maintenance := false
	for time.Now().Before(deadline) {
		if readCriticalLoopOverview(t, w).State == "maintenance" {
			maintenance = true
			break
		}
	}
	if !maintenance {
		t.Fatal("actual loop did not isolate the unknown transfer outcome")
	}
	deliverCargoLoopRequest(t, w, p, true)
	deliverCriticalLoopTick(t, ticks)
	readCriticalLoopOverview(t, w)
	stop()
	if p.Char.Gold != 1000 || p.Account.CargoGold != 500 || p.Session.QueuedPacketsForTest() != 0 ||
		!p.PersistencePoisoned || !p.Session.IsClosed() || len(queue.pending) != 1 || len(backend.entered) != 0 ||
		!errors.Is(w.shutdownErr, errCriticalPersistenceIndeterminate) || w.mobTickCounter != 0 {
		t.Fatal("unknown transfer applied, published, retried, released its fence or allowed gameplay ticks")
	}
}
