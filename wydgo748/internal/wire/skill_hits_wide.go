package wire

// SkillHits keeps the native 7.48 prefix and appends the real 32-bit values
// the TMProject748 client already consumes, so no second 0x39D per target is
// needed just for the floating number.
//
// Layouts read by the TMProject748 client:
//
//	0x39D skill: DMGX@48, count@52, uint32 value @56 (60 bytes total)
//	0x39E: DMGX@52, count@56, values @60+i*4
//	0x36C: DMGX@96, count@100, values @104+i*4
//
// The prefix WORDs still come from wireDamage(target) and only drive the local
// HP bar until the authoritative SetHpMp arrives. The tail carries the real
// damage, or the real heal for a heal target, for the floating number.
func SkillHits(attackerID, attackerX, attackerY, targetX, targetY uint16,
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

	base := buildSkillHitsPacket(attackerID, attackerX, attackerY, targetX, targetY,
		currentExp, currentMP, skill, motion, mastery, maxTargets, targets)
	if len(targets) == 0 {
		return base
	}

	// The wide packet replaces the old compact + SkillHitExtended pair.
	// FlagLocal=0 is intentional for the WHOLE family: the local OnPacketAttack
	// path creates the floating number. The previous format also sent the extra
	// 0x39D with this flag cleared; keeping it set would hide the numbers from
	// the attacker while observers still processed the hit.
	base[30] = 0

	// Skill single-target must not collide with the physical 0x39D/52 layout.
	// The source-built client deliberately reserves 52 bytes for melee and reads a
	// skill's self-describing DMGX tail only from a packet of at least 60 bytes.
	if maxTargets <= 1 {
		extended := make([]byte, 60)
		copy(extended, base)
		putU16(extended, 0, uint16(len(extended)))
		putU32(extended, 48, 0x58474D44)
		putU32(extended, 52, 1)
		putU32(extended, 56, wideTailValue(targets[0]))
		return extended
	}

	// The client helper reserves eight bytes before the vector. It checks the
	// size and reads the vector directly; signature and count keep the contract
	// self-describing and allow future validation without moving offsets.
	baseSize := len(base)
	extended := make([]byte, baseSize+8+4*len(targets))
	copy(extended, base)
	putU16(extended, 0, uint16(len(extended)))
	putU32(extended, baseSize, 0x58474D44) // "DMGX" little-endian.
	putU32(extended, baseSize+4, uint32(len(targets)))
	for i, target := range targets {
		putU32(extended, baseSize+8+i*4, wideTailValue(target))
	}
	return extended
}

// wideTailValue is the real 32-bit amount behind the saturated prefix WORD: the
// heal for a heal target, zero for a miss, otherwise the damage.
func wideTailValue(t SkillTarget) uint32 {
	switch {
	case t.Miss:
		return 0
	case t.Heal > 0:
		return t.Heal
	default:
		return t.Damage
	}
}
