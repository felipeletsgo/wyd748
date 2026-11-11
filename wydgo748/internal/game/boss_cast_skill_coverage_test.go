package game

import (
	"testing"

	"wydgo/internal/model"
)

func bossCastSkillFixture(t *testing.T, x, y uint16) (*World, *Mob, *BossRuntime, *Player) {
	t.Helper()
	w, mob, boss, _ := newBossTestWorld(t, testBossProfile(t), 1000)
	player, _ := networkedTestPlayer(1, "BossTarget", x, y)
	w.players[player.Session] = player
	w.playersByID[player.ID] = player
	w.updatePlayerSpatial(player)
	player.show(mob.ID)
	mob.Def.Score.MagicAttack = 1200
	return w, mob, boss, player
}

func TestBossCastSkillRejectsInvalidTargetRangeLineOfSightAndMissingSkill(t *testing.T) {
	tests := []struct {
		name  string
		x, y  uint16
		setup func(*World, *Player)
		act   BossActionDef
	}{
		{
			name: "target outside cast range", x: 2106, y: 2100,
			act:   BossActionDef{SkillID: 26, CastRange: 2},
			setup: func(w *World, _ *Player) { w.skills = map[int]model.SkillDef{26: {Index: 26}} },
		},
		{
			name: "blocked line of sight", x: 2102, y: 2100,
			act: BossActionDef{SkillID: 26, CastRange: 4},
			setup: func(w *World, _ *Player) {
				w.skills = map[int]model.SkillDef{26: {Index: 26}}
				terrain := model.TerrainMap{
					Height:    make([]byte, model.TerrainCells),
					Attribute: make([]byte, model.AttributeCells),
				}
				terrain.Height[2100*model.TerrainWidth+2101] = model.TerrainBlockedByte
				w.terrain = terrain
			},
		},
		{
			name: "skill missing from catalog", x: 2101, y: 2100,
			act: BossActionDef{SkillID: 26},
		},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			w, mob, boss, player := bossCastSkillFixture(t, test.x, test.y)
			if test.setup != nil {
				test.setup(w, player)
			}
			before := playerCurHP(player.Char)
			w.bossCastSkill(boss, mob, test.act, BossEvent{SourceID: player.ID})
			if got := playerCurHP(player.Char); got != before {
				t.Fatalf("rejected cast changed HP: %d -> %d", before, got)
			}
		})
	}
}

func TestBossCastSkillUsesSkillDataAndResolvesHitOrMiss(t *testing.T) {
	for _, test := range []struct {
		name string
		hit  bool
	}{
		{name: "hit", hit: true},
		{name: "miss"},
	} {
		t.Run(test.name, func(t *testing.T) {
			w, mob, boss, player := bossCastSkillFixture(t, 2101, 2100)
			player.Char.Score.Dex = 4000 // The minimum hit chance makes the miss deterministic.
			applyScore(player.Char)
			w.skills = map[int]model.SkillDef{26: {
				Index: 26, Name: "Test Skill", InstanceValue: 100, InstanceType: 1,
			}}
			if test.hit {
				w.rng = fixedRNG{value: 0}
			} else {
				w.rng = fixedRNG{value: 99}
			}
			before := playerCurHP(player.Char)
			action := BossActionDef{SkillID: 26, Message: "Boss skill test"}
			w.bossCastSkill(boss, mob, action, BossEvent{SourceID: player.ID})
			after := playerCurHP(player.Char)
			if test.hit && after >= before {
				t.Fatalf("successful skill did not damage target: HP %d -> %d", before, after)
			}
			if !test.hit && after != before {
				t.Fatalf("miss changed target HP: %d -> %d", before, after)
			}
			if got := player.Session.QueuedPacketsForTest(); got < 2 {
				t.Fatalf("cast did not publish its notice and result: packets=%d", got)
			}
		})
	}
}
