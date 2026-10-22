package webadmin

import (
	"net/http"
	"sync"
	"time"

	"wydgo/internal/control"
)

// Runtime charts and the panel circuit breaker (DOCS/server/staff-operations.md).
// The sampler reads process counters every 10 s into a fixed 24 h ring; it
// never sends anything to the World.

const (
	metricsInterval  = 10 * time.Second
	metricsCapacity  = int(24 * time.Hour / metricsInterval)
	metricsMaxPoints = 360
	// Breaker thresholds on the latest interval. Optional World reads are
	// refused above them; the default overview page stays available.
	breakerLoopLagMicros  = 100000
	breakerQueueAgeMicros = 250000
	breakerPostgresQueue  = 1024
)

var metricsWindows = map[string]time.Duration{
	"15m": 15 * time.Minute, "1h": time.Hour, "6h": 6 * time.Hour, "24h": 24 * time.Hour,
}

type metricsSampler struct {
	mu      sync.Mutex
	ring    []control.MetricsPoint
	next    int
	count   int
	last    control.RuntimeReading
	hasLast bool
}

func newMetricsSampler() *metricsSampler {
	return &metricsSampler{ring: make([]control.MetricsPoint, metricsCapacity)}
}

// observe turns a cumulative reading into one interval point. The first
// reading only primes the deltas.
func (s *metricsSampler) observe(r control.RuntimeReading) {
	s.mu.Lock()
	defer s.mu.Unlock()
	previous, primed := s.last, s.hasLast
	s.last, s.hasLast = r, true
	if !primed || !r.At.After(previous.At) {
		return
	}
	seconds := r.At.Sub(previous.At).Seconds()
	delta := func(current, before int64) int64 { return max(current-before, 0) }
	s.ring[s.next] = control.MetricsPoint{
		At: r.At, Players: r.Players, Mobs: r.Mobs, QueueDepth: r.QueueDepth,
		LoopLagP99Micros: control.BucketP99(previous.LoopLag, r.LoopLag), TickP99Micros: control.BucketP99(previous.Tick, r.Tick),
		QueueAgeP99Micros: control.BucketP99(previous.QueueAge, r.QueueAge),
		CommandsPerSecond: float64(delta(r.CommandsTotal, previous.CommandsTotal)) / seconds,
		BudgetExceeded:    delta(r.BudgetExceeded, previous.BudgetExceeded), Panics: delta(r.Panics, previous.Panics),
		PostgresQueueDepth: r.PostgresQueueDepth, PostgresFailures: delta(r.PostgresFailures, previous.PostgresFailures),
		PostgresTxMicros: r.PostgresTxMicros, HeapBytes: r.HeapBytes, Goroutines: r.Goroutines,
		GCCycles: delta(r.GCCycles, previous.GCCycles),
	}
	s.next = (s.next + 1) % len(s.ring)
	s.count = min(s.count+1, len(s.ring))
}

func (s *metricsSampler) latest() (control.MetricsPoint, bool) {
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.count == 0 {
		return control.MetricsPoint{}, false
	}
	return s.ring[(s.next-1+len(s.ring))%len(s.ring)], true
}

// window returns the points newer than since, oldest first, merged into at
// most metricsMaxPoints groups: peaks and gauges keep the group maximum,
// interval counts are summed and the command rate is averaged.
func (s *metricsSampler) window(since time.Time) []control.MetricsPoint {
	s.mu.Lock()
	points := make([]control.MetricsPoint, 0, s.count)
	for i := s.count; i > 0; i-- {
		point := s.ring[(s.next-i+len(s.ring))%len(s.ring)]
		if point.At.After(since) {
			points = append(points, point)
		}
	}
	s.mu.Unlock()
	group := (len(points) + metricsMaxPoints - 1) / metricsMaxPoints
	if group <= 1 {
		return points
	}
	merged := make([]control.MetricsPoint, 0, metricsMaxPoints)
	for start := 0; start < len(points); start += group {
		part := points[start:min(start+group, len(points))]
		out := part[len(part)-1]
		out.BudgetExceeded, out.Panics, out.PostgresFailures, out.GCCycles, out.CommandsPerSecond = 0, 0, 0, 0, 0
		for _, p := range part {
			out.Players, out.Mobs, out.QueueDepth = max(out.Players, p.Players), max(out.Mobs, p.Mobs), max(out.QueueDepth, p.QueueDepth)
			out.LoopLagP99Micros, out.TickP99Micros = max(out.LoopLagP99Micros, p.LoopLagP99Micros), max(out.TickP99Micros, p.TickP99Micros)
			out.QueueAgeP99Micros, out.PostgresQueueDepth = max(out.QueueAgeP99Micros, p.QueueAgeP99Micros), max(out.PostgresQueueDepth, p.PostgresQueueDepth)
			out.PostgresTxMicros, out.HeapBytes, out.Goroutines = max(out.PostgresTxMicros, p.PostgresTxMicros), max(out.HeapBytes, p.HeapBytes), max(out.Goroutines, p.Goroutines)
			out.BudgetExceeded += p.BudgetExceeded
			out.Panics += p.Panics
			out.PostgresFailures += p.PostgresFailures
			out.GCCycles += p.GCCycles
			out.CommandsPerSecond += p.CommandsPerSecond / float64(len(part))
		}
		merged = append(merged, out)
	}
	return merged
}

func (h *Handler) startMetricsSampler() {
	if h.cfg.Metrics == nil {
		return
	}
	h.sampler = newMetricsSampler()
	h.samplerStop = make(chan struct{})
	h.samplerDone = make(chan struct{})
	h.sampler.observe(h.cfg.Metrics())
	go func() {
		defer close(h.samplerDone)
		ticker := time.NewTicker(metricsInterval)
		defer ticker.Stop()
		for {
			select {
			case <-ticker.C:
				h.sampler.observe(h.cfg.Metrics())
			case <-h.samplerStop:
				return
			}
		}
	}()
}

// breaker reports whether optional World reads should be refused now.
func (h *Handler) breaker() control.BreakerState {
	if h.sampler == nil {
		return control.BreakerState{}
	}
	point, ok := h.sampler.latest()
	switch {
	case !ok:
		return control.BreakerState{}
	case point.LoopLagP99Micros > breakerLoopLagMicros:
		return control.BreakerState{Open: true, Reason: "loop_lag"}
	case point.QueueAgeP99Micros > breakerQueueAgeMicros:
		return control.BreakerState{Open: true, Reason: "command_queue_age"}
	case point.PostgresQueueDepth > breakerPostgresQueue:
		return control.BreakerState{Open: true, Reason: "persistence_backlog"}
	}
	return control.BreakerState{}
}

func (h *Handler) metricsHistory(w http.ResponseWriter, r *http.Request) {
	_, _, ok := h.staffGate(w, r, "server.metrics", h.sampler != nil, "metrics_unavailable", StatusCapability)
	if !ok {
		return
	}
	name := r.URL.Query().Get("window")
	if name == "" {
		name = "1h"
	}
	span, valid := metricsWindows[name]
	if !valid {
		writeError(w, 400, "invalid_query")
		return
	}
	now := h.now().UTC()
	writeJSON(w, 200, control.MetricsHistory{Version: 1, AsOf: now, Window: name,
		IntervalSeconds: int(metricsInterval / time.Second), Points: h.sampler.window(now.Add(-span)), Breaker: h.breaker()})
}

// optionalReadAllowed refuses an optional World read while the breaker is open.
func (h *Handler) optionalReadAllowed(w http.ResponseWriter) bool {
	if state := h.breaker(); state.Open {
		w.Header().Set("Retry-After", "10")
		writeError(w, 503, "temporarily_unavailable")
		return false
	}
	return true
}
