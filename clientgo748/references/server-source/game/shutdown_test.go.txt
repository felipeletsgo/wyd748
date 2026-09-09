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

type atomicShutdownStore struct {
	flushCraftStore
	playerStateSaves int
	instanceSaves    int
	playerStateErr   error
	instanceErr      error
}

func (s *atomicShutdownStore) SavePlayerState(_ *model.GuildRegistry, _ *model.Account,
	_ string, _ *model.CharState) error {
	s.playerStateSaves++
	return s.playerStateErr
}

func (s *atomicShutdownStore) SaveInstanceState(*model.InstanceStateSnapshot) error {
	s.instanceSaves++
	return s.instanceErr
}

func (s *atomicShutdownStore) Flush() { s.flushCraftStore.Flush() }

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
		Name:  name,
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 1000, CurHP: 900},
	}}}
}

func TestShutdownPersistsEachRuntimeCharacterAndFlushes(t *testing.T) {
	w, st := newShutdownWorld()
	shared := newShutdownAccount("dupla")
	// Dois personagens da MESMA conta compartilham o ponteiro: o desligamento
	// nao pode gravar o mesmo arquivo duas vezes.
	shared.Chars = append(shared.Chars, model.Char{
		UID:   "second-character",
		Name:  "segundo",
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 1000, CurHP: 900},
	})
	addShutdownPlayer(w, 1, shared)
	second := addShutdownPlayer(w, 2, shared)
	second.Char = &shared.Chars[1]
	addShutdownPlayer(w, 3, newShutdownAccount("outra"))

	done := make(chan error, 1)
	w.runShutdown(done)
	if err := <-done; err != nil {
		t.Fatalf("shutdown: %v", err)
	}

	if st.saves != 3 {
		t.Fatalf("saves=%d, quer 3 (cada personagem runtime precisa do seu charstate)", st.saves)
	}
	if st.flushes != 1 {
		t.Fatalf("flushes=%d, quer 1 (fila assincrona precisa drenar)", st.flushes)
	}
	if !w.shuttingDown {
		t.Fatal("mundo deveria ficar marcado como desligando")
	}
}

func TestShutdownUsesAtomicPlayerStateAndForcesDirtyInstanceState(t *testing.T) {
	st := &atomicShutdownStore{}
	w := newZoneTestWorld()
	w.store = st
	w.commands = make(chan command, 8)
	account := newShutdownAccount("atomic")
	account.Chars[0].UID = "atomic-character"
	addShutdownPlayer(w, 1, account)
	w.instanceStateDirty = true

	done := make(chan error, 1)
	w.runShutdown(done)
	if err := <-done; err != nil {
		t.Fatalf("shutdown atomico: %v", err)
	}
	if st.playerStateSaves != 1 || st.saves != 0 {
		t.Fatalf("conta/charstate nao foram atomicos: player=%d account=%d",
			st.playerStateSaves, st.saves)
	}
	if st.instanceSaves != 1 || w.instanceStateDirty {
		t.Fatalf("snapshot sujo nao foi drenado: saves=%d dirty=%v",
			st.instanceSaves, w.instanceStateDirty)
	}
}

func TestShutdownReportsDirtyInstancePersistenceFailure(t *testing.T) {
	st := &atomicShutdownStore{instanceErr: errors.New("instance database unavailable")}
	w := newZoneTestWorld()
	w.store = st
	w.commands = make(chan command, 8)
	w.instanceStateDirty = true

	done := make(chan error, 1)
	w.runShutdown(done)
	if err := <-done; err == nil {
		t.Fatal("shutdown ocultou falha do snapshot de instancias")
	}
	if st.instanceSaves != 1 || !w.instanceStateDirty {
		t.Fatalf("falha limpou dirty indevidamente: saves=%d dirty=%v",
			st.instanceSaves, w.instanceStateDirty)
	}
}

func TestShutdownIgnoresPlayersOutOfWorld(t *testing.T) {
	w, st := newShutdownWorld()
	p := addShutdownPlayer(w, 1, newShutdownAccount("fora"))
	p.InWorld = false // ainda na tela de selecao: nada a persistir

	done := make(chan error, 1)
	w.runShutdown(done)
	if err := <-done; err != nil {
		t.Fatalf("shutdown: %v", err)
	}

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

	done := make(chan error, 1)
	w.runShutdown(done)

	select {
	case err := <-done:
		if err == nil {
			t.Fatal("desligamento deveria reportar falha de save")
		}
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

	first := make(chan error, 1)
	w.runShutdown(first)
	if err := <-first; err != nil {
		t.Fatalf("primeiro shutdown: %v", err)
	}
	afterFirst := st.saves

	second := make(chan error, 1)
	w.runShutdown(second)
	if err := <-second; err != nil {
		t.Fatalf("segundo shutdown: %v", err)
	}

	if st.saves != afterFirst {
		t.Fatalf("segundo desligamento gravou de novo: %d -> %d", afterFirst, st.saves)
	}
}

func TestRepeatedShutdownPreservesFirstFailure(t *testing.T) {
	w, st := newShutdownWorld()
	st.err = errors.New("database unavailable")
	addShutdownPlayer(w, 1, newShutdownAccount("failed"))

	first := make(chan error, 1)
	w.runShutdown(first)
	if err := <-first; err == nil {
		t.Fatal("primeiro shutdown deveria falhar")
	}
	second := make(chan error, 1)
	w.runShutdown(second)
	if err := <-second; err == nil {
		t.Fatal("pedido repetido ocultou a falha do primeiro drain")
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

func TestShutdownReturnsFalseWhenFinalPersistenceFails(t *testing.T) {
	w, st := newShutdownWorld()
	st.err = errors.New("database unavailable")
	addShutdownPlayer(w, 1, newShutdownAccount("failure"))
	go func() {
		for cmd := range w.commands {
			w.safeHandle(cmd)
		}
	}()

	if w.Shutdown(2 * time.Second) {
		t.Fatal("shutdown com falha de persistencia retornou sucesso")
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
