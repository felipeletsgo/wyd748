package net

import (
	stdnet "net"
	"sync"
	"testing"
	"time"

	"wydgo/internal/wire"
)

type observedReadDeadlineConn struct {
	stdnet.Conn
	mu       sync.Mutex
	deadline time.Time
}

func (c *observedReadDeadlineConn) SetReadDeadline(deadline time.Time) error {
	c.mu.Lock()
	defer c.mu.Unlock()
	c.deadline = deadline
	return c.Conn.SetReadDeadline(deadline)
}

func (c *observedReadDeadlineConn) readDeadline() time.Time {
	c.mu.Lock()
	defer c.mu.Unlock()
	return c.deadline
}

func TestAuthenticatedSessionHasNoIdleDeadlineAndResumesPackets(t *testing.T) {
	session, client := pipeSession()
	defer client.Close()
	session.idleTimeout = 40 * time.Millisecond
	observed := &observedReadDeadlineConn{Conn: session.conn}
	session.conn = observed
	received := make(chan []byte, 4)
	go session.Serve(func(_ *Session, packet []byte) { received <- packet })
	writeInitCode(t, client)
	if _, err := client.Write(encryptedTestPacket(wire.OpPing)); err != nil {
		t.Fatal(err)
	}
	select {
	case <-received:
	case <-time.After(time.Second):
		t.Fatal("first packet was not dispatched")
	}
	// Exercise asynchronous World admission while the reader is parked.
	session.MarkAuthenticated()
	select {
	case packet := <-received:
		t.Fatalf("authenticated idle session disconnected: %v", packet)
	case <-time.After(100 * time.Millisecond):
	}
	if !observed.readDeadline().IsZero() {
		t.Fatal("authenticated session retained an idle deadline")
	}
	if _, err := client.Write(encryptedTestPacket(wire.OpPing)); err != nil {
		t.Fatalf("packet after inactivity: %v", err)
	}
	select {
	case packet := <-received:
		if packet == nil {
			t.Fatal("session disconnected instead of resuming packets")
		}
	case <-time.After(time.Second):
		t.Fatal("packet after inactivity was not dispatched")
	}
}

func TestUnauthenticatedSessionStillExpires(t *testing.T) {
	session, client := pipeSession()
	defer client.Close()
	session.idleTimeout = 25 * time.Millisecond
	finished := make(chan []byte, 1)
	go session.Serve(func(_ *Session, packet []byte) { finished <- packet })
	writeInitCode(t, client)
	select {
	case packet := <-finished:
		if packet != nil || !session.IsClosed() {
			t.Fatal("unauthenticated timeout did not close the session")
		}
	case <-time.After(time.Second):
		t.Fatal("unauthenticated socket retained its reservation")
	}
}

func TestAuthenticationDoesNotClearPartialFrameDeadline(t *testing.T) {
	for _, fragmentSize := range []int{1, 2, wire.HeaderSize - 1} {
		t.Run(string(rune('A'+fragmentSize)), func(t *testing.T) {
			session, client := pipeSession()
			defer client.Close()
			session.frameReadTimeout = 25 * time.Millisecond
			observed := &observedReadDeadlineConn{Conn: session.conn}
			session.conn = observed
			finished := make(chan []byte, 1)
			go session.Serve(func(_ *Session, packet []byte) { finished <- packet })
			writeInitCode(t, client)
			if _, err := client.Write(encryptedTestPacket(wire.OpPing)[:fragmentSize]); err != nil {
				t.Fatal(err)
			}
			// Make the transition deterministic: the first-byte read may still be
			// completing when net.Pipe.Write returns.
			for deadline := time.Now().Add(time.Second); ; {
				session.readDeadlineMu.Lock()
				reading := session.readingFrame
				session.readDeadlineMu.Unlock()
				if reading {
					break
				}
				if time.Now().After(deadline) {
					t.Fatal("reader did not begin the frame")
				}
				time.Sleep(time.Millisecond)
			}
			session.MarkAuthenticated()
			if observed.readDeadline().IsZero() {
				t.Fatal("authentication cleared the incomplete-frame deadline")
			}
			select {
			case packet := <-finished:
				if packet != nil {
					t.Fatal("partial frame reached gameplay")
				}
			case <-time.After(time.Second):
				t.Fatal("incomplete authenticated frame did not expire")
			}
		})
	}
}

func TestPreAuthDeadlineIsNotExtendedByPackets(t *testing.T) {
	session, client := pipeSession()
	defer client.Close()
	session.idleTimeout = time.Second
	observed := &observedReadDeadlineConn{Conn: session.conn}
	session.conn = observed
	received := make(chan []byte, 2)
	go session.Serve(func(_ *Session, packet []byte) { received <- packet })
	writeInitCode(t, client)
	for i := 0; i < 2; i++ {
		if _, err := client.Write(encryptedTestPacket(wire.OpPing)); err != nil {
			t.Fatal(err)
		}
		select {
		case <-received:
		case <-time.After(time.Second):
			t.Fatal("packet was not dispatched")
		}
		session.readDeadlineMu.Lock()
		absolute := session.preAuthDeadline
		session.readDeadlineMu.Unlock()
		if !observed.readDeadline().Equal(absolute) {
			t.Fatal("pre-auth packet extended the absolute admission deadline")
		}
	}
}
