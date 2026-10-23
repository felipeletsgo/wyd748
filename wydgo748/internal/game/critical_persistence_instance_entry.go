package game

import (
	"errors"
	"fmt"
	"log"
	"maps"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Admission reports acceptance, not durable success. No legacy save or live
// instance algorithm is invoked on the critical-owner path.
func (w *World) admitCriticalInstanceTicket(p *Player, slot byte, rule model.VolatileRule, expectedUID string) bool {
	err := w.queueCriticalInstanceTicket(p, slot, rule, expectedUID)
	if err != nil && p != nil && p.Char != nil && p.Session != nil && !p.Session.IsClosed() && int(slot) < model.PlayerCarrySlots {
		resyncUsedItem(p.Session, p, slot)
		if !errors.Is(err, errCriticalPersistenceBusy) && !errors.Is(err, errCriticalPersistenceFull) {
			p.Session.Send(wire.MessagePanel(err.Error()))
		}
	}
	return err == nil
}

func (w *World) queueCriticalInstanceTicket(p *Player, slot byte, rule model.VolatileRule, expectedUID string) error {
	if p == nil || p.Account == nil || p.Char == nil || p.Session == nil || !p.InWorld ||
		playerCurHP(p.Char) == 0 || int(slot) >= model.PlayerCarrySlots || !bindCriticalPlayer(p).current(w) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalInstanceReserved() || w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	cfg := rule.Instance
	if cfg == nil || strings.TrimSpace(cfg.ID) == "" || rule.Action != "instance_ticket" {
		return errors.New("This instance is not configured.")
	}
	source := p.Char.Inv[slot]
	if source.Index == 0 || (expectedUID != "" && source.UID != expectedUID) {
		return errCriticalPersistenceIdentity
	}
	if _, exists := w.items[source.Index]; !exists {
		return errors.New("This ticket is not in the server catalog.")
	}
	for _, active := range w.itemInstances {
		if active != nil && strings.TrimSpace(cfg.ExclusiveGroup) != "" &&
			strings.TrimSpace(active.Config.ExclusiveGroup) == strings.TrimSpace(cfg.ExclusiveGroup) {
			return errors.New("Another instance of this type is already active.")
		}
	}
	if !instanceTargetAllowedAt(cfg, p.X, p.Y) {
		return errors.New("This ticket cannot be used from here.")
	}
	at := w.now()
	plan := &criticalInstanceDeparture{at: at, generation: w.criticalInstanceGeneration,
		before: cloneCriticalInstanceMembership(w), proposed: cloneCriticalInstanceMembership(w),
		originals: maps.Clone(w.itemInstances)}
	if plan.proposed.itemInstances == nil {
		plan.proposed.itemInstances = make(map[string]*ItemInstance)
	}
	runtimeID := instanceRuntimeKey(cfg)
	if independentPrivateInstance(*cfg) {
		runtimeID = w.nextItemInstanceRuntimeID(cfg)
	}
	inst := plan.proposed.itemInstances[runtimeID]
	if inst != nil && itemInstanceInExitGraceAt(inst, at) && !sharedTimedInstance(*cfg) {
		if !w.itemInstanceExitGraceAllowsItem(p.ID, source.Index) {
			return errors.New("Only the next Water Scroll can be used during the exit window.")
		}
		runtimeID = w.nextItemInstanceRuntimeID(cfg)
		inst = nil
	}
	newRoom := inst == nil
	individual := !newRoom && !sharedTimedInstance(*cfg)
	var members []*Player
	var scheduleEnd time.Time
	var runKey string
	if individual {
		if !cfg.SharedEntry {
			return errors.New("This instance is already occupied.")
		}
		if inst.CurrentStage != 0 || !inst.TransitionAt.IsZero() || !inst.QuizAt.IsZero() || !inst.ExitAt.IsZero() ||
			inst.Remaining <= 0 || expiredCriticalInstanceEntry(inst, at) {
			return errors.New("This instance is no longer accepting players.")
		}
		members = []*Player{p}
	} else {
		var scheduleOK, runAllowed, partyOK bool
		scheduleEnd, scheduleOK = instanceScheduleEnd(cfg, at)
		if !scheduleOK || (!newRoom && expiredCriticalSharedInstanceEntry(inst, at)) {
			return errors.New("This instance is not open right now.")
		}
		runKey, runAllowed = w.nightmarePartyRunAllowed(cfg, rule.PartyMode, at)
		if !runAllowed {
			return errors.New("The party limit for this Nightmare tier was reached.")
		}
		if !newRoom && rule.PartyMode != "party" && rule.PartyMode != "solo" && rule.PartyMode != "" {
			return errors.New("This entry mode is not configured.")
		}
		members, partyOK = instancePartyMembers(p, rule.PartyMode)
		if !partyOK {
			return errors.New("Only an eligible party leader can start this instance.")
		}
	}
	var err error
	members, err = nightmareEligibleMembers(members, cfg)
	if err != nil {
		return err
	}
	if len(members) == 0 {
		return errors.New("No eligible party member is available.")
	}
	for _, member := range members {
		if member == nil || member.Char == nil || member.Account == nil || !member.InWorld ||
			!bindCriticalPlayer(member).current(w) || !instanceAllowsEvolution(member.Char, cfg.AllowedEvolutions) {
			return errors.New("A party member cannot enter this instance.")
		}
		if w.criticalTradeReserved(member) {
			return errCriticalPersistenceBusy
		}
		if !w.itemInstanceExitGraceAllowsItem(member.ID, source.Index) {
			return errors.New("Only the next Water Scroll can be used during the exit window.")
		}
		for _, active := range w.itemInstances {
			if active != nil && itemInstanceHasMember(active, member.ID) && !itemInstanceInExitGraceAt(active, at) {
				return errors.New("You or a party member is already inside another instance.")
			}
		}
	}
	grace := w.itemInstanceExitGraceForPlayer(p.ID)
	if newRoom && isDurablePrivateWaterConfig(*cfg) && cfg.ChainStart {
		if area, _ := instanceEntryAreaAt(cfg, p.X, p.Y); area.RequiresChain && grace == nil {
			return errors.New("This Room 1 Scroll requires a completed Water boss.")
		}
	}
	capacity := cfg.MaxPlayers
	if !newRoom && sharedTimedInstance(*cfg) {
		capacity = instancePlayerCapacity(cfg)
	}
	count := len(members)
	if !newRoom {
		count = w.instanceReservedParticipantCountWith(w.itemInstances[runtimeID], members)
	}
	if capacity > 0 && count > capacity {
		return fmt.Errorf("This instance accepts at most %d players.", capacity)
	}
	stages := instanceStages(cfg)
	if len(stages) == 0 || (newRoom && instanceSpawnCount(cfg) <= 0 && !isBigCubeConfig(cfg)) {
		return errors.New("This instance has no configured content.")
	}
	if newRoom {
		for _, stage := range stages {
			if !w.terrain.Walkable(stage.SpawnX, stage.SpawnY) ||
				(stage.Quiz != nil && (!w.terrain.Walkable(stage.Quiz.TrueX, stage.Quiz.TrueY) ||
					!w.terrain.Walkable(stage.Quiz.FalseX, stage.Quiz.FalseY))) {
				return errors.New("The instance content area is not walkable.")
			}
			for _, spawn := range stage.Spawns {
				def := w.npcDefByName(spawn.NPC)
				if def == nil || !def.IsMonster() || def.Score == nil || spawn.Count <= 0 ||
					(spawn.X != 0 && spawn.Y != 0 && !w.terrain.Walkable(spawn.X, spawn.Y)) {
					return errors.New("An instance monster is not configured.")
				}
			}
		}
		if !sharedTimedInstance(*cfg) && !independentPrivateInstance(*cfg) &&
			w.instanceAreaOccupied(cfg, runtimeID, members...) {
			return errors.New("This room is already occupied.")
		}
		inst = &ItemInstance{Config: cfgCopy(*cfg), RuntimeID: runtimeID, LeaderID: p.ID,
			MobIDs: make(map[uint16]struct{}), NPCIDs: make(map[uint16]struct{}),
			MobQuadrants: make(map[uint16]uint8), HellGateLichIDs: make(map[uint8]uint16),
			ScheduleEnd: scheduleEnd, State: "entry"}
		if isBigCubeConfig(cfg) && cfg.DurationSeconds > 0 {
			inst.HardDeadline = at.Add(time.Duration(cfg.DurationSeconds) * time.Second)
		} else if cfg.ActiveDurationSeconds > 0 && !scheduleEnd.IsZero() {
			inst.HardDeadline = scheduleEnd.Add(time.Duration(cfg.ActiveDurationSeconds) * time.Second)
		} else if cfg.TotalDurationSeconds > 0 {
			inst.HardDeadline = at.Add(time.Duration(cfg.TotalDurationSeconds) * time.Second)
		}
		prepareHellGateEntry(inst)
		prepareNightmareEntry(inst)
		plan.proposed.itemInstances[runtimeID] = inst
	}
	stage := stages[0]
	if !newRoom && !individual {
		var ok bool
		stage, ok = instanceStageForMob(inst)
		if !ok {
			return errors.New("The shared instance has no active room.")
		}
	}
	ignored := make(map[*Player]struct{}, len(members))
	for _, member := range members {
		ignored[member] = struct{}{}
	}
	space := itemInstanceGameplaySpace(inst)
	destinations, ok := w.planInstancePositionsIgnoringForInstance(members, stage.X, stage.Y, ignored, space)
	if !ok {
		return errors.New("The instance entrance is blocked.")
	}
	changes := make(map[*Player]*model.Account, len(members))
	tiles := make(map[criticalTeleportTile]*Player)
	for i, member := range members {
		account := accountStateSnapshot(member.Account)
		ch := &account.Chars[member.CharSlot]
		ch.X, ch.Y = destinations[i][0], destinations[i][1]
		if (newRoom || individual) && strings.EqualFold(strings.TrimSpace(cfg.NightmareTier), "arcane") && isCelestial(ch) && ch.NightmareTickets > 0 {
			ch.NightmareTickets--
		}
		changes[member] = account
		tiles[criticalTile(ch.X, ch.Y, space)] = member
		for _, oldRoom := range plan.proposed.itemInstances {
			if oldRoom != inst && itemInstanceHasMember(oldRoom, member.ID) {
				plan.proposed.detachInstanceMembersAfterCommit(oldRoom, map[uint16]struct{}{member.ID: {}})
			}
		}
		inst.MemberIDs = append(inst.MemberIDs, member.ID)
		plan.proposed.setPlayerInstanceIndex(member.ID, runtimeID)
	}
	if changes[p] == nil {
		return errCriticalPersistenceIdentity
	}
	if rule.Consume {
		consumeOne(&changes[p].Chars[p.CharSlot].Inv[slot])
	}
	if runKey != "" {
		plan.proposed.recordNightmarePartyRun(runKey)
	}
	if !newRoom && sharedTimedInstance(*cfg) && (inst.ScheduleEnd.IsZero() || scheduleEnd.After(inst.ScheduleEnd)) {
		inst.ScheduleEnd = scheduleEnd
	}
	created := []*Mob(nil)
	if newRoom && nightmareWaiting(inst) {
		created, err = w.planNightmareProtectors(inst, tiles, p)
		if err != nil {
			return err
		}
	} else if newRoom {
		created, err = w.planCriticalInstanceFirstWave(inst, stage, at, tiles, p)
		if err != nil {
			return err
		}
	}
	admitted := false
	release := func() {
		for tile := range tiles {
			delete(w.criticalTeleportTiles, tile)
		}
		for _, mob := range created {
			delete(w.criticalSummonIDs, mob.ID)
		}
	}
	check := func() error {
		for tile, owner := range tiles {
			reserved := w.criticalTeleportTiles[tile]
			x, y := tile.xy()
			if (!admitted && reserved != nil) || (admitted && reserved != owner) ||
				w.positionOccupiedInGameplaySpace(x, y, tile.space, nil, owner, ignored) {
				return errCriticalPersistenceIdentity
			}
		}
		for _, mob := range created {
			reserved := w.criticalSummonIDs[mob.ID]
			if w.mobsByID[mob.ID] != nil || w.groundItems[mob.ID] != nil ||
				(!admitted && reserved != nil) || (admitted && reserved != mob) {
				return errCriticalPersistenceIdentity
			}
		}
		return nil
	}
	err = w.queueCriticalInstanceAccounts(p, members, changes, plan, func() {
		for member, account := range changes {
			ch := &account.Chars[member.CharSlot]
			member.Char.Inv, member.Char.NightmareTickets = ch.Inv, ch.NightmareTickets
			member.X, member.Y, member.Char.X, member.Char.Y = ch.X, ch.Y, ch.X, ch.Y
			clearAirMove(member)
			clearPublishedPlayerMove(member)
		}
		for _, mob := range created {
			w.appendMobInstance(mob)
			w.mobsByID[mob.ID] = mob
			w.registerMobSpatial(mob)
		}
	}, func() {
		for _, mob := range created {
			w.recomputeMobActive(mob)
		}
		live := w.itemInstances[runtimeID]
		if newRoom {
			// Roll only after acknowledgement, before the initial slate snapshot.
			w.rollMagicChamberRune(live, live.CurrentStage)
		}
		w.publishInstanceMemberRelocation(members)
		w.publishCriticalInstanceEntryStatus(live)
		if !p.Session.IsClosed() {
			resyncUsedItem(p.Session, p, slot)
		}
		log.Printf("[#%d] committed instance entry config=%q runtime=%q members=%d mobs=%d",
			p.Session.ID, cfg.ID, runtimeID, len(members), live.Remaining)
	}, release, check)
	if err != nil {
		return err
	}
	if w.criticalTeleportTiles == nil {
		w.criticalTeleportTiles = make(map[criticalTeleportTile]*Player)
	}
	if w.criticalSummonIDs == nil {
		w.criticalSummonIDs = make(map[uint16]*Mob)
	}
	for tile, owner := range tiles {
		w.criticalTeleportTiles[tile] = owner
	}
	for _, mob := range created {
		w.criticalSummonIDs[mob.ID] = mob
	}
	admitted = true
	return nil
}

func expiredCriticalInstanceEntry(inst *ItemInstance, at time.Time) bool {
	for _, deadline := range []time.Time{instanceCombatDeadline(inst), inst.HardDeadline, inst.ScheduleEnd} {
		if !deadline.IsZero() && !at.Before(deadline) {
			return true
		}
	}
	return false
}

func expiredCriticalSharedInstanceEntry(inst *ItemInstance, at time.Time) bool {
	return (!inst.ScheduleEnd.IsZero() && !at.Before(inst.ScheduleEnd)) ||
		(!inst.HardDeadline.IsZero() && !at.Before(inst.HardDeadline))
}

func (w *World) publishCriticalInstanceEntryStatus(inst *ItemInstance) {
	if inst == nil {
		return
	}
	stages := instanceStages(&inst.Config)
	if inst.CurrentStage < 0 || inst.CurrentStage >= len(stages) {
		return
	}
	stage := stages[inst.CurrentStage]
	now := w.now()
	for _, id := range inst.MemberIDs {
		member := w.playersByID[id]
		if member == nil || !member.InWorld || member.Session == nil || member.Session.IsClosed() {
			continue
		}
		seconds := remainingInstanceSeconds(instanceCombatDeadline(inst), now)
		if isBigCubeInstance(inst) && stage.Quiz != nil {
			seconds = remainingInstanceSeconds(inst.QuizDeadline, now)
		}
		member.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, uint32(seconds)))
		member.Session.Send(wire.StandardParm(wire.OpInstanceMobs, instanceSignalID, instanceDisplayedMobCount(inst)))
		name := stage.Name
		if name == "" {
			name = inst.Config.Name
		}
		member.Session.Send(wire.MessagePanel(name))
		if isBigCubeInstance(inst) && stage.Quiz != nil && inst.QuizDeadline.After(now) {
			member.Session.Send(wire.InstanceQuestion(member.ID, strings.ReplaceAll(stage.Quiz.Question, "_", " "),
				uint32(inst.QuizDeadline.Sub(now)/time.Millisecond)))
		}
	}
}
