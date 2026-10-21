package game

import (
	"encoding/binary"
	"fmt"
	"log"
	"sort"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

func ultimateSkillGold(ch *model.Char) uint32 {
	if isCelestialEvolution(ch) {
		return 100_000_000
	}
	if isArch(ch) {
		return 50_000_000
	}
	return 5_000_000
}

func (w *World) onLearnSkill(s *net.Session, p *Player, itemIndex int) {
	w.onLearnSkillAtMaster(s, p, itemIndex, 0)
}

// onLearnSkillAtMaster validates legacy shop (0x379) and 7.48 ApplyBonus type 2
// learning requests. TargetID confirms the server-selected master; it cannot
// select a different master or bypass the current interaction boundary.
func (w *World) onLearnSkillAtMaster(s *net.Session, p *Player, itemIndex int, requestedMasterID uint16) {
	reject := func(message string) {
		if s != nil {
			s.Send(wire.MessagePanel(message))
		}
	}
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 {
		reject("The character could not be validated to learn this skill.")
		return
	}
	ch := p.Char
	var proposed *model.Account
	if w.criticalWrites != nil {
		if w.shuttingDown || p.PersistencePoisoned || s == nil || s != p.Session || s.IsClosed() ||
			w.criticalPlayerReserved(p) {
			return
		}
		if p.Account == nil || p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) ||
			p.Char != &p.Account.Chars[p.CharSlot] {
			reject("The character could not be validated to learn this skill.")
			return
		}
		// Recalculation also changes persisted point balances and base vitals.
		// Validate and plan on a private character, never on a reserved/live one.
		proposed = accountStateSnapshot(p.Account)
		ch = &proposed.Chars[p.CharSlot]
	}
	if requestedMasterID != 0 && requestedMasterID != p.ShopNPC {
		log.Printf("[#%d] invalid skill master requested=%d open=%d", s.ID, requestedMasterID, p.ShopNPC)
		reject("Invalid skill master.")
		return
	}
	master, interactionErr := w.resolveNPCInteraction(p, p.ShopNPC)
	if interactionErr != nil || master.Def.Score == nil || master.Def.Score.Merchant != skillMasterMerchant {
		log.Printf("[#%d] skill learning without a valid open master", s.ID)
		reject("Open a valid skill master first.")
		return
	}
	found := false
	for _, item := range master.Def.Vende {
		if int(item.Index) == itemIndex {
			found = true
			break
		}
	}
	if !found || itemIndex < 5000 || itemIndex > 5095 {
		log.Printf("[#%d] skill item=%d not sold by the open master", s.ID, itemIndex)
		reject("This skill is not available from this master.")
		return
	}
	globalSkill := itemIndex - 5000
	skillClass, localSkill := globalSkill/24, globalSkill%24
	if skillClass != int(ch.Class) {
		log.Printf("[#%d] attempted to learn another class's skill: item=%d", s.ID, itemIndex)
		reject("This skill belongs to another class.")
		return
	}
	skill, ok := w.skills[globalSkill]
	itemDef, itemOK := w.items[uint16(itemIndex)]
	if !ok || !itemOK {
		log.Printf("[#%d] skill %d missing from catalog", s.ID, globalSkill)
		reject("This skill's data is unavailable.")
		return
	}
	bit := uint32(1) << localSkill
	if ch.LearnedSkill&bit != 0 {
		reject("This skill has already been learned.")
		return
	}
	w.recalcPlayer(ch)
	// Only Mortal uses the book level requirement. Arch, Celestial and
	// SubCelestial still validate class, points, mastery, chain and gold.
	levelRequirementMet := isArch(ch) || isCelestialEvolution(ch) ||
		int(playerLevel(ch)) >= itemDef.ReqLevel
	if int(playerSkillPoints(ch)) < skill.SkillPoint {
		log.Printf("[#%d] insufficient points for skill=%d points=%d/%d", s.ID,
			globalSkill, playerSkillPoints(ch), skill.SkillPoint)
		reject("Insufficient Skill Points to learn this skill.")
		return
	}
	if !levelRequirementMet {
		log.Printf("[#%d] insufficient level for skill=%d level=%d/%d", s.ID,
			globalSkill, playerLevel(ch), itemDef.ReqLevel)
		reject("Insufficient level to learn this skill.")
		return
	}
	if int(playerMastery(ch, 1)) < itemDef.ReqInt ||
		int(playerMastery(ch, 2)) < itemDef.ReqDex ||
		int(playerMastery(ch, 3)) < itemDef.ReqCon {
		log.Printf("[#%d] insufficient mastery for skill=%d mastery=%v", s.ID,
			globalSkill, effectiveScore(ch).Mastery)
		reject("Insufficient mastery to learn this skill.")
		return
	}
	oldGold, oldLearned := ch.Gold, ch.LearnedSkill
	if localSkill == 7 || localSkill == 15 || localSkill == 23 {
		if ch.LearnedSkill&((1<<7)|(1<<15)|(1<<23)) != 0 {
			reject("You have already learned the ultimate skill from another branch.")
			return
		}
		branchStart := localSkill - 7
		for i := branchStart; i < localSkill; i++ {
			if ch.LearnedSkill&(uint32(1)<<i) == 0 {
				reject("Learn all previous skills in this branch first.")
				return
			}
		}
		cost := ultimateSkillGold(ch)
		if ch.Gold < cost {
			reject("Insufficient gold to learn the ultimate skill.")
			return
		}
		ch.Gold -= cost
	}
	ch.LearnedSkill |= bit
	w.recalcPlayer(ch)
	filterShortSkills(ch)
	if proposed != nil {
		if err := w.queueSkillLearning(p, proposed, globalSkill, skill); err != nil {
			log.Printf("[#%d] submit learned skill: %v", s.ID, err)
			reject("The skill could not be saved. Please try again.")
		}
		return
	}
	// Persist before confirmation: learning changes the skill mask and may
	// deduct gold. Restore both on failure, as with refinement, trade and quests.
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Gold, p.Char.LearnedSkill = oldGold, oldLearned
		w.recalcPlayer(p.Char)
		filterShortSkills(p.Char)
		log.Printf("[#%d] save learned skill: %v", s.ID, err)
		reject("The skill could not be saved. Please try again.")
		return
	}
	s.Send(playerScorePacket(p))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	s.Send(wire.SetShortSkill(p.ID, p.Char.ShortSkill))
	log.Printf("[#%d] learned skill=%d %q cost=%d remaining points=%d", s.ID,
		globalSkill, skill.Name, skill.SkillPoint, playerSkillPoints(p.Char))
}

type skillCastRequest struct {
	Skill             int
	Motion            byte
	TargetID          uint16
	SecondaryTargetID uint16
	TargetX           uint16
	TargetY           uint16
}

func parseAttackSkill(pkt []byte) skillCastRequest {
	// Unlike p39D_754/p36C in PacketProtocolV754, 7.48 always sends SkillId@24
	// and Motion@28, including multi-target 0x36C packets after offset 44.
	if len(pkt) >= 30 {
		req := skillCastRequest{
			Skill:   int(int16(binary.LittleEndian.Uint16(pkt[24:26]))),
			Motion:  pkt[28],
			TargetX: binary.LittleEndian.Uint16(pkt[20:22]),
			TargetY: binary.LittleEndian.Uint16(pkt[22:24]),
		}
		// The p39D/p39E/p367 family carries its first target at offset 44.
		// Validate that ID; ignore client-provided damage and other state fields.
		if len(pkt) >= 46 {
			req.TargetID = binary.LittleEndian.Uint16(pkt[44:46])
		}
		if len(pkt) >= 50 {
			req.SecondaryTargetID = binary.LittleEndian.Uint16(pkt[48:50])
		}
		return req
	}
	return skillCastRequest{Skill: -1, Motion: 5}
}

func skillManaCost(skill model.SkillDef, mastery, saveMana int) int {
	cost := skill.ManaSpent * (100 + mastery/2) / 100
	if saveMana < 0 {
		saveMana = 0
	}
	if saveMana > 99 {
		saveMana = 99
	}
	return cost * (100 - saveMana) / 100
}

// skillVisualLevel is sent to the client as SkillParm. A zero value makes the
// 7.48 client suppress several target particles even though the server has
// already accepted and applied the skill.
func skillVisualLevel(mastery int) byte {
	return byte(clampInt(mastery, 1, 255))
}

func explosionBashBaseDamage(baseDamage, intelligence, currentMP int) int {
	return baseDamage + intelligence + currentMP
}

func skillCooldownDuration(skill model.SkillDef) time.Duration {
	if skill.Index == 47 || skill.Index == 102 { // Cancellation/Soul Limit: W2PP forces skilldelay=1.
		return time.Second
	}
	return time.Duration(skill.Delay) * time.Second
}

// TK has two magical families (Trust and Magic Sword) and one physical family
// (Trans). Classification uses the class-local skill index.
func tkTransformationSkill(skillIndex int) bool {
	return skillIndex >= 0 && skillIndex < 24 && (skillIndex%24)/8 == 1
}

// baseSkillDamage adapts the historical 7.59 BASE_GetSkillDamage algorithm,
// including its weather modifiers for the caster's position (weather.go).
func (w *World) baseSkillDamage(ch *model.Char, skill model.SkillDef) int {
	local := skill.Index % 24
	kind := local/8 + 1
	special := int(playerMastery(ch, kind))
	level, base := clampInt(int(playerLevel(ch)), 0, 400), skill.InstanceValue
	weapon := w.equipmentDamage(ch)
	damage := 0
	if skill.InstanceType == 0 {
		switch skill.Index {
		case 11:
			damage = special/10 + skill.AffectValue
		case 13:
			damage = 3*special + skill.AffectValue
		case 41:
			damage = special/25 + 2
		case 43:
			damage = skill.AffectValue + special/3 + 15
		case 44:
			damage = 5 * (special/3 + 15)
		case 45:
			damage = special/10 + skill.AffectValue
		}
		return maxInt(1, damage)
	}
	if skill.InstanceType < 1 || skill.InstanceType > 5 {
		switch skill.InstanceType {
		case 6:
			damage = 3*special/2 + base
			if skill.Index == 29 && ch.LearnedSkill&0x80 != 0 {
				damage = 120 * damage / 100
			}
		case 11:
			damage = base
		default:
			damage = 2 * int(effectiveScore(ch).MagicAmp)
		}
		return maxInt(1, damage)
	}

	adc := special%10 > 5
	levelPart := level / 2
	if adc {
		levelPart = level
	}
	// Include base MagicAttack just as the displayed equipment.go calculation
	// does. Omitting it made a high persisted value affect the display but not
	// the skill, which then used only its INT component.
	magicAttackBase := 0
	if ch.Score != nil {
		magicAttackBase = int(ch.Score.MagicAttack)
	}
	if skill.Index == 97 {
		damage = 15*level + base
	} else {
		switch ch.Class {
		case 0:
			if tkTransformationSkill(skill.Index) {
				damage = 3*weapon + 3*playerStr(ch) + levelPart + special + base
			} else {
				damage = levelPart + special + base + weapon +
					playerInt(ch)/4 + playerInt(ch)/40 + magicAttackBase
			}
		case 1, 2:
			damage = playerInt(ch)/30 + playerInt(ch)/3 + levelPart + special + base + magicAttackBase
			if adc {
				damage += special
			}
		case 3:
			if skill.Index == 79 {
				return maxInt(1, playerAttack(ch))
			}
			damage = 3*weapon + 3*playerStr(ch) + level/2 + special + base
		}
	}
	damage = applyWeatherDamage(damage, skillWeatherAt(w.weather, ch.X, ch.Y), skill)
	if skill.Index != 97 {
		if ch.Class != 3 && (ch.Class != 0 || !tkTransformationSkill(skill.Index)) {
			damage = (4*int(effectiveScore(ch).MagicAmp) + 100) * damage / 100
		}
		damage = 5 * damage / 4
		if skillUsesMagic(ch, skill) {
			// Timed mounts keep their exact percentage contract, also used by
			// the displayed score. Adult mounts already contribute to MagicAmp.
			premium := w.premiumMountBonus(ch, w.now())
			damage = int(int64(damage) * int64(100+premium.magicPercent) / 100)
		}
	}
	branch := local / 8
	if int(ch.Class) < 4 && ch.LearnedSkill&(uint32(1)<<(branch*8+7)) != 0 {
		multipliers := [4][3]int{{115, 120, 115}, {110, 115, 115}, {110, 100, 100}, {110, 110, 120}}
		damage = damage * multipliers[ch.Class][branch] / 100
	}
	return maxInt(1, damage)
}

func skillUsesMagic(ch *model.Char, skill model.SkillDef) bool {
	if ch == nil || skill.InstanceType < 1 || skill.InstanceType > 5 {
		return false
	}
	return ch.Class != 3 && (ch.Class != 0 || !tkTransformationSkill(skill.Index))
}

func resistanceForInstance(instance int, resist model.ElementalResists) int {
	switch instance {
	case 1, 2:
		return int(resist.Fire)
	case 3:
		return int(resist.Ice)
	case 4:
		return int(resist.Sacred)
	case 5:
		return int(resist.Thunder)
	default:
		return 0
	}
}

// applySkillResistance follows the historical 7.59 _MSG_Attack algorithm:
// mobs use half resistance before the (150-resist)% multiplier.
func applySkillResistance(damage, instance int, resist model.ElementalResists, targetIsMob bool) int {
	if instance < 1 || instance > 5 {
		return damage
	}
	value := clampInt(resistanceForInstance(instance, resist), 0, 100)
	if targetIsMob {
		value /= 2
	}
	return damage * (150 - value) / 100
}

func skillFinalDamageWithRNG(damage, defense, mastery int, intn func(int) int) int {
	damage -= defense / 2
	if mastery > 15 {
		mastery = 15
	}
	rangeSize := 21 - mastery
	if rangeSize < 1 {
		rangeSize = 1
	}
	damage = damage * (intn(rangeSize) + mastery + 90) / 100
	if damage < -50 {
		damage = 0
	} else if damage < 0 {
		damage = (damage + 50) / 10
	} else if damage <= 45 {
		damage = damage*5/4 + 5
	}
	if damage < 1 {
		damage = 1
	}
	if damage > int(maxScoreValue) {
		damage = int(maxScoreValue)
	}
	return damage
}

func skillFinalDamage(damage, defense, mastery int) int {
	return skillFinalDamageWithRNG(damage, defense, mastery, realRNG{}.Intn)
}

func (w *World) skillFinalDamage(damage, defense, mastery int) int {
	return skillFinalDamageWithRNG(damage, defense, mastery, w.intn)
}

func skillDamageMastery(ch *model.Char) int {
	if ch != nil && ch.Class == 0 && learnedLocal(ch, 14) { // Combat Awareness
		return clampInt(int(playerMastery(ch, 2))/20, 0, 15)
	}
	return 0
}

func offensiveSkillAreaRadius(ch *model.Char, skill model.SkillDef) int {
	switch skill.TargetType {
	case 3:
		return 1
	case 4:
		return 2
	case 6:
		return 3
	case 5:
		// WYD 7.48 client: Special[3]/75, capped at 3, plus a base radius of 3.
		return 3 + clampInt(int(playerMastery(ch, 3))/75, 0, 3)
	default:
		return 0
	}
}

func offensiveSkillTargetLimit(ch *model.Char, skill model.SkillDef) int {
	if skill.Index == 95 { // preserve the existing native multi-hit special case
		return 1
	}
	if offensiveSkillAreaRadius(ch, skill) > 0 {
		return 13
	}
	return clampInt(skill.MaxTarget, 1, 2)
}

func offensiveSkillWireMaxTargets(ch *model.Char, skill model.SkillDef) int {
	if skill.Index == 95 {
		return clampInt(skill.MaxTarget, 1, 13)
	}
	if offensiveSkillAreaRadius(ch, skill) > 0 {
		return 13
	}
	return clampInt(skill.MaxTarget, 1, 13)
}

func skillAreaContains(casterX, casterY, primaryX, primaryY, candidateX, candidateY uint16,
	skill model.SkillDef, radius int) bool {
	if radius <= 0 {
		return false
	}
	if skill.TargetType != 5 {
		return chebyshev(primaryX, primaryY, candidateX, candidateY) <= radius
	}
	// TargetType 5 is the WYD 7.48 directional 90-degree cone. Use integer vector
	// math so target selection is deterministic on every platform.
	ax, ay := int(primaryX)-int(casterX), int(primaryY)-int(casterY)
	bx, by := int(candidateX)-int(casterX), int(candidateY)-int(casterY)
	if maxInt(absInt(bx), absInt(by)) > radius {
		return false
	}
	dot := ax*bx + ay*by
	cross := ax*by - ay*bx
	return dot > 0 && absInt(cross) <= dot
}

func (w *World) skillMonsterTargets(p *Player, req skillCastRequest, skill model.SkillDef) []*Mob {
	bonusRange := 0
	if specialSkillLearned(p.Char, 101) {
		bonusRange = 1
	}
	castRange := uint16(maxInt(attackRange, skill.Range+bonusRange))
	primary := w.mobByID(req.TargetID)
	if primary == nil || primary.Dead || primary.HP == 0 || !w.playerCanInteractWithMob(p, primary) ||
		chebyshev(p.X, p.Y, primary.X, primary.Y) > int(castRange) ||
		!w.combatLineOfSight(p.X, p.Y, primary.X, primary.Y) {
		return nil
	}
	limit := offensiveSkillTargetLimit(p.Char, skill)
	targets := []*Mob{primary}
	if limit == 1 {
		return targets
	}
	area := offensiveSkillAreaRadius(p.Char, skill)
	if area == 0 {
		secondary := w.mobByID(req.SecondaryTargetID)
		if secondary != nil && secondary != primary && !secondary.Dead && secondary.HP > 0 &&
			w.playerCanInteractWithMob(p, secondary) &&
			chebyshev(p.X, p.Y, secondary.X, secondary.Y) <= maxInt(1, skill.Range+bonusRange) &&
			w.combatLineOfSight(p.X, p.Y, secondary.X, secondary.Y) {
			targets = append(targets, secondary)
		}
		return targets
	}
	centerX, centerY := primary.X, primary.Y
	if skill.TargetType == 5 {
		centerX, centerY = p.X, p.Y
	}
	candidates := w.nearbyMobs(centerX, centerY, area)
	sort.Slice(candidates, func(i, j int) bool {
		di := chebyshev(centerX, centerY, candidates[i].X, candidates[i].Y)
		dj := chebyshev(centerX, centerY, candidates[j].X, candidates[j].Y)
		if di != dj {
			return di < dj
		}
		return candidates[i].ID < candidates[j].ID
	})
	for _, candidate := range candidates {
		if len(targets) >= limit {
			break
		}
		if candidate == primary || candidate.Dead || !w.playerCanInteractWithMob(p, candidate) {
			continue
		}
		// Area damage propagates from the primary impact point. A wall between
		// that point and a secondary target blocks the secondary hit.
		losX, losY := primary.X, primary.Y
		if skill.TargetType == 5 {
			losX, losY = p.X, p.Y
		}
		if skillAreaContains(p.X, p.Y, primary.X, primary.Y, candidate.X, candidate.Y, skill, area) &&
			w.combatLineOfSight(losX, losY, candidate.X, candidate.Y) {
			targets = append(targets, candidate)
		}
	}
	return targets
}

func (w *World) onSkillAttack(p *Player, req skillCastRequest) {
	if p == nil || p.Char == nil || w.criticalPlayerReserved(p) {
		return
	}
	skillIndex, motion := req.Skill, req.Motion
	local := skillIndex - int(p.Char.Class)*24
	special := specialSkillLearned(p.Char, skillIndex)
	if !special && (local < 0 || local >= 24 || p.Char.LearnedSkill&(uint32(1)<<local) == 0) {
		return
	}
	skill, ok := w.skills[skillIndex]
	if !ok || skill.Passive != 0 && skillIndex != 47 {
		return
	}
	now := w.now()
	if now.Before(p.SkillReady[skillIndex]) {
		return
	}
	if !w.skillWeaponAllowed(p.Char, skillIndex) {
		return
	}
	if skillIndex == backpackSkill {
		// Backpack has no cast or affect: the client opens Cargo locally.
		return
	}
	kind := local/8 + 1
	if special {
		kind = 4
	}
	mastery := int(playerMastery(p.Char, kind))
	var targets []*Mob
	var playerTargets []*Player
	var supportTargets []*Player
	if skill.Aggressive != 0 {
		if skillIndex == 98 {
			if !w.canCastThornWall(p, req, skill) {
				return
			}
		} else {
			playerTargets = w.skillPlayerTargets(p, req, skill)
			if len(playerTargets) == 0 {
				targets = w.skillMonsterTargets(p, req, skill)
			}
			if len(targets) == 0 && len(playerTargets) == 0 {
				return
			}
		}
	}
	if skill.Aggressive == 0 {
		// Validate the intent before spending MP or starting cooldown, just as
		// offensive skills do. Reuse this list when applying the support skill.
		supportTargets = w.supportTargets(p, req, skill)
		if len(supportTargets) == 0 {
			return
		}
	}
	if skillIndex == 97 && (w.groundCannonAt(p.X, p.Y) == nil || len(targets) == 0 ||
		chebyshev(p.X, p.Y, targets[0].X, targets[0].Y) < 4) {
		return
	}
	if len(targets) > 0 && w.criticalKillParticipantsReserved(p) {
		return
	}
	mana := skillManaCost(skill, mastery, int(effectiveScore(p.Char).SaveMana))
	if playerCurMP(p.Char) < uint32(mana) {
		return
	}
	if skillIndex == coinArmorSkill { // Coin Armor (7.48 skill help): costs Trade x 100 gold.
		cost := uint32(100 * mastery)
		if p.goldenShieldPaid {
			// The committed charge belongs to exactly this resumed cast.
			p.goldenShieldPaid = false
		} else if p.Char.Gold < cost {
			p.Session.Send(wire.MessagePanel(fmt.Sprintf("Coin Armor requires %d gold.", cost)))
			return
		} else if w.criticalWrites != nil {
			// Commit the native gold charge first, then resume this same cast
			// on acknowledgement without charging again.
			images := criticalPreimages(p)
			p.Char.Gold -= cost
			if err := w.queueCriticalLiveMutation("Coin Armor cost", images, func() {
				if !p.Session.IsClosed() {
					p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
				}
				p.goldenShieldPaid = true
				w.onSkillAttack(p, req)
				p.goldenShieldPaid = false
			}, criticalLiveMutationOptions{}); err != nil {
				log.Printf("[#%d] Coin Armor cost admission rejected: %v", p.Session.ID, err)
			}
			return
		} else {
			oldGold := p.Char.Gold
			p.Char.Gold -= cost
			if err := w.saveAccount(p.Account); err != nil {
				p.Char.Gold = oldGold
				log.Printf("[#%d] save Coin Armor cost: %v", p.Session.ID, err)
				return
			}
			p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		}
	}
	if !spendPlayerMP(p.Char, uint32(mana)) {
		return
	}
	if skill.Aggressive != 0 {
		w.breakHideOnAttack(p)
	}
	// SkillData.Delay is in seconds in both client and TMSrv. Using 250 ms
	// instead allowed four casts in the intended interval for one.
	delay := skillCooldownDuration(skill)
	if p.SkillReady == nil {
		p.SkillReady = make(map[int]time.Time)
	}
	p.SkillReady[skillIndex] = now.Add(delay)
	if motion == 0 {
		motion = 5
	}
	if skillIndex == 98 {
		if w.castThornWall(p, req, skill, mastery, motion) {
			w.syncPlayerScoreAndVitals(p)
			w.gameplayLogf("skill", "[#%d] created Thorn Wall skill=98 @(%d,%d)", p.Session.ID, req.TargetX, req.TargetY)
		}
		return
	}

	if skill.Aggressive == 0 {
		affected := w.applySupportSkillToTargets(p, supportTargets, skill, mastery)
		if skillIndex >= 56 && skillIndex <= 63 && len(affected) == 0 {
			// GenerateSummon returns zero for a family conflict, reached limit or
			// unavailable party/mob slot. TMSrv restores the charged MP on failure.
			restorePlayerMP(p.Char, uint32(mana))
		}
		primary := p
		wireTargets := []wire.SkillTarget{{ID: p.ID}}
		if len(affected) > 0 {
			primary = affected[0].player
			wireTargets = make([]wire.SkillTarget, 0, len(affected))
			for _, result := range affected {
				// Healing is negative short Damage; the builder applies the same
				// scaling as damage before converting.
				wireTargets = append(wireTargets, wire.SkillTarget{
					ID: result.player.ID, Heal: result.hpDelta,
					MaxHP: playerMaxHP(result.player.Char)})
			}
		}
		w.sendToPlayerView(p, func() []byte {
			return spectralPacket(p.Char, wire.SkillHits(p.ID, p.X, p.Y, primary.X, primary.Y,
				p.Char.Exp, playerCombatMP(p.Char), int16(skillIndex), motion, skillVisualLevel(mastery),
				skill.MaxTarget, wireTargets))
		})
		for _, result := range affected {
			target := result.player
			// The owner needs both 0x181 and 0x336 to populate the HUD's wide
			// sidecar. With only 0x336 the bars used a scaled WORD against wide
			// MaxHP (29,322 / 1,202,183, about 2.4%). The extra update may flicker
			// but avoids displaying an almost empty bar for a healthy character.
			w.publishPlayerAffects(target)
			w.syncPlayerVitals(target)
			w.updatePartyMember(target)
			if result.revived {
				// Resurrection intentionally removes type 3 then materializes a
				// new entity: ordinary updates do not revive the client's dead
				// TMHuman. Normal updates must never use this path.
				w.rematerializePlayerAfterRevive(target)
			} else {
				w.sendToPlayerView(target, func() []byte { return playerAppearancePacket(target) })
			}
		}
		w.syncPlayerScoreAndVitals(p)
		w.gameplayLogf("skill", "[#%d] used skill=%d %q mp=-%d cooldown=%s", p.Session.ID,
			skillIndex, skill.Name, mana, delay)
		return
	}
	if len(playerTargets) > 0 {
		p.CombatTargetID = playerTargets[0].ID
		w.executePlayerSkill(p, playerTargets, skill, mastery, motion)
		return
	}
	p.CombatTargetID = targets[0].ID
	type skillResult struct {
		mob        *Mob
		calculated uint32
		applied    uint32
	}
	results := make([]skillResult, 0, len(targets))
	wireTargets := make([]wire.SkillTarget, 0, len(targets))
	baseDamage := w.baseSkillDamage(p.Char, skill)
	if skillIndex == 30 { // Divine Judgment adds HP damage and sacrifices five sixths of HP.
		baseDamage += int(playerCurHP(p.Char))
		setPlayerCurHP(p.Char, playerCurHP(p.Char)/6+1)
	}
	if skillIndex == 22 { // Explosion Bash converts current MP and INT into damage.
		baseDamage = explosionBashBaseDamage(baseDamage, playerInt(p.Char), int(playerCurMP(p.Char)))
		setPlayerCurMP(p.Char, 0)
		if len(targets) > 0 {
			p.X, p.Y = w.findFreeGameplayPosition(p, p, targets[0].X, targets[0].Y, 2)
			p.Char.X, p.Char.Y = p.X, p.Y
			w.refreshPlayerVisibility(p)
			w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
		}
	}
	magicDamage := skillUsesMagic(p.Char, skill)
	directDamage := skill.InstanceType >= 1 && skill.InstanceType <= 5
	hitCount := skillHitCount(skill)
	for _, target := range targets {
		if !combatRollHits(playerVersusMobAccuracy(p.Char, target.Def), w.intn) {
			wireTargets = append(wireTargets, wire.SkillTarget{ID: target.ID, Miss: true,
				MaxHP: target.Def.Score.MaxHP})
			results = append(results, skillResult{mob: target})
			continue
		}
		target.TargetID = p.ID
		appliedTotal := uint32(0)
		calculatedTotal := uint32(0)
		if directDamage {
			damageValue := w.skillFinalDamage(baseDamage, effectiveMobElementalDefenseAt(target, now), skillDamageMastery(p.Char))
			damageValue = damageValue * 70 / 100 // Historical 7.59 _MSG_Attack Mortal PvE reduction.
			damageValue = applySkillResistance(damageValue, skill.InstanceType, effectiveMobResistancesAt(target, now), true)
			damageValue = applyCouragePvEDamageAt(p.Char, damageValue, magicDamage, now)
			damageValue = int(addFlatDamage(uint32(clampInt(damageValue, 1, int(maxScoreValue))),
				w.equipmentGemBonuses(p.Char).forceDamage))
			perHitCalculated := uint32(clampInt(damageValue, 1, int(maxScoreValue)))
			calculatedWide := uint64(perHitCalculated) * uint64(hitCount)
			if calculatedWide > uint64(maxScoreValue) {
				calculatedWide = uint64(maxScoreValue)
			}
			calculatedTotal = uint32(calculatedWide)
			hpBeforeSkill := target.HP
			// The boss shield absorbs each skill hit.
			perHitCalculated = w.bossMitigateDamage(target, perHitCalculated)
			for hit := 0; hit < hitCount && target.HP > 0; hit++ {
				applied := minU32(perHitCalculated, target.HP)
				target.HP -= applied
				appliedTotal += applied
				// HP loses at most its remaining value, while floating text shows
				// calculated damage including overkill. This packet carries both
				// the projected bar WORD and the rebuilt client's uint32 DMGX tail.
				wireTargets = append(wireTargets, wire.SkillTarget{
					ID: target.ID, Damage: perHitCalculated, MaxHP: target.Def.Score.MaxHP,
				})
			}
			// Notify once with total skill damage, not once per hit, so each HP
			// threshold is crossed only once.
			if damage := hpBeforeSkill - target.HP; damage > 0 {
				w.notifyMobDamaged(target, hpBeforeSkill, p.ID, damage)
			}
		} else {
			wireTargets = append(wireTargets, wire.SkillTarget{ID: target.ID})
		}
		if skillIndex == 6 { // Divine Fury pulls the target toward the caster.
			oldX, oldY := target.X, target.Y
			target.X, target.Y = w.findFreePosition(p.X, p.Y, 2)
			w.publishMobMove(target, oldX, oldY, uint32(target.Def.Score.AttackRun&0x0f))
		}
		if skillIndex == 26 { // Flash ends the target's combat and aggro.
			target.TargetID = 0
		}
		results = append(results, skillResult{mob: target, calculated: calculatedTotal, applied: appliedTotal})
		w.applySkillMobEffects(p, target, skill, mastery)
	}
	primary := targets[0]
	w.sendToMobView(primary, func() []byte {
		if directDamage {
			return spectralPacket(p.Char, wire.SkillHits(p.ID, p.X, p.Y, primary.X, primary.Y,
				p.Char.Exp, playerCombatMP(p.Char), int16(skillIndex), motion, skillVisualLevel(mastery),
				offensiveSkillWireMaxTargets(p.Char, skill), wireTargets))
		}
		return spectralPacket(p.Char, wire.SkillHits(p.ID, p.X, p.Y, primary.X, primary.Y,
			p.Char.Exp, playerCombatMP(p.Char), int16(skillIndex), motion, skillVisualLevel(mastery),
			offensiveSkillWireMaxTargets(p.Char, skill), wireTargets))
	})
	w.syncPlayerScoreAndVitals(p)
	w.gameplayLogf("skill", "[#%d] executed skill=%d %q targets=%d base=%d magic=%t amp=%d mastery=%d mp=-%d",
		p.Session.ID, skillIndex, skill.Name, len(results), baseDamage, magicDamage,
		effectiveScore(p.Char).MagicAmp, mastery, mana)
	// Capture recipients before the first death. A member can reach the level
	// cap during this batch; recomputing eligibility afterward would omit that
	// member and lose persistence of their final EXP award.
	batchAccounts := uniqueKillAccounts(p, w.partyExpShares(p, 1, w.gameplay.PartyEXPBonusPercent))
	batchPlans := make([]*killRewardPlan, 0, len(results))
	for _, result := range results {
		if result.mob.HP == 0 {
			if plan := w.planMobKill(p, result.mob, result.calculated, result.applied); plan != nil {
				batchPlans = append(batchPlans, plan)
			}
		} else {
			mob := result.mob
			w.sendToMobViewProtocol(mob, func(observer *Player) []byte {
				// Keep mixed client views valid after every non-fatal skill hit.
				return wire.MobHpMp(mob.ID, mob.HP, mob.Def.Score.MaxHP,
					mob.Def.Score.MaxMP, mob.Def.Score.MaxMP)
			})
			w.gameplayLogf("skill", "[#%d] skill=%d %q mob=%d dmg=%d base=%d magic=%t amp=%d mastery=%d hp=%d/%d mp=-%d", p.Session.ID,
				skillIndex, skill.Name, mob.ID, result.applied, baseDamage, magicDamage,
				effectiveScore(p.Char).MagicAmp, mastery, mob.HP, mob.Def.Score.MaxHP, mana)
		}
	}
	w.commitKillRewardBatch(p, batchPlans, batchAccounts, "multi-target kills")
}

func skillHitCount(skill model.SkillDef) int {
	if skill.Index == 95 {
		return clampInt(skill.MaxTarget, 1, 6)
	}
	return 1
}

func uniqueShareAccounts(shares []partyExpShare) []*model.Account {
	accounts := make([]*model.Account, 0, len(shares))
	seen := make(map[*model.Account]struct{}, len(shares))
	for _, share := range shares {
		account := share.player.Account
		if account == nil {
			continue
		}
		if _, exists := seen[account]; exists {
			continue
		}
		seen[account] = struct{}{}
		accounts = append(accounts, account)
	}
	return accounts
}

// uniqueKillAccounts always includes the loot owner, even at the level cap.
// Using only EXP shares omitted the killer's gold/items from the transaction
// when other members were still eligible for experience.
func uniqueKillAccounts(killer *Player, shares []partyExpShare) []*model.Account {
	accounts := make([]*model.Account, 0, len(shares)+1)
	seen := make(map[*model.Account]struct{}, len(shares)+1)
	if killer != nil && killer.Account != nil {
		seen[killer.Account] = struct{}{}
		accounts = append(accounts, killer.Account)
	}
	for _, account := range uniqueShareAccounts(shares) {
		if _, exists := seen[account]; exists {
			continue
		}
		seen[account] = struct{}{}
		accounts = append(accounts, account)
	}
	return accounts
}

// WYD 7.48 Huntress skills (skill help 5074/5085/5086).
const (
	enchantFrostSkill = 74
	backpackSkill     = 85 // Opens Cargo from anywhere; the client opens it locally.
	coinArmorSkill    = 86
	clawWeaponType    = 41
)

// skillWeaponAllowed mirrors the client gate (owner rule): Enchant Frost
// cannot be cast with a claw in weapon slot 6. Hide in Shadow (75) and
// Another Soul (79) carry no weapon restriction.
func (w *World) skillWeaponAllowed(ch *model.Char, skillIndex int) bool {
	if skillIndex != enchantFrostSkill {
		return true
	}
	weapon := ch.Equip[6]
	def, ok := w.items[weapon.Index]
	return weapon.Index == 0 || !ok || itemAbility(weapon, def, "EF_WTYPE") != clawWeaponType
}
