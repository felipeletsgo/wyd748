package game

import (
	"expvar"
	"log"
	"time"

	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// Stale-session replacement (DOCS/server/server-correction-plan.md, W4/D4
// items 4–6). An authenticated login whose account still belongs to a closed
// transport waits, bounded, for that owner's writes and cleanup to finish and
// then takes the ordinary claim. The old owner is never evicted and an open
// previous session keeps the healthy duplicate-session policy (item 7). A
// quarantined account is not settling: it waits for a verified outcome.

const (
	loginReplacementWait  = 5 * time.Second
	loginReplacementLimit = 64
)

var metricLoginReplacement = expvar.NewMap("world_login_replacement_total")

type waitingLogin struct {
	session  *net.Session
	result   *loginResult
	deadline time.Time
}

// accountSettling reports an owner whose transport closed, or that already
// left, while accepted writes of the account have not finished.
func (w *World) accountSettling(key string) bool {
	if w.criticalWrites != nil && w.criticalWrites.queue.failedAccounts[key] != 0 {
		return false
	}
	if previous := w.accountSessions[key]; previous != nil {
		return previous.IsClosed()
	}
	return w.criticalAccountReserved(key)
}

// waitForAccountSettlement parks one login per account. A channel transfer
// keeps its own ticket flow and never waits here.
func (w *World) waitForAccountSettlement(s *net.Session, result *loginResult, key string) bool {
	if result.transfer != nil || result.durableTicket != nil {
		return false
	}
	if existing := w.waitingLogins[key]; existing != nil && existing.session != s && !existing.session.IsClosed() {
		return false
	}
	if w.waitingLogins == nil {
		w.waitingLogins = make(map[string]*waitingLogin)
	}
	if w.waitingLogins[key] == nil && len(w.waitingLogins) >= loginReplacementLimit {
		return false
	}
	w.waitingLogins[key] = &waitingLogin{session: s, result: result, deadline: w.now().Add(loginReplacementWait)}
	metricLoginReplacement.Add("waiting", 1)
	return true
}

// resumeWaitingLogins runs on every tick. A login whose previous owner is
// still settling at its deadline gets the temporary-unavailable reply.
func (w *World) resumeWaitingLogins(now time.Time) {
	for key, waiting := range w.waitingLogins {
		s := waiting.session
		if s.IsClosed() || s.AdmissionRejected() {
			delete(w.waitingLogins, key)
			metricLoginReplacement.Add("abandoned", 1)
			continue
		}
		if w.shuttingDown {
			delete(w.waitingLogins, key)
			s.Send(wire.MessagePanel(w.msg("_NN_ServerReboot_Cant_Connect")))
			time.AfterFunc(300*time.Millisecond, s.Close)
			continue
		}
		if w.accountSettling(key) && now.Before(waiting.deadline) {
			continue
		}
		delete(w.waitingLogins, key)
		w.admitAuthenticatedLogin(s, waiting.result, false)
	}
}

// rejectLoginClaim answers a login whose account claim failed.
func (w *World) rejectLoginClaim(s *net.Session, accountName, key string) {
	switch {
	case w.accountSettling(key):
		log.Printf("[#%d] LOGIN rejected account=%q: previous session still settling", s.ID, accountName)
		metricLoginReplacement.Add("timed_out", 1)
		s.Send(wire.MessagePanel(w.msg("_WG_Login_Temporarily_Unavailable_Try_Again_Shortly")))
	case w.criticalWrites != nil && w.criticalWrites.queue.failedAccounts[key] != 0:
		log.Printf("[#%d] LOGIN rejected account=%q: account awaits verified recovery", s.ID, accountName)
		s.Send(wire.MessagePanel(w.msg("_WG_Login_Temporarily_Unavailable_Try_Again_Shortly")))
	default:
		log.Printf("[#%d] LOGIN rejected account=%q: account already connected", s.ID, accountName)
		s.Send(wire.AlreadyPlaying())
	}
	time.AfterFunc(300*time.Millisecond, s.Close)
}
