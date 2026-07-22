package game

import (
	"math/rand"

	"wydgo/internal/model"
)

// combat.go -- calculo de dano PORTADO do W2PP (SOURCE SERVER/Code/Basedef.cpp), a
// fonte com a formula correta. Regra: dano FISICO cresce com FORCA e DESTREZA; dano
// MAGICO cresce com INTELIGENCIA. So portamos algoritmos (nunca offsets/structs).

// magicDamage = dano magico final (calculo simplificado ignorando resistências por ora).
// Magias normalmente nao erram (ignoram hitLands) a menos que a resistencia elemental anule o dano.
func magicDamage(baseMagic, defResist int) int {
	// Formula exata de BASE_GetMagicDamage depende do tipo (Gelo, Fogo, Raio, Sagrado).
	// Por ora: dano direto subtraido pela resistencia escalar.
	tdam := baseMagic - defResist
	if tdam < 1 {
		tdam = 1
	}
	return tdam
}

// hitDamage = dano final de UM golpe. Portado EXATO de BASE_GetDamage
// (Basedef.cpp:1553): tdam = dam - ac/2; variancia pelo combat skill; suavizacao do
// low-end; minimo 1. dam=ataque do atacante, ac=defesa do alvo, combat=nivel de skill.
func hitDamage(dam, ac, combat int) int {
	tdam := dam - ac/2
	c := combat / 2
	if c > 7 {
		c = 7
	}
	delta := 12 - c
	rnd := rand.Intn(delta) + c + 99 // 99..110 (variancia)
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

func parryLands(rate int) bool {
	return rand.Intn(1000)+1 < clampInt(rate, 1, 650)
}

func playerHitsMob(atk *Player, m *Mob) uint32 {
	if atk == nil || atk.Char == nil {
		return 0
	}
	ch := atk.Char
	accuracy := maxInt(playerDex(ch)/5, int(effectiveExtended(ch).Accuracy)/5)
	if parryLands(parryRate(int(m.Def.Extended.Dex), 0, accuracy)) {
		return 0
	}
	attack := playerAttack(ch)
	dam := hitDamage(attack, effectiveMobDefense(m), int(playerMastery(ch, 0)))
	if criticalHit(int(effectiveExtended(ch).Critical), &atk.AttackProgress) {
		dam = (((rand.Intn(2) + 13) * dam) / 10)
	}
	return uint32(clampInt(dam, 0, int(maxExtendedStat)))
}

// playerHitsPlayer usa a mesma progressao server-side de acerto/critico do PvE,
// trocando apenas defesa/evasao pelo score autoritativo do personagem alvo.
func playerHitsPlayer(atk, target *Player) uint32 {
	if atk == nil || atk.Char == nil || target == nil || target.Char == nil {
		return 0
	}
	accuracy := maxInt(playerDex(atk.Char)/5, int(effectiveExtended(atk.Char).Accuracy)/5)
	if parryLands(parryRate(playerDex(target.Char), int(effectiveExtended(target.Char).Parry), accuracy)) {
		return 0
	}
	dam := hitDamage(playerAttack(atk.Char), playerDefense(target.Char), int(playerMastery(atk.Char, 0)))
	if criticalHit(int(effectiveExtended(atk.Char).Critical), &atk.AttackProgress) {
		dam = ((rand.Intn(2) + 13) * dam) / 10
	}
	return uint32(clampInt(dam, 0, int(maxExtendedStat)))
}

// mobHitsPlayer = dano de um mob atacando um player (pro combate mob->player futuro).
func mobHitsPlayer(m *Mob, def *model.Char) uint32 {
	if m == nil || m.Def == nil || def == nil {
		return 0
	}
	if parryLands(parryRate(playerDex(def), int(effectiveExtended(def).Parry), int(m.Def.Extended.Dex)/5)) {
		return 0
	}
	damBase := effectiveMobAttack(m) + int(m.Def.Extended.Str)/2 +
		int(m.Def.Extended.Dex)/4 + int(m.Def.Extended.Level)
	defense := playerDefense(def)
	dam := hitDamage(damBase, defense, 0)
	return uint32(clampInt(dam, 0, int(maxExtendedStat)))
}
