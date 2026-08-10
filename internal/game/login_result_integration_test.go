package game

import (
	"errors"
	"net/netip"
	"testing"

	"wydgo/internal/account"
	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

type loginReloadStore struct {
	craftStore
	account *model.Account
	loadErr error
	loads   int
}

func (s *loginReloadStore) LoadAccount(string) (*model.Account, error) {
	s.loads++
	return s.account, s.loadErr
}

func loginResultWorld(st *loginReloadStore, session *gameNet.Session) *World {
	return &World{
		store:           st,
		players:         make(map[*gameNet.Session]*Player),
		playersByID:     make(map[uint16]*Player),
		accountSessions: make(map[string]*gameNet.Session),
		authPending:     map[*gameNet.Session]bool{session: true},
		guilds:          &model.GuildRegistry{Version: model.GuildRegistryVersion},
		items:           make(map[uint16]model.ItemDef),
		skills:          make(map[int]model.SkillDef),
	}
}

func loginTestAccount() *model.Account {
	return &model.Account{Name: "Felipe", Chars: []model.Char{{
		Name: "Hero", X: 999, Y: 999,
		Extended: testExtended(model.ExtendedScore{
			Version: model.ExtendedScoreVersion,
			Level:   10, MaxHP: 1000, CurHP: 1000, MaxMP: 500, CurMP: 500,
		}),
	}}}
}

func TestLoginResultReloadsLatestAccountAndCreatesSelectionPlayer(t *testing.T) {
	session := gameNet.NewTestSession(1, 64)
	fresh := loginTestAccount()
	st := &loginReloadStore{account: fresh}
	w := loginResultWorld(st, session)
	authSnapshot := loginTestAccount()
	authSnapshot.Chars[0].Gold = 1
	fresh.Chars[0].Gold = 999

	w.onLoginResult(session, &loginResult{accountName: "Felipe", account: authSnapshot})
	p := w.players[session]
	if p == nil || p.Account != fresh || p.CharSlot != -1 {
		t.Fatal("login nao criou Player com snapshot fresco")
	}
	if st.loads != 1 || p.Account.Chars[0].Gold != 999 {
		t.Fatal("login nao recarregou o estado anti-dupe")
	}
	if p.Account.Chars[0].X != playerEntryX || p.Account.Chars[0].Y != playerEntryY {
		t.Fatal("login nao fixou coordenada de reentrada")
	}
	if w.accountSessions["felipe"] != session || w.authPending[session] {
		t.Fatal("reserva/authPending inconsistentes depois do login")
	}
	if session.QueuedPacketsForTest() == 0 {
		t.Fatal("char-list nao foi enviada")
	}
}

func TestLoginResultRejectsAuthErrorDuplicateAndReloadFailure(t *testing.T) {
	t.Run("credenciais", func(t *testing.T) {
		session := gameNet.NewTestSession(1, 16)
		w := loginResultWorld(&loginReloadStore{}, session)
		w.onLoginResult(session, &loginResult{
			accountName: "Felipe", err: account.ErrInvalidCredentials,
		})
		if w.players[session] != nil || w.authPending[session] ||
			len(w.authClientsByIP) != 0 || session.QueuedPacketsForTest() == 0 {
			t.Fatal("credencial invalida nao foi recusada/avisada")
		}
	})

	t.Run("duplicada", func(t *testing.T) {
		session := gameNet.NewTestSession(2, 16)
		owner := gameNet.NewTestSession(1, 16)
		st := &loginReloadStore{account: loginTestAccount()}
		w := loginResultWorld(st, session)
		w.claimAccountSession(owner, "Felipe")
		w.onLoginResult(session, &loginResult{
			accountName: "Felipe", account: loginTestAccount(),
		})
		if st.loads != 0 || w.players[session] != nil ||
			len(w.authClientsByIP) != 0 || session.QueuedPacketsForTest() == 0 {
			t.Fatal("segunda sessao nao foi bloqueada antes do reload")
		}
	})

	t.Run("reload", func(t *testing.T) {
		session := gameNet.NewTestSession(1, 16)
		st := &loginReloadStore{loadErr: errors.New("disk")}
		w := loginResultWorld(st, session)
		w.onLoginResult(session, &loginResult{
			accountName: "Felipe", account: loginTestAccount(),
		})
		if st.loads != 1 || w.players[session] != nil ||
			len(w.accountSessions) != 0 || len(w.authClientsByIP) != 0 ||
			session.QueuedPacketsForTest() == 0 {
			t.Fatal("falha de reload nao liberou reserva/avisou o client")
		}
	})

	t.Run("limite de janelas", func(t *testing.T) {
		session := gameNet.NewTestSessionWithRemoteIP(5, 16, "198.51.100.30")
		st := &loginReloadStore{account: loginTestAccount()}
		w := loginResultWorld(st, session)
		w.operational.MaxAuthenticatedClientsPerIP = 4
		for id := int64(1); id <= 4; id++ {
			active := gameNet.NewTestSessionWithRemoteIP(id, 1, "198.51.100.30")
			if !w.claimAuthenticatedClientSlot(active) {
				t.Fatal("fixture nao conseguiu preencher as quatro vagas")
			}
		}
		w.onLoginResult(session, &loginResult{
			accountName: "Felipe", account: loginTestAccount(),
		})
		if st.loads != 0 || w.players[session] != nil ||
			len(w.accountSessions) != 0 || session.QueuedPacketsForTest() == 0 {
			t.Fatal("quinta janela nao foi recusada antes de carregar/publicar a conta")
		}
	})

	t.Run("rede bloqueada", func(t *testing.T) {
		session := gameNet.NewTestSessionWithRemoteIP(1, 16, "203.0.113.50")
		st := &loginReloadStore{account: loginTestAccount()}
		w := loginResultWorld(st, session)
		WithNetworkAdmission(model.NetworkAdmissionFile{
			Version: model.NetworkAdmissionVersion,
			Rules: []model.NetworkAdmissionRule{{
				CIDR: netip.MustParsePrefix("203.0.113.0/24"), Action: model.NetworkAdmissionDeny,
				Reason: "hosting provider",
			}},
		})(w)
		w.onLoginResult(session, &loginResult{accountName: "Felipe", account: loginTestAccount()})
		if st.loads != 0 || w.players[session] != nil || len(w.accountSessions) != 0 ||
			len(w.authClientsByIP) != 0 || session.QueuedPacketsForTest() == 0 {
			t.Fatal("rede bloqueada chegou ao reload/reserva da conta")
		}
	})

	t.Run("resultado obsoleto", func(t *testing.T) {
		session := gameNet.NewTestSession(1, 16)
		w := loginResultWorld(&loginReloadStore{}, session)
		delete(w.authPending, session)
		w.onLoginResult(session, &loginResult{account: loginTestAccount()})
		if session.QueuedPacketsForTest() != 0 {
			t.Fatal("resultado de auth cancelado foi processado")
		}
	})
}
