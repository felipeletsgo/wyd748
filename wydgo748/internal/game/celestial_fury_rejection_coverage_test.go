package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestFuryStoneRejectsInvalidCharacterAndIngredientsWithoutMutation(t *testing.T) {
	tests := []struct {
		name      string
		itemIndex uint16
		evolution string
		level     uint32
		fame      uint32
		level90   bool
		arcana    bool
		cythera   uint16
	}{
		{name: "wrong item", itemIndex: 411, evolution: "celestial", level: 89, fame: 500},
		{name: "wrong evolution", itemIndex: furyStoneItem, evolution: "arch", level: 89, fame: 500},
		{name: "insufficient fame", itemIndex: furyStoneItem, evolution: "celestial", level: 89, fame: 499},
		{name: "celestial level too low", itemIndex: furyStoneItem, evolution: "celestial", level: 88, fame: 500},
		{name: "level 90 already unlocked", itemIndex: furyStoneItem, evolution: "celestial", level: 89, fame: 500, level90: true},
		{name: "arcana already active", itemIndex: furyStoneItem, evolution: "celestial", level: maxCelestialLevel, fame: 500, arcana: true, cythera: 3501},
		{name: "invalid base cythera", itemIndex: furyStoneItem, evolution: "celestial", level: maxCelestialLevel, fame: 500, cythera: 9999},
		{name: "missing secret stones", itemIndex: furyStoneItem, evolution: "celestial", level: maxCelestialLevel, fame: 500, cythera: 3501},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			player, _ := networkedTestPlayer(1, "Fury", 2100, 2100)
			w := worldWithNetworkedPlayers(player)
			st := &craftStore{}
			w.store = st
			player.Char.Evolution, player.Char.Score.Level = tc.evolution, tc.level
			player.Char.CelestialLevel40Unlocked = true
			player.Char.CelestialLevel90Unlocked = tc.level90
			player.Char.CelestialArcana = tc.arcana
			player.Char.Equip[cytheraEquipSlot] = model.Item{Index: tc.cythera}
			player.SpecialCoins = map[string]uint32{fameCounter: tc.fame}
			player.Char.Inv[0] = model.Item{Index: tc.itemIndex}
			beforeInventory, beforeEquip := player.Char.Inv, player.Char.Equip
			beforeLevel90, beforeArcana := player.Char.CelestialLevel90Unlocked, player.Char.CelestialArcana

			w.useFuryStone(player.Session, player, &player.Char.Inv[0], 0)

			if player.Char.Inv != beforeInventory || player.Char.Equip != beforeEquip ||
				player.Char.CelestialLevel90Unlocked != beforeLevel90 || player.Char.CelestialArcana != beforeArcana ||
				counterBalance(player, fameCounter) != tc.fame || st.saves != 0 {
				t.Fatalf("rejected Fury use changed state: inv=%+v equip=%+v level90=%t arcana=%t fame=%d saves=%d",
					player.Char.Inv[0], player.Char.Equip[cytheraEquipSlot], player.Char.CelestialLevel90Unlocked,
					player.Char.CelestialArcana, counterBalance(player, fameCounter), st.saves)
			}
		})
	}
}
