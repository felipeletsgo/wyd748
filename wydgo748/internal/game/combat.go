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

func (w *World) canInitiatePvP(attacker, target *Player) bool {
	if allowed, handled := w.guildWarPvP(attacker, target); handled {
		return allowed
	}
	// Excecoes futuras (Kingdom, Castle ou Guild War) devem ser autorizadas
	// explicitamente aqui, sem enfraquecer o PK Mode do combate comum.
	return attacker != nil && target != nil && attacker.PKMode
}

// applyCouragePvEDamage aplica o bonus fixo do affect Courage somente sobre um
// hit valido contra monstro. Ele nao altera Score nem participa de PvP:
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
	if damage > int(maxScoreValue)-bonus {
		return int(maxScoreValue)
	}
	return damage + bonus
}

// combat.go -- calculo de dano PORTADO do WYD 7.48 (SOURCE SERVER/Code/Basedef.cpp), a
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
	start, quadrant := 0, 0
	for jump := 512; jump > 0; jump /= 2 {
		for i := 0; i < len(table); i += jump {
			if table[i] != 0 {
				continue
			}
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
			quadrant++
			if quadrant >= 4 {
				quadrant = 0
				start++
			}
		}
	}
	table[0] = 512
	return table
}()

func attackSpeedPercent(ch *model.Char) int {
	if ch == nil {
		return 50
	}
	// WYD 7.48 encodes attack speed as the high AttackRun nibble. Five represents
	// 100%; every following point adds 10%, reaching 200% at the native cap 15.
	return clampInt((int(effectiveScore(ch).AttackRun>>4)+5)*10, 50, 200)
}

func doubleHitChance(ch *model.Char) int {
	return clampInt(attackSpeedPercent(ch)-100, 0, 100)
}

type physicalHitResult struct {
	Damage   uint32
	Flank    uint32
	Hit      bool
	Double   bool
	Critical bool
}

func (r physicalHitResult) visualFlags() byte {
	var flags byte
	if r.Double {
		flags |= 1
	}
	if r.Critical {
		flags |= 2
	}
	if r.Flank > 0 {
		flags |= 4
	}
	return flags
}

const huntressAirBladeBit uint32 = 0x200000

// applyHuntressAirBlade ports the Huntress Lâmina Aérea passive from the
// physical MSG_AttackTwo path. The native order is important: the passive
// bonus is added before Double Hit, so a double action repeats the bonus too.
func applyHuntressAirBlade(hit physicalHitResult, ch *model.Char, targetDefense int,
	eligible bool, intn func(int) int) physicalHitResult {
	if !eligible || !hit.Hit || ch == nil || ch.Class != 3 ||
		ch.LearnedSkill&huntressAirBladeBit == 0 || intn == nil {
		return hit
	}
	if intn(4) != 0 { // native 25% proc
		return hit
	}
	score := effectiveScore(ch)
	if score == nil {
		return hit
	}
	source := uint64(score.Mastery[3]) + uint64(score.Str)
	if source > uint64(maxScoreValue) {
		source = uint64(maxScoreValue)
	}
	bonus := hitDamageWithRNG(int(source), targetDefense, 0, intn)
	if bonus > 0 {
		bonus /= 2
	}
	if bonus < 60 {
		bonus = 60
	}
	bonus = clampInt(bonus, 0, int(maxScoreValue))
	hit.Flank = uint32(bonus)
	repeats := uint64(1)
	if hit.Double {
		repeats = 2
	}
	total := uint64(hit.Damage) + uint64(hit.Flank)*repeats
	if total > uint64(maxScoreValue) {
		total = uint64(maxScoreValue)
	}
	hit.Damage = uint32(total)
	return hit
}

// rollPhysicalHitFlags ports WYD 7.48 BASE_GetDoubleCritical. Double Hit follows
// the server-owned 1,024-step progression derived from the high AttackRun
// nibble; Critical is an independent 0..254 roll. Both bits may be set by the
// same action, and the client-supplied Progress field is never authoritative.
func rollPhysicalHitFlags(ch *model.Char, progress *uint16, intn func(int) int) (double, critical bool) {
	if ch == nil || progress == nil || intn == nil {
		return false, false
	}
	index := int(*progress & 0x3FF)
	*progress = (*progress + 1) & 0x3FF
	doubleThreshold := clampInt(doubleHitChance(ch)*10, 0, 1000)
	double = hitRateProgression[index] < doubleThreshold
	critical = intn(255) < clampInt(int(effectiveScore(ch).Critical), 0, 255)
	return double, critical
}

func playerHitsMobWithRNG(atk *Player, m *Mob, intn func(int) int) uint32 {
	return playerHitsMobAt(atk, m, intn, time.Now())
}

func playerHitsMobAt(atk *Player, m *Mob, intn func(int) int, now time.Time) uint32 {
	return playerPhysicalHitMobAt(atk, m, intn, now).Damage
}

func playerPhysicalHitMobAt(atk *Player, m *Mob, intn func(int) int, now time.Time) physicalHitResult {
	if atk == nil || atk.Char == nil {
		return physicalHitResult{}
	}
	ch := atk.Char
	double, critical := rollPhysicalHitFlags(ch, &atk.AttackProgress, intn)
	if m == nil || m.Def == nil || !combatRollHits(playerVersusMobAccuracy(ch, m.Def), intn) {
		return physicalHitResult{Double: double, Critical: critical}
	}
	attack := playerAttack(ch)
	if critical {
		attack = (intn(2) + 15) * attack / 10
	}
	dam := hitDamageWithRNG(attack, effectiveMobDefenseAt(m, now), int(playerMastery(ch, 0)), intn)
	if double {
		dam *= 2
	}
	return physicalHitResult{Damage: uint32(clampInt(dam, 0, int(maxScoreValue))),
		Hit: true, Double: double, Critical: critical}
}

// playerPhysicalFollowupHitMobAt resolves an additional physical target from
// the same native attack action. MSG_AttackTwo rolls Double/Critical only for
// Dam[0]; later Dam slots reuse those flags but still run their own miss/parry
// check and defense calculation.
func playerPhysicalFollowupHitMobAt(atk *Player, m *Mob, double, critical bool,
	intn func(int) int, now time.Time) physicalHitResult {
	if atk == nil || atk.Char == nil || m == nil || m.Def == nil ||
		!combatRollHits(playerVersusMobAccuracy(atk.Char, m.Def), intn) {
		return physicalHitResult{}
	}
	attack := playerAttack(atk.Char)
	if critical {
		attack = (intn(2) + 15) * attack / 10
	}
	dam := hitDamageWithRNG(attack, effectiveMobDefenseAt(m, now), int(playerMastery(atk.Char, 0)), intn)
	if double {
		dam *= 2
	}
	return physicalHitResult{Damage: uint32(clampInt(dam, 0, int(maxScoreValue))),
		Hit: true, Double: double, Critical: critical}
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
	return playerPhysicalHitPlayerWithRNG(atk, target, intn).Damage
}

func playerPhysicalHitPlayerWithRNG(atk, target *Player, intn func(int) int) physicalHitResult {
	if atk == nil || atk.Char == nil || target == nil || target.Char == nil {
		return physicalHitResult{}
	}
	double, critical := rollPhysicalHitFlags(atk.Char, &atk.AttackProgress, intn)
	if !combatRollHits(playerVersusPlayerAccuracy(atk.Char, target.Char), intn) {
		return physicalHitResult{Double: double, Critical: critical}
	}
	attack := playerAttack(atk.Char)
	if critical {
		attack = (intn(2) + 13) * attack / 10
	}
	dam := hitDamageWithRNG(attack, playerDefense(target.Char), int(playerMastery(atk.Char, 0)), intn)
	if double {
		dam *= 2
	}
	return physicalHitResult{Damage: uint32(clampInt(dam, 0, int(maxScoreValue))),
		Hit: true, Double: double, Critical: critical}
}

func playerPhysicalFollowupHitPlayerWithRNG(atk, target *Player, double, critical bool,
	intn func(int) int) physicalHitResult {
	if atk == nil || atk.Char == nil || target == nil || target.Char == nil ||
		!combatRollHits(playerVersusPlayerAccuracy(atk.Char, target.Char), intn) {
		return physicalHitResult{}
	}
	attack := playerAttack(atk.Char)
	if critical {
		attack = (intn(2) + 13) * attack / 10
	}
	dam := hitDamageWithRNG(attack, playerDefense(target.Char), int(playerMastery(atk.Char, 0)), intn)
	if double {
		dam *= 2
	}
	return physicalHitResult{Damage: uint32(clampInt(dam, 0, int(maxScoreValue))),
		Hit: true, Double: double, Critical: critical}
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
	return mobPhysicalHitPlayerAt(m, def, intn, now).Damage
}

func mobPhysicalHitPlayerAt(m *Mob, def *model.Char, intn func(int) int, now time.Time) physicalHitResult {
	if m == nil || m.Def == nil || def == nil {
		return physicalHitResult{}
	}
	if !combatRollHits(mobVersusPlayerAccuracy(m.Def, def), intn) {
		return physicalHitResult{}
	}
	damBase := effectiveMobAttackAt(m, now) + int(m.Def.Score.Str)/2 +
		int(m.Def.Score.Dex)/4 + int(m.Def.Score.Level)
	defense := playerDefense(def)
	dam := hitDamageWithRNG(damBase, defense, 0, intn)
	return physicalHitResult{Damage: uint32(clampInt(dam, 0, int(maxScoreValue))), Hit: true}
}

func mobHitsPlayer(m *Mob, def *model.Char) uint32 {
	return mobHitsPlayerWithRNG(m, def, realRNG{}.Intn)
}

func (w *World) mobHitsPlayer(m *Mob, def *model.Char) uint32 {
	return mobHitsPlayerAt(m, def, w.intn, w.now())
}
