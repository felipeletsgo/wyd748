package game

import (
	"context"
	"errors"
	"expvar"
	"log"
	"sync/atomic"
	"time"

	"wydgo/internal/net"
	"wydgo/internal/store"
)

// Account ownership leases (DOCS/server/server-correction-plan.md, W9 phase 1).
// The in-memory account claim keeps login single inside one process. The
// lease in PostgreSQL keeps it single across processes: the login worker
// claims the lease before it reloads the account, and the actor releases it
// when the account session ends. A periodic reconcile removes leases that a
// lost release left behind.

type accountLeaseStore interface {
	RegisterGameProcess() (string, error)
	HeartbeatGameProcess(process string) error
	ClaimAccountLease(account, process string, channel byte, session int64) (string, error)
	ReleaseAccountLease(account, process string, session int64) error
	ReconcileAccountLeases(process string, held []string) error
	ReleaseProcessLeases(process string) error
}

const (
	accountLeaseHeartbeat = store.AccountLeaseStaleAfter / 3
	accountLeaseReconcile = time.Minute
)

var (
	errAccountLeaseHeld = errors.New("account lease: another game process holds the account")
	// errAccountTransferPending refuses a plain login while a durable channel
	// ticket of the account waits (W9 phase 2).
	errAccountTransferPending = errors.New("account lease: a channel transfer of the account is pending")
	errChannelTicketInvalid   = errors.New("channel ticket is not valid")
)

// channelTicketStore holds durable channel tickets (W9 phase 2).
type channelTicketStore interface {
	IssueChannelTicket(tokenHash []byte, account string, channel byte, slot int, ttl time.Duration) error
	ChannelTicketValid(tokenHash []byte, account string, channel byte) (bool, error)
	ClaimAccountLeaseWithTicket(account, process string, channel byte, session int64, tokenHash []byte) (int, error)
	CancelChannelTicket(tokenHash []byte) error
}

var metricAccountLeases = expvar.NewMap("world_account_lease_total")

// AccountLeases is one game process's lease client. Channels of one process
// share it.
type AccountLeases struct {
	store   accountLeaseStore
	tickets channelTicketStore
	process string
	// heartbeatFailures counts consecutive failed heartbeats.
	heartbeatFailures atomic.Int64
}

// NewAccountLeases registers this process. It returns nil when the store has
// no lease support.
func NewAccountLeases(backend store.Store) (*AccountLeases, error) {
	leaseStore, ok := backend.(accountLeaseStore)
	if !ok {
		return nil, nil
	}
	process, err := leaseStore.RegisterGameProcess()
	if err != nil {
		return nil, err
	}
	tickets, _ := backend.(channelTicketStore)
	return &AccountLeases{store: leaseStore, tickets: tickets, process: process}, nil
}

// Process returns this process's registered name.
func (l *AccountLeases) Process() string {
	if l == nil {
		return ""
	}
	return l.process
}

// WithAccountLeases makes login claim a process lease. Production enables it.
func WithAccountLeases(leases *AccountLeases) WorldOption {
	return func(w *World) { w.leases = leases }
}

// Run renews this process's heartbeat until ctx ends.
func (l *AccountLeases) Run(ctx context.Context) {
	if l == nil {
		return
	}
	ticker := time.NewTicker(accountLeaseHeartbeat)
	defer ticker.Stop()
	for {
		select {
		case <-ctx.Done():
			return
		case <-ticker.C:
			if err := l.store.HeartbeatGameProcess(l.process); err != nil {
				failures := l.heartbeatFailures.Add(1)
				metricAccountLeases.Add("heartbeat_failed", 1)
				log.Printf("account leases: heartbeat failed (%d consecutive): %v", failures, err)
				continue
			}
			l.heartbeatFailures.Store(0)
		}
	}
}

// Close releases every lease of this process. Call it after the final saves.
func (l *AccountLeases) Close() {
	if l == nil {
		return
	}
	if err := l.store.ReleaseProcessLeases(l.process); err != nil {
		log.Printf("account leases: release at shutdown failed: %v", err)
	}
}

// claim runs on a login worker. It returns errAccountLeaseHeld when another
// live process owns the account.
func (l *AccountLeases) claim(account string, channel byte, session int64) error {
	owner, err := l.store.ClaimAccountLease(account, l.process, channel, session)
	switch {
	case errors.Is(err, store.ErrChannelTransferPending):
		metricAccountLeases.Add("claim_refused", 1)
		return errAccountTransferPending
	case err != nil:
		metricAccountLeases.Add("claim_failed", 1)
		return err
	case owner != "":
		metricAccountLeases.Add("claim_refused", 1)
		return errAccountLeaseHeld
	}
	metricAccountLeases.Add("claimed", 1)
	return nil
}

// claimWithTicket consumes a durable channel ticket and claims the lease in
// one transaction. It returns the ticket's character slot.
func (l *AccountLeases) claimWithTicket(account string, channel byte, session int64, ticket []byte) (int, error) {
	slot, err := l.tickets.ClaimAccountLeaseWithTicket(account, l.process, channel, session, ticket)
	switch {
	case errors.Is(err, store.ErrChannelTicketInvalid):
		metricAccountLeases.Add("ticket_refused", 1)
		return 0, errChannelTicketInvalid
	case err != nil:
		metricAccountLeases.Add("claim_failed", 1)
		return 0, err
	}
	metricAccountLeases.Add("ticket_claimed", 1)
	return slot, nil
}

// cancelTicket removes an undelivered durable ticket off the actor. An
// unremoved ticket expires with its TTL.
func (l *AccountLeases) cancelTicket(ticket []byte) {
	if l == nil || l.tickets == nil {
		return
	}
	go func() {
		if err := l.tickets.CancelChannelTicket(ticket); err != nil {
			log.Printf("account leases: cancel channel ticket failed: %v", err)
		}
	}()
}

// release ends one session's lease off the actor. A lost release is removed
// by the next reconcile.
func (l *AccountLeases) release(account string, session int64) {
	if l == nil {
		return
	}
	go func() {
		if err := l.store.ReleaseAccountLease(account, l.process, session); err != nil {
			metricAccountLeases.Add("release_failed", 1)
			log.Printf("account leases: release account=%q failed: %v", account, err)
		}
	}()
}

// reconcileAccountLeases runs on the actor: it takes the held accounts and
// removes this process's other leases off the actor. Channels of one process
// share the account sessions, so only the first channel runs it.
func (w *World) reconcileAccountLeases(now time.Time) {
	if w.leases == nil {
		return
	}
	w.releaseSettledAccountLeases()
	if now.Before(w.leaseReconcileAt) {
		return
	}
	w.leaseReconcileAt = now.Add(accountLeaseReconcile)
	if w.cluster != nil && len(w.cluster.worlds) > 0 && w.cluster.worlds[0] != w {
		return
	}
	held := make([]string, 0, len(w.accountSessions))
	for key := range w.accountSessions {
		held = append(held, key)
	}
	worlds := []*World{w}
	if w.cluster != nil {
		worlds = w.cluster.worlds
	}
	for _, world := range worlds {
		for key := range world.leaseReleases {
			held = append(held, key)
		}
	}
	if w.criticalWrites != nil {
		for key := range w.criticalWrites.queue.failedAccounts {
			held = append(held, key)
		}
	}
	leases := w.leases
	go func() {
		if err := leases.store.ReconcileAccountLeases(leases.process, held); err != nil {
			metricAccountLeases.Add("reconcile_failed", 1)
			log.Printf("account leases: reconcile failed: %v", err)
		}
	}()
}

// holdAccountLease records the admitted session that holds the lease.
func (w *World) holdAccountLease(s *net.Session, account string, leased bool) {
	if !leased {
		return
	}
	if w.leaseHolders == nil {
		w.leaseHolders = make(map[string]*net.Session)
	}
	w.leaseHolders[accountSessionKey(account)] = s
}

// releaseAccountLease ends the lease of a session whose account claim ended.
// While a write of the account is in flight or awaits recovery, another
// process must not read the account: the release waits until it settles.
func (w *World) releaseAccountLease(s *net.Session, account string) {
	if w.leases == nil || s == nil {
		return
	}
	key := accountSessionKey(account)
	if w.criticalAccountReserved(key) {
		if w.leaseReleases == nil {
			w.leaseReleases = make(map[string]int64)
		}
		w.leaseReleases[key] = s.ID
		return
	}
	w.leases.release(key, s.ID)
}

// releaseSettledAccountLeases runs on the actor and ends the waiting leases
// whose account writes settled.
func (w *World) releaseSettledAccountLeases() {
	for key, session := range w.leaseReleases {
		if w.criticalAccountReserved(key) {
			continue
		}
		delete(w.leaseReleases, key)
		w.leases.release(key, session)
	}
}
