package game

import (
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

// W2PP ProcessAffect Type 22: the Lightning Storm tick hits as a Thunderbolt,
// not with the bare affect value (Level + Value).
func TestLightningStormTickHitsAsThunderbolt(t *testing.T) {
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	w, p, _ := handlerTestWorld(t)
	w.skills = catalog.Skills
	p.Char.Class = 1
	p.Char.Score.Level = 300
	p.Char.Score.Int = 2000
	p.Char.Score.Mastery = [4]uint32{0, 0, 200, 0}
	applyScore(p.Char)
	w.recalcPlayer(p.Char)
	affect := &model.Affect{Type: 22, Level: 200, Value: 100}
	mob := &Mob{ID: 1600, X: p.X + 1, Y: p.Y, HP: 1_000_000, GenerIndex: -1,
		Def: testNPCDef(model.Score{MaxHP: 1_000_000, CurHP: 1_000_000, MaxMP: 10})}
	w.mobs = append(w.mobs, mob)
	w.registerMobSpatial(mob)
	p.show(mob.ID)

	w.tickAreaDamageAffect(p, affect, 37)
	dealt := 1_000_000 - mob.HP
	if dealt <= uint32(affect.Level+affect.Value) {
		t.Fatalf("storm tick dealt %d, no more than the bare affect value %d", dealt, affect.Level+affect.Value)
	}
}
