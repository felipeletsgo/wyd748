package game

import (
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
	done := make(chan struct{})
	select {
	case w.commands <- command{shutdown: done}:
	case <-time.After(timeout):
		log.Print("desligamento: game loop nao aceitou o comando (fila cheia)")
		return false
	}
	select {
	case <-done:
		return true
	case <-time.After(timeout):
		log.Print("desligamento: tempo esgotado durante a persistencia final")
		return false
	}
}

// runShutdown persiste tudo e libera quem chamou Shutdown. Roda na goroutine do
// World.
func (w *World) runShutdown(done chan struct{}) {
	defer close(done)
	if w.shuttingDown {
		return // pedido repetido: o primeiro ja gravou tudo
	}
	// A partir daqui nenhuma conta nova entra: um login concluido depois do
	// snapshot final nao seria persistido.
	w.shuttingDown = true

	saved, failed := 0, 0
	// Deduplica por conta: dois personagens da mesma conta compartilham o
	// ponteiro e gravariam o mesmo arquivo duas vezes.
	seen := make(map[*model.Account]struct{}, len(w.players))
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Account == nil || p.Char == nil {
			continue
		}
		if _, repeated := seen[p.Account]; repeated {
			continue
		}
		seen[p.Account] = struct{}{}
		// Save SINCRONO: o assincrono so enfileira, e a fila pode nao drenar a
		// tempo. No desligamento a durabilidade vale mais que a latencia.
		if err := w.saveAccount(p.Account); err != nil {
			failed++
			log.Printf("desligamento: salvar conta %q: %v", p.Account.Name, err)
			continue
		}
		saved++
		// Buffs e moedas vivem no sidecar de sessao, gravado junto.
		w.saveCharState(p)
	}

	// Drena o que o autosave deixou enfileirado antes do sinal chegar.
	if fs, ok := w.store.(flushStore); ok {
		fs.Flush()
	}
	log.Printf("desligamento: %d contas persistidas, %d falhas", saved, failed)
}
