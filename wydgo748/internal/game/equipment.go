package game

import "wydgo/internal/model"

var nonRefinableEffects = map[string]bool{
	"EF_GRID": true, "EF_CLASS": true, "EF_POS": true, "EF_WTYPE": true,
	"EF_RANGE": true, "EF_LEVEL": true, "EF_REQ_STR": true, "EF_REQ_INT": true,
	"EF_REQ_DEX": true, "EF_REQ_CON": true, "EF_VOLATILE": true,
	"EF_INCUBATE": true, "EF_INCUDELAY": true, "EF_PREVBONUS": true,
	"EF_MOBTYPE": true, "EF_ITEMTYPE": true, "EF_ITEMLEVEL": true,
	"EF_NOTRADE": true, "EF_NOSANC": true, "EF_DONATE": true, "EF_HONRA": true,
}

func refineAbility(value, sanc int) int {
	if value == 0 || sanc <= 0 {
		return value
	}
	switch sanc {
	case 11:
		return value * 220 / 100
	case 12:
		return value * 250 / 100
	case 13:
		return value * 280 / 100
	case 14:
		return value * 320 / 100
	case 15:
		return value * 370 / 100
	default:
		return value * (sanc + 10) / 10
	}
}

func staticAbility(def model.ItemDef, effect string) int {
	value := 0
	for _, e := range def.StaticEffects {
		if e.Name == effect {
			value += e.Value
		}
	}
	return value
}

// celestialRate ports g_pCelestialRate (Basedef.cpp:135): refinement success
// chance by CURRENT SANC LEVEL. Refining from +0 always succeeds (100); the
// chance drops at each level. Ori (cap +6) uses indices 0..5; Lac (cap +9) 0..8.
var celestialRate = [15]int{100, 95, 90, 85, 80, 75, 70, 65, 60, 50, 40, 15, 15, 15, 15}

// setItemSanc ports BASE_SetItemSanc (Basedef.cpp:2608) to our instance: it
// writes the refinement level into the existing EF_SANC (43) pair, or creates
// it in a free pair. success=0 for normal refinement, so sc = level for 0..9;
// levels 10..15 use the 230+ encoding. Returns false with no EF_SANC or free pair.
func setItemSanc(item *model.Item, level int) bool {
	if item == nil || level < 0 || level > 15 {
		return false
	}
	sc := level
	if level >= 10 {
		// +10..+15 are 230 + 4*(level-10) + gem variant. Keep the variant the
		// Aylin composition chose (Diamond, Emerald, Coral, Garnet) when the
		// item already carries one, so a later refinement keeps its effect.
		variant := 0
		if raw, exists := sancRaw(*item); exists && raw >= 230 && raw <= 253 {
			variant = (int(raw) - 230) % 4
		}
		sc = 230 + (level-10)*4 + variant
	}
	for i := 0; i < 3; i++ {
		if e := item.Eff[i*2]; e == 43 || (e >= 116 && e <= 125) {
			item.Eff[i*2+1] = byte(sc)
			return true
		}
	}
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] == 0 {
			item.Eff[i*2] = 43
			item.Eff[i*2+1] = byte(sc)
			return true
		}
	}
	return false
}

// refinementLocked reports items that never accept refinement: costumes
// (costume slot or the native slot-13 costume rows) and fairies (familiar
// slot 13). Fairy Water keeps its own fixed-target rule for Naiad/Grewpain.
func refinementLocked(def model.ItemDef) bool {
	return model.IsCostumeDef(def) || def.Pos&(1<<model.FamiliarSlot) != 0
}

func itemSanc(item model.Item) int {
	for i := 0; i < 3; i++ {
		effect, value := item.Eff[i*2], int(item.Eff[i*2+1])
		if effect == 43 || effect >= 116 && effect <= 125 {
			switch {
			case value >= 230 && value <= 233:
				return 10
			case value >= 234 && value <= 237:
				return 11
			case value >= 238 && value <= 241:
				return 12
			case value >= 242 && value <= 245:
				return 13
			case value >= 246 && value <= 249:
				return 14
			case value >= 250 && value <= 253:
				return 15
			default:
				return value % 10
			}
		}
	}
	return 0
}

// tintItem applies a color (dye) to the item's sanc pair: it changes the CODE
// of the EF_SANC (43) pair or of a previous color (116-125) to `color`, keeping
// the VALUE (the refinement level is unchanged; itemSanc already reads 116-125).
// It is the inverse of WYD 7.48 pRemoveTintura, which turns 116-125 back into
// 43. Returns false when the item has no sanc pair (it is not refined): only an
// item with a refinement glow can be dyed.
func tintItem(item *model.Item, color int) bool {
	if item == nil || color < 116 || color > 125 {
		return false
	}
	for i := 0; i < 3; i++ {
		if e := item.Eff[i*2]; e == 43 || (e >= 116 && e <= 125) {
			item.Eff[i*2] = byte(color)
			return true
		}
	}
	return false
}

// tintSetOnlyMessage rejects dye dropped on anything but an armor set piece.
const tintSetOnlyMessage = "Dye can only be used on armor set pieces."

// isArmorSetPiece reports whether def is one of the five armor set pieces
// (helmet, chest, pants, gloves, boots): its Pos is exactly one of the slot
// masks 2, 4, 8, 16 or 32. Dye (tint) only paints these pieces; weapons,
// shields, capes, mounts and accessories are rejected.
func isArmorSetPiece(def model.ItemDef) bool {
	return def.Pos >= 2 && def.Pos <= 32 && def.Pos&(def.Pos-1) == 0
}

// untintItem ports WYD 7.48 pRemoveTintura: it turns the color code (116-125)
// of the sanc pair back into EF_SANC (43), removing the dye without touching
// the refinement level. Returns false when the item is not dyed.
func untintItem(item *model.Item) bool {
	if item == nil {
		return false
	}
	for i := 0; i < 3; i++ {
		if e := item.Eff[i*2]; e >= 116 && e <= 125 {
			item.Eff[i*2] = 43
			return true
		}
	}
	return false
}

// itemAbility ports BASE_GetItemAbility: it sums the item-list static effects
// and the three persisted STRUCT_ITEM effects, applying refinement.
func itemAbility(item model.Item, def model.ItemDef, effect string) int {
	value := 0
	for _, e := range def.StaticEffects {
		if e.Name == effect {
			v := e.Value
			if effect == "EF_ATTSPEED" && v == 1 {
				v = 10
			}
			value += v
		}
	}
	for i := 0; i < 3; i++ {
		if def.DynamicEffectNames[item.Eff[i*2]] == effect {
			v := int(item.Eff[i*2+1])
			if effect == "EF_ATTSPEED" && v == 1 {
				v = 10
			}
			value += v
		}
	}
	sanc := itemSanc(item)
	if !nonRefinableEffects[effect] {
		value = refineAbility(value, sanc)
	}
	if effect == "EF_RUNSPEED" {
		if value >= 3 {
			value = 2
		}
		if value > 0 && sanc == 9 {
			value++
		}
	}
	return value
}

func (w *World) equipmentDamage(ch *model.Char) int {
	value := 0
	for slot, item := range ch.Equip {
		if !clientEquipSlotSupported(byte(slot)) || slot == 6 || slot == 7 ||
			slot == mountSlot || item.Index == 0 {
			continue
		}
		if def, ok := w.items[item.Index]; ok {
			value += itemAbility(item, def, "EF_DAMAGE") + itemAbility(item, def, "EF_DAMAGE2")
		}
	}
	left, right := ch.Equip[6], ch.Equip[7]
	ldef, lok := w.items[left.Index]
	rdef, rok := w.items[right.Index]
	ldam, rdam := 0, 0
	if lok {
		ldam = itemAbility(left, ldef, "EF_DAMAGE") + itemAbility(left, ldef, "EF_DAMAGE2")
	}
	if rok {
		rdam = itemAbility(right, rdef, "EF_DAMAGE") + itemAbility(right, rdef, "EF_DAMAGE2")
	}
	if lok && rok && ldef.Unique != 0 && rdef.Unique != 0 {
		multi := 30
		if ldef.Unique == rdef.Unique {
			multi = 50
		}
		// BASE_GetMobAbility: Hunter's Expertise (HT bit 10) and Weapon Master
		// (TK bit 9) use 100% of the secondary weapon damage.
		if ch.Class == 0 && ch.LearnedSkill&(1<<9) != 0 {
			multi = 100
		} else if ch.Class == 3 && ch.LearnedSkill&(1<<10) != 0 {
			multi = 100
		}
		return value + maxInt(ldam, rdam) + minInt(ldam, rdam)*multi/100
	}
	return value + maxInt(ldam, rdam)
}

func (w *World) equipmentDefense(ch *model.Char) int {
	ac, add := 0, 0
	unique := -1
	matchingSet := true
	for slot, item := range ch.Equip {
		if !clientEquipSlotSupported(byte(slot)) {
			continue
		}
		def, ok := w.items[item.Index]
		if !ok || item.Index == 0 || slot == mountSlot {
			if slot >= 1 && slot <= 5 {
				matchingSet = false
			}
			continue
		}
		ac += itemAbility(item, def, "EF_AC")
		add += itemAbility(item, def, "EF_ACADD") + itemAbility(item, def, "EF_ACADD2")
		if slot >= 1 && slot <= 5 {
			if def.Unique == 0 {
				matchingSet = false
			} else if unique < 0 {
				unique = def.Unique
			} else if unique != def.Unique {
				matchingSet = false
			}
		}
	}
	if matchingSet && unique > 0 {
		ac = ac * 105 / 100
	}
	return ac + add
}

func extendedValue(v int64) uint32 {
	if v <= 0 {
		return 0
	}
	if v > int64(maxScoreValue) {
		return maxScoreValue
	}
	return uint32(v)
}

// recalcExtendedPlayer is the only authoritative character calculation. It
// always starts from the persisted base, reapplies equipment/refinement and
// only then applies passives and affects to the runtime copy.
func (w *World) recalcExtendedPlayer(ch *model.Char) {
	ensureScore(ch)
	base := ch.Score
	oldHP, oldMP := playerCurHP(ch), playerCurMP(ch)
	wasFullHP := oldHP != 0 && oldHP == playerMaxHP(ch)
	wasFullMP := oldMP != 0 && oldMP == playerMaxMP(ch)

	syncStatusPoints(ch)
	syncMasteryPoints(ch)
	syncSkillPoints(ch)
	skillSpent := 0
	for bit := 0; bit < 24; bit++ {
		if ch.LearnedSkill&(1<<bit) == 0 {
			continue
		}
		if skill, ok := w.skills[int(ch.Class)*24+bit]; ok {
			skillSpent += skill.SkillPoint
		}
	}
	skillBudget := skillPointBudget(ch)
	if skillSpent >= skillBudget {
		base.SkillPts = 0
	} else {
		base.SkillPts = uint32(skillBudget - skillSpent)
	}

	total := func(effect string) int64 {
		var value int64
		for slot, item := range ch.Equip {
			if !clientEquipSlotSupported(byte(slot)) || slot == mountSlot {
				continue // every canonical mount contributes through the mount bonus boundary
			}
			if def, ok := w.items[item.Index]; ok && item.Index != 0 {
				value += int64(itemAbility(item, def, effect))
			}
		}
		return value
	}

	str := int64(base.Str) + total("EF_STR")
	intel := int64(base.Int) + total("EF_INT")
	dex := int64(base.Dex) + total("EF_DEX")
	con := int64(base.Con) + total("EF_CON")
	special := [4]int64{
		int64(base.Mastery[0]) + total("EF_SPECIAL1"),
		int64(base.Mastery[1]) + total("EF_SPECIAL2"),
		int64(base.Mastery[2]) + total("EF_SPECIAL3"),
		int64(base.Mastery[3]) + total("EF_SPECIAL4"),
	}
	allSpecial := total("EF_SPECIALALL")
	for i := range special {
		special[i] += allSpecial
		if special[i] > 320 {
			special[i] = 320
		}
	}

	level := int64(base.Level)
	combatLevel := level
	if isCelestialEvolution(ch) {
		// In the native score Celestial/Sub counts as current level + MAX_LEVEL.
		combatLevel += int64(maxMortalLevel)
	}
	physical := int64(base.Attack) + int64(w.equipmentDamage(ch)) +
		str/3 + dex/4 + special[0] + combatLevel
	defensePerLevel := int64(1)
	if isArch(ch) || isCelestialEvolution(ch) {
		defensePerLevel = 2
	}
	defense := int64(base.Defense) + int64(w.equipmentDefense(ch)) +
		dex/5 + special[3]/2 + level*defensePerLevel

	class := int(ch.Class)
	maxHP, maxMP := int64(base.MaxHP), int64(base.MaxMP)
	if class >= 0 && class < len(baseClassStats) {
		naturalInt, naturalCon := int64(baseClassStats[class][1]), int64(baseClassStats[class][3])
		hpPerLevel, mpPerLevel := mortalHPPerLevel[class], mortalMPPerLevel[class]
		hpMPLevel := level
		if isArch(ch) {
			hpPerLevel, mpPerLevel = archHPPerLevel[class], archMPPerLevel[class]
		} else if isCelestialEvolution(ch) {
			hpPerLevel, mpPerLevel = archHPPerLevel[class], archMPPerLevel[class]
		}
		maxHP += 2*(con-naturalCon) + hpMPLevel*int64(hpPerLevel)
		maxMP += 2*(intel-naturalInt) + hpMPLevel*int64(mpPerLevel)
		if isCelestialEvolution(ch) {
			bonusHP, bonusMP := celestialHPMPBonus(class, ch.ArchCrystals)
			maxHP += bonusHP
			maxMP += bonusMP
		}
	}
	maxHP += total("EF_HP") + special[3]*2
	maxMP += total("EF_MP")
	maxHP = maxHP * (100 + total("EF_HPADD") + total("EF_HPADD2")) / 100
	maxMP = maxMP * (100 + total("EF_MPADD") + total("EF_MPADD2")) / 100

	// BASE_GetSkillDamage 7.59: Foema/BM use INT/3 + INT/30; the TK magic
	// branch uses INT/4 + INT/40 plus weapon damage. EF_MAGIC and EF_MAGICADD
	// become a /4 amplification and the result receives the 5/4 factor.
	// Adult mount magic is EF_MAGIC, not flat spell damage. Include it before
	// the shared /4 conversion so both the score and spells use the same bonus.
	mAtk, mMag, mEva, mRes, mRun := w.mountBonus(ch)
	magicAmp := int64(base.MagicAmp) + (total("EF_MAGIC")+total("EF_MAGICADD")+int64(mMag)+1)/4
	magicCore := int64(base.MagicAttack) + special[1] + level/2
	switch ch.Class {
	case 1, 2:
		magicCore += intel/3 + intel/30
	case 0:
		// This field summarizes the attack of the TK magic branches. Trans uses
		// only the PhysicalAttack computed above.
		magicMastery := special[1]
		if special[3] > magicMastery {
			magicMastery = special[3]
		}
		magicCore = int64(base.MagicAttack) + magicMastery + level/2
		magicCore += intel/4 + intel/40 + int64(w.equipmentDamage(ch))
	default:
		magicCore = physical
	}
	magicAttack := magicCore * (100 + 4*magicAmp) / 100
	magicAttack = magicAttack * 5 / 4

	runtime := *base
	runtime.Attack = extendedValue(physical)
	runtime.MagicAttack = extendedValue(magicAttack)
	runtime.Defense = extendedValue(defense)
	runtime.MaxHP = extendedValue(maxHP)
	runtime.MaxMP = extendedValue(maxMP)
	runtime.Str = extendedValue(str)
	runtime.Int = extendedValue(intel)
	runtime.Dex = extendedValue(dex)
	runtime.Con = extendedValue(con)
	for i := range runtime.Mastery {
		runtime.Mastery[i] = extendedValue(special[i])
	}
	// Accuracy/Evasion store only additive combat bonus points. Dexterity is a
	// separate operand of the matchup formula in combat_accuracy.go; mixing it
	// into these fields would count DEX twice. Ten bonus points equal 1%.
	runtime.Accuracy = extendedValue(int64(base.Accuracy) + total("EF_HITRATE"))
	runtime.Parry = extendedValue(int64(base.Parry) + total("EF_PARRY"))
	runtime.Evasion = extendedValue(int64(base.Evasion) + int64(runtime.Parry))
	// Critical is in tenths of a percent, the unit the item tooltip shows
	// (EF_CRITICAL 96 = 9.6%). Items add their value unchanged; the Character
	// panel and the roll in rollPhysicalHitFlags read the same unit, capped
	// at 1000 (100%).
	runtime.Critical = uint32(clampInt(int(base.Critical)+
		int(total("EF_CRITICAL")+total("EF_CRITICAL2")), 0, maxCriticalPerMille))
	// BASE_GetMobAbility/BASE_GetMaxAbility use the largest equipped EF_RANGE;
	// summing accessories and every piece artificially inflated the range.
	rangeValue := int(base.Range)
	for slot, item := range ch.Equip {
		if !clientEquipSlotSupported(byte(slot)) || slot == mountSlot {
			continue
		}
		if def, ok := w.items[item.Index]; ok && item.Index != 0 {
			rangeValue = maxInt(rangeValue, itemAbility(item, def, "EF_RANGE"))
		}
	}
	runtime.Range = extendedValue(int64(maxInt(1, rangeValue)))
	runtime.SaveMana = uint32(minInt(99, int(base.SaveMana)+int(total("EF_SAVEMANA"))))
	runtime.MagicAmp = extendedValue(magicAmp)
	runtime.RegenHP = uint32(clampInt(int(base.RegenHP)+int(total("EF_REGENHP")), 0, 255))
	runtime.RegenMP = uint32(clampInt(int(base.RegenMP)+int(total("EF_REGENMP")), 0, 255))
	resAll := int(total("EF_RESISTALL"))
	runtime.ResistFire = uint32(clampInt(int(base.ResistFire)+int(total("EF_RESIST1"))+resAll, 0, 100))
	runtime.ResistIce = uint32(clampInt(int(base.ResistIce)+int(total("EF_RESIST2"))+resAll, 0, 100))
	runtime.ResistHoly = uint32(clampInt(int(base.ResistHoly)+int(total("EF_RESIST3"))+resAll, 0, 100))
	runtime.ResistThunder = uint32(clampInt(int(base.ResistThunder)+int(total("EF_RESIST4"))+resAll, 0, 100))

	// Remaining adult mount bonuses: flat physical damage, evasion,
	// resistance and a speed floor. Magic already feeds the amplification above.
	if mAtk != 0 {
		runtime.Attack = extendedValue(int64(runtime.Attack) + int64(mAtk))
	}
	if mEva != 0 {
		runtime.Evasion = extendedValue(int64(runtime.Evasion) + int64(mEva))
	}
	if mRes != 0 {
		runtime.ResistFire = uint32(clampInt(int(runtime.ResistFire)+mRes, 0, 100))
		runtime.ResistIce = uint32(clampInt(int(runtime.ResistIce)+mRes, 0, 100))
		runtime.ResistHoly = uint32(clampInt(int(runtime.ResistHoly)+mRes, 0, 100))
		runtime.ResistThunder = uint32(clampInt(int(runtime.ResistThunder)+mRes, 0, 100))
	}
	// Imported premium mounts are ordinary UID-bound Equip[14] items rather
	// than native baby/adult mounts. Their catalog values are authoritative:
	// flat physical damage, exact magic attack percentage and additive speed.
	premium := w.premiumMountBonus(ch, w.now())
	if premium.attack != 0 {
		runtime.Attack = extendedValue(int64(runtime.Attack) + int64(premium.attack))
	}
	if premium.magicPercent != 0 {
		runtime.MagicAttack = extendedValue(int64(runtime.MagicAttack) *
			int64(100+premium.magicPercent) / 100)
	}
	if premium.evasion != 0 {
		runtime.Evasion = extendedValue(int64(runtime.Evasion) + int64(premium.evasion))
	}
	if premium.resist != 0 {
		runtime.ResistFire = uint32(clampInt(int(runtime.ResistFire)+premium.resist, 0, 100))
		runtime.ResistIce = uint32(clampInt(int(runtime.ResistIce)+premium.resist, 0, 100))
		runtime.ResistHoly = uint32(clampInt(int(runtime.ResistHoly)+premium.resist, 0, 100))
		runtime.ResistThunder = uint32(clampInt(int(runtime.ResistThunder)+premium.resist, 0, 100))
	}

	runtime.StatusPts = base.StatusPts
	runtime.MasterPts = base.MasterPts
	runtime.SkillPts = base.SkillPts

	// Passives and affects still change the caps below. For now keep the
	// previous resource limited to the intermediate score; restoring "was full"
	// can only happen after the last MaxHP/MP modifier.
	runtime.CurHP = minU32(oldHP, runtime.MaxHP)
	runtime.CurMP = minU32(oldMP, runtime.MaxMP)
	ch.RuntimeScore = &runtime

	runSpeed := minInt(15, int(base.AttackRun&0x0F)+int(total("EF_RUNSPEED")))
	if mRun > runSpeed {
		// The mount imposes a speed floor (riding is faster).
		runSpeed = minInt(15, mRun)
	}
	if premium.runSpeed != 0 {
		// For premium mounts EF_RUNSPEED is the final mounted speed, not an
		// additive item bonus. The authoritative 7.48 movement ceiling is 6.
		runSpeed = clampInt(premium.runSpeed, 1, 6)
	}
	// Template AttackRun high bits are not an extra attack-speed bonus. The
	// client panel derives speed from effective DEX, item percentage points and
	// active affects, above an implicit 100% base. Quantize only the final sum.
	ch.RuntimeScore.AttackRun = uint32(runSpeed)
	w.applyPassiveSkills(ch)
	affectAttackSpeed := w.applyAffectStats(ch)
	attackSpeedPoints := int(ch.RuntimeScore.Dex/5) + int(total("EF_ATTSPEED")) + affectAttackSpeed
	ch.RuntimeScore.AttackRun = packedAttackSpeed(attackSpeedPoints) | ch.RuntimeScore.AttackRun&0x0F
	if heldExperiencePenaltyActive(ch) {
		ch.RuntimeScore.MaxHP /= 2
	}

	// Bear Man and the other BM transformations change MaxHP inside affects.
	// Restoring full HP before this point used the untransformed cap (for
	// example 1833) and left it visible below the final cap (3207) on every
	// later buff. Preserve the live state against the FINAL cap.
	if wasFullHP {
		ch.RuntimeScore.CurHP = ch.RuntimeScore.MaxHP
	} else {
		ch.RuntimeScore.CurHP = minU32(oldHP, ch.RuntimeScore.MaxHP)
	}
	if wasFullMP {
		ch.RuntimeScore.CurMP = ch.RuntimeScore.MaxMP
	} else {
		ch.RuntimeScore.CurMP = minU32(oldMP, ch.RuntimeScore.MaxMP)
	}
	// The base is the score WITHOUT equipment and WITHOUT affects: its own MaxHP
	// is the only valid cap for persistence. The runtime keeps effective resources.
	base.CurHP = minU32(ch.RuntimeScore.CurHP, base.MaxHP)
	base.CurMP = minU32(ch.RuntimeScore.CurMP, base.MaxMP)
	normalizeRuntimeScore(ch)
}

func (w *World) recalcPlayer(ch *model.Char) {
	if ch == nil {
		return
	}
	w.recalcExtendedPlayer(ch)
}

func (w *World) canEquip(ch *model.Char, item model.Item, pos byte) bool {
	if !clientEquipSlotSupported(pos) {
		return false
	}
	if item.Index == 0 {
		return true
	}
	if ch == nil {
		return false
	}
	def, ok := w.items[item.Index]
	if !ok || pos >= 16 || def.Pos&(1<<pos) == 0 {
		return false
	}
	classMask := staticAbility(def, "EF_CLASS")
	advanced := isArch(ch) || isCelestialEvolution(ch)
	weapon := pos == 6 || pos == 7
	if classMask != 0 && !(advanced && weapon) {
		equipClass, validClass := equipmentBodyClass(ch)
		if !validClass || classMask&(1<<equipClass) == 0 {
			return false
		}
	}
	// BASE_CanEquip (WYD 7.48) zeroes Level/STR/INT/DEX/CON for higher
	// evolutions. For Arch the Sephiroth class still governs skills but not
	// equipment requirements; weapons also ignore the class.
	if advanced {
		return true
	}
	levelReq := def.ReqLevel + staticAbility(def, "EF_LEVEL")
	strReq := def.ReqStr + staticAbility(def, "EF_REQ_STR")
	intReq := def.ReqInt + staticAbility(def, "EF_REQ_INT")
	dexReq := def.ReqDex + staticAbility(def, "EF_REQ_DEX")
	conReq := def.ReqCon + staticAbility(def, "EF_REQ_CON")
	return int(playerLevel(ch)) >= levelReq && playerStr(ch) >= strReq &&
		playerInt(ch) >= intReq && playerDex(ch) >= dexReq && playerCon(ch) >= conReq
}

// equipmentBodyClass returns the visual class that governs armor.
//
// Arch stores in Equip[0] the face computed as `MortalFace + 5 + Sephiroth`.
// The tens digit remains the Mortal body class (TK/FM/BM/HT) and reproduces
// the `extra->MortalFace / 10` used by BASE_CanEquip in WYD 7.48. Char.Class,
// on the other hand, is the Sephiroth class and governs skills.
func equipmentBodyClass(ch *model.Char) (byte, bool) {
	if ch == nil {
		return 0, false
	}
	if !isArch(ch) && !isCelestialEvolution(ch) {
		return ch.Class, ch.Class <= 3
	}
	bodyClass := ch.Equip[0].Index / 10
	if bodyClass > 3 {
		return 0, false
	}
	return byte(bodyClass), true
}

func minInt(a, b int) int {
	if a < b {
		return a
	}
	return b
}
func maxInt(a, b int) int {
	if a > b {
		return a
	}
	return b
}
func clampInt(v, min, max int) int {
	if v < min {
		return min
	}
	if v > max {
		return max
	}
	return v
}

// promoteOffhandWeapon ports W2PP _MSG_TradingItem: when the main weapon slot
// (6) is empty and slot 7 holds a weapon rather than a shield (EF_POS 128),
// that weapon moves to slot 6. Without it a character that removed the main
// weapon of a pair kept a lone off-hand weapon drawn through the body.
func (w *World) promoteOffhandWeapon(ch *model.Char) bool {
	if ch == nil || ch.Equip[6].Index != 0 || ch.Equip[7].Index == 0 {
		return false
	}
	offhand := ch.Equip[7]
	def, ok := w.items[offhand.Index]
	if !ok || def.Pos == 128 || !w.canEquip(ch, offhand, 6) {
		return false
	}
	ch.Equip[6], ch.Equip[7] = offhand, model.Item{}
	return true
}
