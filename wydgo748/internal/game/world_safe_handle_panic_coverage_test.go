package game

import (
	"testing"

	"wydgo/internal/net"
)

func TestSafeHandleContainsActorCommandPanic(t *testing.T) {
	player, _ := networkedTestPlayer(1, "Player", 2100, 2100)
	w := worldWithNetworkedPlayers(player)
	before := metricPanicsTotal.Value()

	w.safeHandle(command{apply: func() { panic("test handler failure") }})

	if metricPanicsTotal.Value() != before+1 {
		t.Fatal("handler panic was not recorded")
	}
	if !w.shuttingDown || !player.PersistencePoisoned {
		t.Fatalf("panic did not fail the World closed: shutdown=%t poisoned=%t", w.shuttingDown, player.PersistencePoisoned)
	}
	if !player.Session.IsClosed() || player.Session.CloseCause() != net.CloseCausePanic {
		t.Fatalf("panic did not close the affected session: closed=%t cause=%v", player.Session.IsClosed(), player.Session.CloseCause())
	}
}
