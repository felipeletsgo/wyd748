package game

import (
	"fmt"
	"testing"

	"wydgo/internal/wire"
)

// TestCapsuleOpcodesCrossTheSameInboundBoundaryAsOtherGameplayPackets closes a
// coverage hole in the canonical framing/dispatch tests. Both handlers are
// economically sensitive: 0x2CD exposes a persisted Celestial snapshot and
// 0x3CC materializes it back into a character, so neither may bypass exact
// framing or the World dispatcher.
func TestCapsuleOpcodesCrossTheSameInboundBoundaryAsOtherGameplayPackets(t *testing.T) {
	tests := []struct {
		opcode uint16
		size   int
	}{
		{wire.OpCapsuleInfo, 16},
		{wire.OpPutoutSeal, 52},
	}

	for _, tc := range tests {
		tc := tc
		t.Run(fmt.Sprintf("opcode_0x%X", tc.opcode), func(t *testing.T) {
			if size, exact := exactInboundPacketSize(tc.opcode); !exact || size != tc.size {
				t.Fatalf("framing do opcode 0x%X: size=%d exact=%v, esperado=%d",
					tc.opcode, size, exact, tc.size)
			}

			w, p, _ := handlerTestWorld(t)
			valid := inboundPacket(tc.opcode, tc.size)
			if !w.validateInboundCommand(p.Session, valid) {
				t.Fatalf("opcode 0x%X com layout 7.48 valido foi recusado", tc.opcode)
			}

			// Exercise the real World dispatch as well. The zero-filled semantic
			// payload is expected to be rejected by the feature handler, but it
			// must reach that handler without panic or a protocol violation.
			before := 0
			if state := w.security[p.Session]; state != nil {
				before = state.violations
			}
			w.handle(command{s: p.Session, pkt: valid})
			after := 0
			if state := w.security[p.Session]; state != nil {
				after = state.violations
			}
			if after != before {
				t.Fatalf("opcode 0x%X valido virou violacao no dispatch: %d -> %d",
					tc.opcode, before, after)
			}

			for _, badSize := range []int{tc.size - 1, tc.size + 1, tc.size + 16} {
				wBad, pBad, _ := handlerTestWorld(t)
				if wBad.validateInboundCommand(pBad.Session, inboundPacket(tc.opcode, badSize)) {
					t.Fatalf("opcode 0x%X aceitou tamanho invalido %d", tc.opcode, badSize)
				}
			}
		})
	}
}

// TestSessionTransitionPhaseMatrix protects the server-authoritative login
// state machine independently from individual handlers. A future opcode added
// to character selection must not accidentally become legal while connected,
// authenticating, or already in the world.
func TestSessionTransitionPhaseMatrix(t *testing.T) {
	transitionOpcodes := []uint16{
		wire.OpConnectAccount,
		wire.OpCreateCharacter,
		wire.OpDeleteCharacter,
		wire.OpCharacterLogin,
	}

	for _, opcode := range transitionOpcodes {
		opcode := opcode
		t.Run(fmt.Sprintf("opcode_0x%X", opcode), func(t *testing.T) {
			want := map[sessionPhase]bool{
				phaseConnected:       opcode == wire.OpConnectAccount,
				phaseAuthenticating:  false,
				phaseCharacterSelect: opcode == wire.OpCreateCharacter ||
					opcode == wire.OpDeleteCharacter || opcode == wire.OpCharacterLogin,
				phaseWorld: false,
			}
			for phase, allowed := range want {
				if got := opcodeAllowedInPhase(phase, opcode); got != allowed {
					t.Fatalf("fase=%d opcode=0x%X allowed=%v, esperado=%v",
						phase, opcode, got, allowed)
				}
			}
		})
	}

	for _, opcode := range []uint16{wire.OpPing, wire.OpSysQuit} {
		for _, phase := range []sessionPhase{
			phaseConnected, phaseAuthenticating, phaseCharacterSelect, phaseWorld,
		} {
			if !opcodeAllowedInPhase(phase, opcode) {
				t.Fatalf("opcode universal 0x%X recusado na fase %d", opcode, phase)
			}
		}
	}
}
