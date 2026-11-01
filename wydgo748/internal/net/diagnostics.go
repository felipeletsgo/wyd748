package net

import (
	"errors"
	"expvar"
	"io"
	"os"
	"sync/atomic"
	"time"
)

// Transport diagnostics (DOCS/server/server-correction-plan.md, W0). Every
// series is process-wide with a closed label set. Nothing is labeled by
// session, account or address; the per-session record goes to one log line.

// CloseCause is the first reason recorded for a session lifetime. Later
// failures caused by that closure never replace it.
type CloseCause uint8

const (
	CloseCauseNone CloseCause = iota
	CloseCauseHandshake
	CloseCausePeerClosed
	CloseCauseReadTimeout
	CloseCauseReadError
	CloseCauseProtocol
	CloseCauseInboundFlood
	CloseCauseOutputOverflow
	CloseCauseWriteTimeout
	CloseCauseWriteError
	CloseCauseInvalidOutbound
	CloseCauseAdmission
	CloseCauseServer
	CloseCausePersistence
	CloseCauseShutdown
	CloseCausePanic
	closeCauseCount
)

var closeCauseLabels = [closeCauseCount]string{
	"none", "handshake", "peer_closed", "read_timeout", "read_error", "protocol",
	"inbound_flood", "output_overflow", "write_timeout", "write_error", "invalid_outbound",
	"admission", "server", "persistence", "shutdown", "panic",
}

func (c CloseCause) String() string {
	if c >= closeCauseCount {
		return closeCauseLabels[CloseCauseServer]
	}
	return closeCauseLabels[c]
}

// CloseCauseLabels returns the published label set in a stable order.
func CloseCauseLabels() []string {
	return append([]string(nil), closeCauseLabels[1:]...)
}

// OutputDepthLabels are fractions of the session queue capacity at enqueue.
// "full" means the next packet without a writer drain closes the session.
var OutputDepthLabels = [...]string{"<=25%", "<=50%", "<=75%", "<100%", "full"}

// OutputDepthPercent is each depth label's nominal upper bound.
var OutputDepthPercent = [...]int64{25, 50, 75, 99, 100}

// Mirrors control.DurationBucketLabels; the last label is +Inf.
var outputWaitLimits = [...]time.Duration{
	time.Millisecond, 5 * time.Millisecond, 10 * time.Millisecond, 25 * time.Millisecond,
	50 * time.Millisecond, 100 * time.Millisecond, 250 * time.Millisecond, time.Second,
}

var outputWaitLabels = [...]string{"<=1ms", "<=5ms", "<=10ms", "<=25ms", "<=50ms", "<=100ms", "<=250ms", "<=1s", "+Inf"}

const maxCloseDetailBytes = 160

var (
	metricSessionCloses      = expvar.NewMap("net_session_close_causes_total")
	metricConnectionsRefused = expvar.NewMap("net_connections_refused_total")
	metricOutputPackets      = expvar.NewInt("net_output_packets_total")
	metricOutputBytes        = expvar.NewInt("net_output_bytes_total")
	// Writes counts coalesced socket writes; packets/writes is the batch size.
	metricOutputWrites = expvar.NewInt("net_output_writes_total")
	// Queue depth at enqueue and queue wait at dequeue, as fixed distributions.
	metricOutputDepth = expvar.NewMap("net_output_queue_depth_buckets")
	metricOutputWait  = expvar.NewMap("net_output_wait_buckets")
	outputHighWater   atomic.Int64
	outputWaitMax     atomic.Int64
)

func init() {
	for _, label := range closeCauseLabels[1:] {
		metricSessionCloses.Add(label, 0)
	}
	for _, label := range OutputDepthLabels {
		metricOutputDepth.Add(label, 0)
	}
	for _, label := range outputWaitLabels {
		metricOutputWait.Add(label, 0)
	}
	metricConnectionsRefused.Add("origin", 0)
	metricConnectionsRefused.Add("total_limit", 0)
	metricConnectionsRefused.Add("origin_limit", 0)
	// High-water marks since process start, in packets and microseconds.
	expvar.Publish("net_output_queue_high_water", expvar.Func(func() any { return outputHighWater.Load() }))
	expvar.Publish("net_output_wait_max_micros", expvar.Func(func() any { return outputWaitMax.Load() }))
}

func raiseMax(value *atomic.Int64, candidate int64) {
	for {
		current := value.Load()
		if candidate <= current || value.CompareAndSwap(current, candidate) {
			return
		}
	}
}

func outputDepthLabel(depth, capacity int) string {
	if capacity <= 0 || depth >= capacity {
		return OutputDepthLabels[len(OutputDepthLabels)-1]
	}
	for i, limit := range OutputDepthPercent[:len(OutputDepthPercent)-2] {
		if int64(depth)*100 <= limit*int64(capacity) {
			return OutputDepthLabels[i]
		}
	}
	return OutputDepthLabels[len(OutputDepthLabels)-2]
}

func outputWaitLabel(wait time.Duration) string {
	for i, limit := range outputWaitLimits {
		if wait <= limit {
			return outputWaitLabels[i]
		}
	}
	return outputWaitLabels[len(outputWaitLabels)-1]
}

func readFailureCause(err error) CloseCause {
	switch {
	case errors.Is(err, io.EOF), errors.Is(err, io.ErrUnexpectedEOF):
		return CloseCausePeerClosed
	case errors.Is(err, os.ErrDeadlineExceeded):
		return CloseCauseReadTimeout
	default:
		return CloseCauseReadError
	}
}

func writeFailureCause(err error) CloseCause {
	if errors.Is(err, os.ErrDeadlineExceeded) {
		return CloseCauseWriteTimeout
	}
	return CloseCauseWriteError
}

func boundedCloseDetail(err error) string {
	if err == nil {
		return ""
	}
	detail := err.Error()
	if len(detail) > maxCloseDetailBytes {
		detail = detail[:maxCloseDetailBytes]
	}
	return detail
}
