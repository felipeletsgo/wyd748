package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestCombatSummonRealPoisonPacketAndReplay(t *testing.T) {
	catalog, _ := fairyDustCatalog(t)
	for _, kind := range []byte{summonKindBM, summonKindContract} {
		t.Run(fmt.Sprint(kind), func(t *testing.T) {
			w, _, caster, target := bmSummonPvPWorld(t, 2000)
			w.items, w.skills, w.rng = catalog.Items, catalog.Skills, fixedRNG{}
			clock := newFakeClock(time.Unix(1800000000, 0))
			w.clock = clock
			caster.Char.Class, caster.Char.LearnedSkill = 1, 1<<(40-24)
			caster.Char.Score.Mastery[3] = 100
			w.recalcPlayer(caster.Char)
			setPlayerCurMP(caster.Char, playerMaxMP(caster.Char))
			target.SummonKind, target.HP, target.Def.Score.MaxHP = kind, 100000, 100000
			packet := inboundPacket(wire.OpAttackOne, 48)
			binary.LittleEndian.PutUint32(packet[8:12], 1000)
			binary.LittleEndian.PutUint16(packet[20:22], target.X)
			binary.LittleEndian.PutUint16(packet[22:24], target.Y)
			binary.LittleEndian.PutUint16(packet[24:26], 40)
			binary.LittleEndian.PutUint16(packet[44:46], target.ID)
			packet[28] = 5
			mana := playerCurMP(caster.Char)
			if !w.validateInboundCommand(caster.Session, packet) {
				t.Fatal("real Poison request failed ingress validation")
			}
			w.onAttack(caster.Session, packet)
			affect := activeMobAffectAt(target, 20, clock.Now())
			if affect == nil || affect.OwnerCharacterUID != caster.Char.UID || playerCurMP(caster.Char) >= mana {
				t.Fatal("real Poison did not charge mana and bind its summon effect")
			}
			before, hp := accountStateSnapshot(caster.Account), target.HP
			affects := target.Affects
			w.onAttack(caster.Session, packet)
			if !reflect.DeepEqual(before, accountStateSnapshot(caster.Account)) || hp != target.HP || affects != target.Affects {
				t.Fatal("replayed Poison request changed resources or summon state")
			}
			clock.Advance(8 * time.Second)
			w.tickMobAffects(clock.Now(), 0, 1)
			want := uint32(affect.Level/2 + affect.Value)
			if target.HP != hp-want {
				t.Fatalf("packet-applied Poison dealt %d, want %d", hp-target.HP, want)
			}
		})
	}
}

func TestCombatSummonPoisonPrivateRuntimeEntryAndExit(t *testing.T) {
	w, owner, caster, target := bmSummonPvPWorld(t, 2000)
	now := time.Unix(1800000000, 0)
	w.clock = newFakeClock(now)
	inst := privateTestInstance("summon-room", owner.ID)
	inst.Config.Stages[0].X, inst.Config.Stages[0].Y = 2000, 2000
	inst.MemberIDs = []uint16{owner.ID, caster.ID}
	w.itemInstances = map[string]*ItemInstance{inst.RuntimeID: inst}
	target.InstanceID = inst.RuntimeID
	applySummonTestPoison(w, caster, target)
	target.Affects[0].NextTick = now
	w.tickMobAffects(now, 0, 1)
	if target.HP != 390 {
		t.Fatal("valid private-runtime Poison did not damage the enemy summon")
	}
	inst.MemberIDs = []uint16{owner.ID}
	drainPackets(caster)
	w.tickMobAffects(now.Add(8*time.Second), 0, 1)
	if target.HP != 390 || target.Affects[0] != (model.Affect{}) {
		t.Fatal("Poison survived its caster leaving the runtime")
	}
	if caster.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("private summon update leaked to its departed caster")
	}
}

func TestCombatSummonPoisonDoesNotCatchUpMissedTicks(t *testing.T) {
	w, _, caster, target := bmSummonPvPWorld(t, 2000)
	now := time.Unix(1800000000, 0)
	w.clock = newFakeClock(now)
	applySummonTestPoison(w, caster, target)
	for i := 0; i < 100; i++ {
		w.tickMobAffects(now.Add(20*time.Second), 0, 1)
	}
	if target.HP != 390 {
		t.Fatal("delayed or repeated actor ticks burst multiple poison hits")
	}
	w.tickMobAffects(now.Add(24*time.Second), 0, 1)
	if target.HP != 390 || target.Affects[0].Type != 0 {
		t.Fatal("expiration applied a deferred poison hit")
	}
}

func TestCombatSummonPoisonMassDeathAcrossShards(t *testing.T) {
	w, owner, caster, _ := bmSummonPvPWorld(t, 2000)
	store := &craftStore{}
	w.store = store
	now := w.now()
	var targets []*Mob
	for i := 0; i < 24; i++ {
		kind := summonKindBM
		if i%2 != 0 {
			kind = summonKindContract
		}
		m := &Mob{ID: uint16(1600 + i), X: 2000, Y: 2001, HP: 50,
			SummonerID: owner.ID, SummonKind: kind, GenerIndex: -1,
			Def: testNPCDef(model.Score{MaxHP: 50})}
		w.appendMobInstance(m)
		w.registerMobSpatial(m)
		owner.show(m.ID)
		caster.show(m.ID)
		applySummonTestPoison(w, caster, m)
		m.Affects[0].NextTick = now
		targets = append(targets, m)
	}
	drainPackets(owner)
	drainPackets(caster)
	before := accountStateSnapshot(caster.Account)
	for shard := 0; shard < 4; shard++ {
		w.tickMobAffects(now, shard, 4)
		for _, target := range targets {
			wantDead := int(target.ID)%4 <= shard
			if target.Dead != wantDead || (w.mobsByID[target.ID] == nil) != wantDead ||
				owner.hasVisible(target.ID) == wantDead || caster.hasVisible(target.ID) == wantDead {
				t.Fatalf("shard %d lost removal ownership for summon %d", shard, target.ID)
			}
		}
	}
	if len(w.summons) != 1 || store.saves != 0 || len(w.groundItems) != 0 ||
		!reflect.DeepEqual(before, accountStateSnapshot(caster.Account)) {
		t.Fatal("mass summon death changed account state or removed an unrelated summon")
	}
}

func TestCombatSummonPoisonAfterKillWriteFailure(t *testing.T) {
	for _, unknown := range []bool{false, true} {
		t.Run(fmt.Sprintf("unknown-%t", unknown), func(t *testing.T) {
			w, owner, caster, target := bmSummonPvPWorld(t, 2000)
			gate, release := criticalGate(t)
			failure := criticalCertifiedRejection(t)
			if unknown {
				failure = errors.New("injected unknown kill outcome")
			}
			backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate, failure: failure}
			queue := newCriticalTestQueue(t, &criticalBatchTestStore{backend}, 1, 2)
			t.Cleanup(release)
			w.criticalWrites, w.rng = newCriticalPersistenceOwner(queue), fixedRNG{}
			claimCriticalTestPlayers(t, w, owner, caster)
			now := w.now()
			monster := &Mob{ID: 1502, X: caster.X, Y: caster.Y, HP: 100, GenerIndex: -1,
				Def: testNPCDef(model.Score{MaxHP: 100})}
			monster.Def.ExpReward = 1000
			w.appendMobInstance(monster)
			w.registerMobSpatial(monster)
			for _, mob := range []*Mob{target, monster} {
				applySummonTestPoison(w, caster, mob)
				mob.Affects[0].NextTick = now
			}
			before := accountStateSnapshot(caster.Account)
			w.tickMobAffects(now, 0, 1)
			awaitCriticalWrite(t, backend.entered)
			if target.HP != 500 {
				t.Fatal("pending kill allowed summon damage")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			w.tickMobAffects(now, 0, 1)
			if unknown {
				if target.HP != 500 || !(caster.PersistencePoisoned || caster.Session.IsClosed() || w.shuttingDown) {
					t.Fatal("uncertain reward outcome escaped containment through summon effects")
				}
			} else if target.HP != 390 || monster.HP != 100 || monster.Dead ||
				!reflect.DeepEqual(before, accountStateSnapshot(caster.Account)) || w.criticalPlayerReserved(caster) {
				t.Fatal("certified rejection failed rollback or prevented independent summon effects from resuming")
			}
		})
	}
}

func FuzzSummonPvPDamageRationalBound(f *testing.F) {
	for _, seed := range []uint32{0, 1, 4, 10, 1431655765, 1431655766, model.MaxScoreValue, ^uint32(0)} {
		f.Add(seed)
	}
	f.Fuzz(func(t *testing.T, damage uint32) {
		got := summonPvPDamage(damage)
		product, scaled := uint64(damage)*3, uint64(got)*10
		if got > damage || scaled > product || product-scaled >= 10 {
			t.Fatalf("30%% rational bound failed: input=%d result=%d", damage, got)
		}
		if damage != ^uint32(0) && summonPvPDamage(damage+1) < got {
			t.Fatal("damage factor is not monotonic")
		}
	})
}
