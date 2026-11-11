package game

import (
	"encoding/binary"
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
)

func TestOnUseItemRegisteredActionsRejectMissingCatalogDefinitions(t *testing.T) {
	tests := []struct {
		name string
		rule model.VolatileRule
	}{
		{name: "donate coin", rule: model.VolatileRule{Action: "grant_counter", Consume: true, Counters: map[string]uint32{donateCounter: 1}}},
		{name: "teleport", rule: model.VolatileRule{Action: "recall", Consume: true}},
		{name: "gate key", rule: model.VolatileRule{Action: "gate_key", Consume: true}},
		{name: "summon contract", rule: model.VolatileRule{Action: "summon_contract", Consume: true}},
		{name: "celestial", rule: model.VolatileRule{Action: "celestial_switch", Consume: true}},
		{name: "mount", rule: model.VolatileRule{Action: "mount", Consume: true}},
		{name: "equipment", rule: model.VolatileRule{Action: "refine", Consume: true}},
		{name: "item reward", rule: model.VolatileRule{Action: "loot_box", Consume: true}},
		{name: "sidecar", rule: model.VolatileRule{Action: "buff", Consume: true}},
		{name: "basic", rule: model.VolatileRule{Action: "restore", Consume: true}},
		{name: "experience", rule: model.VolatileRule{Action: "grant_exp", Consume: true, Exp: 1}},
		{name: "equipment gem", rule: model.VolatileRule{Action: "equipment_gem", Consume: true}},
		{name: "ore upgrade", rule: model.VolatileRule{Action: "ore_upgrade", Consume: true}},
		{name: "native", rule: model.VolatileRule{Action: "nightmare_ticket", Consume: true}},
		{name: "instance ticket", rule: model.VolatileRule{Action: "instance_ticket", Consume: true, Instance: &model.VolatileInstance{ID: "missing-catalog"}}},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w, p, st := useItemWorld(tc.rule)
			w.items = map[uint16]model.ItemDef{}
			w.criticalWrites = newCriticalPersistenceOwner(&criticalPersistence{})
			before := p.Char.Inv[0]

			w.onUseItem(p.Session, useItemPacket(0, 0))

			if p.Char.Inv[0] != before {
				t.Fatalf("missing catalog definition changed source item: before=%+v after=%+v", before, p.Char.Inv[0])
			}
			if got := p.Session.QueuedPacketsForTest(); got == 0 {
				t.Fatal("missing catalog definition did not resync the source item")
			}
			if st.saves != 0 {
				t.Fatalf("missing catalog definition persisted %d times", st.saves)
			}
		})
	}
}

func TestOnUseItemCataloguedGateAndSummonUseCriticalHandlers(t *testing.T) {
	tests := []struct {
		name string
		rule model.VolatileRule
	}{
		{name: "gate key without key id", rule: model.VolatileRule{Action: "gate_key", Consume: true}},
		{name: "contract without summon configuration", rule: model.VolatileRule{Action: "summon_contract", Consume: true}},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w, p, st := useItemWorld(tc.rule)
			w.criticalWrites = newCriticalPersistenceOwner(&criticalPersistence{})
			before := p.Char.Inv[0]

			w.onUseItem(p.Session, useItemPacket(0, 0))

			if p.Char.Inv[0] != before {
				t.Fatalf("rejected action changed source item: before=%+v after=%+v", before, p.Char.Inv[0])
			}
			if p.Session.QueuedPacketsForTest() == 0 {
				t.Fatal("rejected action did not resync the source item")
			}
			if st.saves != 0 {
				t.Fatalf("rejected action persisted %d times", st.saves)
			}
		})
	}
}

func TestOnUseItemLegacyRejectedActionsPreserveItem(t *testing.T) {
	tests := []struct {
		name   string
		rule   model.VolatileRule
		packet func() []byte
	}{
		{name: "warp without saved position", rule: model.VolatileRule{Action: "warp_saved", Consume: true}},
		{name: "buff without effects", rule: model.VolatileRule{Action: "buff", Consume: true}},
		{name: "zero experience reward", rule: model.VolatileRule{Action: "grant_exp", Consume: true}},
		{name: "zero gold value", rule: model.VolatileRule{Action: "gold", Consume: true}},
		{name: "tint without target", rule: model.VolatileRule{Action: "tint", Consume: true}, packet: func() []byte { return useItemPacket(0, 1) }},
		{name: "untint without target", rule: model.VolatileRule{Action: "untint", Consume: true}, packet: func() []byte { return useItemPacket(0, 1) }},
		{name: "repliction from inventory", rule: model.VolatileRule{Action: "repliction", Consume: true}},
		{name: "invalid skill bit", rule: model.VolatileRule{Action: "learn_special_skill", Consume: true, LearnedBit: 0}},
		{name: "face transform without mesh", rule: model.VolatileRule{Action: "face_transform", Consume: true}},
		{name: "contract without summon configuration", rule: model.VolatileRule{Action: "summon_contract", Consume: true}},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w, p, st := useItemWorld(tc.rule)
			before := p.Char.Inv[0]
			packet := useItemPacket(0, 0)
			if tc.packet != nil {
				packet = tc.packet()
			}

			w.onUseItem(p.Session, packet)

			if p.Char.Inv[0] != before {
				t.Fatalf("rejected action changed source item: before=%+v after=%+v", before, p.Char.Inv[0])
			}
			if p.Session.QueuedPacketsForTest() == 0 {
				t.Fatal("rejected action did not resync the source item")
			}
			if st.saves != 0 {
				t.Fatalf("rejected action persisted %d times", st.saves)
			}
		})
	}
}

func TestOnUseItemLegacyPersistenceFailuresRestoreConsumableState(t *testing.T) {
	tests := []struct {
		name  string
		rule  model.VolatileRule
		setup func(*testing.T, *World, *Player)
	}{
		{name: "disabled item", rule: model.VolatileRule{Action: "disabled", Consume: true}},
		{name: "experience reward", rule: model.VolatileRule{Action: "grant_exp", Consume: true, Exp: 1}},
		{name: "face transformation", rule: model.VolatileRule{Action: "face_transform", Consume: true, FaceMesh: 202, DurationUnits: 1}},
		{name: "face restoration", rule: model.VolatileRule{Action: "face_restore", Consume: true}, setup: func(t *testing.T, w *World, p *Player) {
			t.Helper()
			if !setFaceAffectAt(p.Char, 202, 1, w.now()) {
				t.Fatal("test fixture could not install face transformation")
			}
		}},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w, p, st := useItemWorld(tc.rule)
			st.err = errors.New("test write failure")
			if tc.setup != nil {
				tc.setup(t, w, p)
			}
			beforeItem := p.Char.Inv[0]
			beforeAccount := accountStateSnapshot(p.Account)
			beforeAffects := p.Char.Affects

			w.onUseItem(p.Session, useItemPacket(0, 0))

			if p.Char.Inv[0] != beforeItem || !reflect.DeepEqual(p.Char.Affects, beforeAffects) ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), beforeAccount) {
				t.Fatalf("failed persistence did not roll back item and character state: item=%+v effects=%+v", p.Char.Inv[0], p.Char.Affects)
			}
			if st.saves == 0 {
				t.Fatal("action did not attempt persistence")
			}
		})
	}
}

func TestOnUseItemLegacyRemainingRejectionsPreserveState(t *testing.T) {
	t.Run("saved-position teleport fails", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "warp_saved", Consume: true})
		p.Char.SavedX, p.Char.SavedY = ^uint16(0), ^uint16(0)
		st.err = errors.New("test write failure")
		beforeItem, beforeX, beforeY := p.Char.Inv[0], p.X, p.Y

		w.onUseItem(p.Session, useItemPacket(0, 0))

		if p.Char.Inv[0] != beforeItem || p.X != beforeX || p.Y != beforeY || st.saves != 1 {
			t.Fatalf("failed saved-position teleport changed state: item=%+v pos=(%d,%d) saves=%d", p.Char.Inv[0], p.X, p.Y, st.saves)
		}
	})

	t.Run("buff with an unknown skill", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "buff", Consume: true, Affects: []model.VolatileAffect{{SkillID: 65535, DurationUnits: 1}}})
		beforeItem, beforeAffects := p.Char.Inv[0], p.Char.Affects

		w.onUseItem(p.Session, useItemPacket(0, 0))

		if p.Char.Inv[0] != beforeItem || p.Char.Affects != beforeAffects || st.saves != 0 {
			t.Fatalf("invalid buff changed state: item=%+v saves=%d", p.Char.Inv[0], st.saves)
		}
	})

	t.Run("unrefined armor cannot be tinted", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "tint", Consume: true, Color: 120})
		beforeTarget := model.Item{Index: 200}
		p.Char.Inv[1] = beforeTarget
		beforeSource := p.Char.Inv[0]

		w.onUseItem(p.Session, useItemPacket(0, 1))

		if p.Char.Inv[0] != beforeSource || p.Char.Inv[1] != beforeTarget || st.saves != 0 {
			t.Fatalf("invalid tint changed state: source=%+v target=%+v saves=%d", p.Char.Inv[0], p.Char.Inv[1], st.saves)
		}
	})

	t.Run("item without dye cannot be untinted", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "untint", Consume: true})
		beforeTarget := model.Item{Index: 200, Eff: [6]byte{43, 9}}
		p.Char.Inv[1] = beforeTarget
		beforeSource := p.Char.Inv[0]

		w.onUseItem(p.Session, useItemPacket(0, 1))

		if p.Char.Inv[0] != beforeSource || p.Char.Inv[1] != beforeTarget || st.saves != 0 {
			t.Fatalf("invalid untint changed state: source=%+v target=%+v saves=%d", p.Char.Inv[0], p.Char.Inv[1], st.saves)
		}
	})

	t.Run("repliction target missing from catalog", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "repliction", Consume: true})
		delete(w.items, 200)
		beforeTarget := model.Item{Index: 200}
		p.Char.Equip[4] = beforeTarget
		beforeSource := p.Char.Inv[0]
		packet := useItemPacket(0, 4)
		binary.LittleEndian.PutUint32(packet[20:24], placeEquip)

		w.onUseItem(p.Session, packet)

		if p.Char.Inv[0] != beforeSource || p.Char.Equip[4] != beforeTarget || st.saves != 0 {
			t.Fatalf("unknown target changed state: source=%+v target=%+v saves=%d", p.Char.Inv[0], p.Char.Equip[4], st.saves)
		}
	})

	t.Run("repliction source is not configured", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "repliction", Consume: true})
		beforeTarget := model.Item{Index: 200, Eff: [6]byte{43, 4}}
		p.Char.Equip[4] = beforeTarget
		beforeSource := p.Char.Inv[0]
		packet := useItemPacket(0, 4)
		binary.LittleEndian.PutUint32(packet[20:24], placeEquip)

		w.onUseItem(p.Session, packet)

		if p.Char.Inv[0] != beforeSource || p.Char.Equip[4] != beforeTarget || st.saves != 0 {
			t.Fatalf("unconfigured repliction changed state: source=%+v target=%+v saves=%d", p.Char.Inv[0], p.Char.Equip[4], st.saves)
		}
	})
}

func TestOnUseItemLegacyEquippedDyeFlowsRestoreAndPublish(t *testing.T) {
	for _, action := range []string{"tint", "untint"} {
		for _, failSave := range []bool{false, true} {
			name := action + "/commit"
			if failSave {
				name = action + "/rollback"
			}
			t.Run(name, func(t *testing.T) {
				rule := model.VolatileRule{Action: action, Consume: true, Color: 120}
				w, p, st := useItemWorld(rule)
				beforeTarget := model.Item{Index: 200, Eff: [6]byte{43, 9}}
				if action == "untint" {
					beforeTarget.Eff[0] = 120
				}
				p.Char.Equip[4] = beforeTarget
				if failSave {
					st.err = errors.New("test write failure")
				}
				packet := useItemPacket(0, 4)
				binary.LittleEndian.PutUint32(packet[20:24], placeEquip)

				w.onUseItem(p.Session, packet)

				if failSave {
					if p.Char.Inv[0].Index != 100 || p.Char.Equip[4] != beforeTarget {
						t.Fatalf("save failure did not restore equipment and source: source=%+v target=%+v", p.Char.Inv[0], p.Char.Equip[4])
					}
				} else {
					wantCode := byte(120)
					if action == "untint" {
						wantCode = 43
					}
					if p.Char.Inv[0].Index != 0 || p.Char.Equip[4].Eff[0] != wantCode {
						t.Fatalf("committed dye action has wrong result: source=%+v target=%+v", p.Char.Inv[0], p.Char.Equip[4])
					}
				}
				if st.saves != 1 {
					t.Fatalf("dye action attempted %d saves, want 1", st.saves)
				}
			})
		}
	}
}

func TestOnUseItemLegacyContractSummonCommitsAfterSave(t *testing.T) {
	rule := model.VolatileRule{Action: "summon_contract", Consume: true,
		Summon: &model.VolatileSummon{Name: "Test Guardian", Face: 300, HP: 500, Attack: 300, Defense: 100, MoveSpeed: 4, AttackRange: 3}}
	w, p, st := useItemWorld(rule)

	w.onUseItem(p.Session, useItemPacket(0, 0))

	if p.Char.Inv[0].Index != 0 || st.saves != 1 || len(w.summons) != 1 {
		t.Fatalf("contract summon was not committed: item=%d saves=%d summons=%d", p.Char.Inv[0].Index, st.saves, len(w.summons))
	}
	for _, summon := range w.summons {
		if summon == nil || summon.SummonKind != summonKindContract {
			t.Fatalf("unexpected contract summon: %+v", summon)
		}
	}
}

func TestOnUseItemLegacyExperienceRewardCommitsAfterSave(t *testing.T) {
	w, p, st := useItemWorld(model.VolatileRule{Action: "grant_exp", Consume: true, Exp: 100})
	beforeExp := p.Char.Exp

	w.onUseItem(p.Session, useItemPacket(0, 0))

	if p.Char.Exp <= beforeExp || p.Char.Inv[0].Index != 0 || st.saves != 1 {
		t.Fatalf("experience reward did not commit: exp=%d before=%d item=%d saves=%d", p.Char.Exp, beforeExp, p.Char.Inv[0].Index, st.saves)
	}
}
