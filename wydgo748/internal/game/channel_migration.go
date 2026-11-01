package game

import (
	"crypto/rand"
	"crypto/sha256"
	"encoding/base64"
	"errors"
	"fmt"
	"log"
	"strconv"
	"strings"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

const channelTicketTTL = 30 * time.Second

type channelTicket struct {
	account string
	channel byte
	slot    int
	expires time.Time
	session *gameNet.Session
}

func (c *ChannelCluster) expireTickets(now time.Time) {
	for token, ticket := range c.tickets {
		if !now.Before(ticket.expires) {
			delete(c.tickets, token)
			if c.transfers[ticket.account] == ticket {
				delete(c.transfers, ticket.account)
			}
		}
	}
}

// newChannelTicketToken makes the token that the client returns to the
// target channel in the login packet.
func newChannelTicketToken(channel byte) (string, error) {
	var secret [32]byte
	if _, err := rand.Read(secret[:]); err != nil {
		return "", err
	}
	token := fmt.Sprintf("*%d.%s", channel, base64.RawURLEncoding.EncodeToString(secret[:]))
	if len(token) >= 52 {
		return "", errors.New("channel ticket exceeds wire field")
	}
	return token, nil
}

// channelTicketHash is the stored form of a durable ticket token.
func channelTicketHash(token string) []byte {
	sum := sha256.Sum256([]byte(token))
	return sum[:]
}

func (c *ChannelCluster) issueTicket(account string, channel byte, slot int, now time.Time) (string, error) {
	c.expireTickets(now)
	if c.world(channel) == nil || slot < 0 || slot >= maxCharactersPerAccount || len(c.tickets) >= 1000 {
		return "", errors.New("channel transfer unavailable")
	}
	token, err := newChannelTicketToken(channel)
	if err != nil {
		return "", err
	}
	if c.tickets == nil {
		c.tickets = make(map[string]*channelTicket)
		c.transfers = make(map[string]*channelTicket)
	}
	key := accountSessionKey(account)
	if c.transfers[key] != nil {
		return "", errors.New("account transfer already pending")
	}
	ticket := &channelTicket{account: key, channel: channel, slot: slot, expires: now.Add(channelTicketTTL)}
	c.tickets[token], c.transfers[key] = ticket, ticket
	return token, nil
}

// Consume on the actor before asynchronous account loading. A claimed ticket
// cannot authenticate another socket, channel, account, or a later replay.
func (w *World) consumeTicket(token, account string, s *gameNet.Session) (*channelTicket, bool) {
	if w.cluster == nil {
		return nil, false
	}
	c := w.cluster
	c.expireTickets(w.now())
	ticket := c.tickets[token]
	if ticket == nil || ticket.session != nil || ticket.channel != w.channel ||
		ticket.account != accountSessionKey(account) || c.transfers[ticket.account] != ticket {
		return nil, false
	}
	ticket.session = s
	return ticket, true
}

func (w *World) transferAdmissionAllowed(name string, s *gameNet.Session) bool {
	if w.cluster == nil {
		return true
	}
	w.cluster.expireTickets(w.now())
	ticket := w.cluster.transfers[accountSessionKey(name)]
	return ticket == nil || ticket.channel == w.channel && ticket.session == s
}

// WithRemoteChannels names the channels that other game processes serve
// (W9). A transfer to one of them uses a durable ticket in PostgreSQL.
func WithRemoteChannels(channels ...byte) WorldOption {
	return func(w *World) {
		w.remoteChannels = make(map[byte]bool, len(channels))
		for _, channel := range channels {
			w.remoteChannels[channel] = true
		}
	}
}

// durableTickets reports whether this World admits and issues durable
// channel tickets: it runs without a cluster and has lease support.
func (w *World) durableTickets() bool {
	return w.cluster == nil && w.leases != nil && w.leases.tickets != nil
}

func (w *World) channelTransferBlocked(p *Player) bool {
	return p == nil || p.Session == nil || p.Account == nil || p.Char == nil || !p.InWorld ||
		p.Session.IsClosed() || p.PersistencePoisoned || w.shuttingDown || w.shutdownLoginsBlocked(w.now()) ||
		playerCurHP(p.Char) == 0 || w.criticalTradeReserved(p) || w.gameplaySpaceForPlayer(p) != "" ||
		p.Trade != nil || p.GhostShop != nil || w.channelWarParticipant(p) || w.remoteTransfers[p.Session] != nil
}

func (w *World) requestChannelTransfer(p *Player, argument string) {
	parsed, err := strconv.ParseUint(strings.TrimSpace(argument), 10, 8)
	channel := byte(parsed)
	var target *World
	if w.cluster != nil {
		target = w.cluster.world(channel)
	}
	remote := target == nil && w.remoteChannels[channel] && w.durableTickets()
	if err != nil || channel == w.channel || target == nil && !remote {
		w.sendChannelMessage(p, "transfer_destination")
		return
	}
	if w.channelTransferBlocked(p) || target != nil && (target.shuttingDown || target.shutdownLoginsBlocked(target.now())) {
		w.sendChannelMessage(p, "transfer_busy")
		return
	}
	if remote {
		w.requestRemoteChannelTransfer(p, channel)
		return
	}
	stopped := func() bool { return target.shuttingDown }
	ready := func() bool { return !target.shutdownLoginsBlocked(target.now()) }
	issue := func() (string, error) {
		return w.cluster.issueTicket(p.Account.Name, channel, p.CharSlot, w.now())
	}
	w.commitChannelTransfer(p, stopped, ready, issue, nil)
}

// commitChannelTransfer saves the player's final state and, once that write
// is acknowledged, issues or reveals the ticket and moves the client. A
// stopped target ends the transfer silently; a target that refuses logins
// reports a failure. cancel removes a durable ticket that will not be
// delivered.
func (w *World) commitChannelTransfer(p *Player, stopped, ready func() bool, issue func() (string, error), cancel func()) {
	failed := func(p *Player) {
		if cancel != nil {
			cancel()
		}
		w.sendChannelMessage(p, "transfer_failed")
	}
	// Reserve the account only after the final durable snapshot is acknowledged.
	publish := func() error {
		if p.Session.IsClosed() || w.shuttingDown || stopped() {
			if cancel != nil {
				cancel()
			}
			return nil
		}
		// Admission can close while the durable snapshot awaits acknowledgement.
		// Keep the source session when the destination would reject its login.
		if w.shutdownLoginsBlocked(w.now()) || !ready() {
			failed(p)
			return nil
		}
		token, err := issue()
		if err != nil {
			failed(p)
			return nil
		}
		p.Session.Send(wire.ChannelMigration(p.ID, p.Account.Name, token))
		w.finishDisconnectAfterInstances(p.Session, p)
		w.releaseAuthenticatedClientSlot(p.Session)
		time.AfterFunc(300*time.Millisecond, p.Session.Close)
		return nil
	}
	if w.criticalWrites != nil {
		write := criticalWrite{accounts: []*model.Account{accountPersistenceSnapshot(p.Account)},
			character: &criticalCharacterState{uid: p.Char.UID, state: buildCharState(p, w.now())}}
		_, err := w.submitCriticalWriteWithRejection(write, []*Player{p}, false,
			func(criticalPersistenceResult) error { return nil }, func(criticalPersistenceResult) error { return publish() },
			w.rejectDetachedProposal("channel transfer", "", failed, p))
		if err != nil {
			failed(p)
		}
		return
	}
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		failed(p)
		return
	}
	_ = publish()
}

// remoteTransfer is a durable ticket issue in flight for one session.
type remoteTransfer struct {
	player  *Player
	channel byte
	token   string
	hash    []byte
}

// channelTicketIssued returns a durable ticket issue to the actor.
type channelTicketIssued struct {
	transfer *remoteTransfer
	err      error
}

// requestRemoteChannelTransfer stores the ticket off the actor first. The
// token reaches the client only after the final write commits, so the target
// cannot load the account before that write.
func (w *World) requestRemoteChannelTransfer(p *Player, channel byte) {
	token, err := newChannelTicketToken(channel)
	if err != nil || p.CharSlot < 0 || p.CharSlot >= maxCharactersPerAccount {
		w.sendChannelMessage(p, "transfer_failed")
		return
	}
	transfer := &remoteTransfer{player: p, channel: channel, token: token, hash: channelTicketHash(token)}
	if w.remoteTransfers == nil {
		w.remoteTransfers = make(map[*gameNet.Session]*remoteTransfer)
	}
	w.remoteTransfers[p.Session] = transfer
	leases, account, slot := w.leases, p.Account.Name, p.CharSlot
	go func() {
		err := leases.tickets.IssueChannelTicket(transfer.hash, account, channel, slot, channelTicketTTL)
		w.commands <- command{s: p.Session, queuedAt: time.Now(), channelTicket: &channelTicketIssued{transfer: transfer, err: err}}
	}()
}

func (w *World) onChannelTicketIssued(s *gameNet.Session, issued *channelTicketIssued) {
	transfer := issued.transfer
	if w.remoteTransfers[s] != transfer {
		w.leases.cancelTicket(transfer.hash)
		return
	}
	delete(w.remoteTransfers, s)
	p := transfer.player
	if issued.err != nil {
		log.Printf("[#%d] channel transfer ticket failed: %v", s.ID, issued.err)
		if errors.Is(issued.err, store.ErrChannelTransferPending) {
			w.sendChannelMessage(p, "transfer_busy")
		} else {
			w.sendChannelMessage(p, "transfer_failed")
		}
		return
	}
	cancel := func() { w.leases.cancelTicket(transfer.hash) }
	if w.players[s] != p || w.channelTransferBlocked(p) {
		cancel()
		if w.players[s] == p && !s.IsClosed() {
			w.sendChannelMessage(p, "transfer_busy")
		}
		return
	}
	always, never := func() bool { return true }, func() bool { return false }
	w.commitChannelTransfer(p, never, always, func() (string, error) { return transfer.token, nil }, cancel)
}

func (w *World) channelWarParticipant(p *Player) bool {
	_, city := w.cityFighters[p]
	return city || towerWarArena.contains(p.X, p.Y)
}

// channelMessages maps channel-transfer results to catalog keys.
var channelMessages = map[string]string{
	"transfer_destination": "_WG_Channel_Transfer_Destination",
	"transfer_busy":        "_WG_Channel_Transfer_Busy",
	"transfer_failed":      "_WG_Channel_Transfer_Failed",
	"leave_guild_first":    "_WG_Leave_Guild_Before_Citizenship",
}

func (w *World) sendChannelMessage(p *Player, key string) {
	if p == nil || p.Session == nil {
		return
	}
	w.panel(p.Session, channelMessages[key])
}
