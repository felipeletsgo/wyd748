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

const instanceSignalID = uint16(0x7530)

type ItemInstance struct {
	Config        model.VolatileInstance
	LeaderID      uint16
	MemberIDs     []uint16
	MobIDs        map[uint16]struct{}
	Remaining     int
	CurrentStage  int
	Deadline      time.Time
	HardDeadline  time.Time
	TransitionAt  time.Time
	QuizAt        time.Time
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
	reserved := func(rule model.VolatileRule) bool {
		if rule.Action != "instance_ticket" || rule.Instance == nil {
			return false
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
		return false
	}
	for _, rule := range w.volatiles.Rules {
		if reserved(rule) {
			return true
		}
	}
	for _, rule := range w.volatiles.Items {
		if reserved(rule) {
			return true
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

func itemInstanceHasMember(inst *ItemInstance, playerID uint16) bool {
	if inst == nil {
		return false
	}
	for _, id := range inst.MemberIDs {
		if id == playerID {
			return true
		}
	}
	return false
}

func (w *World) playerHasItemInstance(playerID uint16) bool {
	for _, inst := range w.itemInstances {
		if itemInstanceHasMember(inst, playerID) {
			return true
		}
	}
	return false
}

// detachPlayerFromItemInstances libera imediatamente a vaga de quem saiu do
// mundo. IDs de entidade mudam no próximo login; mantê-los na sala bloquearia
// uma vaga do Cube e poderia deixar a recompensa presa ao líder desconectado.
func (w *World) detachPlayerFromItemInstances(playerID uint16, now time.Time) {
	if playerID == 0 {
		return
	}
	for _, inst := range w.itemInstances {
		if inst == nil || !itemInstanceHasMember(inst, playerID) {
			continue
		}
		members := make([]uint16, 0, len(inst.MemberIDs)-1)
		for _, id := range inst.MemberIDs {
			if id != playerID {
				members = append(members, id)
			}
		}
		inst.MemberIDs = members
		if len(members) == 0 {
			// O tick comum remove os mobs e encerra a instância sem criar um
			// segundo caminho de cleanup durante logout/desconexão.
			inst.TransitionAt = time.Time{}
			inst.QuizAt = time.Time{}
			inst.ExitAt = time.Time{}
			inst.Deadline = now
			continue
		}
		if inst.LeaderID == playerID {
			inst.LeaderID = members[0]
		}
	}
}

func remainingInstanceSeconds(deadline, now time.Time) int {
	if deadline.IsZero() || !deadline.After(now) {
		return 0
	}
	remaining := int(deadline.Sub(now).Seconds())
	if remaining < 1 {
		return 1
	}
	return remaining
}

// joinSharedItemInstance reproduz o ingresso individual do Cube: cada jogador
// consome o proprio convite e entra numa execucao ainda parada na primeira sala.
// Nenhum ingresso transporta automaticamente os demais membros da party.
func (w *World) joinSharedItemInstance(s *net.Session, p *Player, item *model.Item,
	slot byte, rule model.VolatileRule, inst *ItemInstance) {
	cfg := rule.Instance
	now := w.now()
	if cfg == nil || inst == nil || !cfg.SharedEntry ||
		inst.CurrentStage != 0 || inst.Remaining <= 0 ||
		!inst.TransitionAt.IsZero() || !inst.QuizAt.IsZero() ||
		!inst.ExitAt.IsZero() || !now.Before(inst.Deadline) ||
		itemInstanceHasMember(inst, p.ID) || w.playerHasItemInstance(p.ID) ||
		(cfg.MaxPlayers > 0 && len(inst.MemberIDs) >= cfg.MaxPlayers) ||
		!instanceAllowsEvolution(p.Char, cfg.AllowedEvolutions) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("This Cube cannot accept another player."))
		return
	}
	stages := instanceStages(&inst.Config)
	if len(stages) == 0 {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	destinations, ok := w.planInstancePositions([]*Player{p}, stages[0].X, stages[0].Y)
	if !ok {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("The Cube entrance is blocked."))
		return
	}

	oldItem, oldX, oldY := *item, p.X, p.Y
	p.X, p.Y = destinations[0][0], destinations[0][1]
	p.Char.X, p.Char.Y = p.X, p.Y
	if rule.Consume {
		consumeOne(item)
	}
	if err := w.saveAccountsAtomic(p.Account); err != nil {
		*item = oldItem
		p.X, p.Y = oldX, oldY
		p.Char.X, p.Char.Y = oldX, oldY
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	inst.MemberIDs = append(inst.MemberIDs, p.ID)
	w.refreshPlayerVisibility(p)
	w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
	p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID,
		uint32(remainingInstanceSeconds(inst.Deadline, now))))
	p.Session.Send(wire.StandardParm(wire.OpInstanceMobs, instanceSignalID,
		uint32(inst.Remaining)))
	name := stages[0].Name
	if name == "" {
		name = inst.Config.Name
	}
	p.Session.Send(wire.MessagePanel(name))
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	log.Printf("[#%d] INSTANCIA %q ingresso individual membros=%d",
		s.ID, cfg.ID, len(inst.MemberIDs))
}

func (w *World) useInstanceTicket(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule) {
	cfg := rule.Instance
	if cfg == nil {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if inst := w.itemInstances[cfg.ID]; inst != nil {
		w.joinSharedItemInstance(s, p, item, slot, rule, inst)
		return
	}
	if w.instanceAreaOccupied(cfg) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("This room is already occupied."))
		return
	}
	members, ok := instancePartyMembers(p, rule.PartyMode)
	stages := instanceStages(cfg)
	totalMobs := instanceSpawnCount(cfg)
	validSpawns := len(stages) > 0 && totalMobs > 0
	allMembersAllowed := true
	if cfg.MaxPlayers > 0 && len(members) > cfg.MaxPlayers {
		allMembersAllowed = false
	}
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
		if stage.Quiz != nil &&
			(!w.terrain.Walkable(stage.Quiz.TrueX, stage.Quiz.TrueY) ||
				!w.terrain.Walkable(stage.Quiz.FalseX, stage.Quiz.FalseY)) {
			validSpawns = false
		}
		for _, spawn := range stage.Spawns {
			if spawn.X != 0 && spawn.Y != 0 && !w.terrain.Walkable(spawn.X, spawn.Y) {
				validSpawns = false
			}
			def := w.npcDefByName(spawn.NPC)
			if def == nil || !def.IsMonster() || def.Extended == nil {
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
	inst := &ItemInstance{
		Config: cfgCopy(*cfg), LeaderID: p.ID, MobIDs: make(map[uint16]struct{}),
		Remaining: totalMobs, CurrentStage: 0,
	}
	now := w.now()
	if cfg.TotalDurationSeconds > 0 {
		inst.HardDeadline = now.Add(
			time.Duration(cfg.TotalDurationSeconds) * time.Second)
	}
	for _, member := range members {
		inst.MemberIDs = append(inst.MemberIDs, member.ID)
	}
	// O primeiro conteudo existe antes de confirmar o ticket. Ele ainda nao e
	// publicado; logo qualquer falha pode ser revertida sem deixar sala vazia.
	if !w.spawnItemInstanceStage(inst, 0, now, false, false) {
		*item = oldItem
		for _, old := range positions {
			old.p.X, old.p.Y = old.x, old.y
			old.p.Char.X, old.p.Char.Y = old.x, old.y
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("The instance content could not be created."))
		return
	}
	if err := w.saveAccountsAtomic(accounts...); err != nil {
		w.removeUnpublishedItemInstanceMobs(inst)
		*item = oldItem
		for _, old := range positions {
			old.p.X, old.p.Y = old.x, old.y
			old.p.Char.X, old.p.Char.Y = old.x, old.y
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	w.itemInstances[cfg.ID] = inst
	for _, member := range members {
		w.refreshPlayerVisibility(member)
		w.sendToPlayerView(member, func() []byte { return wire.ActionStop(member.ID, member.X, member.Y) })
	}
	w.sendItemInstanceStageStatus(inst)
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
		if cfg.Stages[index].Quiz != nil {
			quiz := *cfg.Stages[index].Quiz
			cfg.Stages[index].Quiz = &quiz
		}
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

func (w *World) sendItemInstanceStageStatus(inst *ItemInstance) {
	if inst == nil {
		return
	}
	stages := instanceStages(&inst.Config)
	if inst.CurrentStage < 0 || inst.CurrentStage >= len(stages) {
		return
	}
	stage := stages[inst.CurrentStage]
	remaining := remainingInstanceSeconds(inst.Deadline, w.now())
	for _, id := range inst.MemberIDs {
		if member := w.playersByID[id]; member != nil && member.InWorld {
			member.Session.Send(wire.StandardParm(
				wire.OpInstanceTime, instanceSignalID, uint32(remaining)))
			member.Session.Send(wire.StandardParm(
				wire.OpInstanceMobs, instanceSignalID, uint32(inst.Remaining)))
			name := stage.Name
			if name == "" {
				name = inst.Config.Name
			}
			member.Session.Send(wire.MessagePanel(name))
		}
	}
}

func (w *World) removeUnpublishedItemInstanceMobs(inst *ItemInstance) {
	if inst == nil {
		return
	}
	for mobID := range inst.MobIDs {
		if mob := w.mobsByID[mobID]; mob != nil {
			w.removeMobInstance(mob)
		}
	}
	inst.MobIDs = make(map[uint16]struct{})
	inst.Remaining = 0
}

// spawnItemInstanceStage inicia uma sala/onda autoritativa. publish=false e
// usado apenas na abertura: monta todo o primeiro conteudo antes do commit do
// ingresso, sem expor estado que ainda pode sofrer rollback.
func (w *World) spawnItemInstanceStage(inst *ItemInstance, stageIndex int, now time.Time,
	move, publish bool) bool {
	if inst == nil {
		return false
	}
	stages := instanceStages(&inst.Config)
	if stageIndex < 0 || stageIndex >= len(stages) {
		return false
	}
	stage := stages[stageIndex]
	for _, spawn := range stage.Spawns {
		def := w.npcDefByName(spawn.NPC)
		if spawn.Count <= 0 || def == nil || !def.IsMonster() ||
			def.Extended == nil {
			return false
		}
	}
	if move && !w.moveInstanceMembersAtomic(inst, stage) {
		log.Printf("INSTANCIA %q: falha atomica ao mover grupo para sala %d",
			inst.Config.ID, stageIndex+1)
		return false
	}
	inst.CurrentStage = stageIndex
	inst.TransitionAt = time.Time{}
	inst.QuizAt = time.Time{}
	inst.MobIDs = make(map[uint16]struct{})
	inst.Remaining = stageMobCount(stage)
	duration := stageDuration(inst.Config, stage)
	inst.Deadline = now.Add(time.Duration(duration) * time.Second)

	for _, spawn := range stage.Spawns {
		def := w.npcDefByName(spawn.NPC)
		for n := 0; n < spawn.Count; n++ {
			spawnX, spawnY := stage.SpawnX, stage.SpawnY
			if spawn.X != 0 && spawn.Y != 0 {
				spawnX, spawnY = spawn.X, spawn.Y
			}
			searchRadius := stage.AreaRadius
			if searchRadius < 4 {
				searchRadius = 4
			}
			x, y := w.findFreePosition(spawnX, spawnY, uint16(searchRadius))
			mobID := w.allocMobID()
			if mobID == 0 {
				w.removeUnpublishedItemInstanceMobs(inst)
				return false
			}
			mob := &Mob{ID: mobID, Def: def, X: x, Y: y,
				HP: def.Extended.MaxHP, InstanceID: inst.Config.ID}
			w.mobs = append(w.mobs, mob)
			w.mobsByID[mob.ID] = mob
			inst.MobIDs[mob.ID] = struct{}{}
			if publish {
				w.publishMobSpawn(mob)
			} else {
				// A primeira sala ainda nao pode ser enviada, mas precisa entrar
				// no indice espacial para o refresh posterior ao commit encontra-la.
				w.registerMobSpatial(mob)
			}
		}
	}
	if publish {
		w.sendItemInstanceStageStatus(inst)
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
			if quiz := stages[inst.CurrentStage].Quiz; quiz != nil {
				inst.QuizAt = now.Add(time.Duration(quiz.DurationSeconds) * time.Second)
				for _, id := range inst.MemberIDs {
					if member := w.playersByID[id]; member != nil && member.InWorld {
						member.Session.Send(wire.StandardParm(
							wire.OpInstanceTime, instanceSignalID, uint32(quiz.DurationSeconds)))
						member.Session.Send(wire.MessagePanel(
							strings.ReplaceAll(quiz.Question, "_", " ")))
					}
				}
				return
			}
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

func instanceQuizCorrect(p *Player, quiz *model.VolatileInstanceQuiz) bool {
	if p == nil || quiz == nil {
		return false
	}
	x, y := quiz.FalseX, quiz.FalseY
	if quiz.Answer {
		x, y = quiz.TrueX, quiz.TrueY
	}
	minX, minY := uint16(0), uint16(0)
	if x >= 3 {
		minX = x - 3
	}
	if y >= 3 {
		minY = y - 3
	}
	return p.X >= minX && p.X <= x && p.Y >= minY && p.Y <= y
}

// resolveItemInstanceQuiz confirma em um unico commit tanto a EXP dos acertos
// quanto a retirada dos erros. Assim uma falha do banco nao divide a party nem
// concede a recompensa duas vezes.
func (w *World) resolveItemInstanceQuiz(inst *ItemInstance, now time.Time) bool {
	if inst == nil {
		return false
	}
	stages := instanceStages(&inst.Config)
	if inst.CurrentStage < 0 || inst.CurrentStage >= len(stages) {
		return false
	}
	quiz := stages[inst.CurrentStage].Quiz
	if quiz == nil {
		return false
	}
	type state struct {
		player *Player
		char   model.Char
		x, y   uint16
		ok     bool
		levels int
		exp    uint32
	}
	states := make([]state, 0, len(inst.MemberIDs))
	accounts := make([]*model.Account, 0, len(inst.MemberIDs))
	seenAccounts := make(map[*model.Account]struct{}, len(inst.MemberIDs))
	for _, id := range inst.MemberIDs {
		p := w.playersByID[id]
		if p == nil || !p.InWorld || p.Char == nil || p.Account == nil {
			continue
		}
		current := state{player: p, char: cloneCharacterState(p.Char), x: p.X, y: p.Y}
		current.ok = instanceQuizCorrect(p, quiz)
		if current.ok && quiz.RewardExp > 0 {
			oldHP, oldMP := playerCurHP(p.Char), playerCurMP(p.Char)
			current.levels, current.exp = grantExp(p.Char, quiz.RewardExp)
			w.recalcPlayer(p.Char)
			setPlayerCurHP(p.Char, minU32(oldHP, playerMaxHP(p.Char)))
			setPlayerCurMP(p.Char, minU32(oldMP, playerMaxMP(p.Char)))
		} else if !current.ok {
			p.X, p.Y = w.findFreePlayerPosition(
				inst.Config.ExitX, inst.Config.ExitY, 6, p)
			p.Char.X, p.Char.Y = p.X, p.Y
		}
		states = append(states, current)
		if _, exists := seenAccounts[p.Account]; !exists {
			seenAccounts[p.Account] = struct{}{}
			accounts = append(accounts, p.Account)
		}
	}
	if len(states) == 0 {
		inst.MemberIDs = nil
		inst.QuizAt = time.Time{}
		inst.Deadline = now
		return true
	}
	if err := w.saveAccountsAtomic(accounts...); err != nil {
		for _, old := range states {
			*old.player.Char = old.char
			old.player.X, old.player.Y = old.x, old.y
		}
		return false
	}

	survivors := make([]uint16, 0, len(states))
	for _, result := range states {
		p := result.player
		if !result.ok {
			w.refreshPlayerVisibility(p)
			w.sendToPlayerView(p, func() []byte {
				return wire.ActionStop(p.ID, p.X, p.Y)
			})
			p.Session.Send(wire.StandardParm(
				wire.OpInstanceTime, instanceSignalID, 0))
			p.Session.Send(wire.StandardParm(
				wire.OpInstanceMobs, instanceSignalID, 0))
			p.Session.Send(wire.MessagePanel("Wrong answer. You left the Cube."))
			continue
		}
		survivors = append(survivors, p.ID)
		w.syncPlayerVitals(p)
		w.updatePartyMember(p)
		if result.levels > 0 {
			p.Session.Send(wire.UpdateScore(p.ID, *p.Char))
		}
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		p.Session.Send(wire.MessagePanel(fmt.Sprintf(
			"Correct! +%d EXP.", result.exp)))
	}
	inst.MemberIDs = survivors
	inst.QuizAt = time.Time{}
	if len(survivors) == 0 {
		inst.Deadline = now
		return true
	}
	delay := inst.Config.TransitionSeconds
	if delay < 0 {
		delay = 0
	}
	inst.TransitionAt = now.Add(time.Duration(delay) * time.Second)
	return true
}

func (w *World) tickItemInstances(now time.Time) {
	for id, inst := range w.itemInstances {
		hardExpired := !inst.HardDeadline.IsZero() && !now.Before(inst.HardDeadline)
		if !hardExpired && !inst.QuizAt.IsZero() && !now.Before(inst.QuizAt) {
			if !w.resolveItemInstanceQuiz(inst, now) {
				continue
			}
		}
		if !hardExpired && !inst.TransitionAt.IsZero() && !now.Before(inst.TransitionAt) {
			if !w.spawnItemInstanceStage(inst, inst.CurrentStage+1, now, true, true) {
				inst.Deadline = now
			}
		}
		if inst.Remaining == 0 && inst.TransitionAt.IsZero() && inst.QuizAt.IsZero() &&
			inst.ExitAt.IsZero() && now.Before(inst.Deadline) {
			w.completeItemInstance(inst, now)
		}
		expired := inst.TransitionAt.IsZero() && inst.QuizAt.IsZero() &&
			!now.Before(inst.Deadline)
		if hardExpired {
			expired = true
		}
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
