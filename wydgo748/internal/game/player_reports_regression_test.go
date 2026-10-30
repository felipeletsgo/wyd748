package game

import (
	"encoding/binary"
	"fmt"
	"reflect"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestArchStonesConvertInsteadOfRefining(t *testing.T) {
	catalog, volatiles := fairyDustCatalog(t)
	for source := uint16(1752); source <= 1759; source++ {
		for _, powder := range []uint16{412, 413} {
			for _, coordinated := range []bool{false, true} {
				for _, failRoll := range []bool{false, true} {
					t.Run(fmt.Sprintf("%d/%d/coordinated-%t/fail-%t", source, powder, coordinated, failRoll), func(t *testing.T) {
						w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 0)
						if !coordinated {
							w.criticalWrites = nil
						}
						w.items, w.skills, w.volatiles = catalog.Items, catalog.Skills, volatiles
						w.rng = fixedRNG{value: 29}
						if failRoll {
							w.rng = fixedRNG{value: 99}
						}
						p.Char.Inv[0] = model.Item{Index: powder, UID: "stone-powder", Eff: [6]byte{61, 2}}
						stone := model.Item{Index: source, UID: "arch-stone", Eff: [6]byte{43, 3}, ActivatedUnix: 123}
						p.Char.Equip[11] = stone
						before := accountStateSnapshot(p.Account)
						request := indexedUsePacket(0, 11, placeEquip)
						w.onUseItem(p.Session, request)
						want := stone
						if !failRoll {
							want.Index -= 8
						}
						if coordinated {
							proposal := awaitCriticalWrite(t, base.entered)
							if proposal.accounts[0].Chars[p.CharSlot].Equip[11] != want || !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
								t.Fatal("stone conversion proposal lost identity, result or acknowledgement isolation")
							}
							release()
							awaitReadyCriticalResults(t, q, 1)
							w.processCriticalCompletions(1)
							if fallback.saves != 0 {
								t.Fatal("conversion used synchronous persistence")
							}
						}
						if p.Char.Equip[11] != want || itemStackAmount(p.Char.Inv[0]) != 1 || p.Char.Inv[0].UID != "stone-powder" {
							t.Fatalf("conversion changed refinement or consumed the wrong unit: target=%+v powder=%+v", p.Char.Equip[11], p.Char.Inv[0])
						}
					})
				}
			}
		}
	}
	for _, tc := range []struct {
		roll int
		want uint16
	}{{1, 0}, {2, 1748}, {3, 1749}, {8, 1749}, {9, 1750}, {58, 1750}, {59, 1751}, {62, 1751}, {63, 0}, {100, 0}} {
		if got := archStoneResult(1758, tc.roll); got != tc.want {
			t.Fatalf("Lugefer roll %d: %d, want %d", tc.roll, got, tc.want)
		}
	}
}

func TestApprenticeCapeExchangeSettlement(t *testing.T) {
	catalog, _ := fairyDustCatalog(t)
	definitions, generators := officialNPCDataset(t)
	leader := definitions[apprenticeLeaderName]
	if leader == nil || leader.IsMonster() || leader.Equip.Rosto.Index != 58 {
		t.Fatal("missing compatible Apprentice leader definition")
	}
	terrain, err := data.LoadTerrain("../../data/world/maps/HeightMap.dat", "../../data/world/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	placements := 0
	for _, g := range generators {
		if g.Enabled && g.Leader == apprenticeLeaderName {
			placements++
			point := g.Segments[0]
			if point.X != 2234 || point.Y != 1567 || !terrain.Walkable(point.X, point.Y) {
				t.Fatal("Apprentice leader lost its valid quest position")
			}
		}
	}
	if placements != 1 {
		t.Fatalf("Apprentice leader placements=%d", placements)
	}
	for _, coordinated := range []bool{false, true} {
		for _, reject := range []bool{false, true} {
			t.Run(fmt.Sprintf("coordinated-%t/reject-%t", coordinated, reject), func(t *testing.T) {
				w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 0)
				if !coordinated {
					w.criticalWrites = nil
				}
				w.items = catalog.Items
				p.Char.Evolution = ""
				p.Char.Score.Level = 100
				p.Char.Exp = mortalNextLevel[100]
				p.Char.Equip[13] = model.Item{Index: 4080, UID: "apprentice-emblem"}
				p.Char.Equip[15] = model.Item{}
				w.recalcPlayer(p.Char)
				observer, _ := networkedTestPlayer(2, "CapeObserver", p.X, p.Y)
				w.players[observer.Session], w.playersByID[observer.ID] = observer, observer
				w.updatePlayerSpatial(observer)
				observer.show(p.ID)
				m := &Mob{ID: 1450, X: p.X + 1, Y: p.Y, HP: 100, GenerIndex: -1, Def: leader}
				w.appendMobInstance(m)
				w.registerMobSpatial(m)
				p.show(m.ID)
				packet := useNPCPacket(m.ID)
				binary.LittleEndian.PutUint32(packet[16:20], 1)
				before := accountStateSnapshot(p.Account)
				if reject {
					fallback.err = criticalCertifiedRejection(t)
					base.failure = criticalCertifiedRejection(t)
				}
				w.onUseNPC(p.Session, packet)
				if coordinated {
					proposal := awaitCriticalWrite(t, base.entered)
					if proposal.accounts[0].Chars[p.CharSlot].Equip[15].Index != 4006 || !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
						t.Fatal("cape escaped persistence acknowledgement")
					}
					if observer.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("cape appearance reached an observer before commit")
					}
					release()
					awaitReadyCriticalResults(t, q, 1)
					w.processCriticalCompletions(1)
				}
				if reject {
					if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
						t.Fatal("failed cape exchange changed equipment")
					}
					if observer.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("rejected cape appearance reached an observer")
					}
				} else {
					if p.Char.Equip[13].Index != 0 || p.Char.Equip[15].Index != 4006 || p.Char.Equip[15].UID == "" {
						t.Fatal("cape exchange did not consume the equipped emblem and grant the cape")
					}
					if slot := firstFreeVisibleInventorySlot(&model.Char{Inv: before.Chars[p.CharSlot].Inv}); slot < 0 ||
						p.Char.Inv[slot].Index != apprenticeRecallReward || p.Char.Inv[slot].UID == "" {
						t.Fatal("cape exchange did not grant the Recall Scroll (10) reward")
					}
					appearance, ok := observer.Session.DequeuePacketForTest()
					if !ok || !wire.Decrypt(appearance) || wire.ParseHeader(appearance).Type != wire.OpUpdateEquip ||
						!reflect.DeepEqual(appearance[12:], playerAppearancePacket(p)[12:]) {
						t.Fatal("observer did not receive the committed cape appearance")
					}
					after := accountStateSnapshot(p.Account)
					w.onUseNPC(p.Session, packet)
					if !reflect.DeepEqual(after, accountStateSnapshot(p.Account)) {
						t.Fatal("cape exchange replay changed equipment")
					}
				}
			})
		}
	}
}

func TestArchStoneConversionSaveRejectionPreservesItems(t *testing.T) {
	catalog, volatiles := fairyDustCatalog(t)
	for _, coordinated := range []bool{false, true} {
		t.Run(fmt.Sprintf("coordinated-%t", coordinated), func(t *testing.T) {
			w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 0)
			if !coordinated {
				w.criticalWrites = nil
			}
			w.items, w.volatiles = catalog.Items, volatiles
			w.rng = fixedRNG{value: 29}
			p.Char.Inv[0] = model.Item{Index: 412, UID: "rejected-powder", Eff: [6]byte{61, 2}}
			p.Char.Equip[11] = model.Item{Index: 1758, UID: "rejected-stone", Eff: [6]byte{43, 3}, ActivatedUnix: 123}
			before := accountStateSnapshot(p.Account)
			fallback.err, base.failure = criticalCertifiedRejection(t), criticalCertifiedRejection(t)
			request := indexedUsePacket(0, 11, placeEquip)
			w.onUseItem(p.Session, request)
			if coordinated {
				awaitCriticalWrite(t, base.entered)
				w.onUseItem(p.Session, request)
				if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
					t.Fatal("pending repeated conversion changed live items")
				}
				release()
				awaitReadyCriticalResults(t, q, 1)
				w.processCriticalCompletions(1)
			}
			if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
				t.Fatal("rejected conversion consumed powder or changed the stone")
			}
		})
	}
}

func TestApprenticeCapeRejectsInvalidRequests(t *testing.T) {
	catalog, _ := fairyDustCatalog(t)
	for _, invalid := range []string{"level-low", "level-high", "arch", "missing-emblem", "worn-cape", "unconfirmed", "dead", "hidden", "distant", "missing-reward", "missing-scroll", "full-inventory"} {
		t.Run(invalid, func(t *testing.T) {
			w, p, store := handlerTestWorld(t)
			w.items = make(map[uint16]model.ItemDef, len(catalog.Items))
			for id, def := range catalog.Items {
				w.items[id] = def
			}
			p.Char.Evolution = "mortal"
			p.Char.Score.Level = 100
			p.Char.Equip[13] = model.Item{Index: 4080, UID: "kept-emblem"}
			p.Char.Equip[15] = model.Item{}
			m := &Mob{ID: 1450, X: p.X + 1, Y: p.Y, HP: 100, GenerIndex: -1, Def: testNPCDef(model.Score{})}
			m.Def.Name = apprenticeLeaderName
			w.registerMobSpatial(m)
			p.show(m.ID)
			packet := useNPCPacket(m.ID)
			binary.LittleEndian.PutUint32(packet[16:20], 1)
			switch invalid {
			case "level-low":
				p.Char.Score.Level = 99
			case "level-high":
				p.Char.Score.Level = 150
			case "arch":
				p.Char.Evolution = "arch"
			case "missing-emblem":
				p.Char.Equip[13] = model.Item{}
			case "worn-cape":
				p.Char.Equip[15] = model.Item{Index: 4006, UID: "kept-cape"}
			case "unconfirmed":
				binary.LittleEndian.PutUint32(packet[16:20], 0)
			case "dead":
				p.Char.Score.CurHP = 0
			case "hidden":
				p.hide(m.ID)
			case "distant":
				m.X += 100
			case "missing-reward":
				delete(w.items, 4006)
			case "missing-scroll":
				delete(w.items, apprenticeRecallReward)
			case "full-inventory":
				for slot := 0; slot < model.PlayerCarrySlots; slot++ {
					p.Char.Inv[slot] = model.Item{Index: 412, UID: fmt.Sprintf("filler-%d", slot)}
				}
			}
			before := accountStateSnapshot(p.Account)
			w.onUseNPC(p.Session, packet)
			if store.saves != 0 || !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
				t.Fatal("invalid cape request changed account state")
			}
		})
	}
}

func TestApprenticeCapeEmblemSourcesSpawn(t *testing.T) {
	definitions, generators := officialNPCDataset(t)
	terrain, err := data.LoadTerrain("../../data/world/maps/HeightMap.dat", "../../data/world/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	w := newZoneTestWorld()
	w.terrain, w.rng = terrain, fixedRNG{}
	groups := 0
	for _, g := range generators {
		if !g.Enabled || g.Leader != "Rei_Taurus@" {
			continue
		}
		groups++
		leader := definitions[g.Leader]
		if leader == nil || !leader.IsMonster() || g.MinuteGenerate <= 0 || w.npcGeneratorReservation(g) != "" {
			t.Fatal("Apprentice emblem source is absent, disabled or reserved")
		}
		hasEmblem := false
		for _, item := range leader.Carry {
			hasEmblem = hasEmblem || item.Index == 4080
		}
		if !hasEmblem {
			t.Fatal("Apprentice monster lost its emblem drop")
		}
		state := generState{def: g, leader: leader, follower: definitions[g.Follower]}
		w.spawnGroup(&state)
		if state.current == 0 {
			t.Fatal("Apprentice emblem source did not spawn")
		}
	}
	if groups != 3 {
		t.Fatalf("Apprentice emblem source groups=%d, want 3", groups)
	}
	for _, mob := range w.mobsByID {
		if !terrain.Walkable(mob.X, mob.Y) || mob.HP == 0 {
			t.Fatal("Apprentice monster spawned dead or on blocked terrain")
		}
	}
}

func TestResetCompoundRoutesAcceptMortalAliases(t *testing.T) {
	catalog, volatiles := fairyDustCatalog(t)
	quests, err := data.LoadQuests("../../data/content/quests.json")
	if err != nil {
		t.Fatal(err)
	}
	definitions, _ := officialNPCDataset(t)
	for _, evolution := range []string{"", "mortal"} {
		for _, coordinated := range []bool{false, true} {
			for _, quest := range quests.Quests {
				if quest.NPC != "Priest_Kruno" && quest.NPC != "Priest_Ameria" {
					continue
				}
				t.Run(fmt.Sprintf("%s/%s/coordinated-%t", quest.NPC, evolution, coordinated), func(t *testing.T) {
					w, p, _, _, base, q, release := criticalTreasuryFixture(t, 0)
					if !coordinated {
						w.criticalWrites = nil
					}
					w.items, w.skills, w.volatiles = catalog.Items, catalog.Skills, volatiles
					w.questsByNPC = map[string]*model.QuestDef{model.NormalizeQuestNPC(quest.NPC): &quest}
					p.Char.Evolution = evolution
					p.Char.Score.Level = quest.Requires.MinLevel
					p.Char.Exp = mortalNextLevel[p.Char.Score.Level]
					p.Char.LearnedSkill = 0x00ffffff
					p.Char.Score.Mastery[1] = 80
					p.Char.Inv[0] = model.Item{Index: quest.Consumes[0].Index, UID: "compound-fragment"}
					w.recalcPlayer(p.Char)
					m := &Mob{ID: 1450, X: p.X + 1, Y: p.Y, HP: 100, GenerIndex: -1, Def: definitions[quest.NPC]}
					w.registerMobSpatial(m)
					p.show(m.ID)
					packet := useNPCPacket(m.ID)
					binary.LittleEndian.PutUint32(packet[16:20], 1)
					w.onUseNPC(p.Session, packet)
					if coordinated {
						awaitCriticalWrite(t, base.entered)
						release()
						awaitReadyCriticalResults(t, q, 1)
						w.processCriticalCompletions(1)
					}
					if p.Char.Inv[0].Index != quest.Rewards.Items[0].Index {
						t.Fatal("Mortal alias could not obtain its reset compound")
					}
					w.onUseItem(p.Session, useItemPacket(0, 0))
					if coordinated {
						awaitCriticalWrite(t, base.entered)
						awaitReadyCriticalResults(t, q, 1)
						w.processCriticalCompletions(1)
					}
					if p.Char.Inv[0].Index != 0 || p.Char.LearnedSkill&0x00ffffff != 0 {
						t.Fatal("real reset compound did not clear Mortal skills")
					}
				})
			}
		}
	}
}

func TestDismissSummonsPreservesOtherOwnersAndKinds(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	observer, _ := networkedTestPlayer(2, "Observer", p.X, p.Y)
	w.players[observer.Session] = observer
	for i, kind := range []byte{summonKindBM, summonKindContract, summonKindMount, summonKindBM} {
		owner := p.ID
		if i == 3 {
			owner++
		}
		m := &Mob{ID: uint16(1450 + i), X: p.X, Y: p.Y, HP: 100, GenerIndex: -1, SummonerID: owner, SummonKind: kind, Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
		w.appendMobInstance(m)
		w.registerMobSpatial(m)
		p.show(m.ID)
		observer.show(m.ID)
	}
	if !w.dispatchChatCommand(p.Session, p, "dispensar", "") {
		t.Fatal("dismiss command missing")
	}
	if w.mobsByID[1450] != nil || w.summons[1450] != nil {
		t.Fatal("owned BM summon survived dismissal")
	}
	if p.hasVisible(1450) || observer.hasVisible(1450) {
		t.Fatal("dismissed summon remained visible to owner or observer")
	}
	for _, id := range []uint16{1451, 1452, 1453} {
		if w.mobsByID[id] == nil {
			t.Fatal("dismissal removed another owner or summon kind")
		}
	}
	if !w.castSummon(p, model.SkillDef{Index: 60, InstanceValue: 5}, 100) {
		t.Fatal("dismissal did not permit another BM family")
	}
}

func TestDismissSummonsWaitsForPendingWrite(t *testing.T) {
	w, p, _, _, _, q, _ := criticalTreasuryFixture(t, 0)
	m := &Mob{ID: 1450, X: p.X, Y: p.Y, HP: 100, GenerIndex: -1, SummonerID: p.ID, SummonKind: summonKindBM, Def: testNPCDef(model.Score{})}
	w.appendMobInstance(m)
	w.registerMobSpatial(m)
	p.show(m.ID)
	key := accountSessionKey(p.Account.Name)
	q.accounts[key] = 1
	w.dismissBMSummons(p)
	if w.summons[m.ID] != m || !p.hasVisible(m.ID) {
		t.Fatal("pending write did not preserve the owned summon")
	}
	delete(q.accounts, key)
	w.dismissBMSummons(p)
	if w.summons[m.ID] != nil || p.hasVisible(m.ID) {
		t.Fatal("settled write did not allow dismissal")
	}
}
