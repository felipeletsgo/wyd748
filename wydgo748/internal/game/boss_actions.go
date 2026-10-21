package game

import (
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Boss actions mutate the world only on the World goroutine, after the
// authoritative validation in boss_events.go.

// executeBossAction applies a validated action and propagates the depth limit
// for chained events.
func (w *World) executeBossAction(boss *BossRuntime, mob *Mob, action BossActionDef, event BossEvent, depth int) {
	switch action.Kind {
	case ActionChangePhase:
		w.bossChangePhase(boss, mob, action, depth)
	case ActionCastSkill:
		w.bossCastSkill(boss, mob, action, event)
	case ActionSummon:
		w.bossSummonAdds(boss, mob, action)
	case ActionApplyEffect:
		w.bossApplyEffect(boss, mob, action)
	case ActionRemoveEffect:
		w.bossRemoveEffect(boss, mob, action)
	}
}

// bossChangePhase announces the new phase and cancels the previous phase's
// pending action.
func (w *World) bossChangePhase(boss *BossRuntime, mob *Mob, action BossActionDef, depth int) {
	phase, ok := boss.Profile.Phases[action.Phase]
	if !ok || boss.Phase == action.Phase {
		return
	}
	previous := boss.Phase
	if boss.Pending != nil {
		// A new generation invalidates the scheduled callback.
		boss.Generation++
		boss.Pending = nil
	}
	boss.Phase = action.Phase
	log.Printf("BOSS %q: phase %d -> %d (%s)", boss.Profile.ID, previous, phase.ID, phase.Name)
	if phase.EnterMessage != "" {
		w.announceToMobView(mob, phase.EnterMessage)
	}
	_ = depth // Reserved: phase entry does not emit a separate event in v1.
}

// bossCastSkill applies direct damage to the event's validated living player.
func (w *World) bossCastSkill(boss *BossRuntime, mob *Mob, action BossActionDef, event BossEvent) {
	target := w.playerByID(event.SourceID)
	if !w.mobCanTargetPlayer(mob, target) {
		return
	}
	// Use the configured range, falling back to melee range.
	reach := action.CastRange
	if reach <= 0 {
		reach = mobAttackRange
	}
	if chebyshev(mob.X, mob.Y, target.X, target.Y) > reach {
		return // The target left range after admission.
	}
	if !w.combatLineOfSight(mob.X, mob.Y, target.X, target.Y) {
		return
	}
	skill, known := w.skills[action.SkillID]
	if !known {
		// Missing skills must not produce fabricated damage.
		log.Printf("BOSS %q: skill %d missing from SkillData.csv; skipped",
			boss.Profile.ID, action.SkillID)
		return
	}
	if action.Message != "" {
		w.announceToMobView(mob, action.Message)
	}

	now := w.now()
	if !combatRollHits(mobVersusPlayerAccuracy(mob.Def, target.Char), w.intn) {
		w.sendToMobView(mob, func() []byte {
			return wire.SkillHits(mob.ID, mob.X, mob.Y, target.X, target.Y,
				0, mob.Def.Score.MaxMP, int16(skill.Index), 0, 0, 1,
				[]wire.SkillTarget{{ID: target.ID, Miss: true, MaxHP: playerMaxHP(target.Char)}})
		})
		log.Printf("BOSS %q: skill %d (%s) missed player id=%d",
			boss.Profile.ID, action.SkillID, skill.Name, target.ID)
		return
	}
	damage := bossSkillDamageWithRNG(mob, target, skill, w.intn, now)
	w.applyMobDamageToPlayer(mob, target, damage, now, func(applied uint32) []byte {
		// SkillHitExtended (0x36C + 0x39D) selects the skill visual rather than
		// melee. Motion/mastery stay zero; the skill index supplies the effect.
		return wire.SkillHitExtended(mob.ID, target.ID, mob.X, mob.Y, target.X, target.Y,
			applied, playerMaxHP(target.Char), 0, mob.Def.Score.MaxMP, int16(skill.Index), 0, 0)
	})
	log.Printf("BOSS %q: skill %d (%s) on player id=%d damage=%d",
		boss.Profile.ID, action.SkillID, skill.Name, target.ID, damage)
}

// bossSkillDamage uses the magical core, target defense and elemental
// resistance instead of the ordinary physical melee path.
func bossSkillDamage(m *Mob, target *Player, skill model.SkillDef) uint32 {
	return bossSkillDamageWithRNG(m, target, skill, realRNG{}.Intn, time.Now())
}

func (w *World) bossSkillDamage(m *Mob, target *Player, skill model.SkillDef) uint32 {
	return bossSkillDamageWithRNG(m, target, skill, w.intn, w.now())
}

func bossSkillDamageWithRNG(m *Mob, target *Player, skill model.SkillDef, intn func(int) int, now time.Time) uint32 {
	if m == nil || m.Def == nil || target == nil || target.Char == nil {
		return 0
	}
	extended := m.Def.Score
	// The magical core includes MagicAttack, INT and level contributions.
	core := int(extended.MagicAttack) + int(extended.Int)/4 + int(extended.Level)
	if core <= 0 {
		// NPCs without a magical core use their effective physical attack.
		core = effectiveMobAttackAt(m, now)
	}
	// InstanceValue scales the catalog skill effect.
	if skill.InstanceValue > 0 {
		core = core * (100 + skill.InstanceValue) / 100
	}
	damage := skillFinalDamageWithRNG(core, playerDefense(target.Char), 0, intn)
	damage = applySkillResistance(damage, skill.InstanceType, playerElementalResists(target.Char), false)
	return uint32(clampInt(damage, 1, int(maxScoreValue)))
}

// bossSummonAdds creates ordinary mobs from a catalog NPC. The encounter
// tracks them only for counting and cleanup; normal AI and death still apply.
func (w *World) bossSummonAdds(boss *BossRuntime, mob *Mob, action BossActionDef) {
	template := w.npcDefByName(action.SummonNPC)
	if template == nil {
		log.Printf("BOSS %q: add NPC %q missing from catalog", boss.Profile.ID, action.SummonNPC)
		return
	}
	// max_alive bounds simultaneous adds during long encounters.
	wanted := action.SummonCount
	if action.SummonMax > 0 {
		if free := action.SummonMax - boss.addsAlive(); free < wanted {
			wanted = free
		}
	}
	if wanted <= 0 {
		return
	}
	created := 0
	for i := 0; i < wanted; i++ {
		x, y := w.findFreeMobPosition(mobGameplaySpace(mob), mob.X, mob.Y, 3)
		mobID := w.allocMobID()
		if mobID == 0 {
			log.Printf("BOSS %q: summoning stopped: mob IDs exhausted", boss.Profile.ID)
			break
		}
		add := &Mob{
			ID: mobID, Def: template, X: x, Y: y,
			HP: template.Score.MaxHP, GenerIndex: -1,
			InstanceID: mob.InstanceID,
			// LeaderID links the add to its encounter boss.
			LeaderID: mob.ID,
		}
		w.appendMobInstance(add)
		w.publishMobSpawn(add) // Register spatial state and publish to clients.
		boss.Adds[add.ID] = struct{}{}
		created++
	}
	if action.Message != "" {
		w.announceToMobView(mob, action.Message)
	}
	log.Printf("BOSS %q: summoned %d adds %q", boss.Profile.ID, created, action.SummonNPC)
}

// bossApplyEffect applies a boss affect, such as a shield or enrage.
func (w *World) bossApplyEffect(boss *BossRuntime, mob *Mob, action BossActionDef) {
	if !setMobOwnedMobAffectAt(mob, mob, action.AffectType, action.AffectValue,
		action.AffectLevel, action.AffectDuration, w.now()) {
		return
	}
	w.publishMobAffects(mob)
	if action.Message != "" {
		w.announceToMobView(mob, action.Message)
	}
	log.Printf("BOSS %q: effect %d applied", boss.Profile.ID, action.AffectType)
}

// bossRemoveEffect removes a boss affect, such as the shield after add deaths.
func (w *World) bossRemoveEffect(boss *BossRuntime, mob *Mob, action BossActionDef) {
	if !clearMobAffect(mob, action.AffectType) {
		return
	}
	w.publishMobAffects(mob)
	if action.Message != "" {
		w.announceToMobView(mob, action.Message)
	}
	log.Printf("BOSS %q: effect %d removed", boss.Profile.ID, action.AffectType)
}

// bossShieldAffect uses the existing immunity visual (19); ordinary mob
// affect rules (1, 3, 10 and 12) do not consume it.
const bossShieldAffect = 19

// bossMitigateDamage applies the active shield's absorbed percentage.
// Ordinary mob affects do not provide this encounter-specific mitigation.
func (w *World) bossMitigateDamage(m *Mob, damage uint32) uint32 {
	if damage == 0 || m == nil || w.bossFor(m.ID) == nil {
		return damage
	}
	affect := activeMobAffectAt(m, bossShieldAffect, w.now())
	if affect == nil {
		return damage
	}
	absorbed := clampInt(affect.Value, 0, 100)
	return uint32(uint64(damage) * uint64(100-absorbed) / 100)
}

// clearMobAffect reports whether an affect existed, preventing false removal
// announcements.
func clearMobAffect(m *Mob, affectType byte) bool {
	if m == nil || affectType == 0 {
		return false
	}
	removed := false
	for i := range m.Affects {
		if m.Affects[i].Type == affectType {
			m.Affects[i] = model.Affect{}
			removed = true
		}
	}
	return removed
}

// announceToMobView uses the NPC panel channel for players viewing the mob.
func (w *World) announceToMobView(m *Mob, message string) {
	if message == "" {
		return
	}
	w.sendToMobView(m, func() []byte { return wire.MessagePanel(message) })
}

// npcDefByName accepts catalog names and NPCGener underscore names.
func (w *World) npcDefByName(name string) *model.NPCDef {
	wanted := generName(name)
	for i := range w.npcs {
		if generName(w.npcs[i].Name) == wanted {
			return &w.npcs[i]
		}
	}
	return nil
}
