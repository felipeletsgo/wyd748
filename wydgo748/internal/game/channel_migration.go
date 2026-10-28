package game

import (
	"crypto/rand"
	"encoding/base64"
	"errors"
	"fmt"
	"strconv"
	"strings"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
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

func (c *ChannelCluster) issueTicket(account string, channel byte, slot int, now time.Time) (string, error) {
	c.expireTickets(now)
	if c.world(channel) == nil || slot < 0 || slot >= maxCharactersPerAccount || len(c.tickets) >= 1000 {
		return "", errors.New("channel transfer unavailable")
	}
	var secret [32]byte
	if _, err := rand.Read(secret[:]); err != nil {
		return "", err
	}
	token := fmt.Sprintf("*%d.%s", channel, base64.RawURLEncoding.EncodeToString(secret[:]))
	if len(token) >= 52 {
		return "", errors.New("channel ticket exceeds wire field")
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

func (w *World) requestChannelTransfer(p *Player, argument string) {
	channel, err := strconv.ParseUint(strings.TrimSpace(argument), 10, 8)
	if err != nil || w.cluster == nil || w.cluster.world(byte(channel)) == nil || byte(channel) == w.channel {
		w.sendChannelMessage(p, "transfer_destination")
		return
	}
	target := w.cluster.world(byte(channel))
	if p == nil || p.Session == nil || p.Account == nil || p.Char == nil || !p.InWorld ||
		p.Session.IsClosed() || p.PersistencePoisoned || w.shuttingDown || target.shuttingDown ||
		w.shutdownLoginsBlocked(w.now()) || target.shutdownLoginsBlocked(target.now()) ||
		playerCurHP(p.Char) == 0 || w.criticalTradeReserved(p) || w.gameplaySpaceForPlayer(p) != "" ||
		p.Trade != nil || p.GhostShop != nil || w.channelWarParticipant(p) {
		w.sendChannelMessage(p, "transfer_busy")
		return
	}
	// Reserve the account only after the final durable snapshot is acknowledged.
	publish := func() error {
		if p.Session.IsClosed() || w.shuttingDown || target.shuttingDown {
			return nil
		}
		// Admission can close while the durable snapshot awaits acknowledgement.
		// Keep the source session when the destination would reject its login.
		if w.shutdownLoginsBlocked(w.now()) || target.shutdownLoginsBlocked(target.now()) {
			w.sendChannelMessage(p, "transfer_failed")
			return nil
		}
		token, err := w.cluster.issueTicket(p.Account.Name, byte(channel), p.CharSlot, w.now())
		if err != nil {
			w.sendChannelMessage(p, "transfer_failed")
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
		_, err = w.submitCriticalWritePhased(write, []*Player{p}, false,
			func(criticalPersistenceResult) error { return nil }, func(criticalPersistenceResult) error { return publish() })
		if err != nil {
			w.sendChannelMessage(p, "transfer_failed")
		}
		return
	}
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		w.sendChannelMessage(p, "transfer_failed")
		return
	}
	_ = publish()
}

func (w *World) channelWarParticipant(p *Player) bool {
	_, city := w.cityFighters[p]
	return city || towerWarArena.contains(p.X, p.Y)
}

var channelMessages = map[string][2]string{
	"transfer_destination": {"Use srv 1 or srv 2 to select another available channel.", "Use srv 1 ou srv 2 para selecionar outro canal disponivel."},
	"transfer_busy":        {"Leave the battle, instance, trade or shop before changing channels.", "Saia da batalha, instancia, troca ou loja antes de mudar de canal."},
	"transfer_failed":      {"Channel transfer failed. Try again shortly.", "Falha na troca de canal. Tente novamente em instantes."},
	"leave_guild_first":    {"Leave your guild before leaving citizenship.", "Saia da sua guilda antes de abandonar a cidadania."},
}

func (w *World) sendChannelMessage(p *Player, key string) {
	if p == nil || p.Session == nil {
		return
	}
	text := channelMessages[key][0]
	if w.channel == 1 && channelMessages[key][1] != "" {
		text = channelMessages[key][1]
	}
	p.Session.Send(wire.MessagePanel(text))
}
