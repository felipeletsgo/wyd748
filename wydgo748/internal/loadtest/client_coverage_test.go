package loadtest

import (
	"bytes"
	"context"
	"encoding/binary"
	"errors"
	"io"
	"log"
	mathrand "math/rand"
	"net"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestRunBotsCompletesLocal748Handshake(t *testing.T) {
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer listener.Close()
	serverDone := make(chan error, 1)
	go func() {
		conn, err := listener.Accept()
		if err != nil {
			serverDone <- err
			return
		}
		defer conn.Close()
		var init [4]byte
		if _, err := io.ReadFull(conn, init[:]); err != nil {
			serverDone <- err
			return
		}
		if binary.LittleEndian.Uint32(init[:]) != uint32(wire.InitCode) {
			serverDone <- errors.New("wrong InitCode")
			return
		}
		login, checksum, err := wire.ReadPacket(conn)
		if err != nil || !checksum || wire.ParseHeader(login).Type != wire.OpConnectAccount {
			serverDone <- errors.New("invalid account login")
			return
		}
		if _, err := conn.Write(loadtestServerPacket(wire.OpCharList, wire.SceneCharList, 12, 0, 0)); err != nil {
			serverDone <- err
			return
		}
		selection, checksum, err := wire.ReadPacket(conn)
		if err != nil || !checksum || wire.ParseHeader(selection).Type != wire.OpCharacterLogin {
			serverDone <- errors.New("invalid character login")
			return
		}
		if _, err := conn.Write(loadtestServerPacket(wire.OpEnterWorld, 7, 16, 120, 130)); err != nil {
			serverDone <- err
			return
		}
		time.Sleep(20 * time.Millisecond)
		serverDone <- nil
	}()

	report := RunBots(context.Background(), RunConfig{
		Address: listener.Addr().String(), BotCount: 1, Password: "1234",
		Ramp: time.Millisecond, Duration: 120 * time.Millisecond,
	}, nil)
	if report.Started != 1 || report.Connected != 1 || report.Entered != 1 || report.Errors != 0 ||
		report.Packets < 2 || report.BytesSent < 4+116+36 || report.BytesReceived < 24 {
		t.Fatalf("load report: %+v", report)
	}
	if err := <-serverDone; err != nil && !errors.Is(err, net.ErrClosed) {
		t.Fatalf("server: %v", err)
	}
}

func TestRunBotsStopsWhenContextEndsBeforeEntry(t *testing.T) {
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer listener.Close()
	serverDone := make(chan error, 1)
	go func() {
		conn, err := listener.Accept()
		if err != nil {
			serverDone <- err
			return
		}
		defer conn.Close()
		var init [4]byte
		if _, err := io.ReadFull(conn, init[:]); err != nil {
			serverDone <- err
			return
		}
		if _, _, err := wire.ReadPacket(conn); err != nil {
			serverDone <- err
			return
		}
		_, err = io.Copy(io.Discard, conn)
		serverDone <- err
	}()
	report := RunBots(context.Background(), RunConfig{
		Address: listener.Addr().String(), BotCount: 1, Ramp: time.Millisecond, Duration: 60 * time.Millisecond,
	}, nil)
	if report.Started != 1 || report.Connected != 1 || report.Entered != 0 || report.Errors != 0 {
		t.Fatalf("report=%+v", report)
	}
	if err := <-serverDone; err != nil {
		t.Fatal(err)
	}
}

func TestRunBotsAppliesDefaultsAndCapsCount(t *testing.T) {
	var output bytes.Buffer
	logger := log.New(&output, "", 0)
	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	defaultReport := RunBots(ctx, RunConfig{Address: "127.0.0.1:1"}, logger)
	if defaultReport.Started > 1 || defaultReport.Connected != 0 || defaultReport.Errors != defaultReport.Started {
		t.Fatalf("cancelled defaults started unexpected sessions: %+v", defaultReport)
	}

	ctx, cancel = context.WithCancel(context.Background())
	cancel()
	RunBots(ctx, RunConfig{Address: "127.0.0.1:1", BotCount: 991}, logger)
	if !strings.Contains(output.String(), "limiting bots from 991 to 990") {
		t.Fatalf("missing bot cap log: %q", output.String())
	}
}

func TestRunBotsCountsConnectionFailure(t *testing.T) {
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	address := listener.Addr().String()
	if err := listener.Close(); err != nil {
		t.Fatal(err)
	}
	var output bytes.Buffer
	report := RunBots(context.Background(), RunConfig{
		Address: address, BotCount: 1, Ramp: time.Millisecond, Duration: 80 * time.Millisecond,
	}, log.New(&output, "", 0))
	if report.Started != 1 || report.Connected != 0 || report.Errors != 1 || !strings.Contains(output.String(), "connect "+address) {
		t.Fatalf("report=%+v log=%q", report, output.String())
	}
}

func TestRunBotsStopsBeforeCharacterListWhenServerCloses(t *testing.T) {
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer listener.Close()
	serverDone := make(chan error, 1)
	go func() {
		conn, err := listener.Accept()
		if err != nil {
			serverDone <- err
			return
		}
		defer conn.Close()
		var init [4]byte
		_, err = io.ReadFull(conn, init[:])
		if err == nil {
			_, _, err = wire.ReadPacket(conn)
		}
		serverDone <- err
	}()
	report := RunBots(context.Background(), RunConfig{
		Address: listener.Addr().String(), BotCount: 1, Ramp: time.Millisecond, Duration: 100 * time.Millisecond,
	}, nil)
	if report.Started != 1 || report.Connected != 1 || report.Entered != 0 || report.Errors != 0 {
		t.Fatalf("report=%+v", report)
	}
	if err := <-serverDone; err != nil {
		t.Fatal(err)
	}
}

func TestReadLoopTracksWorldPositionAndTargets(t *testing.T) {
	client, server := net.Pipe()
	b := &bot{
		conn: client, counters: &loadCounters{}, entered: make(chan struct{}),
		targets: make(map[uint16]targetPosition), config: RunConfig{SpawnX: 12, SpawnY: 34},
	}
	readDone := make(chan error, 1)
	go func() { readDone <- b.readLoop(context.Background()) }()
	if _, err := server.Write(loadtestServerPacket(wire.OpCharList, wire.SceneCharList, 12, 0, 0)); err != nil {
		t.Fatal(err)
	}
	selection, checksum, err := wire.ReadPacket(server)
	if err != nil || !checksum || wire.ParseHeader(selection).Type != wire.OpCharacterLogin {
		t.Fatalf("selection=%x checksum=%t err=%v", selection, checksum, err)
	}
	packets := [][]byte{
		loadtestServerPacket(wire.OpEnterWorld, 77, 16, 100, 110),
		loadtestServerPacket(wire.OpAction, 76, 16, 900, 901),
		loadtestServerPacket(wire.OpAction, 77, 16, 120, 130),
		loadtestServerPacket(wire.OpCreateMob, 999, 16, 1, 2),
		loadtestServerPacket(wire.OpCreateMob, 1000, 16, 200, 201),
		loadtestServerPacket(wire.OpCreateMob, 1001, 12, 0, 0),
		loadtestServerPacket(wire.OpRemoveMob, 1000, 12, 0, 0),
	}
	for _, packet := range packets {
		if _, err := server.Write(packet); err != nil {
			t.Fatal(err)
		}
	}
	_ = server.Close()
	if err := <-readDone; err != nil {
		t.Fatal(err)
	}
	if b.id.Load() != 77 || b.confirmedX.Load() != 120 || b.confirmedY.Load() != 130 ||
		b.predictedX.Load() != 120 || b.predictedY.Load() != 130 {
		t.Fatalf("position id=%d confirmed=(%d,%d) predicted=(%d,%d)", b.id.Load(), b.confirmedX.Load(), b.confirmedY.Load(), b.predictedX.Load(), b.predictedY.Load())
	}
	if _, exists := b.targets[999]; exists {
		t.Fatal("player ID entered the mob target map")
	}
	if _, exists := b.targets[1000]; exists {
		t.Fatal("removed mob remained a target")
	}
	if got := b.targets[1001]; got != (targetPosition{}) {
		t.Fatalf("short create mob position=%+v", got)
	}
	select {
	case <-b.entered:
	default:
		t.Fatal("EnterWorld did not release the bot")
	}
}

func TestReadLoopUsesSpawnFallbackAndRejectsBadPackets(t *testing.T) {
	t.Run("enter world fallback", func(t *testing.T) {
		client, server := net.Pipe()
		b := &bot{conn: client, counters: &loadCounters{}, entered: make(chan struct{}), config: RunConfig{SpawnX: 12, SpawnY: 34}}
		readDone := make(chan error, 1)
		go func() { readDone <- b.readLoop(context.Background()) }()
		if _, err := server.Write(loadtestServerPacket(wire.OpEnterWorld, 5, 12, 0, 0)); err != nil {
			t.Fatal(err)
		}
		_ = server.Close()
		if err := <-readDone; err != nil || b.confirmedX.Load() != 12 || b.confirmedY.Load() != 34 {
			t.Fatalf("read err=%v position=(%d,%d)", err, b.confirmedX.Load(), b.confirmedY.Load())
		}
	})
	t.Run("checksum", func(t *testing.T) {
		client, server := net.Pipe()
		b := &bot{conn: client, counters: &loadCounters{}}
		readDone := make(chan error, 1)
		go func() { readDone <- b.readLoop(context.Background()) }()
		packet := loadtestServerPacket(wire.OpEnterWorld, 5, 16, 1, 2)
		packet[3] ^= 1
		if _, err := server.Write(packet); err != nil {
			t.Fatal(err)
		}
		_ = server.Close()
		if err := <-readDone; err == nil || err.Error() != "invalid encrypted packet or checksum" {
			t.Fatalf("err=%v", err)
		}
	})
	t.Run("bad size", func(t *testing.T) {
		client, server := net.Pipe()
		b := &bot{conn: client, counters: &loadCounters{}}
		readDone := make(chan error, 1)
		go func() { readDone <- b.readLoop(context.Background()) }()
		if _, err := server.Write([]byte{0, 0}); err != nil {
			t.Fatal(err)
		}
		_ = server.Close()
		if err := <-readDone; !errors.Is(err, wire.ErrBadSize) {
			t.Fatalf("err=%v", err)
		}
	})
	t.Run("context deadline", func(t *testing.T) {
		ctx, cancel := context.WithCancel(context.Background())
		defer cancel()
		cancel()
		b := &bot{conn: loadtestErrorConn{err: errors.New("closed")}, counters: &loadCounters{}}
		if err := b.readLoop(ctx); err != nil {
			t.Fatalf("deadline shutdown: %v", err)
		}
	})
	t.Run("expired read deadline", func(t *testing.T) {
		b := &bot{conn: loadtestErrorConn{err: loadtestTimeoutError{}}, counters: &loadCounters{}}
		ctx := deadlineContext{deadline: time.Now().Add(-time.Millisecond)}
		if err := b.readLoop(ctx); err != nil {
			t.Fatalf("expired context timeout: %v", err)
		}
	})
	t.Run("character login write failure", func(t *testing.T) {
		packet := loadtestServerPacket(wire.OpCharList, wire.SceneCharList, 12, 0, 0)
		b := &bot{conn: packetWriteFailConn{reader: bytes.NewReader(packet)}, counters: &loadCounters{}}
		if err := b.readLoop(context.Background()); err == nil {
			t.Fatal("failed character login write was ignored")
		}
	})
}

type loadtestErrorConn struct{ err error }

func (c loadtestErrorConn) Read([]byte) (int, error)       { return 0, c.err }
func (loadtestErrorConn) Write(p []byte) (int, error)      { return len(p), nil }
func (loadtestErrorConn) Close() error                     { return nil }
func (loadtestErrorConn) LocalAddr() net.Addr              { return &net.TCPAddr{} }
func (loadtestErrorConn) RemoteAddr() net.Addr             { return &net.TCPAddr{} }
func (loadtestErrorConn) SetDeadline(time.Time) error      { return nil }
func (loadtestErrorConn) SetReadDeadline(time.Time) error  { return nil }
func (loadtestErrorConn) SetWriteDeadline(time.Time) error { return nil }

type loadtestTimeoutError struct{}

func (loadtestTimeoutError) Error() string   { return "i/o timeout" }
func (loadtestTimeoutError) Timeout() bool   { return true }
func (loadtestTimeoutError) Temporary() bool { return true }

type deadlineContext struct{ deadline time.Time }

func (c deadlineContext) Deadline() (time.Time, bool) { return c.deadline, true }
func (deadlineContext) Done() <-chan struct{}         { return nil }
func (deadlineContext) Err() error                    { return nil }
func (deadlineContext) Value(any) any                 { return nil }

type packetWriteFailConn struct{ reader *bytes.Reader }

func (c packetWriteFailConn) Read(p []byte) (int, error)     { return c.reader.Read(p) }
func (packetWriteFailConn) Write([]byte) (int, error)        { return 0, errors.New("write failed") }
func (packetWriteFailConn) Close() error                     { return nil }
func (packetWriteFailConn) LocalAddr() net.Addr              { return &net.TCPAddr{} }
func (packetWriteFailConn) RemoteAddr() net.Addr             { return &net.TCPAddr{} }
func (packetWriteFailConn) SetDeadline(time.Time) error      { return nil }
func (packetWriteFailConn) SetReadDeadline(time.Time) error  { return nil }
func (packetWriteFailConn) SetWriteDeadline(time.Time) error { return nil }

func TestSendAttackAndMovementEdgeCases(t *testing.T) {
	if err := (&bot{}).send(nil); err == nil {
		t.Fatal("send without connection succeeded")
	}
	if err := (&bot{}).sendAttack(); err != nil {
		t.Fatalf("attack without visible target: %v", err)
	}
	for _, tc := range []struct {
		skill int
		want  uint16
		flag  byte
	}{
		{skill: 3, want: wire.OpAttackOne, flag: 1},
		{skill: -1, want: wire.OpAttackOne, flag: 0},
	} {
		client, server := net.Pipe()
		b := &bot{
			conn: client, counters: &loadCounters{}, config: RunConfig{Skill: tc.skill},
			targets: map[uint16]targetPosition{1000: {x: 8, y: 9}},
		}
		b.id.Store(7)
		b.applyServerPosition(4, 5)
		readDone := make(chan struct {
			packet []byte
			check  bool
			err    error
		}, 1)
		go func() {
			packet, check, err := wire.ReadPacket(server)
			readDone <- struct {
				packet []byte
				check  bool
				err    error
			}{packet, check, err}
		}()
		if err := b.sendAttack(); err != nil {
			t.Fatal(err)
		}
		got := <-readDone
		if got.err != nil || !got.check || wire.ParseHeader(got.packet).Type != tc.want || got.packet[30] != tc.flag {
			t.Fatalf("skill=%d packet=%x check=%t err=%v", tc.skill, got.packet, got.check, got.err)
		}
		_ = client.Close()
		_ = server.Close()
	}

	botWithoutTarget := &bot{counters: &loadCounters{}}
	if err := botWithoutTarget.sendMove(); err != nil {
		t.Fatal(err)
	}
	standing := &bot{counters: &loadCounters{}, targets: map[uint16]targetPosition{1000: {x: 20, y: 30}}}
	standing.applyServerPosition(20, 30)
	if err := standing.sendMove(); err != nil {
		t.Fatal(err)
	}
	blocked := &bot{
		config: RunConfig{Terrain: model.TerrainMap{
			Height: make([]byte, model.TerrainCells), Attribute: bytes.Repeat([]byte{0x02}, model.AttributeCells),
		}},
		counters: &loadCounters{}, targets: map[uint16]targetPosition{1000: {x: 2, y: 2}},
	}
	blocked.applyServerPosition(1, 1)
	if err := blocked.sendMove(); err != nil {
		t.Fatal(err)
	}
	moveClient, moveServer := net.Pipe()
	_ = moveServer.Close()
	failedMove := &bot{
		conn: moveClient, counters: &loadCounters{},
		targets: map[uint16]targetPosition{1000: {x: 11, y: 10}},
	}
	failedMove.applyServerPosition(10, 10)
	if err := failedMove.sendMove(); err == nil {
		t.Fatal("failed move write was ignored")
	}
	_ = moveClient.Close()
	if chebyshevDistance(1, 1, 2, 4) != 3 || signLoadtest(-1) != -1 || signLoadtest(1) != 1 || signLoadtest(0) != 0 {
		t.Fatal("distance or direction helper regression")
	}
}

func TestSendKeyWrapAndWriteFailure(t *testing.T) {
	client, server := net.Pipe()
	b := &bot{conn: client, counters: &loadCounters{}, key: 255}
	packet := wire.Build(wire.OpAction, 1, 52)
	readDone := make(chan error, 1)
	go func() {
		got, checksum, err := wire.ReadPacket(server)
		if err == nil && (!checksum || wire.ParseHeader(got).Type != wire.OpAction) {
			err = errors.New("invalid sent action")
		}
		readDone <- err
	}()
	if err := b.send(packet); err != nil {
		t.Fatal(err)
	}
	if err := <-readDone; err != nil || b.key != 1 {
		t.Fatalf("read err=%v key=%d", err, b.key)
	}
	_ = client.Close()
	_ = server.Close()

	client, server = net.Pipe()
	b = &bot{conn: client, counters: &loadCounters{}}
	_ = server.Close()
	err := b.send(wire.Build(wire.OpAction, 1, 52))
	if err == nil || b.counters.bytesSent.Load() != 0 {
		t.Fatalf("write err=%v bytes=%d", err, b.counters.bytesSent.Load())
	}
	_ = client.Close()
}

func TestActivityLoopStopsOnCancellation(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	(&bot{counters: &loadCounters{}}).activityLoop(ctx)
}

func TestActivityLoopSendsMovementAndAttack(t *testing.T) {
	client, server := net.Pipe()
	defer client.Close()
	defer server.Close()
	counters := &loadCounters{}
	b := &bot{
		conn: client, counters: counters, config: RunConfig{MovePercent: 100, Skill: -1},
		targets: map[uint16]targetPosition{1000: {x: 101, y: 100}},
		rng:     mathrand.New(mathrand.NewSource(1)),
	}
	b.applyServerPosition(100, 100)
	readDone := make(chan error, 1)
	go func() {
		for range 2 {
			packet, checksum, err := wire.ReadPacket(server)
			if err != nil {
				readDone <- err
				return
			}
			if !checksum {
				readDone <- errors.New("bad outgoing checksum")
				return
			}
			if typ := wire.ParseHeader(packet).Type; typ != wire.OpAction && typ != wire.OpAttackOne {
				readDone <- errors.New("unexpected bot packet")
				return
			}
		}
		readDone <- nil
	}()
	ctx, cancel := context.WithTimeout(context.Background(), 1150*time.Millisecond)
	defer cancel()
	b.activityLoop(ctx)
	if err := <-readDone; err != nil {
		t.Fatal(err)
	}
	if counters.moves.Load() != 1 || counters.attacks.Load() != 1 {
		t.Fatalf("moves=%d attacks=%d", counters.moves.Load(), counters.attacks.Load())
	}
}

func loadtestServerPacket(typ, id uint16, size int, x, y uint16) []byte {
	packet := wire.Build(typ, id, size)
	if size >= 16 {
		binary.LittleEndian.PutUint16(packet[12:14], x)
		binary.LittleEndian.PutUint16(packet[14:16], y)
	}
	wire.FinishPacket(packet, 1)
	return packet
}
