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
	maxPartyMembers = 13 // lider + PartyList[12] do TMSrv
	partyInviteTTL  = 30 * time.Second
	partySectorSize = 128 // regra nativa: membros precisam estar no mesmo setor 128x128
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
		log.Printf("[#%d] PARTY convite invalido: pacote 0x37F com %d bytes", s.ID, len(pkt))
		return
	}
	target := w.playerByID(targetID)
	if target == nil || target == inviter || !target.InWorld || target.Char == nil ||
		!inView(inviter.X, inviter.Y, target.X, target.Y) {
		s.Send(wire.MessagePanel("Jogador indisponivel ou fora de alcance."))
		return
	}
	if target.Party != nil {
		s.Send(wire.MessagePanel("Esse jogador ja esta em um grupo."))
		return
	}
	if inviter.Party != nil {
		if inviter.Party.leader() != inviter {
			s.Send(wire.MessagePanel("Somente o lider pode convidar."))
			return
		}
		if len(inviter.Party.Members) >= maxPartyMembers {
			s.Send(wire.MessagePanel("O grupo esta cheio."))
			return
		}
	}

	target.InviteFrom = inviter.ID
	target.InviteUntil = time.Now().Add(partyInviteTTL)
	level, currentHP, maximumHP := wire.CompatibilityVitals(wireExtendedScore(inviter.Char))
	target.Session.Send(wire.PartyRequest(inviter.ID, inviter.Char.Name, inviter.Char.Class,
		level, currentHP, maximumHP, target.ID))
	log.Printf("[#%d] PARTY convite %s(%d) -> %s(%d)", s.ID,
		inviter.Char.Name, inviter.ID, target.Char.Name, target.ID)
}

// partyRequestTarget aceita o layout exato do TMSrv 7.54 (44B, DWORD@40) e o
// layout mais novo usado pelas sources 7.59/W2PP (48B, WORD@44). O servidor
// continua validando existencia, alcance e estado do alvo depois desta leitura.
func partyRequestTarget(pkt []byte) (uint16, bool) {
	if len(pkt) >= 48 {
		target := binary.LittleEndian.Uint16(pkt[44:46])
		if target != 0 {
			return target, true
		}
	}
	if len(pkt) < 44 {
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
		time.Now().After(member.InviteUntil) || !strings.EqualFold(name, leader.Char.Name) {
		member.InviteFrom = 0
		member.InviteUntil = time.Time{}
		s.Send(wire.MessagePanel("O convite de grupo expirou."))
		return
	}

	party := leader.Party
	if party == nil {
		party = &Party{Members: []*Player{leader}}
		leader.Party = party
	}
	if party.leader() != leader || len(party.Members) >= maxPartyMembers {
		s.Send(wire.MessagePanel("Nao foi possivel entrar nesse grupo."))
		return
	}
	party.Members = append(party.Members, member)
	member.Party = party
	member.InviteFrom = 0
	member.InviteUntil = time.Time{}
	w.syncParty(party)
	log.Printf("[#%d] PARTY %s entrou no grupo de %s (%d membros)", s.ID,
		member.Char.Name, leader.Char.Name, len(party.Members))
}

func (w *World) onPartyRemove(s *net.Session, pkt []byte) {
	requester := w.players[s]
	if requester == nil || requester.Party == nil || len(pkt) < 16 {
		return
	}
	targetID := uint16(binary.LittleEndian.Uint32(pkt[12:16]))
	target := requester
	if targetID != 0 && targetID != requester.ID {
		if requester.Party.leader() != requester {
			s.Send(wire.MessagePanel("Somente o lider pode remover membros."))
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
			level, currentHP, maximumHP := wire.CompatibilityVitals(wireExtendedScore(member.Char))
			receiver.Session.Send(wire.PartyMember(member.ID, member.Char.Name, member.Char.Class,
				byte(index), level, currentHP, maximumHP))
		}
	}
}

// O handler 0x37D do client remove o membro existente e o adiciona novamente no
// fim da lista. Enviar apenas o membro alterado faz a ordem girar a cada regen,
// dano ou ganho de level. Reenviar o snapshot inteiro na ordem canonica deixa o
// painel estavel; os pacotes seguem juntos na mesma fila TCP.
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
		// O prefixo anterior ao membro alterado ja esta na posicao correta.
		// Reenviar apenas o sufixo restaura a ordem com o minimo de pacotes.
		for slot := index; slot < len(party.Members); slot++ {
			candidate := party.Members[slot]
			if candidate == nil || candidate.Char == nil || !candidate.InWorld {
				continue
			}
			level, currentHP, maximumHP := wire.CompatibilityVitals(wireExtendedScore(candidate.Char))
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
		// O primeiro membro herda a lideranca. Limpar antes evita que o painel do
		// client mantenha o antigo lider destacado.
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

// partyExpShares concede a recompensa integral a cada membro elegivel e aplica
// o percentual configurado por participante. Somente membros vivos no mesmo
// setor 128x128 participam do bonus e recebem EXP.
func partyExpShares(killer *Player, reward, bonusPerMember uint32) []partyExpShare {
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
