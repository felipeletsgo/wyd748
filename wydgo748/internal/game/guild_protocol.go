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

// onInviteGuild handles MSG_STANDARDPARM2/0x3D5 from client 7.48. Recruitment
// is direct: this protocol has no second confirmation. The text invitation
// command retains pending invitations; this path handles the native UI button.
func (w *World) onInviteGuild(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || p.Account == nil || len(pkt) != 20 {
		return
	}
	if w.criticalGuildMutationReserved(p) {
		return
	}
	header := wire.ParseHeader(pkt)
	if header.ID != p.ID {
		w.recordSecurityViolation(s, wire.OpInviteGuild, "recruiter ID does not match session")
		return
	}
	now := w.now()
	if !p.NextGuildInvite.IsZero() && now.Before(p.NextGuildInvite) {
		s.Send(wire.MessagePanel(w.msg("_WG_Wait_Before_Trying_Another_Recruitment")))
		return
	}
	targetIDRaw := binary.LittleEndian.Uint32(pkt[12:16])
	inviteTypeRaw := binary.LittleEndian.Uint32(pkt[16:20])
	if targetIDRaw == 0 || targetIDRaw > uint32(^uint16(0)) || inviteTypeRaw > 3 {
		w.recordSecurityViolation(s, wire.OpInviteGuild, "invalid recruitment target or type")
		return
	}
	target := w.playerByID(uint16(targetIDRaw))
	if w.criticalGuildMutationReserved(target) {
		return
	}
	p.NextGuildInvite = now.Add(guildInviteCooldown)
	if target == nil || !target.InWorld || target.Char == nil || target.Account == nil ||
		!w.playersShareGameplaySpace(p, target) {
		s.Send(wire.MessagePanel(w.msg("_NN_Not_Connected")))
		return
	}
	if target == p {
		s.Send(wire.MessagePanel(w.msg("_WG_Cannot_Recruit_Yourself")))
		return
	}
	guild, actorMember := w.guildOf(p.Char)
	if guild == nil || actorMember == nil || actorMember.Rank == model.GuildRankMember {
		s.Send(wire.MessagePanel(w.msg("_WG_Only_Guild_Leader_Or_Sub_Leader")))
		return
	}
	if inviteTypeRaw != 0 && actorMember.Rank != model.GuildRankLeader {
		s.Send(wire.MessagePanel(w.msg("_WG_Only_Guild_Leader_Can_Use_Recruitment")))
		return
	}
	if len(guild.Members) >= guildMaxMembers {
		s.Send(wire.MessagePanel(w.msg("_WG_Guild_Has_Reached_Its_Member_Limit")))
		return
	}
	rank, rankFree := guildInviteRank(guild, inviteTypeRaw)
	if !rankFree {
		s.Send(wire.MessagePanel(w.msg("_WG_Sub_Leader_Slot_D_Already_Taken", inviteTypeRaw)))
		return
	}
	if existing, _ := w.guildOf(target.Char); existing != nil || target.Char.GuildID != 0 {
		s.Send(wire.MessagePanel(w.msg("_WG_Player_Already_Belongs_Guild")))
		return
	}
	if characterKingdom(p.Char) != characterKingdom(target.Char) ||
		characterKingdom(target.Char) != guild.Kingdom {
		s.Send(wire.MessagePanel(w.msg("_WG_Player_Belongs_Another_Kingdom")))
		return
	}
	if now.Weekday() == time.Sunday {
		s.Send(wire.MessagePanel(w.msg("_WG_Guild_Recruitment_Unavailable_Sunday")))
		return
	}
	cost := guildInviteCost
	if inviteTypeRaw != 0 {
		cost = guildSubInviteCost
	}
	if p.Char.Gold < cost {
		s.Send(wire.MessagePanel(w.msg("_WG_Need_D_Gold_Recruit", cost)))
		return
	}
	if w.criticalWrites != nil {
		if err := w.queueGuildJoin(p, target, guild, cost, inviteTypeRaw, false); err != nil {
			s.Send(wire.MessagePanel(w.msg("_WG_Guild_Persistence_Unavailable_Try_Again_Later")))
			log.Printf("[#%d] ERROR 0x3D5 admission: %v", s.ID, err)
		}
		return
	}

	guildSnapshot := w.snapshotGuilds()
	actorChar, targetChar := cloneCharacterState(p.Char), cloneCharacterState(target.Char)
	oldGold := p.Char.Gold
	guild.Members = append(guild.Members, model.GuildMember{
		Character: target.Char.Name,
		Account:   target.Account.Name,
		Rank:      rank,
		JoinedAt:  now.UTC(),
	})
	p.Char.Gold -= cost
	target.Char.GuildID, target.Char.GuildRank = guild.ID, rank
	if err := w.saveGuildState(p.Account, target.Account); err != nil {
		w.restoreGuilds(guildSnapshot)
		*p.Char, *target.Char = actorChar, targetChar
		p.Char.Gold = oldGold
		s.Send(wire.MessagePanel(w.msg("_WG_Save_Failed_Recruitment_Was_Not_Applied")))
		log.Printf("[#%d] ERROR 0x3D5 recruiting %q: %v", s.ID, target.Char.Name, err)
		return
	}

	// GuildID is part of the published player identity. Refresh it only for
	// this identity change; movement keeps using Action without recreating it.
	w.syncPlayerChaos(target)
	target.Session.Send(playerScorePacket(target))
	target.Session.Send(wire.UpdateEtc(target.ID, *target.Char))
	target.Session.Send(wire.MessagePanel(w.msgFor(target, "_WG_Joined_Guild_S", guild.Name)))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	s.Send(wire.MessagePanel(w.msg("_WG_S_Joined_Guild", target.Char.Name)))
	w.announceToGuild(guild.ID, fmt.Sprintf("%s joined the guild.", target.Char.Name), target)
	log.Printf("[#%d] 0x3D5 guild=%d recruited %q type=%d cost=%d", s.ID, guild.ID, target.Char.Name, inviteTypeRaw, cost)
}

// guildInviteRank maps the 0x3D5 InviteType: 0 recruits a member, 1..3
// recruit directly into the Sub1/Sub2/Sub3 slot (ranks 6..8) for the 100M
// leader-only cost. A taken sub-leader slot rejects the recruitment.
func guildInviteRank(guild *model.Guild, inviteType uint32) (byte, bool) {
	if inviteType == 0 {
		return model.GuildRankMember, true
	}
	if guild == nil || inviteType > model.MaxGuildSubLeaders {
		return 0, false
	}
	rank := byte(model.GuildRankSubFirst + inviteType - 1)
	for _, member := range guild.Members {
		if member.Rank == rank {
			return 0, false
		}
	}
	return rank, true
}
