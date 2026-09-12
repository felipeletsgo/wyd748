package protocol

import (
	"bytes"
	"context"
	"crypto/sha256"
	"errors"
	"io"
	"net"
	"testing"
	"time"
)

func TestEncodeDecodeRoundTrip(t *testing.T) {
	body := []byte("native-7.48-transport")
	plain := NewPacket(0x1234, 0x5678, 0x90ABCDEF, body)

	for _, key := range []byte{0, 1, 17, 127, 255} {
		frame, err := Encode(plain, key)
		if err != nil {
			t.Fatalf("Encode(key=%d): %v", key, err)
		}
		if len(frame) != HeaderSize+len(body) {
			t.Fatalf("encoded size = %d, want %d", len(frame), HeaderSize+len(body))
		}
		if frame[2] != key {
			t.Fatalf("encoded key = %d, want %d", frame[2], key)
		}
		packet, err := Decode(frame)
		if err != nil {
			t.Fatalf("Decode(key=%d): %v", key, err)
		}
		if packet.Header.Size != uint16(len(plain)) || packet.Header.Type != 0x1234 || packet.Header.ID != 0x5678 || packet.Header.Tick != 0x90ABCDEF {
			t.Fatalf("decoded header = %+v", packet.Header)
		}
		if !bytes.Equal(packet.Body, body) {
			t.Fatalf("decoded body = %q, want %q", packet.Body, body)
		}
	}
}

func TestKeyWordFingerprint(t *testing.T) {
	got := sha256.Sum256(keyWord[:])
	want := [32]byte{0xe4, 0x79, 0x96, 0xfe, 0x5e, 0x92, 0xde, 0x5d, 0x86, 0xd5, 0x03, 0xd5, 0x41, 0x56, 0x65, 0xf1, 0xf4, 0x64, 0xbf, 0x34, 0x43, 0x70, 0xc7, 0x09, 0xbe, 0x45, 0x06, 0x07, 0xdd, 0x97, 0xcf, 0x8f}
	if got != want {
		t.Fatalf("key table fingerprint = %x, want %x", got, want)
	}
}

func TestStreamDecoderHandlesFragmentationAndConcatenation(t *testing.T) {
	first, err := Encode(NewPacket(0x1001, 1, 10, []byte("first")), 3)
	if err != nil {
		t.Fatal(err)
	}
	second, err := Encode(NewPacket(0x1002, 2, 20, []byte("second")), 4)
	if err != nil {
		t.Fatal(err)
	}
	stream := append(append([]byte(nil), first...), second...)
	var decoder StreamDecoder
	packets, err := decoder.Feed(stream[:3])
	if err != nil {
		t.Fatal(err)
	}
	if len(packets) != 0 || decoder.PendingBytes() != 3 {
		t.Fatalf("after fragment: packets=%d pending=%d", len(packets), decoder.PendingBytes())
	}
	packets, err = decoder.Feed(stream[3:])
	if err != nil {
		t.Fatal(err)
	}
	if len(packets) != 2 {
		t.Fatalf("decoded %d packets, want 2", len(packets))
	}
	if packets[0].Header.Type != 0x1001 || packets[1].Header.Type != 0x1002 {
		t.Fatalf("decoded types = %#x, %#x", packets[0].Header.Type, packets[1].Header.Type)
	}
	if decoder.PendingBytes() != 0 {
		t.Fatalf("pending bytes = %d, want 0", decoder.PendingBytes())
	}
}

func TestDecodeRejectsInvalidChecksum(t *testing.T) {
	frame, err := Encode(NewPacket(0x2001, 0, 0, []byte("checksum")), 9)
	if err != nil {
		t.Fatal(err)
	}
	frame[3] ^= 0x01
	_, err = Decode(frame)
	if !errors.Is(err, ErrBadChecksum) {
		t.Fatalf("Decode error = %v, want ErrBadChecksum", err)
	}
}

func TestDecodeAndStreamRejectInvalidSizes(t *testing.T) {
	if _, err := Decode(make([]byte, HeaderSize-1)); !errors.Is(err, ErrBadSize) {
		t.Fatalf("short Decode error = %v, want ErrBadSize", err)
	}
	var decoder StreamDecoder
	badSize := []byte{0x01, 0x00}
	if _, err := decoder.Feed(badSize); !errors.Is(err, ErrBadSize) {
		t.Fatalf("short stream size error = %v, want ErrBadSize", err)
	}
}

func TestClientSessionConnectSendsHandshakeOnce(t *testing.T) {
	client, peer := net.Pipe()
	defer peer.Close()
	handshake := make(chan []byte, 1)
	go func() {
		buf := make([]byte, 4)
		_, err := io.ReadFull(peer, buf)
		if err != nil {
			handshake <- nil
			return
		}
		handshake <- buf
	}()
	session := NewSession("pipe", SessionOptions{
		ConnectTimeout: time.Second,
		DialContext:    func(context.Context, string) (net.Conn, error) { return client, nil },
	})
	if err := session.Connect(); err != nil {
		t.Fatal(err)
	}
	if got := <-handshake; !bytes.Equal(got, []byte{0x11, 0xF3, 0x11, 0x1F}) {
		t.Fatalf("handshake = %x", got)
	}
	if err := session.Connect(); err != nil {
		t.Fatalf("second Connect: %v", err)
	}
	if err := session.Close(); err != nil {
		t.Fatal(err)
	}
	if err := session.Close(); err != nil {
		t.Fatalf("second Close: %v", err)
	}
}

func TestClientSessionSetAddressOnlyBeforeConnect(t *testing.T) {
	session := NewSession("127.0.0.1:1", SessionOptions{})
	if err := session.SetAddress("127.0.0.1:8281"); err != nil {
		t.Fatal(err)
	}
	if err := session.SetAddress(""); err == nil {
		t.Fatal("empty endpoint accepted")
	}
	// A connected session cannot be redirected by a scene. Use a pipe dialer
	// so the test never depends on a live server.
	client, peer := net.Pipe()
	defer peer.Close()
	session = NewSession("pipe", SessionOptions{DialContext: func(context.Context, string) (net.Conn, error) { return client, nil }})
	go func() { _, _ = io.Copy(io.Discard, peer) }()
	if err := session.Connect(); err != nil {
		t.Fatal(err)
	}
	if err := session.SetAddress("127.0.0.1:8281"); err == nil {
		t.Fatal("connected session accepted endpoint change")
	}
	_ = session.Close()
}

func TestClientSessionSendHandlesPartialWrites(t *testing.T) {
	conn := &partialConn{maxWrite: 3}
	session, err := NewConnectedSession(conn, SessionOptions{})
	if err != nil {
		t.Fatal(err)
	}
	plain := NewPacket(0x3001, 4, 5, []byte("partial write"))
	want, err := Encode(plain, 12)
	if err != nil {
		t.Fatal(err)
	}
	if err := session.Send(plain, 12); err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(conn.written.Bytes(), want) {
		t.Fatalf("written frame = %x, want %x", conn.written.Bytes(), want)
	}
}

func TestClientSessionReceiveDecodesFrame(t *testing.T) {
	client, peer := net.Pipe()
	defer peer.Close()
	session, err := NewConnectedSession(client, SessionOptions{IOTimeout: time.Second})
	if err != nil {
		t.Fatal(err)
	}
	want, err := Encode(NewPacket(0x4001, 7, 8, []byte("response")), 21)
	if err != nil {
		t.Fatal(err)
	}
	go func() {
		_, _ = peer.Write(want)
		_ = peer.Close()
	}()
	packet, err := session.Receive()
	if err != nil {
		t.Fatal(err)
	}
	if packet.Header.Type != 0x4001 || !bytes.Equal(packet.Body, []byte("response")) {
		t.Fatalf("packet = %+v body=%q", packet.Header, packet.Body)
	}
	_ = session.Close()
}

func TestClientSessionReceiveLoopPreservesPacketOrderAndStartsOnce(t *testing.T) {
	client, peer := net.Pipe()
	defer peer.Close()
	session, err := NewConnectedSession(client, SessionOptions{IOTimeout: time.Second, EventBuffer: 8})
	if err != nil {
		t.Fatal(err)
	}
	defer session.Close()

	if err := session.StartReceiving(); err != nil {
		t.Fatal(err)
	}
	session.mu.Lock()
	firstDone := session.receiveDone
	session.mu.Unlock()
	if err := session.StartReceiving(); err != nil {
		t.Fatalf("second StartReceiving: %v", err)
	}
	session.mu.Lock()
	secondDone := session.receiveDone
	session.mu.Unlock()
	if firstDone == nil || firstDone != secondDone {
		t.Fatal("second StartReceiving replaced the active receive loop")
	}
	if _, err := session.Receive(); !errors.Is(err, ErrReceiveLoopRunning) {
		t.Fatalf("concurrent Receive error = %v, want ErrReceiveLoopRunning", err)
	}

	frames := make([][]byte, 3)
	for i, opcode := range []uint16{0x4101, 0x4102, 0x4103} {
		frames[i], err = Encode(NewPacket(opcode, uint16(i+1), uint32(i+10), []byte{byte(i)}), byte(i+20))
		if err != nil {
			t.Fatal(err)
		}
	}
	writeDone := make(chan error, 1)
	go func() {
		for _, frame := range frames {
			if _, err := peer.Write(frame); err != nil {
				writeDone <- err
				return
			}
		}
		writeDone <- nil
	}()

	events := waitForSessionEvents(t, session, len(frames))
	for i, event := range events {
		if event.Kind != SessionPacket || event.Packet.Header.Type != 0x4101+uint16(i) ||
			!bytes.Equal(event.Packet.Body, []byte{byte(i)}) {
			t.Fatalf("event[%d] = %+v body=% X", i, event, event.Packet.Body)
		}
	}
	if err := <-writeDone; err != nil {
		t.Fatalf("write frames: %v", err)
	}
}

func TestClientSessionDrainEventsHonorsLimitAndNeverBlocks(t *testing.T) {
	session := &ClientSession{events: make(chan SessionEvent, 3)}
	for _, opcode := range []uint16{1, 2, 3} {
		session.events <- SessionEvent{Kind: SessionPacket, Packet: Packet{Header: Header{Type: opcode}}}
	}

	first := session.DrainEvents(2)
	if len(first) != 2 || first[0].Packet.Header.Type != 1 || first[1].Packet.Header.Type != 2 {
		t.Fatalf("first drain = %+v", first)
	}
	second := session.DrainEvents(2)
	if len(second) != 1 || second[0].Packet.Header.Type != 3 {
		t.Fatalf("second drain = %+v", second)
	}
	if got := session.DrainEvents(2); len(got) != 0 {
		t.Fatalf("empty drain returned %+v", got)
	}
	if got := session.DrainEvents(0); len(got) != 0 {
		t.Fatalf("zero-limit drain returned %+v", got)
	}
}

func TestClientSessionReportsRemoteDisconnectAfterQueuedPackets(t *testing.T) {
	client, peer := net.Pipe()
	session, err := NewConnectedSession(client, SessionOptions{IOTimeout: time.Second, EventBuffer: 4})
	if err != nil {
		t.Fatal(err)
	}
	defer session.Close()
	if err := session.StartReceiving(); err != nil {
		t.Fatal(err)
	}
	frame, err := Encode(NewPacket(0x4201, 9, 10, []byte("last")), 31)
	if err != nil {
		t.Fatal(err)
	}
	go func() {
		_, _ = peer.Write(frame)
		_ = peer.Close()
	}()

	events := waitForSessionEvents(t, session, 2)
	if events[0].Kind != SessionPacket || events[0].Packet.Header.Type != 0x4201 {
		t.Fatalf("first event = %+v, want packet", events[0])
	}
	if events[1].Kind != SessionDisconnected || events[1].Err == nil {
		t.Fatalf("second event = %+v, want disconnect with cause", events[1])
	}
}

func TestClientSessionReconnectsAfterDrainedEOF(t *testing.T) {
	handshakes := make(chan []byte, 3)
	session := NewSession("test", SessionOptions{IOTimeout: time.Second, DialContext: func(context.Context, string) (net.Conn, error) {
		client, peer := net.Pipe()
		go func() {
			defer peer.Close()
			var handshake [4]byte
			_, _ = io.ReadFull(peer, handshake[:])
			handshakes <- handshake[:]
		}()
		return client, nil
	}})
	defer session.Close()
	for attempt := 0; attempt < 3; attempt++ {
		if err := session.SetAddress("test"); err != nil {
			t.Fatal(err)
		}
		if err := session.Connect(); err != nil {
			t.Fatal(err)
		}
		if err := session.StartReceiving(); err != nil {
			t.Fatal(err)
		}
		events := waitForSessionEvents(t, session, 1)
		if events[0].Kind != SessionDisconnected {
			t.Fatalf("event=%+v", events[0])
		}
		select {
		case handshake := <-handshakes:
			if !bytes.Equal(handshake, []byte{0x11, 0xf3, 0x11, 0x1f}) {
				t.Fatalf("handshake=%x", handshake)
			}
		case <-time.After(time.Second):
			t.Fatal("missing reconnect handshake")
		}
	}
}

func TestClientSessionCloseUnblocksReceiveLoopWithoutPublishingDisconnect(t *testing.T) {
	client, peer := net.Pipe()
	defer peer.Close()
	session, err := NewConnectedSession(client, SessionOptions{EventBuffer: 1})
	if err != nil {
		t.Fatal(err)
	}
	if err := session.StartReceiving(); err != nil {
		t.Fatal(err)
	}
	closed := make(chan error, 1)
	go func() { closed <- session.Close() }()
	select {
	case err := <-closed:
		if err != nil {
			t.Fatalf("Close: %v", err)
		}
	case <-time.After(time.Second):
		t.Fatal("Close did not unblock the receive loop")
	}
	if got := session.DrainEvents(1); len(got) != 0 {
		t.Fatalf("explicit Close published events: %+v", got)
	}
	if err := session.Close(); err != nil {
		t.Fatalf("second Close: %v", err)
	}
}

func TestClientSessionRejectsInvalidEventBuffers(t *testing.T) {
	for _, size := range []int{-1, MaxSessionEventBuffer + 1} {
		client, peer := net.Pipe()
		session, err := NewConnectedSession(client, SessionOptions{EventBuffer: size})
		if err != nil {
			t.Fatal(err)
		}
		if err := session.StartReceiving(); !errors.Is(err, ErrInvalidEventBuffer) {
			t.Fatalf("EventBuffer=%d error=%v, want ErrInvalidEventBuffer", size, err)
		}
		_ = session.Close()
		_ = peer.Close()
	}
}

func waitForSessionEvents(t *testing.T, session *ClientSession, count int) []SessionEvent {
	t.Helper()
	deadline := time.Now().Add(time.Second)
	events := make([]SessionEvent, 0, count)
	for len(events) < count && time.Now().Before(deadline) {
		events = append(events, session.DrainEvents(count-len(events))...)
		if len(events) < count {
			time.Sleep(time.Millisecond)
		}
	}
	if len(events) != count {
		t.Fatalf("received %d session events, want %d", len(events), count)
	}
	return events
}

type partialConn struct {
	written  bytes.Buffer
	maxWrite int
}

func (c *partialConn) Read([]byte) (int, error) { return 0, io.EOF }

func (c *partialConn) Write(p []byte) (int, error) {
	n := len(p)
	if c.maxWrite > 0 && n > c.maxWrite {
		n = c.maxWrite
	}
	_, _ = c.written.Write(p[:n])
	return n, nil
}

func (c *partialConn) Close() error                     { return nil }
func (c *partialConn) LocalAddr() net.Addr              { return testAddr("local") }
func (c *partialConn) RemoteAddr() net.Addr             { return testAddr("remote") }
func (c *partialConn) SetDeadline(time.Time) error      { return nil }
func (c *partialConn) SetReadDeadline(time.Time) error  { return nil }
func (c *partialConn) SetWriteDeadline(time.Time) error { return nil }

type testAddr string

func (a testAddr) Network() string { return "test" }
func (a testAddr) String() string  { return string(a) }
