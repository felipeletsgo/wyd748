package game

import (
	"errors"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// Entry reads (DOCS/server/server-correction-plan.md, W2/P3). The final login
// account reload and the character sidecar read run on bounded workers. The
// actor owns each pending read and installs a result only while the same
// session, account claim and selection still own it. Without
// WithAsyncEntryReads the read runs inline through the same completion path.

// entryReadConcurrency bounds worker goroutines per World. The pool and the
// read budget still bound database work; this bounds goroutines and memory.
const entryReadConcurrency = 16

var errEntryReadsBusy = errors.New("entry reads: all workers are busy")

// WithAsyncEntryReads moves entry reads off the actor. Production enables it.
func WithAsyncEntryReads() WorldOption {
	return func(w *World) { w.asyncEntryReads = true }
}

// loginReload is the actor-owned pending state between the account claim and
// the authoritative reload. Its pointer identifies one admission attempt.
type loginReload struct {
	name string
	auth *loginResult
}

type loginReloadResult struct {
	reload  *loginReload
	account *model.Account
	err     error
}

// entryRead identifies one character selection awaiting its sidecar.
type entryRead struct {
	slot      int
	uid, name string
}

type entryReadResult struct {
	read  *entryRead
	state *model.CharState
	err   error
}

func (w *World) acquireEntryReadSlot() bool {
	if w.entryReadSlots == nil {
		w.entryReadSlots = make(chan struct{}, entryReadConcurrency)
	}
	select {
	case w.entryReadSlots <- struct{}{}:
		return true
	default:
		return false
	}
}

func (w *World) releaseEntryReadSlot() { <-w.entryReadSlots }

// startLoginReload reads the latest stored account after the session already
// owns the account claim and client slot. A stale authentication snapshot
// must never become live state.
func (w *World) startLoginReload(s *net.Session, auth *loginResult, name string) {
	reload := &loginReload{name: name, auth: auth}
	if w.loginReloads == nil {
		w.loginReloads = make(map[*net.Session]*loginReload)
	}
	w.loginReloads[s] = reload
	if !w.asyncEntryReads {
		fresh, err := w.store.LoadAccount(name)
		w.onLoginReload(s, &loginReloadResult{reload: reload, account: fresh, err: err})
		return
	}
	if !w.acquireEntryReadSlot() {
		w.failLoginReload(s, reload, "_WG_Login_Server_Busy_Try_Again")
		return
	}
	go func() {
		fresh, err := w.store.LoadAccount(name)
		w.releaseEntryReadSlot()
		w.commands <- command{s: s, queuedAt: time.Now(), loginReload: &loginReloadResult{reload: reload, account: fresh, err: err}}
	}()
}

// cancelLoginReload releases the claim of a pending reload exactly once. A
// late worker result then finds no pending state and is ignored.
func (w *World) cancelLoginReload(s *net.Session) {
	if reload := w.loginReloads[s]; reload != nil {
		delete(w.loginReloads, s)
		w.releaseAccountSession(s, &model.Account{Name: reload.name})
	}
}

func (w *World) failLoginReload(s *net.Session, reload *loginReload, message string) {
	if w.loginReloads[s] == reload {
		w.cancelLoginReload(s)
	}
	w.releaseAuthenticatedClientSlot(s)
	if message != "" && !s.IsClosed() {
		s.Send(wire.MessagePanel(w.msg(message)))
	}
	time.AfterFunc(300*time.Millisecond, s.Close)
}

func (w *World) onLoginReload(s *net.Session, result *loginReloadResult) {
	reload := result.reload
	if reload == nil || w.loginReloads[s] != reload {
		return // Cancelled by disconnect or shutdown; the claim is already released.
	}
	key := accountSessionKey(reload.name)
	switch {
	case s.IsClosed() || s.AdmissionRejected():
		w.failLoginReload(s, reload, "")
	case w.shuttingDown || w.shutdownLoginsBlocked(w.now()):
		w.failLoginReload(s, reload, "_NN_ServerReboot_Cant_Connect")
	case w.accountSessions[key] != s:
		w.failLoginReload(s, reload, "_WG_Login_Temporarily_Unavailable_Try_Again_Shortly")
	case reload.auth.sanctionVersion != w.staffOps.sanctionVersions[key]:
		// A sanction changed during the reload: the authentication snapshot may
		// miss a new ban or mute. The player signs in again with fresh state.
		log.Printf("[#%d] LOGIN sanction state changed during reload account=%q", s.ID, reload.name)
		w.failLoginReload(s, reload, "_WG_Login_Temporarily_Unavailable_Try_Again_Shortly")
	case result.err != nil || result.account == nil || accountSessionKey(result.account.Name) != key:
		log.Printf("[#%d] LOGIN failed to reload account %q: %v", s.ID, reload.name, result.err)
		w.failLoginReload(s, reload, "_WG_Error_Loading_Account_Try_Again")
	default:
		delete(w.loginReloads, s)
		w.completeLogin(s, reload.auth, result.account)
	}
}

// onEntryRead installs an asynchronous sidecar read on the actor.
func (w *World) onEntryRead(s *net.Session, result *entryReadResult) {
	p := w.players[s]
	if p == nil || result.read == nil || p.entryRead != result.read {
		return // The selection ended or a newer request owns the entry.
	}
	if err := w.finishCriticalEntryRead(p, result.read, result.state, result.err); err != nil {
		log.Printf("[#%d] failed to prepare character entry: %v", s.ID, err)
		if !s.IsClosed() {
			s.Send(wire.MessagePanel(w.msgFor(p, "_WG_Character_Could_Not_Prepared_Try_Again")))
		}
	}
}
