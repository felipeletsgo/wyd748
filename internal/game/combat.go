package game

import (
	"time"

	"wydgo/internal/model"
)

const (
	affectCourage        byte = 30
	couragePhysicalBonus int  = 1_000
	courageMagicBonus    int  = 2_000
)

// applyCouragePvEDamage aplica o bonus fixo do affect Courage somente sobre um
// hit valido contra monstro. Ele nao altera ExtendedScore nem participa de PvP:
// o bonus existe apenas no resultado autoritativo de cada golpe PvE.
func applyCouragePvEDamage(ch *model.Char, damage int, magical bool) int {
	return applyCouragePvEDamageAt(ch, damage, magical, time.Now())
}

func applyCouragePvEDamageAt(ch *model.Char, damage int, magical bool, now time.Time) int {
	if damage <= 0 || activePlayerAffectAt(ch, affectCourage, now) == nil {
		return damage
	}
	bonus := couragePhysicalBonus
	if magical {
		bonus = courageMagicBonus
	}
	if damage > int(maxExtendedStat)-bonus {
		return int(maxExtendedStat)
	}
	return damage + bonus
}

// combat.go -- calculo de dano PORTADO do W2PP (SOURCE SERVER/Code/Basedef.cpp), a
// fonte com a formula correta. Regra: dano FISICO cresce com FORCA e DESTREZA; dano
// MAGICO cresce com INTELIGENCIA. So portamos algoritmos (nunca offsets/structs).

// hitDamage = dano final de UM golpe. Portado EXATO de BASE_GetDamage
// (Basedef.cpp:1553): tdam = dam - ac/2; variancia pelo combat skill; suavizacao do
// low-end; minimo 1. dam=ataque do atacante, ac=defesa do alvo, combat=nivel de skill.
func hitDamageWithRNG(dam, ac, combat int, intn func(int) int) int {
	tdam := dam - ac/2
	c := combat / 2
	if c > 7 {
		c = 7
	}
	delta := 12 - c
	rnd := intn(delta) + c + 99 // 99..110 (variancia)
	tdam = rnd * tdam / 100
	switch {
	case tdam < -50:
		tdam = 0
	case tdam < 0:
		tdam = (tdam + 50) / 7
	case tdam <= 50:
		tdam = 5*tdam/4 + 7
	}
	if tdam <= 0 {
		tdam = 1
	}
	return tdam
}

func hitDamage(dam, ac, combat int) int {
	return hitDamageWithRNG(dam, ac, combat, realRNG{}.Intn)
}

var hitRateProgression = func() [1024]int {
	var table [1024]int
	for i := range table {
		start, quadrant := i/4, i%4
		switch quadrant {
		case 0:
			table[i] = start
		case 1:
			table[i] = 512 - start
		case 2:
			table[i] = start + 512
		case 3:
			table[i] = 1024 - start
		}
		if table[i] > 999 {
			table[i] = 999
		}
	}
	table[0] = 512
	return table
}()

// criticalHit usa a progressao de 1024 entradas do BASE_GetDoubleCritical. O
// client envia Progress, mas o servidor conserva seu proprio cursor e nunca
// aceita que o cliente escolha quando o critico acontece.
func criticalHit(critical int, progress *uint16) bool {
	if progress == nil {
		return false
	}
	index := int(*progress & 0x3FF)
	*progress = (*progress + 1) & 0x3FF
	return hitRateProgression[index] < clampInt(4*critical, 0, 1024)
}

// parryRate porta WGetParryRate: equipamento vale no maximo 100, DEX perde peso
// depois de 1000 e 3000, e a precisao do atacante reduz a chance final.
func parryRate(defenderDex, equipmentParry, attackerAccuracy int) int {
	equipmentParry = clampInt(equipmentParry, 0, 100)
	first := clampInt(defenderDex, 0, 1000)
	second := clampInt(defenderDex-1000, 0, 2000)
	third := maxInt(0, defenderDex-3000)
	return clampInt(first/2+equipmentParry+second/4+third/8-attackerAccuracy, 1, 650)
}

func parryLandsWithRNG(rate int, intn func(int) int) bool {
	return intn(1000)+1 < clampInt(rate, 1, 650)
}

func playerHitsMobWithRNG(atk *Player, m *Mob, intn func(int) int) uint32 {
	return playerHitsMobAt(atk, m, intn, time.Now())
}

func playerHitsMobAt(atk *Player, m *Mob, intn func(int) int, now time.Time) uint32 {
	if atk == nil || atk.Char == nil {
		return 0
	}
	ch := atk.Char
	accuracy := maxInt(playerDex(ch)/5, int(effectiveExtended(ch).Accuracy)/5)
	if parryLandsWithRNG(parryRate(int(m.Def.Extended.Dex), 0, accuracy), intn) {
		return 0
	}
	attack := playerAttack(ch)
	dam := hitDamageWithRNG(attack, effectiveMobDefenseAt(m, now), int(playerMastery(ch, 0)), intn)
	if criticalHit(int(effectiveExtended(ch).Critical), &atk.AttackProgress) {
		dam = (((intn(2) + 13) * dam) / 10)
	}
	return uint32(clampInt(dam, 0, int(maxExtendedStat)))
}

func playerHitsMob(atk *Player, m *Mob) uint32 {
	return playerHitsMobWithRNG(atk, m, realRNG{}.Intn)
}

func (w *World) playerHitsMob(atk *Player, m *Mob) uint32 {
	return playerHitsMobAt(atk, m, w.intn, w.now())
}

// playerHitsPlayer usa a mesma progressao server-side de acerto/critico do PvE,
// trocando apenas defesa/evasao pelo score autoritativo do personagem alvo.
func playerHitsPlayerWithRNG(atk, target *Player, intn func(int) int) uint32 {
	if atk == nil || atk.Char == nil || target == nil || target.Char == nil {
		return 0
	}
	accuracy := maxInt(playerDex(atk.Char)/5, int(effectiveExtended(atk.Char).Accuracy)/5)
	if parryLandsWithRNG(parryRate(playerDex(target.Char), int(effectiveExtended(target.Char).Parry), accuracy), intn) {
		return 0
	}
	dam := hitDamageWithRNG(playerAttack(atk.Char), playerDefense(target.Char), int(playerMastery(atk.Char, 0)), intn)
	if criticalHit(int(effectiveExtended(atk.Char).Critical), &atk.AttackProgress) {
		dam = ((intn(2) + 13) * dam) / 10
	}
	return uint32(clampInt(dam, 0, int(maxExtendedStat)))
}

func playerHitsPlayer(atk, target *Player) uint32 {
	return playerHitsPlayerWithRNG(atk, target, realRNG{}.Intn)
}

func (w *World) playerHitsPlayer(atk, target *Player) uint32 {
	return playerHitsPlayerWithRNG(atk, target, w.intn)
}

// mobHitsPlayer = dano de um mob atacando um player (pro combate mob->player futuro).
func mobHitsPlayerWithRNG(m *Mob, def *model.Char, intn func(int) int) uint32 {
	return mobHitsPlayerAt(m, def, intn, time.Now())
}

func mobHitsPlayerAt(m *Mob, def *model.Char, intn func(int) int, now time.Time) uint32 {
	if m == nil || m.Def == nil || def == nil {
		return 0
	}
	if parryLandsWithRNG(parryRate(playerDex(def), int(effectiveExtended(def).Parry), int(m.Def.Extended.Dex)/5), intn) {
		return 0
	}
	damBase := effectiveMobAttackAt(m, now) + int(m.Def.Extended.Str)/2 +
		int(m.Def.Extended.Dex)/4 + int(m.Def.Extended.Level)
	defense := playerDefense(def)
	dam := hitDamageWithRNG(damBase, defense, 0, intn)
	return uint32(clampInt(dam, 0, int(maxExtendedStat)))
}

func mobHitsPlayer(m *Mob, def *model.Char) uint32 {
	return mobHitsPlayerWithRNG(m, def, realRNG{}.Intn)
}

func (w *World) mobHitsPlayer(m *Mob, def *model.Char) uint32 {
	return mobHitsPlayerAt(m, def, w.intn, w.now())
}
