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
