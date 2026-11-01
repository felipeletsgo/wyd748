package game

import (
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

// W2/P3: entry reads run off the actor. These stores block until released so
// the tests observe the pending state the actor keeps meanwhile.
type gatedLoginStore struct {
	craftStore
	account *model.Account
	entered chan struct{}
	gate    chan struct{}
}

func (s *gatedLoginStore) LoadAccount(string) (*model.Account, error) {
	s.entered <- struct{}{}
	<-s.gate
	return accountStateSnapshot(s.account), nil
}

type gatedSidecarStore struct {
	*charStateMemoryStore
	entered chan struct{}
	gate    chan struct{}
}

func (s *gatedSidecarStore) LoadCharState(uid string) (*model.CharState, error) {
	s.entered <- struct{}{}
	<-s.gate
	return s.charStateMemoryStore.LoadCharState(uid)
}

func awaitSignal(t *testing.T, ch <-chan struct{}, what string) {
	t.Helper()
	select {
	case <-ch:
	case <-time.After(2 * time.Second):
		t.Fatalf("%s did not start", what)
	}
}

func awaitEntryCommand(t *testing.T, w *World) command {
	t.Helper()
	select {
	case cmd := <-w.commands:
		return cmd
	case <-time.After(2 * time.Second):
		t.Fatal("entry read result was not delivered to the actor")
	}
	return command{}
}

func asyncLoginWorld(t *testing.T) (*World, *gameNet.Session, *gatedLoginStore) {
	t.Helper()
	s := newLoginTestSession(1, 64)
	fresh := loginTestAccount()
	fresh.Chars[0].Gold = 99
	st := &gatedLoginStore{account: fresh, entered: make(chan struct{}, 2), gate: make(chan struct{})}
	w := loginResultWorld(&loginReloadStore{}, s)
	w.store, w.asyncEntryReads, w.commands = st, true, make(chan command, 4)
	t.Cleanup(func() {
		select {
		case <-st.gate:
		default:
			close(st.gate)
		}
	})
	return w, s, st
}

func TestAsyncLoginReloadKeepsActorFreeAndAdmitsFreshAccount(t *testing.T) {
	w, s, st := asyncLoginWorld(t)
	stale := loginTestAccount()
	stale.Chars[0].Gold = 1
	w.onLoginResult(s, &loginResult{accountName: "Felipe", account: stale})
	awaitSignal(t, st.entered, "account reload")
	if w.players[s] != nil || w.accountSessions["felipe"] != s || w.loginReloads[s] == nil || s.QueuedPacketsForTest() != 0 {
		t.Fatal("pending reload admitted the stale snapshot or lost the account claim")
	}
	close(st.gate)
	w.handle(awaitEntryCommand(t, w))
	if p := w.players[s]; p == nil || p.Account.Chars[0].Gold != 99 || w.loginReloads[s] != nil || w.accountSessions["felipe"] != s {
		t.Fatal("completed reload did not admit the fresh stored account")
	}
}

func TestAsyncLoginReloadCancelledByDisconnectOrSanctionChange(t *testing.T) {
	for _, mode := range []string{"disconnect", "sanction"} {
		t.Run(mode, func(t *testing.T) {
			w, s, st := asyncLoginWorld(t)
			w.onLoginResult(s, &loginResult{accountName: "Felipe", account: loginTestAccount()})
			awaitSignal(t, st.entered, "account reload")
			if mode == "disconnect" {
				s.Close()
				w.onDisconnect(s)
				if w.accountSessions["felipe"] != nil || w.loginReloads[s] != nil {
					t.Fatal("disconnect did not release the pending account claim")
				}
			} else {
				w.staffOps.sanctionVersions = map[string]uint64{"felipe": 1}
			}
			close(st.gate)
			w.handle(awaitEntryCommand(t, w))
			if w.players[s] != nil || w.accountSessions["felipe"] != nil || w.loginReloads[s] != nil {
				t.Fatal("a cancelled reload admitted the account or kept its claim")
			}
			if mode == "sanction" {
				pkt, ok := s.DequeuePacketForTest()
				want := w.msg("_WG_Login_Temporarily_Unavailable_Try_Again_Shortly")
				if !ok || !wire.Decrypt(pkt) || !strings.Contains(string(pkt[12:]), want) {
					t.Fatal("a sanction change during reload did not ask the player to retry")
				}
			}
		})
	}
}

func asyncEntryFixture(t *testing.T) (*World, *Player, *gatedSidecarStore) {
	t.Helper()
	w, p, fallback, _, _, release := criticalEntryFixture(t)
	release()
	st := &gatedSidecarStore{charStateMemoryStore: fallback, entered: make(chan struct{}, 2), gate: make(chan struct{})}
	w.store, w.asyncEntryReads, w.commands = st, true, make(chan command, 4)
	t.Cleanup(func() {
		select {
		case <-st.gate:
		default:
			close(st.gate)
		}
	})
	return w, p, st
}

func TestAsyncCharacterEntryReadsSidecarOffTheActorOnce(t *testing.T) {
	w, p, st := asyncEntryFixture(t)
	st.state = &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 7}}
	w.onEnterWorld(p.Session, enterWorldPacket(0))
	awaitSignal(t, st.entered, "sidecar read")
	w.onEnterWorld(p.Session, enterWorldPacket(0)) // a repeated request starts no second read
	if p.InWorld || p.Char != nil || p.entryRead == nil || len(st.entered) != 0 {
		t.Fatal("pending sidecar read selected the character or started a duplicate read")
	}
	close(st.gate)
	w.handle(awaitEntryCommand(t, w))
	if !p.InWorld || p.Char == nil || p.SpecialCoins["fame"] != 7 || p.entryRead != nil {
		t.Fatal("completed sidecar read did not finish the entry with the stored state")
	}
	select {
	case cmd := <-w.commands:
		t.Fatalf("a duplicate entry read was delivered: %+v", cmd.entryRead)
	default:
	}
}

func TestAsyncCharacterEntryDropsStaleSelection(t *testing.T) {
	for _, mode := range []string{"replaced-character", "player-gone"} {
		t.Run(mode, func(t *testing.T) {
			w, p, st := asyncEntryFixture(t)
			w.onEnterWorld(p.Session, enterWorldPacket(0))
			awaitSignal(t, st.entered, "sidecar read")
			if mode == "replaced-character" {
				p.Account.Chars[0].UID = "replaced-during-read"
			} else {
				delete(w.players, p.Session)
			}
			close(st.gate)
			w.handle(awaitEntryCommand(t, w))
			if p.InWorld || p.Char != nil {
				t.Fatal("a stale sidecar result entered the world")
			}
			if mode == "replaced-character" {
				pkt, ok := p.Session.DequeuePacketForTest()
				if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpMessagePanel {
					t.Fatal("stale entry did not tell the player to select again")
				}
			}
		})
	}
}
