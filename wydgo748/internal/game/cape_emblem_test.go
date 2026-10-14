package game

import (
	"bytes"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type capeEmblemRNG struct{ value, calls int }

func (r *capeEmblemRNG) Intn(n int) int {
	if n != model.CapeEmblemCount {
		panic("unexpected cape draw bound")
	}
	r.calls++
	return r.value
}

func TestCapeEmblemDrawRangeAndNoReroll(t *testing.T) {
	for value := 0; value < model.CapeEmblemCount; value++ {
		rng := &capeEmblemRNG{value: value}
		w := &World{rng: rng}
		ch := &model.Char{Citizenship: 1}
		if w.ensureCapeEmblem(ch) || rng.calls != 0 {
			t.Fatal("cape-less character drew an emblem")
		}
		ch.Equip[15].Index = 545
		if !w.ensureCapeEmblem(ch) || ch.CapeEmblem != byte(value+1) || ch.Citizenship != 1 {
			t.Fatal("first cape did not draw a bounded independent symbol")
		}
		for _, cape := range []uint16{543, 3191, 3197, 548, 546} {
			ch.Equip[15].Index = cape
			if w.ensureCapeEmblem(ch) || ch.CapeEmblem != byte(value+1) {
				t.Fatal("conversion rerolled the symbol")
			}
		}
		if rng.calls != 1 {
			t.Fatal("cosmetic draw was repeated")
		}
	}
}

func TestCapeEmblemKingdomCriticalCommitAndRejection(t *testing.T) {
	for _, rejected := range []bool{false, true} {
		t.Run(map[bool]string{false: "commit", true: "rejected"}[rejected], func(t *testing.T) {
			w := newZoneTestWorld()
			p := addZonePlayer(w, 1, 2100, 2100, 500)
			p.Char.Score.Level = 219
			p.Char.Citizenship = 1
			p.Char.Equip[13] = model.Item{Index: model.KingdomEmblem}
			observer := addZonePlayer(w, 2, 2101, 2100, 500)
			observer.show(p.ID)
			rng := &capeEmblemRNG{value: 8}
			w.rng = rng
			claimCriticalTestPlayers(t, w, p)
			gate, release := criticalGate(t)
			backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			if rejected {
				backend.failure = criticalCertifiedRejection(t)
			}
			queue := newCriticalTestQueue(t, &criticalStateTestStore{&criticalBatchTestStore{backend}}, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			w.joinKingdom(p.Session, p, model.KingdomHekalotia)
			write := awaitCriticalWrite(t, backend.entered)
			proposed := write.accounts[0].Chars[0]
			if proposed.CapeEmblem != 9 || proposed.Equip[15].Index != 545 || proposed.Equip[13].Index != 0 || proposed.Citizenship != 1 {
				t.Fatal("payment, cape and random emblem were not one proposal")
			}
			w.joinKingdom(p.Session, p, model.KingdomHekalotia)
			if rng.calls != 1 || p.Char.CapeEmblem != 0 || p.Char.Equip[15].Index != 0 ||
				p.Session.QueuedPacketsForTest() != 0 || observer.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("pending/repeated grant published, rerolled or mutated live state")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			if p.Char.CapeEmblem != 0 {
				t.Fatal("worker installed the draw before actor acknowledgement")
			}
			w.processCriticalCompletions(1)
			if rejected {
				if p.Char.CapeEmblem != 0 || p.Char.Equip[15].Index != 0 || observer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("rejected draw escaped containment")
				}
				return
			}
			if p.Char.CapeEmblem != 9 || p.Char.Citizenship != 1 {
				t.Fatal("commit lost symbol or changed the real channel")
			}
			ownerFace, ownerSnapshot := false, false
			for {
				packet, ok := p.Session.DequeuePacketForTest()
				if !ok {
					break
				}
				if !wire.Decrypt(packet) {
					t.Fatal("invalid owner frame")
				}
				h := wire.ParseHeader(packet)
				if h.Type == wire.OpSendItem {
					expected := wire.SendItem(p.ID, placeEquip, 0, wire.CharacterFaceItem(*p.Char))
					ownerFace = ownerFace || bytes.Equal(packet[12:], expected[12:])
				}
				if h.Type == wire.OpCreateMob {
					ownerSnapshot = packet[326] == 9
				}
			}
			observerSnapshot := false
			for {
				packet, ok := observer.Session.DequeuePacketForTest()
				if !ok {
					break
				}
				if !wire.Decrypt(packet) {
					t.Fatal("invalid observer frame")
				}
				if wire.ParseHeader(packet).Type == wire.OpCreateMob {
					observerSnapshot = packet[326] == 9
				}
			}
			if !ownerFace || !ownerSnapshot || !observerSnapshot {
				t.Fatal("owner and observer did not receive the committed cosmetic variant")
			}
			if clientEquipProjection(p.Char)[0].Eff[5] != 9 {
				t.Fatal("SelfEquip would erase the cosmetic variant")
			}
		})
	}
}

func TestCapeEmblemEntryMigrationCommitsBeforePublication(t *testing.T) {
	for _, rejected := range []bool{false, true} {
		t.Run(map[bool]string{false: "commit", true: "rejected"}[rejected], func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalEntryFixture(t)
			p.Account.Chars[0].Equip[15] = model.Item{Index: 545, UID: "legacy-cape"}
			p.Account.Chars[0].Citizenship = 1
			w.items[545] = model.ItemDef{Index: 545}
			rng := &capeEmblemRNG{value: 9}
			w.rng = rng
			if rejected {
				backend.failure = criticalCertifiedRejection(t)
			}
			before := accountStateSnapshot(p.Account)
			w.onEnterWorld(p.Session, enterWorldPacket(0))
			write := awaitCriticalWrite(t, backend.entered)
			if write.accounts[0].Chars[0].CapeEmblem != 10 {
				t.Fatal("entry migration did not persist its draw")
			}
			assertCriticalEntryPending(t, w, p, before, fallback)
			w.onEnterWorld(p.Session, enterWorldPacket(0))
			if rng.calls != 1 {
				t.Fatal("repeated entry rerolled a pending migration")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if rejected {
				if p.InWorld || p.Account.Chars[0].CapeEmblem != 0 {
					t.Fatal("rejected migration entered the world")
				}
				return
			}
			if !p.InWorld || p.Char.CapeEmblem != 10 || p.Char.Citizenship != 1 {
				t.Fatal("migration was not installed before entry")
			}
			assertCriticalEntryPackets(t, p)
			if rng.calls != 1 {
				t.Fatal("acknowledgement rerolled the migration")
			}
		})
	}
}

func TestCapeEmblemLegacyGrantRollbackAndPromotion(t *testing.T) {
	w, p, store := handlerTestWorld(t)
	p.Char.Score.Level = 255
	p.Char.Citizenship = 1
	p.Char.Inv[0] = model.Item{Index: model.SapphireItem, Eff: [6]byte{effectAmount, 25}}
	applyScore(p.Char)
	rng := &capeEmblemRNG{value: 4}
	w.rng = rng
	store.err = criticalCertifiedRejection(t)
	w.joinKingdom(p.Session, p, model.KingdomHekalotia)
	if p.Char.CapeEmblem != 0 || p.Char.Equip[15].Index != 0 || sapphireCount(p.Char) != 25 {
		t.Fatal("failed grant did not roll back")
	}
	store.err = nil
	w.joinKingdom(p.Session, p, model.KingdomHekalotia)
	symbol, calls := p.Char.CapeEmblem, rng.calls
	w.joinKingdom(p.Session, p, model.KingdomHekalotia)
	w.leaveKingdom(p.Session, p, time.Date(2026, 7, 27, 12, 0, 0, 0, time.UTC))
	if symbol != 5 || p.Char.CapeEmblem != symbol || p.Char.Citizenship != 1 || rng.calls != calls {
		t.Fatal("promotion/broker rerolled or changed citizenship")
	}
}
