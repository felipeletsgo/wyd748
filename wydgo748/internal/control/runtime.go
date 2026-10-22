package control

import (
	"expvar"
	"runtime/metrics"
	"strconv"
	"time"
)

// Runtime metrics contract for the staff panel (DOCS/server/staff-operations.md).
// Readings use fixed expvar names and buckets only; nothing here is labeled by
// player, account or mob.

// DurationBucketLabels mirrors the fixed game buckets; the last one is +Inf.
var DurationBucketLabels = [...]string{"<=1ms", "<=5ms", "<=10ms", "<=25ms", "<=50ms", "<=100ms", "<=250ms", "<=1s", "+Inf"}

// DurationBucketMicros is each label's upper bound. Values above one second
// are reported as OverflowMicros.
var DurationBucketMicros = [...]int64{1000, 5000, 10000, 25000, 50000, 100000, 250000, 1000000, OverflowMicros}

const OverflowMicros = 1000001

// RuntimeReading is one read of the process counters. Counters and buckets
// are cumulative since process start.
type RuntimeReading struct {
	At                 time.Time
	Players            int64
	Mobs               int64
	QueueDepth         int64
	PostgresQueueDepth int64
	PostgresTxMicros   int64
	CommandsTotal      int64
	BudgetExceeded     int64
	Panics             int64
	PostgresFailures   int64
	HeapBytes          int64
	Goroutines         int64
	GCCycles           int64
	LoopLag            [len(DurationBucketLabels)]int64
	Tick               [len(DurationBucketLabels)]int64
	QueueAge           [len(DurationBucketLabels)]int64
}

func expvarBuckets(name string) (out [len(DurationBucketLabels)]int64) {
	m, ok := expvar.Get(name).(*expvar.Map)
	if !ok {
		return out
	}
	for i, label := range DurationBucketLabels {
		if v, ok := m.Get(label).(*expvar.Int); ok {
			out[i] = v.Value()
		}
	}
	return out
}

var runtimeSamples = []metrics.Sample{
	{Name: "/memory/classes/heap/objects:bytes"},
	{Name: "/sched/goroutines:goroutines"},
	{Name: "/gc/cycles/total:gc-cycles"},
}

func runtimeValue(sample metrics.Sample) int64 {
	if sample.Value.Kind() == metrics.KindUint64 {
		return int64(sample.Value.Uint64())
	}
	return 0
}

// ReadRuntime reads the process metrics without touching the World. It uses
// atomic expvar values and runtime/metrics, which does not stop the world.
func ReadRuntime() RuntimeReading {
	samples := append([]metrics.Sample(nil), runtimeSamples...)
	metrics.Read(samples)
	return RuntimeReading{
		At: time.Now().UTC(), Players: expvarInt("world_active_players"), Mobs: expvarInt("world_active_mobs"),
		QueueDepth: expvarInt("world_command_queue_depth"), PostgresQueueDepth: expvarInt("postgres_async_queue_depth"),
		PostgresTxMicros: expvarInt("postgres_transaction_duration_micros"), CommandsTotal: expvarInt("world_commands_total"),
		BudgetExceeded: expvarInt("world_command_budget_exceeded_total"), Panics: expvarInt("world_panics_total"),
		PostgresFailures: expvarInt("postgres_async_failures_total"),
		HeapBytes:        runtimeValue(samples[0]), Goroutines: runtimeValue(samples[1]), GCCycles: runtimeValue(samples[2]),
		LoopLag: expvarBuckets("world_loop_lag_buckets"), Tick: expvarBuckets("world_tick_duration_buckets"),
		QueueAge: expvarBuckets("world_command_age_buckets"),
	}
}

// MetricsPoint summarizes one sampling interval. P99 values are the upper
// bound of the bucket holding the 99th percentile of that interval (0 when
// nothing was observed); deltas count events inside the interval.
type MetricsPoint struct {
	At                 time.Time `json:"at"`
	Players            int64     `json:"players"`
	Mobs               int64     `json:"mobs"`
	QueueDepth         int64     `json:"queueDepth"`
	LoopLagP99Micros   int64     `json:"loopLagP99Micros"`
	TickP99Micros      int64     `json:"tickP99Micros"`
	QueueAgeP99Micros  int64     `json:"queueAgeP99Micros"`
	CommandsPerSecond  float64   `json:"commandsPerSecond"`
	BudgetExceeded     int64     `json:"budgetExceeded"`
	Panics             int64     `json:"panics"`
	PostgresQueueDepth int64     `json:"postgresQueueDepth"`
	PostgresFailures   int64     `json:"postgresFailures"`
	PostgresTxMicros   int64     `json:"postgresTxMicros"`
	HeapBytes          int64     `json:"heapBytes"`
	Goroutines         int64     `json:"goroutines"`
	GCCycles           int64     `json:"gcCycles"`
}

type BreakerState struct {
	Open   bool   `json:"open"`
	Reason string `json:"reason"`
}

type MetricsHistory struct {
	Version         int            `json:"version"`
	AsOf            time.Time      `json:"asOf"`
	Window          string         `json:"window"`
	IntervalSeconds int            `json:"intervalSeconds"`
	Points          []MetricsPoint `json:"points"`
	Breaker         BreakerState   `json:"breaker"`
}

// BucketP99 returns the upper bound of the bucket containing the 99th
// percentile of the observations between two cumulative readings.
func BucketP99(previous, current [len(DurationBucketLabels)]int64) int64 {
	var deltas [len(DurationBucketLabels)]int64
	var total int64
	for i := range deltas {
		deltas[i] = max(current[i]-previous[i], 0)
		total += deltas[i]
	}
	if total == 0 {
		return 0
	}
	threshold := (total*99 + 99) / 100
	var seen int64
	for i, delta := range deltas {
		seen += delta
		if seen >= threshold {
			return DurationBucketMicros[i]
		}
	}
	return OverflowMicros
}

// Optional reports whether the query asks for more than the default first
// page. Optional reads are refused while the panel circuit breaker is open.
func (q Query) Optional() bool {
	return q.Search != "" || q.Offset != 0 || q.MinLevel != 0 || q.MaxLevel != 0 || q.HasClass || q.GuildID != 0 || q.Radius != 0
}

func parseBounded(raw string, low, high int) (int, bool) {
	v, err := strconv.Atoi(raw)
	return v, err == nil && v >= low && v <= high
}
