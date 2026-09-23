package game

import (
	"bytes"
	"fmt"
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const commandClearInventory = "/limparinv"
const magicTrumpet = uint16(3330)

func parseChatText(pkt []byte) (string, bool) {
	if len(pkt) < 13 || len(pkt) > 108 {
		return "", false
	}
	body := pkt[12:]
	if end := bytes.IndexByte(body, 0); end >= 0 {
		body = body[:end]
	}
	return strings.TrimSpace(string(body)), true
}

// The client interprets "/name text" as MSG_MessageWhisper. Thus,
// "/limparinv" arrives with MobName="limparinv" and an empty String, not as chat.
func parseWhisperTarget(pkt []byte) (string, bool) {
	if len(pkt) < 28 {
		return "", false
	}
	target := pkt[12:28]
	if end := bytes.IndexByte(target, 0); end >= 0 {
		target = target[:end]
	}
	return normalizeWhisperTarget(string(target)), true
}

func normalizeWhisperTarget(target string) string {
	target = strings.TrimSpace(strings.TrimPrefix(strings.TrimSpace(target), "/"))
	if len(target) >= 2 && target[0] == '"' && target[len(target)-1] == '"' {
		target = strings.TrimSpace(target[1 : len(target)-1])
	}
	return target
}

func parseWhisperText(pkt []byte) (string, bool) {
	// Version 7.48 uses 0x334 with 128 bytes; text ends before Color@124.
	if len(pkt) < 124 {
		return "", false
	}
	body := pkt[28:124]
	if end := bytes.IndexByte(body, 0); end >= 0 {
		body = body[:end]
	}
	return strings.TrimSpace(string(body)), true
}

// Version 7.48 has two paths for the same visual command. Depending on chat
// input focus, /nick text can arrive as a separate p334 or as the p333 String.
// The native /"nick" text syntax is also accepted.
func parseSlashWhisperCommand(message string) (target, body string, ok bool) {
	message = strings.TrimSpace(message)
	if len(message) < 2 || message[0] != '/' {
		return "", "", false
	}
	rest := strings.TrimSpace(message[1:])
	if rest == "" {
		return "", "", false
	}
	if rest[0] == '"' {
		end := strings.IndexByte(rest[1:], '"')
		if end < 0 {
			return "", "", false
		}
		target = strings.TrimSpace(rest[1 : end+1])
		body = strings.TrimSpace(rest[end+2:])
	} else {
		cut := strings.IndexAny(rest, " \t")
		if cut < 0 {
			return "", "", false
		}
		target = strings.TrimSpace(rest[:cut])
		body = strings.TrimSpace(rest[cut:])
	}
	return target, body, target != "" && body != ""
}

// clearInventory zeros the entire structural array, including index 63, which
// has no cell in the 7.48 UI. It returns the number of items/stacks removed.
func clearInventory(ch *model.Char) int {
	if ch == nil {
		return 0
	}
	removed := 0
	for i := range ch.Inv {
		if ch.Inv[i].Index != 0 {
			removed++
		}
		ch.Inv[i] = model.Item{}
	}
	return removed
}

func (w *World) executeClearInventory(s *net.Session, p *Player) {
	if p.GhostShop != nil {
		s.Send(wire.MessagePanel("Close Auto Trade before clearing your inventory."))
		return
	}
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if _, filled := model.CelestialSealID(p.Char.Inv[i]); filled {
			s.Send(wire.MessagePanel("Extract or move the filled Spirit's Seal before clearing inventory."))
			return
		}
	}
	oldInv := p.Char.Inv
	removed := clearInventory(p.Char)
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv = oldInv
		log.Printf("[#%d] ERROR /limparinv account=%q: %v", s.ID, p.Account.Name, err)
		s.Send(wire.MessagePanel("Save failed. The inventory was not changed."))
		return
	}
	s.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
	s.Send(wire.MessagePanel("Inventory cleared."))
	log.Printf("[#%d] /limparinv removed %d item(s) from account %q",
		s.ID, removed, p.Account.Name)
}

func (w *World) onMessageChat(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld {
		return
	}
	message, ok := parseChatText(pkt)
	if !ok || message == "" {
		return
	}

	// Dispatch commands first: depending on chat input focus, the SAME command
	// arrives as 0x333 (here) or 0x334. Both paths use the dispatcher.
	if name, arg, ok := parseSlashCommand(message); ok {
		if w.dispatchChatCommand(s, p, name, arg) {
			return
		}
	}
	if target, body, ok := parseSlashWhisperCommand(message); ok {
		w.deliverWhisper(s, p, target, body)
		return
	}
	if !w.allowChat(p, "local", w.now()) {
		return
	}
	// Local chat: relay as 0x333 WITH THE SENDER ID.
	//
	// The overhead bubble comes from TMHuman::OnPacketMessageChat, an ENTITY
	// handler rather than a scene handler: the client finds the human by
	// Header.ID and calls SetChatMessage. Sending 0x334 or 0x333 with id=0
	// may put the line in the chat list but never draws the bubble.
	// TMFieldScene::OnPacketMessageChat only handles out-of-view fallback and
	// therefore requires a group list.
	//
	// The sender draws its own bubble before sending; relaying back would
	// duplicate the line.
	observers := 0
	for _, observer := range w.nearbyWorldPlayers(p.X, p.Y, viewHalfX) {
		if observer == p || !observer.InWorld || observer.Session == nil ||
			!w.playersVisibleTogether(p, observer) {
			continue
		}
		// Chat text width is selected by the observer's negotiated client ABI.
		observer.Session.Send(wire.MessageChat(p.ID, message))
		observers++
	}
	log.Printf("[#%d] local CHAT %q (%d observer(s))", s.ID, p.Char.Name, observers)
}

func (w *World) onMessageWhisper(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld {
		return
	}
	target, okTarget := parseWhisperTarget(pkt)
	message, okText := parseWhisperText(pkt)
	if !okTarget || !okText {
		return
	}
	// The command comes in MobName ("/criar Alfa" arrives as target="criar",
	// message="Alfa"). It must run BEFORE rejecting empty messages: commands
	// such as /aceitar and /sair arrive without a body.
	if w.dispatchChatCommand(s, p, target, message) {
		return
	}
	// "/nick" without a message: the client sends MobName and an empty String
	// (TMFieldScene copies text only if something follows the nickname).
	// Treat this as a character-information lookup.
	if message == "" {
		w.sendCharacterInfo(s, p, target)
		return
	}
	// Prefixes confirmed in TMFieldScene 7.48. The initial character belongs
	// to the visual protocol and must survive relaying: the client uses it
	// to select the color and text-cut offset.
	switch chatChannelOf(message) {
	case chatChannelParty:
		if !w.allowChat(p, chatChannelParty, w.now()) {
			return
		}
		w.sendPartyChat(p, strings.TrimSpace(message[1:]))
	case chatChannelGlobal:
		if !w.allowChat(p, chatChannelGlobal, w.now()) {
			return
		}
		for _, observer := range w.players {
			if observer == p || !observer.InWorld || observer.Session == nil {
				continue // The sender's client already inserted its own message.
			}
			observer.Session.Send(wire.MessageWhisper(0, p.Char.Name, message, 3))
		}
	case chatChannelCitizenship:
		if !w.allowChat(p, chatChannelCitizenship, w.now()) || p.Char.Citizenship == 0 {
			return
		}
		for _, observer := range w.players {
			if observer == p || !observer.InWorld || observer.Session == nil ||
				observer.Char == nil || observer.Char.Citizenship != p.Char.Citizenship {
				continue
			}
			observer.Session.Send(wire.MessageWhisper(0, p.Char.Name, message, 3))
		}
	case chatChannelKingdom:
		if !w.allowChat(p, chatChannelKingdom, w.now()) {
			return
		}
		kingdom := characterKingdom(p.Char)
		if kingdom == model.KingdomNeutral {
			return
		}
		for _, observer := range w.players {
			if observer == p || !observer.InWorld || observer.Session == nil ||
				observer.Char == nil || characterKingdom(observer.Char) != kingdom {
				continue
			}
			observer.Session.Send(wire.MessageWhisper(0, p.Char.Name, message, 3))
		}
	case chatChannelGuild:
		if !w.allowChat(p, chatChannelGuild, w.now()) {
			return
		}
		w.sendGuildChat(p, message)
	default:
		w.deliverWhisper(s, p, target, message)
	}
}

// Channels selected by prefix in 0x334.
const (
	chatChannelParty       = "party"
	chatChannelGlobal      = "global"
	chatChannelCitizenship = "citizenship"
	chatChannelKingdom     = "kingdom"
	chatChannelGuild       = "guild"
	chatChannelWhisper     = "whisper"
)

// chatChannelOf classifies a message by its typed prefix.
//
// ORDER MATTERS: '--' (global, a project decision) must be tested BEFORE '-'
// (guild), or global messages take the guild route. The client renders both
// through one branch; the server separates them.
func chatChannelOf(message string) string {
	switch {
	case strings.HasPrefix(message, "="):
		return chatChannelParty
	case strings.HasPrefix(message, "--"):
		return chatChannelGlobal
	case strings.HasPrefix(message, "@@"):
		return chatChannelCitizenship
	case strings.HasPrefix(message, "@"):
		return chatChannelKingdom
	case strings.HasPrefix(message, "-"):
		return chatChannelGuild
	default:
		return chatChannelWhisper
	}
}

// parseSlashCommand splits "/name argument" and accepts an EMPTY argument.
// parseSlashWhisperCommand requires a body (a whisper), so it cannot handle
// commands such as /aceitar and /sair.
func parseSlashCommand(message string) (name, arg string, ok bool) {
	message = strings.TrimSpace(message)
	if len(message) < 2 || message[0] != '/' {
		return "", "", false
	}
	rest := strings.TrimSpace(message[1:])
	if rest == "" {
		return "", "", false
	}
	if cut := strings.IndexAny(rest, " \t"); cut >= 0 {
		return rest[:cut], strings.TrimSpace(rest[cut:]), true
	}
	return rest, "", true
}

// dispatchChatCommand is the single command entry point for 0x333 and 0x334.
// It returns true when it consumes the message. A command takes precedence
// over a nickname with the same spelling, as in the native TMSrv.
var chatCommandAliases = map[string]string{
	"guildfame": "guildfame", "famaguild": "guildfame", "citywar": "citywar", "tower": "tower",
	"day": "day", "time": "time", "cp": "cp", "chaos": "cp", "fame": "fame",
	"srv": "srv",
	"parry": "parry",
	"nig":   "nig", "limparinv": "clearinv", "clearinv": "clearinv",
	"spk": "spk", "kingdom": "kingdom", "reino": "kingdom", "king": "king", "rei": "king",
	"criar": "create", "create": "create", "convidar": "invite", "invite": "invite",
	"aceitar": "accept", "accept": "accept", "sair": "leave", "leave": "leave",
	"expulsar": "expel", "expel": "expel", "criarsub": "createsub", "subcreate": "createsub",
	"createsub": "createsub",
}

func (w *World) dispatchChatCommand(s *net.Session, p *Player, name, arg string) bool {
	command, known := chatCommandAliases[strings.ToLower(strings.TrimSpace(name))]
	if !known {
		return false
	}
	switch command {
	case "guildfame", "citywar", "tower":
		w.guildWarCommand(p, command, arg)
	case "day":
		// Periodic internal client sync. !# suppresses visible text and feeds
		// m_nYear/m_nDays, which determine calendar-affect duration.
		s.Send(wire.DaySync())
	case "srv":
		// The Field panel sends this request as a whisper after five seconds.
		// This World has no channel coordinator or 0x52A ticket yet; do not
		// simulate migration or treat "srv" as a character name.
		s.Send(wire.MessagePanel("Channel switching is unavailable on this server."))
	case "time":
		// Manual command: display host date/time in the upper panel.
		s.Send(wire.MessagePanel(w.now().Format("15:04:05 | 02-01-2006")))
	case "cp":
		// CP is the character's signed Chaos/PK Point (-75..+75). It is not
		// the EXP Hold in 0x337 and must never be formatted as XP.
		s.Send(wire.MessagePanel(chaosPointMessage(p.Char.CP)))
	case "fame":
		// Fame is a per-character counter (charstate), separate from CP,
		// EXP Hold, and Special Points. Looking it up never changes state.
		s.Send(wire.MessagePanel(fameMessage(p)))
	case "parry":
		w.sendParryMatchup(s, p, arg)
	case "nig":
		// The client sends this command internally when using Nightmare tickets
		// (SGrid.cpp): it expects !!HHMMSS to update the instance's local clock.
		// This is not a nickname; treating it as a /nig lookup produced the
		// false "nig is not online." message.
		s.Send(wire.MessagePanel(nightmareTimeMessage(w.now())))
	case "clearinv":
		w.executeClearInventory(s, p)
	case "spk":
		w.executeShout(s, p, arg)
	case "kingdom":
		w.kingdomCommandTeleport(s, p, false)
	case "king":
		w.kingdomCommandTeleport(s, p, true)
	case "create":
		w.guildCommandCreate(s, p, arg)
	case "invite":
		w.guildCommandInvite(s, p, arg)
	case "accept":
		w.guildCommandAccept(s, p, arg)
	case "leave":
		w.guildCommandLeave(s, p, arg)
	case "expel":
		w.guildCommandExpel(s, p, arg)
	case "createsub":
		w.guildCommandSubLeader(s, p, arg)
	default:
		return false
	}
	return true
}

func nightmareTimeMessage(now time.Time) string {
	return "!!" + now.Format("150405")
}

func chaosPointMessage(cp int16) string {
	return fmt.Sprintf("Chaos Point: %d (range -75..+75)", model.ClampCP(int(cp)))
}

func fameMessage(p *Player) string {
	return fmt.Sprintf("Fame: %d", counterBalance(p, fameCounter))
}

func (w *World) sendParryMatchup(s *net.Session, p *Player, targetName string) {
	if s == nil || p == nil || p.Char == nil {
		return
	}
	targetName = strings.TrimSpace(targetName)
	if targetName == "" {
		s.Send(wire.MessagePanel("Usage: /parry <player>"))
		return
	}
	target := w.playerByCharacterName(targetName)
	if target == nil || target.Char == nil || !target.InWorld {
		s.Send(wire.MessagePanel("That player is not online."))
		return
	}
	accuracy := playerVersusPlayerAccuracy(p.Char, target.Char)
	evasion := combatEvasionPercent(playerDex(p.Char), playerEvasionBonusPoints(p.Char),
		playerAccuracyBonusPoints(target.Char),
		playerHasConcentration(target.Char))
	s.Send(wire.MessagePanel(fmt.Sprintf("[Accuracy: %d%% | Evasion: %d%%] against %s",
		accuracy, evasion, target.Char.Name)))
}

// sendCharacterInfo answers "/nick" without a message with a character
// summary in a single notice-panel packet.
//
// Only CONNECTED players can be queried: there is no character-to-account
// index, so offline lookup would require scanning every account JSON file.
func (w *World) sendCharacterInfo(s *net.Session, p *Player, target string) {
	if target == "" {
		return
	}
	found := w.playerByCharacterName(target)
	if found == nil || found.Char == nil {
		s.Send(wire.MessagePanel(fmt.Sprintf("%s is not online.", target)))
		return
	}
	ch := found.Char
	// Put everything in ONE notice (0x101). The native WYD 7.48 inspection
	// window (0x324) was tested in-game, but the 7.48 client opens nothing;
	// it is a later 759 addition. Multiple lines also fail because each new
	// notice replaces the previous one in the panel.
	s.Send(wire.MessagePanel(w.characterInfoLine(ch)))
	log.Printf("[#%d] INFO %q queried %q", s.ID, p.Char.Name, ch.Name)
}

// displayLevel converts the zero-based internal level to the visible number.
// The native ConsultaInfoPlayer makes the same +1 adjustment.
func displayLevel(ch *model.Char) uint32 {
	if ch == nil || ch.Score == nil {
		return 1
	}
	return ch.Score.Level + 1
}

// characterInfoLine builds a ONE-line summary that fits in the notice panel.
// It is separate from sending so it can be tested without a network session.
func (w *World) characterInfoLine(ch *model.Char) string {
	parts := []string{fmt.Sprintf("%s  Lv.%d", ch.Name, displayLevel(ch))}

	if guild, member := w.guildOf(ch); guild != nil {
		role := "Member"
		switch {
		case member.Rank == model.GuildRankLeader:
			role = "Leader"
		case model.IsSubLeader(member.Rank):
			role = "Sub-leader"
		}
		guildPart := fmt.Sprintf("Guild: %s (%s)", guild.Name, role)
		if ally := w.guilds.FindByID(guild.Ally); ally != nil {
			guildPart += fmt.Sprintf(" allied with %s", ally.Name)
		}
		parts = append(parts, guildPart)
	} else {
		parts = append(parts, "No guild")
	}

	// Citizenship is granted by NPC Kibita. Until that system exists,
	// characterCitizenship returns empty and the field is omitted.
	if citizenship := characterCitizenship(ch); citizenship != "" {
		parts = append(parts, "Citizenship: "+citizenship)
	}
	if kingdom := characterKingdom(ch); kingdom != model.KingdomNeutral {
		parts = append(parts, "Kingdom: "+model.KingdomName(kingdom))
	}
	return strings.Join(parts, " | ")
}

// characterCitizenship describes citizenship. The stored value is the
// CHANNEL NUMBER (citizenship 1 = channel 1), as in the native client.
func characterCitizenship(ch *model.Char) string {
	if ch == nil || ch.Citizenship == 0 {
		return ""
	}
	return fmt.Sprintf("Channel %d", ch.Citizenship)
}

func (w *World) deliverWhisper(s *net.Session, p *Player, target, message string) {
	if s == nil || p == nil || p.Char == nil || target == "" || message == "" {
		return
	}
	if !w.allowChat(p, chatChannelWhisper, w.now()) {
		return
	}
	recipient := w.playerByCharacterName(target)
	if recipient == nil {
		s.Send(wire.MessagePanel("That player is not online."))
		log.Printf("[#%d] WHISPER %q -> %q: offline", s.ID, p.Char.Name, target)
		return
	}
	if strings.HasPrefix(message, "!") {
		// '!' is not decorative text: the client's 0x334 handler writes the
		// message to the H panel. The death letter uses this same channel.
		recipient.Session.Send(wire.MessageWhisper(0, p.Char.Name, message, 0))
		log.Printf("[#%d] LETTER %q -> %q", s.ID, p.Char.Name, recipient.Char.Name)
		return
	}
	// Mirror TMSrv/WYD 7.48: /nick message arrives as 0x334; the server sets
	// MobName to the sender, preserves String without prefixes, and uses its ID.
	recipient.Session.Send(wire.MessageWhisper(p.ID, p.Char.Name, message, 0))
	log.Printf("[#%d] WHISPER %q -> %q", s.ID, p.Char.Name, recipient.Char.Name)
}

// playerByCharacterName finds an ONLINE player by character name.
//
// The Arch inherits the Mortal's name, so duplicate names exist. Lookup
// needs no tie-breaker because of two invariants:
//
//  1. Duplicates arise only from ascension and therefore belong to the SAME
//     account (normal creation requires globally unique names).
//  2. One account cannot log in twice (claimAccountSession).
//
// Together these guarantee at most ONE online duplicate. If either invariant
// fails, lookup becomes silently ambiguous.
// TestAccountSessionIsExclusiveCaseInsensitive guards the second invariant.
func (w *World) playerByCharacterName(name string) *Player {
	key := strings.ToLower(strings.TrimSpace(name))
	if key == "" {
		return nil
	}
	if indexed := w.playersByName[key]; indexed != nil {
		if indexed.InWorld && indexed.Char != nil && strings.EqualFold(indexed.Char.Name, name) {
			return indexed
		}
		delete(w.playersByName, key)
	}
	// Repair old fixtures/imports that have not passed through the indexer.
	for _, p := range w.players {
		if p.InWorld && p.Char != nil && strings.EqualFold(p.Char.Name, name) {
			if w.playersByName == nil {
				w.playersByName = make(map[string]*Player)
			}
			w.playersByName[key] = p
			return p
		}
	}
	return nil
}

func (w *World) sendPartyChat(sender *Player, message string) {
	if sender == nil || sender.Char == nil || sender.Party == nil || message == "" {
		return
	}
	for _, member := range sender.Party.Members {
		if member == nil || !member.InWorld || member.Char == nil || member == sender {
			continue // TMFieldScene inserts local text before sending.
		}
		member.Session.Send(wire.MessageWhisper(0, sender.Char.Name, "="+message, 1))
	}
}

// executeShout consumes one Shout (item 3330) before announcing. Persistence
// precedes broadcast: if storage fails, neither the item nor the message is
// committed, preventing an unpaid announcement.
func (w *World) executeShout(s *net.Session, p *Player, message string) {
	if message == "" {
		s.Send(wire.MessagePanel("Usage: /spk message"))
		return
	}
	if len(message) > 120 {
		message = message[:120]
	}
	slot := -1
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if p.Char.Inv[i].Index == magicTrumpet {
			slot = i
			break
		}
	}
	if slot < 0 {
		s.Send(wire.MessagePanel("You need a Shout to announce."))
		return
	}
	previous := p.Char.Inv[slot]
	p.Char.Inv[slot] = model.Item{}
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[slot] = previous
		log.Printf("[#%d] ERROR /spk account=%q: %v", s.ID, p.Account.Name, err)
		s.Send(wire.MessagePanel("Failed to consume the Shout. No announcement was sent."))
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, byte(slot), p.Char.Inv[slot]))
	w.broadcast(func() []byte { return wire.MessageWhisper(0, "[SERVER]", message, 7) })
	log.Printf("[#%d] /spk %q", s.ID, message)
}
