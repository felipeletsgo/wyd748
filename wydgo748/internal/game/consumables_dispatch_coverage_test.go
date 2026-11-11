package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
)

func TestOnUseItemIgnoresUninitializedParticipants(t *testing.T) {
	w, p, st := useItemWorld(model.VolatileRule{Action: "restore", Consume: true, HP: 100})
	pkt := useItemPacket(0, 0)
	char := p.Char

	delete(w.players, p.Session)
	w.onUseItem(p.Session, pkt)
	w.players[p.Session] = p
	p.Char = nil
	w.onUseItem(p.Session, pkt)
	p.Char = char
	p.InWorld = false
	w.onUseItem(p.Session, pkt)

	if st.saves != 0 || p.Session.QueuedPacketsForTest() != 0 || p.Char.Inv[0].Index != 100 {
		t.Fatal("an absent, characterless, or out-of-world participant changed state")
	}
}

func TestOnUseItemRoutesCriticalBasicActionThroughPersistence(t *testing.T) {
	w, p, backend, queue, release := criticalCelestialQueueFixture(t, 399)
	const itemID = 4310
	rule := model.VolatileRule{Action: "save_position", Consume: true}
	w.items = map[uint16]model.ItemDef{itemID: {Index: itemID, Name: "Position Scroll"}}
	w.volatiles = model.VolatileCatalog{
		Items:     map[uint16]model.VolatileRule{itemID: rule},
		ItemCodes: map[uint16]int{itemID: 187},
	}
	p.Char.Inv[0] = model.Item{Index: itemID, UID: "position-scroll"}
	oldX, oldY := p.Char.SavedX, p.Char.SavedY

	w.onUseItem(p.Session, useItemPacket(0, 0))
	proposal := awaitCriticalWrite(t, backend.entered)
	proposed := proposal.accounts[0].Chars[p.CharSlot]
	if proposed.SavedX != p.X || proposed.SavedY != p.Y || proposed.Inv[0].Index != 0 ||
		p.Char.SavedX != oldX || p.Char.SavedY != oldY || p.Char.Inv[0].Index != itemID ||
		p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("critical item dispatch did not hold the saved position and item until persistence")
	}

	settleCriticalCelestialWrite(t, w, queue, release)
	if p.Char.SavedX != p.X || p.Char.SavedY != p.Y || p.Char.Inv[0].Index != 0 || len(queue.pending) != 0 {
		t.Fatal("committed position item did not publish its saved position and consumption")
	}
}

func TestHuntingTeleportValidatesWarpAndRollsBackFailedSave(t *testing.T) {
	rule := model.VolatileRule{Action: "hunting_teleport", Consume: true,
		Destinations: make([]model.VolatileDestination, 10)}
	rule.Destinations[0] = model.VolatileDestination{X: 2101, Y: 2101}

	t.Run("invalid warp id", func(t *testing.T) {
		w, p, st := useItemWorld(rule)
		pkt := useItemPacket(0, 0)
		binary.LittleEndian.PutUint32(pkt[32:36], 0)
		w.onUseItem(p.Session, pkt)
		if p.Char.Inv[0].Index != 100 || st.saves != 0 {
			t.Fatal("invalid hunting warp consumed or persisted the scroll")
		}
	})

	t.Run("failed location save", func(t *testing.T) {
		w, p, st := useItemWorld(rule)
		st.err = errors.New("database unavailable")
		oldX, oldY := p.X, p.Y
		pkt := useItemPacket(0, 0)
		binary.LittleEndian.PutUint32(pkt[32:36], 1)
		w.onUseItem(p.Session, pkt)
		if p.Char.Inv[0].Index != 100 || p.X != oldX || p.Y != oldY || st.saves != 1 {
			t.Fatalf("failed teleport was not rolled back: item=%d pos=(%d,%d) saves=%d",
				p.Char.Inv[0].Index, p.X, p.Y, st.saves)
		}
	})

	t.Run("successful teleport", func(t *testing.T) {
		w, p, st := useItemWorld(rule)
		pkt := useItemPacket(0, 0)
		binary.LittleEndian.PutUint32(pkt[32:36], 1)
		w.onUseItem(p.Session, pkt)
		if p.Char.Inv[0].Index != 0 || st.saves != 1 || p.X < 2098 || p.X > 2104 || p.Y < 2098 || p.Y > 2104 {
			t.Fatalf("valid teleport did not commit near its destination: item=%d pos=(%d,%d) saves=%d",
				p.Char.Inv[0].Index, p.X, p.Y, st.saves)
		}
	})
}

func TestOnUseItemRejectsInvalidSpecialSkillAndMissingSummon(t *testing.T) {
	t.Run("special skill bit outside the item contract", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "learn_special_skill", Consume: true, LearnedBit: 24,
		})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || p.Char.LearnedSkill != 0 || st.saves != 0 {
			t.Fatal("invalid special skill book changed character state")
		}
	})

	t.Run("summon definition missing", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "summon_contract", Consume: true})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || st.saves != 0 {
			t.Fatal("contract without a summon definition consumed or saved")
		}
	})
}

func TestFireworkSaveFailurePreservesTheItem(t *testing.T) {
	w, p, st := useItemWorld(model.VolatileRule{Action: "firework", Consume: true})
	st.err = errors.New("database unavailable")
	w.onUseItem(p.Session, useItemPacket(0, 0))
	if p.Char.Inv[0].Index != 100 || st.saves != 1 {
		t.Fatalf("failed firework save changed inventory: item=%d saves=%d", p.Char.Inv[0].Index, st.saves)
	}
}

func TestMagicalPillCannotConsumeASecondTime(t *testing.T) {
	w, p, st := useItemWorld(model.VolatileRule{Action: "magical_pill", Consume: true})
	w.onUseItem(p.Session, useItemPacket(0, 0))
	if !p.Char.MagicalPillUsed || p.Char.Inv[0].Index != 0 || st.saves != 1 {
		t.Fatal("first magical pill was not committed")
	}
	p.Char.Inv[1] = model.Item{Index: 100}
	w.onUseItem(p.Session, useItemPacket(1, 1))
	if p.Char.Inv[1].Index != 100 || st.saves != 1 {
		t.Fatal("already-used magical pill consumed a second item")
	}
}
