package game

import (
	"encoding/binary"
	"log"
	"strings"
	"time"

	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	maxPartyMembers = 13 // Leader plus PartyList[12] in legacy TMSrv.
	partyInviteTTL  = 30 * time.Second
	partySectorSize = 128 // Eligible members must share the same 128x128 sector.
)

func (p *Party) leader() *Player {
	if p == nil || len(p.Members) == 0 {
		return nil
	}
	return p.Members[0]
}

func (p *Party) indexOf(member *Player) int {
	if p == nil || member == nil {
		return -1
	}
	for i, candidate := range p.Members {
		if candidate == member {
			return i
		}
	}
	return -1
}

func (w *World) onPartyRequest(s *net.Session, pkt []byte) {
	inviter := w.players[s]
	if inviter == nil || !inviter.InWorld || inviter.Char == nil {
		return
	}
	targetID, ok := partyRequestTarget(pkt)
	if !ok {
		log.Printf("[#%d] PARTY invalid invitation: rejected 0x37F packet (%d bytes)", s.ID, len(pkt))
		return
	}
	target := w.playerByID(targetID)
	if target == nil || target == inviter || !target.InWorld || target.Char == nil ||
		!w.playersShareGameplaySpace(inviter, target) ||
		!inView(inviter.X, inviter.Y, target.X, target.Y) {
		s.Send(wire.MessagePanel("Player unavailable or out of range."))
		return
	}
	if target.Party != nil {
		s.Send(wire.MessagePanel("That player is already in a party."))
		return
	}
	if inviter.Party != nil {
		if inviter.Party.leader() != inviter {
			s.Send(wire.MessagePanel("Only the leader can invite."))
			return
		}
		if len(inviter.Party.Members) >= maxPartyMembers {
			s.Send(wire.MessagePanel("The party is full."))
			return
		}
	}

	target.InviteFrom = inviter.ID
	target.InviteUntil = w.now().Add(partyInviteTTL)
	level, currentHP, maximumHP := playerLevel(inviter.Char), playerCurHP(inviter.Char), playerMaxHP(inviter.Char)
	target.Session.Send(wire.PartyRequest(inviter.ID, inviter.Char.Name, inviter.Char.Class,
		level, currentHP, maximumHP, target.ID))
	target.Session.Send(wire.MessageParameterized(-938, inviter.Char.Name))
	log.Printf("[#%d] PARTY invitation %s(%d) -> %s(%d)", s.ID,
		inviter.Char.Name, inviter.ID, target.Char.Name, target.ID)
}

// partyRequestTarget requires the 7.48 envelope (44 bytes, DWORD target at +40).
// Validate the complete target before narrowing to a live entity identity;
// the caller still validates target existence, range, and party state.
func partyRequestTarget(pkt []byte) (uint16, bool) {
	if len(pkt) != 44 {
		return 0, false
	}
	target := binary.LittleEndian.Uint32(pkt[40:44])
	if target == 0 || target > uint32(^uint16(0)) {
		return 0, false
	}
	return uint16(target), true
}

func (w *World) onPartyAccept(s *net.Session, pkt []byte) {
	member := w.players[s]
	if member == nil || !member.InWorld || member.Char == nil || member.Party != nil || len(pkt) < 30 {
		return
	}
	leaderID := binary.LittleEndian.Uint16(pkt[12:14])
	leader := w.playerByID(leaderID)
	name := cstr(pkt[14:30])
	if leader == nil || leader.Char == nil || member.InviteFrom != leaderID ||
		w.now().After(member.InviteUntil) || !strings.EqualFold(name, leader.Char.Name) ||
		!w.playersShareGameplaySpace(member, leader) {
		member.InviteFrom = 0
		member.InviteUntil = time.Time{}
		s.Send(wire.MessagePanel("The party invitation expired."))
		return
	}

	party := leader.Party
	if party == nil {
		party = &Party{Members: []*Player{leader}}
		leader.Party = party
	}
	if party.leader() != leader || len(party.Members) >= maxPartyMembers {
		s.Send(wire.MessagePanel("You could not join that party."))
		return
	}
	party.Members = append(party.Members, member)
	member.Party = party
	member.InviteFrom = 0
	member.InviteUntil = time.Time{}
	w.syncParty(party)
	log.Printf("[#%d] PARTY %s joined %s's party (%d members)", s.ID,
		member.Char.Name, leader.Char.Name, len(party.Members))
}

func (w *World) onPartyRemove(s *net.Session, pkt []byte) {
	requester := w.players[s]
	if requester == nil || requester.Party == nil || len(pkt) != 16 {
		return
	}
	// Zero remains a voluntary leave; high-word aliases must not become one.
	targetValue := binary.LittleEndian.Uint32(pkt[12:16])
	if targetValue > uint32(^uint16(0)) {
		return
	}
	targetID := uint16(targetValue)
	target := requester
	if targetID != 0 && targetID != requester.ID {
		if requester.Party.leader() != requester {
			s.Send(wire.MessagePanel("Only the leader can remove members."))
			return
		}
		target = w.playerByID(targetID)
		if target == nil || target.Party != requester.Party {
			return
		}
	}
	w.removePartyPlayer(target)
}

func (w *World) syncParty(party *Party) {
	if party == nil || len(party.Members) < 2 {
		return
	}
	for _, receiver := range party.Members {
		if receiver == nil || !receiver.InWorld {
			continue
		}
		for index, member := range party.Members {
			if member == nil || member.Char == nil || !member.InWorld {
				continue
			}
			level, currentHP, maximumHP := playerLevel(member.Char), playerCurHP(member.Char), playerMaxHP(member.Char)
			receiver.Session.Send(wire.PartyMember(member.ID, member.Char.Name, member.Char.Class,
				byte(index), level, currentHP, maximumHP))
		}
	}
}

// The client's 0x37D handler removes an existing member and appends it again.
// Sending only the changed member rotates the list on regeneration, damage,
// or level changes. Replaying the canonical suffix keeps the panel stable;
// its packets travel together through the same TCP queue.
func (w *World) updatePartyMember(member *Player) {
	if member == nil || member.Party == nil || member.Char == nil {
		return
	}
	index := member.Party.indexOf(member)
	if index < 0 {
		return
	}
	party := member.Party
	for _, receiver := range party.Members {
		if receiver == nil || !receiver.InWorld {
			continue
		}
		// The prefix before the changed member is already in the correct order.
		// Resending only the suffix restores order with the fewest packets.
		for slot := index; slot < len(party.Members); slot++ {
			candidate := party.Members[slot]
			if candidate == nil || candidate.Char == nil || !candidate.InWorld {
				continue
			}
			level, currentHP, maximumHP := playerLevel(candidate.Char), playerCurHP(candidate.Char), playerMaxHP(candidate.Char)
			receiver.Session.Send(wire.PartyMember(candidate.ID, candidate.Char.Name, candidate.Char.Class,
				byte(slot), level, currentHP, maximumHP))
		}
	}
}

func (w *World) removePartyPlayer(leaving *Player) {
	if leaving == nil || leaving.Party == nil {
		return
	}
	party := leaving.Party
	index := party.indexOf(leaving)
	if index < 0 {
		leaving.Party = nil
		return
	}
	wasLeader := index == 0
	party.Members = append(party.Members[:index], party.Members[index+1:]...)
	leaving.Party = nil
	if leaving.InWorld {
		leaving.Session.Send(wire.PartyRemove(0))
	}

	if len(party.Members) < 2 {
		for _, member := range party.Members {
			member.Party = nil
			if member.InWorld {
				member.Session.Send(wire.PartyRemove(0))
			}
		}
		return
	}
	if wasLeader {
		// The first remaining member inherits leadership. Clear the panel first
		// so the client does not keep highlighting the previous leader.
		for _, member := range party.Members {
			if member.InWorld {
				member.Session.Send(wire.PartyRemove(0))
			}
		}
	} else {
		for _, member := range party.Members {
			if member.InWorld {
				member.Session.Send(wire.PartyRemove(leaving.ID))
			}
		}
	}
	w.syncParty(party)
}

type partyExpShare struct {
	player *Player
	reward uint32
}

// partyExpShares gives each eligible member the full reward and applies the
// configured percentage per participant. Only living members in the same
// 128x128 sector contribute to the bonus and receive experience.
func partyExpShares(killer *Player, reward, bonusPerMember uint32) []partyExpShare {
	return partyExpSharesFiltered(killer, reward, bonusPerMember, nil)
}

// partyExpShares applies the same rules as the compatibility helper above,
// plus the World gameplay-space boundary used by real kill paths. A party
// member in another event runtime or in the public world never receives EXP
// from a kill it cannot observe.
func (w *World) partyExpShares(killer *Player, reward, bonusPerMember uint32) []partyExpShare {
	return partyExpSharesFiltered(killer, reward, bonusPerMember, func(member *Player) bool {
		return w.playersShareGameplaySpace(killer, member)
	})
}

func partyExpSharesFiltered(killer *Player, reward, bonusPerMember uint32,
	sameSpace func(*Player) bool) []partyExpShare {
	if killer == nil {
		return nil
	}
	eligible := []*Player{killer}
	if killer.Party != nil {
		eligible = eligible[:0]
		for _, member := range killer.Party.Members {
			if member == nil || !member.InWorld || member.Char == nil || playerCurHP(member.Char) == 0 {
				continue
			}
			if !canReceiveMortalExperience(member.Char) {
				continue
			}
			if sameSpace != nil && !sameSpace(member) {
				continue
			}
			if member.X/partySectorSize != killer.X/partySectorSize ||
				member.Y/partySectorSize != killer.Y/partySectorSize {
				continue
			}
			eligible = append(eligible, member)
		}
	}
	if killer.Party == nil && !canReceiveMortalExperience(killer.Char) {
		return nil
	}
	percent := uint64(100) + uint64(bonusPerMember)*uint64(len(eligible))
	boosted := uint64(reward) * percent / 100
	if boosted > uint64(^uint32(0)) {
		boosted = uint64(^uint32(0))
	}
	shares := make([]partyExpShare, 0, len(eligible))
	for _, member := range eligible {
		shares = append(shares, partyExpShare{player: member, reward: uint32(boosted)})
	}
	return shares
}
