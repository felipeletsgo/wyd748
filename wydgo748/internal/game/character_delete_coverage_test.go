package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

type failingCharStateStore struct {
	craftStore
	err error
}

func (s *failingCharStateStore) LoadCharState(string) (*model.CharState, error) { return nil, nil }
func (s *failingCharStateStore) SaveCharState(string, *model.CharState) error   { return s.err }

func TestDeleteCharacterRejectsInvalidContextAndMetadata(t *testing.T) {
	for _, tc := range []struct {
		name      string
		mutate    func(*World, *Player, *net.Session)
		packet    func() []byte
		wantReply bool
	}{
		{name: "unknown session", mutate: func(_ *World, _ *Player, s *net.Session) { _ = s }, wantReply: false},
		{name: "missing account", mutate: func(_ *World, p *Player, _ *net.Session) { p.Account = nil }, wantReply: false},
		{name: "already in world", mutate: func(_ *World, p *Player, _ *net.Session) { p.InWorld = true }, wantReply: false},
		{name: "wrong packet size", packet: func() []byte { return deleteCharacterPacket(0, "Felipe", deleteTestPassword)[:43] }, wantReply: false},
		{name: "slot out of range", packet: func() []byte { return deleteCharacterPacket(4, "Felipe", deleteTestPassword) }, wantReply: true},
		{name: "slot empty", mutate: func(_ *World, p *Player, _ *net.Session) { p.Account.Chars[0] = model.Char{} }, packet: func() []byte { return deleteCharacterPacket(0, "Felipe", deleteTestPassword) }, wantReply: true},
		{name: "name mismatch", packet: func() []byte { return deleteCharacterPacket(0, "Other", deleteTestPassword) }, wantReply: true},
		{name: "missing stored password", mutate: func(_ *World, p *Player, _ *net.Session) { p.Account.PasswordHash = "" }, packet: func() []byte { return deleteCharacterPacket(0, "Felipe", deleteTestPassword) }, wantReply: true},
		{name: "invalid stored password hash", mutate: func(_ *World, p *Player, _ *net.Session) { p.Account.PasswordHash = "invalid" }, packet: func() []byte { return deleteCharacterPacket(0, "Felipe", deleteTestPassword) }, wantReply: true},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, session := newDeleteWorld(t, model.Char{Name: "Felipe"})
			request := deleteCharacterPacket(0, "Felipe", deleteTestPassword)
			if tc.packet != nil {
				request = tc.packet()
			}
			if tc.mutate != nil {
				tc.mutate(w, p, session)
			}
			before := session.QueuedPacketsForTest()
			if tc.name == "unknown session" {
				w.onDeleteCharacter(net.NewTestSession(2, 64), request)
			} else {
				w.onDeleteCharacter(session, request)
			}
			if got := session.QueuedPacketsForTest() - before; (got > 0) != tc.wantReply {
				t.Fatalf("reply packets=%d, wantReply=%t", got, tc.wantReply)
			}
		})
	}
}

func TestDeleteCharacterRestoresSlotAfterStoreFailureAndToleratesSidecarFailure(t *testing.T) {
	t.Run("account write rejected", func(t *testing.T) {
		w, p, session := newDeleteWorld(t, model.Char{Name: "Felipe"})
		w.store.(*craftStore).err = errors.New("account write failed")
		w.onDeleteCharacter(session, deleteCharacterPacket(0, "Felipe", deleteTestPassword))
		if p.Account.Chars[0].Name != "Felipe" || !nameIsTaken(w, "Felipe") ||
			session.QueuedPacketsForTest() == 0 {
			t.Fatal("failed account write lost the character, name, or failure response")
		}
	})

	t.Run("derived sidecar cleanup fails after commit", func(t *testing.T) {
		w, p, session := newDeleteWorld(t, model.Char{Name: "Felipe", UID: "character-uid"})
		w.store = &failingCharStateStore{craftStore: craftStore{}, err: errors.New("sidecar delete failed")}
		w.onDeleteCharacter(session, deleteCharacterPacket(0, "Felipe", deleteTestPassword))
		if p.Account.Chars[0].Name != "" || nameIsTaken(w, "Felipe") || session.QueuedPacketsForTest() == 0 {
			t.Fatal("sidecar cleanup failure changed the committed character deletion")
		}
	})
}

func TestDeleteCharacterCriticalPersistenceSettlement(t *testing.T) {
	for _, tc := range []struct {
		name    string
		failure error
		admit   bool
	}{
		{name: "committed", admit: true},
		{name: "certified rejection", failure: criticalCertifiedRejection(t), admit: true},
		{name: "admission closed", admit: false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, session := newDeleteWorld(t, model.Char{Name: "Felipe", UID: "character-uid"})
			base := &criticalTestStore{entered: make(chan criticalWrite, 1), failure: tc.failure}
			backend := &criticalStateTestStore{&criticalBatchTestStore{base}}
			queue := newCriticalTestQueue(t, backend, 1, 2)
			if !tc.admit {
				queue.closeAdmission()
			}
			w.accountSessions = map[string]*net.Session{accountSessionKey(p.Account.Name): session}
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			w.onDeleteCharacter(session, deleteCharacterPacket(0, "Felipe", deleteTestPassword))
			if tc.admit {
				write := awaitCriticalWrite(t, base.entered)
				if len(write.accounts) != 1 || write.accounts[0].Chars[0].Name != "" ||
					write.character == nil || write.character.uid != "character-uid" || write.character.state != nil {
					t.Fatalf("delete transaction missing account or sidecar intent: %+v", write)
				}
				w.applyCriticalCompletion(awaitCriticalResult(t, queue))
			}
			deleted := tc.admit && tc.failure == nil
			if (p.Account.Chars[0].Name == "") != deleted || nameIsTaken(w, "Felipe") == deleted ||
				session.QueuedPacketsForTest() == 0 {
				t.Fatalf("deletion settlement inconsistent: char=%q nameTaken=%t packets=%d",
					p.Account.Chars[0].Name, nameIsTaken(w, "Felipe"), session.QueuedPacketsForTest())
			}
		})
	}
}
