package game

import (
	"bytes"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestContractTankTakesMonsterHitsUntilDeath(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 100, 100)
	other, _ := networkedTestPlayer(2, "Other", 101, 100)
	w := worldWithNetworkedPlayers(owner, other)
	w.rng = fixedRNG{value: 0}
	mustCommitContractSummon(t, w, owner, &model.VolatileSummon{Name: "Tank", Face: 207,
		HP: 15000, Defense: 100, MoveSpeed: 4, AttackRange: 3, Tank: true})
	tank := w.mobs[0]
	enemy := combatMob(tank.ID+1, tank.X, tank.Y+1)
	enemy.TargetID = other.ID
	w.appendMobInstance(enemy)
	w.registerMobSpatial(enemy)
	w.activeMobs[enemy.ID] = enemy
	now := time.Unix(100, 0)
	w.tickSummonCombat(now)
	if enemy.TargetID != other.ID {
		t.Fatal("tank redirected another player's fight")
	}
	enemy.TargetID = owner.ID
	w.tickSummonCombat(now)
	w.setMobRetaliationTarget(enemy, owner.ID)
	w.tickMobCombat(now, 0, 1, false)
	if enemy.TargetID != tank.ID {
		t.Fatal("enemy did not retain the tank while the owner attacked")
	}
	ownerHP := playerCurHP(owner.Char)
	w.tickActiveMobActions(now)
	if tank.HP >= 15000 || playerCurHP(owner.Char) != ownerHP {
		t.Fatal("monster attack did not damage the tank alone")
	}
	tank.HP = 1
	w.tickActiveMobActions(now.Add(mobAttackInterval))
	if !tank.Dead || w.summons[tank.ID] != nil || enemy.TargetID != 0 {
		t.Fatal("tank death did not release the monster")
	}
}

func TestSophiaHealsOnlyOwnerAndTanksUntilRevival(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 100, 100)
	ally, _ := networkedTestPlayer(2, "Ally", 100, 101)
	w := worldWithNetworkedPlayers(owner, ally)
	w.rng = fixedRNG{value: 0}
	w.skills = map[int]model.SkillDef{27: {Index: 27, Range: 6, Delay: 2, InstanceValue: 100}}
	now := w.now()
	setPlayerCurHP(owner.Char, 1)
	setPlayerCurHP(ally.Char, 1)
	party := &Party{}
	owner.Party, ally.Party = party, party
	mustCommitContractSummon(t, w, owner, &model.VolatileSummon{Name: "Sophia", Face: 11, Healer: true, HP: 15000, Defense: 2000})
	sophia := w.mobs[0]
	enemy := combatMob(sophia.ID+1, sophia.X, sophia.Y+1)
	enemy.Def.Score.Attack = 4000
	w.appendMobInstance(enemy)
	w.registerMobSpatial(enemy)
	enemy.Awake = false
	delete(w.activeMobs, enemy.ID)
	w.tickSummonCombat(now)
	if playerCurHP(owner.Char) <= 1 || playerCurHP(ally.Char) != 1 || enemy.TargetID == sophia.ID {
		t.Fatal("Sophia did not heal her owner alone without drawing aggro")
	}
	setPlayerCurHP(owner.Char, 0)
	w.tickSummonCombat(now.Add(time.Second))
	if enemy.TargetID != sophia.ID || playerCurHP(owner.Char) != 0 {
		t.Fatal("Sophia did not draw the monster while her owner was dead")
	}
	oldHP := sophia.HP
	w.tickActiveMobActions(now.Add(time.Second))
	if sophia.HP >= oldHP {
		t.Fatal("the monster did not damage Sophia")
	}
	setPlayerCurHP(owner.Char, 1)
	oldHP = sophia.HP
	w.tickActiveMobActions(now.Add(3 * time.Second))
	w.tickMobCombat(now.Add(3*time.Second), 0, 1, false)
	if sophia.HP != oldHP || enemy.TargetID == sophia.ID {
		t.Fatal("the monster kept targeting Sophia after the owner revived")
	}
}

func TestContractGuardTimersAndBaseStats(t *testing.T) {
	owner, ownerSession := networkedTestPlayer(1, "Owner", 100, 100)
	observer, observerSession := networkedTestPlayer(2, "Observer", 101, 100)
	w := worldWithNetworkedPlayers(owner, observer)
	checkTimerSnapshot := func(tail []byte) {
		t.Helper()
		count := 0
		for pkt, ok := ownerSession.DequeuePacketForTest(); ok; pkt, ok = ownerSession.DequeuePacketForTest() {
			if !wire.Decrypt(pkt) {
				t.Fatal("invalid owner packet checksum")
			}
			h := wire.ParseHeader(pkt)
			if h.Type != wire.OpUpdateAffect {
				continue
			}
			count++
			if h.ID != owner.ID || len(pkt) != 140+len(tail) || !bytes.Equal(pkt[140:], tail) {
				t.Fatalf("wrong owner timer snapshot: %+v %x", h, pkt)
			}
		}
		if count != 1 {
			t.Fatalf("owner received %d timer snapshots, want 1", count)
		}
		for pkt, ok := observerSession.DequeuePacketForTest(); ok; pkt, ok = observerSession.DequeuePacketForTest() {
			if !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type == wire.OpUpdateAffect {
				t.Fatal("observer received an invalid or private affect packet")
			}
		}
	}
	owner.Char.Score.Level = 399
	applyScore(owner.Char)
	now := w.now()
	tank := &model.VolatileSummon{Name: "Asder", Face: 1, Tank: true, HP: 15000, Defense: 2000,
		Weapon: 907, Shield: 907, Armor: [5]uint16{100, 101, 102, 103, 104}, Refinement: 9}
	healer := &model.VolatileSummon{Name: "Sophia", Face: 11, Healer: true, HP: 15000, Defense: 2000}
	mustCommitContractSummon(t, w, owner, tank)
	checkTimerSnapshot([]byte{0x10, 0x0e, 0, 0, 0, 0, 0, 0})
	asder := w.mobs[0]
	if got := playerAffectsPacketAt(owner, now); len(got) != 148 || got[140] != 0x10 || got[144] != 0 {
		t.Fatalf("wrong Asder timer payload: %x", got)
	}
	mustCommitContractSummon(t, w, owner, healer)
	checkTimerSnapshot([]byte{0, 0, 0, 0, 0x10, 0x0e, 0, 0})
	if len(w.summons) != 1 || !asder.Dead || w.mobByID(asder.ID) != nil {
		t.Fatal("Sophia did not replace Asder")
	}
	sophia := w.mobs[0]
	for _, m := range []*Mob{asder, sophia} {
		if m.Def.Score.Defense != 2800 || m.HP != 23000 || m.Def.Score.CurMP != m.Def.Score.MaxMP {
			t.Fatalf("wrong starting stats for %s: %+v", m.Def.Name, m.Def.Score)
		}
	}
	items := asder.Def.Equip.Items()
	for _, item := range items[1:8] {
		if _, value := item.RefinementEffect(); value != 9 {
			t.Fatalf("incorrect refinement for item %d: %d", item.Index, value)
		}
	}
	if got := playerAffectsPacketAt(owner, now); len(got) != 148 || got[140] != 0 || got[144] != 0x10 {
		t.Fatalf("wrong Sophia timer payload: %x", got)
	}
	w.destroyPlayerSummon(sophia, 0)
	checkTimerSnapshot(nil)
	if len(w.summons) != 0 || len(playerAffectsPacketAt(owner, now)) != 140 {
		t.Fatal("guard death left runtime state or an active icon")
	}
	mustCommitContractSummon(t, w, owner, tank)
	checkTimerSnapshot([]byte{0x10, 0x0e, 0, 0, 0, 0, 0, 0})
	asder = w.mobs[0]
	w.tickSummonCombat(asder.ExpiresAt)
	checkTimerSnapshot(nil)
	if !asder.Dead || len(w.summons) != 0 || len(playerAffectsPacketAt(owner, asder.ExpiresAt)) != 140 {
		t.Fatal("guard expiry left runtime state or an active icon")
	}
}

func TestAsderCastsSelfBuffsAndFreezesTauntedMonster(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 100, 100)
	w := worldWithNetworkedPlayers(owner)
	w.skills = map[int]model.SkillDef{
		3:  {Index: 3, AffectType: 24, AffectTime: 600},
		5:  {Index: 5, TickType: 17, TickValue: 75, AffectTime: 600},
		13: {Index: 13, AffectType: 14, AffectValue: 10, AffectTime: 600},
		19: {Index: 19, Range: 4, AffectType: 7, AffectValue: 1, TickType: 1, TickValue: 2, AffectTime: 2},
	}
	mustCommitContractSummon(t, w, owner, &model.VolatileSummon{Name: "Asder", Face: 1, Tank: true, HP: 15000, Defense: 2000})
	asder := w.mobs[0]
	enemy := combatMob(asder.ID+1, asder.X, asder.Y+1)
	w.appendMobInstance(enemy)
	w.registerMobSpatial(enemy)
	now := w.now()
	for i := 0; i < 4; i++ {
		w.tickSummonCombat(now.Add(time.Duration(i) * 2 * time.Second))
	}
	afterCasts := now.Add(6 * time.Second)
	if activeMobAffectAt(asder, 17, afterCasts) == nil ||
		effectiveMobDefenseAt(asder, afterCasts) <= int(asder.Def.Score.Defense) ||
		guardMaxHPAt(asder, afterCasts) <= asder.Def.Score.MaxHP {
		t.Fatal("Asder did not gain Samaritan, Aura of Life and Growth")
	}
	if enemy.TargetID != asder.ID || effectiveMobAttackRunAt(enemy, afterCasts) >= 4 {
		t.Fatal("the taunted enemy did not slow down")
	}
	asder.HP -= 100
	hp := asder.HP
	w.tickSummonCombat(now.Add(10 * time.Second))
	if asder.HP <= hp {
		t.Fatal("Aura of Life did not restore Asder's HP")
	}
}

func TestSophiaRejectsPvPWhileHerOwnerLives(t *testing.T) {
	w, owner, attacker, summon := bmSummonPvPWorld(t, 2000)
	summon.SummonKind = summonKindContract
	summon.guardKind = guardSophia
	if w.playerCanInteractWithMob(attacker, summon) {
		t.Fatal("PK opponent can attack Sophia while her owner is alive")
	}
	setPlayerCurHP(owner.Char, 0)
	if !w.playerCanInteractWithMob(attacker, summon) {
		t.Fatal("normal PvP rules did not resume when Sophia became vulnerable")
	}
}
