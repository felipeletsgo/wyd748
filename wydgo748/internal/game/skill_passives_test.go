package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
)

func TestPassiveMerchantSalePrice(t *testing.T) {
	for _, tc := range []struct {
		name        string
		class       uint8
		learned     uint32
		item        uint16
		mastery     uint32
		price, want uint64
	}{
		{"minimum", 3, 1 << 9, 400, 0, 2000, 2120},
		{"mastery", 3, 1 << 9, 400, 100, 2000, 2320},
		{"rounding", 3, 1 << 9, 400, 99, 1001, 1141},
		{"cap", 3, 1 << 9, 400, 320, 2000, 2520},
		{"threshold", 3, 1 << 9, 400, 100, 1000, 1000},
		{"other_class", 0, 1 << 9, 400, 100, 2000, 2000},
		{"not_learned", 3, 0, 400, 100, 2000, 2000},
		{"adjacent_bit", 3, 1 << 10, 400, 100, 2000, 2000},
		{"excluded_693", 3, 1 << 9, 693, 100, 2000, 2000},
		{"excluded_694", 3, 1 << 9, 694, 100, 2000, 2000},
		{"excluded_695", 3, 1 << 9, 695, 100, 2000, 2000},
	} {
		t.Run(tc.name, func(t *testing.T) {
			ch := &model.Char{Class: tc.class, LearnedSkill: tc.learned,
				Score:        testScore(model.Score{Mastery: [4]uint32{0, 0, 255, 0}}),
				RuntimeScore: testScore(model.Score{Mastery: [4]uint32{0, 0, tc.mastery, 0}})}
			if got := passiveMerchantSalePrice(ch, tc.item, tc.price); got != tc.want {
				t.Fatalf("price=%d, want %d", got, tc.want)
			}
		})
	}
	if got := passiveMerchantSalePrice(nil, 400, 2000); got != 2000 {
		t.Fatalf("nil character changed price: %d", got)
	}
}

func TestPassiveMerchantSaleLifecycle(t *testing.T) {
	for _, mode := range []string{"success_and_replay", "save_failure", "gold_cap", "no_shop", "distant_shop"} {
		t.Run(mode, func(t *testing.T) {
			w, st, p, shop := cityEconomyFixture(t, model.ItemDef{Index: 400, Price: 8000})
			p.Char.Class, p.Char.LearnedSkill = 3, 1<<9
			p.Char.RuntimeScore = testScore(*p.Char.Score)
			p.Char.RuntimeScore.Mastery[2] = 100
			p.Char.Gold = 1000
			p.Char.Inv[0] = model.Item{Index: 400, UID: "11111111111141118111111111110400"}
			switch mode {
			case "save_failure":
				st.err = errors.New("unavailable")
			case "gold_cap":
				// Fits the old price (1800 after tax), not the passive price (2088).
				p.Char.Gold = maxCharacterGold - 2000
			case "no_shop":
				p.ShopNPC = 0
			case "distant_shop":
				p.X, p.Y = shop.X+100, shop.Y+100
			}
			beforeItem, beforeGold := p.Char.Inv[0], p.Char.Gold
			sell := make([]byte, 20)
			binary.LittleEndian.PutUint16(sell[12:14], shop.ID)
			sell[14] = placeInv
			w.onSellItem(p.Session, sell)
			if mode == "success_and_replay" {
				if p.Char.Inv[0].Index != 0 || p.Char.Gold != 3088 ||
					w.guilds.Wars.Cities.Territories[0].Treasury != 58 || st.gameSaves != 1 {
					t.Fatalf("sale: item=%v gold=%d treasury=%d saves=%d", p.Char.Inv[0], p.Char.Gold,
						w.guilds.Wars.Cities.Territories[0].Treasury, st.gameSaves)
				}
				w.onSellItem(p.Session, sell)
				if p.Char.Gold != 3088 || st.gameSaves != 1 || w.guilds.Wars.Cities.Territories[0].Treasury != 58 {
					t.Fatal("replayed sale credited twice")
				}
			} else if p.Char.Inv[0] != beforeItem || p.Char.Gold != beforeGold ||
				w.guilds.Wars.Cities.Territories[0].Treasury != 0 {
				t.Fatalf("rejected sale changed state: item=%v gold=%d", p.Char.Inv[0], p.Char.Gold)
			}
		})
	}
}
