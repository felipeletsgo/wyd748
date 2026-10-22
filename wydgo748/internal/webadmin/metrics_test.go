package webadmin

import (
	"encoding/json"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
)

type metricsFeed struct{ reading control.RuntimeReading }

func (f *metricsFeed) read() control.RuntimeReading { return f.reading }

func (f *metricsFeed) advance(lagBucket int, ticks int64) {
	f.reading.At = f.reading.At.Add(metricsInterval)
	f.reading.LoopLag[lagBucket] += ticks
	f.reading.Tick[0] += ticks
	f.reading.CommandsTotal += 200
	f.reading.Players = 42
}

func TestMetricsSamplerIntervalsAndDownsampling(t *testing.T) {
	s := newMetricsSampler()
	feed := &metricsFeed{reading: control.RuntimeReading{At: time.Date(2026, 10, 22, 0, 0, 0, 0, time.UTC)}}
	s.observe(feed.reading)
	if _, ok := s.latest(); ok {
		t.Fatal("first reading must only prime deltas")
	}
	for i := range metricsCapacity + 10 {
		bucket := 0
		if i == metricsCapacity { // one slow interval near the end
			bucket = 6
		}
		feed.advance(bucket, 20)
		s.observe(feed.reading)
	}
	last, _ := s.latest()
	if last.Players != 42 || last.CommandsPerSecond != 20 || last.LoopLagP99Micros != 1000 {
		t.Fatal(last)
	}
	all := s.window(feed.reading.At.Add(-25 * time.Hour))
	if len(all) != metricsMaxPoints {
		t.Fatal("24 h window must be downsampled", len(all))
	}
	peak := int64(0)
	for _, p := range all {
		peak = max(peak, p.LoopLagP99Micros)
	}
	if peak != 250000 {
		t.Fatal("downsampling lost the slow interval", peak)
	}
	if recent := s.window(feed.reading.At.Add(-15 * time.Minute)); len(recent) != 90 {
		t.Fatal("15 minutes holds 90 raw points", len(recent))
	}
}

func TestBreakerRefusesOptionalReadsOnly(t *testing.T) {
	h, entries, c, _ := newTestHandler(t)
	feed := &metricsFeed{reading: control.RuntimeReading{At: time.Now().UTC()}}
	h.cfg.Metrics = feed.read
	h.startMetricsSampler()
	defer h.Close()
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability}}
	cookie, _, _ := loginTest(t, h)
	feed.advance(6, 10) // p99 loop lag 250 ms
	h.sampler.observe(feed.reading)
	if r := request(h, "GET", "/api/v1/staff/overview?search=abc", "", cookie, ""); r.Code != 503 || !strings.Contains(r.Body.String(), "temporarily_unavailable") {
		t.Fatal("optional read allowed under load", r.Code)
	}
	if r := request(h, "GET", "/api/v1/staff/overview", "", cookie, ""); r.Code != 200 || c.actor != "operator" {
		t.Fatal("default page must stay available", r.Code)
	}
	if r := request(h, "GET", "/api/v1/staff/accounts", "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"presenceAvailable":false`) {
		t.Fatal("presence must be skipped under load", r.Body.String())
	}
	r := request(h, "GET", "/api/v1/staff/metrics?window=15m", "", cookie, "")
	var history control.MetricsHistory
	if r.Code != 200 || json.Unmarshal(r.Body.Bytes(), &history) != nil || !history.Breaker.Open || history.Breaker.Reason != "loop_lag" || len(history.Points) != 1 {
		t.Fatal(r.Code, r.Body.String())
	}
	feed.advance(0, 10)
	h.sampler.observe(feed.reading)
	if r := request(h, "GET", "/api/v1/staff/overview?search=abc", "", cookie, ""); r.Code != 200 {
		t.Fatal("breaker did not close", r.Code)
	}
	if r := request(h, "GET", "/api/v1/staff/metrics?window=7d", "", cookie, ""); r.Code != 400 {
		t.Fatal(r.Code)
	}
}

func TestMetricsRequireStatusAndSampler(t *testing.T) {
	h, entries, _, _ := newTestHandler(t)
	entries["operator"] = Staff{Capabilities: []string{PlayersCapability}}
	cookie, _, _ := loginTest(t, h)
	if r := request(h, "GET", "/api/v1/staff/metrics", "", cookie, ""); r.Code != 403 {
		t.Fatal(r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability}}
	cookie, _, _ = loginTest(t, h)
	if r := request(h, "GET", "/api/v1/staff/metrics", "", cookie, ""); r.Code != 503 || !strings.Contains(r.Body.String(), "metrics_unavailable") {
		t.Fatal(r.Code)
	}
}
