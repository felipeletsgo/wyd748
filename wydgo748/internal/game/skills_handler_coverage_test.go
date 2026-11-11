package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestOnSkillAttackRejectsInvalidCastsBeforeResources(t *testing.T) {
	for _, tc := range []struct {
		name       string
		class      byte
		skill      int
		learned    bool
		definition model.SkillDef
		missing    bool
		cooldown   bool
		claw       bool
		noTarget   bool
	}{
		{name: "unlearned", class: 1, skill: 24, definition: model.SkillDef{Index: 24}},
		{name: "missing catalog skill", class: 1, skill: 24, learned: true, missing: true},
		{name: "passive skill", class: 1, skill: 24, learned: true, definition: model.SkillDef{Index: 24, Passive: 1}},
		{name: "active cooldown", class: 1, skill: 24, learned: true, definition: model.SkillDef{Index: 24}, cooldown: true},
		{name: "claw blocks enchant frost", class: 3, skill: 74, learned: true, definition: model.SkillDef{Index: 74}, claw: true},
		{name: "backpack stays client local", class: 3, skill: backpackSkill, learned: true, definition: model.SkillDef{Index: backpackSkill}},
		{name: "hostile skill has no target", class: 1, skill: 24, learned: true, definition: model.SkillDef{Index: 24, Aggressive: 1, Range: 6}, noTarget: true},
		{name: "insufficient mana", class: 1, skill: 27, learned: true, definition: model.SkillDef{Index: 27, ManaSpent: 1000, Range: 6}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, _ := handlerTestWorld(t)
			p.Char.Class = tc.class
			if tc.learned {
				p.Char.LearnedSkill = uint32(1) << (tc.skill - int(tc.class)*24)
			}
			p.Char.Score.MaxMP = 100
			p.Char.Score.CurMP = 100
			applyScore(p.Char)
			if !tc.missing {
				w.skills = map[int]model.SkillDef{tc.skill: tc.definition}
			}
			if tc.cooldown {
				p.SkillReady = map[int]time.Time{tc.skill: w.now().Add(time.Minute)}
			}
			if tc.claw {
				w.items[832] = model.ItemDef{Index: 832, StaticEffects: []model.StaticEffect{{Name: "EF_WTYPE", Value: 41}}}
				p.Char.Equip[6] = model.Item{Index: 832}
			}
			beforeMP := playerCurMP(p.Char)
			beforeReady := len(p.SkillReady)
			beforePackets := p.Session.QueuedPacketsForTest()
			req := skillCastRequest{Skill: tc.skill}
			if tc.noTarget {
				req.TargetID = 0xFFFF
			}

			w.onSkillAttack(p, req)

			if playerCurMP(p.Char) != beforeMP || len(p.SkillReady) != beforeReady ||
				p.Session.QueuedPacketsForTest() != beforePackets {
				t.Fatalf("rejected skill mutated resources/cooldown or published: MP %d->%d cooldowns=%v packets=%d->%d",
					beforeMP, playerCurMP(p.Char), p.SkillReady, beforePackets, p.Session.QueuedPacketsForTest())
			}
		})
	}
}

func TestSkillAttackBlessRevivalRematerializesDeadPartyMember(t *testing.T) {
	caster, _ := networkedTestPlayer(1, "Foema", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
	target.Char.UID = "revival-target-uid"
	party := &Party{Members: []*Player{caster, target}}
	caster.Party, target.Party = party, party
	w := worldWithNetworkedPlayers(caster, target)
	w.rng = fixedRNG{value: 0}
	caster.Char.Class = 1
	caster.Char.LearnedSkill = 1 << 7 // Bless Revival, Foema local skill 7.
	caster.Char.Score.Mastery[1] = 90
	caster.Char.Score.MaxMP, caster.Char.Score.CurMP = 1000, 1000
	applyScore(caster.Char)
	target.Char.Score.MaxHP, target.Char.Score.CurHP = 1000, 1000
	applyScore(target.Char)
	setPlayerCurHP(target.Char, 0)
	w.indexPlayerCharacter(target)
	target.DeadAt = w.now()
	beforeMP, beforeMaxMP := playerCurMP(caster.Char), caster.Char.Score.MaxMP
	w.skills = map[int]model.SkillDef{31: {
		Index: 31, Name: "Bless Revival", ManaSpent: 10, Range: 6, MaxTarget: 1,
	}}

	w.onSkillAttack(caster, skillCastRequest{Skill: 31, TargetID: target.ID, Motion: 5})

	revivedHP, maxHP := playerCurHP(target.Char), playerMaxHP(target.Char)
	if revivedHP == 0 || uint64(revivedHP)*100 < uint64(maxHP)*45 || uint64(revivedHP)*100 > uint64(maxHP)*60 ||
		!target.DeadAt.IsZero() || !target.InWorld ||
		w.playersByID[target.ID] != target || w.playersByCharacterUID[target.Char.UID] != target {
		t.Fatalf("revived member was not restored: hp=%d max=%d deadAt=%v inWorld=%v byID=%v uid=%q byUID=%v",
			revivedHP, maxHP, target.DeadAt, target.InWorld,
			w.playersByID[target.ID] == target, target.Char.UID, w.playersByCharacterUID[target.Char.UID] == target)
	}
	skillCost := uint32(skillManaCost(w.skills[31], 90, int(effectiveScore(caster.Char).SaveMana)))
	wantMP := beforeMP - skillCost - beforeMaxMP/10
	if playerCurMP(caster.Char) != wantMP || caster.SkillReady[31].IsZero() {
		t.Fatalf("successful revival did not charge and start cooldown: mp=%d want=%d skillCost=%d reviveCost=%d ready=%v",
			playerCurMP(caster.Char), wantMP, skillCost, beforeMaxMP/10, caster.SkillReady[31])
	}
}

func TestSkillAttackCastsThornWallThroughSpecialSkillPath(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.LearnedSkill |= 1 << 26 // Special skill 98.
	p.Char.Score.MaxMP, p.Char.Score.CurMP = 1000, 1000
	applyScore(p.Char)
	w.terrain = loadedFlatTerrain()
	w.npcs = []model.NPCDef{{
		Name: "Vinha", Tipo: model.TipoNPC,
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 500, CurHP: 100},
	}}
	w.nextMobID = firstMobID
	w.skills = map[int]model.SkillDef{98: {
		Index: 98, Name: "Thorn Wall", ManaSpent: 10, Range: 6, Aggressive: 1,
	}}

	w.onSkillAttack(p, skillCastRequest{Skill: 98, TargetX: 2102, TargetY: 2100})

	if len(w.sephiraObjects) != 1 || len(w.mobsByID) != 1 || playerCurMP(p.Char) != 990 ||
		p.SkillReady[98].IsZero() {
		t.Fatalf("special cast did not create the wall and settle its resources: walls=%d mobs=%d mp=%d cooldown=%v",
			len(w.sephiraObjects), len(w.mobsByID), playerCurMP(p.Char), p.SkillReady[98])
	}
}

func TestCriticalCoinArmorChargesBeforeResumingCast(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 3
	p.Char.LearnedSkill = 1 << 14
	p.Char.Gold = 1000
	p.Char.Score.Mastery[2] = 5
	p.Char.Score.MaxMP, p.Char.Score.CurMP = 1000, 1000
	applyScore(p.Char)
	w.items[729] = model.ItemDef{Index: 729, StaticEffects: []model.StaticEffect{{Name: "EF_WTYPE", Value: 41}}}
	p.Char.Equip[6] = model.Item{Index: 729}
	w.skills = map[int]model.SkillDef{86: {
		Index: 86, Name: "Coin Armor", ManaSpent: 10, Delay: 1, AffectType: 1, AffectValue: 10, AffectTime: 10,
	}}
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("failed to claim the Coin Armor account session")
	}
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1)}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	beforeMP := playerCurMP(p.Char)

	w.onSkillAttack(p, skillCastRequest{Skill: 86, TargetID: p.ID, Motion: 5})

	proposal := awaitCriticalWrite(t, backend.entered)
	if len(proposal.accounts) != 1 || proposal.accounts[0] == p.Account || proposal.accounts[0].Chars[0].Gold != 500 ||
		p.Char.Gold != 1000 || playerCurMP(p.Char) != beforeMP || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatalf("uncommitted Coin Armor cost changed live state: proposal=%+v gold=%d mp=%d packets=%d",
			proposal, p.Char.Gold, playerCurMP(p.Char), p.Session.QueuedPacketsForTest())
	}
	w.applyCriticalCompletion(awaitCriticalResult(t, queue))
	if p.Char.Gold != 500 || playerCurMP(p.Char) != beforeMP-10 ||
		!hasActiveAffectAt(p.Char, 1, w.now()) || p.goldenShieldPaid {
		t.Fatalf("committed Coin Armor did not resume once: gold=%d mp=%d affect=%+v pendingCharge=%v",
			p.Char.Gold, playerCurMP(p.Char), p.Char.Affects[0], p.goldenShieldPaid)
	}
}
