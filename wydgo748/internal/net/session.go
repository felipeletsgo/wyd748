// Package net handles socket I/O: one TCP connection per Session, with an
// InitCode handshake, a read loop and a non-blocking outbound queue.
// It delegates raw packets through a callback without knowing gameplay rules.
package net

import (
	"bufio"
	"encoding/binary"
	"errors"
	"fmt"
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
	// defaultMaxQueuedBytes bounds retained output together with the packet
	// capacity, so a few large packets cannot hold unbounded memory.
	defaultMaxQueuedBytes = 512 * 1024
	// writeBatchBytes bounds one coalesced write. The writer joins only packets
	// already queued; it never waits to fill a batch.
	writeBatchBytes = 64 * 1024
)

var (
	serverStart        = time.Now()
	sendKey            atomic.Uint32
	errInvalidChecksum = errors.New("invalid checksum")
)

// tickAt emulates server GetTickCount with a nonzero base. The client uses
// header ticks for animation and death decisions.
func tickAt(elapsed time.Duration) uint32 {
	return uint32(elapsed/time.Millisecond) + 60000
}

// outboundPacket keeps its enqueue time, measured from serverStart, so the
// writer can observe queue wait without a second clock source.
type outboundPacket struct {
	data     []byte
	queuedAt time.Duration
}

// Session represents one TCP client connection.
type Session struct {
	ID                      int64
	conn                    stdnet.Conn
	remoteIP                string
	out                     chan outboundPacket
	done                    chan struct{}
	doneOnce                sync.Once
	closeMu                 sync.Mutex
	closeCause              CloseCause
	closeDetail             string
	queueHighWater          atomic.Int64
	maxQueueWait            atomic.Int64
	sentPackets             atomic.Int64
	queuedBytes             atomic.Int64
	maxQueuedBytes          int64
	admissionRejected       atomic.Bool
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
			s.CloseWithCause(CloseCauseReadError, fmt.Errorf("clear authenticated idle deadline: %w", err))
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

// Done is closed when the session lifetime ends. Workers waiting on behalf
// of the session use it to stop promptly.
func (s *Session) Done() <-chan struct{} {
	if s == nil {
		return nil
	}
	return s.done
}

// AdmissionRejected is terminal even while the final message is draining.
func (s *Session) AdmissionRejected() bool {
	return s.admissionRejected.Load()
}

// RejectAdmission queues one final message and closes after a bounded grace
// period. The latch prevents pipelined retries from entering authentication.
func (s *Session) RejectAdmission(message []byte) {
	if !s.admissionRejected.CompareAndSwap(false, true) {
		return
	}
	// The grace period must not let a later peer close hide the decision.
	s.recordClose(CloseCauseAdmission, nil)
	s.Send(message)
	time.AfterFunc(300*time.Millisecond, s.Close)
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
			s.CloseWithCause(CloseCauseInvalidOutbound, fmt.Errorf("invalid outbound packet: %d bytes", len(pkt)))
		}
		return
	}
	if s.IsClosed() {
		return
	}
	size, limit := int64(len(pkt)), s.maxQueuedBytes
	if limit <= 0 {
		limit = defaultMaxQueuedBytes
	}
	if s.queuedBytes.Add(size) > limit {
		s.queuedBytes.Add(-size)
		s.CloseWithCause(CloseCauseOutputOverflow, fmt.Errorf("output queue holds more than %d bytes", limit))
		return
	}
	elapsed := time.Since(serverStart)
	binary.LittleEndian.PutUint32(pkt[8:12], tickAt(elapsed))
	wire.FinishPacket(pkt, byte(sendKey.Add(1)))
	select {
	case s.out <- outboundPacket{data: pkt, queuedAt: elapsed}:
		depth := len(s.out)
		raiseMax(&s.queueHighWater, int64(depth))
		raiseMax(&outputHighWater, int64(depth))
		metricOutputDepth.Add(outputDepthLabel(depth, cap(s.out)), 1)
	default:
		s.queuedBytes.Add(-size)
		s.CloseWithCause(CloseCauseOutputOverflow, fmt.Errorf("output queue full at %d packets", cap(s.out)))
	}
}

// QueueFree reports how many more packets fit in the output queue now. Scene
// publication uses it to defer optional creations instead of overflowing.
func (s *Session) QueueFree() int {
	if s == nil || s.out == nil {
		return 0
	}
	return cap(s.out) - len(s.out)
}

// QueueCapacity is the configured packet capacity of the output queue.
func (s *Session) QueueCapacity() int {
	if s == nil || s.out == nil {
		return 0
	}
	return cap(s.out)
}

// NewTestSession queues packets for other packages without a real connection.
// Tests choose bufSize to accommodate their expected outbound packets.
func NewTestSession(id int64, bufSize int) *Session {
	return &Session{ID: id, out: make(chan outboundPacket, bufSize), done: make(chan struct{}),
		maxQueuedBytes: testQueuedBytes(bufSize)}
}

// Test sessions bound memory by packet count only, as their callers size it.
func testQueuedBytes(bufSize int) int64 {
	return int64(max(bufSize, 1)) * wire.MaxPacketSize
}

// NewTestSessionWithRemoteIP supplies the origin recorded by the listener,
// allowing IP-limit tests without trusting packet fields.
func NewTestSessionWithRemoteIP(id int64, bufSize int, remoteIP string) *Session {
	return &Session{
		ID: id, remoteIP: remoteIP, out: make(chan outboundPacket, bufSize), done: make(chan struct{}),
		maxQueuedBytes: testQueuedBytes(bufSize),
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
		s.queuedBytes.Add(-int64(len(pkt.data)))
		return pkt.data, true
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

// Close ends the connection and its lifetime as an explicit server decision.
// Callers that want to drain an authentication rejection before closing are
// responsible for that delay.
func (s *Session) Close() {
	s.CloseWithCause(CloseCauseServer, nil)
}

// CloseWithCause records cause only when no earlier cause exists, then closes.
func (s *Session) CloseWithCause(cause CloseCause, detail error) {
	if s == nil {
		return
	}
	s.recordClose(cause, detail)
	if s.conn != nil {
		_ = s.conn.Close()
	}
	s.closeDone()
}

func (s *Session) recordClose(cause CloseCause, detail error) {
	if s == nil || cause == CloseCauseNone {
		return
	}
	if cause >= closeCauseCount {
		cause = CloseCauseServer
	}
	s.closeMu.Lock()
	defer s.closeMu.Unlock()
	if s.closeCause == CloseCauseNone {
		s.closeCause, s.closeDetail = cause, boundedCloseDetail(detail)
	}
}

// CloseCause returns the first recorded cause, or CloseCauseNone while open.
func (s *Session) CloseCause() CloseCause {
	if s == nil {
		return CloseCauseNone
	}
	s.closeMu.Lock()
	defer s.closeMu.Unlock()
	return s.closeCause
}

// finishCloseRecord publishes the single close record of a served session.
func (s *Session) finishCloseRecord() {
	s.closeMu.Lock()
	cause, detail := s.closeCause, s.closeDetail
	s.closeMu.Unlock()
	if cause == CloseCauseNone {
		cause = CloseCauseServer
	}
	metricSessionCloses.Add(cause.String(), 1)
	if detail != "" {
		detail = fmt.Sprintf(" detail=%q", detail)
	}
	log.Printf("[#%d] disconnected: cause=%s%s queue_high_water=%d/%d max_queue_wait=%s sent=%d",
		s.ID, cause, detail, s.queueHighWater.Load(), cap(s.out),
		time.Duration(s.maxQueueWait.Load())*time.Microsecond, s.sentPackets.Load())
}

// writeLoop drains outbound packets. Every exit closes the connection so a
// failed or stalled writer also releases the reader and gameplay reservation.
func (s *Session) writeLoop() {
	defer s.Close()
	timeout := s.frameWriteTimeout
	if timeout <= 0 {
		timeout = defaultFrameWriteTimeout
	}
	var batch []byte
	for {
		select {
		case pkt, ok := <-s.out:
			if !ok || s.IsClosed() {
				return
			}
			// Join the packets already queued into one write. Order is kept and
			// the client reads a byte stream, so frame boundaries are unchanged.
			batch = s.takeOutbound(batch[:0], pkt)
			packets := int64(1)
		coalesce:
			for len(batch) < writeBatchBytes {
				select {
				case next := <-s.out:
					batch = s.takeOutbound(batch, next)
					packets++
				default:
					break coalesce
				}
			}
			if err := s.conn.SetWriteDeadline(time.Now().Add(timeout)); err != nil {
				s.recordClose(CloseCauseWriteError, fmt.Errorf("set write deadline: %w", err))
				return
			}
			n, err := s.conn.Write(batch)
			if err != nil {
				s.recordClose(writeFailureCause(err), err)
				return
			}
			if n != len(batch) {
				s.recordClose(CloseCauseWriteError, io.ErrShortWrite)
				return
			}
			s.sentPackets.Add(packets)
			metricOutputPackets.Add(packets)
			metricOutputBytes.Add(int64(n))
			metricOutputWrites.Add(1)
			if cap(batch) > 2*writeBatchBytes {
				batch = nil // Do not retain a buffer grown by one oversized burst.
			}
		case <-s.done:
			return
		}
	}
}

// takeOutbound records one dequeued packet and appends it to the batch.
func (s *Session) takeOutbound(batch []byte, pkt outboundPacket) []byte {
	s.queuedBytes.Add(-int64(len(pkt.data)))
	wait := max(time.Since(serverStart)-pkt.queuedAt, 0)
	micros := wait.Microseconds()
	raiseMax(&s.maxQueueWait, micros)
	raiseMax(&outputWaitMax, micros)
	metricOutputWait.Add(outputWaitLabel(wait), 1)
	return append(batch, pkt.data...)
}

// Serve performs InitCode and dispatches decrypted packets to handler.
// handler(s, nil) signals disconnection when the read lifetime ends.
func (s *Session) Serve(handler func(*Session, []byte)) {
	// The reader's own exit reason applies only when no earlier cause exists.
	var cause CloseCause
	var detail error
	defer s.finishCloseRecord()
	defer s.conn.Close()
	defer handler(s, nil)
	// End the lifetime before notifying World so a retry can recognize a dead
	// socket reservation even when the world command queue is full.
	defer func() {
		s.recordClose(cause, detail)
		s.closeDone()
	}()

	r := bufio.NewReader(s.conn)
	handshakeTimeout := s.handshakeTimeout
	if handshakeTimeout <= 0 {
		handshakeTimeout = defaultHandshakeTimeout
	}
	if err := s.conn.SetReadDeadline(time.Now().Add(handshakeTimeout)); err != nil {
		cause, detail = CloseCauseHandshake, fmt.Errorf("set InitCode deadline: %w", err)
		return
	}

	// The handshake has four InitCode bytes before any packet.
	var ic [4]byte
	if _, err := io.ReadFull(r, ic[:]); err != nil {
		cause, detail = CloseCauseHandshake, fmt.Errorf("missing InitCode: %w", err)
		return
	}
	if got := binary.LittleEndian.Uint32(ic[:]); got != uint32(wire.InitCode) {
		cause, detail = CloseCauseHandshake, fmt.Errorf("invalid InitCode: 0x%X", got)
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
			cause, detail = CloseCauseReadError, fmt.Errorf("set read deadline: %w", err)
			return
		}
		var sizeBytes [2]byte
		if _, err := io.ReadFull(r, sizeBytes[:1]); err != nil {
			cause, detail = readFailureCause(err), err
			return
		}
		// Start the frame timeout at its first byte, including a stalled Size
		// header. Removing AFK eviction must not enable single-byte Slowloris.
		if err := s.setPacketReadDeadline(false); err != nil {
			log.Printf("[#%d] set frame deadline: %v", s.ID, err)
		}
		if _, err := io.ReadFull(r, sizeBytes[1:]); err != nil {
			cause, detail = readFailureCause(err), fmt.Errorf("incomplete frame header: %w", err)
			return
		}
		size := int(binary.LittleEndian.Uint16(sizeBytes[:]))
		if size < wire.HeaderSize || size > wire.MaxPacketSize {
			cause, detail = CloseCauseProtocol, wire.ErrBadSize
			return
		}
		buf := make([]byte, size)
		copy(buf[:2], sizeBytes[:])
		_, err := io.ReadFull(r, buf[2:])
		if err != nil {
			cause, detail = readFailureCause(err), err
			return
		}
		checksumOK := wire.Decrypt(buf)
		if !checksumOK {
			cause, detail = CloseCauseProtocol, errInvalidChecksum
			return
		}
		now := time.Now()
		if now.Sub(windowStarted) >= time.Second {
			windowStarted, packetCount, byteCount = now, 0, 0
		}
		packetCount++
		byteCount += len(buf)
		if packetCount > packetLimit || byteCount > byteLimit {
			cause, detail = CloseCauseInboundFlood, fmt.Errorf("inbound flood: packets=%d bytes=%d/s", packetCount, byteCount)
			return
		}
		handler(s, buf)
	}
}
