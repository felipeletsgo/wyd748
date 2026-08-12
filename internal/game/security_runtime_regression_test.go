package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/wire"
)

func TestValidateInboundCommandAcceptsObservedAttackOne96Layout(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Attack96", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	packet := inboundPacket(wire.OpAttackOne, attackOneObservedExtendedSize)

	if !w.validateInboundCommand(p.Session, packet) {
		t.Fatal("0x39D de 96 bytes observado no client 7.48 foi recusado")
	}
	if state := w.security[p.Session]; state != nil && state.violations != 0 {
		t.Fatalf("0x39D/96 valido foi contabilizado como violacao: %+v", state)
	}
}

func TestValidateInboundCommandStillRejectsArbitraryAttackOneTail(t *testing.T) {
	p, _ := networkedTestPlayer(1, "AttackTail", 2100, 2100)
	w := worldWithNetworkedPlayers(p)

	if w.validateInboundCommand(p.Session, inboundPacket(wire.OpAttackOne, 80)) {
		t.Fatal("0x39D com tamanho nao observado foi aceito")
	}
}

func TestMovementReconcilesObservedVisualLeadWithoutTeleport(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	p.X, p.Y = 1115, 1706
	p.Char.X, p.Char.Y = p.X, p.Y

	// Reproduz a primeira recusa do log real:
	// auth=(1115,1706) pos=(1128,1708) target=(1134,1708) route="666666".
	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], 1128)
	binary.LittleEndian.PutUint16(move[14:16], 1708)
	binary.LittleEndian.PutUint32(move[16:20], 6)
	binary.LittleEndian.PutUint16(move[24:26], 1134)
	binary.LittleEndian.PutUint16(move[26:28], 1708)
	copy(move[28:34], []byte("666666"))

	_, _, wireRoute, authorityRoute, ok := w.validatedPlayerMoveRoute(p, move)
	if !ok {
		t.Fatal("plano legitimo com 13 tiles de lead visual foi recusado")
	}
	if len(wireRoute) != 6 {
		t.Fatalf("wire route=%d, want 6", len(wireRoute))
	}
	if len(authorityRoute) <= len(wireRoute) || len(authorityRoute) > maxMovementQueuedSteps {
		t.Fatalf("ponte autoritativa invalida: wire=%d authority=%d", len(wireRoute), len(authorityRoute))
	}
	if p.X != 1115 || p.Y != 1706 {
		t.Fatalf("reconciliacao promoveu PosX/Y do client como teleporte: (%d,%d)", p.X, p.Y)
	}
}

func TestMovementStillRejectsVisualLeadBeyondOneFullRoute(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	p.X, p.Y = 1115, 1706
	p.Char.X, p.Char.Y = p.X, p.Y

	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], 1140) // 25 tiles a frente.
	binary.LittleEndian.PutUint16(move[14:16], 1706)
	binary.LittleEndian.PutUint16(move[24:26], 1141)
	binary.LittleEndian.PutUint16(move[26:28], 1706)
	move[28] = '6'

	if _, _, _, _, ok := w.validatedPlayerMoveRoute(p, move); ok {
		t.Fatal("lead visual acima de Route[24] foi aceito")
	}
}
