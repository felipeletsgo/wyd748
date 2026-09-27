package game

import (
	"encoding/binary"
	"log"
	"strings"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// removePlayerFromWorld clears only ephemeral character state. The account
// and session stay authenticated so 0x215 can return to character selection
// without opening another TCP connection.
func (w *World) removePlayerFromWorld(p *Player, reason string) {
	if p == nil {
		return
	}
	w.detachPlayerFromItemInstances(p.ID, w.now())
	// A round invitation belongs to this character lifecycle, not the TCP
	// connection: relogging into the same slot cannot revive an old token.
	if w.quiz != nil {
		delete(w.quiz.participants, p.Session)
	}
	w.unregisterPlayerSpatial(p)
	w.unindexPlayerCharacter(p)
	delete(w.playersByID, p.ID)
	w.closeGhostShop(p, reason)
	w.cancelTrade(p, reason)
	w.removePartyPlayer(p)
	// Summons belong to their owner: despawn them when the owner leaves the world
	// so they cannot follow the next player to reuse this ID.
	w.removePlayerSummons(p.ID)
	if p.InWorld {
		for _, other := range w.players {
			if other != p && other.InWorld && other.hasVisible(p.ID) {
				other.Session.Send(wire.RemoveMob(p.ID, 0))
				other.hide(p.ID)
			}
		}
	}
	p.InWorld = false
	resetCharacterRuntime(p)
}

// returnToCharacterSelectionAfterCommittedChange ends only the character
// session after a structural mutation has committed to the store. Arch and
// Celestial change the body, score, and/or character list; rebuilding these
// in FieldScene can leave parts of the 7.48 client with stale state. Packet
// 0x116 makes the native transition, and the following 0x110 replaces all
// four selection slots with the newly persisted authoritative aggregate.
//
// This function deliberately does not save again: a later failure must not
// turn a committed evolution into a RAM-only rollback.
func (w *World) returnToCharacterSelectionAfterCommittedChange(p *Player, reason string) {
	if p == nil || p.Session == nil || p.Account == nil || !p.InWorld || p.ID == 0 {
		return
	}
	s := p.Session
	charID := p.ID
	w.removePlayerFromWorld(p, reason)
	w.flushInstanceStateIfDirty()
	s.Send(wire.CNFCharacterLogout(charID))
	// Evolution returns through the same negotiated selection ABI as login.
	s.Send(selectionUpdatePacket(s, wire.OpCNFNewCharacter, uint16(s.ID), p))
}

// resetCharacterRuntime clears all character-owned Player state while keeping
// session-owned state (Session and Account).
//
// The Player is reused across character selection: the account stays
// authenticated and the same object receives the next character. Any omitted
// field leaks to that character. This previously leaked special coins (which
// autosave then duplicated), skill cooldowns, and guild invitations.
//
// Clear new character-owned Player fields here.
// TestCharacterRuntimeIsFullyReset catches omissions.
func resetCharacterRuntime(p *Player) {
	if p == nil {
		return
	}
	p.Char = nil
	p.CharSlot = -1
	p.ID = 0
	p.X, p.Y = 0, 0
	p.AirMoveActive = false
	p.AirMoveRoute = 0
	p.AirMoveStartedAt = time.Time{}
	p.AirMoveSourceX, p.AirMoveSourceY = 0, 0
	p.Visible = nil

	// Open NPC/window context.
	p.ShopNPC = 0
	p.ShopTax = 0
	p.CraftNPC = 0
	p.CargoNPC = 0
	clearCityWarContext(p)
	p.BrowsingGhostShopID = 0
	p.GhostShop = nil
	p.Trade = nil

	// Party and guild invitations belong to the character, not the account.
	p.Party = nil
	p.InviteFrom = 0
	p.InviteUntil = time.Time{}
	p.ChallengeFrom = 0
	p.ChallengeMode = 0
	p.ChallengeUntil = time.Time{}
	p.GuildInviteFrom = 0
	p.GuildInviteUntil = time.Time{}
	p.NextGuildInvite = time.Time{}

	// Combat.
	p.CombatTargetID = 0
	p.LastAttackerID = 0
	p.LastAttackAt = time.Time{}
	p.LastAttackTick = 0
	p.LastSkillAt = time.Time{}
	p.LastSkillTicks = nil
	p.AttackProgress = 0
	p.DeadAt = time.Time{}
	p.PKMode = false

	// Cooldowns and timers.
	p.SkillReady = nil
	p.LastPotion = time.Time{}
	p.LastPremiumFirework = time.Time{}
	p.LastCraft = time.Time{}
	p.NextRegen = time.Time{}
	p.NextCPRecovery = time.Time{}
	p.NextMountTick = time.Time{}
	p.EggIncubationUID = ""
	p.NextEggIncubationTick = time.Time{}
	p.NextKingdomTeleport = time.Time{}

	// Movement published to observers.
	p.MovePublished = false
	p.MovePublishedStartX = 0
	p.MovePublishedStartY = 0
	p.MovePublishedTargetX = 0
	p.MovePublishedTargetY = 0
	p.MovePublishedRoute = [maxMovementRouteBytes]byte{}
	p.MoveAuthorityRoute = nil
	p.MoveAuthorityStep = 0
	p.MoveAuthorityCatchupSteps = 0
	p.MoveAuthorityX = 0
	p.MoveAuthorityY = 0
	p.MoveAuthorityStartedAt = time.Time{}
	p.MoveAuthorityStepInterval = 0

	// Special coins live in character-owned charstate.
	p.SpecialCoins = nil
	p.clientIntegrityPending = nil
}

// onCharacterLogout handles 0x215. The 7.48 client expects response 0x116
// to return to TM_SELECTCHAR_STATE.
func (w *World) onCharacterLogout(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) != 12 {
		return
	}
	charID, name := p.ID, p.Char.Name
	// Publish confirmation 0x116 only after account and charstate commit
	// together. Continuing after a failed save would lose buffs/counters on
	// relog and falsely tell the client that the transition completed.
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		log.Printf("[#%d] failed to save %q during character logout: %v", s.ID, name, err)
		s.Send(wire.MessagePanel("The character could not be saved. Try again."))
		return
	}
	w.removePlayerFromWorld(p, "return to character selection")
	// Character logout also detaches a private Water member. Persist that UID
	// association before the session returns to character select.
	w.flushInstanceStateIfDirty()
	s.Send(wire.CNFCharacterLogout(charID))
	log.Printf("[#%d] CHARACTER-LOGOUT %q -> selection", s.ID, name)
}

// onCharacterTransferUnavailable ends the 7.48 client's wait without changing
// the account, names, or slots. This World has no coordinator/persistence for
// the "Integrated server" destination; claiming local success would lose data.
func (w *World) onCharacterTransferUnavailable(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Account == nil || p.InWorld || len(pkt) != 52 ||
		binary.LittleEndian.Uint32(pkt[12:16]) != 0 {
		return
	}
	slot := int32(binary.LittleEndian.Uint32(pkt[16:20]))
	if slot < 0 || slot >= 4 {
		return
	}
	s.Send(wire.CharacterTransferUnavailable(slot))
}

// onDeleteCharacter revalidates the password because 0x211 is destructive.
// The client sends Slot@12, MobName@16, and Password@32 (44 bytes).
func (w *World) onDeleteCharacter(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Account == nil || p.InWorld || len(pkt) != 44 {
		return
	}
	slot := int(int32(binary.LittleEndian.Uint32(pkt[12:16])))
	name, password := cstr(pkt[16:32]), cstr(pkt[32:44])
	if slot < 0 || slot >= len(p.Account.Chars) || p.Account.Chars[slot].Name == "" ||
		!strings.EqualFold(name, p.Account.Chars[slot].Name) || p.Account.PasswordHash == "" {
		s.Send(wire.MessagePanel("The character could not be deleted."))
		return
	}
	ok, err := account.VerifyPassword(p.Account.PasswordHash, password)
	if err != nil || !ok {
		log.Printf("[#%d] deletion denied for %q: invalid password", s.ID, name)
		s.Send(wire.MessagePanel("Wrong password."))
		return
	}
	previous := p.Account.Chars[slot]
	p.Account.Chars[slot] = model.Char{}
	if err := w.saveAccount(p.Account); err != nil {
		p.Account.Chars[slot] = previous
		log.Printf("[#%d] failed to delete character %q: %v", s.ID, name, err)
		s.Send(wire.MessagePanel("The deletion could not be saved."))
		return
	}
	// PostgreSQL removes this through ON DELETE CASCADE; the JSON adapter needs
	// explicit cleanup. Failure here cannot undo the committed deletion: the
	// sidecar is derived and another character will never reuse the UID.
	if stateStore, ok := w.store.(charStateStore); ok && previous.UID != "" {
		if err := stateStore.SaveCharState(previous.UID, nil); err != nil {
			log.Printf("[#%d] failed to clear deleted character %q charstate: %v",
				s.ID, name, err)
		}
	}
	// Release the name only when no character still uses it. The slot was
	// cleared above, so accountUsesName sees only the remaining characters.
	if w.charNames != nil && !accountUsesName(p.Account, previous.Name) {
		delete(w.charNames, strings.ToLower(previous.Name))
	}
	// Deletion replaces all four selection slots because source and stock
	// clients keep different STRUCT_SELCHAR sizes.
	s.Send(selectionUpdatePacket(s, wire.OpCNFDeleteCharacter, uint16(s.ID), p))
	log.Printf("[#%d] character deleted: %q slot=%d", s.ID, name, slot)
}

// accountUsesName reports whether this account still has a character named so.
//
// Arch inherits the Mortal's name (as in the native client), so one name can
// belong to two characters. This is the only permitted duplicate: normal
// creation (0x20F) requires a globally unique name.
//
// Without this check, deleting either twin would release the shared name
// while the other remained, allowing another account to violate uniqueness.
func accountUsesName(acc *model.Account, name string) bool {
	if acc == nil || name == "" {
		return false
	}
	for i := range acc.Chars {
		if strings.EqualFold(acc.Chars[i].Name, name) {
			return true
		}
	}
	return false
}

// onREQMobByID recovers an entity referenced by Action but not yet created
// locally. Only entities in view are returned, preventing a global map query.
func (w *World) onREQMobByID(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || len(pkt) != 16 {
		return
	}
	id := binary.LittleEndian.Uint16(pkt[12:14])
	if id == 0 || id == p.ID {
		return
	}
	if m := w.mobByID(id); m != nil && w.mobVisibleToPlayer(p, m) &&
		inView(p.X, p.Y, m.X, m.Y) {
		wasVisible := p.hasVisible(id)
		w.showMob(p, m)
		if wasVisible { // Recover an entity lost from the client's local scene.
			ancient := m.Def.Equip.AncientCodes()
			p.Session.Send(wire.CreateMobVisual(m.ID, m.Def.Name, m.X, m.Y,
				m.Def.Mesh(), ancient[:], mobPublicExtendedAt(m, w.now()), m.Affects[:], 0))
		}
		return
	}
	if target := w.playerByID(id); target != nil && target.InWorld && target.Char != nil &&
		w.playersShareGameplaySpace(p, target) && inView(p.X, p.Y, target.X, target.Y) {
		sendPlayerEnterView(p, target)
		p.show(id)
		return
	}
	if shop := w.ghostShops[id]; shop != nil && inView(p.X, p.Y, shop.X, shop.Y) {
		p.Session.Send(wire.CreateMobTrade(shop.ID, shop.Name, shop.X, shop.Y,
			shop.Mesh[:], &shop.Score, shop.Title))
		p.show(id)
	}
}

// isPlayerEmoteMotion limits C->S intent to the 7.48 keyboard/click values.
// Effect motions remain server-owned.
func isPlayerEmoteMotion(motion uint16) bool {
	return motion == 13 || (motion >= 15 && motion <= 25) || motion == 27
}

// onMotion echoes the authoritative player ID to release the sender's pending
// emote and publishes it to observers. A dead character cannot request one.
// Client Parm and Direction cannot forge fireworks, level-ups, death, or
// other server-owned effects.
func (w *World) onMotion(s *net.Session, pkt []byte) {
	if len(pkt) != 20 {
		w.noticeProtocol(s, wire.OpMotion, "unexpected size")
		return
	}
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 {
		return
	}
	motion := binary.LittleEndian.Uint16(pkt[12:14])
	parm := binary.LittleEndian.Uint16(pkt[14:16])
	if parm != 0 || !isPlayerEmoteMotion(motion) {
		return
	}
	w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, motion, 0) })
}

// 0x2BC is opaque telemetry: recognize its size without inventing
// authoritative state that the protocol has not yet defined.
func (w *World) onClientUnknown2BC(s *net.Session, pkt []byte) {
	if len(pkt) != 108 {
		w.noticeProtocol(s, wire.OpClientUnknown2BC, "unexpected size")
	}
}

func (w *World) noticeProtocol(s *net.Session, opcode uint16, detail string) {
	if s == nil {
		return
	}
	key := uint16(s.ID)<<4 ^ opcode
	now := w.now()
	if last := w.lastProtocolNotice[key]; !last.IsZero() && now.Sub(last) < time.Minute {
		return
	}
	w.lastProtocolNotice[key] = now
	log.Printf("[#%d] protocol 0x%X ignored: %s", s.ID, opcode, detail)
}
