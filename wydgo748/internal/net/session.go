// Package net handles socket I/O: one TCP connection per Session, with an
// InitCode handshake, a read loop and a non-blocking outbound queue.
// It delegates raw packets through a callback without knowing gameplay rules.
package net

import (
	"bufio"
	"encoding/binary"
	"errors"
	"io"
	"log"
	stdnet "net"
	"sync"
	"sync/atomic"
	"time"

	"wydgo/internal/wire"
)

const (
	// Operational limits, not gameplay rules. The margin accommodates
	// movement and loading bursts from a normal 7.48 client.
	maxInboundPacketsPerSecond = 256
	maxInboundBytesPerSecond   = 512 * 1024
	defaultHandshakeTimeout    = 5 * time.Second
	defaultSessionIdleTimeout  = 10 * time.Minute
	defaultFrameReadTimeout    = 10 * time.Second
	defaultFrameWriteTimeout   = 10 * time.Second
)

var (
	serverStart = time.Now()
	sendKey     atomic.Uint32
)

// tick emulates server GetTickCount with a nonzero base. The client uses
// header ticks for animation and death decisions.
func tick() uint32 {
	return uint32(time.Since(serverStart)/time.Millisecond) + 60000
}

// Session represents one TCP client connection.
type Session struct {
	ID                      int64
	conn                    stdnet.Conn
	remoteIP                string
	out                     chan []byte
	done                    chan struct{}
	doneOnce                sync.Once
	readDeadlineMu          sync.Mutex
	authenticated           bool
	readingFrame            bool
	preAuthDeadline         time.Time
	handshakeTimeout        time.Duration
	idleTimeout             time.Duration
	frameReadTimeout        time.Duration
	frameWriteTimeout       time.Duration
	maxInboundPacketsPerSec int
	maxInboundBytesPerSec   int
}

// MarkAuthenticated is called only after World admits a verified account.
// A parked read must be released from the pre-auth deadline immediately, but
// authentication must never clear the deadline of an incomplete frame.
func (s *Session) MarkAuthenticated() {
	s.readDeadlineMu.Lock()
	defer s.readDeadlineMu.Unlock()
	if s.IsClosed() {
		return
	}
	s.authenticated = true
	if s.conn != nil && !s.readingFrame {
		if err := s.conn.SetReadDeadline(time.Time{}); err != nil {
			log.Printf("[#%d] clear authenticated idle deadline: %v", s.ID, err)
			s.Close()
		}
	}
}

// IsAuthenticated exposes admission state, not a client-supplied claim.
func (s *Session) IsAuthenticated() bool {
	s.readDeadlineMu.Lock()
	defer s.readDeadlineMu.Unlock()
	return s.authenticated
}

func (s *Session) setPacketReadDeadline(waitingForPacket bool) error {
	s.readDeadlineMu.Lock()
	defer s.readDeadlineMu.Unlock()
	s.readingFrame = !waitingForPacket
	deadline := s.preAuthDeadline
	if s.authenticated {
		deadline = time.Time{}
	}
	if !waitingForPacket {
		timeout := s.frameReadTimeout
		if timeout <= 0 {
			timeout = defaultFrameReadTimeout
		}
		frameDeadline := time.Now().Add(timeout)
		if deadline.IsZero() || frameDeadline.Before(deadline) {
			deadline = frameDeadline
		}
	}
	return s.conn.SetReadDeadline(deadline)
}

// IsClosed reports whether the I/O lifetime has ended. Test sessions without
// a lifetime channel remain active until the test replaces them.
func (s *Session) IsClosed() bool {
	if s == nil || s.done == nil {
		return false
	}
	select {
	case <-s.done:
		return true
	default:
		return false
	}
}

func (s *Session) closeDone() {
	if s == nil || s.done == nil {
		return
	}
	s.doneOnce.Do(func() { close(s.done) })
}

// Send finalizes Tick@8, Size@0 and encryption, then queues without blocking.
// Queue overflow disconnects a slow client without blocking the world loop.
// It is safe to call from any goroutine; gameplay normally owns the caller.
func (s *Session) Send(pkt []byte) {
	if s == nil || len(pkt) < wire.HeaderSize || len(pkt) > wire.MaxPacketSize || s.out == nil {
		// A builder regression must not panic in the World goroutine.
		// Fail closed; handler diagnostics identify the originating packet.
		if s != nil {
			log.Printf("[#%d] invalid outbound packet: %d bytes", s.ID, len(pkt))
			s.Close()
		}
		return
	}
	if s.IsClosed() {
		return
	}
	binary.LittleEndian.PutUint32(pkt[8:12], tick())
	wire.FinishPacket(pkt, byte(sendKey.Add(1)))
	select {
	case s.out <- pkt:
	default:
		s.Close()
	}
}

// NewTestSession queues packets for other packages without a real connection.
// Tests choose bufSize to accommodate their expected outbound packets.
func NewTestSession(id int64, bufSize int) *Session {
	return &Session{ID: id, out: make(chan []byte, bufSize), done: make(chan struct{})}
}

// NewTestSessionWithRemoteIP supplies the origin recorded by the listener,
// allowing IP-limit tests without trusting packet fields.
func NewTestSessionWithRemoteIP(id int64, bufSize int, remoteIP string) *Session {
	return &Session{
		ID: id, remoteIP: remoteIP, out: make(chan []byte, bufSize), done: make(chan struct{}),
	}
}

// QueuedPacketsForTest counts encrypted packets still in the test queue.
// Gameplay tests use it to detect extra visual updates such as CreateMob.
func (s *Session) QueuedPacketsForTest() int {
	if s == nil || s.out == nil {
		return 0
	}
	return len(s.out)
}

// DequeuePacketForTest removes and returns the next encrypted outbound packet.
// It is intended for contract tests that must validate the packet payload, not
// only the number of visual updates emitted by a handler.
func (s *Session) DequeuePacketForTest() ([]byte, bool) {
	if s == nil || s.out == nil {
		return nil, false
	}
	select {
	case pkt := <-s.out:
		return pkt, true
	default:
		return nil, false
	}
}

// RemoteAddr returns the remote address for diagnostics.
func (s *Session) RemoteAddr() string {
	if s == nil || s.conn == nil || s.conn.RemoteAddr() == nil {
		return ""
	}
	return s.conn.RemoteAddr().String()
}

// RemoteIP returns the address without its port. Connection and authentication
// limits must not use a changing source port as part of the identity.
func (s *Session) RemoteIP() string {
	if s == nil {
		return ""
	}
	if s.remoteIP != "" {
		return s.remoteIP
	}
	addr := s.RemoteAddr()
	host, _, err := stdnet.SplitHostPort(addr)
	if err != nil {
		return addr
	}
	return host
}

// Close ends the connection and its lifetime. Callers that want to drain an
// authentication rejection before closing are responsible for that delay.
func (s *Session) Close() {
	if s.conn != nil {
		_ = s.conn.Close()
	}
	s.closeDone()
}

// writeLoop drains outbound packets. Every exit closes the connection so a
// failed or stalled writer also releases the reader and gameplay reservation.
func (s *Session) writeLoop() {
	defer s.Close()
	timeout := s.frameWriteTimeout
	if timeout <= 0 {
		timeout = defaultFrameWriteTimeout
	}
	for {
		select {
		case pkt, ok := <-s.out:
			if !ok || s.IsClosed() {
				return
			}
			if err := s.conn.SetWriteDeadline(time.Now().Add(timeout)); err != nil {
				return
			}
			if n, err := s.conn.Write(pkt); err != nil || n != len(pkt) {
				return
			}
		case <-s.done:
			return
		}
	}
}

// Serve performs InitCode and dispatches decrypted packets to handler.
// handler(s, nil) signals disconnection when the read lifetime ends.
func (s *Session) Serve(handler func(*Session, []byte)) {
	defer s.conn.Close()
	defer handler(s, nil)
	// End the lifetime before notifying World so a retry can recognize a dead
	// socket reservation even when the world command queue is full.
	defer s.closeDone()

	r := bufio.NewReader(s.conn)
	handshakeTimeout := s.handshakeTimeout
	if handshakeTimeout <= 0 {
		handshakeTimeout = defaultHandshakeTimeout
	}
	if err := s.conn.SetReadDeadline(time.Now().Add(handshakeTimeout)); err != nil {
		log.Printf("[#%d] set InitCode deadline: %v", s.ID, err)
		return
	}

	// The handshake has four InitCode bytes before any packet.
	var ic [4]byte
	if _, err := io.ReadFull(r, ic[:]); err != nil {
		log.Printf("[#%d] missing InitCode: %v", s.ID, err)
		return
	}
	if got := binary.LittleEndian.Uint32(ic[:]); got != uint32(wire.InitCode) {
		log.Printf("[#%d] invalid InitCode: 0x%X", s.ID, got)
		return
	}
	log.Printf("[#%d] connected: %s (InitCode OK)", s.ID, s.RemoteAddr())

	go s.writeLoop()

	idleTimeout := s.idleTimeout
	if idleTimeout <= 0 {
		idleTimeout = defaultSessionIdleTimeout
	}
	s.readDeadlineMu.Lock()
	// Bound the entire unauthenticated lifetime, not each keepalive packet.
	s.preAuthDeadline = time.Now().Add(idleTimeout)
	s.readDeadlineMu.Unlock()
	packetLimit := s.maxInboundPacketsPerSec
	if packetLimit <= 0 {
		packetLimit = maxInboundPacketsPerSecond
	}
	byteLimit := s.maxInboundBytesPerSec
	if byteLimit <= 0 {
		byteLimit = maxInboundBytesPerSecond
	}
	windowStarted := time.Now()
	packetCount, byteCount := 0, 0
	for {
		// Authenticated players may remain idle indefinitely. TCP keepalive and
		// socket errors detect dead peers independently of gameplay activity.
		if err := s.setPacketReadDeadline(true); err != nil {
			log.Printf("[#%d] set read deadline: %v", s.ID, err)
			return
		}
		var sizeBytes [2]byte
		if _, err := io.ReadFull(r, sizeBytes[:1]); err != nil {
			if !errors.Is(err, io.EOF) {
				log.Printf("[#%d] read: %v", s.ID, err)
			}
			log.Printf("[#%d] disconnected", s.ID)
			return
		}
		// Start the frame timeout at its first byte, including a stalled Size
		// header. Removing AFK eviction must not enable single-byte Slowloris.
		if err := s.setPacketReadDeadline(false); err != nil {
			log.Printf("[#%d] set frame deadline: %v", s.ID, err)
		}
		if _, err := io.ReadFull(r, sizeBytes[1:]); err != nil {
			log.Printf("[#%d] incomplete frame header: %v", s.ID, err)
			return
		}
		size := int(binary.LittleEndian.Uint16(sizeBytes[:]))
		if size < wire.HeaderSize || size > wire.MaxPacketSize {
			log.Printf("[#%d] read: %v", s.ID, wire.ErrBadSize)
			return
		}
		buf := make([]byte, size)
		copy(buf[:2], sizeBytes[:])
		_, err := io.ReadFull(r, buf[2:])
		if err != nil {
			if !errors.Is(err, io.EOF) {
				log.Printf("[#%d] read: %v", s.ID, err)
			}
			log.Printf("[#%d] disconnected", s.ID)
			return
		}
		checksumOK := wire.Decrypt(buf)
		if !checksumOK {
			log.Printf("[#%d] invalid checksum; connection closed", s.ID)
			return
		}
		now := time.Now()
		if now.Sub(windowStarted) >= time.Second {
			windowStarted, packetCount, byteCount = now, 0, 0
		}
		packetCount++
		byteCount += len(buf)
		if packetCount > packetLimit || byteCount > byteLimit {
			log.Printf("[#%d] inbound flood: packets=%d bytes=%d/s", s.ID, packetCount, byteCount)
			return
		}
		handler(s, buf)
	}
}
