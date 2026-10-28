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

type accountPresenceRequest struct {
	ctx      context.Context
	accounts []string
	reply    chan map[string]bool
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

// AccountPresence resolves up to one directory page against the authoritative
// World state in a single queued read. It shares the overview admission gate.
func (w *World) AccountPresence(ctx context.Context, accounts []string) (map[string]bool, error) {
	if err := ctx.Err(); err != nil {
		return nil, err
	}
	if len(accounts) > 50 {
		return nil, control.ErrBusy
	}
	requested := make([]string, 0, len(accounts))
	seen := make(map[string]struct{}, len(accounts))
	for _, accountName := range accounts {
		name := strings.ToLower(strings.TrimSpace(accountName))
		if name == "" || len(name) > 12 {
			return nil, control.ErrBusy
		}
		for i := 0; i < len(name); i++ {
			if name[i] < 'a' || name[i] > 'z' {
				if name[i] < '0' || name[i] > '9' {
					return nil, control.ErrBusy
				}
			}
		}
		if _, exists := seen[name]; exists {
			continue
		}
		seen[name] = struct{}{}
		requested = append(requested, name)
	}
	if len(requested) == 0 {
		return map[string]bool{}, nil
	}
	if !w.controlPending.CompareAndSwap(false, true) {
		return nil, control.ErrBusy
	}
	req := &accountPresenceRequest{ctx: ctx, accounts: requested, reply: make(chan map[string]bool, 1)}
	select {
	case w.commands <- command{accountPresence: req, queuedAt: time.Now()}:
	default:
		w.controlPending.Store(false)
		return nil, control.ErrBusy
	}
	select {
	case result := <-req.reply:
		return result, nil
	case <-ctx.Done():
		return nil, ctx.Err()
	}
}

func (w *World) handleControl(req *controlRequest) {
	defer w.controlPending.Store(false)
	if req.ctx.Err() != nil {
		return
	}
	result := control.Overview{Version: 1, ModerationEpoch: w.moderationEpoch(), AsOf: time.Now().UTC(), State: "running",
		QueueDepth: 0,
		TickMicros: metricTickDurationMicros.Value(), LagMicros: metricLoopLagMicros.Value(),
		Players: make([]control.Player, 0), Offset: req.query.Offset}
	if w.shuttingDown {
		result.State = "maintenance"
	}
	for _, channel := range w.channelWorlds() {
		result.Authenticated += len(channel.players)
		result.Mobs += len(channel.mobsByID)
		result.QueueDepth += channel.commandQueueDepth()
	}
	search := strings.ToLower(req.query.Search)
	for _, p := range w.channelPlayers() {
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
		score := ch.RuntimeScore
		if score == nil {
			score = ch.Score
		}
		if score != nil {
			row.Level = score.Level
			row.HP, row.MaxHP = score.CurHP, score.MaxHP
			row.MP, row.MaxMP = score.CurMP, score.MaxMP
		}
		if !overviewFilterMatches(req.query, row) {
			continue
		}
		if p.Session != nil {
			row.SessionID = p.Session.ID
		}
		if p.Party != nil {
			row.PartySize = len(p.Party.Members)
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

// overviewFilterMatches applies the optional overview filters to a row built
// in the same pass; it adds no extra scan of players.
func overviewFilterMatches(q control.Query, row control.Player) bool {
	distance := func(a, b uint16) uint16 { return max(a, b) - min(a, b) }
	return (q.MinLevel == 0 || row.Level >= q.MinLevel) && (q.MaxLevel == 0 || row.Level <= q.MaxLevel) &&
		(!q.HasClass || row.Class == q.Class) && (q.GuildID == 0 || row.GuildID == q.GuildID) &&
		(q.Radius == 0 || (distance(row.X, q.AreaX) <= q.Radius && distance(row.Y, q.AreaY) <= q.Radius))
}

func (w *World) handleAccountPresence(req *accountPresenceRequest) {
	defer w.controlPending.Store(false)
	if req.ctx.Err() != nil {
		return
	}
	result := make(map[string]bool, len(req.accounts))
	for _, accountName := range req.accounts {
		result[accountName] = false
	}
	for _, player := range w.channelPlayers() {
		if player == nil || !player.InWorld || player.Account == nil {
			continue
		}
		accountName := strings.ToLower(player.Account.Name)
		if _, requested := result[accountName]; requested {
			result[accountName] = true
		}
	}
	req.reply <- result
}
