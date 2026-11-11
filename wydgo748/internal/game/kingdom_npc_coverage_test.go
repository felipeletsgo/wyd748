package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestHandleKingdomNPCRejectsIncompleteInteractions(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	king := &Mob{Def: &model.NPCDef{Name: "King_Glantuar"}}
	for _, tc := range []struct {
		name   string
		player *Player
		mob    *Mob
	}{
		{"nil player", nil, king},
		{"nil character", &Player{}, king},
		{"nil mob", p, nil},
		{"nil definition", p, &Mob{}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if w.handleKingdomNPC(p.Session, tc.player, tc.mob) {
				t.Fatal("incomplete NPC interaction was claimed")
			}
		})
	}
}

func TestKingNPCRoutesIdealStoneCraftBeforeKingdomJoin(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Evolution = archEvolution
	p.Char.Equip[10] = model.Item{Index: 1742}
	p.Char.Equip[sefirotSlot] = model.Item{Index: 1761}
	for i, stone := 0, uint16(5334); stone <= 5337; i, stone = i+1, stone+1 {
		p.Char.Inv[i] = model.Item{Index: stone}
	}
	king := &Mob{Def: &model.NPCDef{Name: "King_Harabard"}}
	if !w.handleKingdomNPC(p.Session, p, king) {
		t.Fatal("king did not claim the Ideal Stone recipe")
	}
	if st.saves != 1 || characterKingdom(p.Char) != model.KingdomNeutral ||
		p.Char.Equip[10].Index != 0 || p.Char.Equip[sefirotSlot].Index != 0 {
		t.Fatalf("king interaction joined a kingdom or failed to craft: saves=%d cape=%d equipment=%v",
			st.saves, characterKingdom(p.Char), p.Char.Equip[10])
	}
	for _, item := range p.Char.Inv {
		if item.Index == idealStoneItem {
			return
		}
	}
	t.Fatal("king interaction did not deliver the Ideal Stone")
}

func TestKingdomBrokerNPCRoutesLeaveUsingWorldClock(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	w.clock = newFakeClock(time.Date(2026, time.July, 27, 12, 0, 0, 0, time.Local))
	p.Char.Equip[model.CapeSlot] = model.Item{Index: 545}
	p.Char.Inv[0] = model.Item{Index: model.SapphireItem, Eff: [6]byte{effectAmount, 16}}
	broker := &Mob{Def: &model.NPCDef{Name: "Kingdom_Brocker"}}
	if !w.handleKingdomNPC(p.Session, p, broker) {
		t.Fatal("kingdom broker interaction was not claimed")
	}
	if st.saves != 1 || characterKingdom(p.Char) != model.KingdomNeutral || sapphireCount(p.Char) != 0 {
		t.Fatalf("broker leave was not committed: saves=%d kingdom=%d sapphires=%d",
			st.saves, characterKingdom(p.Char), sapphireCount(p.Char))
	}
}
