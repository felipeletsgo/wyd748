package game

import (
	"bytes"
	"errors"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

type criticalPurchaseRNG struct{ calls int }

func (r *criticalPurchaseRNG) Intn(int) int { r.calls++; return 0 }

func criticalHellStoreFixture(t *testing.T) (*World, *Player, *craftStore, *Mob, *criticalTestStore, *criticalPersistence, *criticalPurchaseRNG, func()) {
	t.Helper()
	w, p, fallback, merchant := hellStoreWorld(t)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("fixture account admission failed")
	}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	rng := &criticalPurchaseRNG{}
	w.rng = rng
	return w, p, fallback, merchant, backend, queue, rng, release
}

func assertHellStorePublication(t *testing.T, p *Player) {
	t.Helper()
	for _, expected := range [][]byte{wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold), wire.UpdateEtc(p.ID, *p.Char)} {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) || len(packet) != len(expected) {
			t.Fatal("purchase response is missing or has the wrong encrypted frame size")
		}
		header, want := wire.ParseHeader(packet), wire.ParseHeader(expected)
		// Builders leave Size/Tick unset; Session.Send finalizes those fields.
		if header.ID != want.ID || header.Type != want.Type || int(header.Size) != len(expected) || !bytes.Equal(packet[12:], expected[12:]) {
			t.Fatal("purchase response order/content differs from the committed inventory and gold")
		}
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("purchase emitted an unexpected extra response")
	}
}

func TestCriticalHellStorePurchaseProposesOnceAndPublishesAfterCommit(t *testing.T) {
	for option := hellStoreWeapon; option <= hellStoreRandom; option++ {
		for _, slot := range []int{0, 1} {
			t.Run(fmt.Sprintf("category-%d/character-%d", option, slot), func(t *testing.T) {
				w, p, fallback, merchant, backend, queue, rng, release := criticalHellStoreFixture(t)
				if slot == 1 {
					oldUID := p.Char.UID
					p.Account.Chars = append(p.Account.Chars, cloneCharacterState(p.Char))
					p.CharSlot, p.Char = 1, &p.Account.Chars[1]
					p.Char.UID += "-second"
					delete(w.playersByCharacterUID, oldUID)
					w.indexPlayerCharacter(p)
				}
				before, account, character, score := accountStateSnapshot(p.Account), p.Account, p.Char, p.Char.Score
				request := hellStorePacket(p.ID, merchant.ID, int16(option), 7)
				w.handle(command{s: p.Session, pkt: request})
				write := awaitCriticalWrite(t, backend.entered)
				item := write.accounts[0].Chars[slot].Inv[7]
				gold := before.Chars[slot].Gold - uint32(hellStorePrices[option])
				expected := accountPersistenceSnapshot(before)
				expected.Chars[slot].Inv[7], expected.Chars[slot].Gold = item, gold
				inPool := false
				for category := hellStoreWeapon; category <= hellStoreAmulet; category++ {
					if (option == hellStoreRandom || option == category) && containsIndex(hellStorePool(w.items, category), item.Index) {
						inPool = true
					}
				}
				if !inPool || item.UID == "" || write.character != nil || write.instances != nil ||
					write.accounts[0] == account || !reflect.DeepEqual(write.accounts[0], expected) || rng.calls == 0 {
					t.Fatal("worker did not receive the selected-category detached gold/item proposal")
				}
				rolls := rng.calls
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, queue, 1)
					}
					w.onHellStoreBuy(p.Session, request)
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || rng.calls != rolls || fallback.saves != 0 ||
						p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 || len(backend.entered) != 0 ||
						!w.criticalPlayerReserved(p) {
						t.Fatalf("%s purchase charged, granted, rerolled, confirmed or duplicated before acknowledgement", phase)
					}
				}
				w.processCriticalCompletions(1)
				expectedLive := accountStateSnapshot(before)
				expectedLive.Chars[slot].Inv[7], expectedLive.Chars[slot].Gold = item, gold
				if p.Account != account || p.Char != character || p.Char.Score != score || rng.calls != rolls ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), expectedLive) || len(queue.pending) != 0 || fallback.saves != 0 {
					t.Fatal("purchase acknowledgement replaced identity/unrelated fields, rerolled or omitted committed state")
				}
				assertHellStorePublication(t, p)
				w.onHellStoreBuy(p.Session, request)
				assertCargoErrorPanel(t, p)
				if rng.calls != rolls || len(queue.pending) != 0 || p.Char.Gold != gold || p.Char.Inv[7] != item {
					t.Fatal("post-commit duplicate into the now-occupied slot charged, rerolled or duplicated an item")
				}
				if uint64(gold) >= hellStorePrices[option] {
					w.onHellStoreBuy(p.Session, hellStorePacket(p.ID, merchant.ID, int16(option), 8))
					next := awaitCriticalWrite(t, backend.entered)
					if next.accounts[0].Chars[slot].Inv[8].UID == item.UID || next.accounts[0].Chars[slot].Inv[8].Index == 0 ||
						next.accounts[0].Chars[slot].Gold != gold-uint32(hellStorePrices[option]) || p.Char.Inv[8].Index != 0 {
						t.Fatal("next empty-slot purchase did not create a new detached economic identity and charge")
					}
					awaitReadyCriticalResults(t, queue, 1)
					w.processCriticalCompletions(1)
					assertHellStorePublication(t, p)
				}
			})
		}
	}
}

func TestCriticalHellStoreRejectionsDoNotGrantChargeOrSaveInline(t *testing.T) {
	for _, reason := range []string{"full", "stopping", "account-owner", "slot-pointer", "closed", "poisoned", "world-stopping",
		"wrong-id", "other-merchant", "hidden-merchant", "distant-merchant", "category", "negative-slot", "outside-slot",
		"occupied-slot", "insufficient-gold", "empty-pool", "dead", "out-of-world", "truncated", "oversized"} {
		t.Run(reason, func(t *testing.T) {
			w, p, fallback, merchant, backend, queue, _, _ := criticalHellStoreFixture(t)
			request, wantPanel := hellStorePacket(p.ID, merchant.ID, hellStoreWeapon, 7), true
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
			case "wrong-id":
				request = hellStorePacket(p.ID+1, merchant.ID, hellStoreWeapon, 7)
				wantPanel = false
			case "other-merchant":
				merchant.Def.Name = "Merchant"
			case "hidden-merchant":
				delete(p.Visible, merchant.ID)
			case "distant-merchant":
				p.X += npcInteractionRange + 10
			case "category":
				request = hellStorePacket(p.ID, merchant.ID, 5, 7)
			case "negative-slot":
				request = hellStorePacket(p.ID, merchant.ID, hellStoreWeapon, -1)
			case "outside-slot":
				request = hellStorePacket(p.ID, merchant.ID, hellStoreWeapon, model.PlayerCarrySlots)
			case "occupied-slot":
				p.Char.Inv[7] = model.Item{Index: 400}
			case "insufficient-gold":
				p.Char.Gold = uint32(hellStorePrices[hellStoreWeapon]) - 1
			case "empty-pool":
				w.items = map[uint16]model.ItemDef{}
			case "dead":
				setPlayerCurHP(p.Char, 0)
				wantPanel = false
			case "out-of-world":
				p.InWorld = false
				wantPanel = false
			case "truncated":
				request = request[:23]
				wantPanel = false
			case "oversized":
				request = append(request, 0)
				wantPanel = false
			}
			before, character, pending, id := accountStateSnapshot(p.Account), cloneCharacterState(p.Char), len(queue.pending), queue.lastID
			w.onHellStoreBuy(p.Session, request)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(*p.Char, character) ||
				fallback.saves != 0 || len(queue.pending) != pending || queue.lastID != id || p.criticalLifecycle != nil ||
				w.criticalPlayerWriteReserved(p) {
				t.Fatal("rejected purchase changed an aggregate, saved inline, installed a token or accepted a job")
			}
			if wantPanel {
				assertCargoErrorPanel(t, p)
			} else if p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("ineligible participant or malformed request received purchase confirmation")
			}
			if reason == "wrong-id" && violationsFor(w, p) != 1 {
				t.Fatal("wrong session ID was not recorded as a security violation")
			}
		})
	}
}

func TestCriticalHellStoreReservedTradeCounterpartIsNotCancelledOrRerolled(t *testing.T) {
	w, p, fallback, merchant, backend, queue, rng, release := criticalHellStoreFixture(t)
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
	before := accountStateSnapshot(p.Account)
	request := hellStorePacket(p.ID, merchant.ID, hellStoreAmulet, 7)
	for _, phase := range []string{"held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.onHellStoreBuy(p.Session, request)
		if p.Trade != trade || opponent.Trade != peerTrade || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
			p.Session.QueuedPacketsForTest() != 0 || opponent.Session.QueuedPacketsForTest() != 0 ||
			rng.calls != 0 || fallback.saves != 0 || len(queue.pending) != 1 || len(backend.entered) != 0 {
			t.Fatalf("%s reserved counterpart was canceled or the purchase was rolled/admitted", phase)
		}
	}
	w.processCriticalCompletions(1)
	w.onHellStoreBuy(p.Session, request)
	awaitCriticalWrite(t, backend.entered)
	if p.Trade != nil || opponent.Trade != nil || p.Char.Inv[7].Index != 0 || p.Char.Gold != 20_000_000 || rng.calls == 0 {
		t.Fatal("eligible purchase did not preserve the existing trade-cancellation point and detached planning")
	}
	for _, player := range []*Player{p, opponent} {
		packet, ok := player.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpCloseTrade || player.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("eligible trade cancellation did not publish only its existing close packet before commit")
		}
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	assertHellStorePublication(t, p)
}

func TestCriticalHellStoreUnknownOutcomeRetainsIdentityWithoutFinalOverwrite(t *testing.T) {
	for _, reason := range []string{"error", "panic", "identity"} {
		t.Run(reason, func(t *testing.T) {
			w, p, fallback, merchant, backend, queue, _, release := criticalHellStoreFixture(t)
			if reason == "error" {
				backend.failure = errors.New("commit acknowledgement lost")
			}
			backend.panicAfterWrite = reason == "panic"
			before := accountStateSnapshot(p.Account)
			w.onHellStoreBuy(p.Session, hellStorePacket(p.ID, merchant.ID, hellStoreAmulet, 7))
			awaitCriticalWrite(t, backend.entered)
			w.handle(command{s: p.Session})
			if reason == "identity" {
				p.criticalLifecycle = &criticalPlayerLifecycle{}
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			w.onHellStoreBuy(p.Session, hellStorePacket(p.ID, merchant.ID, hellStoreAmulet, 7))
			w.resumeCriticalDisconnects(8)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 ||
				!w.shuttingDown || !p.PersistencePoisoned || !w.criticalPlayerReserved(p) || len(queue.pending) != 1 ||
				len(backend.entered) != 0 || w.players[p.Session] != p || p.Session.QueuedPacketsForTest() != 0 ||
				w.claimAccountSession(gameNet.NewTestSession(3, 8), p.Account.Name) {
				t.Fatal("unknown purchase granted, charged, confirmed, replayed or released ownership")
			}
			done := make(chan error, 1)
			w.runShutdown(done)
			if err := <-done; err == nil || fallback.saves != 0 {
				t.Fatal("terminal snapshot overwrote an uncertain purchase")
			}
		})
	}
}

func TestCriticalHellStoreDisconnectFinalSnapshotIncludesCommittedItemIdentityAndGold(t *testing.T) {
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	queue := newCriticalTestQueue(t, &criticalStateTestStore{&criticalBatchTestStore{base}}, 1, 2)
	t.Cleanup(release)
	w, p, fallback, merchant := hellStoreWorld(t)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("fixture account admission failed")
	}
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	w.onHellStoreBuy(p.Session, hellStorePacket(p.ID, merchant.ID, hellStoreAmulet, 7))
	proposal := awaitCriticalWrite(t, base.entered)
	item, gold := proposal.accounts[0].Chars[p.CharSlot].Inv[7], proposal.accounts[0].Chars[p.CharSlot].Gold
	w.handle(command{s: p.Session})
	if p.Char.Inv[7].Index != 0 || p.Char.Gold != 20_000_000 || len(base.entered) != 0 {
		t.Fatal("disconnect published proposed state or submitted stale final inventory")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	final := awaitCriticalWrite(t, base.entered)
	if p.Char.Inv[7] != item || p.Char.Gold != gold || final.accounts[0].Chars[p.CharSlot].Inv[7] != item ||
		final.accounts[0].Chars[p.CharSlot].Gold != gold || final.character == nil ||
		final.character.uid != p.Char.UID || !reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) ||
		p.Session.QueuedPacketsForTest() != 0 || fallback.saves != 0 ||
		w.claimAccountSession(gameNet.NewTestSession(3, 8), p.Account.Name) {
		t.Fatal("final atomic snapshot lost the committed purchase or released readmission early")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if w.players[p.Session] != nil || len(queue.pending) != 0 || len(base.entered) != 0 || fallback.saves != 0 ||
		p.Session.QueuedPacketsForTest() != 0 || !w.claimAccountSession(gameNet.NewTestSession(4, 8), p.Account.Name) {
		t.Fatal("final acknowledgement did not release ownership without replay or closed-transport publication")
	}
}

func TestCriticalHellStoreActualDispatchKeepsWorldResponsiveDuringCommit(t *testing.T) {
	items, volatiles := hellStoreCatalog(t)
	w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, nil,
		func(w *World, p *Player) {
			w.items, w.volatiles = items, volatiles
			w.mobsByID[1102].Def.Name = hellStoreNPCName
			p.Char.Gold = 20_000_000
		})
	request := hellStorePacket(p.ID, 1102, hellStoreAmulet, 7)
	select {
	case w.commands <- command{s: p.Session, pkt: request}:
	case <-time.After(time.Second):
		t.Fatal("World did not accept the Hell Gate purchase")
	}
	proposal := awaitCriticalWrite(t, backend.entered)
	item := proposal.accounts[0].Chars[0].Inv[7]
	for range 3 {
		deliverCriticalLoopTick(t, ticks)
		overview := readCriticalLoopOverview(t, w)
		regenerated := false
		for _, player := range overview.Players {
			if player.UID == unrelatedUID && player.HP > 500 {
				regenerated = true
			}
		}
		if overview.State != "running" || !regenerated || p.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("held purchase blocked unrelated ticks/commands or confirmed before commit")
		}
	}
	release()
	deadline := time.Now().Add(2 * time.Second)
	for p.Session.QueuedPacketsForTest() < 2 && time.Now().Before(deadline) {
		readCriticalLoopOverview(t, w)
	}
	stop()
	// Inspect mutable state only after joining the World actor.
	if p.Char.Inv[7] != item || item.UID == "" || p.Char.Gold != 15_000_000 || len(queue.pending) != 0 ||
		w.criticalPlayerReserved(p) || w.mobTickCounter < 3 || len(backend.entered) != 0 || w.shuttingDown {
		t.Fatal("actual loop omitted the committed item/gold, repeated the purchase or failed to advance ticks")
	}
	assertHellStorePublication(t, p)
}

type criticalPurchaseTerminalStore struct {
	*craftStore
	account *model.Account
	flushes int
}

func (s *criticalPurchaseTerminalStore) SaveAccount(account *model.Account) error {
	s.account = accountStateSnapshot(account)
	return s.craftStore.SaveAccount(account)
}

func (s *criticalPurchaseTerminalStore) Flush() { s.flushes++ }

func TestCriticalHellStoreShutdownPersistsCommittedItemAndGoldAfterDrain(t *testing.T) {
	w, p, fallback, merchant, backend, queue, _, release := criticalHellStoreFixture(t)
	terminal := &criticalPurchaseTerminalStore{craftStore: fallback}
	w.store = terminal
	w.onHellStoreBuy(p.Session, hellStorePacket(p.ID, merchant.ID, hellStoreWeapon, 7))
	proposal := awaitCriticalWrite(t, backend.entered)
	item, gold := proposal.accounts[0].Chars[p.CharSlot].Inv[7], proposal.accounts[0].Chars[p.CharSlot].Gold
	first, second := make(chan error, 1), make(chan error, 1)
	w.runShutdown(first)
	w.runShutdown(second)
	w.handle(command{s: p.Session})
	if terminal.account != nil || fallback.saves != 0 || terminal.flushes != 0 || !queue.stopping ||
		w.shutdownFinished || p.Char.Inv[7].Index != 0 || p.Char.Gold != 20_000_000 || !p.Session.IsClosed() {
		t.Fatal("shutdown persisted proposed or stale inventory before purchase acknowledgement")
	}
	for _, done := range []chan error{first, second} {
		select {
		case <-done:
			t.Fatal("shutdown completed while the accepted purchase was still held")
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
			t.Fatal("shutdown did not finish after the accepted purchase settled")
		}
	}
	if terminal.account == nil || !reflect.DeepEqual(accountPersistenceSnapshot(terminal.account), proposal.accounts[0]) ||
		terminal.account.Chars[p.CharSlot].Inv[7] != item || terminal.account.Chars[p.CharSlot].Gold != gold ||
		p.Char.Inv[7] != item || p.Char.Gold != gold || fallback.saves != 1 || terminal.flushes != 1 ||
		!w.shutdownFinished || len(queue.pending) != 0 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("terminal snapshot lost committed item identity/gold, saved twice or published to a closed session")
	}
	third := make(chan error, 1)
	w.runShutdown(third)
	if err := <-third; err != nil || fallback.saves != 1 || terminal.flushes != 1 {
		t.Fatal("repeated shutdown replayed the terminal purchase snapshot")
	}
}

func TestCriticalHellStoreActualDispatchUnknownOutcomeEntersMaintenance(t *testing.T) {
	items, volatiles := hellStoreCatalog(t)
	w, p, _, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, errors.New("commit acknowledgement lost"),
		func(w *World, p *Player) {
			w.items, w.volatiles = items, volatiles
			w.mobsByID[1102].Def.Name = hellStoreNPCName
			p.Char.Gold = 20_000_000
		})
	request := hellStorePacket(p.ID, 1102, hellStoreAmulet, 7)
	select {
	case w.commands <- command{s: p.Session, pkt: request}:
	case <-time.After(time.Second):
		t.Fatal("World did not accept the purchase intention")
	}
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
		t.Fatal("actual loop did not contain the uncertain purchase outcome")
	}
	select {
	case w.commands <- command{s: p.Session, pkt: request}:
	case <-time.After(time.Second):
		t.Fatal("World did not accept the rejected-after-maintenance command")
	}
	deliverCriticalLoopTick(t, ticks)
	readCriticalLoopOverview(t, w)
	stop()
	if p.Char.Inv[7].Index != 0 || p.Char.Gold != 20_000_000 || p.Session.QueuedPacketsForTest() != 0 ||
		!p.PersistencePoisoned || !p.Session.IsClosed() || len(queue.pending) != 1 || len(backend.entered) != 0 ||
		!errors.Is(w.shutdownErr, errCriticalPersistenceIndeterminate) || w.mobTickCounter != 0 {
		t.Fatal("unknown purchase granted, charged, published, replayed, released ownership or advanced gameplay")
	}
}
