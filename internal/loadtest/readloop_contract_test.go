package loadtest

import (
	"context"
	"encoding/binary"
	mathrand "math/rand"
	stdnet "net"
	"testing"
	"time"

	"wydgo/internal/wire"
)

func finalizedServerPacket(opcode, id uint16, size int, fill func([]byte)) []byte {
	pkt := wire.Build(opcode, id, size)
	if fill != nil {
		fill(pkt)
	}
	wire.FinishPacket(pkt, 7)
	return pkt
}

func TestReadLoopCharListSendsCharacterLogin(t *testing.T) {
	client, server := stdnet.Pipe()
	defer client.Close()
	defer server.Close()
	b := &bot{conn: client, counters: &loadCounters{}, entered: make(chan struct{}), targets: make(map[uint16]targetPosition)}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	readDone := make(chan error, 1)
	go func() { readDone <- b.readLoop(ctx) }()

	response := make(chan struct {
		pkt []byte
		ok  bool
		err error
	}, 1)
	go func() {
		pkt, ok, err := wire.ReadPacket(server)
		response <- struct {
			pkt []byte
			ok  bool
			err error
		}{pkt, ok, err}
	}()
	if _, err := server.Write(finalizedServerPacket(wire.OpCharList, wire.SceneCharList, wire.HeaderSize, nil)); err != nil {
		t.Fatal(err)
	}
	got := <-response
	if got.err != nil || !got.ok || wire.ParseHeader(got.pkt).Type != wire.OpCharacterLogin || len(got.pkt) != 36 {
		t.Fatalf("resposta ao CharList invalida: ok=%v err=%v pkt=%v", got.ok, got.err, got.pkt)
	}
	if b.counters.packets.Load() != 1 || b.counters.bytesRecv.Load() != wire.HeaderSize {
		t.Fatalf("contadores recv divergiram packets=%d bytes=%d", b.counters.packets.Load(), b.counters.bytesRecv.Load())
	}
	_ = server.Close()
	select {
	case err := <-readDone:
		if err != nil {
			t.Fatalf("EOF deveria encerrar limpo: %v", err)
		}
	case <-time.After(time.Second):
		t.Fatal("readLoop nao encerrou apos EOF")
	}
}

func TestReadLoopTracksWorldPositionTargetsAndRemoval(t *testing.T) {
	client, server := stdnet.Pipe()
	defer client.Close()
	b := &bot{
		conn: client, counters: &loadCounters{}, entered: make(chan struct{}),
		targets: make(map[uint16]targetPosition), config: RunConfig{SpawnX: 1162, SpawnY: 1700},
	}
	readDone := make(chan error, 1)
	go func() { readDone <- b.readLoop(context.Background()) }()

	write := func(pkt []byte) {
		t.Helper()
		if _, err := server.Write(pkt); err != nil {
			t.Fatal(err)
		}
	}
	write(finalizedServerPacket(wire.OpEnterWorld, 55, 20, func(pkt []byte) {
		binary.LittleEndian.PutUint16(pkt[12:14], 1200)
		binary.LittleEndian.PutUint16(pkt[14:16], 1300)
	}))
	select {
	case <-b.entered:
	case <-time.After(time.Second):
		t.Fatal("EnterWorld nao fechou canal entered")
	}
	if b.id.Load() != 55 || b.predictedX.Load() != 1200 || b.predictedY.Load() != 1300 ||
		b.confirmedX.Load() != 1200 || b.confirmedY.Load() != 1300 {
		t.Fatalf("EnterWorld nao reconciliou estado: id=%d pred=(%d,%d) conf=(%d,%d)", b.id.Load(), b.predictedX.Load(), b.predictedY.Load(), b.confirmedX.Load(), b.confirmedY.Load())
	}

	write(finalizedServerPacket(wire.OpCreateMob, 1001, 20, func(pkt []byte) {
		binary.LittleEndian.PutUint16(pkt[12:14], 1210)
		binary.LittleEndian.PutUint16(pkt[14:16], 1310)
	}))
	deadline := time.Now().Add(time.Second)
	for {
		b.targetMu.RLock()
		pos, ok := b.targets[1001]
		b.targetMu.RUnlock()
		if ok {
			if pos.x != 1210 || pos.y != 1310 {
				t.Fatalf("CreateMob pos=%+v", pos)
			}
			break
		}
		if time.Now().After(deadline) {
			t.Fatal("CreateMob nao entrou no tracking")
		}
		time.Sleep(time.Millisecond)
	}

	write(finalizedServerPacket(wire.OpAction, 55, 20, func(pkt []byte) {
		binary.LittleEndian.PutUint16(pkt[12:14], 1250)
		binary.LittleEndian.PutUint16(pkt[14:16], 1350)
	}))
	deadline = time.Now().Add(time.Second)
	for b.confirmedX.Load() != 1250 {
		if time.Now().After(deadline) {
			t.Fatal("Action do proprio bot nao reconciliou posicao")
		}
		time.Sleep(time.Millisecond)
	}

	// Action de outro ID nao pode mover o bot.
	write(finalizedServerPacket(wire.OpAction, 99, 20, func(pkt []byte) {
		binary.LittleEndian.PutUint16(pkt[12:14], 1)
		binary.LittleEndian.PutUint16(pkt[14:16], 1)
	}))
	write(finalizedServerPacket(wire.OpRemoveMob, 1001, wire.HeaderSize, nil))
	deadline = time.Now().Add(time.Second)
	for {
		b.targetMu.RLock()
		_, ok := b.targets[1001]
		b.targetMu.RUnlock()
		if !ok {
			break
		}
		if time.Now().After(deadline) {
			t.Fatal("RemoveMob nao removeu target")
		}
		time.Sleep(time.Millisecond)
	}
	if b.confirmedX.Load() != 1250 || b.confirmedY.Load() != 1350 {
		t.Fatal("Action alheia alterou posicao")
	}

	_ = server.Close()
	if err := <-readDone; err != nil {
		t.Fatalf("readLoop terminou com erro: %v", err)
	}
	if b.counters.packets.Load() != 5 {
		t.Fatalf("packets=%d, esperado 5", b.counters.packets.Load())
	}
}

func TestReadLoopEnterWorldShortPacketUsesConfiguredSpawn(t *testing.T) {
	client, server := stdnet.Pipe()
	defer client.Close()
	b := &bot{
		conn: client, counters: &loadCounters{}, entered: make(chan struct{}), targets: make(map[uint16]targetPosition),
		config: RunConfig{SpawnX: 1162, SpawnY: 1700},
	}
	done := make(chan error, 1)
	go func() { done <- b.readLoop(context.Background()) }()
	if _, err := server.Write(finalizedServerPacket(wire.OpEnterWorld, 7, wire.HeaderSize, nil)); err != nil {
		t.Fatal(err)
	}
	<-b.entered
	if b.predictedX.Load() != 1162 || b.predictedY.Load() != 1700 {
		t.Fatalf("fallback spawn=(%d,%d)", b.predictedX.Load(), b.predictedY.Load())
	}
	_ = server.Close()
	if err := <-done; err != nil {
		t.Fatal(err)
	}
}

func TestReadLoopRejectsInvalidChecksum(t *testing.T) {
	client, server := stdnet.Pipe()
	defer client.Close()
	b := &bot{conn: client, counters: &loadCounters{}, entered: make(chan struct{}), targets: make(map[uint16]targetPosition)}
	done := make(chan error, 1)
	go func() { done <- b.readLoop(context.Background()) }()
	pkt := finalizedServerPacket(wire.OpEnterWorld, 1, 20, nil)
	pkt[len(pkt)-1] ^= 0x7F
	if _, err := server.Write(pkt); err != nil {
		t.Fatal(err)
	}
	_ = server.Close()
	if err := <-done; err == nil {
		t.Fatal("checksum corrompido foi aceito")
	}
}

func TestActivityLoopCoversTickerMoveAndAttackAccounting(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), 1050*time.Millisecond)
	defer cancel()
	b := &bot{
		config: RunConfig{MovePercent: 100}, counters: &loadCounters{},
		targets: make(map[uint16]targetPosition), rng: mathrand.New(mathrand.NewSource(1)),
	}
	b.activityLoop(ctx)
	if b.counters.moves.Load() < 1 || b.counters.attacks.Load() < 1 {
		t.Fatalf("activityLoop nao contabilizou ciclo: moves=%d attacks=%d", b.counters.moves.Load(), b.counters.attacks.Load())
	}
}
