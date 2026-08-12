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

// celestialRate porta g_pCelestialRate (Basedef.cpp:135): chance de sucesso do
// refino por NIVEL DE SANC ATUAL. Refinar de +0 e certo (100); a chance cai a
// cada nivel. Ori (teto +6) usa indices 0..5; Lac (teto +9) usa 0..8.
var celestialRate = [15]int{100, 95, 90, 85, 80, 75, 70, 65, 60, 50, 40, 15, 15, 15, 15}

// setItemSanc porta BASE_SetItemSanc (Basedef.cpp:2608) para nossa instancia:
// grava o nivel de refino no par EF_SANC (43) existente, ou o cria num slot
// livre. success=0 no refino normal, entao sc = nivel para 0..9; niveis 10..15
// usam o encoding 230+. Retorna false quando nao ha slot EF_SANC nem livre.
func setItemSanc(item *model.Item, level int) bool {
	if item == nil || level < 0 || level > 15 {
		return false
	}
	sc := level
	switch level {
	case 10:
		sc = 230
	case 11:
		sc = 234
	case 12:
		sc = 238
	case 13:
		sc = 242
	case 14:
		sc = 246
	case 15:
		sc = 250
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

// tintItem aplica uma cor (tintura) ao slot de sanc do item: troca o CODIGO do
// par EF_SANC (43) ou de uma cor anterior (116-125) para `color`, preservando o
// VALOR (o nivel de refino nao muda; itemSanc ja le 116-125). E o inverso do
// pRemoveTintura do Micronics, que converte 116-125 de volta para 43. Retorna
// false quando o item nao tem slot de sanc (nao esta refinado): so ha o que
// pintar num item com brilho de refino.
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

// untintItem porta o pRemoveTintura do Micronics: converte o codigo de cor
// (116-125) do slot de sanc de volta para EF_SANC (43), removendo a tintura sem
// tocar no nivel de refino. Retorna false quando o item nao esta pintado.
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

// itemAbility porta BASE_GetItemAbility: soma efeitos estaticos do itemlist e
// os tres efeitos do STRUCT_ITEM persistido, aplicando a refinacao.
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
		if slot == 6 || slot == 7 || item.Index == 0 || model.IsMount(item.Index) {
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
		// BASE_GetMobAbility: as passivas aumentam o aproveitamento da arma
		// secundaria; nao substituem o multiplicador inteiro por 100%.
		if ch.Class == 0 && ch.LearnedSkill&(1<<9) != 0 {
			multi += 15
		} else if ch.Class == 3 && ch.LearnedSkill&(1<<11) != 0 {
			multi += 10
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
		def, ok := w.items[item.Index]
		if !ok || item.Index == 0 || model.IsMount(item.Index) {
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
	if v > int64(maxExtendedStat) {
		return maxExtendedStat
	}
	return uint32(v)
}

// recalcExtendedPlayer e o unico calculo autoritativo de personagem. Ele
// começa sempre na base persistida, reaplica equipamento/refinação e somente
// depois aplica passivas e affects na copia runtime.
func (w *World) recalcExtendedPlayer(ch *model.Char) {
	ensureExtendedScore(ch)
	base := ch.Extended
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
		for _, item := range ch.Equip {
			if model.IsMount(item.Index) {
				continue // a montaria contribui via mountBonus, nao pelos slots de efeito
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
		// No score nativo Celestial/Sub conta como nivel atual + MAX_LEVEL.
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

	// BASE_GetSkillDamage 7.59: Foema/BM usam INT/3 + INT/30; o
	// ramo magico do TK usa INT/4 + INT/40 e dano da arma. EF_MAGIC e
	// EF_MAGICADD viram amplificacao /4 e o resultado recebe o fator 5/4.
	magicAmp := int64(base.MagicAmp) + (total("EF_MAGIC")+total("EF_MAGICADD")+1)/4
	magicCore := int64(base.MagicAttack) + special[1] + level/2
	switch ch.Class {
	case 1, 2:
		magicCore += intel/3 + intel/30
	case 0:
		// Este campo resume o ataque dos ramos magicos do TK. Trans usa
		// exclusivamente o PhysicalAttack calculado acima.
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
	runtime.Critical = extendedValue(int64(base.Critical) +
		(total("EF_CRITICAL")+total("EF_CRITICAL2"))/4)
	// BASE_GetMobAbility/BASE_GetMaxAbility usam o maior EF_RANGE equipado;
	// somar adornos e todas as pecas inflava artificialmente o alcance.
	rangeValue := int(base.Range)
	for _, item := range ch.Equip {
		if model.IsMount(item.Index) {
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

	// Bonus da montaria equipada (slot 14): dano fisico/magico, evasao,
	// resistencia e piso de velocidade -- portado da tabela do Montarias.cpp via
	// data/mounts.json. mRun entra no calculo de runSpeed abaixo.
	mAtk, mMag, mEva, mRes, mRun := w.mountBonus(ch)
	if mAtk != 0 {
		runtime.Attack = extendedValue(int64(runtime.Attack) + int64(mAtk))
	}
	if mMag != 0 {
		runtime.MagicAttack = extendedValue(int64(runtime.MagicAttack) + int64(mMag))
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

	runtime.StatusPts = base.StatusPts
	runtime.MasterPts = base.MasterPts
	runtime.SkillPts = base.SkillPts

	// Passivas e affects ainda vao alterar os tetos abaixo. Mantenha por ora o
	// recurso anterior limitado ao score intermediario; a restauracao de
	// "estava cheio" so pode acontecer depois do ultimo modificador de MaxHP/MP.
	runtime.CurHP = minU32(oldHP, runtime.MaxHP)
	runtime.CurMP = minU32(oldMP, runtime.MaxMP)
	ch.ExtendedRuntime = &runtime

	attackSpeed := minInt(15, int(base.AttackRun>>4)+int(total("EF_ATTSPEED")))
	runSpeed := minInt(15, int(base.AttackRun&0x0F)+int(total("EF_RUNSPEED")))
	if mRun > runSpeed {
		// A montaria impoe um piso de velocidade (cavalgar e mais rapido).
		runSpeed = minInt(15, mRun)
	}
	ch.ExtendedRuntime.AttackRun = byte(attackSpeed<<4 | runSpeed)
	w.applyPassiveSkills(ch)
	w.applyAffectStats(ch)

	// Homem Urso e as demais transformacoes BM mudam MaxHP dentro dos affects.
	// Restaurar o HP cheio antes desse ponto usava o teto sem transformacao
	// (por exemplo 1833) e o deixava visivel por baixo do teto final (3207) a
	// cada buff subsequente. Preserve o estado vivo contra o teto FINAL.
	if wasFullHP {
		ch.ExtendedRuntime.CurHP = ch.ExtendedRuntime.MaxHP
	} else {
		ch.ExtendedRuntime.CurHP = minU32(oldHP, ch.ExtendedRuntime.MaxHP)
	}
	if wasFullMP {
		ch.ExtendedRuntime.CurMP = ch.ExtendedRuntime.MaxMP
	} else {
		ch.ExtendedRuntime.CurMP = minU32(oldMP, ch.ExtendedRuntime.MaxMP)
	}
	// O base e o score SEM equipamento e SEM affect: seu proprio MaxHP e o unico
	// teto valido para persistencia. O runtime conserva os recursos efetivos.
	base.CurHP = minU32(ch.ExtendedRuntime.CurHP, base.MaxHP)
	base.CurMP = minU32(ch.ExtendedRuntime.CurMP, base.MaxMP)
	projectExtendedRuntime(ch)
}

func (w *World) recalcPlayer(ch *model.Char) {
	if ch == nil {
		return
	}
	w.recalcExtendedPlayer(ch)
}

func (w *World) canEquip(ch *model.Char, item model.Item, pos byte) bool {
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
	// BASE_CanEquip (W2PP) zera Level/STR/INT/DEX/CON para evolucoes
	// superiores. No Arch a classe do Sephiroth continua governando skills,
	// mas nao os requisitos de equipamento; armas tambem ignoram a classe.
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

// equipmentBodyClass devolve a classe visual que governa armaduras.
//
// O Arch guarda em Equip[0] o rosto calculado por `MortalFace + 5 + Sephiroth`.
// O algarismo das dezenas permanece sendo a classe do corpo Mortal (TK/FM/BM/HT)
// e reproduz o `extra->MortalFace / 10` usado por BASE_CanEquip no W2PP.
// Char.Class, por outro lado, e a classe do Sephiroth e governa as habilidades.
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
