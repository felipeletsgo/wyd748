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

// uxmalScheduleAllowed reproduz as tres janelas de quatro minutos do TMSrv:
// 16-19, 36-39 e 56-59. A validacao continua baseada no relogio do World para
// manter os testes deterministas e nao aceita minuto enviado pelo client.
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
		return fmt.Errorf("template ausente")
	}
	if !strings.EqualFold(strings.TrimSpace(cfg.StateMachine), "uxmal") {
		return fmt.Errorf("stateMachine deve ser uxmal")
	}
	if strings.TrimSpace(cfg.Uxmal.NPC) == "" {
		return fmt.Errorf("npc de entrada ausente")
	}
	entryNPC := w.npcDefByName(cfg.Uxmal.NPC)
	if entryNPC == nil || entryNPC.IsMonster() || entryNPC.Score == nil {
		return fmt.Errorf("npc de entrada invalido %q", cfg.Uxmal.NPC)
	}
	if cfg.Uxmal.TicketItem == 0 {
		return fmt.Errorf("ticket ausente")
	}
	if _, ok := w.items[cfg.Uxmal.TicketItem]; !ok {
		return fmt.Errorf("ticket %d nao existe no catalogo", cfg.Uxmal.TicketItem)
	}
	if len(cfg.Uxmal.EntryAreas) == 0 {
		return fmt.Errorf("area de entrada ausente")
	}
	for i, area := range cfg.Uxmal.EntryAreas {
		if area.MinX == 0 || area.MinY == 0 || area.MaxX < area.MinX || area.MaxY < area.MinY {
			return fmt.Errorf("area de entrada %d invalida", i)
		}
	}
	if len(cfg.Stages) != 7 || len(cfg.Uxmal.RoomPositions) != 7 ||
		len(cfg.Uxmal.MaxParties) != 7 || len(cfg.Uxmal.Runes) != 7 ||
		len(cfg.Uxmal.TicketNextSanc) != 7 {
		return fmt.Errorf("sao necessarias sete salas")
	}
	for room, stage := range cfg.Stages {
		if stage.AreaRadius <= 0 || stage.SpawnX == 0 || stage.SpawnY == 0 || len(stage.Spawns) == 0 {
			return fmt.Errorf("sala %d incompleta", room)
		}
		if !w.terrain.Walkable(stage.SpawnX, stage.SpawnY) {
			return fmt.Errorf("sala %d possui spawn bloqueado", room)
		}
		if len(cfg.Uxmal.RoomPositions[room]) != 3 || cfg.Uxmal.MaxParties[room] < 1 || cfg.Uxmal.MaxParties[room] > 3 {
			return fmt.Errorf("sala %d sem tres vagas/maxParties invalido", room)
		}
		for _, spawn := range stage.Spawns {
			def := w.npcDefByName(spawn.NPC)
			if spawn.Count <= 0 || def == nil || !def.IsMonster() || def.Score == nil {
				return fmt.Errorf("sala %d usa template hostil ausente %q", room, spawn.NPC)
			}
		}
		for slot, pos := range cfg.Uxmal.RoomPositions[room] {
			if pos.X == 0 || pos.Y == 0 || (slot == 2 && cfg.Uxmal.MaxParties[room] < 3) {
				// A terceira coordenada da sala +0 fica preenchida para manter o
				// formato, mas a capacidade continua sendo duas parties.
				if slot < cfg.Uxmal.MaxParties[room] {
					return fmt.Errorf("sala %d vaga %d invalida", room, slot)
				}
			}
			if slot < cfg.Uxmal.MaxParties[room] && !w.terrain.Walkable(pos.X, pos.Y) {
				return fmt.Errorf("sala %d vaga %d usa terreno bloqueado", room, slot)
			}
		}
		if len(cfg.Uxmal.Runes[room]) == 0 {
			return fmt.Errorf("sala %d sem runas", room)
		}
		for _, runeItem := range cfg.Uxmal.Runes[room] {
			if runeItem == 0 {
				return fmt.Errorf("sala %d possui runa vazia", room)
			}
			if _, ok := w.items[runeItem]; !ok {
				return fmt.Errorf("sala %d usa runa inexistente %d", room, runeItem)
			}
		}
		if next := cfg.Uxmal.TicketNextSanc[room]; next < 0 || next > 6 {
			return fmt.Errorf("sala %d possui sancao de pista invalida %d", room, next)
		}
	}
	return nil
}

func (w *World) isUxmalNPC(m *Mob) bool {
	return w != nil && w.uxmal != nil && w.uxmal.Uxmal != nil && m != nil && m.Def != nil &&
		strings.EqualFold(m.Def.Name, w.uxmal.Uxmal.NPC)
}

// handleUxmalNPC fica antes da allowlist de quests comuns. O clique no Uxmal
// consome uma Pista de Runas server-side e nunca abre a loja/quest generica.
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
	// A abertura usa o mesmo caminho transacional das demais instâncias:
	// primeiro reserva a onda, depois move a party e só então publica os mobs.
	// Assim o spawn não colide com jogadores ainda na entrada e uma falha de
	// persistência desfaz simultaneamente posição, item e estado da instância.
	if spawned, _ := w.spawnItemInstanceStage(inst, 0, now, true, false); !spawned {
		delete(w.itemInstances, inst.RuntimeID)
		p.Char.Inv[ticketSlot] = oldTicket
		s.Send(wire.SendItem(p.ID, placeInv, byte(ticketSlot), oldTicket))
		s.Send(wire.MessagePanel("The Rune Track room cannot be opened right now."))
		return
	}
	for mobID := range inst.MobIDs {
		if mob := w.mobsByID[mobID]; mob != nil {
			w.publishMobSpawn(mob)
		}
	}
	for _, member := range members {
		w.refreshPlayerVisibility(member)
		w.sendToPlayerView(member, func() []byte { return wire.ActionStop(member.ID, member.X, member.Y) })
	}
	s.Send(wire.SendItem(p.ID, placeInv, byte(ticketSlot), p.Char.Inv[ticketSlot]))
	w.sendItemInstanceStageStatus(inst)
	log.Printf("[#%d] UXMAL sala=%d vaga=%d lider=%s membros=%d", s.ID, room, partySlot, p.Char.Name, len(members))
}

func (w *World) grantUxmalRewards(inst *ItemInstance, now time.Time) bool {
	if inst == nil || inst.RewardGranted || w.uxmal == nil || w.uxmal.Uxmal == nil {
		return inst != nil && inst.RewardGranted
	}
	runes := w.uxmal.Uxmal.Runes[inst.UxmalRoom]
	if len(runes) == 0 {
		return false
	}
	type snapshot struct {
		player *Player
		char   model.Char
	}
	snapshots := make([]snapshot, 0, len(inst.MemberIDs))
	accounts := make([]*model.Account, 0, len(inst.MemberIDs))
	seen := make(map[*model.Account]struct{})
	rollback := func() {
		for _, old := range snapshots {
			*old.player.Char = old.char
		}
	}
	ground := make([]struct {
		p    *Player
		item model.Item
	}, 0)
	for _, id := range inst.MemberIDs {
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
		leader := w.playersByID[inst.LeaderID]
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
	// Ground rewards are part of the same authoritative transition as the
	// inventory rewards.  Materialize them before the account commit, but do
	// not publish them yet.  That lets us abort the whole reward if one floor
	// item cannot be allocated (invalid catalog entry or exhausted ground-ID
	// range) instead of consuming the room reward and merely logging a loss.
	createdGround := make([]*GroundItem, 0, len(ground))
	rollbackGround := func() {
		for _, item := range createdGround {
			if item != nil {
				w.unregisterGroundItem(item)
			}
		}
		createdGround = nil
	}
	for _, drop := range ground {
		item := w.createGroundDropForInstance(drop.p.X, drop.p.Y, drop.item, false, inst.RuntimeID)
		if item == nil {
			rollbackGround()
			rollback()
			return false
		}
		createdGround = append(createdGround, item)
	}
	if err := w.saveAccountsAndInstanceState(accounts...); err != nil {
		rollbackGround()
		rollback()
		return false
	}
	for _, item := range createdGround {
		w.publishItemSpawn(item)
	}
	for _, id := range inst.MemberIDs {
		if member := w.playersByID[id]; member != nil && member.InWorld {
			member.Session.Send(wire.UpdateEtc(member.ID, *member.Char))
			member.Session.Send(wire.MessagePanel("Rune Track completed: rune received."))
		}
	}
	inst.RewardGranted = true
	inst.ExitAt = now.Add(10 * time.Second)
	inst.ExitDeadline = inst.ExitAt
	return true
}

func (w *World) onUxmalMobKilled(m *Mob, now time.Time) {
	inst := w.itemInstances[m.InstanceID]
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
			// Nao marque a execucao como concluida se a persistencia falhar. O
			// tick de cleanup continua com o prazo original e o admin pode
			// reprocessar a recompensa sem duplicar o item.
			inst.Remaining = 1
			log.Printf("UXMAL sala=%d: recompensa pendente por falha de persistencia", inst.UxmalRoom)
		}
	}
}
