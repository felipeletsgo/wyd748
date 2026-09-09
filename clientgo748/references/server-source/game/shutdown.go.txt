package game

import (
	"errors"
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
)

// shutdown.go -- desligamento controlado. Sem ele, um restart (deploy, systemd,
// queda de VPS) descarta o que estiver na fila de autosave: o jogador perde ate
// alguns segundos de progresso e reaparece com estado velho.
//
// O desligamento roda COMO UM COMANDO do game loop. Isso e deliberado: o World
// e dono unico do estado, entao gravar de fora exigiria trava ou correria com um
// handler em curso. Como comando, o drain ve um estado quieto e consistente.

// flushStore e o store que sabe drenar escritas assincronas pendentes.
type flushStore interface {
	Flush()
}

// Shutdown pede o desligamento e ESPERA o mundo terminar de persistir, ou ate o
// timeout. Devolve false se o loop nao respondeu a tempo -- nesse caso o
// chamador deve encerrar mesmo assim, mas o log dira que o drain ficou
// incompleto.
func (w *World) Shutdown(timeout time.Duration) bool {
	done := make(chan error, 1)
	select {
	case w.commands <- command{queuedAt: time.Now(), shutdown: done}:
	case <-time.After(timeout):
		log.Print("desligamento: game loop nao aceitou o comando (fila cheia)")
		return false
	}
	select {
	case err := <-done:
		if err != nil {
			log.Printf("desligamento: persistencia final incompleta: %v", err)
			return false
		}
		return true
	case <-time.After(timeout):
		log.Print("desligamento: tempo esgotado durante a persistencia final")
		return false
	}
}

// runShutdown persiste tudo e libera quem chamou Shutdown. Roda na goroutine do
// World.
func (w *World) runShutdown(done chan error) {
	finish := func(err error) {
		if done != nil {
			done <- err
			close(done)
		}
	}
	if w.shuttingDown {
		finish(w.shutdownErr)
		return // pedido repetido: o primeiro ja gravou tudo
	}
	// A partir daqui nenhuma conta nova entra: um login concluido depois do
	// snapshot final nao seria persistido.
	w.shuttingDown = true

	saved, failed := 0, 0
	var failures []error
	// A sessao normal permite um personagem por conta, mas fixtures e caminhos
	// administrativos podem representar mais de um. Cada UID precisa do proprio
	// charstate; deduplicar apenas pelo ponteiro da conta perderia os demais.
	type playerStateKey struct {
		account *model.Account
		uid     string
	}
	seen := make(map[playerStateKey]struct{}, len(w.players))
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Account == nil || p.Char == nil {
			continue
		}
		key := playerStateKey{account: p.Account, uid: p.Char.UID}
		if _, repeated := seen[key]; repeated {
			continue
		}
		seen[key] = struct{}{}
		// Save SINCRONO: o assincrono so enfileira, e a fila pode nao drenar a
		// tempo. No desligamento a durabilidade vale mais que a latencia.
		if err := w.saveAccountAndCharStateResult(p); err != nil {
			failed++
			log.Printf("desligamento: salvar conta %q: %v", p.Account.Name, err)
			failures = append(failures, fmt.Errorf("conta %q: %w", p.Account.Name, err))
			continue
		}
		saved++
	}
	if w.instanceStateDirty {
		if err := w.persistInstanceState(); err != nil {
			failed++
			failures = append(failures, fmt.Errorf("estado de instancias: %w", err))
			log.Printf("desligamento: salvar estado de instancias: %v", err)
		} else {
			w.instanceStateDirty = false
		}
	}

	// Drena o que o autosave deixou enfileirado antes do sinal chegar.
	if fs, ok := w.store.(flushStore); ok {
		fs.Flush()
	}
	log.Printf("desligamento: %d contas persistidas, %d falhas", saved, failed)
	w.shutdownErr = errors.Join(failures...)
	finish(w.shutdownErr)
}
