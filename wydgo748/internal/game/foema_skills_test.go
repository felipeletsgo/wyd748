package game

import (
	"encoding/binary"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Official TMSRVIN InstanceType 6: Heal and Recovery without the old x1.4
// and 548 cap; Bless Revival (31) adds x1.2 to Recovery and, for a
// Celestial, the caster's level.
func TestFoemaHealFollowsOfficialServer(t *testing.T) {
	plain := &model.Char{Class: 1, Score: testScore(model.Score{Level: 300})}
	if got := foemaHealAmount(plain, 27, 200, 100); got != 500 {
		t.Fatalf("Heal=%d, want 100 + 2*200", got)
	}
	if got := foemaHealAmount(plain, 29, 200, 150); got != 450 {
		t.Fatalf("Recovery=%d, want 150 + 3*200/2", got)
	}
	revival := &model.Char{Class: 1, LearnedSkill: 1 << 7, Score: testScore(model.Score{Level: 300})}
	if got := foemaHealAmount(revival, 29, 200, 150); got != 540 {
		t.Fatalf("Recovery with Bless Revival=%d, want 450 x 1.2", got)
	}
	revival.Evolution = "celestial"
	if got := foemaHealAmount(revival, 27, 200, 100); got != 800 {
		t.Fatalf("Celestial Heal with Bless Revival=%d, want 500 + level 300", got)
	}
}

func TestHolyJudgementKeepsTwoThirdsOfHP(t *testing.T) {
	ch := &model.Char{Class: 1, Score: testScore(model.Score{MaxHP: 3000, CurHP: 3000})}
	if got := holyJudgementSacrifice(ch); got != 3000 || playerCurHP(ch) != 2000 {
		t.Fatalf("Holy Judgement bonus=%d hp=%d, want 3000/2000", got, playerCurHP(ch))
	}
	ch.LearnedSkill = 1 << 7
	if got := holyJudgementSacrifice(ch); got != 2200 || playerCurHP(ch) != 1333 {
		t.Fatalf("Holy Judgement with Bless Revival bonus=%d hp=%d, want 2200/1333", got, playerCurHP(ch))
	}
}

func TestHolyJudgementPreservesLifeAtLowHP(t *testing.T) {
	for _, tc := range []struct {
		hp, want uint32
	}{{0, 0}, {1, 1}, {2, 1}, {3, 2}} {
		ch := &model.Char{Class: 1, Score: testScore(model.Score{MaxHP: 1000, CurHP: tc.hp})}
		if bonus := holyJudgementSacrifice(ch); bonus != int(tc.hp) || playerCurHP(ch) != tc.want {
			t.Errorf("initial HP=%d: damage bonus=%d remaining HP=%d, want %d/%d",
				tc.hp, bonus, playerCurHP(ch), tc.hp, tc.want)
		}
	}
}

func TestHolyJudgementRepeatedCastsPreserveLivingCaster(t *testing.T) {
	for _, mode := range []string{"PvE", "PvP"} {
		for _, hp := range []uint32{1, 2, 3} {
			t.Run(fmt.Sprintf("%s/HP%d", mode, hp), func(t *testing.T) {
				caster, _ := networkedTestPlayer(1, "Foema", 2100, 2100)
				target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
				w := worldWithNetworkedPlayers(caster, target)
				w.rng = fixedRNG{value: 0}
				w.store = &craftStore{}
				w.items = make(map[uint16]model.ItemDef)
				w.ghostShops = make(map[uint16]*GhostShop)
				caster.Char.Class = 1
				caster.Char.LearnedSkill = 1 << (30 - 24)
				caster.PKMode = true
				setPlayerCurHP(caster.Char, hp)
				w.skills = map[int]model.SkillDef{30: {
					Index: 30, Name: "Holy Judgement", InstanceType: 2, InstanceValue: 1,
					ManaSpent: 10, Range: 6, MaxTarget: 1, Aggressive: 1,
				}}
				targetID := target.ID
				target.Char.Score.MaxHP, target.Char.Score.CurHP = 1_000_000, 1_000_000
				applyScore(target.Char)
				targetHP := func() uint32 { return playerCurHP(target.Char) }
				if mode == "PvE" {
					mob := &Mob{ID: 1300, X: caster.X + 2, Y: caster.Y, HP: 1_000_000, GenerIndex: -1,
						Def: testNPCDef(model.Score{MaxHP: 1_000_000, CurHP: 1_000_000})}
					w.appendMobInstance(mob)
					w.registerMobSpatial(mob)
					caster.show(mob.ID)
					targetID = mob.ID
					targetHP = func() uint32 { return mob.HP }
				}
				for cast := 0; cast < 10; cast++ {
					beforeHP, beforeMP := targetHP(), playerCurMP(caster.Char)
					w.onSkillAttack(caster, skillCastRequest{Skill: 30, TargetID: targetID, Motion: 5})
					if targetHP() >= beforeHP || playerCurMP(caster.Char) >= beforeMP {
						t.Fatalf("cast %d did not execute: target HP=%d/%d caster MP=%d/%d",
							cast, targetHP(), beforeHP, playerCurMP(caster.Char), beforeMP)
					}
					wantHP := uint32(1)
					if hp == 3 && cast == 0 {
						wantHP = 2
					}
					if playerCurHP(caster.Char) != wantHP || !caster.DeadAt.IsZero() {
						t.Fatalf("cast %d left caster HP=%d dead=%v, want living HP=%d",
							cast, playerCurHP(caster.Char), caster.DeadAt, wantHP)
					}
				}
			})
		}
	}
}

func TestBlessRevivalChance(t *testing.T) {
	for mastery, want := range map[int]int{0: 30, 45: 36, 255: 64, 320: 72, 999: 90} {
		if got := foemaRevivalChance(mastery); got != want {
			t.Fatalf("revival chance mastery=%d: %d, want %d", mastery, got, want)
		}
	}
}

// Official TMSRVIN and native 7.48 FUN_0054FDB9: the holder's class and
// learned bits 20 (Magic Weapon) and 23 (tree finisher) scale the buffs.
func TestFoemaBuffBonusesUseHolderSkills(t *testing.T) {
	now := time.Now()
	apply := func(class byte, learned uint32, affect model.Affect) *model.Score {
		ch := &model.Char{Class: class, LearnedSkill: learned, Score: testScore(model.Score{
			Attack: 1000, Defense: 100, Mastery: [4]uint32{100, 100, 100, 100},
		})}
		affect.ExpiresAt = now.Add(time.Minute)
		ch.Affects[0] = affect
		(&World{}).applyAffectStats(ch)
		return effectiveScore(ch)
	}
	weapon := model.Affect{Type: 9, Level: 60, Value: 5}
	if e := apply(0, 0, weapon); e.Attack != 1086 {
		t.Fatalf("Magic Weapon on others=%d, want (1000 + 35) x 1.05", e.Attack)
	}
	if e := apply(1, 1<<20, weapon); e.Attack != 1410 {
		t.Fatalf("Magic Weapon on its Foema=%d, want (1000 + 175) x 1.2", e.Attack)
	}
	if e := apply(1, 1<<20|1<<23, weapon); e.Attack != 1492 {
		t.Fatalf("Magic Weapon with bit 23=%d, want (1000 + 175) x 1.27", e.Attack)
	}
	shield := model.Affect{Type: 11, Level: 60, Value: 15}
	if e := apply(1, 1<<23, shield); e.Defense != 235 {
		t.Fatalf("Magic Shield with bit 23=%d, want 100 + 35 + 100", e.Defense)
	}
	athena := model.Affect{Type: 15, Level: 100, Value: 7}
	if e := apply(1, 1<<23, athena); e.Mastery[1] != 120 {
		t.Fatalf("Athena with bit 23 mastery=%d, want 100 + 17 x 1.2", e.Mastery[1])
	}
}

func TestCureRemovesAffect33(t *testing.T) {
	ch := &model.Char{}
	ch.Affects[0] = model.Affect{Type: 33, ExpiresAt: time.Now().Add(time.Minute)}
	if !cleansePlayer(ch, false) || ch.Affects[0].Type != 0 {
		t.Fatal("Cure kept affect 33")
	}
}

func TestTeleportRejectsTargetsAboveLevelLimit(t *testing.T) {
	caster, _ := networkedTestPlayer(1, "Foema", 2200, 2200)
	target, _ := networkedTestPlayer(2, "Member", 2210, 2200)
	party := &Party{Members: []*Player{caster, target}}
	caster.Party, target.Party = party, party
	setLevel := func(p *Player, level uint32) {
		p.Char.Score.Level = level
		if p.Char.RuntimeScore != nil {
			p.Char.RuntimeScore.Level = level
		}
	}
	setLevel(caster, 100)
	setLevel(target, 251)
	w := testSpatialWorld(nil, caster, target)
	skill := model.SkillDef{Index: 42, InstanceType: 9, Range: 20, MaxTarget: 1}
	if got := w.applySupportSkill(caster, skillCastRequest{TargetID: target.ID}, skill, 50); len(got) != 0 {
		t.Fatalf("Teleport moved a level 251 target with limit 250: %+v", got)
	}
	setLevel(target, 250)
	if got := w.applySupportSkill(caster, skillCastRequest{TargetID: target.ID}, skill, 50); len(got) != 1 {
		t.Fatal("Teleport rejected a target at the level limit")
	}
}

// Official TMSRVIN sub_575830: Cancellation removes one Immunity from an
// immune target and applies affect 32 only to a target without Immunity.
func TestCancellationBreaksImmunityOrBlocksPotions(t *testing.T) {
	caster, _ := networkedTestPlayer(1, "Caster", 2200, 2200)
	target, _ := networkedTestPlayer(2, "Target", 2201, 2200)
	w := testSpatialWorld(nil, caster, target)
	skill := model.SkillDef{Index: cancellationSkill, TargetType: 1, Aggressive: 1,
		AffectType: affectCancellation, AffectTime: 1, MaxTarget: 1}
	now := w.now()
	target.Char.Affects[0] = model.Affect{Type: 19, ExpiresAt: now.Add(time.Minute)}
	w.executePlayerSkill(caster, []*Player{target}, skill, 100, 0)
	if target.Char.Affects[0].Type != 0 || activePlayerAffectAt(target.Char, affectCancellation, now) != nil {
		t.Fatalf("immune target: %+v", target.Char.Affects[:2])
	}
	w.executePlayerSkill(caster, []*Player{target}, skill, 100, 0)
	if activePlayerAffectAt(target.Char, affectCancellation, now) == nil {
		t.Fatal("Cancellation did not apply affect 32 without Immunity")
	}
}

// Heal sends the caster's own attack packet; FakeExp must carry the durable
// Hold so the client keeps showing the PvP EXP debt.
func TestHealKeepsCasterHoldInAttackPacket(t *testing.T) {
	caster, _ := networkedTestPlayer(1, "Foema", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
	w := worldWithNetworkedPlayers(caster, target)
	w.rng = fixedRNG{value: 0}
	caster.Char.Class = 1
	caster.Char.LearnedSkill = 1 << (27 - 24)
	caster.Char.Hold = 12345
	party := &Party{Members: []*Player{caster, target}}
	caster.Party, target.Party = party, party
	setPlayerCurHP(target.Char, 100)
	w.skills = map[int]model.SkillDef{27: {
		Index: 27, Name: "Heal", InstanceType: 6, InstanceValue: 100,
		ManaSpent: 10, Range: 6, MaxTarget: 1,
	}}
	drainPackets(caster)
	w.onSkillAttack(caster, skillCastRequest{Skill: 27, TargetID: target.ID, Motion: 5})
	found := false
	for {
		pkt, ok := caster.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if !wire.Decrypt(pkt) {
			t.Fatal("undecodable packet")
		}
		h := wire.ParseHeader(pkt)
		if (h.Type == wire.OpAttackOne || h.Type == wire.OpAttackTwo || h.Type == wire.OpAttackMulti) &&
			binary.LittleEndian.Uint16(pkt[12:]) == caster.ID {
			found = true
			if got := binary.LittleEndian.Uint32(pkt[40:]); got != 12345 {
				t.Fatalf("Heal FakeExp=%d, want Hold 12345", got)
			}
		}
	}
	if !found {
		t.Fatal("Heal sent no attack packet to the caster")
	}
	if caster.Char.Hold != 12345 {
		t.Fatalf("Heal changed durable Hold to %d", caster.Char.Hold)
	}
}

func TestResurrectionFollowsW2PPRule(t *testing.T) {
	recalls := 0
	for rev := 0; rev < 115; rev++ {
		if resurrectionRecalls(func(int) int { return rev }) {
			recalls++
		}
	}
	// rand()%115 folded above 100: values 40..114 recall (75 of 115).
	if recalls != 75 {
		t.Fatalf("recall outcomes=%d of 115, want 75", recalls)
	}
	if got := resurrectionHP(10_000, func(int) int { return 49 }); got != 5000 {
		t.Fatalf("max Resurrection HP=%d, want 50%%", got)
	}
	if got := resurrectionHP(50, func(int) int { return 0 }); got != 1 {
		t.Fatalf("low MaxHP Resurrection HP=%d, want 1", got)
	}
}

// W2PP rev >= 40: the dead caster is recalled to the bound city alive.
func TestResurrectionRecallBranchRevivesInBoundCity(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.rng = fixedRNG{value: 99}
	bit, ok := specialSkillBit(99)
	if !ok {
		t.Fatal("skill 99 has no special bit")
	}
	p.Char.LearnedSkill |= 1 << bit
	p.X, p.Y = 1058, 1725 // Noatum
	p.Char.X, p.Char.Y = p.X, p.Y
	w.updatePlayerSpatial(p)
	setPlayerCurHP(p.Char, 0)
	p.DeadAt = w.now()
	w.skills = map[int]model.SkillDef{99: {Index: 99, Name: "Resurrect", MaxTarget: 1}}
	w.onSkillAttack(p, skillCastRequest{Skill: 99, TargetID: p.ID, Motion: 5})
	if playerCurHP(p.Char) == 0 || !p.DeadAt.IsZero() {
		t.Fatalf("Resurrection recall left the caster dead: hp=%d", playerCurHP(p.Char))
	}
	if p.X == 1058 && p.Y == 1725 {
		t.Fatal("Resurrection recall branch did not move the caster to town")
	}
}
