package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/control"
	gameNet "wydgo/internal/net"
)

func TestClosedOwnerCannotReloadBeforeDisconnectCleanup(t *testing.T) {
	owner := newLoginTestSession(1, 64)
	other := newLoginTestSession(2, 64)
	st := &loginReloadStore{account: loginTestAccount()}
	w := loginResultWorld(st, other)
	w.claimAccountSession(owner, st.account.Name)
	live := loginTestAccount()
	live.Chars[0].Gold = 9000 // Not present in the last stored snapshot.
	w.players[owner] = &Player{Session: owner, Account: live, Char: &live.Chars[0], InWorld: true}
	owner.Close()
	// Exercise the production queue preparation/pop routines: the auth result
	// advances ahead of an older disconnect on another session's second turn.
	batch := []command{
		{s: owner, pkt: []byte{1}},
		{s: owner},
		{s: other, login: &loginResult{accountName: live.Name, account: loginTestAccount()}},
	}
	order := w.prepareCommandBatchQueues(batch)
	if len(order) != 2 || order[0] != owner || order[1] != other {
		t.Fatal("fixture did not create the expected per-session batch order")
	}
	if _, ok := w.popPreparedCommand(batch, owner); !ok {
		t.Fatal("first owner command missing")
	}
	login, ok := w.popPreparedCommand(batch, other)
	if !ok || login.login == nil {
		t.Fatal("reordered login result missing")
	}
	w.handle(login)
	if st.loads != 0 || w.players[other] != nil || w.accountSessions["felipe"] != owner {
		t.Fatal("relogin loaded stale storage while the closed owner still held live state")
	}
	if w.players[owner].Char.Gold != 9000 {
		t.Fatal("rejected relogin changed the old owner's unsaved state")
	}
	if cleanup, ok := w.popPreparedCommand(batch, owner); !ok || cleanup.pkt != nil || cleanup.login != nil {
		t.Fatal("owner disconnect was not retained for its next turn")
	}
	w.releaseCommandBatchScratch(batch, order)
}

func TestEnterWorldRejectsDuplicateUIDAndRepeatedEntry(t *testing.T) {
	for _, repeated := range []bool{false, true} {
		s := newLoginTestSession(2, 64)
		w := loginResultWorld(&loginReloadStore{}, s)
		account := loginTestAccount()
		account.Chars[0].UID = "same-character"
		p := &Player{ID: 2, Session: s, Account: account, CharSlot: -1, InWorld: repeated}
		w.players[s] = p
		owner := &Player{ID: 1, Account: account, Char: &account.Chars[0], InWorld: true}
		if !repeated {
			w.playersByID[owner.ID] = owner
		}
		pkt := make([]byte, characterLoginPacketSize)
		binary.LittleEndian.PutUint32(pkt[12:16], 0)
		w.onEnterWorld(s, pkt)
		if p.Char != nil || p.CharSlot != -1 || account.Chars[0].X != 999 {
			t.Fatal("duplicate/repeated entry mutated the character before rejection")
		}
	}
}

func TestShutdownDiscardsPoisonedAggregateIncludingHealthySibling(t *testing.T) {
	w := newZoneTestWorld()
	st := &atomicShutdownStore{}
	w.store = st
	account := newShutdownAccount("poisoned")
	account.Chars[0].UID = "poisoned-character"
	addShutdownPlayer(w, 1, account).PersistencePoisoned = true
	addShutdownPlayer(w, 2, account) // A sibling save would include poisoned RAM.
	healthy := newShutdownAccount("healthy")
	healthy.Chars[0].UID = "healthy-character"
	addShutdownPlayer(w, 3, healthy)
	done := make(chan error, 1)
	w.runShutdown(done)
	if err := <-done; err == nil {
		t.Fatal("discarded poisoned state was reported as a complete final snapshot")
	}
	if st.playerStateSaves != 1 || st.flushes != 1 {
		t.Fatalf("saves=%d flushes=%d: only the healthy aggregate should be saved", st.playerStateSaves, st.flushes)
	}
}

func TestMaintenanceClosesLateLoginAndStatusDoesNotExpireEvents(t *testing.T) {
	s := gameNet.NewTestSession(1, 16)
	w := loginResultWorld(&loginReloadStore{}, s)
	w.shuttingDown = true
	w.handle(command{s: s, login: &loginResult{account: loginTestAccount()}})
	if !s.IsClosed() || w.authPending[s] || w.players[s] != nil {
		t.Fatal("maintenance left a late login socket or authentication reservation alive")
	}
	expired := time.Unix(1, 0)
	w.quiz = &quizState{status: control.QuizStatus{Active: true, EndsAt: &expired}}
	w.globalDrop = &globalDropState{status: control.GlobalDropStatus{Active: true, EndsAt: &expired}}
	if !w.quizSnapshot().Active || !w.globalDropSnapshot().Active {
		t.Fatal("read-only maintenance status queries changed final event state")
	}
	w.shuttingDown = false
	if w.quizSnapshot().Active || w.globalDropSnapshot().Active {
		t.Fatal("normal status queries no longer expire overdue events")
	}
}

func TestCPRecoverySpreadsDueAccountsAndPreservesRollback(t *testing.T) {
	w, st := newShutdownWorld()
	now := time.Unix(1000, 0)
	for id := uint16(1); id <= 3; id++ {
		p := addShutdownPlayer(w, id, newShutdownAccount(fmt.Sprintf("cp-%d", id)))
		p.Char.CP = -5
		p.NextCPRecovery = now
	}
	for tick := 1; tick <= 3; tick++ {
		w.tickChaosRecovery(now)
		if st.saves != tick {
			t.Fatalf("tick %d saved %d accounts; expected exactly one additional save", tick, st.saves)
		}
		for id, p := range w.playersByID {
			expected := int16(-5)
			if int(id) <= tick {
				expected = -4
			}
			if p.Char.CP != expected {
				t.Fatalf("player %d CP=%d, expected %d", id, p.Char.CP, expected)
			}
		}
	}
	w.tickChaosRecovery(now)
	if st.saves != 3 {
		t.Fatal("CP recovery ran twice in the same interval")
	}
	failing := &craftStore{err: errors.New("injected commit failure")}
	w.store = failing
	w.tickChaosRecovery(now.Add(chaosRecoveryInterval))
	if failing.saves != 1 || w.playersByID[1].Char.CP != -4 {
		t.Fatal("failed CP commit did not restore the previous authoritative value")
	}
}

func TestRateLimitCardinalityRetainsOverflowAdmissionAndBlocksRepeatedKeys(t *testing.T) {
	states := make(map[string]*fixedWindowRate)
	now := time.Unix(1000, 0)
	for i := 0; i < 6; i++ {
		if ok, _ := allowFixedWindow(states, fmt.Sprintf("origin-%d", i), now, time.Minute, 2, 8); !ok {
			t.Fatal("exact-key capacity was exhausted prematurely")
		}
	}
	if ok, _ := allowFixedWindow(states, "new-origin", now, time.Minute, 2, 8); !ok {
		t.Fatal("cardinality cap caused a blanket refusal of new origins")
	}
	allowFixedWindow(states, "origin-0", now, time.Minute, 2, 8)
	if ok, _ := allowFixedWindow(states, "origin-0", now, time.Minute, 2, 8); ok {
		t.Fatal("overflow admission reset the limit of an existing origin")
	}
	for i := 0; i < 1000; i++ {
		allowFixedWindow(states, fmt.Sprintf("overflow-%d", i), now, time.Minute, 2, 8)
	}
	if len(states) > 8 {
		t.Fatalf("limiter cardinality grew to %d", len(states))
	}
	if ok, _ := allowFixedWindow(states, "new-origin", now.Add(time.Minute), time.Minute, 2, 8); !ok {
		t.Fatal("overflow window did not expire")
	}
}
