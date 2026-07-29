package game

import (
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const instanceSignalID = uint16(0x7530)

type ItemInstance struct {
	Config        model.VolatileInstance
	LeaderID      uint16
	MemberIDs     []uint16
	MobIDs        map[uint16]struct{}
	Remaining     int
	CurrentStage  int
	Deadline      time.Time
	TransitionAt  time.Time
	ExitAt        time.Time
	RewardGranted bool
}

const itemInstanceGeneratorReserveRadius = 8

// generatorReservedForItemInstance identifica os spawns permanentes que
// pertencem a uma sala configurada. Algumas distribuicoes posicionam o
// NPCGener quatro tiles ao lado do centro usado por _MSG_UseItem, por isso a
// tolerancia pequena e intencional. Geradores de respawn normais continuam
// intactos fora dessas areas.
func (w *World) generatorReservedForItemInstance(g model.NPCGener) bool {
	x, y := g.Segments[0].X, g.Segments[0].Y
	if x == 0 || y == 0 {
		return false
	}
	for _, rule := range w.volatiles.Items {
		if rule.Action != "instance_ticket" || rule.Instance == nil {
			continue
		}
		cfg := rule.Instance
		for _, stage := range instanceStages(cfg) {
			radius := stage.AreaRadius
			if radius < itemInstanceGeneratorReserveRadius {
				radius = itemInstanceGeneratorReserveRadius
			}
			if chebyshev(x, y, stage.X, stage.Y) <= radius {
				return true
			}
		}
	}
	return false
}

func instancePartyMembers(leader *Player, partyMode string) ([]*Player, bool) {
	if leader == nil {
		return nil, false
	}
	if partyMode == "solo" {
		return []*Player{leader}, true
	}
	if leader.Party == nil {
		if partyMode == "party" {
			return nil, false
		}
		return []*Player{leader}, true
	}
	if len(leader.Party.Members) == 0 || leader.Party.Members[0] != leader {
		return nil, false
	}
	members := make([]*Player, 0, len(leader.Party.Members))
	for _, member := range leader.Party.Members {
		if member != nil && member.InWorld && member.Char != nil && playerCurHP(member.Char) > 0 {
			members = append(members, member)
		}
	}
	return members, len(members) > 0
}

func instanceStages(cfg *model.VolatileInstance) []model.VolatileInstanceStage {
	if cfg == nil {
		return nil
	}
	if len(cfg.Stages) > 0 {
		return cfg.Stages
	}
	return []model.VolatileInstanceStage{{
		Name: cfg.Name, X: cfg.X, Y: cfg.Y, SpawnX: cfg.SpawnX, SpawnY: cfg.SpawnY,
		AreaRadius: cfg.AreaRadius, DurationSeconds: cfg.DurationSeconds, Spawns: cfg.Spawns,
	}}
}

func instanceSpawnCount(cfg *model.VolatileInstance) int {
	stages := instanceStages(cfg)
	if len(stages) == 0 {
		return 0
	}
	total := 0
	for _, spawn := range stages[0].Spawns {
		total += spawn.Count
	}
	return total
}

func instanceAllowsEvolution(ch *model.Char, allowed []string) bool {
	if ch == nil || len(allowed) == 0 {
		return true
	}
	actual := ch.Evolution
	if actual == "" {
		actual = "mortal"
	}
	for _, evolution := range allowed {
		if matchesEvolution(ch, evolution) ||
			(actual == "subcelestial" && evolution == "subcelestial") {
			return true
		}
	}
	return false
}

// instanceAreaOccupied reproduz o GetUserInArea nativo: uma sala fisicamente
// ocupada nao pode ser sobrescrita mesmo se o mapa de instancias estiver vazio
// (por exemplo, depois de reload de dados ou teleporte externo).
func (w *World) instanceAreaOccupied(cfg *model.VolatileInstance) bool {
	if cfg == nil {
		return true
	}
	for _, player := range w.playersByID {
		if player == nil || !player.InWorld {
			continue
		}
		for _, stage := range instanceStages(cfg) {
			if chebyshev(player.X, player.Y, stage.X, stage.Y) <= stage.AreaRadius {
				return true
			}
		}
	}
	return false
}

func (w *World) saveAccountsAtomic(accounts ...*model.Account) error {
	gs, ok := w.store.(guildStore)
	if !ok {
		return fmt.Errorf("store atual nao suporta transacao multi-conta")
	}
	for _, account := range accounts {
		pinAccountEntryPositions(account)
	}
	return gs.SaveGameState(nil, accounts...)
}

func (w *World) planInstancePositions(members []*Player, x, y uint16) ([][2]uint16, bool) {
	result := make([][2]uint16, 0, len(members))
	reserved := make(map[uint32]struct{}, len(members))
	for _, member := range members {
		found := false
		for distance := 0; distance <= 4 && !found; distance++ {
			for dy := -distance; dy <= distance && !found; dy++ {
				for dx := -distance; dx <= distance; dx++ {
					if distance > 0 && absInt(dx) != distance && absInt(dy) != distance {
						continue
					}
					nx, ny := int(x)+dx, int(y)+dy
					if nx <= 0 || ny <= 0 || nx > 65535 || ny > 65535 {
						continue
					}
					ux, uy := uint16(nx), uint16(ny)
					key := uint32(ux)<<16 | uint32(uy)
					if _, used := reserved[key]; used || !w.terrain.Walkable(ux, uy) ||
						w.positionOccupiedExcept(ux, uy, nil, member) {
						continue
					}
					reserved[key] = struct{}{}
					result = append(result, [2]uint16{ux, uy})
					found = true
					break
				}
			}
		}
		if !found {
			return nil, false
		}
	}
	return result, true
}

func (w *World) useInstanceTicket(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule) {
	cfg := rule.Instance
	if cfg == nil || w.itemInstances[cfg.ID] != nil || w.instanceAreaOccupied(cfg) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("This room is already occupied."))
		return
	}
	members, ok := instancePartyMembers(p, rule.PartyMode)
	stages := instanceStages(cfg)
	totalMobs := instanceSpawnCount(cfg)
	validSpawns := len(stages) > 0 && totalMobs > 0
	allMembersAllowed := true
	for _, member := range members {
		if !instanceAllowsEvolution(member.Char, cfg.AllowedEvolutions) {
			allMembersAllowed = false
			break
		}
	}
	for _, stage := range stages {
		if !w.terrain.Walkable(stage.SpawnX, stage.SpawnY) {
			validSpawns = false
		}
		for _, spawn := range stage.Spawns {
			def := w.npcDefByName(spawn.NPC)
			if def == nil || def.Extended == nil || def.Extended.Merchant != 0 {
				validSpawns = false
			}
		}
	}
	var firstStage model.VolatileInstanceStage
	if len(stages) > 0 {
		firstStage = stages[0]
	}
	destinations, positionsOK := w.planInstancePositions(members, firstStage.X, firstStage.Y)
	if !ok || !allMembersAllowed || !validSpawns ||
		!positionsOK {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("The instance cannot be started."))
		return
	}

	oldItem := *item
	type oldPosition struct {
		p    *Player
		x, y uint16
	}
	positions := make([]oldPosition, 0, len(members))
	accounts := make([]*model.Account, 0, len(members))
	seenAccounts := make(map[*model.Account]struct{})
	for index, member := range members {
		positions = append(positions, oldPosition{member, member.X, member.Y})
		member.X, member.Y = destinations[index][0], destinations[index][1]
		member.Char.X, member.Char.Y = member.X, member.Y
		if _, seen := seenAccounts[member.Account]; !seen {
			seenAccounts[member.Account] = struct{}{}
			accounts = append(accounts, member.Account)
		}
	}
	if rule.Consume {
		consumeOne(item)
	}
	if err := w.saveAccountsAtomic(accounts...); err != nil {
		*item = oldItem
		for _, old := range positions {
			old.p.X, old.p.Y = old.x, old.y
			old.p.Char.X, old.p.Char.Y = old.x, old.y
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	inst := &ItemInstance{
		Config: cfgCopy(*cfg), LeaderID: p.ID, MobIDs: make(map[uint16]struct{}),
		Remaining: totalMobs, CurrentStage: 0,
	}
	for _, member := range members {
		inst.MemberIDs = append(inst.MemberIDs, member.ID)
		w.refreshPlayerVisibility(member)
		w.sendToPlayerView(member, func() []byte { return wire.ActionStop(member.ID, member.X, member.Y) })
	}
	w.itemInstances[cfg.ID] = inst
	w.spawnItemInstanceStage(inst, 0, w.now(), false)
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	log.Printf("[#%d] INSTANCIA %q aberta membros=%d mobs=%d", s.ID, cfg.ID, len(members), totalMobs)
}

func cfgCopy(cfg model.VolatileInstance) model.VolatileInstance {
	cfg.Spawns = append([]model.VolatileInstanceSpawn(nil), cfg.Spawns...)
	cfg.AllowedEvolutions = append([]string(nil), cfg.AllowedEvolutions...)
	cfg.Stages = append([]model.VolatileInstanceStage(nil), cfg.Stages...)
	for index := range cfg.Stages {
		cfg.Stages[index].Spawns = append(
			[]model.VolatileInstanceSpawn(nil), cfg.Stages[index].Spawns...)
	}
	return cfg
}

func stageDuration(cfg model.VolatileInstance, stage model.VolatileInstanceStage) int {
	if stage.DurationSeconds > 0 {
		return stage.DurationSeconds
	}
	return cfg.DurationSeconds
}

func stageMobCount(stage model.VolatileInstanceStage) int {
	total := 0
	for _, spawn := range stage.Spawns {
		total += spawn.Count
	}
	return total
}

// moveInstanceMembersAtomic troca a sala do grupo em um unico commit. Fazer
// teleportPlayer membro a membro poderia deixar metade do grupo na sala
// anterior quando o PostgreSQL falhasse no meio da transicao.
func (w *World) moveInstanceMembersAtomic(inst *ItemInstance, stage model.VolatileInstanceStage) bool {
	if inst == nil {
		return false
	}
	members := make([]*Player, 0, len(inst.MemberIDs))
	for _, id := range inst.MemberIDs {
		if member := w.playersByID[id]; member != nil && member.InWorld &&
			member.Char != nil && member.Account != nil {
			members = append(members, member)
		}
	}
	if len(members) == 0 {
		return false
	}
	destinations, ok := w.planInstancePositions(members, stage.X, stage.Y)
	if !ok {
		return false
	}
	type oldPosition struct {
		player *Player
		x, y   uint16
	}
	oldPositions := make([]oldPosition, 0, len(members))
	accounts := make([]*model.Account, 0, len(members))
	seenAccounts := make(map[*model.Account]struct{}, len(members))
	for index, member := range members {
		oldPositions = append(oldPositions, oldPosition{member, member.X, member.Y})
		member.X, member.Y = destinations[index][0], destinations[index][1]
		member.Char.X, member.Char.Y = member.X, member.Y
		if _, seen := seenAccounts[member.Account]; !seen {
			seenAccounts[member.Account] = struct{}{}
			accounts = append(accounts, member.Account)
		}
	}
	if err := w.saveAccountsAtomic(accounts...); err != nil {
		for _, old := range oldPositions {
			old.player.X, old.player.Y = old.x, old.y
			old.player.Char.X, old.player.Char.Y = old.x, old.y
		}
		return false
	}
	for _, member := range members {
		w.refreshPlayerVisibility(member)
		w.sendToPlayerView(member, func() []byte {
			return wire.ActionStop(member.ID, member.X, member.Y)
		})
	}
	return true
}

// spawnItemInstanceStage inicia uma sala/onda autoritativa. A transicao move os
// membros por uma transacao atomica antes de publicar posicao e visibilidade; a primeira sala ja teve as posicoes
// confirmadas no mesmo commit que consumiu o ticket.
func (w *World) spawnItemInstanceStage(inst *ItemInstance, stageIndex int, now time.Time, move bool) bool {
	if inst == nil {
		return false
	}
	stages := instanceStages(&inst.Config)
	if stageIndex < 0 || stageIndex >= len(stages) {
		return false
	}
	stage := stages[stageIndex]
	if move && !w.moveInstanceMembersAtomic(inst, stage) {
		log.Printf("INSTANCIA %q: falha atomica ao mover grupo para sala %d",
			inst.Config.ID, stageIndex+1)
		return false
	}
	inst.CurrentStage = stageIndex
	inst.TransitionAt = time.Time{}
	inst.MobIDs = make(map[uint16]struct{})
	inst.Remaining = stageMobCount(stage)
	duration := stageDuration(inst.Config, stage)
	inst.Deadline = now.Add(time.Duration(duration) * time.Second)

	for _, id := range inst.MemberIDs {
		if member := w.playersByID[id]; member != nil && member.InWorld {
			member.Session.Send(wire.StandardParm(
				wire.OpInstanceTime, instanceSignalID, uint32(duration)))
			member.Session.Send(wire.StandardParm(
				wire.OpInstanceMobs, instanceSignalID, uint32(inst.Remaining)))
			name := stage.Name
			if name == "" {
				name = inst.Config.Name
			}
			member.Session.Send(wire.MessagePanel(name))
		}
	}
	for _, spawn := range stage.Spawns {
		def := w.npcDefByName(spawn.NPC)
		if def == nil || def.Extended == nil {
			return false
		}
		for n := 0; n < spawn.Count; n++ {
			searchRadius := stage.AreaRadius
			if searchRadius < 4 {
				searchRadius = 4
			}
			x, y := w.findFreePosition(stage.SpawnX, stage.SpawnY, uint16(searchRadius))
			mob := &Mob{ID: w.allocMobID(), Def: def, X: x, Y: y,
				HP: def.Extended.MaxHP, InstanceID: inst.Config.ID}
			w.mobs = append(w.mobs, mob)
			w.mobsByID[mob.ID] = mob
			inst.MobIDs[mob.ID] = struct{}{}
			w.publishMobSpawn(mob)
		}
	}
	log.Printf("INSTANCIA %q sala=%d/%d mobs=%d",
		inst.Config.ID, stageIndex+1, len(stages), inst.Remaining)
	return true
}

func firstFreeVisibleInventorySlot(ch *model.Char) int {
	if ch == nil {
		return -1
	}
	for slot := 0; slot < model.PlayerCarrySlots; slot++ {
		if ch.Inv[slot].Index == 0 {
			return slot
		}
	}
	return -1
}

// grantItemInstanceReward implementa PutItem do Water: a recompensa pertence
// somente ao lider. Inventario cheio usa o mesmo fallback nativo de item no
// chao; persistencia falha mantem a sala pendente para uma nova tentativa.
func (w *World) grantItemInstanceReward(inst *ItemInstance) bool {
	if inst == nil || inst.RewardGranted {
		return true
	}
	if inst.Config.RewardItem == 0 {
		inst.RewardGranted = true
		return true
	}
	leader := w.playersByID[inst.LeaderID]
	if leader == nil || !leader.InWorld || leader.Char == nil {
		return false
	}
	reward, err := materializeItem(model.Item{Index: inst.Config.RewardItem})
	if err != nil {
		log.Printf("INSTANCIA %q: materializar recompensa %d: %v",
			inst.Config.ID, inst.Config.RewardItem, err)
		return false
	}
	slot := firstFreeVisibleInventorySlot(leader.Char)
	if slot < 0 {
		if w.groundItems == nil {
			w.groundItems = make(map[uint16]*GroundItem)
		}
		if w.createGroundDrop(leader.X, leader.Y, reward, true) == nil {
			return false
		}
		inst.RewardGranted = true
		leader.Session.Send(wire.MessagePanel("Inventory full: the next Water Scroll was dropped."))
		return true
	}

	leader.Char.Inv[slot] = reward
	if err := w.saveAccount(leader.Account); err != nil {
		leader.Char.Inv[slot] = model.Item{}
		log.Printf("INSTANCIA %q: salvar recompensa %d: %v",
			inst.Config.ID, inst.Config.RewardItem, err)
		return false
	}
	inst.RewardGranted = true
	leader.Session.Send(wire.SendItem(leader.ID, placeInv, byte(slot), reward))
	leader.Session.Send(wire.MessagePanel("You received the next Water Scroll."))
	log.Printf("[#%d] INSTANCIA %q concluida: item %d -> inv[%d]",
		leader.Session.ID, inst.Config.ID, inst.Config.RewardItem, slot)
	return true
}

func (w *World) completeItemInstance(inst *ItemInstance, now time.Time) {
	if inst == nil || !inst.ExitAt.IsZero() {
		return
	}
	if w.grantItemInstanceReward(inst) {
		inst.ExitAt = now.Add(10 * time.Second)
	}
}

func (w *World) onItemInstanceMobKilled(m *Mob, now time.Time) {
	if m == nil || m.InstanceID == "" {
		return
	}
	inst := w.itemInstances[m.InstanceID]
	if inst == nil {
		return
	}
	if _, belongs := inst.MobIDs[m.ID]; !belongs {
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
		stages := instanceStages(&inst.Config)
		if inst.CurrentStage+1 < len(stages) {
			delay := inst.Config.TransitionSeconds
			if delay <= 0 {
				delay = 10
			}
			inst.TransitionAt = now.Add(time.Duration(delay) * time.Second)
			for _, id := range inst.MemberIDs {
				if member := w.playersByID[id]; member != nil && member.InWorld {
					member.Session.Send(wire.StandardParm(
						wire.OpInstanceTime, instanceSignalID, uint32(delay)))
					member.Session.Send(wire.MessagePanel("The next room will open shortly."))
				}
			}
		} else {
			w.completeItemInstance(inst, now)
		}
	}
}

func (w *World) tickItemInstances(now time.Time) {
	for id, inst := range w.itemInstances {
		if !inst.TransitionAt.IsZero() && !now.Before(inst.TransitionAt) {
			if !w.spawnItemInstanceStage(inst, inst.CurrentStage+1, now, true) {
				inst.Deadline = now
			}
		}
		if inst.Remaining == 0 && inst.TransitionAt.IsZero() &&
			inst.ExitAt.IsZero() && now.Before(inst.Deadline) {
			w.completeItemInstance(inst, now)
		}
		expired := inst.TransitionAt.IsZero() && !now.Before(inst.Deadline)
		completed := !inst.ExitAt.IsZero() && !now.Before(inst.ExitAt)
		if !expired && !completed {
			continue
		}
		for mobID := range inst.MobIDs {
			if mob := w.mobsByID[mobID]; mob != nil {
				mob.Dead = true
				w.publishMobDeath(mob, 0, 0, nil)
				w.removeMobInstance(mob)
			}
		}
		allExited := true
		for _, playerID := range inst.MemberIDs {
			if member := w.playersByID[playerID]; member != nil && member.InWorld {
				if !w.teleportPlayer(member, inst.Config.ExitX, inst.Config.ExitY) {
					allExited = false
					continue
				}
				member.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, 0))
				member.Session.Send(wire.StandardParm(wire.OpInstanceMobs, instanceSignalID, 0))
			}
		}
		if !allExited {
			continue
		}
		delete(w.itemInstances, id)
		log.Printf("INSTANCIA %q encerrada", id)
	}
}
