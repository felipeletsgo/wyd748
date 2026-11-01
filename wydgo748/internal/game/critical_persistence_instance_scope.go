package game

import (
	"maps"
	"reflect"
	"slices"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

// Per-runtime instance ownership (DOCS/server/server-correction-plan.md, W8).
// A keyed plan changes only the runtimes and globals that differ between its
// detached before and proposed states. It reserves, writes, checks and
// installs only that scope, so writes and ticks of other runtimes continue.
// A plan that reads other runtimes to decide (exclusive groups, scans of all
// rooms) stays unkeyed and reserves the whole aggregate.

type criticalInstanceScope struct {
	runtimes map[string]bool
	globals  bool
}

// instanceRowChannel is the channel whose rows hold this World's runtimes:
// a cluster member writes its own channel; a single World is the top level.
func (w *World) instanceRowChannel() byte {
	if w.cluster == nil || w.channel == 0 {
		return 1
	}
	return w.channel
}

func (w *World) criticalInstanceWholeReserved() bool {
	return w != nil && w.criticalWrites != nil &&
		(w.criticalWrites.queue.instances != 0 || w.criticalWrites.queue.failedInstances != 0)
}

// criticalInstanceRuntimeReserved reports a pending or failed write that owns
// this runtime, either through its key or through the whole aggregate.
func (w *World) criticalInstanceRuntimeReserved(runtimeID string) bool {
	if w.criticalInstanceWholeReserved() {
		return true
	}
	if w == nil || w.criticalWrites == nil {
		return false
	}
	q := w.criticalWrites.queue
	name := instanceRowKey(store.InstanceParticipant{Channel: w.instanceRowChannel(), RuntimeID: runtimeID})
	return q.instanceKeys[name] != 0 || q.failedInstanceKeys[name] != 0
}

// computeScope diffs the detached before and proposed states of a plan.
func (plan *criticalInstanceDeparture) computeScope() criticalInstanceScope {
	scope := criticalInstanceScope{runtimes: make(map[string]bool)}
	before, proposed := plan.before, plan.proposed
	for id, inst := range before.itemInstances {
		next, present := proposed.itemInstances[id]
		if !present || (inst == nil) != (next == nil) || inst != nil && !reflect.DeepEqual(*inst, *next) {
			scope.runtimes[id] = true
		}
	}
	for id := range proposed.itemInstances {
		if _, present := before.itemInstances[id]; !present {
			scope.runtimes[id] = true
		}
	}
	for player, runtime := range before.playerInstance {
		if next := proposed.playerInstance[player]; next != runtime {
			scope.runtimes[runtime] = true
			if next != "" {
				scope.runtimes[next] = true
			}
		}
	}
	for player, runtime := range proposed.playerInstance {
		if _, present := before.playerInstance[player]; !present && runtime != "" {
			scope.runtimes[runtime] = true
		}
	}
	for id := range mergedKeys(before.pendingInstanceMembers, proposed.pendingInstanceMembers) {
		if !maps.Equal(before.pendingInstanceMembers[id], proposed.pendingInstanceMembers[id]) {
			scope.runtimes[id] = true
		}
	}
	for id := range mergedKeys(before.pendingInstanceLeaders, proposed.pendingInstanceLeaders) {
		if before.pendingInstanceLeaders[id] != proposed.pendingInstanceLeaders[id] {
			scope.runtimes[id] = true
		}
	}
	delete(scope.runtimes, "")
	scope.globals = !maps.Equal(before.nightmarePartyRuns, proposed.nightmarePartyRuns) ||
		before.gambleJackpot != proposed.gambleJackpot || before.gamblePool != proposed.gamblePool
	return scope
}

func mergedKeys[V any](a, b map[string]V) map[string]struct{} {
	keys := make(map[string]struct{}, len(a)+len(b))
	for key := range a {
		keys[key] = struct{}{}
	}
	for key := range b {
		keys[key] = struct{}{}
	}
	return keys
}

// keyed narrows a plan to its computed scope. Call after proposed is final.
func (plan *criticalInstanceDeparture) keyed(w *World) []store.InstanceParticipant {
	if plan == nil {
		return nil
	}
	scope := plan.computeScope()
	plan.scope = &scope
	keys := make([]store.InstanceParticipant, 0, len(scope.runtimes)+1)
	channel := w.instanceRowChannel()
	for id := range scope.runtimes {
		keys = append(keys, store.InstanceParticipant{Channel: channel, RuntimeID: id})
	}
	if scope.globals {
		keys = append(keys, store.InstanceParticipant{Channel: channel})
	}
	slices.SortFunc(keys, func(a, b store.InstanceParticipant) int { return strings.Compare(a.RuntimeID, b.RuntimeID) })
	return keys
}

// writeKeys returns the plan's participant keys, or nil for a whole plan.
func (plan *criticalInstanceDeparture) writeKeys(w *World) []store.InstanceParticipant {
	if plan == nil || plan.whole {
		return nil
	}
	return plan.keyed(w)
}

// checkScoped verifies that the participants of a keyed plan are unchanged.
func (plan *criticalInstanceDeparture) checkScoped(w *World) error {
	scope := plan.scope
	current := cloneCriticalInstanceMembership(w)
	for id := range scope.runtimes {
		if w.itemInstances[id] != plan.originals[id] ||
			!reflect.DeepEqual(current.itemInstances[id], plan.before.itemInstances[id]) ||
			!maps.Equal(current.pendingInstanceMembers[id], plan.before.pendingInstanceMembers[id]) ||
			current.pendingInstanceLeaders[id] != plan.before.pendingInstanceLeaders[id] {
			return errCriticalPersistenceIdentity
		}
	}
	scoped := func(index map[uint16]string) map[uint16]string {
		out := make(map[uint16]string)
		for player, runtime := range index {
			if scope.runtimes[runtime] {
				out[player] = runtime
			}
		}
		return out
	}
	if !maps.Equal(scoped(current.playerInstance), scoped(plan.before.playerInstance)) {
		return errCriticalPersistenceIdentity
	}
	if scope.globals && (!maps.Equal(current.nightmarePartyRuns, plan.before.nightmarePartyRuns) ||
		current.gambleJackpot != plan.before.gambleJackpot || current.gamblePool != plan.before.gamblePool) {
		return errCriticalPersistenceIdentity
	}
	return nil
}

// installScoped installs only the plan's participants. Other runtimes keep
// their live state and dirt; this write persisted the scoped rows already.
func (plan *criticalInstanceDeparture) installScoped(w *World) {
	scope := plan.scope
	if w.itemInstances == nil {
		w.itemInstances = make(map[string]*ItemInstance)
	}
	for id := range scope.runtimes {
		next := plan.proposed.itemInstances[id]
		if next == nil {
			delete(w.itemInstances, id)
		} else if existing := w.itemInstances[id]; existing != nil {
			*existing = *next // keep pointer identity held by mobs
		} else {
			w.itemInstances[id] = next
		}
		if members, present := plan.proposed.pendingInstanceMembers[id]; present {
			if w.pendingInstanceMembers == nil {
				w.pendingInstanceMembers = make(map[string]map[string]struct{})
			}
			w.pendingInstanceMembers[id] = members
		} else {
			delete(w.pendingInstanceMembers, id)
		}
		if leader, present := plan.proposed.pendingInstanceLeaders[id]; present {
			if w.pendingInstanceLeaders == nil {
				w.pendingInstanceLeaders = make(map[string]string)
			}
			w.pendingInstanceLeaders[id] = leader
		} else {
			delete(w.pendingInstanceLeaders, id)
		}
	}
	for player := range mergedPlayerKeys(plan.before.playerInstance, plan.proposed.playerInstance) {
		before, next := plan.before.playerInstance[player], plan.proposed.playerInstance[player]
		if before == next {
			continue
		}
		if next == "" {
			delete(w.playerInstance, player)
		} else {
			if w.playerInstance == nil {
				w.playerInstance = make(map[uint16]string)
			}
			w.playerInstance[player] = next
		}
	}
	if scope.globals {
		w.nightmarePartyRuns = plan.proposed.nightmarePartyRuns
		w.gambleJackpot, w.gamblePool = plan.proposed.gambleJackpot, plan.proposed.gamblePool
	}
	// A new generation tells unkeyed plans that state changed; prior dirt of
	// other runtimes stays pending for the next flush.
	w.criticalInstanceGeneration = &criticalInstanceGeneration{}
}

func mergedPlayerKeys(a, b map[uint16]string) map[uint16]struct{} {
	keys := make(map[uint16]struct{}, len(a)+len(b))
	for key := range a {
		keys[key] = struct{}{}
	}
	for key := range b {
		keys[key] = struct{}{}
	}
	return keys
}

// mergeCommittedChannelInstances records a keyed commit in the cluster's last
// committed root. Keys are exclusive, so commits of one key never reorder.
func (w *World) mergeCommittedChannelInstances(saved *model.InstanceStateSnapshot, keys []store.InstanceParticipant) {
	if w.cluster == nil || saved == nil {
		return
	}
	root := w.cluster.instances
	if root == nil {
		root = &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}
		w.cluster.instances = root
	}
	channelOf := func(snapshot *model.InstanceStateSnapshot, channel byte, create bool) *model.InstanceStateSnapshot {
		if channel == 1 {
			return snapshot
		}
		if snapshot.Channels[channel] == nil && create {
			if snapshot.Channels == nil {
				snapshot.Channels = make(map[byte]*model.InstanceStateSnapshot)
			}
			snapshot.Channels[channel] = &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}
		}
		return snapshot.Channels[channel]
	}
	for _, key := range keys {
		source := channelOf(saved, key.Channel, false)
		target := channelOf(root, key.Channel, true)
		if key.RuntimeID == "" {
			if source == nil {
				target.NightmarePartyRuns, target.GambleJackpot, target.GamblePool = nil, 0, 0
			} else {
				target.NightmarePartyRuns = maps.Clone(source.NightmarePartyRuns)
				target.GambleJackpot, target.GamblePool = source.GambleJackpot, source.GamblePool
			}
			continue
		}
		target.Instances = slices.DeleteFunc(target.Instances, func(r model.InstanceRuntimeState) bool {
			return r.RuntimeID == key.RuntimeID
		})
		if source != nil {
			for _, runtime := range source.Instances {
				if runtime.RuntimeID == key.RuntimeID {
					target.Instances = append(target.Instances, runtime)
					break
				}
			}
		}
		slices.SortFunc(target.Instances, func(a, b model.InstanceRuntimeState) int {
			return strings.Compare(a.RuntimeID, b.RuntimeID)
		})
	}
}
