package control

import (
	"expvar"
	"sort"
	"strconv"

	"wydgo/internal/net"
)

// Disconnect, pool and critical persistence diagnostics for the staff panel
// (DOCS/server/server-correction-plan.md, W0). Map keys are closed label sets.

// Closed critical persistence label sets; the game publishes these keys.
var (
	CriticalOutcomeLabels   = [...]string{"committed", "rejected", "indeterminate"}
	CriticalAdmissionLabels = [...]string{"busy", "full", "stopping", "unsupported", "invalid"}
	RefusalLabels           = [...]string{"origin", "total_limit", "origin_limit"}
)

// TransportReading holds cumulative transport counters since process start.
type TransportReading struct {
	Closes        map[string]int64
	Refused       map[string]int64
	OutputPackets int64
	OutputBytes   int64
	OutputDepth   [len(net.OutputDepthLabels)]int64
	OutputWait    [len(DurationBucketLabels)]int64
}

// PoolReading holds game store pool gauges and cumulative counters.
type PoolReading struct {
	Acquired, Idle, Total, Max                                        int64
	Acquires, AcquireMicros, EmptyAcquires, EmptyWaitMicros, Canceled int64
}

// CriticalReading holds critical persistence gauges and cumulative counters.
type CriticalReading struct {
	Pending, Queued, Running, Quarantined, OldestMicros, Deferred int64
	Outcomes, Admission                                           map[string]int64
	Duration                                                      [len(DurationBucketLabels)]int64
}

type TransportPoint struct {
	Closes                 map[string]int64 `json:"closes"`
	Refused                int64            `json:"refused"`
	OutputPacketsPerSecond float64          `json:"outputPacketsPerSecond"`
	OutputBytesPerSecond   float64          `json:"outputBytesPerSecond"`
	QueueP99Percent        int64            `json:"queueP99Percent"`
	QueueMaxPercent        int64            `json:"queueMaxPercent"`
	WaitP99Micros          int64            `json:"waitP99Micros"`
}

type PoolPoint struct {
	Acquired          int64 `json:"acquired"`
	Idle              int64 `json:"idle"`
	Total             int64 `json:"total"`
	Max               int64 `json:"max"`
	AcquireMeanMicros int64 `json:"acquireMeanMicros"`
	EmptyAcquires     int64 `json:"emptyAcquires"`
	EmptyWaitMicros   int64 `json:"emptyWaitMicros"`
	Canceled          int64 `json:"canceled"`
}

type CriticalPoint struct {
	Pending           int64            `json:"pending"`
	Queued            int64            `json:"queued"`
	Running           int64            `json:"running"`
	Quarantined       int64            `json:"quarantined"`
	Deferred          int64            `json:"deferred"`
	OldestMicros      int64            `json:"oldestMicros"`
	DurationP99Micros int64            `json:"durationP99Micros"`
	Outcomes          map[string]int64 `json:"outcomes"`
	AdmissionRejected map[string]int64 `json:"admissionRejected"`
}

type ChannelPoint struct {
	Channel            int   `json:"channel"`
	Players            int64 `json:"players"`
	QueueDepth         int64 `json:"queueDepth"`
	Deferred           int64 `json:"deferred"`
	LoopLagP99Micros   int64 `json:"loopLagP99Micros"`
	TickP99Micros      int64 `json:"tickP99Micros"`
	CommandP99Micros   int64 `json:"commandP99Micros"`
	QueueAgeP99Micros  int64 `json:"queueAgeP99Micros"`
	HeartbeatAgeMicros int64 `json:"heartbeatAgeMicros"`
}

func expvarCounts(name string, labels []string) map[string]int64 {
	out := make(map[string]int64, len(labels))
	m, _ := expvar.Get(name).(*expvar.Map)
	for _, label := range labels {
		out[label] = 0
		if m == nil {
			continue
		}
		if v, ok := m.Get(label).(*expvar.Int); ok {
			out[label] = v.Value()
		}
	}
	return out
}

func readTransport() TransportReading {
	r := TransportReading{
		Closes:        expvarCounts("net_session_close_causes_total", net.CloseCauseLabels()),
		Refused:       expvarCounts("net_connections_refused_total", RefusalLabels[:]),
		OutputPackets: expvarInt("net_output_packets_total"), OutputBytes: expvarInt("net_output_bytes_total"),
		OutputWait: expvarBuckets("net_output_wait_buckets"),
	}
	depth := expvarCounts("net_output_queue_depth_buckets", net.OutputDepthLabels[:])
	for i, label := range net.OutputDepthLabels {
		r.OutputDepth[i] = depth[label]
	}
	return r
}

func readPool() PoolReading {
	return PoolReading{
		Acquired: expvarInt("postgres_pool_acquired_conns"), Idle: expvarInt("postgres_pool_idle_conns"),
		Total: expvarInt("postgres_pool_total_conns"), Max: expvarInt("postgres_pool_max_conns"),
		Acquires: expvarInt("postgres_pool_acquire_total"), AcquireMicros: expvarInt("postgres_pool_acquire_micros_total"),
		EmptyAcquires: expvarInt("postgres_pool_empty_acquire_total"), EmptyWaitMicros: expvarInt("postgres_pool_empty_wait_micros_total"),
		Canceled: expvarInt("postgres_pool_canceled_acquire_total"),
	}
}

func readCritical() CriticalReading {
	return CriticalReading{
		Pending: expvarInt("world_critical_pending"), Queued: expvarInt("world_critical_queued_jobs"),
		Running: expvarInt("world_critical_running"), Quarantined: expvarInt("world_critical_quarantined_now"),
		OldestMicros: expvarInt("world_critical_oldest_pending_micros"), Deferred: expvarInt("world_critical_deferred_commands"),
		Outcomes:  expvarCounts("world_critical_outcomes_total", CriticalOutcomeLabels[:]),
		Admission: expvarCounts("world_critical_admission_rejected_total", CriticalAdmissionLabels[:]),
		Duration:  expvarBuckets("world_critical_duration_buckets"),
	}
}

func readChannels() map[string]ChannelReading {
	if f, ok := expvar.Get(ChannelMetricsName).(expvar.Func); ok {
		if readings, ok := f().(map[string]ChannelReading); ok {
			return readings
		}
	}
	return nil
}

// bucketQuantile returns the bound of the bucket holding the 99th percentile
// of the interval, or 0 without observations. Bounds align with the counts.
func bucketQuantile(previous, current, bounds []int64) int64 {
	var total int64
	for i := range current {
		total += max(current[i]-previous[i], 0)
	}
	if total == 0 {
		return 0
	}
	threshold := (total*99 + 99) / 100
	var seen int64
	for i := range current {
		seen += max(current[i]-previous[i], 0)
		if seen >= threshold {
			return bounds[i]
		}
	}
	return bounds[len(bounds)-1]
}

// bucketMax returns the bound of the highest bucket observed in the interval.
func bucketMax(previous, current, bounds []int64) int64 {
	for i := len(current) - 1; i >= 0; i-- {
		if current[i] > previous[i] {
			return bounds[i]
		}
	}
	return 0
}

func countDeltas(previous, current map[string]int64) map[string]int64 {
	out := make(map[string]int64, len(current))
	for label, value := range current {
		out[label] = max(value-previous[label], 0)
	}
	return out
}

func sumCounts(counts map[string]int64) (total int64) {
	for _, value := range counts {
		total += value
	}
	return total
}

// IntervalDiagnostics converts two cumulative readings into interval points.
func IntervalDiagnostics(previous, current RuntimeReading) (TransportPoint, PoolPoint, CriticalPoint, []ChannelPoint) {
	seconds := current.At.Sub(previous.At).Seconds()
	if seconds <= 0 {
		seconds = 1
	}
	pt, ct := previous.Transport, current.Transport
	transport := TransportPoint{
		Closes: countDeltas(pt.Closes, ct.Closes), Refused: sumCounts(countDeltas(pt.Refused, ct.Refused)),
		OutputPacketsPerSecond: float64(max(ct.OutputPackets-pt.OutputPackets, 0)) / seconds,
		OutputBytesPerSecond:   float64(max(ct.OutputBytes-pt.OutputBytes, 0)) / seconds,
		QueueP99Percent:        bucketQuantile(pt.OutputDepth[:], ct.OutputDepth[:], net.OutputDepthPercent[:]),
		QueueMaxPercent:        bucketMax(pt.OutputDepth[:], ct.OutputDepth[:], net.OutputDepthPercent[:]),
		WaitP99Micros:          BucketP99(pt.OutputWait, ct.OutputWait),
	}
	pp, cp := previous.Pool, current.Pool
	pool := PoolPoint{Acquired: cp.Acquired, Idle: cp.Idle, Total: cp.Total, Max: cp.Max,
		EmptyAcquires: max(cp.EmptyAcquires-pp.EmptyAcquires, 0), EmptyWaitMicros: max(cp.EmptyWaitMicros-pp.EmptyWaitMicros, 0),
		Canceled: max(cp.Canceled-pp.Canceled, 0)}
	if acquires := cp.Acquires - pp.Acquires; acquires > 0 {
		pool.AcquireMeanMicros = max(cp.AcquireMicros-pp.AcquireMicros, 0) / acquires
	}
	pc, cc := previous.Critical, current.Critical
	critical := CriticalPoint{Pending: cc.Pending, Queued: cc.Queued, Running: cc.Running, Quarantined: cc.Quarantined,
		Deferred: cc.Deferred, OldestMicros: cc.OldestMicros, DurationP99Micros: BucketP99(pc.Duration, cc.Duration),
		Outcomes: countDeltas(pc.Outcomes, cc.Outcomes), AdmissionRejected: countDeltas(pc.Admission, cc.Admission)}
	channels := make([]ChannelPoint, 0, len(current.Channels))
	for name, now := range current.Channels {
		channel, err := strconv.Atoi(name)
		if err != nil {
			continue
		}
		before := previous.Channels[name]
		channels = append(channels, ChannelPoint{Channel: channel, Players: now.Players, QueueDepth: now.QueueDepth,
			Deferred: now.DeferredCommands, LoopLagP99Micros: BucketP99(before.LoopLag, now.LoopLag),
			TickP99Micros: BucketP99(before.Tick, now.Tick), CommandP99Micros: BucketP99(before.Command, now.Command),
			QueueAgeP99Micros: BucketP99(before.QueueAge, now.QueueAge), HeartbeatAgeMicros: now.HeartbeatAgeMicros})
	}
	sort.Slice(channels, func(i, j int) bool { return channels[i].Channel < channels[j].Channel })
	return transport, pool, critical, channels
}

// MergeDiagnostics folds consecutive points: gauges and percentiles keep the
// maximum, interval counts are summed and rates are averaged.
func MergeDiagnostics(points []MetricsPoint) (TransportPoint, PoolPoint, CriticalPoint, []ChannelPoint) {
	var transport TransportPoint
	var pool PoolPoint
	var critical CriticalPoint
	transport.Closes, critical.Outcomes, critical.AdmissionRejected = map[string]int64{}, map[string]int64{}, map[string]int64{}
	channels := map[int]ChannelPoint{}
	var acquireMeanTotal, acquireSamples int64
	for _, p := range points {
		t := p.Transport
		for label, value := range t.Closes {
			transport.Closes[label] += value
		}
		transport.Refused += t.Refused
		transport.OutputPacketsPerSecond += t.OutputPacketsPerSecond / float64(len(points))
		transport.OutputBytesPerSecond += t.OutputBytesPerSecond / float64(len(points))
		transport.QueueP99Percent = max(transport.QueueP99Percent, t.QueueP99Percent)
		transport.QueueMaxPercent = max(transport.QueueMaxPercent, t.QueueMaxPercent)
		transport.WaitP99Micros = max(transport.WaitP99Micros, t.WaitP99Micros)
		q := p.Pool
		pool.Acquired, pool.Idle = max(pool.Acquired, q.Acquired), max(pool.Idle, q.Idle)
		pool.Total, pool.Max = max(pool.Total, q.Total), max(pool.Max, q.Max)
		pool.EmptyAcquires += q.EmptyAcquires
		pool.EmptyWaitMicros += q.EmptyWaitMicros
		pool.Canceled += q.Canceled
		if q.AcquireMeanMicros > 0 {
			acquireMeanTotal += q.AcquireMeanMicros
			acquireSamples++
		}
		c := p.Critical
		critical.Pending, critical.Queued = max(critical.Pending, c.Pending), max(critical.Queued, c.Queued)
		critical.Running, critical.Quarantined = max(critical.Running, c.Running), max(critical.Quarantined, c.Quarantined)
		critical.Deferred, critical.OldestMicros = max(critical.Deferred, c.Deferred), max(critical.OldestMicros, c.OldestMicros)
		critical.DurationP99Micros = max(critical.DurationP99Micros, c.DurationP99Micros)
		for label, value := range c.Outcomes {
			critical.Outcomes[label] += value
		}
		for label, value := range c.AdmissionRejected {
			critical.AdmissionRejected[label] += value
		}
		for _, ch := range p.Channels {
			merged := channels[ch.Channel]
			merged.Channel = ch.Channel
			merged.Players, merged.QueueDepth = max(merged.Players, ch.Players), max(merged.QueueDepth, ch.QueueDepth)
			merged.Deferred = max(merged.Deferred, ch.Deferred)
			merged.LoopLagP99Micros = max(merged.LoopLagP99Micros, ch.LoopLagP99Micros)
			merged.TickP99Micros = max(merged.TickP99Micros, ch.TickP99Micros)
			merged.CommandP99Micros = max(merged.CommandP99Micros, ch.CommandP99Micros)
			merged.QueueAgeP99Micros = max(merged.QueueAgeP99Micros, ch.QueueAgeP99Micros)
			merged.HeartbeatAgeMicros = max(merged.HeartbeatAgeMicros, ch.HeartbeatAgeMicros)
			channels[ch.Channel] = merged
		}
	}
	if acquireSamples > 0 {
		pool.AcquireMeanMicros = acquireMeanTotal / acquireSamples
	}
	out := make([]ChannelPoint, 0, len(channels))
	for _, ch := range channels {
		out = append(out, ch)
	}
	sort.Slice(out, func(i, j int) bool { return out[i].Channel < out[j].Channel })
	return transport, pool, critical, out
}
