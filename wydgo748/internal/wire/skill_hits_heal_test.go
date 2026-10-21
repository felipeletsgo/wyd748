package wire

import (
	"encoding/binary"
	"testing"
)

// The prefix WORD saturates at 32767, so heals carry their real 32-bit amount
// in the DMGX tail like damage does; a miss keeps a zero tail.
func TestSkillHitsTailCarriesRealHealAndDamage(t *testing.T) {
	single := SkillHits(1, 10, 10, 11, 11, 0, 0, 27, 6, 0, 1,
		[]SkillTarget{{ID: 2, Heal: 250000}})
	if len(single) != 60 || binary.LittleEndian.Uint32(single[56:60]) != 250000 {
		t.Fatalf("single heal tail = %d (len %d)", binary.LittleEndian.Uint32(single[56:60]), len(single))
	}
	if word := int16(binary.LittleEndian.Uint16(single[46:48])); word != -32767 {
		t.Fatalf("heal prefix word = %d, want saturated -32767", word)
	}
	multi := SkillHits(1, 10, 10, 11, 11, 0, 0, 27, 6, 0, 13, []SkillTarget{
		{ID: 2, Heal: 70000}, {ID: 3, Damage: 1500000}, {ID: 4, Miss: true},
	})
	base := len(multi) - 8 - 3*4
	for i, want := range []uint32{70000, 1500000, 0} {
		if got := binary.LittleEndian.Uint32(multi[base+8+i*4:]); got != want {
			t.Fatalf("target %d tail = %d, want %d", i, got, want)
		}
	}
}
