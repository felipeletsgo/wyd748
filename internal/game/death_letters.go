package game

import (
	"fmt"

	"wydgo/internal/wire"
)

// receiveDeathLetter registra uma carta antes de confirmar a morte no client.
// O limite impede que mortes repetidas façam o JSON da conta crescer sem fim.
func (w *World) receiveDeathLetter(victim *Player, killerName, killerKind string) {
	if victim == nil || victim.Session == nil || killerName == "" {
		return
	}
	victim.Session.Send(wire.MessageWhisper(0, killerName,
		fmt.Sprintf("!Voce foi morto por %s %q.", killerKind, killerName), 0))
}
