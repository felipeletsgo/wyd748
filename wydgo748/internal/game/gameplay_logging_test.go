package game

import (
	"bytes"
	"log"
	"strings"
	"sync"
	"testing"
	"time"
)

// slowLogSink models a blocked console: every write costs a fixed delay.
type slowLogSink struct {
	mu     sync.Mutex
	delay  time.Duration
	writes int
	buf    bytes.Buffer
}

func (s *slowLogSink) Write(p []byte) (int, error) {
	time.Sleep(s.delay)
	s.mu.Lock()
	defer s.mu.Unlock()
	s.writes++
	return s.buf.Write(p)
}

func captureLog(t *testing.T, sink *slowLogSink) {
	t.Helper()
	output, flags := log.Writer(), log.Flags()
	log.SetOutput(sink)
	log.SetFlags(0)
	t.Cleanup(func() {
		log.SetOutput(output)
		log.SetFlags(flags)
	})
}

func TestSummaryModeKeepsRoutineCombatAndPickupLogsOffTheActor(t *testing.T) {
	sink := &slowLogSink{delay: time.Millisecond}
	captureLog(t, sink)
	start := time.Date(2026, 10, 31, 12, 0, 0, 0, time.UTC)
	w := &World{gameplayLogMode: gameplayLogSummary, nextGameplayLog: start.Add(gameplayLogSummaryInterval)}

	began := time.Now()
	for range 500 {
		w.gameplayLogf("pvp", "[#1] PvP attack against %q", "Target")
		w.gameplayLogf("miss", "[#1] missed attack on mob id=%d", 9)
		w.gameplayLogf("pickup", "[#1] picked up item %d", 1)
	}
	elapsed := time.Since(began)
	if sink.writes != 0 || elapsed > 100*time.Millisecond {
		t.Fatalf("summary mode wrote %d routine lines in %s", sink.writes, elapsed)
	}
	w.flushGameplayLog(start.Add(gameplayLogSummaryInterval), false)
	line := sink.buf.String()
	if sink.writes != 1 || !strings.Contains(line, "pvp=500") || !strings.Contains(line, "misses=500") ||
		!strings.Contains(line, "pickups=500") {
		t.Fatalf("summary line = %q (%d writes)", line, sink.writes)
	}
	t.Logf("1500 routine diagnostics took %s with a %s sink; one summary line", elapsed, sink.delay)

	verbose := &World{gameplayLogMode: gameplayLogVerbose}
	sink.writes = 0
	verbose.gameplayLogf("pvp", "[#1] PvP attack against %q", "Target")
	if sink.writes != 1 {
		t.Fatal("verbose mode must still write each focused reproduction line")
	}
}
