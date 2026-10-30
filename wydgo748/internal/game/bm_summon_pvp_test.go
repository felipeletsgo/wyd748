package game

import (
	"testing"

	"wydgo/internal/model"
)

// pvpTestTerrain marks x < 2100 as a PK cell (0x40) and the rest as a safe
// cell (0x01), on otherwise open ground.
func pvpTestTerrain() model.TerrainMap {
	m := model.TerrainMap{
		Height:    make([]byte, model.TerrainCells),
		Attribute: make([]byte, model.AttributeCells),
	}
	for i := range m.Attribute {
		if i%model.AttributeWidth < 2100/4 {
			m.Attribute[i] = 0x40
		} else {
			m.Attribute[i] = 0x01
		}
	}
	return m
}

func bmSummonPvPWorld(t *testing.T, x uint16) (*World, *Player, *Player, *Mob) {
	t.Helper()
	owner, _ := networkedTestPlayer(1, "Owner", x, 2000)
	attacker, _ := networkedTestPlayer(2, "Attacker", x+1, 2000)
	w := worldWithNetworkedPlayers(owner, attacker)
	w.terrain = pvpTestTerrain()
	owner.PKMode, attacker.PKMode = true, true
	summon := &Mob{ID: 1500, X: x, Y: 2001, HP: 500, GenerIndex: -1, SummonerID: owner.ID,
		SummonKind: summonKindBM, SummonRange: mobAttackRange,
		Def: testNPCDef(model.Score{MaxHP: 500, CurHP: 500})}
	w.appendMobInstance(summon)
	w.registerMobSpatial(summon)
	owner.show(summon.ID)
	attacker.show(summon.ID)
	return w, owner, attacker, summon
}

func TestOpponentCanAttackBMSummonUnderPvPRules(t *testing.T) {
	w, owner, attacker, summon := bmSummonPvPWorld(t, 2000)
	if !w.playerCanInteractWithMob(attacker, summon) {
		t.Fatal("PK opponent in a PK cell cannot attack the summon")
	}
	if w.playerCanInteractWithMob(owner, summon) {
		t.Fatal("owner can attack its own summon")
	}
	attacker.PKMode = false
	if w.playerCanInteractWithMob(attacker, summon) {
		t.Fatal("opponent without PK mode can attack the summon")
	}
	attacker.PKMode = true
	party := &Party{Members: []*Player{owner, attacker}}
	owner.Party, attacker.Party = party, party
	if w.playerCanInteractWithMob(attacker, summon) {
		t.Fatal("party member can attack the summon")
	}
	if w.mobCanTargetPlayer(summon, attacker) {
		t.Fatal("ordinary mob AI may target with a player summon")
	}
}

func TestBMSummonCannotBeAttackedInSafeCell(t *testing.T) {
	w, _, attacker, summon := bmSummonPvPWorld(t, 2200)
	if w.playerCanInteractWithMob(attacker, summon) {
		t.Fatal("summon in a safe cell was attackable")
	}
}

func TestKilledBMSummonGrantsNoRewardAndDisappears(t *testing.T) {
	w, _, attacker, summon := bmSummonPvPWorld(t, 2000)
	summon.Def.ExpReward = 5000
	exp := attacker.Char.Exp
	summon.HP = 0
	if plan := w.planMobKill(attacker, summon, 500, 500); plan != nil {
		t.Fatal("summon kill produced a reward plan")
	}
	if attacker.Char.Exp != exp {
		t.Fatalf("summon kill changed EXP %d -> %d", exp, attacker.Char.Exp)
	}
	if w.mobsByID[summon.ID] != nil || w.summons[summon.ID] != nil || !summon.Dead {
		t.Fatal("killed summon stayed in the world")
	}
}

func TestBMSummonDoesNotAttackPlayersInSafeCell(t *testing.T) {
	for _, tc := range []struct {
		name string
		x    uint16
		want bool
	}{{"pk cell", 2000, true}, {"safe cell", 2200, false}} {
		t.Run(tc.name, func(t *testing.T) {
			w, owner, attacker, _ := bmSummonPvPWorld(t, tc.x)
			if got := w.summonTarget(owner, attacker.ID).user != nil; got != tc.want {
				t.Fatalf("summon target accepted=%v, want %v", got, tc.want)
			}
		})
	}
}

func TestPhysicalAttackDamagesOpponentBMSummon(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.rng = fixedRNG{value: 0}
	p.PKMode = true
	p.Char.Score.Attack, p.Char.Score.Accuracy = 1000, 1000
	applyScore(p.Char)
	owner, _ := networkedTestPlayer(p.ID+1, "Owner", p.X+2, p.Y)
	w.players[owner.Session] = owner
	w.playersByID[owner.ID] = owner
	summon := &Mob{ID: 1501, X: p.X + 1, Y: p.Y, HP: 100_000, GenerIndex: -1, SummonerID: owner.ID,
		SummonKind: summonKindBM, Def: testNPCDef(model.Score{MaxHP: 100_000, CurHP: 100_000})}
	w.appendMobInstance(summon)
	w.registerMobSpatial(summon)
	p.show(summon.ID)
	w.onAttack(p.Session, physicalAttackPacket(1000, summon.ID, summon.X, summon.Y))
	if summon.HP >= 100_000 {
		t.Fatal("physical attack did not damage the opponent's summon")
	}
}

// Native AI attacks between players and summons deal 30% (TMSRVIN sub_4DECC0).
func TestBMSummonPvPHitUsesNativeThirtyPercent(t *testing.T) {
	w, owner, attacker, summon := bmSummonPvPWorld(t, 2000)
	w.rng = fixedRNG{value: 0}
	summon.Def.Score.Attack, summon.Def.Score.Accuracy = 2000, 1000
	summon.X, summon.Y = attacker.X, attacker.Y+1
	w.registerMobSpatial(summon)
	owner.CombatTargetID = attacker.ID
	full := mobPhysicalHitPlayerAt(summon, attacker.Char, fixedRNG{value: 0}.Intn, w.now()).Damage
	if full < 10 {
		t.Fatalf("fixture damage too small: %d", full)
	}
	hp := playerCurHP(attacker.Char)
	w.tickSummonCombat(w.now())
	if got := hp - playerCurHP(attacker.Char); got != full*3/10 {
		t.Fatalf("summon PvP damage=%d, want 30%% of %d", got, full)
	}
}
