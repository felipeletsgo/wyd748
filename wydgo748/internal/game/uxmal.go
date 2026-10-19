package game

import (
	"fmt"
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// uxmalScheduleAllowed reproduces TMSrv's three four-minute windows: 16-19,
// 36-39 and 56-59. Validation uses the World clock so tests stay deterministic
// and never accepts a minute sent by the client.
func uxmalScheduleAllowed(cfg *model.VolatileInstance, now time.Time) bool {
	if cfg == nil || len(cfg.Schedule) == 0 {
		return true
	}
	_, ok := instanceScheduleEnd(cfg, now)
	return ok
}

func uxmalInsideEntry(cfg *model.VolatileInstance, x, y uint16) bool {
	if cfg == nil || cfg.Uxmal == nil {
		return false
	}
	for _, area := range cfg.Uxmal.EntryAreas {
		if x >= area.MinX && x <= area.MaxX && y >= area.MinY && y <= area.MaxY {
			return true
		}
	}
	return false
}

func (w *World) validateUxmalConfig() error {
	cfg := w.uxmal
	if cfg == nil || cfg.Uxmal == nil {
		return fmt.Errorf("missing template")
	}
	if cfg.Uxmal.Version != 0 && cfg.Uxmal.Version != 759 {
		return fmt.Errorf("unsupported Rune Track version %d", cfg.Uxmal.Version)
	}
	if cfg.Uxmal.Version == 759 {
		seen := map[int]bool{}
		for _, g := range cfg.Uxmal.Generators {
			if runeGeneratorRoom(g.Label) < 0 || seen[g.Label] || g.MaxNumMob <= 0 || g.MinGroup < 0 || g.MaxGroup < g.MinGroup || g.Segments[0].X == 0 || g.Segments[0].Y == 0 {
				return fmt.Errorf("invalid Rune Track generator %d", g.Label)
			}
			seen[g.Label] = true
			for role, name := range []string{g.Leader, g.Follower} {
				if name == "" && role == 1 {
					continue
				}
				def := w.npcDefByName(name)
				if !strings.HasPrefix(name, "RT_") || len(name) > 12 || def == nil || def.Score == nil || def.Score.MaxHP == 0 {
					return fmt.Errorf("generator %d requires an event-only NPC: %q", g.Label, name)
				}
			}
		}
		if len(seen) != 327 {
			return fmt.Errorf("Rune Track requires all 327 generators, got %d", len(seen))
		}
	}
	if !strings.EqualFold(strings.TrimSpace(cfg.StateMachine), "uxmal") {
		return fmt.Errorf("stateMachine must be uxmal")
	}
	if strings.TrimSpace(cfg.Uxmal.NPC) == "" {
		return fmt.Errorf("missing entry NPC")
	}
	entryNPC := w.npcDefByName(cfg.Uxmal.NPC)
	if entryNPC == nil || entryNPC.IsMonster() || entryNPC.Score == nil {
		return fmt.Errorf("invalid entry NPC %q", cfg.Uxmal.NPC)
	}
	if cfg.Uxmal.TicketItem == 0 {
		return fmt.Errorf("missing ticket")
	}
	if _, ok := w.items[cfg.Uxmal.TicketItem]; !ok {
		return fmt.Errorf("ticket %d does not exist in the catalog", cfg.Uxmal.TicketItem)
	}
	if len(cfg.Uxmal.EntryAreas) == 0 {
		return fmt.Errorf("missing entry area")
	}
	for i, area := range cfg.Uxmal.EntryAreas {
		if area.MinX == 0 || area.MinY == 0 || area.MaxX < area.MinX || area.MaxY < area.MinY {
			return fmt.Errorf("invalid entry area %d", i)
		}
	}
	if len(cfg.Stages) != 7 || len(cfg.Uxmal.RoomPositions) != 7 ||
		len(cfg.Uxmal.MaxParties) != 7 || len(cfg.Uxmal.Runes) != 7 ||
		len(cfg.Uxmal.TicketNextSanc) != 7 {
		return fmt.Errorf("seven rooms are required")
	}
	for room, stage := range cfg.Stages {
		if stage.AreaRadius <= 0 || stage.SpawnX == 0 || stage.SpawnY == 0 || (cfg.Uxmal.Version != 759 && len(stage.Spawns) == 0) {
			return fmt.Errorf("room %d is incomplete", room)
		}
		if !w.terrain.Walkable(stage.SpawnX, stage.SpawnY) {
			return fmt.Errorf("room %d has a blocked spawn", room)
		}
		limit := 3
		if cfg.Uxmal.Version == 759 && room == 4 {
			limit = 4
		}
		if len(cfg.Uxmal.RoomPositions[room]) < cfg.Uxmal.MaxParties[room] || cfg.Uxmal.MaxParties[room] < 1 || cfg.Uxmal.MaxParties[room] > limit {
			return fmt.Errorf("room %d has insufficient slots or invalid maxParties", room)
		}
		for _, spawn := range stage.Spawns {
			def := w.npcDefByName(spawn.NPC)
			if spawn.Count <= 0 || def == nil || !def.IsMonster() || def.Score == nil {
				return fmt.Errorf("room %d uses missing hostile template %q", room, spawn.NPC)
			}
		}
		for slot, pos := range cfg.Uxmal.RoomPositions[room] {
			if pos.X == 0 || pos.Y == 0 || (slot == 2 && cfg.Uxmal.MaxParties[room] < 3) {
				// The third coordinate of room +0 stays filled to keep the
				// format, but its capacity remains two parties.
				if slot < cfg.Uxmal.MaxParties[room] {
					return fmt.Errorf("room %d slot %d is invalid", room, slot)
				}
			}
			if slot < cfg.Uxmal.MaxParties[room] && !w.terrain.Walkable(pos.X, pos.Y) {
				return fmt.Errorf("room %d slot %d uses blocked terrain", room, slot)
			}
		}
		if len(cfg.Uxmal.Runes[room]) == 0 {
			return fmt.Errorf("room %d has no runes", room)
		}
		for _, runeItem := range cfg.Uxmal.Runes[room] {
			if runeItem == 0 {
				return fmt.Errorf("room %d has an empty rune", room)
			}
			if _, ok := w.items[runeItem]; !ok {
				return fmt.Errorf("room %d uses nonexistent rune %d", room, runeItem)
			}
		}
		if next := cfg.Uxmal.TicketNextSanc[room]; next < 0 || next > 6 {
			return fmt.Errorf("room %d has invalid track refinement %d", room, next)
		}
	}
	return nil
}

func (w *World) isUxmalNPC(m *Mob) bool {
	return w != nil && w.uxmal != nil && w.uxmal.Uxmal != nil && m != nil && m.Def != nil &&
		strings.EqualFold(m.Def.Name, w.uxmal.Uxmal.NPC)
}

// handleUxmalNPC runs before the ordinary quest allowlist. Clicking Uxmal
// consumes a Rune Track server-side and never opens the generic shop/quest.
func (w *World) handleUxmalNPC(s *net.Session, p *Player, m *Mob) bool {
	if !w.isUxmalNPC(m) {
		return false
	}
	w.startUxmal(s, p)
	return true
}

func uxmalTicketRoom(item model.Item) int {
	room := itemSanc(item)
	if room < 0 {
		return 0
	}
	if room > 6 {
		return 6
	}
	return room
}

func (w *World) uxmalTicketSlot(ch *model.Char, room int) int {
	if ch == nil || w.uxmal == nil || w.uxmal.Uxmal == nil {
		return -1
	}
	for slot := 0; slot < model.PlayerCarrySlots; slot++ {
		item := ch.Inv[slot]
		if item.Index == w.uxmal.Uxmal.TicketItem && uxmalTicketRoom(item) == room {
			return slot
		}
	}
	return -1
}

func (w *World) uxmalMembers(leader *Player) ([]*Player, bool) {
	if leader == nil || leader.Char == nil || !leader.InWorld || playerCurHP(leader.Char) == 0 {
		return nil, false
	}
	if !uxmalInsideEntry(w.uxmal, leader.X, leader.Y) {
		return nil, false
	}
	if leader.Party == nil {
		return []*Player{leader}, true
	}
	if leader.Party.leader() != leader {
		return nil, false
	}
	members := make([]*Player, 0, len(leader.Party.Members))
	for _, member := range leader.Party.Members {
		if member == nil || !member.InWorld || member.Char == nil || playerCurHP(member.Char) == 0 ||
			!uxmalInsideEntry(w.uxmal, member.X, member.Y) {
			continue
		}
		members = append(members, member)
	}
	return members, len(members) > 0 && members[0] == leader
}

func (w *World) uxmalFreeSlot(room int) int {
	if w == nil || w.uxmal == nil || w.uxmal.Uxmal == nil || room < 0 || room >= 7 {
		return -1
	}
	limit := w.uxmal.Uxmal.MaxParties[room]
	for slot := 0; slot < limit; slot++ {
		runtimeID := fmt.Sprintf("uxmal:%d:%d", room, slot)
		if inst := w.itemInstances[runtimeID]; inst == nil {
			return slot
		}
	}
	return -1
}

func (w *World) uxmalStage(room, slot int) (model.VolatileInstanceStage, bool) {
	if w == nil || w.uxmal == nil || w.uxmal.Uxmal == nil || room < 0 || room >= 7 || slot < 0 || slot >= 3 {
		return model.VolatileInstanceStage{}, false
	}
	if room >= len(w.uxmal.Stages) || slot >= len(w.uxmal.Uxmal.RoomPositions[room]) {
		return model.VolatileInstanceStage{}, false
	}
	stage := w.uxmal.Stages[room]
	position := w.uxmal.Uxmal.RoomPositions[room][slot]
	stage.X, stage.Y = position.X, position.Y
	stage.SpawnX, stage.SpawnY = position.X, position.Y
	return stage, true
}

func (w *World) startUxmal(s *net.Session, p *Player) {
	if s == nil || p == nil || p.Char == nil || w.uxmal == nil || w.uxmal.Uxmal == nil {
		return
	}
	if w.uxmal.Uxmal.Version == 759 {
		w.registerRuneTrack(s, p)
		return
	}
	now := w.now()
	if !uxmalScheduleAllowed(w.uxmal, now) {
		s.Send(wire.MessagePanel("Rune Track is closed. Return at minutes 16, 36, or 56."))
		return
	}
	members, ok := w.uxmalMembers(p)
	if !ok {
		s.Send(wire.MessagePanel("The leader and every member must be at the Rune Track entrance."))
		return
	}
	if len(members) > w.uxmal.MaxPlayers && w.uxmal.MaxPlayers > 0 {
		s.Send(wire.MessagePanel("Rune Track accepts at most 13 players."))
		return
	}
	if w.itemInstanceForPlayer(p.ID) != nil {
		s.Send(wire.MessagePanel("You are already inside a Rune Track."))
		return
	}
	room := uxmalTicketRoom(model.Item{})
	ticketSlot := -1
	for candidate := 0; candidate <= 6; candidate++ {
		if slot := w.uxmalTicketSlot(p.Char, candidate); slot >= 0 {
			room, ticketSlot = candidate, slot
			break
		}
	}
	if ticketSlot < 0 {
		s.Send(wire.MessagePanel("You do not have a Rune Track ticket."))
		return
	}
	for _, member := range members {
		if w.itemInstanceForPlayer(member.ID) != nil {
			s.Send(wire.MessagePanel("You or a party member is already inside another instance."))
			return
		}
	}
	partySlot := w.uxmalFreeSlot(room)
	if partySlot < 0 {
		s.Send(wire.MessagePanel("The quest is currently occupied."))
		return
	}
	stage, ok := w.uxmalStage(room, partySlot)
	if !ok {
		s.Send(wire.MessagePanel("The Rune Track room is not configured."))
		return
	}
	oldTicket := p.Char.Inv[ticketSlot]
	cfg := cfgCopy(*w.uxmal)
	cfg.Stages = []model.VolatileInstanceStage{stage}
	cfg.ID = fmt.Sprintf("uxmal-room-%d", room)
	cfg.Name = stage.Name
	inst := &ItemInstance{Config: cfg, RuntimeID: fmt.Sprintf("uxmal:%d:%d", room, partySlot),
		LeaderID: p.ID, CurrentStage: 0, State: "running", UxmalRoom: room, UxmalSlot: partySlot,
		MobIDs: make(map[uint16]struct{}), NPCIDs: make(map[uint16]struct{}),
		MobQuadrants: make(map[uint16]uint8), HellGateLichIDs: make(map[uint8]uint16)}
	for _, member := range members {
		inst.MemberIDs = append(inst.MemberIDs, member.ID)
	}
	consumeOne(&p.Char.Inv[ticketSlot])
	w.itemInstances[inst.RuntimeID] = inst
	// Opening uses the same transactional path as other instances: reserve the
	// wave, move the party, and only then publish the mobs. The spawn cannot
	// collide with players still at the entrance, and a persistence failure
	// reverts position, item and instance state together.
	failed := func() {
		delete(w.itemInstances, inst.RuntimeID)
		p.Char.Inv[ticketSlot] = oldTicket
		if !s.IsClosed() {
			s.Send(wire.SendItem(p.ID, placeInv, byte(ticketSlot), oldTicket))
			s.Send(wire.MessagePanel("The Rune Track room cannot be opened right now."))
		}
	}
	opened := func() {
		for mobID := range inst.MobIDs {
			if mob := w.mobsByID[mobID]; mob != nil {
				w.publishMobSpawn(mob)
			}
		}
		for _, member := range members {
			w.refreshPlayerVisibility(member)
			w.sendToPlayerView(member, func() []byte { return wire.ActionStop(member.ID, member.X, member.Y) })
		}
		if !s.IsClosed() {
			s.Send(wire.SendItem(p.ID, placeInv, byte(ticketSlot), p.Char.Inv[ticketSlot]))
		}
		w.sendItemInstanceStageStatus(inst)
		log.Printf("[#%d] UXMAL room=%d slot=%d leader=%s members=%d", s.ID, room, partySlot, p.Char.Name, len(members))
	}
	if spawned, _ := w.spawnItemInstanceStageThen(inst, 0, now, true, false, opened, failed); !spawned {
		failed()
	}
}

func (w *World) grantUxmalRewards(inst *ItemInstance, now time.Time) bool {
	if inst == nil || inst.RewardGranted || w.uxmal == nil || w.uxmal.Uxmal == nil {
		return inst != nil && inst.RewardGranted
	}
	runes := w.uxmal.Uxmal.Runes[inst.UxmalRoom]
	return w.grantRuneRewards(inst, now, inst.MemberIDs, inst.LeaderID, runes, 0, func() {
		inst.RewardGranted = true
		inst.ExitAt = now.Add(10 * time.Second)
		inst.ExitDeadline = inst.ExitAt
	})
}

// grantRuneRewards persists one party's complete grant, including any floor
// fallback, without completing the other parties sharing its room.
func (w *World) grantRuneRewards(inst *ItemInstance, now time.Time, memberIDs []uint16,
	leaderID uint16, runes []uint16, gold uint32, complete func()) bool {
	if len(runes) == 0 || w.criticalInstanceReserved() {
		return false
	}
	leader := w.playersByID[leaderID]
	validLeader := false
	uniqueMembers := make(map[uint16]bool, len(memberIDs))
	for _, id := range memberIDs {
		p := w.playersByID[id]
		if uniqueMembers[id] || p == nil || !p.InWorld || p.Char == nil || p.Account == nil || w.criticalPlayerReserved(p) {
			return false
		}
		uniqueMembers[id] = true
		validLeader = validLeader || p == leader
	}
	if !validLeader {
		return false
	}
	type snapshot struct {
		player *Player
		char   model.Char
	}
	snapshots := make([]snapshot, 0, len(inst.MemberIDs))
	oldInstance := cloneRuneInstance(inst)
	accounts := make([]*model.Account, 0, len(inst.MemberIDs))
	seen := make(map[*model.Account]struct{})
	rollback := func() {
		*inst = *oldInstance
		for _, old := range snapshots {
			*old.player.Char = old.char
		}
	}
	ground := make([]struct {
		p    *Player
		item model.Item
	}, 0)
	for _, id := range memberIDs {
		member := w.playersByID[id]
		if member == nil || !member.InWorld || member.Char == nil || member.Account == nil {
			continue
		}
		snapshots = append(snapshots, snapshot{player: member, char: cloneCharacterState(member.Char)})
		if _, exists := seen[member.Account]; !exists {
			seen[member.Account] = struct{}{}
			accounts = append(accounts, member.Account)
		}
		rune, err := materializeItem(model.Item{Index: runes[w.intn(len(runes))]})
		if err != nil {
			rollback()
			return false
		}
		if slot := firstFreeVisibleInventorySlot(member.Char); slot >= 0 {
			member.Char.Inv[slot] = rune
		} else {
			ground = append(ground, struct {
				p    *Player
				item model.Item
			}{member, rune})
		}
	}
	if inst.UxmalRoom < 6 {
		leader := w.playersByID[leaderID]
		if leader == nil || leader.Char == nil {
			rollback()
			return false
		}
		leaderSnapshot := false
		for _, old := range snapshots {
			if old.player == leader {
				leaderSnapshot = true
				break
			}
		}
		if !leaderSnapshot {
			rollback()
			return false
		}
		if leader != nil && leader.Char != nil {
			next := model.Item{Index: w.uxmal.Uxmal.TicketItem}
			if !setItemSanc(&next, w.uxmal.Uxmal.TicketNextSanc[inst.UxmalRoom]) {
				rollback()
				return false
			}
			next, err := materializeItem(next)
			if err != nil {
				rollback()
				return false
			}
			if slot := firstFreeVisibleInventorySlot(leader.Char); slot >= 0 {
				leader.Char.Inv[slot] = next
			} else {
				ground = append(ground, struct {
					p    *Player
					item model.Item
				}{leader, next})
			}
		}
	}
	if gold != 0 {
		leader := w.playersByID[leaderID]
		if leader == nil || leader.Char == nil {
			rollback()
			return false
		}
		leader.Char.Gold = uint32(min(uint64(2_000_000_000), uint64(leader.Char.Gold)+uint64(gold)))
	}
	// Ground rewards are part of the same authoritative transition as the
	// inventory rewards.  Materialize them before the account commit, but do
	// not publish them yet.  That lets us abort the whole reward if one floor
	// item cannot be allocated (invalid catalog entry or exhausted ground-ID
	// range) instead of consuming the room reward and merely logging a loss.
	createdGround := make([]*GroundItem, 0, len(ground))
	rollbackGround := func() {
		for _, item := range createdGround {
			if item != nil {
				item.criticalDrop = 0
				w.unregisterGroundItem(item)
			}
		}
		createdGround = nil
	}
	for _, drop := range ground {
		item := w.createGroundDropCell(drop.p.X, drop.p.Y, drop.item, false, inst.RuntimeID, ownerOf(drop.p))
		if item == nil {
			rollbackGround()
			rollback()
			return false
		}
		createdGround = append(createdGround, item)
		if w.criticalWrites != nil {
			// Reserve the unpublished allocation before actor visibility can
			// discover it. No client may pick it up before commit confirmation.
			item.criticalDrop = 1
		}
	}
	groundContexts := make([]GroundItem, len(createdGround))
	for i, item := range createdGround {
		groundContexts[i] = *item
	}
	checkGround := func() error {
		for i, item := range createdGround {
			if w.groundItems[item.ID] != item || *item != groundContexts[i] {
				return errCriticalPersistenceIdentity
			}
		}
		return nil
	}
	// Stage the runtime completion marker with the detached grant. Uxmal is
	// process-local, so this does not make it a restart-resumable event.
	complete()
	// Unpublished ground rewards stay registered but invisible while pending.
	admitted := w.commitInstanceMutation("Uxmal reward", accounts, rollback, func() {
		for _, item := range createdGround {
			item.criticalDrop = 0
			w.publishItemSpawn(item)
		}
		for _, old := range snapshots {
			if member := old.player; member.InWorld && member.Session != nil && !member.Session.IsClosed() {
				for slot, item := range member.Char.Inv {
					if item != old.char.Inv[slot] {
						member.Session.Send(wire.SendItem(member.ID, placeInv, byte(slot), item))
					}
				}
				member.Session.Send(wire.UpdateEtc(member.ID, *member.Char))
				member.Session.Send(wire.MessagePanel("Rune Track completed: rune received."))
			}
		}
	}, func() {
		// Same follow-up as an inline save failure: keep the room open.
		rollbackGround()
		if inst.RuneTrack == nil {
			inst.Remaining = 1
		}
		log.Printf("UXMAL room=%d: reward pending after persistence rejection", inst.UxmalRoom)
	}, checkGround)
	if !admitted {
		rollbackGround()
	}
	return admitted
}

func (w *World) onUxmalMobKilled(m *Mob, now time.Time) {
	inst := w.itemInstances[m.InstanceID]
	if inst != nil && inst.RuneTrack != nil {
		return
	}
	if inst == nil || inst.RewardGranted {
		return
	}
	delete(inst.MobIDs, m.ID)
	if inst.Remaining > 0 {
		inst.Remaining--
	}
	for _, id := range inst.MemberIDs {
		if member := w.playersByID[id]; member != nil && member.InWorld {
			member.Session.Send(wire.StandardParm(wire.OpInstanceMobs, instanceSignalID, uint32(inst.Remaining)))
		}
	}
	if inst.Remaining == 0 {
		if !w.grantUxmalRewards(inst, now) {
			// Do not mark the run complete when persistence fails. The cleanup
			// tick keeps the original deadline and staff can reprocess the
			// reward without duplicating the item.
			inst.Remaining = 1
			log.Printf("UXMAL room=%d: reward pending after a persistence failure", inst.UxmalRoom)
		}
	}
}
