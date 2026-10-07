package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// guildStore exposes transactional guild/account persistence through a minimal
// optional interface, following the tradeBatchStore pattern.
type guildStore interface {
	LoadGuilds() (*model.GuildRegistry, error)
	SaveGameState(guilds *model.GuildRegistry, accounts ...*model.Account) error
}

// guildInviteTTL follows party invitations: forgotten invitations must expire.
const guildInviteTTL = 30 * time.Second

// saveGuildState saves the registry and affected accounts in one transaction.
// An unavailable transaction is rejected rather than persisting half a change.
func (w *World) saveGuildState(accounts ...*model.Account) error {
	if w.criticalGuildReserved() {
		return errCriticalPersistenceBusy
	}
	if err := w.syncPersistenceAllowed("guild state"); err != nil {
		return err
	}
	gs, ok := w.store.(guildStore)
	if !ok {
		return fmt.Errorf("current store does not support guild persistence")
	}
	warsBefore := w.guilds.Wars.Clone()
	w.pruneGuildWarReferences()
	err := gs.SaveGameState(w.guilds, accountPersistenceSnapshots(accounts...)...)
	if err != nil {
		w.guilds.Wars = warsBefore
	}
	return err
}

// Authorization errors are separate from presentation for tests without a session.
var (
	errGuildMissing   = errors.New("character does not belong to a guild")
	errGuildNotLeader = errors.New("action allowed only for the leader")
)

// requireGuildLeader resolves membership and requires leadership. Invite and
// expel commands share this rule and translate its errors into panel messages.
func (w *World) requireGuildLeader(ch *model.Char) (*model.Guild, error) {
	guild, member := w.guildOf(ch)
	if guild == nil {
		return nil, errGuildMissing
	}
	if member.Rank != model.GuildRankLeader {
		return nil, errGuildNotLeader
	}
	return guild, nil
}

// guildAuthMessage translates authorization errors into panel text.
func guildAuthMessage(err error, action string) string {
	if errors.Is(err, errGuildNotLeader) {
		return fmt.Sprintf("Only the leader can %s.", action)
	}
	return "You do not belong to a guild."
}

// guildOf resolves membership through the canonical registry.
func (w *World) guildOf(ch *model.Char) (*model.Guild, *model.GuildMember) {
	if w.guilds == nil || ch == nil || ch.Name == "" {
		return nil, nil
	}
	return w.guilds.FindByCharacter(ch.Name)
}

// repairGuildState reconciles denormalized character fields at login with the
// canonical registry, including offline dissolution or rank changes. It returns
// whether the caller needs to persist the repair.
func (w *World) repairGuildState(ch *model.Char) bool {
	if ch == nil || ch.Name == "" {
		return false
	}
	guild, member := w.guildOf(ch)
	if guild == nil {
		if ch.GuildID == 0 && ch.GuildRank == 0 {
			return false
		}
		log.Printf("guild: %q referenced missing guild %d; state cleared",
			ch.Name, ch.GuildID)
		ch.GuildID, ch.GuildRank = 0, 0
		return true
	}
	if ch.GuildID == guild.ID && ch.GuildRank == member.Rank {
		return false
	}
	log.Printf("guild: %q repaired to guild=%d rank=%d (previous guild=%d rank=%d)",
		ch.Name, guild.ID, member.Rank, ch.GuildID, ch.GuildRank)
	ch.GuildID, ch.GuildRank = guild.ID, member.Rank
	return true
}

// onlineGuildMembers returns connected players belonging to the given guild.
func (w *World) onlineGuildMembers(guildID uint16) []*Player {
	if guildID == 0 {
		return nil
	}
	var members []*Player
	for _, p := range w.players {
		if p.InWorld && p.Char != nil && p.Char.GuildID == guildID {
			members = append(members, p)
		}
	}
	return members
}

// publishGuildIdentity sends only the updates needed after GuildID/GuildLevel
// changes. Guild mark/rank changes must not recreate movement or coordinates.
func (w *World) publishGuildIdentity(p *Player) {
	if p == nil || p.Char == nil || !p.InWorld || p.Session == nil {
		return
	}
	w.syncPlayerChaos(p)
	p.Session.Send(playerScorePacket(p))
	p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
}

// sendGuildChat delivers '-' channel text only to online members. Preserve the
// prefix: the client uses its first character for color and text offset.
func (w *World) sendGuildChat(sender *Player, message string) {
	if sender == nil || sender.Char == nil || message == "" {
		return
	}
	guild, _ := w.guildOf(sender.Char)
	if guild == nil {
		sender.Session.Send(wire.MessagePanel("You do not belong to a guild."))
		log.Printf("[#%d] guild chat rejected: %q does not belong to a guild",
			sender.Session.ID, sender.Char.Name)
		return
	}
	delivered := 0
	for _, member := range w.onlineGuildMembers(guild.ID) {
		if member == sender {
			continue // The sender's client already inserted the local line.
		}
		member.Session.Send(wire.MessageWhisper(0, sender.Char.Name, message, 3))
		delivered++
	}
	// Record delivery count; the sender's local line alone does not prove delivery.
	log.Printf("[#%d] GUILD chat %q -> guild %d (%d online member(s) excluding sender)",
		sender.Session.ID, sender.Char.Name, guild.ID, delivered)
}

// ---------------------------------------------------------------------------
// Commands
// ---------------------------------------------------------------------------

func (w *World) guildCommandCreate(s *net.Session, p *Player, arg string) {
	if w.criticalGuildMutationReserved(p) {
		return
	}
	name := strings.TrimSpace(arg)
	if name == "" {
		s.Send(wire.MessagePanel("Usage: /create <guild name>"))
		return
	}
	if err := model.ValidateGuildName(name); err != nil {
		s.Send(wire.MessagePanel("Invalid guild name."))
		log.Printf("[#%d] /create rejected: %v", s.ID, err)
		return
	}
	if guild, _ := w.guildOf(p.Char); guild != nil {
		s.Send(wire.MessagePanel("You already belong to a guild."))
		return
	}
	if w.guilds.FindByName(name) != nil {
		s.Send(wire.MessagePanel("A guild with that name already exists."))
		return
	}
	id, err := w.guilds.NextGuildID()
	if err != nil {
		s.Send(wire.MessagePanel("Guild limit reached."))
		log.Printf("[#%d] /create: %v", s.ID, err)
		return
	}
	if w.criticalWrites != nil {
		if err := w.queueGuildCreate(p, name); err != nil {
			s.Send(wire.MessagePanel("Save failed. The guild was not created."))
			log.Printf("[#%d] ERROR /create admission %q: %v", s.ID, name, err)
		}
		return
	}

	previousGuilds := w.guilds.Guilds
	oldID, oldRank := p.Char.GuildID, p.Char.GuildRank
	w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{
		ID:        id,
		Name:      name,
		Kingdom:   characterKingdom(p.Char),
		CreatedAt: w.now().UTC(),
		Members: []model.GuildMember{{
			Character: p.Char.Name,
			Account:   p.Account.Name,
			Rank:      model.GuildRankLeader,
			JoinedAt:  w.now().UTC(),
		}},
	})
	p.Char.GuildID, p.Char.GuildRank = id, model.GuildRankLeader

	if err := w.saveGuildState(p.Account); err != nil {
		w.guilds.Guilds = previousGuilds
		p.Char.GuildID, p.Char.GuildRank = oldID, oldRank
		s.Send(wire.MessagePanel("Save failed. The guild was not created."))
		log.Printf("[#%d] ERROR /create %q: %v", s.ID, name, err)
		return
	}
	s.Send(wire.MessagePanel(fmt.Sprintf("Guild %q created.", name)))
	w.publishGuildIdentity(p)
	log.Printf("[#%d] GUILD created id=%d %q by %q", s.ID, id, name, p.Char.Name)
}

func (w *World) guildCommandInvite(s *net.Session, p *Player, arg string) {
	if w.criticalGuildMutationReserved(p) {
		return
	}
	target := strings.TrimSpace(arg)
	if target == "" {
		s.Send(wire.MessagePanel("Usage: /invite <name>"))
		return
	}
	guild, err := w.requireGuildLeader(p.Char)
	if err != nil {
		s.Send(wire.MessagePanel(guildAuthMessage(err, "invite")))
		return
	}
	if strings.EqualFold(target, p.Char.Name) {
		s.Send(wire.MessagePanel("You are already in the guild."))
		return
	}
	// Without a character/account index, only online players can be invited.
	// Offline invitations would require scanning every account.
	invited := w.playerByCharacterName(target)
	if invited == nil || invited.Char == nil {
		s.Send(wire.MessagePanel("That player is not online."))
		return
	}
	if other, _ := w.guildOf(invited.Char); other != nil {
		s.Send(wire.MessagePanel("That player already belongs to a guild."))
		return
	}
	if w.criticalGuildMutationReserved(invited) {
		return
	}
	if characterKingdom(invited.Char) != guild.Kingdom {
		s.Send(wire.MessagePanel("That player belongs to another kingdom."))
		return
	}
	invited.GuildInviteFrom = guild.ID
	invited.GuildInviteUntil = w.now().Add(guildInviteTTL)
	invited.Session.Send(wire.MessagePanel(fmt.Sprintf(
		"%s invited you to the guild %s. Type /accept.", p.Char.Name, guild.Name)))
	s.Send(wire.MessagePanel(fmt.Sprintf("Invitation sent to %s.", invited.Char.Name)))
	log.Printf("[#%d] GUILD invitation %q -> %q (guild %d)", s.ID, p.Char.Name, invited.Char.Name, guild.ID)
}

func (w *World) guildCommandAccept(s *net.Session, p *Player, _ string) {
	if w.criticalGuildMutationReserved(p) {
		return
	}
	if p.GuildInviteFrom == 0 || w.now().After(p.GuildInviteUntil) {
		p.GuildInviteFrom = 0
		s.Send(wire.MessagePanel("You have no valid guild invitation."))
		return
	}
	if guild, _ := w.guildOf(p.Char); guild != nil {
		p.GuildInviteFrom = 0
		s.Send(wire.MessagePanel("You already belong to a guild."))
		return
	}
	guild := w.guilds.FindByID(p.GuildInviteFrom)
	if guild == nil {
		p.GuildInviteFrom = 0
		s.Send(wire.MessagePanel("That guild no longer exists."))
		return
	}
	if characterKingdom(p.Char) != guild.Kingdom {
		p.GuildInviteFrom = 0
		s.Send(wire.MessagePanel("You do not belong to this guild's kingdom."))
		return
	}
	if w.criticalWrites != nil {
		if err := w.queueGuildJoin(p, p, guild, 0, 0, true); err != nil {
			s.Send(wire.MessagePanel("Guild persistence is unavailable. Try again later."))
			log.Printf("[#%d] ERROR /accept admission: %v", s.ID, err)
		}
		return
	}

	previousMembers := append([]model.GuildMember(nil), guild.Members...)
	oldID, oldRank := p.Char.GuildID, p.Char.GuildRank
	guild.Members = append(guild.Members, model.GuildMember{
		Character: p.Char.Name,
		Account:   p.Account.Name,
		Rank:      model.GuildRankMember,
		JoinedAt:  w.now().UTC(),
	})
	p.Char.GuildID, p.Char.GuildRank = guild.ID, model.GuildRankMember

	if err := w.saveGuildState(p.Account); err != nil {
		guild.Members = previousMembers
		p.Char.GuildID, p.Char.GuildRank = oldID, oldRank
		s.Send(wire.MessagePanel("Save failed. You did not join the guild."))
		log.Printf("[#%d] ERROR /accept: %v", s.ID, err)
		return
	}
	p.GuildInviteFrom = 0
	w.publishGuildIdentity(p)
	s.Send(wire.MessagePanel(fmt.Sprintf("You joined the guild %s.", guild.Name)))
	w.announceToGuild(guild.ID, fmt.Sprintf("%s joined the guild.", p.Char.Name), p)
	log.Printf("[#%d] GUILD %q joined guild %d", s.ID, p.Char.Name, guild.ID)
}

// removeGuildMember removes canonical membership and resolves succession,
// returning whether the guild dissolved and the promoted character, if any.
func (w *World) removeGuildMember(guild *model.Guild, character string) (dissolved bool, promoted string) {
	kept := guild.Members[:0:0]
	var removedRank byte
	for _, member := range guild.Members {
		if strings.EqualFold(member.Character, character) {
			removedRank = member.Rank
			continue
		}
		kept = append(kept, member)
	}
	guild.Members = kept
	if len(guild.Members) == 0 {
		// The last member left: dissolve rather than leave an orphan guild.
		for i := range w.guilds.Guilds {
			if w.guilds.Guilds[i].ID == guild.ID {
				w.guilds.Guilds = append(w.guilds.Guilds[:i], w.guilds.Guilds[i+1:]...)
				break
			}
		}
		return true, ""
	}
	if removedRank == model.GuildRankLeader {
		// The oldest member, first in the list, succeeds the leader.
		guild.Members[0].Rank = model.GuildRankLeader
		return false, guild.Members[0].Character
	}
	return false, ""
}

// applyGuildMembership synchronizes online character fields after a registry change.
func (w *World) applyGuildMembership(character string, guildID uint16, rank byte) *Player {
	target := w.playerByCharacterName(character)
	if target == nil || target.Char == nil {
		return nil
	}
	target.Char.GuildID, target.Char.GuildRank = guildID, rank
	return target
}

func (w *World) guildCommandLeave(s *net.Session, p *Player, _ string) {
	if w.criticalGuildMutationReserved(p) {
		return
	}
	guild, _ := w.guildOf(p.Char)
	if guild == nil {
		s.Send(wire.MessagePanel("You do not belong to a guild."))
		return
	}
	// Succession updates the first remaining member's account, not just the
	// departing leader. Resolve that participant before removing membership.
	if member := guild.Member(p.Char.Name); member != nil && member.Rank == model.GuildRankLeader {
		for _, successor := range guild.Members {
			if strings.EqualFold(successor.Character, p.Char.Name) {
				continue
			}
			if w.criticalGuildMutationReserved(w.playerByCharacterName(successor.Character)) {
				return
			}
			break
		}
	}
	guildID, guildName := guild.ID, guild.Name
	if w.criticalWrites != nil {
		w.guildChangeAdmission(p, w.queueGuildDeparture(p, p, guild, true))
		return
	}
	snapshot := w.snapshotGuilds()
	oldID, oldRank := p.Char.GuildID, p.Char.GuildRank

	dissolved, promoted := w.removeGuildMember(guild, p.Char.Name)
	p.Char.GuildID, p.Char.GuildRank = 0, 0

	accounts := []*model.Account{p.Account}
	var promotedPlayer *Player
	if promoted != "" {
		promotedPlayer = w.applyGuildMembership(promoted, guildID, model.GuildRankLeader)
		if promotedPlayer != nil && promotedPlayer.Account != p.Account {
			accounts = append(accounts, promotedPlayer.Account)
		}
	}
	if dissolved {
		// No remaining online members need synchronization after dissolution.
		log.Printf("[#%d] GUILD %d %q dissolved (last member left)", s.ID, guildID, guildName)
	}

	if err := w.saveGuildState(accounts...); err != nil {
		w.restoreGuilds(snapshot)
		p.Char.GuildID, p.Char.GuildRank = oldID, oldRank
		if promotedPlayer != nil {
			w.repairGuildState(promotedPlayer.Char)
		}
		s.Send(wire.MessagePanel("Save failed. You are still in the guild."))
		log.Printf("[#%d] ERROR /leave: %v", s.ID, err)
		return
	}
	s.Send(wire.MessagePanel(fmt.Sprintf("You left the guild %s.", guildName)))
	w.publishGuildIdentity(p)
	if promoted != "" {
		w.publishGuildIdentity(promotedPlayer)
		w.announceToGuild(guildID, fmt.Sprintf("%s now leads the guild.", promoted), nil)
	}
	log.Printf("[#%d] GUILD %q left guild %d", s.ID, p.Char.Name, guildID)
}

func (w *World) guildCommandExpel(s *net.Session, p *Player, arg string) {
	if w.criticalGuildMutationReserved(p) {
		return
	}
	target := strings.TrimSpace(arg)
	if target == "" {
		s.Send(wire.MessagePanel("Usage: /expel <name>"))
		return
	}
	guild, err := w.requireGuildLeader(p.Char)
	if err != nil {
		s.Send(wire.MessagePanel(guildAuthMessage(err, "expel")))
		return
	}
	if strings.EqualFold(target, p.Char.Name) {
		s.Send(wire.MessagePanel("Type /leave to leave the guild."))
		return
	}
	victim := guild.Member(target)
	if victim == nil {
		s.Send(wire.MessagePanel("That player is not in your guild."))
		return
	}
	// Only online members can be expelled without a character/account index
	// for updating the persisted account of an offline member.
	victimPlayer := w.playerByCharacterName(victim.Character)
	if victimPlayer == nil || victimPlayer.Char == nil {
		s.Send(wire.MessagePanel("You can only expel members who are online."))
		return
	}
	// The native packet requires a strictly higher rank.
	if _, actor := w.guildOf(p.Char); actor != nil && !model.CanDeprivate(actor.Rank, victim.Rank) {
		s.Send(wire.MessagePanel("You cannot expel that member."))
		return
	}
	w.expelGuildMember(s, p, guild, victimPlayer)
}

// onGuildDeprivate handles 0x28C _MSG_Deprivate, the UI expel button. Unlike
// /expel's character-name lookup, it carries the target client ID in Parm@12.
// DoDeprivate requires a strictly higher rank: sub-leaders may expel ordinary
// members and the leader may expel anyone below them, never peers or the leader.
func (w *World) onGuildDeprivate(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || len(pkt) < 16 {
		return
	}
	if w.criticalGuildMutationReserved(p) {
		return
	}
	targetID := uint16(binary.LittleEndian.Uint32(pkt[12:16]))
	guild, actor := w.guildOf(p.Char)
	if guild == nil {
		s.Send(wire.MessagePanel("You do not belong to a guild."))
		return
	}
	target := w.playerByID(targetID)
	if target == nil || target.Char == nil || !w.playersShareGameplaySpace(p, target) {
		s.Send(wire.MessagePanel("That player is not online."))
		log.Printf("[#%d] 0x28C target %d not found", s.ID, targetID)
		return
	}
	if target == p {
		s.Send(wire.MessagePanel("Type /leave to leave the guild."))
		return
	}
	victim := guild.Member(target.Char.Name)
	if victim == nil {
		s.Send(wire.MessagePanel("That player is not in your guild."))
		return
	}
	if !model.CanDeprivate(actor.Rank, victim.Rank) {
		s.Send(wire.MessagePanel("You cannot expel that member."))
		log.Printf("[#%d] 0x28C rejected: rank %d cannot expel rank %d",
			s.ID, actor.Rank, victim.Rank)
		return
	}
	w.expelGuildMember(s, p, guild, target)
}

// expelGuildMember is shared by packet 0x28C and the /expel command.
func (w *World) expelGuildMember(s *net.Session, actor *Player, guild *model.Guild, victim *Player) {
	if w.criticalGuildMutationReserved(actor, victim) {
		return
	}
	if w.criticalWrites != nil {
		w.guildChangeAdmission(actor, w.queueGuildDeparture(actor, victim, guild, false))
		return
	}
	guildID, victimName := guild.ID, victim.Char.Name
	snapshot := w.snapshotGuilds()
	oldID, oldRank := victim.Char.GuildID, victim.Char.GuildRank

	_, promoted := w.removeGuildMember(guild, victimName)
	victim.Char.GuildID, victim.Char.GuildRank = 0, 0
	var promotedPlayer *Player
	if promoted != "" {
		promotedPlayer = w.applyGuildMembership(promoted, guildID, model.GuildRankLeader)
	}

	accounts := []*model.Account{actor.Account}
	if victim.Account != actor.Account {
		accounts = append(accounts, victim.Account)
	}
	if err := w.saveGuildState(accounts...); err != nil {
		w.restoreGuilds(snapshot)
		victim.Char.GuildID, victim.Char.GuildRank = oldID, oldRank
		if promotedPlayer != nil {
			w.repairGuildState(promotedPlayer.Char)
		}
		s.Send(wire.MessagePanel("Save failed. Nobody was expelled."))
		log.Printf("[#%d] ERROR expelling %q: %v", s.ID, victimName, err)
		return
	}
	s.Send(wire.MessagePanel(fmt.Sprintf("%s was expelled from the guild.", victimName)))
	victim.Session.Send(wire.MessagePanel("You were expelled from the guild."))
	w.publishGuildIdentity(victim)
	w.publishGuildIdentity(promotedPlayer)
	log.Printf("[#%d] GUILD %q expelled %q from guild %d", s.ID, actor.Char.Name, victimName, guildID)
}

// guildCommandSubLeader promotes a member to the lowest available sub-leader
// slot (6, 7 or 8), matching native Sub1/Sub2/Sub3.
func (w *World) guildCommandSubLeader(s *net.Session, p *Player, arg string) {
	if w.criticalGuildMutationReserved(p) {
		return
	}
	target := strings.TrimSpace(arg)
	if target == "" {
		s.Send(wire.MessagePanel("Usage: /subcreate <name>"))
		return
	}
	guild, err := w.requireGuildLeader(p.Char)
	if err != nil {
		s.Send(wire.MessagePanel(guildAuthMessage(err, "promote")))
		return
	}
	promoted := w.playerByCharacterName(target)
	if promoted == nil || promoted.Char == nil {
		s.Send(wire.MessagePanel("That player is not online."))
		return
	}
	member := guild.Member(promoted.Char.Name)
	if w.criticalGuildMutationReserved(promoted) {
		return
	}
	if member == nil {
		s.Send(wire.MessagePanel("That player is not in your guild."))
		return
	}
	if member.Rank != model.GuildRankMember {
		s.Send(wire.MessagePanel("That member already holds a rank."))
		return
	}
	rank, ok := guild.FreeSubLeaderRank()
	if !ok {
		s.Send(wire.MessagePanel("The guild already has three sub-leaders."))
		return
	}
	if w.criticalWrites != nil {
		w.guildChangeAdmission(p, w.queueGuildPromotion(p, promoted, guild, rank))
		return
	}

	snapshot := w.snapshotGuilds()
	oldRank := promoted.Char.GuildRank
	member.Rank = rank
	promoted.Char.GuildRank = rank

	accounts := []*model.Account{p.Account}
	if promoted.Account != p.Account {
		accounts = append(accounts, promoted.Account)
	}
	if err := w.saveGuildState(accounts...); err != nil {
		w.restoreGuilds(snapshot)
		promoted.Char.GuildRank = oldRank
		s.Send(wire.MessagePanel("Save failed. The promotion was not applied."))
		log.Printf("[#%d] ERROR /subcreate: %v", s.ID, err)
		return
	}
	s.Send(wire.MessagePanel(fmt.Sprintf("%s is now a sub-leader.", promoted.Char.Name)))
	promoted.Session.Send(wire.MessagePanel("You were promoted to sub-leader."))
	w.publishGuildIdentity(promoted)
	log.Printf("[#%d] GUILD %q promoted to sub-leader (rank %d) in guild %d",
		s.ID, promoted.Char.Name, rank, guild.ID)
}

// onGuildAlly handles 0xE12 _MSG_GuildAlly. The sender must lead the claimed
// guild; the target is a guild ID, not a character.
func (w *World) onGuildAlly(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || len(pkt) < 20 {
		return
	}
	if w.criticalGuildMutationReserved(p) {
		return
	}
	claimed := uint16(binary.LittleEndian.Uint32(pkt[12:16]))
	allyID := uint16(binary.LittleEndian.Uint32(pkt[16:20]))
	guild, err := w.requireGuildLeader(p.Char)
	if err != nil {
		s.Send(wire.MessagePanel(guildAuthMessage(err, "ally")))
		return
	}
	// The client claims its own guild in the first field; do not trust it.
	if claimed != 0 && claimed != guild.ID {
		log.Printf("[#%d] 0xE12 guild mismatch: packet=%d actual=%d", s.ID, claimed, guild.ID)
	}
	if w.criticalWrites != nil {
		if allyID == guild.ID {
			s.Send(wire.MessagePanel("A guild cannot ally with itself."))
			return
		}
		if allyID != 0 && w.guilds.FindByID(allyID) == nil {
			s.Send(wire.MessagePanel("Allied guild not found."))
			return
		}
		w.guildChangeAdmission(p, w.queueGuildAlliance(p, guild, allyID))
		return
	}
	// Snapshot before mutation so a failed save restores the previous alliance
	// rather than preserving an uncommitted change in memory.
	snapshot := w.snapshotGuilds()
	if allyID == 0 {
		guild.Ally = 0
	} else {
		if allyID == guild.ID {
			s.Send(wire.MessagePanel("A guild cannot ally with itself."))
			return
		}
		if w.guilds.FindByID(allyID) == nil {
			s.Send(wire.MessagePanel("Allied guild not found."))
			return
		}
		guild.Ally = allyID
	}

	if err := w.saveGuildState(p.Account); err != nil {
		w.restoreGuilds(snapshot)
		s.Send(wire.MessagePanel("Save failed. The alliance was not changed."))
		log.Printf("[#%d] ERROR 0xE12: %v", s.ID, err)
		return
	}
	if guild.Ally == 0 {
		s.Send(wire.MessagePanel("Alliance broken."))
	} else {
		s.Send(wire.MessagePanel("Alliance formed."))
	}
	log.Printf("[#%d] GUILD %d alliance -> %d", s.ID, guild.ID, guild.Ally)
}

// onGuildWar handles 0xE0E _MSG_War. Native g_pGuildWar holds a unilateral
// declaration; a reciprocal declaration starts the war. This handler persists
// the guild boundary only; zone/siege scheduling is handled elsewhere.
func (w *World) onGuildWar(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || len(pkt) != 20 {
		return
	}
	if w.criticalGuildMutationReserved(p) {
		return
	}
	if wire.ParseHeader(pkt).ID != p.ID {
		w.recordSecurityViolation(s, wire.OpGuildWar, "declarer ID does not match session")
		return
	}
	guild, err := w.requireGuildLeader(p.Char)
	if err != nil {
		s.Send(wire.MessagePanel(guildAuthMessage(err, "declare war")))
		return
	}
	claimed := binary.LittleEndian.Uint32(pkt[12:16])
	if claimed != 0 && claimed != uint32(guild.ID) {
		log.Printf("[#%d] 0xE0E guild mismatch: packet=%d actual=%d", s.ID, claimed, guild.ID)
	}
	targetRaw := binary.LittleEndian.Uint32(pkt[16:20])
	if targetRaw > uint32(model.MaxGuildID) {
		w.recordSecurityViolation(s, wire.OpGuildWar, "target guild outside allowed range")
		return
	}
	targetID := uint16(targetRaw)
	if targetID == guild.ID {
		s.Send(wire.MessagePanel("A guild cannot declare war on itself."))
		return
	}
	if targetID != 0 && w.guilds.FindByID(targetID) == nil {
		s.Send(wire.MessagePanel("Target guild not found."))
		return
	}
	if targetID == 0 {
		w.cancelGuildWar(s, guild)
		return
	}
	if guild.WarTarget != 0 {
		s.Send(wire.MessagePanel("Your guild already has a war declaration."))
		return
	}
	if w.criticalWrites != nil {
		w.guildChangeAdmission(p, w.queueGuildWarDeclaration(p, guild, targetID))
		return
	}

	target := w.guilds.FindByID(targetID)
	reciprocal := target.WarTarget == guild.ID
	snapshot := w.snapshotGuilds()
	guild.WarTarget = targetID
	if err := w.saveGuildState(); err != nil {
		w.restoreGuilds(snapshot)
		s.Send(wire.MessagePanel("Save failed. The war declaration was not applied."))
		log.Printf("[#%d] ERROR 0xE0E declaring war guild=%d target=%d: %v", s.ID, guild.ID, targetID, err)
		return
	}
	if reciprocal {
		w.announceToGuild(guild.ID, fmt.Sprintf("War started against guild %s.", target.Name), nil)
		w.announceToGuild(target.ID, fmt.Sprintf("War started against guild %s.", guild.Name), nil)
	} else {
		w.announceToGuild(guild.ID, fmt.Sprintf("War declared against guild %s.", target.Name), nil)
		w.announceToGuild(target.ID, fmt.Sprintf("Guild %s declared war on your guild.", guild.Name), nil)
	}
	log.Printf("[#%d] 0xE0E guild=%d war -> %d reciprocal=%t", s.ID, guild.ID, targetID, reciprocal)
}

func (w *World) cancelGuildWar(s *net.Session, guild *model.Guild) {
	if w.criticalGuildMutationReserved(w.players[s]) {
		return
	}
	if guild.WarTarget == 0 {
		return
	}
	if w.criticalWrites != nil {
		p := w.players[s]
		if p != nil {
			w.guildChangeAdmission(p, w.queueGuildWarDeclaration(p, guild, 0))
		}
		return
	}
	target := w.guilds.FindByID(guild.WarTarget)
	reciprocal := target != nil && target.WarTarget == guild.ID
	snapshot := w.snapshotGuilds()
	targetID := guild.WarTarget
	guild.WarTarget = 0
	if reciprocal {
		target.WarTarget = 0
	}
	if err := w.saveGuildState(); err != nil {
		w.restoreGuilds(snapshot)
		s.Send(wire.MessagePanel("Save failed. The war declaration was not canceled."))
		log.Printf("[#%d] ERROR 0xE0E canceling war guild=%d target=%d: %v", s.ID, guild.ID, targetID, err)
		return
	}
	message := "War declaration canceled."
	if reciprocal {
		message = "War canceled."
	}
	w.announceToGuild(guild.ID, message, nil)
	if target != nil {
		w.announceToGuild(target.ID, message, nil)
	}
	log.Printf("[#%d] 0xE0E guild=%d canceled war target=%d reciprocal=%t", s.ID, guild.ID, targetID, reciprocal)
}

// announceToGuild sends a panel notice to online members, optionally skipping
// the actor who already received the response.
func (w *World) announceToGuild(guildID uint16, message string, skip *Player) {
	for _, member := range w.onlineGuildMembers(guildID) {
		if member == skip {
			continue
		}
		member.Session.Send(wire.MessagePanel(message))
	}
}

// snapshotGuilds/restoreGuilds roll back memory after a failed save. Clone
// member lists deeply because commands mutate them in place.
func (w *World) snapshotGuilds() []model.Guild {
	out := make([]model.Guild, len(w.guilds.Guilds))
	for i, guild := range w.guilds.Guilds {
		out[i] = guild
		out[i].Members = append([]model.GuildMember(nil), guild.Members...)
	}
	return out
}

func (w *World) restoreGuilds(snapshot []model.Guild) {
	w.guilds.Guilds = snapshot
}
