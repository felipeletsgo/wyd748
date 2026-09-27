package game

import (
	"bytes"
	"log"
	"strings"
	"testing"
	"time"
)

func TestNPCGenerSummaryAggregatesAndResets(t *testing.T) {
	var output bytes.Buffer
	oldWriter, oldFlags := log.Writer(), log.Flags()
	log.SetOutput(&output)
	log.SetFlags(0)
	defer func() {
		log.SetOutput(oldWriter)
		log.SetFlags(oldFlags)
	}()

	now := time.Unix(100, 0)
	w := &World{
		npcGenerLogMode: npcGenerLogSummary,
		generators:      make([]generState, 3),
		mobs:            make([]*Mob, 8),
		npcGenerLog:     npcGenerLogStats{groups: 4, mobs: 8, relocations: 2},
	}
	w.flushNPCGenerLog(now, true)
	text := output.String()
	if !strings.Contains(text, "NPCGener initial summary") ||
		!strings.Contains(text, "groups=4") ||
		!strings.Contains(text, "mobs=8") ||
		!strings.Contains(text, "relocated=2") {
		t.Fatalf("incomplete summary: %q", text)
	}
	if w.npcGenerLog != (npcGenerLogStats{}) ||
		!w.nextGenerLog.Equal(now.Add(npcGenerSummaryInterval)) {
		t.Fatalf("aggregator was not reset: %+v next=%v", w.npcGenerLog, w.nextGenerLog)
	}
}

func TestNPCGenerQuietEmitsNothing(t *testing.T) {
	var output bytes.Buffer
	oldWriter := log.Writer()
	log.SetOutput(&output)
	defer log.SetOutput(oldWriter)

	w := &World{npcGenerLogMode: npcGenerLogQuiet,
		npcGenerLog: npcGenerLogStats{groups: 2, mobs: 10, relocations: 1}}
	w.flushNPCGenerLog(time.Now(), true)
	if output.Len() != 0 || w.npcGenerLog != (npcGenerLogStats{}) {
		t.Fatalf("quiet emitted a log or did not clear counters: %q %+v", output.String(), w.npcGenerLog)
	}
}

func TestWithNPCGenerLogModes(t *testing.T) {
	for input, want := range map[string]npcGenerLogMode{
		"quiet": npcGenerLogQuiet, "summary": npcGenerLogSummary,
		"verbose": npcGenerLogVerbose, "invalid": npcGenerLogSummary,
	} {
		w := &World{}
		WithNPCGenerLog(input)(w)
		if w.npcGenerLogMode != want {
			t.Fatalf("mode %q=%d, want %d", input, w.npcGenerLogMode, want)
		}
	}
}
