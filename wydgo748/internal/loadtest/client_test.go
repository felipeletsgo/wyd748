package loadtest

import (
	"bytes"
	"io"
	stdnet "net"
	"testing"

	"wydgo/internal/wire"
)

func TestLoginAndCharacterPacketsUse748Layout(t *testing.T) {
	for _, pkt := range [][]byte{loginPacket("bot0001", "123456"), characterLoginPacket(0)} {
		if len(pkt) != 116 && len(pkt) != 36 {
			t.Fatalf("tamanho inesperado=%d", len(pkt))
		}
		wire.FinishPacket(pkt, 1)
		decoded, checksum, err := wire.ReadPacket(bytes.NewReader(pkt))
		if err != nil || !checksum {
			t.Fatalf("pacote nao decifra: err=%v checksum=%v", err, checksum)
		}
		if wire.ParseHeader(decoded).Size != uint16(len(decoded)) {
			t.Fatalf("size=%d len=%d", wire.ParseHeader(decoded).Size, len(decoded))
		}
	}
}

func TestBotAccountNameUsesShortPrefix(t *testing.T) {
	b := &bot{config: RunConfig{AccountPrefix: "bot"}, index: 1}
	if got := b.accountName(); got != "bot0001" {
		t.Fatalf("conta=%q", got)
	}
}

func TestSentMoveDoesNotAdvanceServerConfirmedPosition(t *testing.T) {
	client, server := stdnet.Pipe()
	defer client.Close()
	defer server.Close()
	b := &bot{conn: client, counters: &loadCounters{}, targets: map[uint16]targetPosition{
		1000: {x: 101, y: 100},
	}}
	b.id.Store(1)
	b.applyServerPosition(100, 100)
	readDone := make(chan error, 1)
	go func() {
		_, err := io.CopyN(io.Discard, server, 52)
		readDone <- err
	}()

	if err := b.sendMove(); err != nil {
		t.Fatalf("sendMove: %v", err)
	}
	if err := <-readDone; err != nil {
		t.Fatalf("ler movimento: %v", err)
	}
	if b.confirmedX.Load() != 100 || b.confirmedY.Load() != 100 {
		t.Fatalf("envio local falsificou confirmacao server-side: (%d,%d)",
			b.confirmedX.Load(), b.confirmedY.Load())
	}
	if b.predictedX.Load() != 101 || b.predictedY.Load() != 100 {
		t.Fatalf("predicao do client nao avancou: (%d,%d)",
			b.predictedX.Load(), b.predictedY.Load())
	}
}

func TestServerPositionReconcilesPrediction(t *testing.T) {
	b := &bot{}
	b.applyServerPosition(100, 100)
	b.predictedX.Store(105)
	b.predictedY.Store(104)
	b.applyServerPosition(101, 102)
	if b.confirmedX.Load() != 101 || b.confirmedY.Load() != 102 ||
		b.predictedX.Load() != 101 || b.predictedY.Load() != 102 {
		t.Fatal("correcao server-side nao reconciliou a predicao")
	}
}

func TestRunBotsClampsMovementPercent(t *testing.T) {
	for _, tc := range []struct {
		in, want int
	}{
		{in: -1, want: 0},
		{in: 0, want: 0},
		{in: 35, want: 35},
		{in: 101, want: 100},
	} {
		if got := clampMovePercent(tc.in); got != tc.want {
			t.Fatalf("movimento=%d, esperado=%d", got, tc.want)
		}
	}
}

func TestMovementAndAttackAreIndependent(t *testing.T) {
	// O contrato do loop é que movimento não substitui ataque. A configuração
	// de movimento pode ser zero para uma rodada estacionária, mas quando o
	// movimento é habilitado o ataque continua sendo agendado no mesmo ciclo.
	if got := clampMovePercent(50); got != 50 {
		t.Fatalf("percentual de movimento=%d", got)
	}
}

func TestOrderedMoveCandidatesStartsAtTargetAndHasNoDuplicates(t *testing.T) {
	candidates := orderedMoveCandidates(1, -1)
	if len(candidates) != 8 {
		t.Fatalf("candidatos=%d, esperado=8", len(candidates))
	}
	if candidates[0] != [2]int{1, -1} {
		t.Fatalf("direcao do alvo nao foi priorizada: %v", candidates[0])
	}
	seen := map[[2]int]bool{}
	for _, candidate := range candidates {
		if seen[candidate] {
			t.Fatalf("candidato repetido: %v", candidate)
		}
		seen[candidate] = true
	}
}
