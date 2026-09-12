package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/wire"
)

func TestInboundLearnedSkillClearsOnlyGlobalSkillThrottle(t *testing.T) {
	p, _ := networkedTestPlayer(1, "SkillIngress", 2100, 2100)
	p.Char.Class = 0
	p.Char.LearnedSkill = 1 << 0
	w := worldWithNetworkedPlayers(p)

	pkt := inboundPacket(wire.OpAttackOne, 48)
	binary.LittleEndian.PutUint16(pkt[24:26], 0)
	binary.LittleEndian.PutUint32(pkt[8:12], 1200)
	p.LastSkillAt = time.Unix(10, 0)

	if !w.validateInboundCommand(p.Session, pkt) {
		t.Fatal("skill aprendida valida foi recusada na borda")
	}
	if !p.LastSkillAt.IsZero() {
		t.Fatalf("piso global entre skills permaneceu ativo: %v", p.LastSkillAt)
	}
}

func TestInboundPhysicalAttackDoesNotClearSkillThrottleState(t *testing.T) {
	p, _ := networkedTestPlayer(1, "PhysicalIngress", 2100, 2100)
	p.Char.Class = 0
	p.Char.LearnedSkill = 1 << 0
	w := worldWithNetworkedPlayers(p)

	pkt := inboundPacket(wire.OpAttackOne, 48)
	binary.LittleEndian.PutUint16(pkt[24:26], 0xFFFF)
	binary.LittleEndian.PutUint32(pkt[8:12], 1300)
	last := time.Unix(20, 0)
	p.LastSkillAt = last

	if !w.validateInboundCommand(p.Session, pkt) {
		t.Fatal("ataque fisico valido foi recusado na borda")
	}
	if !p.LastSkillAt.Equal(last) {
		t.Fatalf("ataque fisico alterou estado do throttle de skill: got=%v want=%v", p.LastSkillAt, last)
	}
}
