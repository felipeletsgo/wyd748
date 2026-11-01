package game

import (
	"errors"
	"expvar"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
)

func expvarMapValue(t *testing.T, m *expvar.Map, key string) int64 {
	t.Helper()
	v, ok := m.Get(key).(*expvar.Int)
	if !ok {
		t.Fatalf("metric key %q is not published", key)
	}
	return v.Value()
}

func lagBucket(t *testing.T, channel string, label string) int64 {
	t.Helper()
	reading, ok := readChannelMetrics(time.Now())[channel]
	if !ok {
		t.Fatalf("channel %s has no reading", channel)
	}
	for i, name := range control.DurationBucketLabels {
		if name == label {
			return reading.LoopLag[i]
		}
	}
	t.Fatalf("unknown bucket %q", label)
	return 0
}

// The cluster ticks every channel back to back on one goroutine. A shared
// previous-tick value measured the gap between channels and hid real lag.
func TestClusterTicksKeepPerChannelLagAndSummedGauges(t *testing.T) {
	first, second := &World{channel: 241}, &World{channel: 242}
	start := time.Now()
	first.observeTick(start, time.Millisecond)
	second.observeTick(start.Add(time.Millisecond), time.Millisecond)
	beforeFirst, beforeSecond := lagBucket(t, "241", "<=250ms"), lagBucket(t, "242", "<=250ms")
	late := start.Add(worldTickInterval + 200*time.Millisecond)
	first.observeTick(late, time.Millisecond)
	second.observeTick(late.Add(time.Millisecond), time.Millisecond)
	if lagBucket(t, "241", "<=250ms") != beforeFirst+1 || lagBucket(t, "242", "<=250ms") != beforeSecond+1 {
		t.Fatal("a late cluster tick was not recorded as lag on each channel")
	}

	first.observeWorldGauges(3, 10, 1)
	second.observeWorldGauges(4, 20, 2)
	var players, mobs, queue int64
	readings := readChannelMetrics(time.Now())
	for _, reading := range readings {
		players, mobs, queue = players+reading.Players, mobs+reading.Mobs, queue+reading.QueueDepth
	}
	if readings["241"].Players != 3 || readings["242"].Players != 4 {
		t.Fatalf("channel gauges overwrote each other: %+v %+v", readings["241"], readings["242"])
	}
	if metricActivePlayers.Value() != players || metricActiveMobs.Value() != mobs || metricCommandQueueDepth.Value() != queue {
		t.Fatal("process gauges are not the sum of channel gauges")
	}
	if readings["241"].HeartbeatAgeMicros <= 0 && readings["241"].Tick == ([len(control.DurationBucketLabels)]int64{}) {
		t.Fatal("channel heartbeat and tick distribution are missing")
	}
	if _, ok := expvar.Get(control.ChannelMetricsName).(expvar.Func)().(map[string]control.ChannelReading); !ok {
		t.Fatal("channel readings are not published with the control contract type")
	}
}

func TestCriticalQueueMetricsTrackRunningOutcomeAndAge(t *testing.T) {
	gate, release := criticalGate(t)
	defer release()
	backend := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate}
	q := newCriticalTestQueue(t, backend, 1, 2)
	committed := expvarMapValue(t, metricCriticalOutcomes, "committed")
	indeterminate := expvarMapValue(t, metricCriticalOutcomes, "indeterminate")

	if _, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "metrics-a"}}}); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	if metricCriticalRunning.Value() < 1 {
		t.Fatal("running worker is not visible")
	}
	observeCriticalQueue(q, time.Now().Add(time.Second))
	if metricCriticalPending.Value() != 1 || metricCriticalOldestMicros.Value() < time.Second.Microseconds() {
		t.Fatalf("pending=%d oldest=%dus", metricCriticalPending.Value(), metricCriticalOldestMicros.Value())
	}
	release()
	result := awaitCriticalResult(t, q)
	if err := q.acknowledge(result.id); err != nil {
		t.Fatal(err)
	}
	if expvarMapValue(t, metricCriticalOutcomes, "committed") != committed+1 {
		t.Fatal("committed outcome not counted")
	}

	backend.failure = errors.New("connection reset")
	if _, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "metrics-b"}}}); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	awaitCriticalResult(t, q)
	if expvarMapValue(t, metricCriticalOutcomes, "indeterminate") != indeterminate+1 {
		t.Fatal("an unclassified store error was not counted as indeterminate")
	}
	observeCriticalQueue(q, time.Now())
	if metricCriticalQueuedJobs.Value() != 0 {
		t.Fatalf("queued jobs = %d", metricCriticalQueuedJobs.Value())
	}
}

func TestCriticalAdmissionRefusalsUseClosedLabels(t *testing.T) {
	for _, tc := range []struct {
		err   error
		label string
	}{
		{errCriticalPersistenceBusy, "busy"},
		{errCriticalPersistenceFull, "full"},
		{errCriticalPersistenceStopping, "stopping"},
		{errCriticalPersistenceUnsupported, "unsupported"},
		{errors.New("critical persistence: invalid participant count"), "invalid"},
	} {
		before := expvarMapValue(t, metricCriticalAdmission, tc.label)
		observeCriticalAdmission(tc.err)
		if expvarMapValue(t, metricCriticalAdmission, tc.label) != before+1 {
			t.Fatalf("%v was not counted as %s", tc.err, tc.label)
		}
	}
}
