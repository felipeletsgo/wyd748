package game

import (
	"encoding/binary"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestCombatSummonPvPRequiresSourceCell(t *testing.T) {
	for _, targetKind := range []string{"player", "BM", "contract", "monster"} {
		for _, safe := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s/safe-%t", targetKind, safe), func(t *testing.T) {
				w, targetOwner, attacker, target := bmSummonPvPWorld(t, 2096)
				w.rng = fixedRNG{}
				target.X, target.Y, target.HP = 2099, 2000, 100000
				target.Def.Score.MaxHP = target.HP
				if targetKind == "contract" {
					target.SummonKind = summonKindContract
				} else if targetKind == "monster" {
					target.SummonerID, target.SummonKind = 0, 0
					delete(w.summons, target.ID)
				}
				w.registerMobSpatial(target)
				source := &Mob{ID: 1501, X: 2098, Y: 2000, HP: 10000, GenerIndex: -1,
					SummonerID: attacker.ID, SummonKind: summonKindBM, SummonRange: 6,
					Def: testNPCDef(model.Score{MaxHP: 10000, Attack: 1000, Accuracy: 1000})}
				if safe {
					source.X = 2100
				}
				w.appendMobInstance(source)
				w.registerMobSpatial(source)
				attacker.CombatTargetID = target.ID
				before := target.HP
				if targetKind == "player" {
					attacker.CombatTargetID, before = targetOwner.ID, playerCurHP(targetOwner.Char)
				}
				w.tickSummonCombat(w.now())
				after := target.HP
				if targetKind == "player" {
					after = playerCurHP(targetOwner.Char)
				}
				if wantDamage := !safe || targetKind == "monster"; (after < before) != wantDamage {
					t.Fatalf("HP %d -> %d, want damage=%t", before, after, wantDamage)
				}
				if safe && targetKind != "monster" && !source.NextAttack.IsZero() {
					t.Fatal("rejected PvP hit consumed the summon attack clock")
				}
			})
		}
	}
}

func TestSummonPvPDamageFullRange(t *testing.T) {
	for _, input := range []uint32{0, 1, 3, 4, 9, 10, 100, 1431655765, 1431655766, model.MaxScoreValue, ^uint32(0)} {
		want := uint32(uint64(input) * 3 / 10)
		if got := summonPvPDamage(input); got != want {
			t.Fatalf("30%% of %d = %d, want %d", input, got, want)
		}
	}
}

func applySummonTestPoison(w *World, caster *Player, target *Mob) {
	w.applySkillMobEffects(caster, target, model.SkillDef{
		AffectType: 20, AffectValue: 100, AffectTime: 3,
	}, 20)
}

func requireSummonPacket(t *testing.T, p *Player, opcode, entity uint16) []byte {
	t.Helper()
	for packet, ok := p.Session.DequeuePacketForTest(); ok; packet, ok = p.Session.DequeuePacketForTest() {
		if !wire.Decrypt(packet) {
			t.Fatal("invalid summon packet")
		}
		if h := wire.ParseHeader(packet); h.Type == opcode && h.ID == entity {
			return packet
		}
	}
	t.Fatalf("missing packet 0x%X for summon %d", opcode, entity)
	return nil
}

func TestCombatSummonPoisonTicksExpiresAndPublishes(t *testing.T) {
	for _, kind := range []byte{summonKindBM, summonKindContract} {
		t.Run(fmt.Sprint(kind), func(t *testing.T) {
			w, owner, attacker, target := bmSummonPvPWorld(t, 2000)
			target.SummonKind = kind
			now := time.Unix(1800000000, 0)
			clock := newFakeClock(now)
			w.clock = clock
			applySummonTestPoison(w, attacker, target)
			drainPackets(owner)
			drainPackets(attacker)
			for _, seconds := range []int{8, 16} {
				clock.current = now.Add(time.Duration(seconds) * time.Second)
				before := target.HP
				w.tickMobAffects(clock.Now(), 0, 1)
				if target.HP != before-110 || w.activeMobs[target.ID] != nil || target.Awake {
					t.Fatal("summon poison failed or enabled hostile-mob AI")
				}
				for _, viewer := range []*Player{owner, attacker} {
					packet := requireSummonPacket(t, viewer, wire.OpSetHpMp, target.ID)
					if binary.LittleEndian.Uint32(packet[12:16]) != target.HP {
						t.Fatal("observer received a different poison HP value")
					}
				}
				w.tickMobAffects(clock.Now(), 0, 1)
				if target.HP != before-110 {
					t.Fatal("repeated tick applied poison twice")
				}
			}
			clock.current = now.Add(24 * time.Second)
			w.tickMobAffects(clock.Now(), 0, 1)
			if target.HP != 280 || target.Affects[0] != (model.Affect{}) {
				t.Fatal("expired poison damaged the summon or retained its source")
			}
			for _, viewer := range []*Player{owner, attacker} {
				packet := requireSummonPacket(t, viewer, wire.OpUpdateScore, target.ID)
				if binary.LittleEndian.Uint16(packet[152:154]) != 0 {
					t.Fatal("expired poison remained in an observer snapshot")
				}
			}
			owner.hide(target.ID)
			w.showMob(owner, target)
			packet := requireSummonPacket(t, owner, wire.OpCreateMob, wire.SceneField)
			if binary.LittleEndian.Uint16(packet[70:72]) != 0 {
				t.Fatal("view reentry restored expired poison")
			}
		})
	}
}

func TestCombatSummonPoisonRejectsInvalidContext(t *testing.T) {
	for _, scenario := range []string{"safe-target", "safe-owner", "safe-caster", "PK-off", "party", "caster-runtime", "owner-runtime", "reused-identity", "other-shard", "mount", "thorn-wall"} {
		t.Run(scenario, func(t *testing.T) {
			w, owner, attacker, target := bmSummonPvPWorld(t, 2096)
			now := w.now()
			applySummonTestPoison(w, attacker, target)
			target.Affects[0].NextTick = now
			shard, shardCount := 0, 1
			switch scenario {
			case "safe-target":
				target.X = 2100
			case "safe-owner":
				owner.X = 2100
			case "safe-caster":
				attacker.X = 2100
			case "PK-off":
				attacker.PKMode = false
			case "party":
				party := &Party{Members: []*Player{owner, attacker}}
				owner.Party, attacker.Party = party, party
			case "caster-runtime", "owner-runtime":
				id := attacker.ID
				if scenario == "owner-runtime" {
					id = owner.ID
				}
				w.itemInstances = map[string]*ItemInstance{"foreign": privateTestInstance("foreign", id)}
			case "reused-identity":
				w.unindexPlayerCharacter(attacker)
				attacker.Char.UID += "-replacement"
			case "other-shard":
				shard, shardCount = 1, 2 // Target ID 1500 belongs to shard 0.
			case "mount":
				target.SummonKind = summonKindMount
			case "thorn-wall":
				target.SummonKind = summonKindThornWall
			}
			w.tickMobAffects(now, shard, shardCount)
			if target.HP != 500 || target.Dead {
				t.Fatal("invalid summon poison context changed HP")
			}
			if (scenario == "caster-runtime" || scenario == "reused-identity") && target.Affects[0].Type != 0 {
				t.Fatal("stale poison source survived context replacement")
			}
		})
	}
}

func TestCombatSummonPoisonDeathHasNoRewards(t *testing.T) {
	w, owner, attacker, target := bmSummonPvPWorld(t, 2000)
	st := &craftStore{}
	w.store = st
	now := w.now()
	applySummonTestPoison(w, attacker, target)
	target.Affects[0].NextTick = now
	target.HP, target.Def.ExpReward, target.Def.Gold = 100, 5000, 5000
	target.Def.Carry = []model.Item{{Index: 412}}
	drainPackets(owner)
	drainPackets(attacker)
	before := accountStateSnapshot(attacker.Account)
	w.tickMobAffects(now, 0, 1)
	if !target.Dead || w.mobsByID[target.ID] != nil || w.summons[target.ID] != nil ||
		owner.hasVisible(target.ID) || attacker.hasVisible(target.ID) {
		t.Fatal("poison death retained a summon or observer visibility")
	}
	if !reflect.DeepEqual(before, accountStateSnapshot(attacker.Account)) || st.saves != 0 || len(w.groundItems) != 0 {
		t.Fatal("summon poison death granted rewards or wrote an account")
	}
}

func TestCombatSummonAffectsWaitForParticipantAcknowledgement(t *testing.T) {
	for _, held := range []string{"owner", "caster"} {
		t.Run(held, func(t *testing.T) {
			w, owner, caster, target := bmSummonPvPWorld(t, 2000)
			now := w.now()
			applySummonTestPoison(w, caster, target)
			target.Affects[0].NextTick = now
			target.Affects[1] = model.Affect{Type: 12, ExpiresAt: now.Add(-time.Second)}
			participant := owner
			if held == "caster" {
				participant = caster
				// The expired effect also belongs to the retained caster.
				target.Affects[1].OwnerID, target.Affects[1].OwnerCharacterUID = caster.ID, caster.Char.UID
			}
			release, queue := holdCriticalCombatParticipant(t, w, participant)
			before := *target
			for _, completed := range []bool{false, true} {
				if completed {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.tickMobAffects(now, 0, 1)
				if !reflect.DeepEqual(before, *target) {
					t.Fatal("unacknowledged write changed summon effects or HP")
				}
			}
			w.processCriticalCompletions(1)
			w.tickMobAffects(now, 0, 1)
			if target.HP != 390 || target.Affects[1].Type != 0 {
				t.Fatal("summon effects did not resume after acknowledgement")
			}
		})
	}
}

func TestCombatSummonPoisonWaitsForSameTickKillCommit(t *testing.T) {
	w, owner, caster, target := bmSummonPvPWorld(t, 2000)
	gate, release := criticalGate(t)
	t.Cleanup(release)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	queue := newCriticalTestQueue(t, &criticalBatchTestStore{backend}, 1, 2)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	claimCriticalTestPlayers(t, w, owner, caster)
	w.rng = fixedRNG{}
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
	w.tickMobAffects(now, 0, 1)
	awaitCriticalWrite(t, backend.entered)
	if target.HP != 500 {
		t.Fatal("summon poison advanced while a normal kill retained its caster")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.tickMobAffects(now, 0, 1)
	if target.HP != 500 {
		t.Fatal("worker completion bypassed kill acknowledgement")
	}
	w.processCriticalCompletions(1)
	w.tickMobAffects(now, 0, 1)
	if target.HP != 390 {
		t.Fatal("summon poison did not resume after kill acknowledgement")
	}
}
