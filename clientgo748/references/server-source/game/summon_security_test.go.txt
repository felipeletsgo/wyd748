package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestSummonCannotAttackAcrossBlockedLineOfSight(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Summoner", 10, 10)
	w := worldWithNetworkedPlayers(owner)
	w.terrain = loadedFlatTerrain()
	w.terrain.Height[10*model.TerrainWidth+12] = model.TerrainBlockedByte
	target := &Mob{ID: 1200, X: 13, Y: 10, HP: 100, GenerIndex: -1,
		Def: testNPCDef(model.Score{MaxHP: 100, Defense: 0})}
	summon := &Mob{ID: 1201, X: 10, Y: 10, HP: 100, SummonerID: owner.ID,
		SummonKind: summonKindBM, SummonRange: 6, GenerIndex: -1,
		Def: testNPCDef(model.Score{MaxHP: 100, Attack: 100})}
	w.mobs = []*Mob{target, summon}
	w.registerMobSpatial(target)
	w.registerMobSpatial(summon)
	owner.CombatTargetID = target.ID

	w.tickSummonCombat(time.Now())
	if target.HP != 100 {
		t.Fatalf("summon hit through blocked terrain: hp=%d", target.HP)
	}
}
