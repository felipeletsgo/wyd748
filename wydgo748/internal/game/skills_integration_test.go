package game

import (
	"bytes"
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func dequeueSkillMessage(t *testing.T, p *Player) string {
	t.Helper()
	pkt, ok := p.Session.DequeuePacketForTest()
	if !ok {
		t.Fatal("skill rejection did not enqueue a message")
	}
	if !wire.Decrypt(pkt) {
		t.Fatal("skill rejection message has an invalid checksum")
	}
	if header := wire.ParseHeader(pkt); header.Type != wire.OpMessagePanel || header.ID != 0 {
		t.Fatalf("skill rejection packet has header %+v", header)
	}
	body := pkt[wire.HeaderSize:]
	if terminator := bytes.IndexByte(body, 0); terminator >= 0 {
		body = body[:terminator]
	}
	return string(body)
}

func TestUltimateSkillCostByEvolution(t *testing.T) {
	for _, tc := range []struct {
		name, evolution string
		cost            uint32
	}{
		{"mortal", "", 5_000_000},
		{"arch", archEvolution, 50_000_000},
		{"celestial", "celestial", 100_000_000},
		{"subcelestial", "subcelestial", 100_000_000},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if got := ultimateSkillGold(&model.Char{Evolution: tc.evolution}); got != tc.cost {
				t.Fatalf("ultimate skill fee=%d, want %d", got, tc.cost)
			}
			for _, available := range []uint32{tc.cost - 1, tc.cost} {
				w, p, st := handlerTestWorld(t)
				p.Char.Class = 0
				p.Char.Evolution = tc.evolution
				p.Char.Score.Level = 100
				p.Char.Gold = available
				p.Char.LearnedSkill = 0x7f
				w.skills = map[int]model.SkillDef{7: {Index: 7, Name: "Ultimate", SkillPoint: 1}}
				w.items[5007] = model.ItemDef{Index: 5007}
				master := &Mob{ID: 1200, X: p.X + 1, Y: p.Y, Def: &model.NPCDef{
					Name: "Master", Tipo: model.TipoNPC,
					Score: &model.Score{Version: model.ScoreVersion, Merchant: skillMasterMerchant},
					Vende: []model.Item{{Index: 5007}},
				}}
				w.registerMobSpatial(master)
				p.ShopNPC = master.ID
				p.show(master.ID)
				w.onLearnSkillAtMaster(p.Session, p, 5007, master.ID)
				if available < tc.cost {
					if p.Char.LearnedSkill != 0x7f || p.Char.Gold != available || st.saves != 0 {
						t.Fatal("insufficient gold learned, charged or persisted the ultimate skill")
					}
				} else if p.Char.LearnedSkill != 0xff || p.Char.Gold != 0 || st.saves != 1 {
					t.Fatalf("ultimate skill: learned=%x gold=%d saves=%d", p.Char.LearnedSkill, p.Char.Gold, st.saves)
				}
			}
		})
	}
}

func skillMasterApplyBonusPacket(item, master uint16) []byte {
	pkt := make([]byte, applyBonusPacketSize)
	binary.LittleEndian.PutUint16(pkt[12:14], 2)
	binary.LittleEndian.PutUint16(pkt[14:16], item)
	binary.LittleEndian.PutUint16(pkt[16:18], master)
	return pkt
}

func skillMasterPacketTestWorld(t *testing.T) (*World, *Player, *craftStore, *Mob) {
	t.Helper()
	w, p, st := handlerTestWorld(t)
	p.Char.Class = 0
	p.Char.Score.Level = 20
	p.Char.Score.SkillPts = 100
	applyScore(p.Char)
	w.skills = map[int]model.SkillDef{
		0: {Index: 0, Name: "Skill0", SkillPoint: 1},
		1: {Index: 1, Name: "Skill1", SkillPoint: 1},
	}
	w.items[5000] = model.ItemDef{Index: 5000}
	w.items[5001] = model.ItemDef{Index: 5001}
	master := &Mob{
		ID: 1200, X: p.X + 1, Y: p.Y,
		Def: &model.NPCDef{
			Name: "Master", Tipo: model.TipoNPC,
			Score: &model.Score{
				Version: model.ScoreVersion, Merchant: skillMasterMerchant,
			},
			Vende: []model.Item{{Index: 5000}},
		},
	}
	w.registerMobSpatial(master)
	p.ShopNPC = master.ID
	p.show(master.ID)
	return w, p, st, master
}

func TestApplyBonusSkillMasterPacketContract(t *testing.T) {
	t.Run("persists and publishes all three states", func(t *testing.T) {
		w, p, st, master := skillMasterPacketTestWorld(t)
		packetsBefore := p.Session.QueuedPacketsForTest()

		w.onApplyBonus(p.Session, skillMasterApplyBonusPacket(5000, master.ID))

		if p.Char.LearnedSkill&1 == 0 || st.saves != 1 {
			t.Fatalf("skill was not learned/persisted: mask=%X saves=%d", p.Char.LearnedSkill, st.saves)
		}
		if got := p.Session.QueuedPacketsForTest(); got != packetsBefore+3 {
			t.Fatalf("success published %d additional packets, want 3", got-packetsBefore)
		}
	})

	t.Run("rejects a target other than the open shop", func(t *testing.T) {
		w, p, st, master := skillMasterPacketTestWorld(t)
		learnedBefore := p.Char.LearnedSkill
		packetsBefore := p.Session.QueuedPacketsForTest()

		w.onApplyBonus(p.Session, skillMasterApplyBonusPacket(5000, master.ID+1))

		if p.Char.LearnedSkill != learnedBefore || st.saves != 0 ||
			p.Session.QueuedPacketsForTest() != packetsBefore+1 {
			t.Fatal("incorrect target mutated or persisted state, or did not publish exactly one message")
		}
		if got := dequeueSkillMessage(t, p); got != "Invalid skill master." {
			t.Fatalf("message=%q", got)
		}
	})

	t.Run("reports an already learned skill", func(t *testing.T) {
		w, p, st, master := skillMasterPacketTestWorld(t)
		p.Char.LearnedSkill = 1
		packetsBefore := p.Session.QueuedPacketsForTest()

		w.onApplyBonus(p.Session, skillMasterApplyBonusPacket(5000, master.ID))

		if p.Char.LearnedSkill != 1 || st.saves != 0 ||
			p.Session.QueuedPacketsForTest() != packetsBefore+1 {
			t.Fatal("already learned skill mutated or persisted state, or did not publish exactly one message")
		}
		if got := dequeueSkillMessage(t, p); got != "This skill has already been learned." {
			t.Fatalf("message=%q", got)
		}
	})

	for _, test := range []struct {
		name    string
		item    uint16
		message string
	}{
		{name: "item outside range", item: 5096, message: "This skill is not available from this master."},
		{name: "item not sold", item: 5001, message: "This skill is not available from this master."},
	} {
		t.Run(test.name, func(t *testing.T) {
			w, p, st, master := skillMasterPacketTestWorld(t)
			if test.item == 5096 {
				master.Def.Vende = append(master.Def.Vende, model.Item{Index: test.item})
			}
			packetsBefore := p.Session.QueuedPacketsForTest()

			w.onApplyBonus(p.Session, skillMasterApplyBonusPacket(test.item, master.ID))

			if p.Char.LearnedSkill != 0 || st.saves != 0 ||
				p.Session.QueuedPacketsForTest() != packetsBefore+1 {
				t.Fatal("invalid item mutated or persisted state, or did not publish exactly one message")
			}
			if got := dequeueSkillMessage(t, p); got != test.message {
				t.Fatalf("message=%q, want %q", got, test.message)
			}
		})
	}

	t.Run("rollback on persistence failure", func(t *testing.T) {
		w, p, st, master := skillMasterPacketTestWorld(t)
		p.Char.LearnedSkill = 1 << 3
		learnedBefore := p.Char.LearnedSkill
		packetsBefore := p.Session.QueuedPacketsForTest()
		st.err = errors.New("save failed")

		w.onApplyBonus(p.Session, skillMasterApplyBonusPacket(5000, master.ID))

		if st.saves != 1 {
			t.Fatalf("persistence failure attempted %d saves, want 1", st.saves)
		}
		if p.Char.LearnedSkill != learnedBefore {
			t.Fatalf("rollback did not preserve LearnedSkill: %X -> %X", learnedBefore, p.Char.LearnedSkill)
		}
		if p.Session.QueuedPacketsForTest() != packetsBefore+1 {
			t.Fatal("persistence failure published state or did not publish exactly one message")
		}
		if got := dequeueSkillMessage(t, p); got != "The skill could not be saved. Please try again." {
			t.Fatalf("message=%q", got)
		}
	})

	for _, test := range []struct {
		name    string
		setup   func(*World, *Player)
		message string
	}{
		{
			name: "reports insufficient skill points",
			setup: func(w *World, _ *Player) {
				skill := w.skills[0]
				skill.SkillPoint = 100000
				w.skills[0] = skill
			},
			message: "Insufficient Skill Points to learn this skill.",
		},
		{
			name: "reports insufficient level",
			setup: func(w *World, _ *Player) {
				item := w.items[5000]
				item.ReqLevel = 399
				w.items[5000] = item
			},
			message: "Insufficient level to learn this skill.",
		},
	} {
		t.Run(test.name, func(t *testing.T) {
			w, p, st, master := skillMasterPacketTestWorld(t)
			test.setup(w, p)
			packetsBefore := p.Session.QueuedPacketsForTest()

			w.onApplyBonus(p.Session, skillMasterApplyBonusPacket(5000, master.ID))

			if p.Char.LearnedSkill != 0 || st.saves != 0 ||
				p.Session.QueuedPacketsForTest() != packetsBefore+1 {
				t.Fatal("rejection mutated state or did not publish exactly one message")
			}
			if got := dequeueSkillMessage(t, p); got != test.message {
				t.Fatalf("message=%q, want %q", got, test.message)
			}
		})
	}
}

func TestLearnSkillAtMasterPersistsAndRollsBack(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Class = 0
	p.Char.Score.Level = 20
	p.Char.Score.SkillPts = 100
	applyScore(p.Char)
	w.skills = map[int]model.SkillDef{
		0: {Index: 0, Name: "Skill0", SkillPoint: 1},
		1: {Index: 1, Name: "Skill1", SkillPoint: 1},
	}
	w.items[5000] = model.ItemDef{Index: 5000}
	w.items[5001] = model.ItemDef{Index: 5001}
	master := &Mob{
		ID: 1200, X: p.X + 1, Y: p.Y,
		Def: &model.NPCDef{
			Name: "Master", Tipo: model.TipoNPC,
			Score: &model.Score{
				Version: model.ScoreVersion, Merchant: skillMasterMerchant,
			},
			Vende: []model.Item{{Index: 5000}, {Index: 5001}},
		},
	}
	w.registerMobSpatial(master)
	p.ShopNPC = master.ID
	p.show(master.ID)

	w.onLearnSkill(p.Session, p, 5000)
	if p.Char.LearnedSkill&1 == 0 || st.saves != 1 {
		t.Fatalf("skill was not learned/persisted: mask=%X saves=%d", p.Char.LearnedSkill, st.saves)
	}

	st.err = errors.New("save failed")
	w.onLearnSkillAtMaster(p.Session, p, 5001, master.ID)
	if p.Char.LearnedSkill&(1<<1) != 0 {
		t.Fatal("persistence failure left the second skill learned")
	}
}

func TestAdvancedEvolutionsIgnoreOnlySkillLevelRequirement(t *testing.T) {
	tests := []struct {
		name      string
		evolution string
		learned   bool
	}{
		{name: "mortal", learned: false},
		{name: "arch", evolution: archEvolution, learned: true},
		{name: "celestial", evolution: "celestial", learned: true},
		{name: "subcelestial", evolution: "subcelestial", learned: true},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			w, p, st := handlerTestWorld(t)
			p.Char.Class = 0
			p.Char.Evolution = test.evolution
			p.Char.Score.Level = 0
			p.Char.Score.Str = uint32(baseClassStats[0][0])
			p.Char.Score.Int = uint32(baseClassStats[0][1])
			p.Char.Score.Dex = uint32(baseClassStats[0][2])
			p.Char.Score.Con = uint32(baseClassStats[0][3])
			w.skills = map[int]model.SkillDef{0: {
				Index: 0, Name: "LevelRestricted", SkillPoint: 1,
			}}
			w.items[5000] = model.ItemDef{Index: 5000, ReqLevel: 399}
			master := &Mob{
				ID: 1200, X: p.X + 1, Y: p.Y,
				Def: &model.NPCDef{
					Name: "Master", Tipo: model.TipoNPC,
					Score: &model.Score{
						Version: model.ScoreVersion, Merchant: skillMasterMerchant,
					},
					Vende: []model.Item{{Index: 5000}},
				},
			}
			w.registerMobSpatial(master)
			p.ShopNPC = master.ID
			p.show(master.ID)

			w.onLearnSkillAtMaster(p.Session, p, 5000, master.ID)
			got := p.Char.LearnedSkill&1 != 0
			if got != test.learned {
				t.Fatalf("learned=%t, want %t", got, test.learned)
			}
			wantSaves := 0
			if test.learned {
				wantSaves = 1
			}
			if st.saves != wantSaves {
				t.Fatalf("saves=%d, want %d", st.saves, wantSaves)
			}
		})
	}

	t.Run("mastery remains required", func(t *testing.T) {
		w, p, st := handlerTestWorld(t)
		p.Char.Class = 0
		p.Char.Evolution = archEvolution
		p.Char.Score.Level = 0
		w.skills = map[int]model.SkillDef{0: {
			Index: 0, Name: "MasteryRestricted", SkillPoint: 1,
		}}
		w.items[5000] = model.ItemDef{Index: 5000, ReqLevel: 399, ReqInt: 1}
		master := &Mob{
			ID: 1200, X: p.X + 1, Y: p.Y,
			Def: &model.NPCDef{
				Name: "Master", Tipo: model.TipoNPC,
				Score: &model.Score{
					Version: model.ScoreVersion, Merchant: skillMasterMerchant,
				},
				Vende: []model.Item{{Index: 5000}},
			},
		}
		w.registerMobSpatial(master)
		p.ShopNPC = master.ID
		p.show(master.ID)

		w.onLearnSkillAtMaster(p.Session, p, 5000, master.ID)
		if p.Char.LearnedSkill&1 != 0 || st.saves != 0 {
			t.Fatal("level bypass incorrectly ignored the mastery requirement")
		}
	})
}

func TestSkillAttackPvEAppliesDamageManaAndDebuff(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.rng = fixedRNG{value: 0}
	p.Char.Class = 1
	p.Char.LearnedSkill = 1
	p.Char.Score.Int = 500
	p.Char.Score.MagicAttack = 1000
	p.Char.Score.Mastery[1] = 40
	p.Char.Score.CurMP = 800
	p.Char.Score.MaxMP = 800
	applyScore(p.Char)
	skill := model.SkillDef{
		Index: 24, Name: "Magic", InstanceType: 2, InstanceValue: 100,
		ManaSpent: 20, Delay: 1, Range: 6, MaxTarget: 1, Aggressive: 1,
		AffectType: 3, AffectValue: 10, AffectTime: 5,
	}
	w.skills = map[int]model.SkillDef{24: skill}
	mob := &Mob{
		ID: 1300, X: p.X + 2, Y: p.Y, HP: 100_000, GenerIndex: -1,
		Def: testNPCDef(model.Score{
			Level: 20, MaxHP: 100_000, CurHP: 100_000, MaxMP: 100,
			Defense: 100, ResistFire: 20,
		}),
	}
	w.mobs = append(w.mobs, mob)
	w.registerMobSpatial(mob)
	p.show(mob.ID)
	oldMP := playerCurMP(p.Char)

	w.onSkillAttack(p, skillCastRequest{Skill: 24, Motion: 5, TargetID: mob.ID})
	if mob.HP >= 100_000 || mob.TargetID != p.ID || p.CombatTargetID != mob.ID {
		t.Fatalf("PvE attack did not update combat: hp=%d mobTarget=%d playerTarget=%d",
			mob.HP, mob.TargetID, p.CombatTargetID)
	}
	if playerCurMP(p.Char) >= oldMP {
		t.Fatalf("skill did not consume mana: %d -> %d", oldMP, playerCurMP(p.Char))
	}
	if activeMobAffect(mob, 3) == nil {
		t.Fatal("skill debuff was not applied to the mob")
	}
	if p.SkillReady[24].Before(time.Now()) {
		t.Fatal("cooldown was not recorded")
	}
}

func TestSkillAttackSupportHealsAndRejectsCooldown(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 1
	p.Char.LearnedSkill = 1 << 3 // skill global 27, local 3
	p.Char.Score.Mastery[1] = 40
	p.Char.Score.CurHP = 100
	p.Char.Score.MaxHP = 1000
	p.Char.Score.CurMP = 500
	p.Char.Score.MaxMP = 500
	applyScore(p.Char)
	skill := model.SkillDef{
		Index: 27, Name: "Heal", InstanceValue: 100, ManaSpent: 10,
		Delay: 4, Range: 6, MaxTarget: 1,
	}
	w.skills = map[int]model.SkillDef{27: skill}

	w.onSkillAttack(p, skillCastRequest{Skill: 27})
	hpAfterFirst, mpAfterFirst := playerCurHP(p.Char), playerCurMP(p.Char)
	if hpAfterFirst <= 100 || mpAfterFirst >= 500 {
		t.Fatalf("healing did not apply HP/MP: hp=%d mp=%d", hpAfterFirst, mpAfterFirst)
	}

	w.onSkillAttack(p, skillCastRequest{Skill: 27})
	if playerCurHP(p.Char) != hpAfterFirst || playerCurMP(p.Char) != mpAfterFirst {
		t.Fatal("recast during cooldown changed resources")
	}
}

func TestFoemaBuffInvalidTargetDoesNotConsumeResources(t *testing.T) {
	for _, skill := range foemaBuffTestSkills() {
		t.Run(skill.Name, func(t *testing.T) {
			for _, invalid := range []string{"stale", "foreign party", "out of range", "offline", "dead"} {
				t.Run(invalid, func(t *testing.T) {
					caster, _ := networkedTestPlayer(1, "Caster", 2100, 2100)
					target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
					caster.Char.Class = 1
					caster.Char.LearnedSkill = 1 << (skill.Index - 24)
					party := &Party{Members: []*Player{caster, target}}
					caster.Party, target.Party = party, party
					w := worldWithNetworkedPlayers(caster, target)
					w.skills = map[int]model.SkillDef{skill.Index: skill}
					req := skillCastRequest{Skill: skill.Index, TargetID: target.ID}
					switch invalid {
					case "stale":
						req.TargetID = 999
					case "foreign party":
						target.Party = nil
					case "out of range":
						target.X += 50
					case "offline":
						target.InWorld = false
					case "dead":
						setPlayerCurHP(target.Char, 0)
					}
					mpBefore := playerCurMP(caster.Char)
					w.onSkillAttack(caster, req)
					if got := playerCurMP(caster.Char); got != mpBefore {
						t.Errorf("invalid target consumed mana: %d -> %d", mpBefore, got)
					}
					if _, ok := caster.SkillReady[skill.Index]; ok {
						t.Error("invalid target started cooldown")
					}
					for _, p := range []*Player{caster, target} {
						for _, affect := range p.Char.Affects {
							if affect.Type != 0 {
								t.Errorf("invalid cast applied affect to %s: %+v", p.Char.Name, affect)
							}
						}
						if got := p.Session.QueuedPacketsForTest(); got != 0 {
							t.Errorf("invalid cast published %d packets to %s", got, p.Char.Name)
						}
					}
				})
			}
		})
	}
}

func TestFoemaStatBuffCastRecastAndExpiration(t *testing.T) {
	for _, skill := range foemaBuffTestSkills() {
		if skill.Index == 46 {
			continue // The mana-shield contract is not covered by these stat formulas.
		}
		t.Run(skill.Name, func(t *testing.T) {
			w, p, _ := handlerTestWorld(t)
			clock := newFakeClock(time.Date(2026, 9, 16, 12, 0, 0, 0, time.UTC))
			w.clock = clock
			stateStore := &charStateMemoryStore{}
			w.store = stateStore
			p.Char.Class = 1
			p.Char.LearnedSkill = 1 << (skill.Index - 24)
			p.Char.Score.Mastery[3] = 41 // Exercises integer truncation in 43/44.
			p.Char.Score.AttackRun = 0x34
			w.recalcPlayer(p.Char)
			before := *effectiveScore(p.Char)
			base := p.Char.Score
			mastery := int(playerMastery(p.Char, 3))
			w.skills = map[int]model.SkillDef{skill.Index: skill}
			req := skillCastRequest{Skill: skill.Index, TargetID: p.ID}
			manaCost := uint32(skill.ManaSpent * (100 + mastery/2) / 100)
			w.onSkillAttack(p, req)
			if got := playerCurMP(p.Char); got != before.CurMP-manaCost {
				t.Fatalf("cast MP=%d, want %d", got, before.CurMP-manaCost)
			}
			checkStats := func(active bool) {
				t.Helper()
				wantAttack, wantDefense, wantSpeed := before.Attack, before.Defense, before.AttackRun
				if active {
					switch skill.Index {
					case 41:
						wantSpeed = before.AttackRun&0xF0 | minU32(15, (before.AttackRun&0x0F)+1)
					case 43:
						wantDefense += uint32(mastery/3 + 15)
					case 44: // Official TMSRVIN affect 9.
						flat, percent := mastery/3+15, 5
						if p.Char.Class == 1 && learnedLocal(p.Char, 44-24) {
							flat, percent = flat*5, 20
							if learnedLocal(p.Char, 47-24) {
								percent = 27
							}
						}
						wantAttack = (before.Attack + uint32(flat)) * uint32(100+percent) / 100
					}
				}
				got := effectiveScore(p.Char)
				if got.Attack != wantAttack || got.Defense != wantDefense || got.AttackRun != wantSpeed {
					t.Fatalf("active=%v stats attack/defense/speed=%d/%d/%x, want %d/%d/%x",
						active, got.Attack, got.Defense, got.AttackRun, wantAttack, wantDefense, wantSpeed)
				}
				if skill.Index == 41 && movementTilesPerSecond(p) != float64(clampInt(int(wantSpeed&0xF), 1, 7)) {
					t.Fatal("movement authority did not use the effective buff speed")
				}
				if p.Char.Score.Attack != base.Attack || p.Char.Score.Defense != base.Defense ||
					p.Char.Score.AttackRun != base.AttackRun || p.Char.Score.Mastery != base.Mastery {
					t.Fatal("buff modified base stats")
				}
			}
			checkStats(true)
			oldAffects, oldMP := p.Char.Affects, playerCurMP(p.Char)
			w.onSkillAttack(p, req)
			if playerCurMP(p.Char) != oldMP || p.Char.Affects != oldAffects {
				t.Fatal("recast during cooldown mutated MP or affects")
			}
			clock.Advance(time.Duration(skill.Delay) * time.Second)
			w.onSkillAttack(p, req)
			if playerCurMP(p.Char) != oldMP-manaCost ||
				!p.Char.Affects[0].ExpiresAt.After(oldAffects[0].ExpiresAt) {
				t.Fatal("valid recast did not charge MP and extend the buff")
			}
			for i := 0; i < 3; i++ {
				w.recalcPlayer(p.Char)
				checkStats(true)
			}
			count := 0
			for _, affect := range p.Char.Affects {
				if affect.Type == byte(skill.AffectType) {
					count++
				}
			}
			if count != 1 {
				t.Fatalf("recast left %d copies of affect %d", count, skill.AffectType)
			}
			clock.Advance(oldAffects[0].ExpiresAt.Sub(w.now()))
			w.tickPlayerAffects(w.now())
			checkStats(true) // The original expiry must not remove a refreshed buff.
			clock.Advance(p.Char.Affects[0].ExpiresAt.Sub(w.now()))
			w.tickPlayerAffects(w.now())
			checkStats(false)
			if p.Char.Affects[0].Type != 0 || stateStore.asyncSaves == 0 || stateStore.state == nil {
				t.Fatal("expiration did not remove and persist the buff")
			}
			for _, affect := range stateStore.state.Affects {
				if affect.Type == byte(skill.AffectType) {
					t.Fatal("expired buff remained in persisted state")
				}
			}
		})
	}
}

func TestSupportBuffManualRecastIsNotBlockedByAutomaticWindow(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 0
	p.Char.LearnedSkill = 1 << 5
	p.Char.Score.Mastery[1] = 40
	p.Char.Score.CurMP = 2_000
	p.Char.Score.MaxMP = 2_000
	for i := range p.Char.ShortSkill {
		p.Char.ShortSkill[i] = 0xFF
	}
	p.Char.ShortSkill[0] = 5
	applyScore(p.Char)
	skill := model.SkillDef{
		Index: 5, Name: "Aura", ManaSpent: 10, AffectTime: 99,
		TickType: 17, TickValue: 75,
	}
	w.skills = map[int]model.SkillDef{5: skill}

	// Manual recast must work even well before the automatic window.
	p.Char.Affects[0] = model.Affect{
		Type: 17, Value: 75, Level: 40,
		ExpiresAt: time.Now().Add(2 * time.Minute),
	}
	oldExpiry := p.Char.Affects[0].ExpiresAt
	oldMP := playerCurMP(p.Char)
	w.onSkillAttack(p, skillCastRequest{Skill: 5, Motion: 5, TargetID: p.ID})
	if !p.Char.Affects[0].ExpiresAt.After(oldExpiry) {
		t.Fatal("recast manual acima de 30 segundos foi ignorado")
	}
	if playerCurMP(p.Char) >= oldMP {
		t.Fatal("manual recast did not consume mana")
	}

}

func TestSkillAttackPvPBreaksHideAndPreservesOverkillState(t *testing.T) {
	caster, _ := networkedTestPlayer(1, "Caster", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
	w := worldWithNetworkedPlayers(caster, target)
	w.rng = fixedRNG{value: 0}
	w.store = &craftStore{}
	w.items = make(map[uint16]model.ItemDef)
	w.ghostShops = make(map[uint16]*GhostShop)
	caster.Char.Class = 0
	caster.Char.LearnedSkill = 1
	caster.Char.Score.Attack = 50_000
	caster.Char.Score.MagicAttack = 50_000
	caster.Char.Score.Int = 10_000
	caster.Char.Score.CurMP = 1000
	caster.Char.Score.MaxMP = 1000
	target.Char.Score.CurHP = 100
	target.Char.Score.MaxHP = 100
	caster.Char.Affects[0] = model.Affect{Type: 28, ExpiresAt: time.Now().Add(time.Minute)}
	applyScore(caster.Char)
	applyScore(target.Char)
	w.skills = map[int]model.SkillDef{0: {
		Index: 0, Name: "PvP", InstanceType: 2, InstanceValue: 5000,
		ManaSpent: 10, Delay: 1, Range: 6, MaxTarget: 1, Aggressive: 1,
	}}

	hpBefore := playerCurHP(target.Char)
	mpBefore := playerCurMP(caster.Char)
	w.onSkillAttack(caster, skillCastRequest{Skill: 0, TargetID: target.ID, Motion: 5})
	if playerCurHP(target.Char) != hpBefore || playerCurMP(caster.Char) != mpBefore ||
		!hasActiveAffect(caster.Char, 28) || !caster.SkillReady[0].IsZero() {
		t.Fatalf("PvP skill with mode disabled mutated state: hp=%d/%d mp=%d/%d hide=%t cooldown=%v",
			playerCurHP(target.Char), hpBefore, playerCurMP(caster.Char), mpBefore,
			hasActiveAffect(caster.Char, 28), caster.SkillReady[0])
	}

	caster.PKMode = true
	w.onSkillAttack(caster, skillCastRequest{Skill: 0, TargetID: target.ID, Motion: 5})
	if hasActiveAffect(caster.Char, 28) {
		t.Fatal("attacking did not remove Hide in Shadows")
	}
	if playerCurHP(target.Char) != 0 || target.DeadAt.IsZero() || target.LastAttackerID != caster.ID {
		t.Fatalf("morte PvP incompleta: hp=%d dead=%v attacker=%d",
			playerCurHP(target.Char), target.DeadAt, target.LastAttackerID)
	}
}

func TestBMSummonCastUsesW2PPFamilyAndLimitRules(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 2
	p.Char.Score.Int = 1000
	p.Char.Score.Con = 500
	p.Char.Score.Mastery[2] = 255
	applyScore(p.Char)

	if !w.castSummon(p, model.SkillDef{Index: 56, InstanceValue: 1}, 255) {
		t.Fatal("Condor was not summoned")
	}
	if want := summonCount(1, 255); len(w.summons) != want {
		t.Fatalf("Condor count=%d, want %d", len(w.summons), want)
	}
	for _, summon := range w.summons {
		if summon.SummonerID != p.ID || summon.SummonKind != summonKindBM ||
			summon.Def.Equip.Rosto.Index != summonTemplates[0].face {
			t.Fatalf("invalid Condor: %+v", summon)
		}
	}

	if w.castSummon(p, model.SkillDef{Index: 56, InstanceValue: 1}, 255) {
		t.Fatal("recast at the Condor limit should fail")
	}
	if w.castSummon(p, model.SkillDef{Index: 57, InstanceValue: 2}, 30) {
		t.Fatal("Boar should be rejected while a Condor remains in the group")
	}
	liveCondor := 0
	for _, summon := range w.summons {
		if !summon.Dead {
			if summon.Def.Equip.Rosto.Index != summonTemplates[0].face {
				t.Fatalf("familia de Condor foi substituida: face=%d", summon.Def.Equip.Rosto.Index)
			}
			liveCondor++
		}
	}
	if want := summonCount(1, 255); liveCondor != want {
		t.Fatalf("live Condors=%d, want %d", liveCondor, want)
	}
}

func TestBMSummonFailedRecastRestoresMana(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 2
	p.Char.LearnedSkill = 1 << 8 // skill global 56
	p.Char.Score.MaxMP, p.Char.Score.CurMP = 2_000, 2_000
	p.Char.Score.Mastery[2] = 30
	applyScore(p.Char)
	w.skills = map[int]model.SkillDef{
		56: {Index: 56, Name: "Condor", InstanceType: 11, InstanceValue: 1,
			ManaSpent: 100, MaxTarget: 1, Delay: 1},
	}

	w.onSkillAttack(p, skillCastRequest{Skill: 56, Motion: 5})
	if len(w.summons) != 1 {
		t.Fatalf("first cast created %d Condors, want 1", len(w.summons))
	}
	mpAfterFirst := playerCurMP(p.Char)
	if mpAfterFirst >= 2_000 {
		t.Fatalf("first cast did not consume MP: %d", mpAfterFirst)
	}

	p.SkillReady[56] = time.Time{}
	w.onSkillAttack(p, skillCastRequest{Skill: 56, Motion: 5})
	if got := playerCurMP(p.Char); got != mpAfterFirst {
		t.Fatalf("failed recast did not restore MP: got=%d want=%d", got, mpAfterFirst)
	}
}

func TestBMSummonPartialAllocationStillConsumesMana(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 2
	p.Char.LearnedSkill = 1 << 8
	p.Char.Score.MaxMP, p.Char.Score.CurMP = 2_000, 2_000
	p.Char.Score.Mastery[2] = 60 // two Condors, but only one free ID
	applyScore(p.Char)
	w.skills = map[int]model.SkillDef{56: {Index: 56, InstanceType: 11,
		InstanceValue: 1, ManaSpent: 100, MaxTarget: 1, Delay: 1}}
	for id := int(firstMobID); id < int(^uint16(0)); id++ {
		w.mobsByID[uint16(id)] = &Mob{ID: uint16(id)}
	}
	w.onSkillAttack(p, skillCastRequest{Skill: 56, Motion: 5})
	if len(w.summons) != 1 || playerCurMP(p.Char) >= 2_000 {
		t.Fatalf("partial summon was free: summons=%d mp=%d", len(w.summons), playerCurMP(p.Char))
	}
}

func TestContractSummonReplacesPreviousAndFollowsOwnerTarget(t *testing.T) {
	w, owner, _ := handlerTestWorld(t)
	template := &model.VolatileSummon{
		Name: "Knight", Face: 300, HP: 500, Attack: 300, Defense: 100,
		MoveSpeed: 4, AttackRange: 1,
	}
	mustCommitContractSummon(t, w, owner, template)
	var first *Mob
	for _, summon := range w.summons {
		first = summon
	}
	template.Name, template.Face = "Archer", 301
	mustCommitContractSummon(t, w, owner, template)
	if !first.Dead {
		t.Fatal("previous contract summon remained alive")
	}
	live := 0
	for _, summon := range w.summons {
		if !summon.Dead && summon.SummonKind == summonKindContract {
			live++
		}
	}
	if live != 1 {
		t.Fatalf("single active contract limit failed: %d", live)
	}
}

func TestContractSummonPreflightFailureDoesNotConsumeItem(t *testing.T) {
	rule := model.VolatileRule{Action: "summon_contract", Consume: true,
		Summon: &model.VolatileSummon{Name: "Invalid", HP: 500}}
	w, p, st := useItemWorld(rule)

	w.onUseItem(p.Session, useItemPacket(0, 0))

	if p.Char.Inv[0].Index != 100 || st.saves != 0 || len(w.summons) != 0 {
		t.Fatalf("invalid preflight mutated state: item=%d saves=%d summons=%d",
			p.Char.Inv[0].Index, st.saves, len(w.summons))
	}
}

func TestContractSummonPersistenceFailureKeepsItemAndPreviousSummon(t *testing.T) {
	template := &model.VolatileSummon{Name: "Archer", Face: 301, HP: 500,
		Attack: 300, Defense: 100, MoveSpeed: 4, AttackRange: 5}
	rule := model.VolatileRule{Action: "summon_contract", Consume: true, Summon: template}
	w, p, st := useItemWorld(rule)
	oldTemplate := &model.VolatileSummon{Name: "Knight", Face: 300, HP: 500,
		Attack: 300, Defense: 100, MoveSpeed: 4, AttackRange: 1}
	mustCommitContractSummon(t, w, p, oldTemplate)
	var previous *Mob
	for _, summon := range w.summons {
		previous = summon
	}
	st.err = errors.New("database unavailable")
	beforePackets := p.Session.QueuedPacketsForTest()

	w.onUseItem(p.Session, useItemPacket(0, 0))

	if p.Char.Inv[0].Index != 100 || previous == nil || previous.Dead ||
		w.mobsByID[previous.ID] != previous || len(w.summons) != 1 {
		t.Fatalf("save failure replaced summon/item: item=%d old=%+v summons=%d",
			p.Char.Inv[0].Index, previous, len(w.summons))
	}
	if got := p.Session.QueuedPacketsForTest(); got != beforePackets+1 {
		t.Fatalf("contract rollback did not republish the slot: %d -> %d", beforePackets, got)
	}
}

func TestGoldenShieldGoldCostPersistsBeforeSkillPublication(t *testing.T) {
	newPlayer := func(t *testing.T) (*World, *Player, *craftStore) {
		t.Helper()
		w, p, st := handlerTestWorld(t)
		p.Char.Class = 3
		p.Char.LearnedSkill = 1 << 14 // global 86 Coin Armor, local 14
		p.Char.Gold = 1_000
		p.Char.Score.Mastery[2] = 5 // W2PP cost: 100 gold per mastery point = 500
		p.Char.Score.CurMP = 1_000
		p.Char.Score.MaxMP = 1_000
		applyScore(p.Char)
		w.skills = map[int]model.SkillDef{86: {
			Index: 86, Name: "Coin_Armor", ManaSpent: 10, Delay: 1,
			AffectType: 1, AffectValue: 10, AffectTime: 10,
		}}
		return w, p, st
	}

	t.Run("success", func(t *testing.T) {
		w, p, st := newPlayer(t)
		w.onSkillAttack(p, skillCastRequest{Skill: 86, TargetID: p.ID, Motion: 5})
		if st.saves != 1 || p.Char.Gold != 500 {
			t.Fatalf("incorrect Coin Armor cost: saves=%d gold=%d", st.saves, p.Char.Gold)
		}
	})

	t.Run("rollback", func(t *testing.T) {
		w, p, st := newPlayer(t)
		st.err = errors.New("database unavailable")
		beforeMP := playerCurMP(p.Char)
		beforePackets := p.Session.QueuedPacketsForTest()
		w.onSkillAttack(p, skillCastRequest{Skill: 86, TargetID: p.ID, Motion: 5})
		if p.Char.Gold != 1_000 || playerCurMP(p.Char) != beforeMP ||
			p.Session.QueuedPacketsForTest() != beforePackets {
			t.Fatalf("save failure published/charged the skill: gold=%d mp=%d packets=%d",
				p.Char.Gold, playerCurMP(p.Char), p.Session.QueuedPacketsForTest())
		}
	})
}

func mustCommitContractSummon(t *testing.T, w *World, owner *Player, template *model.VolatileSummon) {
	t.Helper()
	plan, ok := w.planContractSummon(owner, template)
	if !ok {
		t.Fatal("valid contract did not create a summon plan")
	}
	w.commitContractSummon(plan)
}
