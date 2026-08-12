package wire

// SkillHitsWide preserva o prefixo nativo 7.48 e acrescenta os danos reais
// que o patch .xstat ja sabe consumir. Isso evita enviar um segundo 0x39D por
// alvo apenas para o floating damage.
//
// Layouts lidos pelo Patch-WYD748-ExtendedStats.ps1:
//   0x39D: dano uint32 @48 (52 bytes total)
//   0x39E: DMGX@52, count@56, danos @60+i*4
//   0x36C: DMGX@96, count@100, danos @104+i*4
//
// Os WORDs do prefixo continuam vindo de wireDamage(target), portanto a barra
// de HP permanece na escala projetada do alvo; a cauda leva o dano real apenas
// para o texto flutuante.
func SkillHitsWide(attackerID, attackerX, attackerY, targetX, targetY uint16,
	currentExp, currentMP uint32, skill int16, motion, mastery byte,
	maxTargets int, targets []SkillTarget) []byte {
	capacity := 1
	if maxTargets == 2 {
		capacity = 2
	} else if maxTargets > 2 {
		capacity = 13
	}
	if len(targets) > capacity {
		targets = targets[:capacity]
	}

	base := SkillHits(attackerID, attackerX, attackerY, targetX, targetY,
		currentExp, currentMP, skill, motion, mastery, maxTargets, targets)
	if len(targets) == 0 {
		return base
	}

	// O pacote wide substitui o antigo par compacto+SkillHitExtended. O
	// FlagLocal=0 e intencional para TODA a familia: o OnPacketAttack local e o
	// caminho que cria o floating damage. O formato anterior tambem enviava os
	// 0x39D extras com esse flag zerado; consolidar sem zera-lo faria o atacante
	// deixar de ver os numeros embora observadores ainda processassem o hit.
	base[30] = 0

	// 0x39D possui o formato wide historico: apenas um DWORD em @48.
	if maxTargets <= 1 {
		extended := make([]byte, 52)
		copy(extended, base)
		putU16(extended, 0, uint16(len(extended)))
		putU32(extended, 48, targets[0].Damage)
		return extended
	}

	// O helper do patch reserva oito bytes antes do vetor. Ele hoje testa o
	// tamanho e le o vetor diretamente, mas manter assinatura+count deixa o
	// contrato auto-descritivo e permite validacao futura sem mudar offsets.
	baseSize := len(base)
	extended := make([]byte, baseSize+8+4*len(targets))
	copy(extended, base)
	putU16(extended, 0, uint16(len(extended)))
	putU32(extended, baseSize, 0x58474D44) // "DMGX" little-endian.
	putU32(extended, baseSize+4, uint32(len(targets)))
	for i, target := range targets {
		putU32(extended, baseSize+8+i*4, target.Damage)
	}
	return extended
}
