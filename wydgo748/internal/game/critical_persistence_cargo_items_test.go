package game

import (
	"bytes"
	"encoding/binary"
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

func criticalCargoSwapRequest(p *Player, st, sp, dt, dp byte) []byte {
	packet := inboundPacket(wire.OpSwapItem, 20)
	binary.LittleEndian.PutUint16(packet[6:8], p.ID)
	packet[12], packet[13], packet[14], packet[15] = st, sp, dt, dp
	binary.LittleEndian.PutUint32(packet[16:20], 1102)
	return packet
}

func seedCriticalCargoItems(p *Player) {
	p.Char.Inv[3] = model.Item{Index: 4300, UID: "cargo-item-a", Eff: [6]byte{43, 4},
		ActivatedUnix: 1_700_000_000, ExpiresUnix: 2_700_000_000}
	p.Account.Cargo[7] = model.Item{Index: 4200, UID: "cargo-item-b", Eff: [6]byte{2, 5}}
	p.Account.Cargo[119] = model.Item{Index: 4011, UID: "cargo-item-c", Eff: [6]byte{3, 6}}
}

func assertCriticalCargoSwapPublication(t *testing.T, p *Player, request []byte) {
	t.Helper()
	expected := wire.SwapItem(p.ID, request[12], request[13], request[14], request[15],
		binary.LittleEndian.Uint16(request[16:18]))
	packet, ok := p.Session.DequeuePacketForTest()
	if !ok || len(packet) != 20 || !wire.Decrypt(packet) {
		t.Fatal("Cargo swap must publish one encrypted 20-byte cursor completion")
	}
	header := wire.ParseHeader(packet)
	if header.Type != wire.OpSwapItem || header.Size != 20 || header.ID != p.ID ||
		!bytes.Equal(packet[12:], expected[12:]) || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("Cargo completion changed the recipient, full payload or single-response contract")
	}
}

func TestCriticalCargoItemSwapDetachedCommitAndAcknowledgement(t *testing.T) {
	for _, path := range []struct {
		name           string
		st, sp, dt, dp byte
	}{
		{"inventory-cargo", placeInv, 3, placeStorage, 7},
		{"cargo-inventory", placeStorage, 7, placeInv, 3},
		{"cargo-cargo", placeStorage, 7, placeStorage, 119},
		{"empty-destination", placeInv, 3, placeStorage, 8},
		{"empty-source", placeStorage, 8, placeInv, 3},
		{"empty-slots", placeStorage, 8, placeStorage, 9},
		{"same-slot", placeStorage, 7, placeStorage, 7},
	} {
		for _, secondSlot := range []bool{false, true} {
			name := path.name + "/first-character"
			if secondSlot {
				name = path.name + "/second-character"
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
				seedCriticalCargoItems(p)
				account, character, score := p.Account, p.Char, p.Char.Score
				before := accountStateSnapshot(account)
				proposed := accountStateSnapshot(account)
				planned := &Player{Account: proposed, Char: &proposed.Chars[p.CharSlot]}
				a, b := playerSlotOf(planned, path.st, path.sp), playerSlotOf(planned, path.dt, path.dp)
				*a, *b = *b, *a
				request := criticalCargoSwapRequest(p, path.st, path.sp, path.dt, path.dp)
				w.onSwapItem(p.Session, request)
				write := awaitCriticalWrite(t, backend.entered)
				if len(write.accounts) != 1 || write.character != nil || write.instances != nil ||
					write.accounts[0] == account || &write.accounts[0].Chars[p.CharSlot] == character ||
					!reflect.DeepEqual(write.accounts[0], accountPersistenceSnapshot(proposed)) {
					t.Fatal("worker did not receive the exact detached two-slot account proposal")
				}
				p.CargoNPC = 0
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, queue, 1)
					}
					w.onSwapItem(p.Session, request)
					assertCriticalCargoUnchanged(t, p, before, fallback)
					if p.CargoNPC != 0 || !w.criticalPlayerReserved(p) || len(queue.pending) != 1 ||
						len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 {
						t.Fatalf("%s mutated context, duplicated, confirmed or released pending work", phase)
					}
				}
				w.processCriticalCompletions(1)
				if p.Account != account || p.Char != character || p.Char.Score != score ||
					!reflect.DeepEqual(accountStateSnapshot(account), proposed) || fallback.saves != 0 ||
					w.criticalPlayerReserved(p) || len(queue.pending) != 0 {
					t.Fatal("completion lost identity, item metadata or unrelated account state")
				}
				assertCriticalCargoSwapPublication(t, p, request)
				// No transaction ID exists: an identical post-ack request swaps back.
				w.onSwapItem(p.Session, request)
				write = awaitCriticalWrite(t, backend.entered)
				if !reflect.DeepEqual(write.accounts[0], accountPersistenceSnapshot(before)) ||
					!reflect.DeepEqual(accountStateSnapshot(account), proposed) || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("post-ack request was deduplicated or applied before its own commit")
				}
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if !reflect.DeepEqual(accountStateSnapshot(account), before) {
					t.Fatal("second committed swap did not restore the original two slots")
				}
				assertCriticalCargoSwapPublication(t, p, request)
			})
		}
	}
}

func TestCriticalCargoItemSwapRejectsWithoutMutationOrInlineFallback(t *testing.T) {
	for _, reason := range []string{"full", "stopping", "account-owner", "character-index", "slot-pointer",
		"closed", "poisoned", "world-stopping", "source-limit", "destination-limit", "inventory-limit",
		"source-lock", "destination-lock", "hidden-banker", "distant-banker", "wrong-target",
		"target-width", "cargo-equipment", "truncated", "oversized"} {
		t.Run(reason, func(t *testing.T) {
			w, p, fallback, backend, queue, _ := criticalCargoFixture(t)
			seedCriticalCargoItems(p)
			request := criticalCargoSwapRequest(p, placeInv, 3, placeStorage, 7)
			wantResponse, pending := false, 0
			switch reason {
			case "full":
				for _, name := range []string{"occupied-a", "occupied-b"} {
					if _, err := queue.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}); err != nil {
						t.Fatal(err)
					}
				}
				awaitCriticalWrite(t, backend.entered)
				wantResponse, pending = true, 2
			case "stopping":
				queue.closeAdmission()
				wantResponse = true
			case "account-owner":
				delete(w.accountSessions, accountSessionKey(p.Account.Name))
				wantResponse = true
			case "character-index":
				delete(w.playersByCharacterUID, p.Char.UID)
				wantResponse = true
			case "slot-pointer":
				copy := cloneCharacterState(p.Char)
				p.Char = &copy
				wantResponse = true
			case "closed":
				p.Session.Close()
			case "poisoned":
				p.PersistencePoisoned = true
			case "world-stopping":
				w.shuttingDown = true
			case "source-limit":
				request[12], request[13] = placeStorage, model.PlayerCargoSlots
			case "destination-limit":
				request[15] = model.PlayerCargoSlots
			case "inventory-limit":
				request[13] = model.PlayerCarrySlots
			case "source-lock", "destination-lock":
				p.GhostShop = &GhostShop{CarryPos: emptyGhostShopPositions()}
				p.GhostShop.CarryPos[0], p.GhostShop.Items[0] = 7, p.Account.Cargo[7]
				if reason == "source-lock" {
					request = criticalCargoSwapRequest(p, placeStorage, 7, placeInv, 3)
				}
				wantResponse = true
			case "hidden-banker":
				delete(p.Visible, 1102)
				wantResponse = true
			case "distant-banker":
				p.X += npcInteractionRange + 20
				wantResponse = true
			case "wrong-target":
				binary.LittleEndian.PutUint32(request[16:], 999)
				wantResponse = true
			case "target-width":
				binary.LittleEndian.PutUint32(request[16:], 0x10000+1102)
				wantResponse = true
			case "cargo-equipment":
				request = criticalCargoSwapRequest(p, placeStorage, 7, placeEquip, 1)
				wantResponse = true
			case "truncated":
				request = request[:19]
			case "oversized":
				request = append(request, 0)
			}
			before := accountStateSnapshot(p.Account)
			w.onSwapItem(p.Session, request)
			assertCriticalCargoUnchanged(t, p, before, fallback)
			if len(queue.pending) != pending || len(backend.entered) != 0 || w.criticalPlayerReserved(p) {
				t.Fatal("rejection retained work, reserved the participant or saved inline")
			}
			responses, panels, snapshots := 0, 0, 0
			for {
				packet, ok := p.Session.DequeuePacketForTest()
				if !ok {
					break
				}
				if !wire.Decrypt(packet) || (wire.ParseHeader(packet).Type != wire.OpMessagePanel &&
					wire.ParseHeader(packet).Type != wire.OpSendItem) {
					t.Fatal("rejection emitted a successful cursor completion")
				}
				if wire.ParseHeader(packet).Type == wire.OpMessagePanel {
					panels++
				} else {
					typ, pos := request[12], request[13]
					if snapshots == 1 {
						typ, pos = request[14], request[15]
					}
					expected := wire.SendItem(p.ID, typ, pos, *playerSlotOf(p, typ, pos))
					if wire.ParseHeader(packet).ID != p.ID || int(wire.ParseHeader(packet).Size) != len(expected) ||
						!bytes.Equal(packet[12:], expected[12:]) {
						t.Fatal("rejection snapshot did not preserve the authoritative slot/item payload")
					}
					snapshots++
				}
				responses++
			}
			if (wantResponse && (responses != 3 || panels != 1 || snapshots != 2)) || (!wantResponse && responses != 0) {
				t.Fatalf("rejection produced %d responses, want panel plus two snapshots=%v", responses, wantResponse)
			}
		})
	}
}

func TestCriticalCargoItemSwapReservedTradeCounterpartRetainsContext(t *testing.T) {
	w, p, fallback, backend, queue, release := criticalCargoFixture(t)
	seedCriticalCargoItems(p)
	opponent, _ := networkedTestPlayer(2, "TradePeer", 2101, 2100)
	w.players[opponent.Session], w.playersByID[opponent.ID] = opponent, opponent
	w.indexPlayerCharacter(opponent)
	if !w.claimAccountSession(opponent.Session, opponent.Account.Name) {
		t.Fatal("trade counterpart account admission failed")
	}
	p.Trade, opponent.Trade = &TradeState{OpponentID: opponent.ID}, &TradeState{OpponentID: p.ID}
	trade, peerTrade := p.Trade, opponent.Trade
	if _, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{opponent.Account}}, []*Player{opponent},
		func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	p.CargoNPC = 0
	before := accountStateSnapshot(p.Account)
	request := criticalCargoSwapRequest(p, placeInv, 3, placeStorage, 7)
	for _, phase := range []string{"held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.onSwapItem(p.Session, request)
		assertCriticalCargoUnchanged(t, p, before, fallback)
		if p.CargoNPC != 0 || p.Trade != trade || opponent.Trade != peerTrade ||
			p.Session.QueuedPacketsForTest() != 0 || opponent.Session.QueuedPacketsForTest() != 0 ||
			len(queue.pending) != 1 || len(backend.entered) != 0 {
			t.Fatalf("%s reserved counterpart lost trade/banker context or admitted a swap", phase)
		}
	}
	w.processCriticalCompletions(1)
	w.onSwapItem(p.Session, request)
	awaitCriticalWrite(t, backend.entered)
	if p.Trade != nil || opponent.Trade != nil || p.CargoNPC != 1102 {
		t.Fatal("eligible swap did not preserve banker binding and original trade cancellation")
	}
	for _, player := range []*Player{p, opponent} {
		packet, ok := player.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpCloseTrade ||
			player.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("eligible trade cancellation must precede swap confirmation")
		}
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	assertCriticalCargoSwapPublication(t, p, request)
}

func TestCriticalCargoItemSwapUnknownOutcomeDoesNotApplyReplayOrOverwrite(t *testing.T) {
	for _, reason := range []string{"error", "panic", "identity"} {
		t.Run(reason, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalCargoFixture(t)
			seedCriticalCargoItems(p)
			if reason == "error" {
				backend.failure = errors.New("commit acknowledgement lost")
			}
			backend.panicAfterWrite = reason == "panic"
			before := accountStateSnapshot(p.Account)
			request := criticalCargoSwapRequest(p, placeInv, 3, placeStorage, 7)
			w.onSwapItem(p.Session, request)
			awaitCriticalWrite(t, backend.entered)
			w.handle(command{s: p.Session})
			if reason == "identity" {
				p.criticalLifecycle = &criticalPlayerLifecycle{}
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			w.onSwapItem(p.Session, request)
			w.resumeCriticalDisconnects(8)
			assertCriticalCargoUnchanged(t, p, before, fallback)
			if !w.shuttingDown || !p.PersistencePoisoned || !p.Session.IsClosed() ||
				!w.criticalPlayerReserved(p) || len(queue.pending) != 1 || len(backend.entered) != 0 ||
				w.players[p.Session] != p || p.Session.QueuedPacketsForTest() != 0 ||
				w.claimAccountSession(gameNet.NewTestSession(3, 8), p.Account.Name) {
				t.Fatal("uncertain swap applied, confirmed, replayed or released account ownership")
			}
			done := make(chan error, 1)
			w.runShutdown(done)
			wantError := errCriticalPersistenceIndeterminate
			if reason == "identity" {
				wantError = errCriticalPersistenceIdentity
			}
			if err := <-done; !errors.Is(err, wantError) || fallback.saves != 0 {
				t.Fatal("terminal persistence overwrote the uncertain swap or lost its error")
			}
		})
	}
}

func TestCriticalCargoItemSwapCapacityReturnsWithoutReplayingRejectedRequest(t *testing.T) {
	w, p, fallback, backend, queue, release := criticalCargoFixture(t)
	seedCriticalCargoItems(p)
	for _, name := range []string{"occupied-a", "occupied-b"} {
		if _, err := w.criticalWrites.submit(criticalWrite{accounts: []*model.Account{{Name: name}}},
			func(criticalPersistenceResult) error { return nil }); err != nil {
			t.Fatal(err)
		}
	}
	awaitCriticalWrite(t, backend.entered)
	before := accountStateSnapshot(p.Account)
	request := criticalCargoSwapRequest(p, placeInv, 3, placeStorage, 7)
	w.onSwapItem(p.Session, request)
	assertCriticalCargoUnchanged(t, p, before, fallback)
	if p.Session.QueuedPacketsForTest() != 3 || w.criticalPlayerReserved(p) || len(queue.pending) != 2 {
		t.Fatal("capacity rejection retained a pending swap or did not repair the client slots")
	}
	for p.Session.QueuedPacketsForTest() != 0 {
		p.Session.DequeuePacketForTest()
	}
	release()
	awaitCriticalWrite(t, backend.entered)
	awaitReadyCriticalResults(t, queue, 2)
	w.processCriticalCompletions(2)
	if len(queue.pending) != 0 || len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("capacity recovery replayed a request that admission had rejected")
	}
	w.onSwapItem(p.Session, request)
	proposal := awaitCriticalWrite(t, backend.entered)
	assertCriticalCargoUnchanged(t, p, before, fallback)
	if proposal.accounts[0].Chars[0].Inv[3] != before.Cargo[7] ||
		proposal.accounts[0].Cargo[7] != before.Chars[0].Inv[3] || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("a new request did not submit one detached swap after capacity returned")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	assertCriticalCargoSwapPublication(t, p, request)
}

type criticalCargoSwapInlineStore struct {
	*craftStore
	player  *Player
	t       *testing.T
	account *model.Account
}

func (s *criticalCargoSwapInlineStore) SaveAccount(account *model.Account) error {
	s.t.Helper()
	if s.player.Session.QueuedPacketsForTest() != 0 {
		s.t.Fatal("default Cargo path confirmed before persistence")
	}
	s.account = accountStateSnapshot(account)
	return s.craftStore.SaveAccount(account)
}

func TestCriticalCargoItemSwapDefaultPathStillSavesBeforeConfirmAndRollsBack(t *testing.T) {
	for _, failure := range []error{nil, errors.New("save rejected")} {
		name := "success"
		if failure != nil {
			name = "rollback"
		}
		t.Run(name, func(t *testing.T) {
			w, p, fallback := cargoGoldContractWorld(t)
			seedCriticalCargoItems(p)
			backend := &criticalCargoSwapInlineStore{craftStore: &fallback.craftStore, player: p, t: t}
			backend.err = failure
			w.store = backend
			before := accountStateSnapshot(p.Account)
			request := criticalCargoSwapRequest(p, placeInv, 3, placeStorage, 7)
			w.onSwapItem(p.Session, request)
			if w.criticalWrites != nil || fallback.saves != 1 || backend.account == nil ||
				backend.account.Chars[0].Inv[3] != before.Cargo[7] || backend.account.Cargo[7] != before.Chars[0].Inv[3] {
				t.Fatal("default path did not synchronously submit the original swap proposal")
			}
			if failure != nil {
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Session.QueuedPacketsForTest() != 3 {
					t.Fatal("default save failure did not roll back and repair client slots")
				}
			} else {
				if p.Char.Inv[3] != before.Cargo[7] || p.Account.Cargo[7] != before.Chars[0].Inv[3] {
					t.Fatal("default successful swap did not install the saved item values")
				}
				assertCriticalCargoSwapPublication(t, p, request)
			}
		})
	}
}

func TestCriticalCargoItemSwapDisconnectFinalSnapshotAndReadmission(t *testing.T) {
	w, p, fallback := cargoGoldContractWorld(t)
	seedCriticalCargoItems(p)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("fixture account admission failed")
	}
	gate, release := criticalGate(t)
	finalGate, releaseFinal := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	backend := &criticalCargoDisconnectStore{criticalTestStore: base, finalRelease: finalGate}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	t.Cleanup(releaseFinal)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	before := accountStateSnapshot(p.Account)
	session, uid, id, key := p.Session, p.Char.UID, p.ID, accountSessionKey(p.Account.Name)
	reconnecting := gameNet.NewTestSession(3, 8)
	w.onSwapItem(session, criticalCargoSwapRequest(p, placeInv, 3, placeStorage, 7))
	proposal := awaitCriticalWrite(t, base.entered)
	w.handle(command{s: session})
	for _, phase := range []string{"swap-held", "swap-completed-unacknowledged"} {
		if phase == "swap-completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.handle(command{s: session})
		assertCriticalCargoUnchanged(t, p, before, fallback)
		if !session.IsClosed() || w.players[session] != p || w.playersByID[id] != p ||
			w.playersByCharacterUID[uid] != p || w.accountSessions[key] != session ||
			len(queue.pending) != 1 || len(base.entered) != 0 || w.claimAccountSession(reconnecting, p.Account.Name) {
			t.Fatalf("%s lost disconnect ownership or submitted a stale final snapshot", phase)
		}
	}
	w.processCriticalCompletions(1)
	final := awaitCriticalWrite(t, base.entered)
	if !reflect.DeepEqual(final.accounts[0], proposal.accounts[0]) || final.character == nil ||
		final.character.uid != uid || !reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) ||
		p.Char.Inv[3] != before.Cargo[7] || p.Account.Cargo[7] != before.Chars[0].Inv[3] {
		t.Fatal("atomic final account/sidecar omitted the committed item identities/metadata")
	}
	for _, phase := range []string{"final-held", "final-completed-unacknowledged"} {
		if phase == "final-completed-unacknowledged" {
			releaseFinal()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.handle(command{s: session})
		w.resumeCriticalDisconnects(8)
		if len(queue.pending) != 1 || len(base.entered) != 0 || fallback.saves != 0 ||
			w.players[session] != p || w.accountSessions[key] != session || session.QueuedPacketsForTest() != 0 ||
			w.claimAccountSession(reconnecting, p.Account.Name) {
			t.Fatalf("%s replayed final persistence or released readmission early", phase)
		}
	}
	w.processCriticalCompletions(1)
	if w.players[session] != nil || w.playersByID[id] != nil || w.playersByCharacterUID[uid] != nil ||
		w.accountSessions[key] != nil || len(queue.pending) != 0 || len(base.entered) != 0 || fallback.saves != 0 ||
		session.QueuedPacketsForTest() != 0 || !w.claimAccountSession(reconnecting, p.Account.Name) || w.shuttingDown {
		t.Fatal("final acknowledgement failed to release exactly once without closed-transport confirmation")
	}
}

func TestCriticalCargoItemSwapShutdownUsesCommittedSnapshotAfterDrain(t *testing.T) {
	w, p, fallback, backend, queue, release := criticalCargoFixture(t)
	seedCriticalCargoItems(p)
	terminal := &criticalPurchaseTerminalStore{craftStore: &fallback.craftStore}
	w.store = terminal
	before := accountStateSnapshot(p.Account)
	w.onSwapItem(p.Session, criticalCargoSwapRequest(p, placeInv, 3, placeStorage, 7))
	proposal := awaitCriticalWrite(t, backend.entered)
	first, second := make(chan error, 1), make(chan error, 1)
	w.runShutdown(first)
	w.runShutdown(second)
	w.handle(command{s: p.Session})
	assertCriticalCargoUnchanged(t, p, before, fallback)
	if terminal.account != nil || terminal.flushes != 0 || !queue.stopping || w.shutdownFinished || !p.Session.IsClosed() {
		t.Fatal("shutdown saved proposed/stale items or finished before the accepted swap")
	}
	for _, done := range []chan error{first, second} {
		select {
		case <-done:
			t.Fatal("shutdown finished before swap acknowledgement")
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
			t.Fatal("shutdown did not finish after the accepted swap settled")
		}
	}
	if terminal.account == nil || !reflect.DeepEqual(accountPersistenceSnapshot(terminal.account), proposal.accounts[0]) ||
		p.Char.Inv[3] != before.Cargo[7] || p.Account.Cargo[7] != before.Chars[0].Inv[3] ||
		fallback.saves != 1 || terminal.flushes != 1 || !w.shutdownFinished || len(queue.pending) != 0 ||
		p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("terminal snapshot lost the committed swap, saved twice or confirmed to a closed session")
	}
	third := make(chan error, 1)
	w.runShutdown(third)
	if err := <-third; err != nil || fallback.saves != 1 || terminal.flushes != 1 {
		t.Fatal("repeated shutdown replayed the terminal snapshot")
	}
}

func TestCriticalCargoItemSwapActualDispatchKeepsWorldResponsiveAndContainsUnknownOutcome(t *testing.T) {
	for _, unknown := range []bool{false, true} {
		name, failure := "success", error(nil)
		if unknown {
			name, failure = "unknown", errors.New("commit acknowledgement lost")
		}
		t.Run(name, func(t *testing.T) {
			w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, failure,
				func(_ *World, p *Player) { seedCriticalCargoItems(p) })
			request := criticalCargoSwapRequest(p, placeInv, 3, placeStorage, 7)
			deliver := func() {
				select {
				case w.commands <- command{s: p.Session, pkt: request}:
				case <-time.After(time.Second):
					t.Fatal("World did not accept the Cargo swap intention")
				}
			}
			deliver()
			proposal := awaitCriticalWrite(t, backend.entered)
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
					t.Fatal("held swap blocked unrelated ticks/commands or confirmed early")
				}
			}
			release()
			deadline := time.Now().Add(2 * time.Second)
			settled := false
			for time.Now().Before(deadline) {
				state := readCriticalLoopOverview(t, w).State
				if (unknown && state == "maintenance") || (!unknown && p.Session.QueuedPacketsForTest() == 1) {
					settled = true
					break
				}
			}
			if !settled {
				t.Fatal("actual actor did not deliver the swap outcome")
			}
			if unknown {
				deliver()
				deliverCriticalLoopTick(t, ticks)
				readCriticalLoopOverview(t, w)
			}
			stop()
			// Join the actor before reading maps, reservations and live items.
			if len(backend.entered) != 0 || w.mobTickCounter < 3 {
				t.Fatal("repeated dispatch duplicated a swap or the World did not advance ticks")
			}
			if unknown {
				if p.Char.Inv[3].UID != "cargo-item-a" || p.Account.Cargo[7].UID != "cargo-item-b" ||
					p.Session.QueuedPacketsForTest() != 0 || !p.PersistencePoisoned || !p.Session.IsClosed() ||
					len(queue.pending) != 1 || !errors.Is(w.shutdownErr, errCriticalPersistenceIndeterminate) {
					t.Fatal("unknown swap applied, confirmed, replayed or released ownership")
				}
			} else {
				if p.Char.Inv[3] != proposal.accounts[0].Chars[0].Inv[3] ||
					p.Account.Cargo[7] != proposal.accounts[0].Cargo[7] || len(queue.pending) != 0 ||
					w.criticalPlayerReserved(p) || w.shuttingDown {
					t.Fatal("actual actor did not install precisely the committed two slots")
				}
				assertCriticalCargoSwapPublication(t, p, request)
			}
		})
	}
}
