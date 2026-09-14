package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func drainPlayerPackets(p *Player) {
	if p == nil || p.Session == nil {
		return
	}
	for {
		if _, ok := p.Session.DequeuePacketForTest(); !ok {
			return
		}
	}
}

func cityWarChallengeFixture(t *testing.T) (*World, *guildFlowStore, *Player, *Mob, *fakeClock) {
	t.Helper()
	w, st, _, challenger, clock := scheduledWarWorld(t, "2026-09-12 12:00")
	w.guilds.FindByID(2).Fame = 200
	w.guilds.Wars.Cities.Territories[0].Owner = 1
	challenger.X, challenger.Y = 2107, 2143
	challenger.Char.X, challenger.Char.Y = challenger.X, challenger.Y
	w.updatePlayerSpatial(challenger)
	collector := &Mob{
		ID: 3100, X: 2107, Y: 2144,
		Def: &model.NPCDef{
			Name: "Balmus", Tipo: model.TipoNPC,
			Score: &model.Score{Merchant: 6, Level: 0},
		},
	}
	w.registerMobSpatial(collector)
	challenger.show(collector.ID)
	w.tickGuildWars(clock.Now())
	drainPlayerPackets(challenger)
	return w, st, challenger, collector, clock
}

func challengePacket(opcode, playerID, npcID uint16, option uint32) []byte {
	if opcode == wire.OpChallengeConfirm {
		return wire.StandardParm2(opcode, playerID, uint32(npcID), option)
	}
	return wire.StandardParm(opcode, playerID, uint32(npcID))
}

func TestCityWarNativeChallengePromptConfirmAndReplay(t *testing.T) {
	w, _, p, collector, _ := cityWarChallengeFixture(t)

	w.onGuildChallenge(p.Session, challengePacket(wire.OpChallenge, p.ID, collector.ID, 0))
	pkt, ok := p.Session.DequeuePacketForTest()
	if !ok {
		t.Fatal("0x28E did not produce native 0x18D prompt")
	}
	if !wire.Decrypt(pkt) {
		t.Fatal("could not decrypt native city-war prompt")
	}
	header := wire.ParseHeader(pkt)
	if len(pkt) != wire.HeaderSize || header.Size != wire.HeaderSize || header.Type != wire.OpReqChallenge || header.ID != wire.SceneField {
		t.Fatalf("unexpected city-war prompt: len=%d header=%+v", len(pkt), header)
	}
	if p.CityWarNPC != collector.ID || p.CityWarCity != 0 || p.CityWarUntil.IsZero() {
		t.Fatalf("authoritative challenge context not established: npc=%d city=%d until=%v", p.CityWarNPC, p.CityWarCity, p.CityWarUntil)
	}

	w.onGuildChallenge(p.Session, challengePacket(wire.OpChallengeConfirm, p.ID, collector.ID, 0))
	if got := w.guilds.FindByID(2).Fame; got != 100 {
		t.Fatalf("confirmation fee incorrect: fame=%d", got)
	}
	if got := len(w.guilds.Wars.Cities.Entries); got != 1 || w.guilds.Wars.Cities.Entries[0].Guild != 2 || w.guilds.Wars.Cities.Entries[0].City != 0 {
		t.Fatalf("registration not committed once: %+v", w.guilds.Wars.Cities.Entries)
	}
	if !p.CityWarUntil.IsZero() || p.CityWarNPC != 0 || p.CityWarGuild != 0 {
		t.Fatal("confirmation context was not consumed")
	}

	w.onGuildChallenge(p.Session, challengePacket(wire.OpChallengeConfirm, p.ID, collector.ID, 0))
	if w.guilds.FindByID(2).Fame != 100 || len(w.guilds.Wars.Cities.Entries) != 1 {
		t.Fatal("replayed 0x28F charged fame or registered twice")
	}
}

func TestCityWarChallengeContextExpiryOwnerChangeAndNPCSwitch(t *testing.T) {
	t.Run("ttl", func(t *testing.T) {
		w, _, p, collector, clock := cityWarChallengeFixture(t)
		w.onGuildChallenge(p.Session, challengePacket(wire.OpChallenge, p.ID, collector.ID, 0))
		drainPlayerPackets(p)
		clock.Advance(cityWarInteractionTTL + time.Second)
		w.onGuildChallenge(p.Session, challengePacket(wire.OpChallengeConfirm, p.ID, collector.ID, 0))
		if w.guilds.FindByID(2).Fame != 200 || len(w.guilds.Wars.Cities.Entries) != 0 || !p.CityWarUntil.IsZero() {
			t.Fatal("expired challenge context was accepted or retained")
		}
	})

	t.Run("owner changed", func(t *testing.T) {
		w, _, p, collector, _ := cityWarChallengeFixture(t)
		w.onGuildChallenge(p.Session, challengePacket(wire.OpChallenge, p.ID, collector.ID, 0))
		drainPlayerPackets(p)
		w.guilds.Wars.Cities.Territories[0].Owner = 0
		w.onGuildChallenge(p.Session, challengePacket(wire.OpChallengeConfirm, p.ID, collector.ID, 0))
		if w.guilds.FindByID(2).Fame != 200 || len(w.guilds.Wars.Cities.Entries) != 0 || !p.CityWarUntil.IsZero() {
			t.Fatal("stale owner context was accepted or retained")
		}
	})

	t.Run("another npc clears", func(t *testing.T) {
		w, _, p, collector, _ := cityWarChallengeFixture(t)
		w.onGuildChallenge(p.Session, challengePacket(wire.OpChallenge, p.ID, collector.ID, 0))
		drainPlayerPackets(p)
		other := &Mob{
			ID: 3101, X: p.X + 1, Y: p.Y,
			Def: &model.NPCDef{Name: "Other NPC", Tipo: model.TipoNPC, Score: &model.Score{}},
		}
		w.registerMobSpatial(other)
		p.show(other.ID)
		w.onUseNPC(p.Session, useNPCPacket(other.ID))
		if p.CityWarNPC != 0 || !p.CityWarUntil.IsZero() {
			t.Fatal("clicking another NPC preserved stale city-war context")
		}
	})
}

func TestCityWarRejectsInvalidCollectors(t *testing.T) {
	w, _, p, collector, _ := cityWarChallengeFixture(t)
	collector.Def.Score.Merchant = nativeShopMerchant
	w.onGuildChallenge(p.Session, challengePacket(wire.OpChallenge, p.ID, collector.ID, 0))
	if p.CityWarNPC != 0 || !p.CityWarUntil.IsZero() {
		t.Fatal("ordinary merchant accepted as city collector")
	}
	drainPlayerPackets(p)

	collector.Def.Score.Merchant = 6
	collector.X, collector.Y = 2200, 2200
	p.X, p.Y = 2200, 2199
	p.Char.X, p.Char.Y = p.X, p.Y
	w.registerMobSpatial(collector)
	w.updatePlayerSpatial(p)
	w.onGuildChallenge(p.Session, challengePacket(wire.OpChallenge, p.ID, collector.ID, 0))
	if p.CityWarNPC != 0 || !p.CityWarUntil.IsZero() {
		t.Fatal("collector outside a configured city accepted")
	}
}

func TestCityWarRejectedConfirmationConsumesContext(t *testing.T) {
	for _, rejection := range []string{"option", "target overflow", "missing npc", "wrong merchant", "out of reach", "wrong size", "truncated"} {
		t.Run(rejection, func(t *testing.T) {
			w, _, p, collector, _ := cityWarChallengeFixture(t)
			request := challengePacket(wire.OpChallenge, p.ID, collector.ID, 0)
			confirm := challengePacket(wire.OpChallengeConfirm, p.ID, collector.ID, 0)
			w.onGuildChallenge(p.Session, request)
			bad := append([]byte(nil), confirm...)
			switch rejection {
			case "option":
				binary.LittleEndian.PutUint32(bad[16:20], 1)
			case "target overflow":
				binary.LittleEndian.PutUint32(bad[12:16], 0x10000)
			case "missing npc":
				binary.LittleEndian.PutUint32(bad[12:16], 3201)
			case "wrong merchant":
				collector.Def.Score.Merchant = nativeShopMerchant
			case "out of reach":
				p.X += 100
			case "wrong size":
				bad = bad[:16]
			case "truncated":
				bad = bad[:8]
			}
			w.onGuildChallenge(p.Session, bad)
			if p.CityWarNPC != 0 || !p.CityWarUntil.IsZero() {
				t.Fatal("rejected confirmation retained the previous context")
			}
			collector.Def.Score.Merchant = 6
			p.X = p.Char.X
			w.onGuildChallenge(p.Session, confirm)
			if w.guilds.FindByID(2).Fame != 200 || len(w.guilds.Wars.Cities.Entries) != 0 {
				t.Fatal("confirmation after rejection reused the previous context")
			}
			w.onGuildChallenge(p.Session, request)
			w.onGuildChallenge(p.Session, confirm)
			if w.guilds.FindByID(2).Fame != 100 || len(w.guilds.Wars.Cities.Entries) != 1 {
				t.Fatal("a fresh collector request could not register after rejection")
			}
		})
	}
}

func cityEconomyFixture(t *testing.T, item model.ItemDef) (*World, *guildFlowStore, *Player, *Mob) {
	t.Helper()
	w, st, p, _ := guildWarTestWorld(t)
	w.guilds.Wars.Cities.Territories[0].Owner = 1
	w.items = map[uint16]model.ItemDef{item.Index: item}
	shop := &Mob{
		ID: 3200, X: 2101, Y: 2100,
		Def: &model.NPCDef{
			Name: "Armia Merchant", Tipo: model.TipoNPC,
			Score: &model.Score{Merchant: nativeShopMerchant},
			Vende: []model.Item{{Index: item.Index}},
		},
	}
	w.registerMobSpatial(shop)
	p.show(shop.ID)
	p.ShopNPC = shop.ID
	return w, st, p, shop
}

func TestCityTaxBuySellAndTOTOArithmetic(t *testing.T) {
	t.Run("buy", func(t *testing.T) {
		w, st, p, shop := cityEconomyFixture(t, model.ItemDef{Index: 400, Price: 1000})
		p.Char.Gold = 5000
		buy := make([]byte, 24)
		binary.LittleEndian.PutUint16(buy[12:14], shop.ID)
		binary.LittleEndian.PutUint16(buy[14:16], 0)
		w.onBuyItem(p.Session, buy)
		if p.Char.Inv[0].Index != 400 || p.Char.Gold != 3900 || w.guilds.Wars.Cities.Territories[0].Treasury != 25 || st.gameSaves != 1 {
			t.Fatalf("taxed buy mismatch: item=%d gold=%d treasury=%d saves=%d", p.Char.Inv[0].Index, p.Char.Gold, w.guilds.Wars.Cities.Territories[0].Treasury, st.gameSaves)
		}
	})

	t.Run("sell", func(t *testing.T) {
		w, st, p, _ := cityEconomyFixture(t, model.ItemDef{Index: 400, Price: 1000})
		p.Char.Gold = 1000
		p.Char.Inv[0] = model.Item{Index: 400, UID: "11111111111141118111111111110400"}
		sell := make([]byte, 20)
		sell[14], sell[16] = placeInv, 0
		w.onSellItem(p.Session, sell)
		if p.Char.Inv[0].Index != 0 || p.Char.Gold != 1225 || w.guilds.Wars.Cities.Territories[0].Treasury != 6 || st.gameSaves != 1 {
			t.Fatalf("taxed sell mismatch: item=%d gold=%d treasury=%d saves=%d", p.Char.Inv[0].Index, p.Char.Gold, w.guilds.Wars.Cities.Territories[0].Treasury, st.gameSaves)
		}
	})

	t.Run("toto", func(t *testing.T) {
		w, st, p, shop := cityEconomyFixture(t, model.ItemDef{Index: 4147, Price: 3000})
		p.Char.Gold = 10000
		w.onBuyToto(p.Session, totoBuyPacket(shop.ID, 0, 5, 1, 7, 2, 3))
		if p.Char.Inv[5].Index != 4147 || p.Char.Gold != 6700 || w.guilds.Wars.Cities.Territories[0].Treasury != 75 || st.gameSaves != 1 {
			t.Fatalf("taxed TOTO mismatch: item=%d gold=%d treasury=%d saves=%d", p.Char.Inv[5].Index, p.Char.Gold, w.guilds.Wars.Cities.Territories[0].Treasury, st.gameSaves)
		}
	})
}

func TestCityTaxPersistenceFailureRollsBackPlayerAndTreasury(t *testing.T) {
	w, st, p, shop := cityEconomyFixture(t, model.ItemDef{Index: 400, Price: 1000})
	p.Char.Gold = 1000
	p.Char.Inv[0] = model.Item{Index: 400, UID: "11111111111141118111111111110400"}
	st.err = errors.New("postgres unavailable")
	sell := make([]byte, 20)
	sell[14], sell[16] = placeInv, 0
	w.onSellItem(p.Session, sell)
	if p.Char.Inv[0].Index != 400 || p.Char.Gold != 1000 || w.guilds.Wars.Cities.Territories[0].Treasury != 0 {
		t.Fatalf("failed taxed sale did not roll back atomically: item=%d gold=%d treasury=%d", p.Char.Inv[0].Index, p.Char.Gold, w.guilds.Wars.Cities.Territories[0].Treasury)
	}
	if p.ShopNPC != shop.ID {
		t.Fatal("persistence rollback unexpectedly invalidated the open merchant")
	}
}

func TestCityTaxPurchaseFailureAndRetry(t *testing.T) {
	for _, ticket := range []bool{false, true} {
		name := "shop"
		if ticket {
			name = "toto"
		}
		t.Run(name, func(t *testing.T) {
			index := uint16(400)
			if ticket {
				index = 4147
			}
			w, st, p, shop := cityEconomyFixture(t, model.ItemDef{Index: index, Price: 3000})
			p.Char.Gold = 10000
			beforeInv := p.Char.Inv
			buy := func() {
				if ticket {
					w.onBuyToto(p.Session, totoBuyPacket(shop.ID, 0, 0, 1, 7, 2, 3))
					return
				}
				pkt := make([]byte, 24)
				binary.LittleEndian.PutUint16(pkt[12:14], shop.ID)
				w.onBuyItem(p.Session, pkt)
			}
			st.err = errors.New("tax commit unavailable")
			buy()
			if st.gameSaves != 1 || p.Char.Gold != 10000 || p.Char.Inv != beforeInv || w.guilds.Wars.Cities.Territories[0].Treasury != 0 {
				t.Fatal("failed taxed purchase did not restore the player and treasury")
			}
			st.err = nil
			w.guilds.Wars.Cities.Territories[0].Treasury = model.GuildWarTreasuryCap
			buy()
			if st.gameSaves != 1 || p.Char.Gold != 10000 || p.Char.Inv != beforeInv || w.guilds.Wars.Cities.Territories[0].Treasury != model.GuildWarTreasuryCap {
				t.Fatal("treasury overflow did not reject and roll back the purchase")
			}
			w.guilds.Wars.Cities.Territories[0].Treasury = 0
			buy()
			if st.gameSaves != 2 || p.Char.Gold != 6700 || p.Char.Inv[0].Index != index || p.Char.Inv[1] != beforeInv[1] || w.guilds.Wars.Cities.Territories[0].Treasury != 75 {
				t.Fatal("retry did not commit exactly one taxed purchase")
			}
		})
	}
}

func TestCityTreasuryRejectsUnauthorizedCollection(t *testing.T) {
	for _, rejection := range []string{"saturday", "other owner", "forged leader"} {
		t.Run(rejection, func(t *testing.T) {
			date := "2026-09-13 12:00"
			if rejection == "saturday" {
				date = "2026-09-12 12:00"
			}
			w, st, p, _, _ := scheduledWarWorld(t, date)
			w.guilds.Wars.Cities.Territories[0].Owner = 1
			w.guilds.Wars.Cities.Territories[0].Treasury = 100
			if rejection == "other owner" {
				w.guilds.Wars.Cities.Territories[0].Owner = 2
			}
			if rejection == "forged leader" {
				w.guilds.FindByID(1).Members[0].Rank = model.GuildRankMember
			}
			gold, inv := p.Char.Gold, p.Char.Inv
			if err := w.withdrawCityTreasury(p, 0); err == nil || p.Char.Gold != gold || p.Char.Inv != inv || w.guilds.Wars.Cities.Territories[0].Treasury != 100 || st.gameSaves != 0 {
				t.Fatal("unauthorized collection changed the player or treasury")
			}
		})
	}
}

func TestCityTreasuryGoldAndChequeBoundaries(t *testing.T) {
	t.Run("direct gold cap", func(t *testing.T) {
		w, _, p, _, _ := scheduledWarWorld(t, "2026-09-13 12:00")
		w.guilds.Wars.Cities.Territories[0].Owner = 1
		w.guilds.Wars.Cities.Territories[0].Treasury = 100
		p.Char.Gold = maxCharacterGold - 99
		if err := w.withdrawCityTreasury(p, 0); err == nil || p.Char.Gold != maxCharacterGold-99 || w.guilds.Wars.Cities.Territories[0].Treasury != 100 {
			t.Fatal("direct treasury withdrawal ignored max-gold boundary")
		}
		p.Char.Gold = maxCharacterGold - 100
		if err := w.withdrawCityTreasury(p, 0); err != nil || p.Char.Gold != maxCharacterGold || w.guilds.Wars.Cities.Territories[0].Treasury != 0 {
			t.Fatalf("direct treasury withdrawal failed at exact capacity: err=%v gold=%d treasury=%d", err, p.Char.Gold, w.guilds.Wars.Cities.Territories[0].Treasury)
		}
	})

	t.Run("full inventory", func(t *testing.T) {
		w, _, p, _, _ := scheduledWarWorld(t, "2026-09-13 12:00")
		w.guilds.Wars.Cities.Territories[0].Owner = 1
		w.guilds.Wars.Cities.Territories[0].Treasury = cityWarChequeValue
		for i := 0; i < model.PlayerCarrySlots; i++ {
			p.Char.Inv[i] = model.Item{Index: 400}
		}
		if err := w.withdrawCityTreasury(p, 0); err == nil || w.guilds.Wars.Cities.Territories[0].Treasury != cityWarChequeValue {
			t.Fatal("full inventory withdrew or lost treasury cheques")
		}
	})

	t.Run("partial cheques", func(t *testing.T) {
		w, _, p, _, _ := scheduledWarWorld(t, "2026-09-13 12:00")
		w.guilds.Wars.Cities.Territories[0].Owner = 1
		w.guilds.Wars.Cities.Territories[0].Treasury = 3_500_000_000
		for i := 0; i < model.PlayerCarrySlots-2; i++ {
			p.Char.Inv[i] = model.Item{Index: 400}
		}
		if err := w.withdrawCityTreasury(p, 0); err != nil {
			t.Fatal(err)
		}
		if p.Char.Inv[model.PlayerCarrySlots-2].Index != cityWarChequeItem || p.Char.Inv[model.PlayerCarrySlots-1].Index != cityWarChequeItem || w.guilds.Wars.Cities.Territories[0].Treasury != 1_500_000_000 {
			t.Fatalf("partial cheque withdrawal mismatch: tail=%d/%d treasury=%d", p.Char.Inv[model.PlayerCarrySlots-2].Index, p.Char.Inv[model.PlayerCarrySlots-1].Index, w.guilds.Wars.Cities.Territories[0].Treasury)
		}
	})

	t.Run("invalid city", func(t *testing.T) {
		w, _, p, _, _ := scheduledWarWorld(t, "2026-09-13 12:00")
		if err := w.withdrawCityTreasury(p, -1); err == nil {
			t.Fatal("invalid city treasury request accepted")
		}
	})
}

func TestCityTreasuryPersistenceFailureAndRetry(t *testing.T) {
	for name, amount := range map[string]uint64{"gold": 100, "cheques": cityWarChequeValue * 2} {
		t.Run(name, func(t *testing.T) {
			w, st, p, _, _ := scheduledWarWorld(t, "2026-09-13 12:00")
			w.guilds.Wars.Cities.Territories[0].Owner = 1
			w.guilds.Wars.Cities.Territories[0].Treasury = amount
			beforeGold, beforeInv := p.Char.Gold, p.Char.Inv
			drainPlayerPackets(p)
			st.err = errors.New("treasury commit unavailable")
			if err := w.withdrawCityTreasury(p, 0); err == nil {
				t.Fatal("failed treasury commit reported success")
			}
			if p.Char.Gold != beforeGold || p.Char.Inv != beforeInv || w.guilds.Wars.Cities.Territories[0].Treasury != amount {
				t.Fatal("failed withdrawal did not restore the player and treasury")
			}
			if p.Char != &p.Account.Chars[p.CharSlot] {
				t.Fatal("rollback detached the player from the authoritative account")
			}
			if p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("failed withdrawal published an uncommitted balance or item")
			}
			st.err = nil
			if err := w.withdrawCityTreasury(p, 0); err != nil {
				t.Fatalf("retry failed: %v", err)
			}
			if w.guilds.Wars.Cities.Territories[0].Treasury != 0 {
				t.Fatal("retry did not empty the treasury")
			}
			if amount < cityWarChequeValue {
				if p.Char.Gold != beforeGold+uint32(amount) || p.Char.Inv != beforeInv {
					t.Fatal("retry did not grant gold exactly once")
				}
			} else {
				first, second := p.Char.Inv[0], p.Char.Inv[1]
				if p.Char.Gold != beforeGold || first.Index != cityWarChequeItem || second.Index != cityWarChequeItem || first.UID == "" || first.UID == second.UID {
					t.Fatal("retry did not grant two uniquely identified cheques")
				}
			}
			afterGold, afterInv := p.Char.Gold, p.Char.Inv
			saves := st.gameSaves
			if err := w.withdrawCityTreasury(p, 0); err == nil || p.Char.Gold != afterGold || p.Char.Inv != afterInv || st.gameSaves != saves {
				t.Fatal("repeated withdrawal duplicated the payout or saved empty treasury")
			}
		})
	}
}
