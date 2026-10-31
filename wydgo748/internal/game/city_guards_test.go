package game

import (
	"encoding/json"
	"os"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestCityGuardAndComposerDefinitions(t *testing.T) {
	for _, name := range []string{"Aylinde", "Guard", "Guard_"} {
		t.Run(name, func(t *testing.T) {
			raw, err := os.ReadFile("../../data/npcs/" + name + ".json")
			if err != nil {
				t.Fatal(err)
			}
			var def model.NPCDef
			if err := json.Unmarshal(raw, &def); err != nil {
				t.Fatal(err)
			}
			if def.IsMonster() || def.IsCityGuard() != (name != "Aylinde") {
				t.Fatalf("unexpected NPC role: %+v", def)
			}
		})
	}
}

func TestCityGuardTargetsMonstersNeverPlayers(t *testing.T) {
	now := time.Unix(2_000_000_000, 0)
	player, _ := networkedTestPlayer(1, "Visitor", 100, 100)
	guard := &Mob{ID: 1000, X: 100, Y: 100, HP: 100, TargetID: player.ID,
		Def: &model.NPCDef{Tipo: model.TipoNPC, CombatRole: "city_guard",
			Score: &model.Score{Attack: 10, MaxHP: 100}}}
	monster := &Mob{ID: 1001, X: 101, Y: 100, HP: 1000,
		Def: &model.NPCDef{Tipo: model.TipoMonstro, Score: &model.Score{MaxHP: 1000}}}
	w := testSpatialWorld([]*Mob{guard, monster}, player)
	w.mobsByID[guard.ID], w.mobsByID[monster.ID] = guard, monster
	w.recomputeMobActive(guard)
	if !guard.Awake {
		t.Fatal("city guard did not activate")
	}
	hp := playerCurHP(player.Char)
	w.tickMobCombat(now, 0, 1, false)
	if guard.TargetID != monster.ID {
		t.Fatalf("guard target = %d, want monster %d", guard.TargetID, monster.ID)
	}
	// Only execute the guard here; the hostile monster may independently attack.
	delete(w.activeMobs, monster.ID)
	w.tickActiveMobActions(now)
	if monster.HP >= 1000 || playerCurHP(player.Char) != hp {
		t.Fatalf("wrong damage recipient: monster HP=%d player HP=%d", monster.HP, playerCurHP(player.Char))
	}
	w.mobAttackPlayer(guard, player, now)
	w.applyMobDamageToPlayer(guard, player, 9999, now, func(uint32) []byte {
		t.Fatal("friendly NPC reached player damage publication")
		return nil
	})
	if playerCurHP(player.Char) != hp || w.playerCanInteractWithMob(player, guard) {
		t.Fatal("guard crossed the friendly NPC combat boundary")
	}
	for _, change := range []func(){
		func() { monster.SummonerID = player.ID },
		func() { monster.SummonerID = 0; monster.InstanceID = "private-room" },
		func() { monster.InstanceID = ""; monster.Def.Tipo = model.TipoNPC },
	} {
		change()
		w.tickCityGuard(guard, now, false)
		if guard.TargetID != 0 {
			t.Fatal("guard retained a summon, private monster or friendly NPC")
		}
	}
	delete(w.playersByID, player.ID)
	w.recomputeMobActive(guard)
	if !guard.Awake {
		t.Fatal("city defense incorrectly depends on a nearby player")
	}
}

func TestCompounderCannotAttackThroughStaleTarget(t *testing.T) {
	player, _ := networkedTestPlayer(1, "Visitor", 100, 100)
	compounder := &Mob{ID: 1000, X: 100, Y: 100, HP: 9000, TargetID: player.ID,
		Def: &model.NPCDef{Tipo: model.TipoNPC, Score: &model.Score{Attack: 500}}}
	w := testSpatialWorld([]*Mob{compounder}, player)
	w.activeMobs[compounder.ID] = compounder
	hp := playerCurHP(player.Char)
	w.tickActiveMobActions(time.Now())
	w.mobAttackPlayer(compounder, player, time.Now())
	if compounder.TargetID != 0 || compounder.Awake || playerCurHP(player.Char) != hp {
		t.Fatal("passive Compounder retained combat state or damaged a player")
	}
}
