package game

import (
	"testing"

	"wydgo/internal/model"
	netpkg "wydgo/internal/net"
)

func TestAccountSessionIsExclusiveCaseInsensitive(t *testing.T) {
	w := &World{}
	first := &netpkg.Session{ID: 1}
	second := &netpkg.Session{ID: 2}
	if !w.claimAccountSession(first, "Felipe") {
		t.Fatal("primeira sessao deveria reservar a conta")
	}
	if w.claimAccountSession(second, "felipe") {
		t.Fatal("segunda sessao reservou a mesma conta com outra capitalizacao")
	}
	if !w.claimAccountSession(first, "FELIPE") {
		t.Fatal("a sessao proprietaria perdeu a propria reserva")
	}
}

func TestAccountSessionReleaseOnlyByOwner(t *testing.T) {
	w := &World{}
	owner := &netpkg.Session{ID: 1}
	other := &netpkg.Session{ID: 2}
	acc := &model.Account{Name: "felipe"}
	w.claimAccountSession(owner, acc.Name)
	w.releaseAccountSession(other, acc)
	if w.claimAccountSession(other, acc.Name) {
		t.Fatal("sessao alheia liberou a conta do proprietario")
	}
	w.releaseAccountSession(owner, acc)
	if !w.claimAccountSession(other, acc.Name) {
		t.Fatal("conta permaneceu presa depois da desconexao do proprietario")
	}
}

func TestAccountSessionReclaimsClosedSocketBeforeDisconnectCommand(t *testing.T) {
	w := &World{}
	owner := netpkg.NewTestSession(1, 1)
	other := netpkg.NewTestSession(2, 1)
	if !w.claimAccountSession(owner, "Felipe") {
		t.Fatal("sessao inicial nao reservou a conta")
	}
	owner.Close() // simula socket encerrado antes do comando nil chegar ao World
	if !w.claimAccountSession(other, "felipe") {
		t.Fatal("reserva de socket morto bloqueou o novo login")
	}
}
