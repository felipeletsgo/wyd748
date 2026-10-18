package game

import (
	"maps"
	"reflect"
	"slices"
	"time"

	"wydgo/internal/model"
)

// Departure is planned without borrowing World locks, timers or atomic fields.
// The existing membership algorithm runs on detached runtime containers; live
// entity lookups are read-only and remain on the actor, never on the worker.
type criticalInstanceDeparture struct {
	at         time.Time
	generation *criticalInstanceGeneration
	before     *World
	proposed   *World
	originals  map[string]*ItemInstance
}

func cloneCriticalInstanceMembership(w *World) *World {
	copy := &World{
		playersByID:            w.playersByID,
		itemInstances:          make(map[string]*ItemInstance, len(w.itemInstances)),
		playerInstance:         maps.Clone(w.playerInstance),
		pendingInstanceMembers: make(map[string]map[string]struct{}, len(w.pendingInstanceMembers)),
		pendingInstanceLeaders: maps.Clone(w.pendingInstanceLeaders),
		nightmarePartyRuns:     maps.Clone(w.nightmarePartyRuns),
		gambleJackpot:          w.gambleJackpot, gamblePool: w.gamblePool,
	}
	for id, inst := range w.itemInstances {
		if inst == nil {
			copy.itemInstances[id] = nil
			continue
		}
		clone := *inst
		clone.Config = cfgCopy(inst.Config)
		clone.MemberIDs = slices.Clone(inst.MemberIDs)
		clone.MemberCharacterUIDs = slices.Clone(inst.MemberCharacterUIDs)
		clone.MobIDs, clone.NPCIDs = maps.Clone(inst.MobIDs), maps.Clone(inst.NPCIDs)
		clone.NightmareProtectors = maps.Clone(inst.NightmareProtectors)
		clone.NightmareSpawns = maps.Clone(inst.NightmareSpawns)
		clone.MobQuadrants, clone.HellGateLichIDs = maps.Clone(inst.MobQuadrants), maps.Clone(inst.HellGateLichIDs)
		copy.itemInstances[id] = &clone
	}
	for id, members := range w.pendingInstanceMembers {
		copy.pendingInstanceMembers[id] = maps.Clone(members)
	}
	return copy
}

func (w *World) prepareCriticalInstanceDeparture(p *Player) (*criticalInstanceDeparture, error) {
	member := false
	for _, inst := range w.itemInstances {
		if inst != nil && itemInstanceHasMember(inst, p.ID) {
			member = true
			break
		}
	}
	if !member {
		// A stale space index must not silently become a public teleport.
		if w.playerRuntimeInstanceID(p.ID) != "" {
			return nil, errCriticalPersistenceIdentity
		}
		return nil, nil
	}
	if w.criticalInstanceReserved() {
		return nil, errCriticalPersistenceBusy
	}
	plan := &criticalInstanceDeparture{
		at: w.now(), generation: w.criticalInstanceGeneration,
		before: cloneCriticalInstanceMembership(w), proposed: cloneCriticalInstanceMembership(w),
		originals: maps.Clone(w.itemInstances),
	}
	plan.proposed.detachPlayerFromItemInstancesMode(p.ID, plan.at, false)
	return plan, nil
}

func (plan *criticalInstanceDeparture) check(w *World) error {
	if plan == nil {
		return nil
	}
	current := cloneCriticalInstanceMembership(w)
	if w.criticalInstanceGeneration != plan.generation || !maps.Equal(w.itemInstances, plan.originals) ||
		!reflect.DeepEqual(current.itemInstances, plan.before.itemInstances) ||
		!maps.Equal(current.playerInstance, plan.before.playerInstance) ||
		!reflect.DeepEqual(current.pendingInstanceMembers, plan.before.pendingInstanceMembers) ||
		!maps.Equal(current.pendingInstanceLeaders, plan.before.pendingInstanceLeaders) ||
		!maps.Equal(current.nightmarePartyRuns, plan.before.nightmarePartyRuns) ||
		!reflect.DeepEqual(current.instanceStateSnapshotAt(plan.at), plan.before.instanceStateSnapshotAt(plan.at)) {
		return errCriticalPersistenceIdentity
	}
	return nil
}

func (plan *criticalInstanceDeparture) snapshot() *model.InstanceStateSnapshot {
	if plan == nil {
		return nil
	}
	return plan.proposed.instanceStateSnapshotAt(plan.at)
}

func (plan *criticalInstanceDeparture) install(w *World) {
	if plan == nil {
		return
	}
	// Preserve runtime pointer identities held by mobs and other actor consumers.
	if w.itemInstances == nil {
		w.itemInstances = make(map[string]*ItemInstance)
	}
	for id := range w.itemInstances {
		if _, present := plan.proposed.itemInstances[id]; !present {
			delete(w.itemInstances, id)
		}
	}
	for id, inst := range plan.proposed.itemInstances {
		if inst != nil {
			if existing := w.itemInstances[id]; existing != nil {
				*existing = *inst
			} else {
				w.itemInstances[id] = inst
			}
		}
	}
	w.playerInstance = plan.proposed.playerInstance
	w.pendingInstanceMembers = plan.proposed.pendingInstanceMembers
	w.pendingInstanceLeaders = plan.proposed.pendingInstanceLeaders
	w.nightmarePartyRuns = plan.proposed.nightmarePartyRuns
	w.markInstanceStateDirty()
	// This write covers the complete retained aggregate, including prior dirt.
	w.instanceStateDirty = false
}
