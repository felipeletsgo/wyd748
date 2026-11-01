package control

import (
	"expvar"
	"testing"
	"time"

	"wydgo/internal/net"
)

func TestIntervalDiagnosticsUsesDeltasAndGauges(t *testing.T) {
	at := time.Date(2026, 10, 31, 12, 0, 0, 0, time.UTC)
	previous := RuntimeReading{At: at,
		Transport: TransportReading{Closes: map[string]int64{"output_overflow": 1, "peer_closed": 5},
			Refused: map[string]int64{"origin_limit": 2}, OutputPackets: 100, OutputBytes: 1000},
		Pool:     PoolReading{Acquires: 10, AcquireMicros: 1000, EmptyAcquires: 1, EmptyWaitMicros: 500},
		Critical: CriticalReading{Outcomes: map[string]int64{"committed": 4}},
		Channels: map[string]ChannelReading{"1": {}},
	}
	current := previous
	current.At = at.Add(10 * time.Second)
	current.Transport = TransportReading{Closes: map[string]int64{"output_overflow": 3, "peer_closed": 5},
		Refused: map[string]int64{"origin_limit": 5}, OutputPackets: 1100, OutputBytes: 21000}
	current.Transport.OutputDepth[0], current.Transport.OutputDepth[4] = 99, 1
	current.Transport.OutputWait[1] = 10
	current.Pool = PoolReading{Acquired: 7, Max: 8, Acquires: 14, AcquireMicros: 3000, EmptyAcquires: 3, EmptyWaitMicros: 2500, Canceled: 1}
	current.Critical = CriticalReading{Pending: 2, OldestMicros: 900, Outcomes: map[string]int64{"committed": 9, "indeterminate": 1}}
	current.Critical.Duration[2] = 5
	lag := ChannelReading{Players: 30, HeartbeatAgeMicros: 400}
	lag.LoopLag[5] = 2
	current.Channels = map[string]ChannelReading{"2": {Players: 4}, "1": lag}

	transport, pool, critical, channels := IntervalDiagnostics(previous, current)
	if transport.Closes["output_overflow"] != 2 || transport.Closes["peer_closed"] != 0 || transport.Refused != 3 {
		t.Fatalf("transport counts: %+v", transport)
	}
	if transport.OutputPacketsPerSecond != 100 || transport.OutputBytesPerSecond != 2000 {
		t.Fatalf("transport rates: %+v", transport)
	}
	if transport.QueueP99Percent != 25 || transport.QueueMaxPercent != 100 || transport.WaitP99Micros != 5000 {
		t.Fatalf("queue distribution: %+v", transport)
	}
	if pool.Acquired != 7 || pool.Max != 8 || pool.AcquireMeanMicros != 500 || pool.EmptyAcquires != 2 ||
		pool.EmptyWaitMicros != 2000 || pool.Canceled != 1 {
		t.Fatalf("pool: %+v", pool)
	}
	if critical.Pending != 2 || critical.OldestMicros != 900 || critical.Outcomes["committed"] != 5 ||
		critical.Outcomes["indeterminate"] != 1 || critical.DurationP99Micros != 10000 {
		t.Fatalf("critical: %+v", critical)
	}
	if len(channels) != 2 || channels[0].Channel != 1 || channels[0].LoopLagP99Micros != 100000 ||
		channels[0].HeartbeatAgeMicros != 400 || channels[1].Players != 4 {
		t.Fatalf("channels: %+v", channels)
	}
}

func TestMergeDiagnosticsSumsCountsAndKeepsPeaks(t *testing.T) {
	points := []MetricsPoint{
		{Transport: TransportPoint{Closes: map[string]int64{"write_timeout": 1}, OutputPacketsPerSecond: 10, WaitP99Micros: 1000},
			Pool:     PoolPoint{Acquired: 8, AcquireMeanMicros: 100, EmptyAcquires: 2},
			Critical: CriticalPoint{Pending: 3, Outcomes: map[string]int64{"committed": 2}},
			Channels: []ChannelPoint{{Channel: 2, TickP99Micros: 5000}, {Channel: 1, Players: 9}}},
		{Transport: TransportPoint{Closes: map[string]int64{"write_timeout": 2}, OutputPacketsPerSecond: 30, WaitP99Micros: 250000},
			Pool:     PoolPoint{Acquired: 2, AcquireMeanMicros: 300, EmptyAcquires: 1},
			Critical: CriticalPoint{Pending: 1, Outcomes: map[string]int64{"committed": 1}},
			Channels: []ChannelPoint{{Channel: 1, Players: 12, HeartbeatAgeMicros: 700}}},
	}
	transport, pool, critical, channels := MergeDiagnostics(points)
	if transport.Closes["write_timeout"] != 3 || transport.OutputPacketsPerSecond != 20 || transport.WaitP99Micros != 250000 {
		t.Fatalf("transport: %+v", transport)
	}
	if pool.Acquired != 8 || pool.AcquireMeanMicros != 200 || pool.EmptyAcquires != 3 {
		t.Fatalf("pool: %+v", pool)
	}
	if critical.Pending != 3 || critical.Outcomes["committed"] != 3 {
		t.Fatalf("critical: %+v", critical)
	}
	if len(channels) != 2 || channels[0].Players != 12 || channels[0].HeartbeatAgeMicros != 700 || channels[1].TickP99Micros != 5000 {
		t.Fatalf("channels: %+v", channels)
	}
}

func TestReadRuntimeUsesPublishedTransportLabels(t *testing.T) {
	reading := ReadRuntime()
	for _, label := range net.CloseCauseLabels() {
		if _, ok := reading.Transport.Closes[label]; !ok {
			t.Fatalf("close cause %q missing from the staff reading", label)
		}
	}
	wait, ok := expvar.Get("net_output_wait_buckets").(*expvar.Map)
	if !ok {
		t.Fatal("transport wait buckets are not published")
	}
	for _, label := range DurationBucketLabels {
		if wait.Get(label) == nil {
			t.Fatalf("transport wait bucket %q does not match the staff labels", label)
		}
	}
	if len(net.OutputDepthLabels) != len(net.OutputDepthPercent) {
		t.Fatal("queue depth labels and bounds differ")
	}
}
