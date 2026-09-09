package game

import (
	"log"
	"strings"
	"time"
)

// Gameplay logging is deliberately separate from the NPC generator logger.
// A four-character AoE group can produce hundreds of ordinary kill/drop lines
// per minute; writing each line synchronously to a console stalls the World
// actor and does not improve the client protocol.  Verbose remains available
// for a focused reproduction, while summary is the safe operational default.
type gameplayLogMode byte

const (
	gameplayLogQuiet gameplayLogMode = iota
	gameplayLogSummary
	gameplayLogVerbose
)

const gameplayLogSummaryInterval = time.Minute

type gameplayLogStats struct {
	Skills  uint64
	Attacks uint64
	Kills   uint64
	Drops   uint64
	Gold    uint64
	Batches uint64
}

// WithGameplayLog selects quiet, summary or verbose. Invalid values fall back
// to summary; the data/server.txt loader validates the public configuration.
func WithGameplayLog(mode string) WorldOption {
	return func(w *World) {
		switch strings.ToLower(strings.TrimSpace(mode)) {
		case "quiet":
			w.gameplayLogMode = gameplayLogQuiet
		case "verbose":
			w.gameplayLogMode = gameplayLogVerbose
		default:
			w.gameplayLogMode = gameplayLogSummary
		}
	}
}

// gameplayLogf records hot-path gameplay diagnostics without forcing a
// synchronous log write in the normal summary mode.  The kind is a closed set
// used only for low-cardinality counters.
func (w *World) gameplayLogf(kind, format string, args ...interface{}) {
	switch kind {
	case "skill":
		w.gameplayLog.Skills++
	case "attack":
		w.gameplayLog.Attacks++
	case "kill":
		w.gameplayLog.Kills++
	case "drop":
		w.gameplayLog.Drops++
	case "gold":
		w.gameplayLog.Gold++
	case "batch":
		w.gameplayLog.Batches++
	}
	if w.gameplayLogMode == gameplayLogVerbose {
		log.Printf(format, args...)
	}
}

func (w *World) flushGameplayLog(now time.Time, initial bool) {
	if w.gameplayLogMode == gameplayLogQuiet {
		w.gameplayLog = gameplayLogStats{}
		w.nextGameplayLog = now.Add(gameplayLogSummaryInterval)
		return
	}
	if w.gameplayLogMode == gameplayLogVerbose || (!initial && now.Before(w.nextGameplayLog)) {
		return
	}
	stats := w.gameplayLog
	if initial || stats != (gameplayLogStats{}) {
		log.Printf("GAMEPLAY resumo: skills=%d ataques=%d mortes=%d drops=%d gold=%d lotes=%d",
			stats.Skills, stats.Attacks, stats.Kills, stats.Drops, stats.Gold, stats.Batches)
	}
	w.gameplayLog = gameplayLogStats{}
	w.nextGameplayLog = now.Add(gameplayLogSummaryInterval)
}
