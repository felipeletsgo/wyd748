package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

// flushCraftStore adiciona o drain ao store de teste, para verificar que o
// desligamento realmente pede o flush da fila assincrona.
type flushCraftStore struct {
	craftStore
	flushes int
}

func (s *flushCraftStore) Flush() { s.flushes++ }

func newShutdownWorld() (*World, *flushCraftStore) {
	st := &flushCraftStore{}
	w := newZoneTestWorld()
	w.store = st
	w.commands = make(chan command, 8)
	return w, st
}

func addShutdownPlayer(w *World, id uint16, account *model.Account) *Player {
	session := net.NewTestSession(int64(id), 64)
	p := &Player{ID: id, Session: session, Account: account, Char: &account.Chars[0],
		InWorld: true, X: 2100, Y: 2100, Visible: map[uint16]struct{}{}}
	w.players[session] = p
	w.playersByID[id] = p
	return p
}

func newShutdownAccount(name string) *model.Account {
	return &model.Account{Name: name, Chars: []model.Char{{
		Name:     name,
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 1000, CurHP: 900},
	}}}
}

func TestShutdownPersistsEachAccountOnceAndFlushes(t *testing.T) {
	w, st := newShutdownWorld()
	shared := newShutdownAccount("dupla")
	// Dois personagens da MESMA conta compartilham o ponteiro: o desligamento
	// nao pode gravar o mesmo arquivo duas vezes.
	shared.Chars = append(shared.Chars, model.Char{
		Name:     "segundo",
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 1000, CurHP: 900},
	})
	addShutdownPlayer(w, 1, shared)
	second := addShutdownPlayer(w, 2, shared)
	second.Char = &shared.Chars[1]
	addShutdownPlayer(w, 3, newShutdownAccount("outra"))

	done := make(chan struct{})
	w.runShutdown(done)
	<-done

	if st.saves != 2 {
		t.Fatalf("saves=%d, quer 2 (uma por CONTA, nao por personagem)", st.saves)
	}
	if st.flushes != 1 {
		t.Fatalf("flushes=%d, quer 1 (fila assincrona precisa drenar)", st.flushes)
	}
	if !w.shuttingDown {
		t.Fatal("mundo deveria ficar marcado como desligando")
	}
}

func TestShutdownIgnoresPlayersOutOfWorld(t *testing.T) {
	w, st := newShutdownWorld()
	p := addShutdownPlayer(w, 1, newShutdownAccount("fora"))
	p.InWorld = false // ainda na tela de selecao: nada a persistir

	done := make(chan struct{})
	w.runShutdown(done)
	<-done

	if st.saves != 0 {
		t.Fatalf("saves=%d, quer 0 para jogador fora do mundo", st.saves)
	}
}

// TestShutdownSurvivesSaveFailure garante que uma conta com disco ruim nao
// impede as demais de serem gravadas nem trava o desligamento.
func TestShutdownSurvivesSaveFailure(t *testing.T) {
	w, st := newShutdownWorld()
	st.err = errors.New("disco indisponivel")
	addShutdownPlayer(w, 1, newShutdownAccount("falha"))

	done := make(chan struct{})
	w.runShutdown(done)

	select {
	case <-done:
	case <-time.After(time.Second):
		t.Fatal("desligamento travou apos falha de save")
	}
	if st.flushes != 1 {
		t.Fatalf("flushes=%d, quer 1 mesmo com falha de save", st.flushes)
	}
}

func TestShutdownIsIdempotent(t *testing.T) {
	w, st := newShutdownWorld()
	addShutdownPlayer(w, 1, newShutdownAccount("unica"))

	first := make(chan struct{})
	w.runShutdown(first)
	<-first
	afterFirst := st.saves

	second := make(chan struct{})
	w.runShutdown(second)
	<-second

	if st.saves != afterFirst {
		t.Fatalf("segundo desligamento gravou de novo: %d -> %d", afterFirst, st.saves)
	}
}

// TestShutdownRunsThroughGameLoop cobre o caminho real: Shutdown enfileira um
// comando e espera o loop responder.
func TestShutdownRunsThroughGameLoop(t *testing.T) {
	w, st := newShutdownWorld()
	addShutdownPlayer(w, 1, newShutdownAccount("loop"))

	// Um consumidor minimo no lugar de Run(), que rodaria ticks para sempre.
	go func() {
		for cmd := range w.commands {
			w.safeHandle(cmd)
		}
	}()

	if !w.Shutdown(2 * time.Second) {
		t.Fatal("Shutdown deveria concluir dentro do timeout")
	}
	if st.saves != 1 {
		t.Fatalf("saves=%d, quer 1", st.saves)
	}
}

func TestLoginRefusedWhileShuttingDown(t *testing.T) {
	w, _ := newShutdownWorld()
	w.shuttingDown = true
	session := net.NewTestSession(99, 8)

	// Pacote de login valido em tamanho; o gate deve barrar antes de autenticar.
	w.onLogin(session, make([]byte, 44))

	if w.authPending[session] {
		t.Fatal("login nao pode iniciar autenticacao durante o desligamento")
	}
	if session.QueuedPacketsForTest() == 0 {
		t.Fatal("jogador deveria receber aviso de manutencao")
	}
}
