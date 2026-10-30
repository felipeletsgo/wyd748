package game

import (
	"errors"
	"fmt"
	"reflect"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func fairyDustCatalog(t *testing.T) (model.Catalog, model.VolatileCatalog) {
	t.Helper()
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := data.LoadVolatilesWithInstances("../../data/content/volatiles.json",
		"../../data/content/instances.json", "../../data/spawns/instances", catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	return catalog, volatiles
}

func TestFairyDustAllEvolutions(t *testing.T) {
	catalog, volatiles := fairyDustCatalog(t)
	for _, evolution := range []string{"", "mortal", "arch", "celestial", "subcelestial"} {
		for _, itemID := range []uint16{414, 4142} {
			for _, coordinated := range []bool{false, true} {
				for _, reject := range []bool{false, true} {
					t.Run(fmt.Sprintf("%s/%d/coordinated-%t/reject-%t", evolution, itemID, coordinated, reject), func(t *testing.T) {
						w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 0)
						if !coordinated {
							w.criticalWrites = nil
						}
						w.items, w.skills, w.volatiles = catalog.Items, catalog.Skills, volatiles
						p.Char.Evolution, p.Char.Score.Level = evolution, 10
						table := progressionTable(p.Char)
						p.Char.Exp = table[10] + (table[11]-table[10])/2
						p.Char.Inv[0] = model.Item{Index: itemID, UID: "fairy-dust-source"}
						w.recalcPlayer(p.Char)
						before := accountStateSnapshot(p.Account)
						if reject {
							fallback.err = errors.New("save unavailable")
							base.failure = criticalCertifiedRejection(t)
						}
						w.onUseItem(p.Session, useItemPacket(0, 0))
						if coordinated {
							proposal := awaitCriticalWrite(t, base.entered)
							prepared := &proposal.accounts[0].Chars[p.CharSlot]
							if prepared.Score.Level != 11 || prepared.Exp != table[11] || prepared.Inv[0].Index != 0 {
								t.Fatal("Fairy Dust proposal did not bind one evolution-specific level and consumption")
							}
							w.onUseItem(p.Session, useItemPacket(0, 0))
							if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || len(q.pending) != 1 {
								t.Fatal("Fairy Dust changed live state or admitted repeated use before acknowledgement")
							}
							release()
							awaitReadyCriticalResults(t, q, 1)
							w.processCriticalCompletions(1)
							if fallback.saves != 0 || len(q.pending) != 0 || w.criticalPlayerReserved(p) {
								t.Fatal("Fairy Dust lost coordinated persistence or reservation cleanup")
							}
						}
						if reject {
							if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
								t.Fatal("failed Fairy Dust save changed progression or consumed the item")
							}
						} else if p.Char.Score.Level != 11 || p.Char.Exp != table[11] || p.Char.Inv[0].Index != 0 || p.Char.Evolution != evolution {
							t.Fatalf("Fairy Dust did not grant exactly one level: evolution=%s level=%d exp=%d item=%d",
								p.Char.Evolution, p.Char.Score.Level, p.Char.Exp, p.Char.Inv[0].Index)
						}
					})
				}
			}
		}
	}
}

func TestFairyDustPreservesProgressionLimits(t *testing.T) {
	catalog, volatiles := fairyDustCatalog(t)
	cases := []struct {
		evolution string
		level     uint32
		unlocked  bool
		accepted  bool
	}{
		{"mortal", 399, false, false}, {"arch", 399, true, false},
		{"celestial", 199, true, false}, {"subcelestial", 199, true, false},
		{"arch", 354, false, false}, {"arch", 369, false, false},
		{"celestial", 39, false, false}, {"celestial", 89, false, false},
		{"arch", 354, true, true}, {"arch", 369, true, true},
		{"celestial", 39, true, true}, {"celestial", 89, true, true},
		{"mortal", 398, false, true}, {"arch", 398, true, true},
		{"celestial", 198, true, true}, {"subcelestial", 198, false, true},
	}
	for _, tc := range cases {
		for _, coordinated := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s/%d/unlocked-%t/coordinated-%t", tc.evolution, tc.level, tc.unlocked, coordinated), func(t *testing.T) {
				w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 0)
				if !coordinated {
					w.criticalWrites = nil
				}
				w.items, w.skills, w.volatiles = catalog.Items, catalog.Skills, volatiles
				p.Char.Evolution, p.Char.Score.Level = tc.evolution, tc.level
				p.Char.ArchLevel355, p.Char.ArchLevel370 = tc.unlocked, tc.unlocked
				p.Char.CelestialLevel40Unlocked, p.Char.CelestialLevel90Unlocked = tc.unlocked, tc.unlocked
				table := progressionTable(p.Char)
				p.Char.Exp = table[tc.level]
				p.Char.Inv[0] = model.Item{Index: 414, UID: "fairy-dust-limit"}
				w.recalcPlayer(p.Char)
				before := accountStateSnapshot(p.Account)
				w.onUseItem(p.Session, useItemPacket(0, 0))
				if tc.accepted && coordinated {
					awaitCriticalWrite(t, base.entered)
					release()
					awaitReadyCriticalResults(t, q, 1)
					w.processCriticalCompletions(1)
				}
				if tc.accepted {
					if p.Char.Score.Level != tc.level+1 || p.Char.Exp != table[tc.level+1] || p.Char.Inv[0].Index != 0 {
						t.Fatal("Fairy Dust failed after an unlocked level or before the final level")
					}
				} else if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || len(q.pending) != 0 || fallback.saves != 0 {
					t.Fatal("Fairy Dust bypassed a progression lock or cap, or consumed a rejected item")
				}
			})
		}
	}
}
