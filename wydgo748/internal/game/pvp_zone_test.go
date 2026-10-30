package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

// pvpZoneWorld puts the attacker at x=2000 (PK cell) and the target at the
// requested x; pvpTestTerrain marks x >= 2100 as a safe cell.
func pvpZoneWorld(t *testing.T, attackerX, targetX uint16) (*World, *Player, *Player) {
	t.Helper()
	attacker, _ := networkedTestPlayer(1, "Attacker", attackerX, 2000)
	target, _ := networkedTestPlayer(2, "Target", targetX, 2000)
	w := worldWithNetworkedPlayers(attacker, target)
	w.terrain = pvpTestTerrain()
	w.rng = fixedRNG{value: 0}
	attacker.PKMode = true
	attacker.Char.Score.Attack, attacker.Char.Score.Accuracy = 1000, 1000
	applyScore(attacker.Char)
	attacker.show(target.ID)
	return w, attacker, target
}

func TestServerPvPAuthorityRequiresBothPlayersInPKCells(t *testing.T) {
	for _, tc := range []struct {
		name              string
		attackerX, target uint16
		want              bool
	}{
		{"both in PK cells", 2096, 2097, true},
		{"target in safe cell", 2099, 2100, false},
		{"attacker in safe cell", 2100, 2099, false},
		{"both in safe cells", 2200, 2201, false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, attacker, target := pvpZoneWorld(t, tc.attackerX, tc.target)
			if got := w.canInitiatePvP(attacker, target); got != tc.want {
				t.Fatalf("canInitiatePvP=%v, want %v", got, tc.want)
			}
		})
	}
}

// A modified client may send an attack in a safe area; the server ignores it.
func TestPhysicalPvPAttackInSafeCellChangesNothing(t *testing.T) {
	for _, tc := range []struct {
		name       string
		x          uint16
		wantDamage bool
	}{{"pk cell", 2090, true}, {"safe cell", 2200, false}} {
		t.Run(tc.name, func(t *testing.T) {
			w, attacker, target := pvpZoneWorld(t, tc.x, tc.x+1)
			hp := playerCurHP(target.Char)
			w.onAttack(attacker.Session, physicalAttackPacket(1000, target.ID, target.X, target.Y))
			if damaged := playerCurHP(target.Char) < hp; damaged != tc.wantDamage {
				t.Fatalf("damaged=%v, want %v (HP %d -> %d)", damaged, tc.wantDamage, hp, playerCurHP(target.Char))
			}
		})
	}
}

func TestPlayerPoisonDealsNoDamageInSafeCell(t *testing.T) {
	for _, tc := range []struct {
		name       string
		x          uint16
		wantDamage bool
	}{{"pk cell", 2090, true}, {"safe cell", 2200, false}} {
		t.Run(tc.name, func(t *testing.T) {
			w, owner, target := pvpZoneWorld(t, 2000, tc.x)
			target.Char.Affects[1] = model.Affect{Type: 20, Level: 20, Value: 10, OwnerID: owner.ID,
				OwnerCharacterUID: owner.Char.UID, ExpiresAt: w.now().Add(time.Minute), NextTick: w.now().Add(-time.Second)}
			hp := playerCurHP(target.Char)
			w.tickPlayerAffects(w.now())
			if damaged := playerCurHP(target.Char) < hp; damaged != tc.wantDamage {
				t.Fatalf("poison damaged=%v, want %v", damaged, tc.wantDamage)
			}
		})
	}
}
