package game

import (
	"context"
	"encoding/json"
	"log"
	"sort"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// Staff operations across channel processes (DOCS/server/server-correction-plan.md,
// W9 phase 5). The staff panel talks to one process. Its player list adds the
// players of the other channel processes, and a kick or teleport of a player
// that is not on its channel goes to the other processes; the process that
// holds the session runs it and returns the result. A whisper to a character
// that no process delivers reports "not connected" to the sender.

const (
	outboxStaffOverview = "staff.overview"
	outboxStaffPresence = "staff.presence"
	outboxStaffKick     = "staff.kick"
	outboxStaffTeleport = "staff.teleport"
	// remoteTeleportWait bounds how long a peer waits for its teleport to
	// settle before it stops waiting to reply.
	remoteTeleportWait = 10 * time.Second
)

// overviewPart is one process's share of the staff overview.
type overviewPart struct {
	Online        int              `json:"online"`
	Authenticated int              `json:"authenticated"`
	Mobs          int              `json:"mobs"`
	QueueDepth    int              `json:"queueDepth"`
	Players       []control.Player `json:"players"`
}

type remoteStaffCommand struct {
	Actor    string                  `json:"actor"`
	Kick     control.KickCommand     `json:"kick,omitempty"`
	Teleport control.TeleportCommand `json:"teleport,omitempty"`
}

// remotePeers is the number of other channel processes that answer requests.
func (w *World) remotePeers() int {
	if w.outbox == nil {
		return 0
	}
	return len(w.remoteChannels)
}

// mergeRemoteOverview adds the other processes' players to a complete local
// overview and applies the page of q.
func (w *World) mergeRemoteOverview(ctx context.Context, q control.Query, result control.Overview) control.Overview {
	body, _ := json.Marshal(q)
	for _, reply := range w.outbox.request(ctx, outboxStaffOverview, outboxPayload{Body: body}, w.remotePeers()) {
		var part overviewPart
		if json.Unmarshal(reply.Body, &part) != nil {
			continue
		}
		result.Online += part.Online
		result.Authenticated += part.Authenticated
		result.Mobs += part.Mobs
		result.QueueDepth += part.QueueDepth
		result.Players = append(result.Players, part.Players...)
	}
	sort.Slice(result.Players, func(i, j int) bool { return result.Players[i].UID < result.Players[j].UID })
	result.Matched = len(result.Players)
	start := min(q.Offset, result.Matched)
	end := min(start+control.PageSize, result.Matched)
	result.HasMore = end < result.Matched
	result.Players = result.Players[start:end]
	result.Offset = q.Offset
	return result
}

// mergeRemotePresence marks the accounts that another process has in world.
func (w *World) mergeRemotePresence(ctx context.Context, accounts []string, result map[string]bool) {
	body, _ := json.Marshal(accounts)
	for _, reply := range w.outbox.request(ctx, outboxStaffPresence, outboxPayload{Body: body}, w.remotePeers()) {
		var present map[string]bool
		if json.Unmarshal(reply.Body, &present) != nil {
			continue
		}
		for account, online := range present {
			if _, requested := result[account]; requested && online {
				result[account] = true
			}
		}
	}
}

// remoteKick asks the other processes to kick a session that is not here.
func (w *World) remoteKick(ctx context.Context, actor string, c control.KickCommand, local control.KickResult) control.KickResult {
	body, _ := json.Marshal(remoteStaffCommand{Actor: actor, Kick: c})
	for _, reply := range w.outbox.request(ctx, outboxStaffKick, outboxPayload{Body: body}, w.remotePeers()) {
		var result control.KickResult
		if json.Unmarshal(reply.Body, &result) == nil && result.Code != "stale_target" && result.Code != "" {
			return result
		}
	}
	return local
}

// remoteTeleport asks the other processes to teleport a session that is not here.
func (w *World) remoteTeleport(ctx context.Context, actor string, c control.TeleportCommand, local control.TeleportResult) control.TeleportResult {
	body, _ := json.Marshal(remoteStaffCommand{Actor: actor, Teleport: c})
	for _, reply := range w.outbox.request(ctx, outboxStaffTeleport, outboxPayload{Body: body}, w.remotePeers()) {
		var result control.TeleportResult
		if json.Unmarshal(reply.Body, &result) == nil && result.Code != "stale_target" && result.Code != "" {
			return result
		}
	}
	return local
}

// onRemoteStaffRequest runs another process's staff request on the actor and
// replies. The requesting process already authorized the actor and checked
// its moderation epoch; this process checks the session, UID and account.
func (w *World) onRemoteStaffRequest(kind string, payload outboxPayload) {
	switch kind {
	case outboxStaffOverview:
		var q control.Query
		if json.Unmarshal(payload.Body, &q) != nil {
			return
		}
		body, _ := json.Marshal(w.overviewPart(q))
		w.outbox.reply(payload, outboxPayload{Body: body})
	case outboxStaffPresence:
		var accounts []string
		if json.Unmarshal(payload.Body, &accounts) != nil || len(accounts) > 50 {
			return
		}
		present := make(map[string]bool, len(accounts))
		for _, player := range w.channelPlayers() {
			if player != nil && player.InWorld && player.Account != nil {
				present[accountSessionKey(player.Account.Name)] = true
			}
		}
		result := make(map[string]bool, len(accounts))
		for _, account := range accounts {
			result[account] = present[account]
		}
		body, _ := json.Marshal(result)
		w.outbox.reply(payload, outboxPayload{Body: body})
	case outboxStaffKick:
		var cmd remoteStaffCommand
		if json.Unmarshal(payload.Body, &cmd) != nil || cmd.Kick.Validate() != nil {
			return
		}
		req := &kickRequest{ctx: context.Background(), actor: cmd.Actor, command: cmd.Kick, remote: true,
			authorize: func() bool { return true }, reply: make(chan control.KickResult, 1)}
		w.handleKick(req)
		select {
		case result := <-req.reply:
			body, _ := json.Marshal(result)
			w.outbox.reply(payload, outboxPayload{Body: body})
		default:
		}
	case outboxStaffTeleport:
		var cmd remoteStaffCommand
		if json.Unmarshal(payload.Body, &cmd) != nil || cmd.Teleport.Validate() != nil {
			return
		}
		req := &teleportRequest{ctx: context.Background(), actor: cmd.Actor, command: cmd.Teleport, remote: true,
			authorize: func() bool { return true }, reply: make(chan control.TeleportResult, 1)}
		w.handleTeleport(req)
		outbox := w.outbox
		go func() {
			select {
			case result := <-req.reply:
				body, _ := json.Marshal(result)
				outbox.reply(payload, outboxPayload{Body: body})
			case <-time.After(remoteTeleportWait):
				log.Printf("[web-admin] remote teleport operation=%q did not settle in time", cmd.Teleport.OperationID)
			}
		}()
	}
}

// overviewPart lists every matching player of this process without paging.
func (w *World) overviewPart(q control.Query) overviewPart {
	q.Offset = 0
	overview := w.buildOverview(q, true)
	return overviewPart{Online: overview.Online, Authenticated: overview.Authenticated, Mobs: overview.Mobs,
		QueueDepth: overview.QueueDepth, Players: overview.Players}
}

// remoteWhisper offers a whisper to the other processes and reports "not
// connected" to the sender when none of them delivers it.
func (w *World) remoteWhisper(s *net.Session, sender, target, message string) {
	outbox, peers := w.outbox, w.remotePeers()
	go func() {
		ctx, cancel := context.WithTimeout(context.Background(), outboxRequestTimeout)
		defer cancel()
		for _, reply := range outbox.request(ctx, outboxWhisper, outboxPayload{Sender: sender, Target: target, Message: message}, peers) {
			if reply.Delivered {
				return
			}
		}
		w.commands <- command{queuedAt: time.Now(), apply: func() {
			if !s.IsClosed() {
				s.Send(wire.MessagePanel(w.msg("_NN_Not_Connected")))
			}
		}}
	}()
}
