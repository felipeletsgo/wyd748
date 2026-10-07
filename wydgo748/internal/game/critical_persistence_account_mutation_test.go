package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestCriticalAccountEconomySettlement(t *testing.T) {
	for _, operation := range []string{"arch-crystal", "reset-premium", "reset-sapphires", "gem", "ore-success", "ore-failure", "premium-firework", "grant-exp", "quest-reward", "grant-next-level"} {
		for _, outcome := range []string{"commit", "rejection", "unknown", "stale-uid"} {
			t.Run(operation+"/"+outcome, func(t *testing.T) {
				w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
				w.warConfig.Enabled = false
				p.Char.Inv = [64]model.Item{}
				p.Char.Equip = [16]model.Item{}
				p.Char.Class, p.Char.Score.Level = 0, 100
				p.Char.Score.Str, p.Char.Score.Int, p.Char.Score.Dex, p.Char.Score.Con = 158, 54, 17, 6
				w.items = map[uint16]model.ItemDef{}
				w.volatiles = model.VolatileCatalog{Items: map[uint16]model.VolatileRule{}, ItemCodes: map[uint16]int{}}
				var request func()
				switch operation {
				case "grant-exp", "quest-reward", "grant-next-level":
					p.Char.Exp = mortalNextLevel[100]
					p.Char.Inv[0] = model.Item{Index: 900, UID: "experience-reward"}
					w.items[900] = model.ItemDef{Index: 900}
					w.gameplay.EXPRatePercent = 100
					action, gold := "grant_exp", uint32(0)
					if operation == "quest-reward" {
						action, gold = "quest_reward", 100
					}
					if operation == "grant-next-level" {
						action = "grant_next_level"
					}
					w.volatiles.Items[900] = model.VolatileRule{Action: action, Exp: 10_000, Gold: gold, Consume: true, MaxLevelExclusive: 200}
					request = func() { w.onUseItem(p.Session, useItemPacket(0, 0)) }
				case "arch-crystal":
					p.Char.Evolution, p.Char.Exp = "arch", 2_600_000_000
					p.Char.Score.Level = levelForExp(p.Char.Exp)
					p.Char.Inv[0] = model.Item{Index: firstArchCrystal, UID: "arch-oath"}
					w.items[firstArchCrystal] = model.ItemDef{Index: firstArchCrystal}
					w.volatiles.Items[firstArchCrystal] = model.VolatileRule{Action: "arch_crystal", Consume: true}
					request = func() { w.onUseItem(p.Session, useItemPacket(0, 0)) }
				case "reset-premium", "reset-sapphires":
					for slot := 1; slot <= 3; slot++ {
						p.Char.Inv[slot] = model.Item{Index: model.SapphirePack}
					}
					if operation == "reset-premium" {
						p.Char.Inv[0] = model.Item{Index: abilityResetPremiumItem}
					}
					master := &Mob{Def: &model.NPCDef{Score: &model.Score{Merchant: abilityResetMasterMerchant}}}
					request = func() { w.handleAbilityResetMasterNPC(p.Session, p, master, 1) }
				case "gem":
					p.Char.Inv[0] = model.Item{Index: 3387, UID: "gem-source"}
					p.Char.Equip[1] = model.Item{Index: 1000, UID: "gem-target", Eff: [6]byte{43, 238}}
					w.items[3387], w.items[1000] = model.ItemDef{Index: 3387}, model.ItemDef{Index: 1000, Pos: 2}
					w.volatiles.Items[3387] = model.VolatileRule{Action: "equipment_gem", Consume: true, Variant: 1}
					request = func() { w.onUseItem(p.Session, indexedUsePacket(0, 1, placeEquip)) }
				case "ore-success", "ore-failure":
					p.Char.Inv[0] = model.Item{Index: 578, UID: "ore-source"}
					p.Char.Inv[1] = model.Item{Index: 2000, UID: "ore-target", Eff: [6]byte{43, 7}}
					w.items[578] = model.ItemDef{Index: 578}
					w.items[2000] = model.ItemDef{Index: 2000, Pos: 2, Grade: 1, Unique: 8, Extra: 2001}
					w.items[2001] = model.ItemDef{Index: 2001, Pos: 2, Grade: 4, Unique: 8}
					percent := 0
					if operation == "ore-success" {
						percent = 100
					}
					w.volatiles.Items[578] = model.VolatileRule{Action: "ore_upgrade", Consume: true, Variant: 3, SuccessPercent: percent}
					request = func() { w.onUseItem(p.Session, indexedUsePacket(0, 1, placeInv)) }
				case "premium-firework":
					p.Char.Inv[0] = model.Item{Index: 3442, UID: "premium-firework", Eff: [6]byte{effectAmount, 2}}
					w.items[3442] = model.ItemDef{Index: 3442}
					w.volatiles.Items[3442] = model.VolatileRule{Action: "firework", Consume: true, CustomPattern: true}
					request = func() { w.onUsePremiumFirework(p.Session, premiumFireworkPacket(0, p.X, p.Y, [16]byte{1})) }
				}
				for id := range w.volatiles.Items {
					w.volatiles.ItemCodes[id] = 1
				}
				w.recalcPlayer(p.Char)
				peer := criticalTinyTradePeer(t, w, p)
				account, character, score, trade, peerTrade := p.Account, p.Char, p.Char.Score, p.Trade, peer.Trade
				before, peerBefore := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account)
				if outcome == "rejection" {
					base.failure = criticalCertifiedRejection(t)
				} else if outcome == "unknown" {
					base.failure = errors.New("account economy acknowledgement lost")
				}
				request()
				proposal := awaitCriticalWrite(t, base.entered)
				if len(proposal.accounts) != 2 || !reflect.DeepEqual(proposal.accounts[1], accountPersistenceSnapshot(peerBefore)) {
					t.Fatal("economy operation did not atomically retain the reciprocal trade account")
				}
				prepared := &proposal.accounts[0].Chars[p.CharSlot]
				switch operation {
				case "grant-exp", "quest-reward":
					gold := before.Chars[p.CharSlot].Gold
					if operation == "quest-reward" {
						gold += 100
					}
					if prepared.Exp != before.Chars[p.CharSlot].Exp+10_000 || prepared.Gold != gold || prepared.Inv[0].Index != 0 {
						t.Fatal("experience reward did not bind progression, gold and source consumption")
					}
				case "grant-next-level":
					if prepared.Exp != mortalNextLevel[101] || prepared.Score.Level != 101 || prepared.Inv[0].Index != 0 {
						t.Fatal("next-level reward did not bind the exact threshold and consumption")
					}
				case "arch-crystal":
					if prepared.ArchCrystals != 1 || prepared.Exp != 2_500_000_000 || prepared.Inv[0].Index != 0 {
						t.Fatal("Arch oath did not bind experience debit and consumed crystal")
					}
				case "reset-premium", "reset-sapphires":
					if prepared.Score.Str != 58 || prepared.Score.Int != 4 || prepared.Score.Dex != 7 || prepared.Score.Con != 6 ||
						(operation == "reset-premium" && (prepared.Inv[0].Index != 0 || prepared.Inv[1] != before.Chars[p.CharSlot].Inv[1])) ||
						(operation == "reset-sapphires" && (prepared.Inv[1].Index != 0 || prepared.Inv[2].Index != 0 || prepared.Inv[3].Index != 0)) {
						t.Fatal("attribute reset changed payment priority or attribute limits")
					}
				case "gem":
					if prepared.Inv[0].Index != 0 || prepared.Equip[1].UID != "gem-target" || prepared.Equip[1].Eff[1] != 239 {
						t.Fatal("gem did not preserve target identity and bind consumption")
					}
				case "ore-success", "ore-failure":
					index := uint16(2000)
					if operation == "ore-success" {
						index = 2001
					}
					if prepared.Inv[0].Index != 0 || prepared.Inv[1].Index != index || prepared.Inv[1].UID != "ore-target" || prepared.Inv[1].Eff[1] != 7 {
						t.Fatal("ore did not bind its single roll to consumption and retained identity")
					}
				case "premium-firework":
					if prepared.Inv[0].UID != "premium-firework" || prepared.Inv[0].Eff[1] != 1 {
						t.Fatal("firework did not consume exactly one retained unit")
					}
				}
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, q, 1)
					}
					request()
					w.cancelTrade(peer, "competing cancellation")
					w.tickTrades(w.now().Add(time.Hour))
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
						p.Trade != trade || peer.Trade != peerTrade || !w.criticalPlayerReserved(p) || !w.criticalPlayerReserved(peer) ||
						!p.LastPremiumFirework.IsZero() || len(base.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 {
						t.Fatalf("%s mutated, published or repeated account economy", phase)
					}
				}
				if outcome == "stale-uid" {
					p.Char.UID = "replacement-character"
					before = accountStateSnapshot(p.Account)
				}
				w.processCriticalCompletions(1)
				if outcome == "commit" {
					if w.shuttingDown || len(q.pending) != 0 || p.Account != account || p.Char != character || p.Char.Score != score ||
						!reflect.DeepEqual(accountPersistenceSnapshot(p.Account), proposal.accounts[0]) || p.Session.QueuedPacketsForTest() == 0 {
						t.Fatal("account economy did not apply its acknowledged proposal preserving actor identity")
					}
					reset := operation == "reset-premium" || operation == "reset-sapphires"
					if reset && (p.Trade != trade || peer.Trade != peerTrade) || !reset && (p.Trade != nil || peer.Trade != nil) {
						t.Fatal("committed operation changed its trade lifecycle contract")
					}
					if operation == "premium-firework" && p.LastPremiumFirework != w.now() {
						t.Fatal("committed firework did not install its cooldown")
					}
				} else {
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
						p.Trade != trade || peer.Trade != peerTrade || !p.LastPremiumFirework.IsZero() {
						t.Fatal("failed operation installed an unverified reward, debit or cancellation")
					}
					if outcome == "rejection" {
						if w.shuttingDown || len(q.pending) != 0 || w.criticalPlayerReserved(p) || p.Session.QueuedPacketsForTest() == 0 || peer.Session.QueuedPacketsForTest() != 0 {
							t.Fatal("certified rejection did not resynchronize without cancelling the trade")
						}
					} else if !w.shuttingDown || len(q.pending) != 1 || !w.criticalPlayerReserved(p) || !w.criticalPlayerReserved(peer) ||
						p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("unknown outcome released retained state or published success")
					}
				}
				packets := p.Session.QueuedPacketsForTest()
				w.processCriticalCompletions(1)
				if p.Session.QueuedPacketsForTest() != packets || len(base.entered) != 0 || fallback.saves != 0 || fallback.gameSaves != 0 {
					t.Fatal("economy completion replayed or used inline persistence")
				}
			})
		}
	}
}
