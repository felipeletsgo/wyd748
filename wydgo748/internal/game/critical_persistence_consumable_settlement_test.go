package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"maps"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

var criticalConsumableSettlementActions = []string{
	"firework", "chaos_remission", "magical_pill", "learn_special_skill", "restore", "gold", "save_position", "disabled",
	"buff", "face_transform", "face_restore", "grant_counter", "grant_counter_once", "timed_access", "loot_box", "mount_revive",
	"refine", "refine-failure", "refine-earring", "refine-earring-failure", "refine_set", "tint", "untint", "repliction", "mount-longevity", "mount-longevity_restore",
	"mount-level_set", "mount-hatch", "incubation", "incubation-failure",
	"teleport", "warp_saved", "hunting_teleport", "territory_pass",
}

// Exercise real MSG_UseItem dispatch rather than bypassing its registered-item
// and pre-cancellation boundaries with a direct producer call.
func configureCriticalConsumableSettlement(w *World, p *Player, operation string) (model.VolatileRule, []byte) {
	p.Char.Inv, p.Char.Equip = [64]model.Item{}, [16]model.Item{}
	p.Char.Class, p.Char.Score.Level, p.Char.Exp, p.Char.Evolution = 0, 100, mortalNextLevel[100], "mortal"
	p.Char.Gold, p.Char.CP = 1000, 0
	p.Char.Inv[0] = model.Item{Index: 900, UID: "consumable-source"}
	w.items = map[uint16]model.ItemDef{900: {Index: 900}, 901: {Index: 901, Name: "Reward"}}
	w.volatiles = model.VolatileCatalog{Items: map[uint16]model.VolatileRule{}, ItemCodes: map[uint16]int{900: 1}}
	w.rng = fixedRNG{value: 0}
	rule := model.VolatileRule{Action: operation, Consume: true}
	packet := useItemPacket(0, 0)
	switch operation {
	case "teleport", "warp_saved", "hunting_teleport", "territory_pass":
		rule.X, rule.Y = cityWarZones[1].exitX, cityWarZones[1].exitY
		rule.RequiredEvolution = "mortal"
		p.Char.SavedX, p.Char.SavedY = rule.X, rule.Y
		if operation == "hunting_teleport" {
			for i := 0; i < 10; i++ {
				rule.Destinations = append(rule.Destinations, model.VolatileDestination{X: rule.X, Y: rule.Y})
			}
			binary.LittleEndian.PutUint32(packet[32:36], 1)
		}
	case "learn_special_skill":
		rule.LearnedBit = 25
	case "restore":
		rule.HP, rule.MP = 10, 10
	case "gold":
		rule.Gold = 100
	case "save_position":
		p.Char.SavedX, p.Char.SavedY = 1, 2
	case "buff":
		rule.AffectType, rule.DurationUnits = 39, 900
		rule.Accumulate, rule.MaxDurationUnits = true, 10800
	case "face_transform", "face_restore":
		rule.FaceMesh, rule.DurationUnits = 202, 10
		if operation == "face_restore" {
			setFaceAffectAt(p.Char, 202, 10, w.now())
		}
	case "grant_counter", "grant_counter_once":
		rule.Counters = map[string]uint32{"settlement-reward": 2}
		if operation == "grant_counter_once" {
			rule.RewardItems = []uint16{901}
		}
	case "timed_access":
		rule.AccessKey, rule.DurationSeconds = "settlement-access", 60
	case "loot_box":
		rule.RewardItems = []uint16{901}
	case "mount_revive", "mount-longevity", "mount-longevity_restore", "mount-level_set":
		mount := model.Item{Index: model.MountAdultBase, UID: "retained-mount"}
		mount.SetMountHP(10_000)
		mount.SetMountLongev(20)
		p.Char.Equip[mountSlot] = mount
		w.items[mount.Index] = model.ItemDef{Index: mount.Index, Price: 100}
		if operation == "mount_revive" {
			p.Char.Equip[mountSlot].SetMountHP(0)
		} else {
			rule.Action, rule.MountAction = "mount", operation[len("mount-"):]
			rule.Amount = 1
			if rule.MountAction == "level_set" {
				rule.Amount = 100
			}
		}
	case "mount-hatch", "incubation", "incubation-failure":
		p.Char.Equip[mountSlot] = model.Item{Index: model.MountEggBase, UID: "retained-egg"}
		w.items[model.MountEggBase] = model.ItemDef{Index: model.MountEggBase,
			StaticEffects: []model.StaticEffect{{Name: "EF_INCUBATE", Value: 6}}}
		if operation == "mount-hatch" {
			rule.Action, rule.MountAction = "mount", "hatch"
		} else {
			rule.Action = "refine"
			if operation == "incubation-failure" {
				w.rng = fixedRNG{value: 99}
				setEggProgress(&p.Char.Equip[mountSlot], 5)
			}
		}
		binary.LittleEndian.PutUint32(packet[20:24], placeEquip)
		binary.LittleEndian.PutUint32(packet[24:28], mountSlot)
	case "refine-earring", "refine-earring-failure":
		rule.Action, rule.RefineMax, rule.RefineTenChance = "refine", 9, 5
		w.volatiles.ItemCodes[900] = 5
		w.items[591] = model.ItemDef{Index: 591, Pos: 256}
		p.Char.Equip[8] = model.Item{Index: 591, UID: "earring-target", Eff: [6]byte{43, 9}}
		binary.LittleEndian.PutUint32(packet[20:24], placeEquip)
		binary.LittleEndian.PutUint32(packet[24:28], 8)
		if operation == "refine-earring-failure" {
			w.rng = fixedRNG{value: 99}
		}
	case "refine", "refine-failure", "refine_set", "tint", "untint", "repliction":
		p.Char.Equip[4] = model.Item{Index: 1000, UID: "equipment-target", Eff: [6]byte{43, 1}}
		w.items[1000] = model.ItemDef{Index: 1000, Pos: 4,
			StaticEffects: []model.StaticEffect{{Name: "EF_ITEMLEVEL", Value: 1}}, DynamicEffectNames: map[byte]string{43: "EF_SANC"}}
		binary.LittleEndian.PutUint32(packet[20:24], placeEquip)
		binary.LittleEndian.PutUint32(packet[24:28], 4)
		rule.RefineMax, rule.Color = 6, 116
		if operation == "refine-failure" {
			rule.Action = "refine"
			w.rng = fixedRNG{value: 99}
		}
		if operation == "untint" {
			p.Char.Equip[4].Eff[0] = 116
		}
		if operation == "refine_set" {
			rule.OnceQuestID = -123
		}
		if operation == "repliction" {
			w.volatiles.Repliction = model.ReplictionCatalog{
				Items: map[uint16]model.ReplictionItem{900: {ItemLevel: 1, MaxSanc: 6}},
				Pools: map[int][]model.ReplictionBonus{4: {{Effect1: 2, Value1: 30, Effect2: 3, Value2: 25}}},
			}
		}
	}
	w.volatiles.Items[900] = rule
	w.recalcPlayer(p.Char)
	if operation == "restore" {
		setPlayerCurHP(p.Char, playerMaxHP(p.Char)/2)
		setPlayerCurMP(p.Char, playerMaxMP(p.Char)/2)
	}
	return rule, packet
}

func TestCriticalConsumableSettlementMatrix(t *testing.T) {
	for _, operation := range criticalConsumableSettlementActions {
		for _, outcome := range []string{"commit", "rejection", "unknown", "stale-uid"} {
			t.Run(operation+"/"+outcome, func(t *testing.T) {
				w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
				w.warConfig.Enabled = false
				rule, packet := configureCriticalConsumableSettlement(w, p, operation)
				peer := criticalTinyTradePeer(t, w, p)
				account, character, score, trade, peerTrade := p.Account, p.Char, p.Char.Score, p.Trade, peer.Trade
				before, peerBefore := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account)
				coins, peerCoins := maps.Clone(p.SpecialCoins), maps.Clone(peer.SpecialCoins)
				eggUID, eggTick := p.EggIncubationUID, p.NextEggIncubationTick
				if outcome == "rejection" {
					base.failure = criticalCertifiedRejection(t)
				} else if outcome == "unknown" {
					base.failure = errors.New("consumable acknowledgement lost")
				}
				w.onUseItem(p.Session, packet)
				proposal := awaitCriticalWrite(t, base.entered)
				prepared := &proposal.accounts[0].Chars[p.CharSlot]
				if operation == "refine-earring" || operation == "refine-earring-failure" {
					want := 10
					if operation == "refine-earring-failure" {
						want = 9
					}
					if itemSanc(prepared.Equip[8]) != want || prepared.Equip[8].UID != "earring-target" {
						t.Fatal("earring proposal lost its refinement result or identity")
					}
				}
				if len(proposal.accounts) != 2 || !reflect.DeepEqual(proposal.accounts[1], accountPersistenceSnapshot(peerBefore)) ||
					prepared.Inv[0].Index != 0 && operation != "loot_box" && operation != "grant_counter_once" {
					t.Fatal("consumable proposal lost consumption or its reciprocal account")
				}
				if criticalSidecarConsumable(rule.Action) || rule.Action == "refine_set" {
					if proposal.character == nil || proposal.character.uid != p.Char.UID || proposal.character.state == nil {
						t.Fatal("consumable lost its atomic sidecar")
					}
				}
				if operation == "loot_box" || operation == "grant_counter_once" {
					if prepared.Inv[0].Index != 901 || prepared.Inv[0].UID == "" || prepared.Inv[0].UID == "consumable-source" {
						t.Fatal("reward did not replace the consumed unit with one private identity")
					}
				}
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, q, 1)
					}
					w.onUseItem(p.Session, packet)
					w.cancelTrade(peer, "competing cancellation")
					w.tickTrades(w.now().Add(time.Hour))
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
						!maps.Equal(p.SpecialCoins, coins) || !maps.Equal(peer.SpecialCoins, peerCoins) || p.Trade != trade || peer.Trade != peerTrade ||
						!p.LastPotion.IsZero() || p.EggIncubationUID != eggUID || p.NextEggIncubationTick != eggTick ||
						len(base.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 {
						t.Fatalf("%s consumable changed or published retained state", phase)
					}
				}
				if outcome == "stale-uid" {
					p.Char.UID = "replacement-character"
					before = accountStateSnapshot(p.Account)
				}
				w.processCriticalCompletions(1)
				if outcome == "commit" {
					if w.shuttingDown || len(q.pending) != 0 || p.Account != account || p.Char != character || p.Char.Score != score ||
						!reflect.DeepEqual(accountPersistenceSnapshot(p.Account), proposal.accounts[0]) || p.Trade != nil || peer.Trade != nil ||
						p.Session.QueuedPacketsForTest() == 0 || peer.Session.QueuedPacketsForTest() == 0 {
						t.Fatal("consumable did not install exactly its acknowledged proposal preserving actor identity")
					}
					if proposal.character != nil && !reflect.DeepEqual(buildCharState(p, w.now()), proposal.character.state) {
						t.Fatal("installed sidecar differs from the committed currency, affects or quest state")
					}
					if operation == "restore" && p.LastPotion != w.now() {
						t.Fatal("committed potion did not install its cooldown")
					}
					if operation == "incubation" || operation == "incubation-failure" {
						if p.EggIncubationUID != eggIncubationClockKey(&p.Char.Equip[mountSlot]) || p.NextEggIncubationTick != w.now().Add(eggIncubationHour) {
							t.Fatal("incubation did not start the acknowledged online clock")
						}
					}
				} else {
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
						!maps.Equal(p.SpecialCoins, coins) || !maps.Equal(peer.SpecialCoins, peerCoins) || p.Trade != trade || peer.Trade != peerTrade ||
						!p.LastPotion.IsZero() || p.EggIncubationUID != eggUID || p.NextEggIncubationTick != eggTick {
						t.Fatal("failed consumable installed an unverified effect or released trade state")
					}
					if outcome == "rejection" {
						if w.shuttingDown || len(q.pending) != 0 || w.criticalPlayerReserved(p) || p.Session.QueuedPacketsForTest() == 0 || peer.Session.QueuedPacketsForTest() != 0 {
							t.Fatal("certified rejection did not settle failure without cancelling the trade")
						}
					} else if !w.shuttingDown || len(q.pending) != 1 || !w.criticalPlayerReserved(p) || !w.criticalPlayerReserved(peer) ||
						p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("unverified outcome released fences or published success")
					}
				}
				packets := p.Session.QueuedPacketsForTest()
				w.processCriticalCompletions(1)
				if p.Session.QueuedPacketsForTest() != packets || len(base.entered) != 0 || fallback.saves != 0 || fallback.gameSaves != 0 {
					t.Fatal("consumable completion replayed or saved on World")
				}
			})
		}
	}
}

// Dye only paints the five armor set pieces; any other target is rejected
// before a critical write is proposed, so the dye and the target stay intact.
func TestCriticalTintRejectsTargetsOutsideArmorSet(t *testing.T) {
	// Pos -1 removes the target definition (unknown item).
	for _, pos := range []int{4, 64, 128, 192, 1 << 15, -1} {
		t.Run(fmt.Sprint(pos), func(t *testing.T) {
			w, p, _, _, _, _, _ := criticalTreasuryFixture(t, 100)
			w.warConfig.Enabled = false
			rule, packet := configureCriticalConsumableSettlement(w, p, "tint")
			if pos < 0 {
				delete(w.items, 1000)
			} else {
				def := w.items[1000]
				def.Pos = pos
				w.items[1000] = def
			}
			req, ok := parseUseItemRequest(packet)
			if !ok {
				t.Fatal("tint packet did not parse")
			}
			err := w.queueEquipmentConsumable(p, 0, req, rule)
			var rejection *nativeConsumableRejection
			rejected := errors.As(err, &rejection) && rejection.message == tintSetOnlyMessage
			if pos == 4 && (err != nil || rejected) {
				t.Fatalf("armor piece tint was rejected: %v", err)
			}
			if pos != 4 && !rejected {
				t.Fatalf("non-set tint was admitted: %v", err)
			}
		})
	}
}
