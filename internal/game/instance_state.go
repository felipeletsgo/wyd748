package game

import (
	"log"
	"sort"
	"strings"
	"time"

	"wydgo/internal/model"
)

// instanceRuntimeStore is optional so lightweight test stores and the legacy
// JSON adaptor can keep working. PostgreSQL and JSONStore implement it.
type instanceRuntimeStore interface {
	LoadInstanceState() (*model.InstanceStateSnapshot, error)
	SaveInstanceState(*model.InstanceStateSnapshot) error
}

// instanceRuntimeTransactionStore extends the optional store with the atomic
// account+event commit used when opening/joining a shared instance.
type instanceRuntimeTransactionStore interface {
	instanceRuntimeStore
	SaveGameStateWithInstanceState(*model.GuildRegistry, *model.InstanceStateSnapshot,
		...*model.Account) error
}

func (w *World) instanceStateSnapshot() *model.InstanceStateSnapshot {
	snapshot := &model.InstanceStateSnapshot{
		Version:            model.InstanceStateVersion,
		NightmarePartyRuns: make(map[string]int, len(w.nightmarePartyRuns)),
	}
	for key, count := range w.nightmarePartyRuns {
		if count > 0 {
			snapshot.NightmarePartyRuns[key] = count
		}
	}
	now := w.now()
	for runtimeID, inst := range w.itemInstances {
		if inst == nil || (!sharedTimedInstance(inst.Config) &&
			!strings.EqualFold(strings.TrimSpace(inst.Config.StateMachine), "hell_gate")) {
			continue
		}
		if !inst.HardDeadline.IsZero() && !now.Before(inst.HardDeadline) {
			continue
		}
		snapshot.Instances = append(snapshot.Instances, model.InstanceRuntimeState{
			RuntimeID: runtimeID, ConfigID: inst.Config.ID,
			SharedGroup: inst.Config.SharedGroup, State: inst.State,
			CurrentStage: inst.CurrentStage, ScheduleEnd: inst.ScheduleEnd,
			HardDeadline: inst.HardDeadline, CombatDeadline: instanceCombatDeadline(inst),
			HellGateVariant:       inst.HellGateVariant,
			HellGateClearedMask:   inst.HellGateClearedMask,
			HellGateLichSpawned:   inst.HellGateLichSpawnedMask,
			HellGateValidLichMask: inst.HellGateValidLichMask,
			HellGateWrongLich:     inst.HellGateWrongLich,
		})
	}
	sort.Slice(snapshot.Instances, func(i, j int) bool {
		return snapshot.Instances[i].RuntimeID < snapshot.Instances[j].RuntimeID
	})
	return snapshot
}

func (w *World) persistInstanceState() error {
	store, ok := w.store.(instanceRuntimeStore)
	if !ok {
		return nil
	}
	return store.SaveInstanceState(w.instanceStateSnapshot())
}

func (w *World) saveAccountsAndInstanceState(accounts ...*model.Account) error {
	store, ok := w.store.(instanceRuntimeTransactionStore)
	if ok {
		for _, account := range accounts {
			pinAccountEntryPositions(account)
		}
		return store.SaveGameStateWithInstanceState(nil, w.instanceStateSnapshot(), accounts...)
	}
	if err := w.saveAccountsAtomic(accounts...); err != nil {
		return err
	}
	return w.persistInstanceState()
}

func (w *World) restoreInstanceState() error {
	store, ok := w.store.(instanceRuntimeStore)
	if !ok {
		return nil
	}
	snapshot, err := store.LoadInstanceState()
	if err != nil {
		return err
	}
	if snapshot == nil || snapshot.Version != model.InstanceStateVersion {
		return nil
	}
	for key, count := range snapshot.NightmarePartyRuns {
		if count > 0 {
			w.nightmarePartyRuns[key] = count
		}
	}
	now := w.now()
	for _, saved := range snapshot.Instances {
		if saved.RuntimeID == "" || saved.ConfigID == "" ||
			(!saved.HardDeadline.IsZero() && !now.Before(saved.HardDeadline)) {
			continue
		}
		cfg, found := w.instanceConfigByID(saved.ConfigID, saved.RuntimeID)
		if !found || (!sharedTimedInstance(cfg) &&
			!strings.EqualFold(strings.TrimSpace(cfg.StateMachine), "hell_gate")) {
			continue
		}
		inst := &ItemInstance{
			Config: cfgCopy(cfg), RuntimeID: saved.RuntimeID, State: saved.State,
			CurrentStage: saved.CurrentStage, ScheduleEnd: saved.ScheduleEnd,
			HardDeadline: saved.HardDeadline, MobIDs: make(map[uint16]struct{}),
			NPCIDs: make(map[uint16]struct{}), MobQuadrants: make(map[uint16]uint8),
			HellGateLichIDs:         make(map[uint8]uint16),
			HellGateVariant:         saved.HellGateVariant,
			HellGateClearedMask:     saved.HellGateClearedMask,
			HellGateLichSpawnedMask: saved.HellGateLichSpawned,
			HellGateValidLichMask:   saved.HellGateValidLichMask,
			HellGateWrongLich:       saved.HellGateWrongLich,
		}
		if inst.CurrentStage < 0 {
			inst.CurrentStage = 0
		}
		w.itemInstances[saved.RuntimeID] = inst
		// Rebuild content from data; no old player or mob ID is trusted after a
		// process restart. A shared zone with no members remains joinable. Hell
		// Gate needs its state-machine phase restored before the generic stage
		// path, otherwise a partially solved puzzle would respawn only the entry
		// controller and silently discard the selected Lich pair.
		if _, hellGate := hellGateInstance(inst); hellGate {
			if !w.restoreHellGateRuntime(inst, saved, now) {
				delete(w.itemInstances, saved.RuntimeID)
				continue
			}
		} else {
			stages := instanceStages(&inst.Config)
			if inst.CurrentStage >= len(stages) || func() bool {
				ok, _ := w.spawnItemInstanceStage(inst, inst.CurrentStage, now, false, false)
				return !ok
			}() {
				delete(w.itemInstances, saved.RuntimeID)
				continue
			}
		}
		if !saved.CombatDeadline.IsZero() {
			setInstanceCombatDeadline(inst, saved.CombatDeadline)
		}
		log.Printf("INSTANCIA %q restaurada apos reinicio", saved.RuntimeID)
	}
	return nil
}

// restoreHellGateRuntime reconstructs the content that can be represented by
// the durable aggregate. Entity IDs and per-mob HP are intentionally not
// trusted across a process restart; the selected variant, puzzle masks and
// phase are authoritative and are reapplied to fresh native templates.
func (w *World) restoreHellGateRuntime(inst *ItemInstance,
	saved model.InstanceRuntimeState, now time.Time) bool {
	if inst == nil {
		return false
	}
	switch strings.ToLower(strings.TrimSpace(saved.State)) {
	case "final":
		return w.spawnHellGateFinal(inst, now)
	case "lich_puzzle":
		inst.HellGateVariant = saved.HellGateVariant
		inst.HellGateValidLichMask = saved.HellGateValidLichMask
		if inst.HellGateValidLichMask == 0 {
			if inst.HellGateVariant == 2 {
				inst.HellGateValidLichMask = hellGateBit(1) | hellGateBit(3)
			} else {
				inst.HellGateValidLichMask = hellGateBit(2) | hellGateBit(4)
			}
		}
		// Rebuild only the unfinished quadrants. Calling the normal transition
		// helper here would respawn every wave and silently undo progress already
		// persisted in ClearedMask; a restart must never resurrect a killed wave.
		cfg, ok := hellGateInstance(inst)
		if !ok || len(cfg.Quadrants) != 4 {
			return false
		}
		// A restauração pode falhar depois de materializar um ou mais
		// quadrantes (por exemplo, falta de IDs ou template inválido). Nunca
		// deixe essa população parcialmente criada no mapa quando o agregado
		// for descartado pelo chamador.
		fail := func() bool {
			w.removeHellGateMobs(inst)
			return false
		}
		w.removeHellGateMobs(inst)
		inst.State = "lich_puzzle"
		inst.HellGateClearedMask = saved.HellGateClearedMask
		inst.HellGateLichSpawnedMask = 0
		inst.HellGateWrongLich = saved.HellGateWrongLich
		inst.MobIDs = make(map[uint16]struct{})
		inst.MobQuadrants = make(map[uint16]uint8)
		inst.HellGateLichIDs = make(map[uint8]uint16)
		inst.Remaining = 0
		for _, q := range cfg.Quadrants {
			quadrant := uint8(q.ID)
			bit := hellGateBit(quadrant)
			if saved.HellGateClearedMask&bit != 0 {
				continue
			}
			if saved.HellGateLichSpawned&bit != 0 {
				if !w.spawnHellGateLich(inst, quadrant) {
					return fail()
				}
				continue
			}
			created, spawnedOK := w.spawnHellGateWave(inst, q.Spawns, quadrant, true)
			if !spawnedOK {
				return fail()
			}
			for _, mob := range created {
				inst.MobIDs[mob.ID] = struct{}{}
				inst.Remaining++
			}
		}
		if inst.CombatDeadline.IsZero() {
			deadline := inst.HardDeadline
			if deadline.IsZero() && inst.Config.ActiveDurationSeconds > 0 {
				deadline = now.Add(time.Duration(inst.Config.ActiveDurationSeconds) * time.Second)
			}
			setInstanceCombatDeadline(inst, deadline)
		}
		return true
	default:
		stages := instanceStages(&inst.Config)
		if len(stages) == 0 || inst.CurrentStage >= len(stages) {
			return false
		}
		ok, _ := w.spawnItemInstanceStage(inst, inst.CurrentStage, now, false, false)
		return ok
	}
}

func (w *World) instanceConfigByID(configID, runtimeID string) (model.VolatileInstance, bool) {
	// ConfigID e a identidade persistida. Procure-a em uma passagem separada:
	// varios ingressos podem compartilhar o mesmo runtime (SharedGroup), e o
	// range aleatorio dos maps fazia a restauracao escolher outro alias antes
	// de chegar ao template exato.
	for _, instance := range w.volatiles.Instances {
		if instance.ID == configID {
			return cfgCopy(instance), true
		}
	}
	for _, rule := range w.volatiles.Items {
		if rule.Instance == nil {
			continue
		}
		if rule.Instance.ID == configID {
			return cfgCopy(*rule.Instance), true
		}
	}
	for _, rule := range w.volatiles.Rules {
		if rule.Instance == nil {
			continue
		}
		if rule.Instance.ID == configID {
			return cfgCopy(*rule.Instance), true
		}
	}
	if runtimeID == "" {
		return model.VolatileInstance{}, false
	}
	for _, instance := range w.volatiles.Instances {
		if instanceRuntimeKey(&instance) == runtimeID {
			return cfgCopy(instance), true
		}
	}
	for _, rule := range w.volatiles.Items {
		if rule.Instance != nil && instanceRuntimeKey(rule.Instance) == runtimeID {
			return cfgCopy(*rule.Instance), true
		}
	}
	for _, rule := range w.volatiles.Rules {
		if rule.Instance != nil && instanceRuntimeKey(rule.Instance) == runtimeID {
			return cfgCopy(*rule.Instance), true
		}
	}
	return model.VolatileInstance{}, false
}

func (w *World) markInstanceStateDirty() {
	if w != nil {
		w.instanceStateDirty = true
	}
}

func (w *World) flushInstanceStateIfDirty() {
	if w == nil || !w.instanceStateDirty {
		return
	}
	if err := w.persistInstanceState(); err != nil {
		log.Printf("estado de instancias: %v", err)
		return
	}
	w.instanceStateDirty = false
}
