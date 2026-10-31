package game

import (
	"encoding/binary"
	"errors"
	"path/filepath"
	"testing"
	"time"
	"wydgo/internal/data"
	"wydgo/internal/model"
)

func TestFreezeSpiralIncludesSummonBesidePlayerTarget(t *testing.T) {
	w, owner, attacker, summon := bmSummonPvPWorld(t, 2000)
	w.rng = fixedRNG{value: 0}
	owner.Char.Score.MaxHP, owner.Char.Score.CurHP = 1000000, 1000000
	applyScore(owner.Char)
	attacker.Char.Class, attacker.Char.LearnedSkill = 0, 1<<23
	attacker.Char.Score.MaxMP, attacker.Char.Score.CurMP = 10000, 10000
	attacker.Char.Score.Str = 1000
	applyScore(attacker.Char)
	w.skills = map[int]model.SkillDef{23: {Index: 23, TargetType: 5, Range: 4, Aggressive: 1, MaxTarget: 8, InstanceType: 3, InstanceValue: 210, ManaSpent: 150}}
	w.onSkillAttack(attacker, skillCastRequest{Skill: 23, TargetID: owner.ID})
	if summon.HP >= 500 {
		t.Fatal("Freeze Spiral skipped the enemy summon next to its player target")
	}
}

func TestPursuitPullsOnlyAuthorizedPlayer(t *testing.T) {
	c := reportTestCatalog(t)
	for _, pk := range []bool{true, false} {
		w, owner, attacker, _ := bmSummonPvPWorld(t, 2000)
		owner.X, owner.Char.X = 1994, 1994
		w.updatePlayerSpatial(owner)
		w.rng = fixedRNG{value: 0}
		w.skills = c.Skills
		attacker.Char.Class, attacker.Char.LearnedSkill = 0, 1<<6
		attacker.Char.Score.MaxMP, attacker.Char.Score.CurMP = 10000, 10000
		applyScore(attacker.Char)
		attacker.PKMode = pk
		w.onSkillAttack(attacker, skillCastRequest{Skill: 6, TargetID: owner.ID})
		if pk && chebyshev(attacker.X, attacker.Y, owner.X, owner.Y) > 2 {
			t.Fatal("Pursuit did not pull its authorized player target")
		}
		if !pk && (owner.X != 1994 || playerCurMP(attacker.Char) != 10000) {
			t.Fatal("Pursuit bypassed PK permissions")
		}
	}
}

func reportTestCatalog(t *testing.T) model.Catalog {
	t.Helper()
	c, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	return c
}

func TestMasteryTestAmuletWorksForMortals(t *testing.T) {
	c := reportTestCatalog(t)
	npcs, err := data.LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	stock := map[uint16]bool{}
	for _, npc := range npcs {
		if npc.Name == "ShopAmulet" {
			for _, it := range npc.Vende {
				stock[it.Index] = true
			}
		}
	}
	for _, id := range []uint16{6016, 6017, 6018} {
		if !stock[id] {
			t.Fatalf("ShopAmulet misses test item %d", id)
		}
	}
	for class := byte(0); class < 4; class++ {
		ch := &model.Char{Class: class, Evolution: "mortal", Score: testScore(model.Score{MaxHP: 100, CurHP: 100})}
		ch.Equip[9] = model.Item{Index: 6016}
		w := &World{items: c.Items}
		w.recalcPlayer(ch)
		for _, mastery := range effectiveScore(ch).Mastery {
			if mastery != 300 {
				t.Fatalf("class %d mastery=%d", class, mastery)
			}
		}
	}
}

func TestTestCrystal320CommitAndRollback(t *testing.T) {
	c := reportTestCatalog(t)
	for _, source := range []uint16{6017, 6018} {
		for _, fail := range []bool{false, true} {
			w, p, store := useItemWorld(model.VolatileRule{})
			w.items = c.Items
			v, err := data.LoadVolatilesWithInstances("../../data/content/volatiles.json", "../../data/content/instances.json", "../../data/spawns/instances", c.Items, c.Skills)
			if err != nil {
				t.Fatal(err)
			}
			w.volatiles = v
			var armor model.Item
			for id, def := range c.Items {
				if def.Pos == 4 {
					armor = model.Item{Index: id, UID: "test-armor", Eff: [6]byte{118, 9, 2, 5, 60, 6}}
					break
				}
			}
			p.Char.Equip[2] = armor
			p.Char.Inv[0] = model.Item{Index: source, UID: "test-crystal"}
			if fail {
				store.err = errors.New("rejected crystal")
			}
			packet := useItemPacket(0, 2)
			binary.LittleEndian.PutUint32(packet[20:24], placeEquip)
			w.onUseItem(p.Session, packet)
			if fail {
				if p.Char.Equip[2] != armor || p.Char.Inv[0].Index != source {
					t.Fatal("crystal rejection changed items")
				}
			} else {
				want := byte(2)
				if source == 6018 {
					want = 60
				}
				if p.Char.Inv[0].Index != 0 || p.Char.Equip[2].UID != armor.UID || p.Char.Equip[2].Eff != ([6]byte{118, 9, want, 160, want, 160}) {
					t.Fatalf("crystal=%d produced %v", source, p.Char.Equip[2])
				}
			}
		}
	}
}

func TestTestCrystalsRejectNonArmorAndPreserveIdentity(t *testing.T) {
	c := reportTestCatalog(t)
	w := &World{items: c.Items}
	for _, id := range []uint16{6017, 6018} {
		item := model.Item{Index: 6016, UID: "amulet", Eff: [6]byte{43, 9, 2, 5, 60, 6}}
		before := item
		if _, err := w.applyRepliction(&item, c.Items[6016], id); err == nil || item != before {
			t.Fatal("test crystal changed an accessory")
		}
	}
}

func TestAuraOfLifeCannotResurrectMobVictim(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	setPlayerCurHP(p.Char, 0)
	p.DeadAt = w.now()
	p.Char.Affects[0] = model.Affect{Type: 17, Level: 100, Value: 100, ExpiresAt: w.now().Add(time.Minute)}
	oldDeath := p.DeadAt
	for i := 0; i < 3; i++ {
		w.tickPlayerAffects(w.now().Add(time.Duration(i) * 8 * time.Second))
		w.tickPlayerRegen(w.now().Add(time.Duration(i) * 8 * time.Second))
		if playerCurHP(p.Char) != 0 || p.DeadAt != oldDeath {
			t.Fatal("periodic recovery resurrected a dead character")
		}
	}
	setPlayerCurHP(p.Char, 1)
	p.Char.Affects[0].NextTick = time.Time{}
	w.tickPlayerAffects(w.now())
	if playerCurHP(p.Char) <= 1 {
		t.Fatal("Aura stopped healing living characters")
	}
}

func TestEighthSkillsCanHitEnemySummon(t *testing.T) {
	catalog, err := data.LoadCatalog(filepath.Join("..", "..", "data", "catalog", "itemlist.csv"), filepath.Join("..", "..", "data", "catalog", "Itemname.csv"), filepath.Join("..", "..", "data", "catalog", "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	for _, index := range []int{23, 95} {
		for _, pk := range []bool{true, false} {
			w, _, attacker, summon := bmSummonPvPWorld(t, 2000)
			attacker.Char.Class = byte(index / 24)
			attacker.Char.LearnedSkill = 1 << uint(index%24)
			attacker.Char.Score.MaxMP, attacker.Char.Score.CurMP = 10000, 10000
			attacker.Char.Score.Attack, attacker.Char.Score.Str = 1000, 1000
			applyScore(attacker.Char)
			attacker.PKMode = pk
			w.rng = fixedRNG{value: 0}
			w.skills = catalog.Skills
			before := summon.HP
			w.onSkillAttack(attacker, skillCastRequest{Skill: index, TargetID: summon.ID})
			if pk && summon.HP >= before {
				t.Fatalf("skill %d did not damage authorized summon", index)
			}
			if !pk && (summon.HP != before || playerCurMP(attacker.Char) != 10000) {
				t.Fatalf("skill %d bypassed PK rules", index)
			}
		}
	}
}

func TestNewCelestialEmptyShortcutEncoding(t *testing.T) {
	for class := byte(0); class < 4; class++ {
		ch := &model.Char{Class: class, LearnedSkill: celestialSoulBit, ShortSkill: emptyShortSkills()}
		filterShortSkills(ch)
		for _, v := range ch.ShortSkill {
			if v != 0xff {
				t.Fatalf("class %d has an unlearned shortcut %d", class, v)
			}
		}
	}
}

func TestCriticalTestCrystal320SettlesAfterAcknowledgement(t *testing.T) {
	for _, rejected := range []bool{false, true} {
		w, p, _ := handlerTestWorld(t)
		c := reportTestCatalog(t)
		w.items = c.Items
		var armor model.Item
		for id, def := range c.Items {
			if def.Pos == 4 {
				armor = model.Item{Index: id, UID: "armor", Eff: [6]byte{43, 9, 2, 5, 60, 6}}
				break
			}
		}
		p.Char.Equip[2] = armor
		p.Char.Inv[0] = model.Item{Index: 6017, UID: "crystal"}
		claimCriticalTestPlayers(t, w, p)
		gate, release := criticalGate(t)
		backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
		if rejected {
			backend.failure = criticalCertifiedRejection(t)
		}
		queue := newCriticalTestQueue(t, backend, 1, 2)
		t.Cleanup(release)
		w.criticalWrites = newCriticalPersistenceOwner(queue)
		req := useItemRequest{dstType: placeEquip, dstPos: 2}
		rule := model.VolatileRule{Action: "repliction", Consume: true}
		if err := w.queueEquipmentConsumable(p, 0, req, rule); err != nil {
			t.Fatal(err)
		}
		proposal := awaitCriticalWrite(t, backend.entered)
		if proposal.accounts[0].Chars[0].Equip[2].Eff != ([6]byte{43, 9, 2, 160, 2, 160}) {
			t.Fatal("crystal proposal lost the 320 bonus")
		}
		if p.Char.Equip[2] != armor || p.Char.Inv[0].Index != 6017 {
			t.Fatal("crystal mutated live state before commit")
		}
		if err := w.queueEquipmentConsumable(p, 0, req, rule); err == nil {
			t.Fatal("pending crystal operation repeated")
		}
		release()
		awaitReadyCriticalResults(t, queue, 1)
		w.processCriticalCompletions(1)
		if rejected {
			if p.Char.Equip[2] != armor || p.Char.Inv[0].Index != 6017 {
				t.Fatal("rejected crystal changed state")
			}
		} else if p.Char.Equip[2].Eff != ([6]byte{43, 9, 2, 160, 2, 160}) || p.Char.Inv[0].Index != 0 {
			t.Fatal("crystal did not settle")
		}
	}
}
