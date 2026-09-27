package game

import (
	"strings"
	"time"

	"wydgo/internal/wire"
)

// The native WYD 7.48 grid uses a half-window of 16. This emulator expands it
// to 32 so PvP and war maps retain entities up to the camera's visual limit.
// The window remains local (65x65); it does not send the entire world.
const viewHalfX = 32
const viewHalfY = 32

func inView(ax, ay, bx, by uint16) bool {
	return absDiff(ax, bx) <= viewHalfX && absDiff(ay, by) <= viewHalfY
}

func (p *Player) hasVisible(id uint16) bool {
	if p == nil || p.Visible == nil {
		return false
	}
	_, ok := p.Visible[id]
	return ok
}

func (p *Player) show(id uint16) {
	if p.Visible == nil {
		p.Visible = make(map[uint16]struct{})
	}
	p.Visible[id] = struct{}{}
}

func (p *Player) hide(id uint16) {
	delete(p.Visible, id)
}

func (w *World) showMob(p *Player, m *Mob) {
	if p == nil || !p.InWorld || m == nil || m.Dead ||
		!w.mobVisibleToPlayer(p, m) || p.hasVisible(m.ID) {
		return
	}
	anct := m.Def.Equip.AncientCodes()
	p.Session.Send(wire.CreateMobVisual(m.ID, m.Def.Name, m.X, m.Y,
		m.Def.Mesh(), anct[:], mobPublicExtendedAt(m, w.now()), m.Affects[:], 0))
	p.show(m.ID)
}

// mobVisibleToPlayer is the visibility boundary for private instance mobs.
// They share the physical map with the rest of the world, but their packets
// must only reach members currently registered in the same stage.  Keeping
// this check in the publication layer prevents a caller that forgets the
// instance context from leaking CreateMob, movement, damage or death packets.
func (w *World) mobVisibleToPlayer(p *Player, m *Mob) bool {
	if p == nil || m == nil {
		return true
	}
	if m.InstanceID != "" {
		return instanceMemberInStage(w.instanceForMob(m), p)
	}
	// Global NPCs/merchants remain public. Hostile public monsters, however,
	// belong to the public gameplay space and must not leak into an event
	// runtime that happens to overlap the same physical coordinates.
	if m.Def != nil && m.Def.IsMonster() && m.SummonerID == 0 {
		return w.gameplaySpaceForPlayer(p) == ""
	}
	if m.SummonerID != 0 {
		owner := w.playerByID(m.SummonerID)
		if owner != nil {
			return w.gameplaySpaceForPlayer(p) == w.gameplaySpaceForPlayer(owner)
		}
	}
	return true
}

// gameplaySpaceForPlayer returns the authoritative runtime that owns a player,
// or the empty public-world space. A shared event still has a private
// interaction space: its RuntimeID is shared by all admitted participants,
// not by unrelated players who happen to occupy the same physical map tile.
// This distinction is shared by publication, combat, support, summons, trade
// and party admission.
func (w *World) gameplaySpaceForPlayer(p *Player) string {
	if w == nil || p == nil {
		return ""
	}
	runtimeID := w.playerRuntimeInstanceID(p.ID)
	if runtimeID == "" {
		return ""
	}
	return runtimeID
}

func (w *World) playersShareGameplaySpace(a, b *Player) bool {
	if a == nil || b == nil {
		return false
	}
	aSpace := w.gameplaySpaceForPlayer(a)
	bSpace := w.gameplaySpaceForPlayer(b)
	if aSpace == "" && bSpace == "" {
		return true
	}
	return aSpace != "" && aSpace == bSpace
}

// privateWaterRuntimeIDForPlayer remains as a compatibility-named wrapper for
// ground-item and legacy visibility code. The underlying lookup is O(1) and
// now uses the same authoritative gameplay-space index as combat.
func (w *World) privateWaterRuntimeIDForPlayer(playerID uint16) string {
	if w == nil || playerID == 0 {
		return ""
	}
	p := w.playersByID[playerID]
	if p == nil {
		// Tests and restore paths may not have a live Player object yet. The
		// runtime index still gives a safe answer for a water-owned ID.
		runtimeID := w.playerRuntimeInstanceID(playerID)
		if runtimeID != "" && isDurablePrivateWaterInstance(w.itemInstances[runtimeID]) {
			return runtimeID
		}
		return ""
	}
	runtimeID := w.gameplaySpaceForPlayer(p)
	if runtimeID != "" && isDurablePrivateWaterInstance(w.itemInstances[runtimeID]) {
		return runtimeID
	}
	return ""
}

// playersVisibleTogether applies the interaction-space boundary
// symmetrically. Public players continue to see one another normally; members
// of the same event runtime can see one another; a runtime member never sees
// or leaks to another runtime/public player merely because coordinates match.
func (w *World) playersVisibleTogether(a, b *Player) bool {
	if a == nil || b == nil || a == b {
		return a != nil && b != nil && a == b
	}
	return w.playersShareGameplaySpace(a, b)
}

func (w *World) groundItemVisibleToPlayer(p *Player, g *GroundItem) bool {
	if p == nil || g == nil {
		return false
	}
	if g.Permanent {
		return true
	}
	// Every non-permanent ground item belongs to exactly one gameplay space.
	// Empty means public; a runtime id means only that runtime. This is generic
	// for Water, Cube, Nightmare, Hell Gate, Uxmal and player drops alike.
	return w.gameplaySpaceForPlayer(p) == strings.TrimSpace(g.InstanceID)
}

func (w *World) hideMob(p *Player, m *Mob, removeType uint32) {
	if p == nil || m == nil || !p.hasVisible(m.ID) {
		return
	}
	p.Session.Send(wire.RemoveMob(m.ID, removeType))
	p.hide(m.ID)
}

func (w *World) showGhostShop(p *Player, shop *GhostShop) {
	if p == nil || !p.InWorld || shop == nil || p.hasVisible(shop.ID) {
		return
	}
	p.Session.Send(wire.CreateMobTrade(shop.ID, shop.Name, shop.X, shop.Y,
		shop.Mesh[:], &shop.Score, shop.Title))
	p.show(shop.ID)
}

func (w *World) hideGhostShop(p *Player, shop *GhostShop) {
	if p == nil || shop == nil || !p.hasVisible(shop.ID) {
		return
	}
	p.Session.Send(wire.RemoveMob(shop.ID, 0))
	p.hide(shop.ID)
}

// playerEnterViewPackets materializes one player with the canonical 7.48+
// packet ABI. HP/MP follows CreateMob so a reused entity ID cannot retain a
// stale dead-state resource cache.
func playerEnterViewPackets(subject *Player) [][]byte {
	if subject == nil || subject.Char == nil {
		return nil
	}
	return [][]byte{
		wire.CreateMobWithGuildRank(subject.ID, subject.Char.Name, subject.X, subject.Y,
			bodyMesh(subject.Char), bodyAncient(subject.Char), wireScoreState(subject.Char),
			subject.Char.Affects[:], 2, subject.Char.GuildID, subject.Char.GuildRank, subject.Char.CP),
		wire.HpMp(subject.ID, wireScoreState(subject.Char)),
		wire.ActionStop(subject.ID, subject.X, subject.Y),
	}
}

func sendPlayerEnterView(observer, subject *Player) {
	if observer == nil || observer.Session == nil {
		return
	}
	for _, pkt := range playerEnterViewPackets(subject) {
		observer.Session.Send(pkt)
	}
}

// refreshAppearance publishes a real avatar appearance change (equipment,
// face transformation, color, refinement, cape, or mount) via MSG_UpdateEquip
// 0x36B. This is the native SendEquip/GridMulticast: it has no coordinates and
// therefore does not interrupt or snap an ongoing walk. CreateMob is reserved
// for entering or re-entering view.
func (w *World) refreshAppearance(subject *Player) {
	if subject == nil || !subject.InWorld || subject.Char == nil {
		return
	}
	w.sendToPlayerView(subject, func() []byte {
		return playerAppearancePacket(subject)
	})
}

func playerAppearancePacket(subject *Player) []byte {
	if subject == nil || subject.Char == nil {
		return nil
	}
	return wire.VisualEquip(subject.ID, bodyMesh(subject.Char), bodyAncient(subject.Char))
}

func (w *World) showPlayerPair(a, b *Player) {
	if a == nil || b == nil || a == b || !a.InWorld || !b.InWorld ||
		a.Char == nil || b.Char == nil || !w.playersVisibleTogether(a, b) {
		return
	}
	if !a.hasVisible(b.ID) {
		sendPlayerEnterView(a, b)
		a.show(b.ID)
		w.sendRemainingPlayerMove(a, b)
	}
	if !b.hasVisible(a.ID) {
		sendPlayerEnterView(b, a)
		b.show(a.ID)
		w.sendRemainingPlayerMove(b, a)
	}
}

func (w *World) hidePlayerPair(a, b *Player) {
	if a == nil || b == nil || a == b {
		return
	}
	if a.hasVisible(b.ID) {
		a.Session.Send(wire.RemoveMob(b.ID, 0))
		a.hide(b.ID)
	}
	if b.hasVisible(a.ID) {
		b.Session.Send(wire.RemoveMob(a.ID, 0))
		b.hide(a.ID)
	}
}

// rematerializePlayerAfterRevive removes the dead representation from other
// clients before applying the new living position and state. RemoveType 0
// does not remove a TMHuman already in ECMOTION_DEAD in the 7.48 client;
// type 3 removes it immediately. refreshPlayerVisibility recreates the player
// only for observers still within the new view window.
func (w *World) rematerializePlayerAfterRevive(subject *Player) {
	if subject == nil || !subject.InWorld || subject.Char == nil || playerCurHP(subject.Char) == 0 {
		return
	}
	for _, observer := range w.nearbyWorldPlayers(subject.X, subject.Y, viewHalfX) {
		if observer == subject || !observer.hasVisible(subject.ID) {
			continue
		}
		observer.Session.Send(wire.RemoveMob(subject.ID, 3))
		observer.hide(subject.ID)
	}
	w.refreshPlayerVisibility(subject)
	w.syncPlayerVitals(subject)
	w.sendToPlayerView(subject, func() []byte {
		return wire.ActionStop(subject.ID, subject.X, subject.Y)
	})
	w.updatePartyMember(subject)
}

// refreshPlayerVisibility applies deltas as a player crosses a view boundary:
// CreateMob on entry and RemoveMob type 0 on exit.
func (w *World) refreshPlayerVisibility(p *Player) {
	if p == nil || !p.InWorld {
		return
	}
	w.updatePlayerSpatial(p)
	nearMobs := w.nearbyMobs(p.X, p.Y, viewHalfX)
	nearMobIDs := make(map[uint16]struct{}, len(nearMobs))
	for _, m := range nearMobs {
		if !w.mobVisibleToPlayer(p, m) {
			continue
		}
		nearMobIDs[m.ID] = struct{}{}
		if !p.hasVisible(m.ID) {
			w.showMob(p, m)
		}
	}
	// Visible contains multiple entity types. Remove only IDs confirmed by the
	// canonical index to be mobs that left the spatial window.
	for id := range p.Visible {
		m := w.mobsByID[id]
		if m == nil {
			continue
		}
		if _, nearby := nearMobIDs[id]; !nearby {
			w.hideMob(p, m, 0)
		}
	}
	nearShops := w.nearbyGhostShops(p.X, p.Y, viewHalfX)
	nearShopIDs := make(map[uint16]struct{}, len(nearShops))
	for _, shop := range nearShops {
		nearShopIDs[shop.ID] = struct{}{}
		if !p.hasVisible(shop.ID) {
			w.showGhostShop(p, shop)
		}
	}
	for id := range p.Visible {
		shop := w.ghostShops[id]
		if shop == nil {
			continue
		}
		if _, nearby := nearShopIDs[id]; !nearby {
			w.hideGhostShop(p, shop)
		}
	}
	nearPlayers := w.nearbyWorldPlayers(p.X, p.Y, viewHalfX)
	nearPlayerIDs := make(map[uint16]struct{}, len(nearPlayers))
	for _, other := range nearPlayers {
		if other == p || !w.playersVisibleTogether(p, other) {
			continue
		}
		nearPlayerIDs[other.ID] = struct{}{}
		w.showPlayerPair(p, other)
	}
	for id := range p.Visible {
		other := w.playersByID[id]
		if other == nil || other == p {
			continue
		}
		if _, nearby := nearPlayerIDs[id]; !nearby || !w.playersVisibleTogether(p, other) {
			w.hidePlayerPair(p, other)
		}
	}
	nearGround := w.nearbyGroundItems(p.X, p.Y, viewHalfX)
	nearGroundIDs := make(map[uint16]struct{}, len(nearGround))
	for _, g := range nearGround {
		if !w.groundItemVisibleToPlayer(p, g) {
			continue
		}
		nearGroundIDs[g.ID] = struct{}{}
		if !p.hasVisible(g.ID) {
			p.Session.Send(w.groundItemCreatePacket(g))
			p.show(g.ID)
		}
	}
	for id := range p.Visible {
		g := w.groundItems[id]
		if g == nil {
			continue
		}
		if _, nearby := nearGroundIDs[id]; !nearby {
			p.Session.Send(wire.RemoveItem(uint32(g.ID)))
			p.hide(g.ID)
		}
	}
}

func (w *World) publishGhostShopSpawn(shop *GhostShop) {
	if shop == nil {
		return
	}
	for _, p := range w.nearbyWorldPlayers(shop.X, shop.Y, viewHalfX) {
		w.showGhostShop(p, shop)
	}
}

func (w *World) publishGhostShopRemove(shop *GhostShop) {
	if shop == nil {
		return
	}
	for _, p := range w.nearbyWorldPlayers(shop.X, shop.Y, viewHalfX) {
		if p.hasVisible(shop.ID) {
			p.Session.Send(wire.RemoveMob(shop.ID, 0))
			p.hide(shop.ID)
		}
	}
}

func (w *World) publishGhostShopItemSold(shop *GhostShop, pos uint32) {
	if shop == nil {
		return
	}
	for _, p := range w.nearbyWorldPlayers(shop.X, shop.Y, viewHalfX) {
		if p.hasVisible(shop.ID) {
			// The auto-shop window was opened for the clone's virtual ID.
			p.Session.Send(wire.ItemSold(shop.ID, pos))
		}
	}
}

// publishMobSpawn materializes a new instance only for nearby players.
func (w *World) publishMobSpawn(m *Mob) {
	w.registerMobSpatial(m)
	w.publishRegisteredMobSpawn(m)
}

// publishRegisteredMobSpawn publishes an entity that has already crossed all
// fallible registration steps. Boss spawn uses this to avoid exposing an
// orphan when RegisterBoss rejects its profile.
func (w *World) publishRegisteredMobSpawn(m *Mob) {
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if w.mobVisibleToPlayer(p, m) {
			w.showMob(p, m)
		}
	}
}

// publishMobMove sends movement only to players already seeing the mob and
// updates clients whose view changed because the mob moved.
func (w *World) publishMobMove(m *Mob, oldX, oldY uint16, speed uint32) {
	w.moveMobSpatial(m, oldX, oldY)
	observers := make(map[uint16]*Player)
	for _, p := range w.nearbyWorldPlayers(oldX, oldY, viewHalfX) {
		observers[p.ID] = p
	}
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		observers[p.ID] = p
	}
	for _, p := range observers {
		if !w.mobVisibleToPlayer(p, m) {
			if p.hasVisible(m.ID) {
				w.hideMob(p, m, 0)
			}
			continue
		}
		wasVisible := p.hasVisible(m.ID)
		nowVisible := inView(p.X, p.Y, m.X, m.Y)
		switch {
		case wasVisible && nowVisible:
			p.Session.Send(wire.MobMove(m.ID, oldX, oldY, m.X, m.Y, speed))
		case wasVisible && !nowVisible:
			w.hideMob(p, m, 0)
		case !wasVisible && nowVisible:
			w.showMob(p, m)
		}
	}
}

// publishPlayerMove replicates only a real destination change. Native TMSrv
// preserves Route[24] and publishes it with the segment's reported origin.
// Dropping that route made each observer recalculate the path, causing small
// corrections on turns, slopes, and destination changes.
func (w *World) publishPlayerMove(player *Player, fromX, fromY, targetX, targetY uint16, route []byte) {
	if player == nil || !player.InWorld || player.Char == nil {
		return
	}
	if fromX == targetX && fromY == targetY {
		return
	}
	// BASE_GetSpeed 7.48: low nibble of AttackRun, clamped to 1..7. Using the
	// score prevents speed hacks and preserves the appearance of run buffs.
	speed := uint32(playerAttackRun(player.Char) & 0x0F)
	// The player is still indexed in the previous cell. Query the union of
	// both windows so existing observers receive the segment; new observers
	// are materialized by the subsequent refresh.
	observers := make(map[uint16]*Player)
	if player.MovePublished {
		for _, observer := range w.nearbyWorldPlayers(
			player.MovePublishedTargetX, player.MovePublishedTargetY, viewHalfX) {
			observers[observer.ID] = observer
		}
	}
	for _, observer := range w.nearbyWorldPlayers(fromX, fromY, viewHalfX) {
		observers[observer.ID] = observer
	}
	for _, observer := range w.nearbyWorldPlayers(targetX, targetY, viewHalfX) {
		observers[observer.ID] = observer
	}
	for _, observer := range observers {
		if observer != player && !w.playersVisibleTogether(observer, player) {
			w.hidePlayerPair(observer, player)
			continue
		}
		if observer != player && observer.hasVisible(player.ID) {
			observer.Session.Send(wire.PlayerMove(player.ID, fromX, fromY, targetX, targetY, speed, route))
		}
	}
	player.MovePublished = true
	player.MovePublishedStartX, player.MovePublishedStartY = fromX, fromY
	player.MovePublishedTargetX, player.MovePublishedTargetY = targetX, targetY
	player.MovePublishedRoute = [maxMovementRouteBytes]byte{}
	for index, step := range route {
		if index >= len(player.MovePublishedRoute) || step == 0 {
			break
		}
		player.MovePublishedRoute[index] = step
	}
}

func (w *World) sendRemainingPlayerMove(observer, subject *Player) {
	if observer == nil || subject == nil || observer == subject || !subject.MovePublished ||
		subject.MoveAuthorityStep >= len(subject.MoveAuthorityRoute) {
		return
	}
	route := subject.MoveAuthorityRoute[subject.MoveAuthorityStep:]
	observer.Session.Send(wire.PlayerMove(subject.ID, subject.X, subject.Y,
		subject.MovePublishedTargetX, subject.MovePublishedTargetY,
		uint32(playerAttackRun(subject.Char)&0x0F), route))
}

// publishPlayerStop ends a route without ActionStop/Effect=1. That effect is
// for spawn, teleport, and hard correction; sending it on every stop snapped
// the remote avatar. The route ends by itself at the destination. An
// intermediate stop receives one Effect=0 reorientation.
func (w *World) publishPlayerStop(player *Player) {
	if player == nil || !player.InWorld || player.Char == nil || !player.MovePublished {
		return
	}
	startX, startY := player.MovePublishedStartX, player.MovePublishedStartY
	plannedX, plannedY := player.MovePublishedTargetX, player.MovePublishedTargetY
	if player.X != plannedX || player.Y != plannedY {
		speed := uint32(playerAttackRun(player.Char) & 0x0F)
		observers := make(map[uint16]*Player)
		for _, point := range [][2]uint16{{startX, startY}, {plannedX, plannedY}, {player.X, player.Y}} {
			for _, observer := range w.nearbyWorldPlayers(point[0], point[1], viewHalfX) {
				observers[observer.ID] = observer
			}
		}
		for _, observer := range observers {
			if observer != player && !w.playersVisibleTogether(observer, player) {
				w.hidePlayerPair(observer, player)
				continue
			}
			if observer != player && observer.hasVisible(player.ID) {
				observer.Session.Send(wire.PlayerMove(
					player.ID, startX, startY, player.X, player.Y, speed, nil))
			}
		}
	}
	clearPublishedPlayerMove(player)
}

func clearPublishedPlayerMove(player *Player) {
	if player == nil {
		return
	}
	player.MovePublished = false
	player.MovePublishedStartX = 0
	player.MovePublishedStartY = 0
	player.MovePublishedTargetX = 0
	player.MovePublishedTargetY = 0
	player.MovePublishedRoute = [maxMovementRouteBytes]byte{}
	player.MoveAuthorityRoute = nil
	player.MoveAuthorityStep = 0
	player.MoveAuthorityCatchupSteps = 0
	player.MoveAuthorityX = 0
	player.MoveAuthorityY = 0
	player.MoveAuthorityStartedAt = time.Time{}
	player.MoveAuthorityStepInterval = 0
}

func (w *World) publishMobDeath(m *Mob, killerID uint16, progressByPlayer map[*Player]clientExperienceState) {
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if !w.mobVisibleToPlayer(p, m) || !p.hasVisible(m.ID) {
			continue
		}
		p.Session.Send(mobDeathPacket(p, m.ID, killerID, progressByPlayer))
		p.Session.Send(wire.RemoveMob(m.ID, 1))
		p.hide(m.ID)
	}
}

func mobDeathPacket(recipient *Player, killedID, killerID uint16,
	progressByPlayer map[*Player]clientExperienceState) []byte {
	progress := clientExperienceState{}
	if recipient != nil && recipient.Char != nil {
		progress = clientExperienceState{exp: recipient.Char.Exp, hold: recipient.Char.Hold}
	}
	if personal, ok := progressByPlayer[recipient]; ok {
		progress = personal
	}
	return wire.CNFMobKill(killedID, killerID, progress.hold, progress.exp)
}

// publishPlayerDeath sends each recipient their own total experience. The
// client calls SetMyHumanExp when the killer is the recipient or a party
// member; reusing the victim's experience would display the wrong total.
// PvP death grants neither experience nor gold.
func (w *World) publishPlayerDeath(victim *Player, killerID uint16) {
	if victim == nil {
		return
	}
	// Death invalidates the server-owned route before any later client packet.
	clearAirMove(victim)
	if !victim.InWorld {
		return
	}
	for _, recipient := range w.nearbyWorldPlayers(victim.X, victim.Y, viewHalfX) {
		if !w.playersVisibleTogether(recipient, victim) {
			continue
		}
		if recipient != victim && !recipient.hasVisible(victim.ID) {
			continue
		}
		recipient.Session.Send(playerDeathPacket(recipient, victim, killerID))
	}
}

func playerDeathPacket(recipient, victim *Player, killerID uint16) []byte {
	var exp, hold uint32
	if recipient != nil && recipient.Char != nil {
		exp = recipient.Char.Exp
		hold = recipient.Char.Hold
	}
	return wire.CNFMobKill(victim.ID, killerID, hold, exp)
}

// publishMobRemoval removes a mob that did not die in combat (spawn rollback,
// reload, or administrative cleanup). Unlike publishMobDeath, it sends no
// CNFMobKill and creates no false visual hit in the client.
func (w *World) publishMobRemoval(m *Mob) {
	if m == nil {
		return
	}
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if !w.mobVisibleToPlayer(p, m) || !p.hasVisible(m.ID) {
			continue
		}
		p.Session.Send(wire.RemoveMob(m.ID, 0))
		p.hide(m.ID)
	}
}

func (w *World) sendToMobView(m *Mob, build func() []byte) {
	if m == nil {
		return
	}
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if w.mobVisibleToPlayer(p, m) && p.hasVisible(m.ID) {
			p.Session.Send(build())
		}
	}
}

// sendToMobViewProtocol is the mixed-client counterpart of sendToMobView. Use
// it when a payload embeds a protocol-specific structure such as Score; the
// ordinary helper remains preferable for packets whose ABI is shared.
func (w *World) sendToMobViewProtocol(m *Mob, build func(*Player) []byte) {
	if m == nil {
		return
	}
	for _, observer := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if w.mobVisibleToPlayer(observer, m) && observer.hasVisible(m.ID) {
			observer.Session.Send(build(observer))
		}
	}
}

func (w *World) sendToPlayerView(subject *Player, build func() []byte) {
	if subject == nil || !subject.InWorld {
		return
	}
	for _, p := range w.nearbyWorldPlayers(subject.X, subject.Y, viewHalfX) {
		if !w.playersVisibleTogether(p, subject) {
			continue
		}
		if p == subject || p.hasVisible(subject.ID) {
			p.Session.Send(build())
		}
	}
}

// sendToPlayerViewProtocol lets one authoritative mutation fan out different
// packet layouts to stock and source observers without duplicating gameplay.
func (w *World) sendToPlayerViewProtocol(subject *Player, build func(*Player) []byte) {
	if subject == nil || !subject.InWorld {
		return
	}
	for _, observer := range w.nearbyWorldPlayers(subject.X, subject.Y, viewHalfX) {
		if !w.playersVisibleTogether(observer, subject) {
			continue
		}
		if observer == subject || observer.hasVisible(subject.ID) {
			observer.Session.Send(build(observer))
		}
	}
}

// publishPlayerAffects feeds two distinct 7.48 client channels: 0x336 is
// public and calls TMHuman::CheckAffect; 0x3B9 is private and updates only
// the session owner's icons and timers.
func (w *World) publishPlayerAffects(subject *Player) {
	if subject == nil || subject.Char == nil || subject.Session == nil {
		return
	}
	w.sendToPlayerViewProtocol(subject, func(observer *Player) []byte {
		return observedPlayerScorePacket(observer, subject)
	})
	subject.Session.Send(playerAffectsPacket(subject))
}

// syncPlayerVitals keeps character HP/MP identical for the owner and all
// observers. Besides the bars, SetHpMp with HP>0 makes the 7.48 client clear
// ECMOTION_DEAD/m_cDie, which is required during revival.
func (w *World) syncPlayerVitals(subject *Player) {
	if subject == nil || subject.Char == nil {
		return
	}
	w.sendToPlayerViewProtocol(subject, func(observer *Player) []byte {
		return wire.HpMp(subject.ID, wireScoreState(subject.Char))
	})
}

// syncPlayerVitalsToObservers sends 0x181 to players who SEE the subject,
// excluding the subject. It serves flows that sent a private 0x336 to the owner.
//
// This avoids bar flicker: the rebuilt client's wide handler copies the
// uint32 tail to its sidecar before calling the native handler. The 7.48
// analysis confirmed that all four branches and the fall-through converge
// on the same call. Each vitals packet redraws the native bar, so sending
// 0x336 then 0x181 to the same player redraws it twice.
//
// The private 0x336 already carries HP/MP in legacy WORDs and the wide tail,
// so the owner loses nothing. Observers still receive 0x181 because they
// never received the private 0x336.
func (w *World) syncPlayerVitalsToObservers(subject *Player) {
	if subject == nil || subject.Char == nil || !subject.InWorld {
		return
	}
	for _, p := range w.nearbyWorldPlayers(subject.X, subject.Y, viewHalfX) {
		if p != subject && w.playersVisibleTogether(p, subject) && p.hasVisible(subject.ID) {
			p.Session.Send(wire.HpMp(subject.ID, wireScoreState(subject.Char)))
		}
	}
}

// syncPlayerScoreAndVitals completes a state change that also passed through
// the attack flow. Packet 0x181 calls the legacy client handler directly and
// redraws the HP/MP WORDs for one frame; ordinary casts do not need it.
// Extended 0x336 updates the uint32 sidecar without that flicker. Flows that
// need bar or pose changes (damage, healing, death, revival) call
// syncPlayerVitals explicitly.
func (w *World) syncPlayerScoreAndVitals(subject *Player) {
	if subject == nil || subject.Char == nil {
		return
	}
	w.sendToPlayerViewProtocol(subject, func(observer *Player) []byte {
		return observedPlayerScorePacket(observer, subject)
	})
}

// syncPlayerChaos publishes the PK/chaos byte embedded in CreateMob.
// UpdateEtc has no CP field in protocol 7.54; sending only 0x337 would leave
// observers seeing the old name color. The packet keeps the same position and
// has no Action, so it does not restart walking.
func (w *World) syncPlayerChaos(subject *Player) {
	if subject == nil || !subject.InWorld || subject.Char == nil {
		return
	}
	w.sendToPlayerViewProtocol(subject, func(observer *Player) []byte {
		return wire.CreateMobWithGuildRank(subject.ID, subject.Char.Name, subject.X, subject.Y,
			bodyMesh(subject.Char), bodyAncient(subject.Char), wireScoreState(subject.Char),
			subject.Char.Affects[:], 2, subject.Char.GuildID, subject.Char.GuildRank, subject.Char.CP)
	})
}

func (w *World) publishItemSpawn(g *GroundItem) {
	for _, p := range w.nearbyWorldPlayers(g.X, g.Y, viewHalfX) {
		if w.groundItemVisibleToPlayer(p, g) && !p.hasVisible(g.ID) {
			p.Session.Send(w.groundItemCreatePacket(g))
			p.show(g.ID)
		}
	}
}

func (w *World) publishItemRemove(g *GroundItem) {
	if g == nil {
		return
	}
	for _, p := range w.nearbyWorldPlayers(g.X, g.Y, viewHalfX) {
		if p.hasVisible(g.ID) {
			p.Session.Send(wire.RemoveItem(uint32(g.ID)))
			p.hide(g.ID)
		}
	}
}
