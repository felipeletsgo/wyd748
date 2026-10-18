package game

import (
	"strings"
	"time"

	"wydgo/internal/wire"
)

// Social parties are process-local. Stable character UIDs, not recycled actor
// IDs, reconnect to the original party while that exact encounter still exists.
type nightmarePartyReturn struct {
	party    *Party
	instance *ItemInstance
}

func (w *World) liveNightmare(inst *ItemInstance) bool {
	return inst != nil && strings.TrimSpace(inst.Config.NightmareTier) != "" &&
		w.itemInstances[inst.RuntimeID] == inst &&
		(inst.HardDeadline.IsZero() || w.now().Before(inst.HardDeadline)) &&
		(inst.ExitAt.IsZero() || w.now().Before(inst.ExitAt))
}

// Called only after the disconnect's instance membership has committed. A
// voluntary exit, character change or party kick does not create a return hint.
func (w *World) rememberNightmareParty(p *Player) bool {
	if p == nil || p.Char == nil || p.Party == nil || p.Party.closed || p.Party.indexOf(p) < 0 {
		return false
	}
	uid := strings.TrimSpace(p.Char.UID)
	if uid == "" {
		return false
	}
	for id, pending := range w.pendingInstanceMembers {
		if _, exists := pending[uid]; !exists || !w.liveNightmare(w.itemInstances[id]) {
			continue
		}
		if w.nightmarePartyReturns == nil {
			w.nightmarePartyReturns = make(map[string]nightmarePartyReturn)
		}
		w.nightmarePartyReturns[uid] = nightmarePartyReturn{p.Party, w.itemInstances[id]}
		return true
	}
	return false
}

// EnterWorld and actor visibility must precede the party identities and HUD.
// Replay only this character's current state, never restart the encounter.
func (w *World) restoreNightmareSession(p *Player) {
	if p == nil || !p.InWorld || p.Char == nil {
		return
	}
	inst := w.itemInstanceForPlayer(p.ID)
	if !w.liveNightmare(inst) {
		return
	}
	p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID,
		uint32(remainingInstanceSeconds(instanceCombatDeadline(inst), w.now()))))
	p.Session.Send(wire.StandardParm(wire.OpInstanceMobs, instanceSignalID, instanceDisplayedMobCount(inst)))
	w.restoreNightmareParty(p)
}

func (w *World) restoreNightmareParty(p *Player) {
	if p == nil || !p.InWorld || p.Char == nil {
		return
	}
	uid := strings.TrimSpace(p.Char.UID)
	hint, exists := w.nightmarePartyReturns[uid]
	if !exists {
		return
	}
	party := hint.party
	if !w.liveNightmare(hint.instance) || w.itemInstanceForPlayer(p.ID) != hint.instance ||
		party == nil || party.closed || p.Party != nil || len(party.Members) >= maxPartyMembers {
		delete(w.nightmarePartyReturns, uid)
		return
	}
	if w.criticalPartyReserved(p) {
		return
	}
	for _, member := range party.Members {
		if member == nil || !member.InWorld || member.Char == nil ||
			w.playersByID[member.ID] != member || member.Party != party {
			delete(w.nightmarePartyReturns, uid)
			return
		}
		if w.criticalPartyReserved(member) {
			return // Retry on World after the reserved operation completes.
		}
	}
	party.Members = append(party.Members, p)
	p.Party = party
	p.InviteFrom = 0
	p.InviteUntil = time.Time{}
	delete(w.nightmarePartyReturns, uid)
	w.syncParty(party)
}

func (w *World) tickNightmarePartyReturns() {
	for uid, hint := range w.nightmarePartyReturns {
		if !w.liveNightmare(hint.instance) || hint.party == nil || hint.party.closed {
			// Do not leave a one-member reconnect shell after its last offline
			// member loses the encounter. Respect concurrent party reservations.
			if party := hint.party; party != nil && len(party.Members) == 1 {
				if w.criticalPartyReserved(party.Members[0]) {
					continue
				}
				otherReturn := false
				for otherUID, other := range w.nightmarePartyReturns {
					if otherUID != uid && other.party == party {
						otherReturn = true
						break
					}
				}
				if !otherReturn && party.Members[0].Party == party {
					w.removePartyPlayer(party.Members[0])
				}
			}
			delete(w.nightmarePartyReturns, uid)
			continue
		}
		if p := w.playerByCharacterUID(uid); p != nil {
			w.restoreNightmareParty(p)
		}
	}
}
