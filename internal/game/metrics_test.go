package game

import (
	"testing"
	"time"

	"wydgo/internal/wire"
)

func TestObserveTickRecordsLagOnlyWhenLate(t *testing.T) {
	start := time.Date(2026, 7, 24, 12, 0, 0, 0, time.UTC)
	lastTickAt.Store(0)
	metricLoopLagMicros.Set(0)
	metricLoopLagMaxMicros.Set(0)

	// Primeiro tick: sem intervalo anterior, nao ha lag a calcular.
	observeTick(start, 3*time.Millisecond)
	if got := metricLoopLagMicros.Value(); got != 0 {
		t.Fatalf("primeiro tick nao deveria reportar lag, veio %d", got)
	}
	if got := metricTickDurationMicros.Value(); got != 3000 {
		t.Fatalf("duracao=%d micros, quer 3000", got)
	}

	// Tick pontual (exatamente o intervalo nominal): lag zero.
	observeTick(start.Add(worldTickInterval), time.Millisecond)
	if got := metricLoopLagMicros.Value(); got != 0 {
		t.Fatalf("tick pontual reportou lag=%d", got)
	}

	// Tick atrasado 200 ms alem do intervalo nominal.
	late := start.Add(worldTickInterval).Add(worldTickInterval + 200*time.Millisecond)
	observeTick(late, time.Millisecond)
	if got := metricLoopLagMicros.Value(); got != 200_000 {
		t.Fatalf("lag=%d micros, quer 200000", got)
	}
	if got := metricLoopLagMaxMicros.Value(); got != 200_000 {
		t.Fatalf("lag maximo=%d micros, quer 200000", got)
	}

	// Um tick pontual depois nao pode APAGAR o pior caso observado.
	observeTick(late.Add(worldTickInterval), time.Millisecond)
	if got := metricLoopLagMaxMicros.Value(); got != 200_000 {
		t.Fatalf("lag maximo foi perdido: %d", got)
	}
}

// TestCommandLabelIsBoundsSafe protege uma regressao real: wire.ParseHeader
// indexa 12 bytes sem checar o tamanho, e o rotulo e calculado FORA do recover
// de safeHandle. Um pacote truncado nao pode derrubar o game loop.
func TestCommandLabelIsBoundsSafe(t *testing.T) {
	for _, tc := range []struct {
		name string
		cmd  command
		want string
	}{
		{"sem pacote", command{login: &loginResult{}}, "login"},
		{"truncado", command{pkt: []byte{1, 2, 3}}, "malformed"},
		{"vazio", command{pkt: []byte{}}, "malformed"},
		{"opcode desconhecido", command{pkt: make([]byte, wire.HeaderSize)}, "unknown"},
		{"completo", command{pkt: inboundPacket(wire.OpPing, wire.HeaderSize)}, "0x3A0"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if got := commandLabel(tc.cmd); got != tc.want {
				t.Fatalf("commandLabel=%q, quer %q", got, tc.want)
			}
		})
	}
}

// TestSafeHandleRejectsTruncatedPacketBeforeParsing confirma que a fronteira de
// seguranca recusa framing invalido sem depender do recover do game loop.
func TestSafeHandleRejectsTruncatedPacketBeforeParsing(t *testing.T) {
	before := metricPanicsTotal.Value()
	w := &World{}

	w.safeHandle(command{pkt: []byte{1, 2, 3}})

	if got := metricPanicsTotal.Value(); got != before {
		t.Fatalf("pacote truncado chegou ao parser e gerou panic: antes=%d depois=%d", before, got)
	}
}
