package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestManaControlDamageSplit(t *testing.T) {
	now := time.Date(2026, 9, 17, 12, 0, 0, 0, time.UTC)
	for _, tc := range []struct {
		name               string
		damage, mp, wantHP uint32
		wantMP             uint32
	}{
		{"full mana", 100, 1000, 10, 910},
		{"exact mana", 100, 90, 10, 0},
		{"insufficient mana", 100, 40, 60, 0},
		{"empty mana", 100, 0, 100, 0},
		{"zero damage", 0, 100, 0, 100},
		{"one damage", 1, 100, 1, 100},
		{"fraction rounds to HP", 19, 100, 2, 83},
		{"wide damage", 1_000_000, 950_000, 100_000, 50_000},
		{"uint32 multiplication", ^uint32(0), ^uint32(0), 429_496_730, 429_496_730},
	} {
		t.Run(tc.name, func(t *testing.T) {
			ch := &model.Char{Class: 1, Score: testScore(model.Score{
				MaxHP: 1000, CurHP: 1000, MaxMP: tc.mp, CurMP: tc.mp,
			})}
			ch.Affects[0] = model.Affect{Type: 18, Value: 100, Level: 200, ExpiresAt: now.Add(time.Minute)}
			// A duplicate slot must not absorb twice.
			ch.Affects[1] = ch.Affects[0]
			hpDamage := absorbManaControlDamageAt(ch, tc.damage, now)
			if hpDamage != tc.wantHP || playerCurMP(ch) != tc.wantMP {
				t.Fatalf("HP damage/MP left=%d/%d, want %d/%d", hpDamage, playerCurMP(ch), tc.wantHP, tc.wantMP)
			}
			if uint64(hpDamage)+uint64(tc.mp-playerCurMP(ch)) != uint64(tc.damage) {
				t.Fatal("damage was lost or duplicated in the split")
			}
			if playerCurHP(ch) != 1000 {
				t.Fatal("split helper must leave HP application to the damage caller")
			}
		})
	}
}

func TestManaControlInactiveAndDepletion(t *testing.T) {
	now := time.Date(2026, 9, 17, 12, 0, 0, 0, time.UTC)
	ch := &model.Char{Class: 1, Score: testScore(model.Score{
		MaxHP: 1000, CurHP: 1000, MaxMP: 1000, CurMP: 100,
	})}
	if got := absorbManaControlDamageAt(nil, 100, now); got != 100 {
		t.Fatal("nil character changed damage")
	}
	for _, expiry := range []time.Time{time.Time{}, now.Add(-time.Second), now} {
		ch.Affects[0] = model.Affect{Type: 18, ExpiresAt: expiry}
		if got := absorbManaControlDamageAt(ch, 100, now); got != 100 || playerCurMP(ch) != 100 {
			t.Fatal("inactive/expired buff absorbed damage")
		}
	}
	ch.Affects[0] = model.Affect{Type: 18, ExpiresAt: now.Add(time.Minute)}
	for i, want := range []uint32{10, 90, 100} {
		if got := absorbManaControlDamageAt(ch, 100, now); got != want {
			t.Fatalf("hit %d damage=%d, want %d", i+1, got, want)
		}
	}
	restorePlayerMP(ch, 90)
	if got := absorbManaControlDamageAt(ch, 100, now); got != 10 || playerCurMP(ch) != 0 {
		t.Fatal("active buff did not resume after mana recovery")
	}
	restorePlayerMP(ch, 90)
	setPlayerCurHP(ch, 0)
	if got := absorbManaControlDamageAt(ch, 100, now); got != 100 || playerCurMP(ch) != 90 {
		t.Fatal("dead character spent mana")
	}
}

func TestManaControlDoesNotDiscountSkillManaCost(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 1
	p.Char.LearnedSkill = 1<<(46-24) | 1<<(27-24)
	p.Char.Score.SaveMana = 25 // Keep the legitimate equipment/base discount.
	w.recalcPlayer(p.Char)
	w.skills = map[int]model.SkillDef{
		46: {Index: 46, ManaSpent: 130, MaxTarget: 1, AffectType: 18, AffectValue: 100, AffectTime: 600},
		27: {Index: 27, ManaSpent: 40, MaxTarget: 1, InstanceValue: 1},
	}
	mp := playerCurMP(p.Char)
	w.onSkillAttack(p, skillCastRequest{Skill: 46, TargetID: p.ID})
	if got := playerCurMP(p.Char); got != mp-97 || !hasActiveAffectAt(p.Char, 18, w.now()) {
		t.Fatalf("Mana Control cast: MP=%d, want %d with active buff", got, mp-97)
	}
	if got := effectiveScore(p.Char).SaveMana; got != 25 {
		t.Fatalf("Mana Control changed SaveMana to %d, want 25", got)
	}
	mp = playerCurMP(p.Char)
	w.onSkillAttack(p, skillCastRequest{Skill: 27, TargetID: p.ID})
	if got := playerCurMP(p.Char); got != mp-30 {
		t.Fatalf("spell with Mana Control cost=%d, want 30", mp-got)
	}
}

func manaControlCombatFixture(t *testing.T, active bool) (*World, *Player, *Player, *Mob) {
	t.Helper()
	attacker, _ := networkedTestPlayer(1, "Attacker", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2102, 2100)
	attacker.Char.Class = 3
	attacker.Char.LearnedSkill = 0x40 | 1<<23
	attacker.Char.Score.Attack = 1000
	attacker.Char.Score.Accuracy = 1000
	attacker.PKMode = true
	target.Char.Class = 1
	target.Char.Score.MaxHP, target.Char.Score.CurHP = 100_000, 100_000
	target.Char.Score.MaxMP, target.Char.Score.CurMP = 100_000, 100_000
	w := worldWithNetworkedPlayers(attacker, target)
	w.clock = newFakeClock(time.Date(2026, 9, 17, 12, 0, 0, 0, time.UTC))
	w.rng = fixedRNG{value: 0}
	w.store = &craftStore{}
	if active {
		target.Char.Affects[0] = model.Affect{Type: 18, Value: 100, ExpiresAt: w.now().Add(time.Minute)}
	}
	w.recalcPlayer(attacker.Char)
	w.recalcPlayer(target.Char)
	mob := &Mob{ID: 1000, X: 2101, Y: 2100, HP: 100_000,
		Def: testNPCDef(model.Score{MaxHP: 100_000, CurHP: 100_000,
			Attack: 1000, MagicAttack: 1000, Accuracy: 1000})}
	w.mobs = append(w.mobs, mob)
	w.registerMobSpatial(mob)
	w.showPlayerPair(attacker, target)
	attacker.show(mob.ID)
	target.show(mob.ID)
	drainPlayerPackets(attacker)
	drainPlayerPackets(target)
	return w, attacker, target, mob
}

func assertManaControlVitalsPacket(t *testing.T, receiver, target *Player) {
	t.Helper()
	found := false
	for {
		pkt, ok := receiver.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if !wire.Decrypt(pkt) {
			t.Fatal("invalid encrypted packet")
		}
		header := wire.ParseHeader(pkt)
		if header.Type != wire.OpSetHpMp || header.ID != target.ID {
			continue
		}
		found = true
		if len(pkt) != 28 || binary.LittleEndian.Uint32(pkt[12:16]) != playerCurHP(target.Char) ||
			binary.LittleEndian.Uint32(pkt[16:20]) != playerCurMP(target.Char) {
			t.Fatalf("receiver %d got stale or malformed HP/MP: %x", receiver.ID, pkt)
		}
	}
	if !found {
		t.Fatalf("receiver %d did not receive authoritative HP/MP for %d", receiver.ID, target.ID)
	}
}

func TestManaControlCombatPaths(t *testing.T) {
	skillCast := func(index, hits int) func(*World, *Player, *Player, *Mob) {
		return func(w *World, attacker, target *Player, _ *Mob) {
			attacker.Char.Class = byte(index / 24)
			attacker.Char.LearnedSkill = 1 << uint(index%24)
			w.recalcPlayer(attacker.Char)
			w.skills = map[int]model.SkillDef{index: {Index: index, InstanceType: 2,
				InstanceValue: 100, Range: 6, MaxTarget: hits, Aggressive: 1}}
			w.onSkillAttack(attacker, skillCastRequest{Skill: index, TargetID: target.ID})
		}
	}
	for _, tc := range []struct {
		name string
		hits uint32
		hit  func(*World, *Player, *Player, *Mob)
	}{
		{"physical PvP", 1, func(w *World, a, p *Player, _ *Mob) {
			w.onAttack(a.Session, physicalAttackPacket(1000, p.ID, p.X, p.Y))
		}},
		{"Iron Spear secondary PvP", 1, func(w *World, a, p *Player, m *Mob) {
			w.onAttack(a.Session, physicalAttackTwoPacket(1000, m.ID, m.X, m.Y, p.ID))
		}},
		{"magic PvP", 1, skillCast(24, 1)},
		{"multi-hit PvP", 3, skillCast(95, 3)},
		{"mob physical", 1, func(w *World, _ *Player, p *Player, m *Mob) {
			w.mobAttackPlayer(m, p, w.now())
		}},
		{"boss magic", 1, func(w *World, _ *Player, p *Player, m *Mob) {
			w.skills = map[int]model.SkillDef{24: {Index: 24, InstanceType: 2, InstanceValue: 100}}
			w.bossCastSkill(&BossRuntime{Profile: &BossProfile{ID: "mana-test"}}, m,
				BossActionDef{SkillID: 24, CastRange: 6}, BossEvent{SourceID: p.ID})
		}},
		{"summon PvP", 1, func(w *World, a, p *Player, m *Mob) {
			m.SummonerID, m.SummonRange = a.ID, 2
			w.summons[m.ID] = m
			a.CombatTargetID = p.ID
			w.tickSummonCombat(w.now())
		}},
		{"poison", 1, func(w *World, _ *Player, p *Player, _ *Mob) {
			p.Char.Affects[1] = model.Affect{Type: 20, Value: 100, ExpiresAt: w.now().Add(time.Minute)}
			w.tickPlayerAffects(w.now())
		}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			baselineWorld, a, p, m := manaControlCombatFixture(t, false)
			beforeHP, beforeMP := playerCurHP(p.Char), playerCurMP(p.Char)
			tc.hit(baselineWorld, a, p, m)
			damage := beforeHP - playerCurHP(p.Char)
			if damage == 0 || playerCurHP(p.Char) == 0 || playerCurMP(p.Char) != beforeMP || damage%tc.hits != 0 {
				t.Fatalf("invalid baseline: HP damage=%d MP spent=%d", damage, beforeMP-playerCurMP(p.Char))
			}
			// Identical deterministic hits; round independently for each hit.
			absorbed := (damage / tc.hits * 90 / 100) * tc.hits
			for _, available := range []uint32{beforeMP, absorbed / 2, 0} {
				w, attacker, target, mob := manaControlCombatFixture(t, true)
				setPlayerCurMP(target.Char, available)
				tc.hit(w, attacker, target, mob)
				spent := minU32(available, absorbed)
				if gotHP, gotMP := playerCurHP(target.Char), playerCurMP(target.Char); gotHP != beforeHP-damage+spent || gotMP != available-spent {
					t.Fatalf("available MP=%d: HP/MP=%d/%d, want %d/%d (incoming=%d)",
						available, gotHP, gotMP, beforeHP-damage+spent, available-spent, damage)
				}
				assertManaControlVitalsPacket(t, target, target)
				assertManaControlVitalsPacket(t, attacker, target)
			}
		})
	}
}

func TestManaControlMountAndLethalDamage(t *testing.T) {
	for _, tc := range []struct {
		name                    string
		hp, mp, damage, mountHP uint32
		wantHP, wantMP          uint32
		wantDead                bool
	}{
		{"mount before mana", 1000, 1000, 1000, 20000, 925, 325, false},
		{"mana prevents death", 15, 90, 100, 0, 5, 0, false},
		{"mana exhaustion kills in same hit", 50, 40, 100, 0, 0, 0, true},
		{"HP share remains lethal with full mana", 5, 1000, 100, 0, 0, 910, true},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, observer, p, m := manaControlCombatFixture(t, true)
			setPlayerCurHP(p.Char, tc.hp)
			setPlayerCurMP(p.Char, tc.mp)
			if tc.mountHP > 0 {
				mount := model.Item{Index: 2379}
				mount.SetMountHP(int(tc.mountHP))
				p.Char.Equip[mountSlot] = mount
			}
			w.applyMobDamageToPlayer(m, p, tc.damage, w.now(), func(applied uint32) []byte {
				return wire.AttackHitWide(m.ID, p.ID, m.X, m.Y, p.X, p.Y,
					applied, playerMaxHP(p.Char), 0, m.Def.Score.MaxMP)
			})
			if playerCurHP(p.Char) != tc.wantHP || playerCurMP(p.Char) != tc.wantMP || !p.DeadAt.IsZero() != tc.wantDead {
				t.Fatalf("HP/MP/dead=%d/%d/%v, want %d/%d/%v", playerCurHP(p.Char), playerCurMP(p.Char),
					!p.DeadAt.IsZero(), tc.wantHP, tc.wantMP, tc.wantDead)
			}
			if tc.mountHP > 0 && p.Char.Equip[mountSlot].MountHP() != 19750 {
				t.Fatal("Mana Control changed the mount's prior absorption")
			}
			assertManaControlVitalsPacket(t, p, p)
			assertManaControlVitalsPacket(t, observer, p)
		})
	}
}
