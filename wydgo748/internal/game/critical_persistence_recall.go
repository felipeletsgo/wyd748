package game

import (
	"log"

	"wydgo/internal/wire"
)

// Returns admission, not durable success. All revival and visibility effects
// belong to the actor acknowledgement; a rejection leaves the corpse and room.
func (w *World) queueCriticalRecall(p *Player, reason string) bool {
	if w.criticalTradeReserved(p) {
		return false
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	dead := playerCurHP(ch) == 0
	if dead {
		hp := uint32(200)
		if maximum := playerMaxHP(ch); maximum < hp {
			hp = maximum
		}
		setPlayerCurHP(ch, hp)
	}
	city, x, y := w.recallSpawn(ch)
	x, y = w.findFreePlayerPosition(x, y, 3, p)
	err := w.queuePlayerTeleportSnapshot(p, proposed, x, y, "recall "+reason, func() {
		p.Session.Send(playerScorePacket(p))
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		w.syncPlayerVitals(p)
	}, func() {
		w.updatePartyMember(p)
		log.Printf("[#%d] recall (%s) -> (%d,%d), bound city %s[%d], revived=%v",
			p.ID, reason, p.X, p.Y, cityWarZones[city].name, city, dead)
	}, nil, dead, nil, true)
	if err != nil {
		log.Printf("[#%d] recall (%s) admission rejected: %v", p.ID, reason, err)
	}
	return err == nil
}
