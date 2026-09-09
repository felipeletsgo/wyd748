package game

import (
	"encoding/binary"
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	guildInviteCooldown = 2 * time.Second
	guildInviteCost     = uint32(4_000_000)
	guildSubInviteCost  = uint32(100_000_000)
	guildMaxMembers     = 64_000
)

// onInviteGuild porta MSG_STANDARDPARM2/0x3D5 do client 7.48. A inclusao e
// direta, como no TMSrv: nao existe uma segunda confirmacao no protocolo. O
// comando textual /convidar continua usando convite pendente por compatibilidade
// com a UI antiga; este caminho e o botao nativo da janela de jogadores.
func (w *World) onInviteGuild(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || p.Account == nil || len(pkt) != 20 {
		return
	}
	header := wire.ParseHeader(pkt)
	if header.ID != p.ID {
		w.recordSecurityViolation(s, wire.OpInviteGuild, "ID do recrutador diverge da sessao")
		return
	}
	now := w.now()
	if !p.NextGuildInvite.IsZero() && now.Before(p.NextGuildInvite) {
		s.Send(wire.MessagePanel("Wait before trying another recruitment."))
		return
	}
	targetIDRaw := binary.LittleEndian.Uint32(pkt[12:16])
	inviteTypeRaw := binary.LittleEndian.Uint32(pkt[16:20])
	if targetIDRaw == 0 || targetIDRaw > uint32(^uint16(0)) || inviteTypeRaw > 3 {
		w.recordSecurityViolation(s, wire.OpInviteGuild, "alvo/tipo de recrutamento invalido")
		return
	}
	p.NextGuildInvite = now.Add(guildInviteCooldown)
	target := w.playerByID(uint16(targetIDRaw))
	if target == nil || !target.InWorld || target.Char == nil || target.Account == nil ||
		!w.playersShareGameplaySpace(p, target) {
		s.Send(wire.MessagePanel("That player is not online."))
		return
	}
	if target == p {
		s.Send(wire.MessagePanel("You cannot recruit yourself."))
		return
	}
	guild, actorMember := w.guildOf(p.Char)
	if guild == nil || actorMember == nil || actorMember.Rank == model.GuildRankMember {
		s.Send(wire.MessagePanel("Only a guild leader or sub-leader can recruit."))
		return
	}
	if inviteTypeRaw != 0 && actorMember.Rank != model.GuildRankLeader {
		s.Send(wire.MessagePanel("Only the guild leader can use this recruitment type."))
		return
	}
	if len(guild.Members) >= guildMaxMembers {
		s.Send(wire.MessagePanel("Your guild has reached its member limit."))
		return
	}
	if existing, _ := w.guildOf(target.Char); existing != nil || target.Char.GuildID != 0 {
		s.Send(wire.MessagePanel("That player already belongs to a guild."))
		return
	}
	if characterKingdom(p.Char) != characterKingdom(target.Char) ||
		(guild.Kingdom != model.KingdomNeutral && characterKingdom(target.Char) != guild.Kingdom) {
		s.Send(wire.MessagePanel("That player belongs to another kingdom."))
		return
	}
	if now.Weekday() == time.Sunday {
		s.Send(wire.MessagePanel("Guild recruitment is unavailable on Sunday."))
		return
	}
	cost := guildInviteCost
	if inviteTypeRaw != 0 {
		cost = guildSubInviteCost
	}
	if p.Char.Gold < cost {
		s.Send(wire.MessagePanel(fmt.Sprintf("You need %d gold to recruit.", cost)))
		return
	}

	guildSnapshot := w.snapshotGuilds()
	actorChar, targetChar := cloneCharacterState(p.Char), cloneCharacterState(target.Char)
	oldGold := p.Char.Gold
	guild.Members = append(guild.Members, model.GuildMember{
		Character: target.Char.Name,
		Account:   target.Account.Name,
		Rank:      model.GuildRankMember,
		JoinedAt:  now.UTC(),
	})
	p.Char.Gold -= cost
	target.Char.GuildID, target.Char.GuildRank = guild.ID, model.GuildRankMember
	if err := w.saveGuildState(p.Account, target.Account); err != nil {
		w.restoreGuilds(guildSnapshot)
		*p.Char, *target.Char = actorChar, targetChar
		p.Char.Gold = oldGold
		s.Send(wire.MessagePanel("Save failed. The recruitment was not applied."))
		log.Printf("[#%d] ERRO 0x3D5 recrutando %q: %v", s.ID, target.Char.Name, err)
		return
	}

	// GuildID faz parte da identidade materializada do jogador. Reenvie o
	// CreateMob somente nesta mudança de identidade; movimento continua usando
	// Action, sem recriar o avatar a cada tick.
	w.syncPlayerChaos(target)
	target.Session.Send(playerScorePacket(target))
	target.Session.Send(wire.UpdateEtc(target.ID, *target.Char))
	target.Session.Send(wire.MessagePanel(fmt.Sprintf("You joined the guild %s.", guild.Name)))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	s.Send(wire.MessagePanel(fmt.Sprintf("%s joined the guild.", target.Char.Name)))
	w.announceToGuild(guild.ID, fmt.Sprintf("%s joined the guild.", target.Char.Name), target)
	log.Printf("[#%d] 0x3D5 guild=%d recrutou %q tipo=%d custo=%d", s.ID, guild.ID, target.Char.Name, inviteTypeRaw, cost)
}
