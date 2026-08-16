package loadtest

import (
	"bytes"
	"context"
	"encoding/binary"
	"log"
	stdnet "net"
	"strings"
	"testing"
	"time"

	"wydgo/internal/wire"
)

func TestRunBotsCancelledContextCannotProgressPastConnectionAttempt(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	report := RunBots(ctx, RunConfig{
		Address: "127.0.0.1:0", BotCount: 2, Ramp: time.Millisecond, Duration: time.Second,
	}, nil)
	// O primeiro worker tem delay zero. Quando timer.C e ctx.Done ja estao
	// prontos, select pode escolher qualquer um; portanto Started pode ser 0 ou 1.
	// O contrato relevante e que uma execucao cancelada nao consegue conectar,
	// entrar no mundo nem produzir gameplay.
	if report.Started > 1 || report.Connected != 0 || report.Entered != 0 ||
		report.Attacks != 0 || report.Moves != 0 || report.Packets != 0 || report.Errors > 1 {
		t.Fatalf("contexto ja cancelado progrediu carga alem do worker inicial: %+v", report)
	}
}

func TestRunBotsClampsOversizedBotCountBeforeCancelledRamp(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	var logs bytes.Buffer
	logger := log.New(&logs, "", 0)
	report := RunBots(ctx, RunConfig{
		Address: "127.0.0.1:0", BotCount: 5000, Ramp: time.Second, Duration: time.Second,
	}, logger)
	if !strings.Contains(logs.String(), "limitando bots de 5000 para 990") {
		t.Fatalf("clamp de 990 nao foi observado no log: %q", logs.String())
	}
	if report.Started > 1 || report.Connected != 0 || report.Entered != 0 {
		t.Fatalf("rampa cancelada progrediu carga inesperadamente: %+v", report)
	}
}

func TestBotNearestTargetUsesServerObservedPositions(t *testing.T) {
	b := &bot{targets: map[uint16]targetPosition{
		10: {x: 110, y: 100},
		20: {x: 102, y: 103},
		30: {x: 99, y: 99},
	}}
	b.predictedX.Store(100)
	b.predictedY.Store(100)
	id, pos := b.nearestTarget()
	if id != 30 || pos.x != 99 || pos.y != 99 {
		t.Fatalf("alvo mais proximo incorreto: id=%d pos=%+v", id, pos)
	}
	if d := chebyshevDistance(100, 100, 102, 103); d != 3 {
		t.Fatalf("Chebyshev=%d, esperado 3", d)
	}
	if absLoadtest(-7) != 7 || signLoadtest(-7) != -1 || signLoadtest(0) != 0 || signLoadtest(7) != 1 {
		t.Fatal("helpers de direcao/distancia divergiram")
	}
}

type botPacketResult struct {
	pkt      []byte
	checksum bool
	err      error
}

func TestBotSendAttackBuildsPhysicalAndSkillIntentOnly(t *testing.T) {
	for _, tc := range []struct {
		name   string
		skill  int
		opcode uint16
	}{
		{name: "physical", skill: -1, opcode: wire.OpAttackOne},
		{name: "skill", skill: 7, opcode: wire.OpAttackOne},
	} {
		t.Run(tc.name, func(t *testing.T) {
			client, server := stdnet.Pipe()
			defer client.Close()
			defer server.Close()
			b := &bot{
				config: RunConfig{Skill: tc.skill}, conn: client, counters: &loadCounters{},
				targets: map[uint16]targetPosition{500: {x: 101, y: 100}},
			}
			b.id.Store(9)
			b.applyServerPosition(100, 100)
			result := make(chan botPacketResult, 1)
			go func() {
				pkt, checksum, err := wire.ReadPacket(server)
				result <- botPacketResult{pkt: pkt, checksum: checksum, err: err}
			}()
			if err := b.sendAttack(); err != nil {
				t.Fatal(err)
			}
			got := <-result
			if got.err != nil || !got.checksum {
				t.Fatalf("bot escreveu frame invalido: checksum=%v err=%v", got.checksum, got.err)
			}
			pkt := got.pkt
			if opcode := wire.ParseHeader(pkt).Type; opcode != tc.opcode {
				t.Fatalf("opcode=0x%X, esperado 0x%X", opcode, tc.opcode)
			}
			if binary.LittleEndian.Uint16(pkt[44:46]) != 500 {
				t.Fatalf("alvo do ataque=%d, esperado 500", binary.LittleEndian.Uint16(pkt[44:46]))
			}
		})
	}
}

func TestBotSendAttackWithoutTargetOrConnectionFailsSafely(t *testing.T) {
	b := &bot{config: RunConfig{Skill: -1}, counters: &loadCounters{}, targets: map[uint16]targetPosition{}}
	if err := b.sendAttack(); err != nil {
		t.Fatalf("sem alvo deveria ser no-op: %v", err)
	}
	b.targets[1] = targetPosition{x: 1, y: 1}
	if err := b.sendAttack(); err == nil {
		t.Fatal("alvo com conexao ausente deveria falhar no envio")
	}
}

func TestInitCodeBytesUsesNativeLittleEndianValue(t *testing.T) {
	got := initCodeBytes()
	var want [4]byte
	binary.LittleEndian.PutUint32(want[:], uint32(wire.InitCode))
	if !bytes.Equal(got, want[:]) {
		t.Fatalf("InitCode=% X, esperado=% X", got, want)
	}
}
