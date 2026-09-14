package game

import (
	"context"
	"sort"
	"strings"
	"time"
	"unicode/utf8"

	"wydgo/internal/control"
)

type controlRequest struct {
	ctx   context.Context
	query control.Query
	reply chan control.Overview
}

// Overview admits at most ONE outstanding read, even if its caller times out.
// Only handleControl, on the World goroutine, may inspect mutable game state.
func (w *World) Overview(ctx context.Context, q control.Query) (control.Overview, error) {
	if err := ctx.Err(); err != nil {
		return control.Overview{}, err
	}
	if q.Offset < 0 || q.Offset > 10000 || utf8.RuneCountInString(q.Search) > 48 {
		return control.Overview{}, control.ErrBusy
	}
	if !w.controlPending.CompareAndSwap(false, true) {
		return control.Overview{}, control.ErrBusy
	}
	req := &controlRequest{ctx: ctx, query: q, reply: make(chan control.Overview, 1)}
	select {
	case w.commands <- command{control: req, queuedAt: time.Now()}:
	default:
		w.controlPending.Store(false)
		return control.Overview{}, control.ErrBusy
	}
	select {
	case result := <-req.reply:
		return result, nil
	case <-ctx.Done():
		return control.Overview{}, ctx.Err()
	}
}

func (w *World) handleControl(req *controlRequest) {
	defer w.controlPending.Store(false)
	if req.ctx.Err() != nil {
		return
	}
	result := control.Overview{Version: 1, AsOf: time.Now().UTC(), State: "running",
		Authenticated: len(w.players), Mobs: len(w.mobsByID), QueueDepth: w.commandQueueDepth(),
		TickMicros: metricTickDurationMicros.Value(), LagMicros: metricLoopLagMicros.Value(),
		Players: make([]control.Player, 0), Offset: req.query.Offset}
	if w.shuttingDown {
		result.State = "maintenance"
	}
	search := strings.ToLower(req.query.Search)
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || p.Account == nil {
			continue
		}
		result.Online++
		ch := p.Char
		if search != "" && !strings.Contains(strings.ToLower(ch.Name), search) && !strings.Contains(strings.ToLower(p.Account.Name), search) && !strings.Contains(strings.ToLower(ch.UID), search) {
			continue
		}
		row := control.Player{
			UID: ch.UID, Name: ch.Name, Account: p.Account.Name, Class: ch.Class,
			GuildID: ch.GuildID, GuildRank: ch.GuildRank, X: p.X, Y: p.Y,
		}
		if p.Session != nil {
			row.SessionID = p.Session.ID
		}
		if p.Party != nil {
			row.PartySize = len(p.Party.Members)
		}
		score := ch.RuntimeScore
		if score == nil {
			score = ch.Score
		}
		if score != nil {
			row.Level = score.Level
			row.HP, row.MaxHP = score.CurHP, score.MaxHP
			row.MP, row.MaxMP = score.CurMP, score.MaxMP
		}
		result.Players = append(result.Players, row)
	}
	sort.Slice(result.Players, func(i, j int) bool { return result.Players[i].UID < result.Players[j].UID })
	result.Matched = len(result.Players)
	start := min(req.query.Offset, result.Matched)
	end := min(start+control.PageSize, result.Matched)
	result.HasMore = end < result.Matched
	result.Players = result.Players[start:end]
	// Buffered reply remains non-blocking after a disconnected/timed-out caller.
	req.reply <- result
}
