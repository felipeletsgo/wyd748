package game

import (
	"fmt"
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
		if inst == nil || !resumableInstance(inst) {
			continue
		}
		if !inst.HardDeadline.IsZero() && !now.Before(inst.HardDeadline) {
			continue
		}
		saved := model.InstanceRuntimeState{
			RuntimeID: runtimeID, ConfigID: inst.Config.ID,
			SharedGroup: inst.Config.SharedGroup, State: inst.State,
			CurrentStage: inst.CurrentStage, ScheduleEnd: inst.ScheduleEnd,
			HardDeadline: inst.HardDeadline, CombatDeadline: instanceCombatDeadline(inst),
			TransitionAt: inst.TransitionAt, QuizAt: inst.QuizAt, ExitAt: inst.ExitAt,
			RewardGranted:         inst.RewardGranted,
			HellGateVariant:       inst.HellGateVariant,
			HellGateClearedMask:   inst.HellGateClearedMask,
			HellGateLichSpawned:   inst.HellGateLichSpawnedMask,
			HellGateValidLichMask: inst.HellGateValidLichMask,
			HellGateWrongLich:     inst.HellGateWrongLich,
		}
		memberUIDs, leaderUID := w.instanceCharacterUIDs(runtimeID, inst)
		// A resumable event without a stable owner cannot be reattached after
		// a restart; keeping it would create an unreachable, resource-consuming
		// instance. Legacy fixtures without CharacterUID therefore remain
		// process-local and are cleaned by the normal runtime path.
		if len(memberUIDs) == 0 && !sharedTimedInstance(inst.Config) {
			continue
		}
		saved.MemberCharacterUIDs = memberUIDs
		saved.LeaderCharacterUID = leaderUID
		snapshot.Instances = append(snapshot.Instances, saved)
	}
	sort.Slice(snapshot.Instances, func(i, j int) bool {
		return snapshot.Instances[i].RuntimeID < snapshot.Instances[j].RuntimeID
	})
	return snapshot
}

// resumableInstance identifies event aggregates whose state and stable member
// identities may outlive a process restart. Water, shared Nightmare-like
// timed zones and Hell Gate all resume; ordinary Cube/short-lived private
// rooms remain process-local and are discarded on restart.
func resumableInstance(inst *ItemInstance) bool {
	if inst == nil {
		return false
	}
	return isDurablePrivateWaterInstance(inst) || sharedTimedInstance(inst.Config) ||
		strings.EqualFold(strings.TrimSpace(inst.Config.StateMachine), "hell_gate")
}

func resumableConfigOrWater(cfg model.VolatileInstance) bool {
	return isDurablePrivateWaterConfig(cfg) || sharedTimedInstance(cfg) ||
		strings.EqualFold(strings.TrimSpace(cfg.StateMachine), "hell_gate")
}

// isDurablePrivateWaterInstance identifies the Water rooms that may outlive a
// socket/process restart. It intentionally relies on the authoritative
// instance ID rather than a volatile item ID or client packet.
func isDurablePrivateWaterInstance(inst *ItemInstance) bool {
	if inst == nil {
		return false
	}
	return isDurablePrivateWaterConfig(inst.Config)
}

func isDurablePrivateWaterConfig(cfg model.VolatileInstance) bool {
	if sharedTimedInstance(cfg) ||
		strings.EqualFold(strings.TrimSpace(cfg.StateMachine), "hell_gate") {
		return false
	}
	id := strings.ToLower(strings.TrimSpace(cfg.ID))
	return strings.HasPrefix(id, "water-")
}

func appendUniqueString(values []string, value string) []string {
	value = strings.TrimSpace(value)
	if value == "" {
		return values
	}
	for _, existing := range values {
		if existing == value {
			return values
		}
	}
	return append(values, value)
}

// instanceCharacterUIDs merges live members with the detached identities kept
// by the world. The result is sorted so snapshots remain deterministic.
func (w *World) instanceCharacterUIDs(runtimeID string, inst *ItemInstance) ([]string, string) {
	if inst == nil {
		return nil, ""
	}
	memberUIDs := []string(nil)
	leaderUID := strings.TrimSpace(inst.LeaderCharacterUID)
	if pendingLeader := strings.TrimSpace(w.pendingInstanceLeaders[runtimeID]); pendingLeader != "" {
		// An explicit detached leader wins over a temporary live promotion.
		leaderUID = pendingLeader
	}
	// Keep the durable identity list authoritative even while a lightweight
	// fixture or a restored runtime has not rebuilt its pending map yet. The
	// append helper deduplicates this list with live and pending members below.
	for _, uid := range inst.MemberCharacterUIDs {
		memberUIDs = appendUniqueString(memberUIDs, uid)
	}
	for _, playerID := range inst.MemberIDs {
		p := w.playersByID[playerID]
		if p == nil || p.Char == nil || strings.TrimSpace(p.Char.UID) == "" {
			continue
		}
		uid := strings.TrimSpace(p.Char.UID)
		memberUIDs = appendUniqueString(memberUIDs, uid)
		if playerID == inst.LeaderID && leaderUID == "" {
			leaderUID = uid
		}
	}
	if pending := w.pendingInstanceMembers[runtimeID]; pending != nil {
		for uid := range pending {
			memberUIDs = appendUniqueString(memberUIDs, uid)
		}
	}
	sort.Strings(memberUIDs)
	return memberUIDs, leaderUID
}

func (w *World) stageSpawnPosition(inst *ItemInstance) (uint16, uint16, int, bool) {
	if inst == nil {
		return 0, 0, 0, false
	}
	stages := instanceStages(&inst.Config)
	if inst.CurrentStage < 0 || inst.CurrentStage >= len(stages) {
		return 0, 0, 0, false
	}
	stage := stages[inst.CurrentStage]
	x, y := stage.SpawnX, stage.SpawnY
	if x == 0 || y == 0 {
		x, y = stage.X, stage.Y
	}
	return x, y, stage.AreaRadius, x != 0 && y != 0
}

// attachRestoredInstanceMember reconnects a character to a resumable event
// before the first EnterWorld packet is built. The authoritative room position
// therefore appears as the initial position instead of a corrective teleport.
// A reconnect never trusts the former world/client ID.
func (w *World) attachRestoredInstanceMember(p *Player) {
	if w == nil || p == nil || p.Char == nil || strings.TrimSpace(p.Char.UID) == "" {
		return
	}
	uid := strings.TrimSpace(p.Char.UID)
	now := w.now()
	for runtimeID, pending := range w.pendingInstanceMembers {
		if _, ok := pending[uid]; !ok {
			continue
		}
		inst := w.itemInstances[runtimeID]
		if inst == nil || (!inst.HardDeadline.IsZero() && !now.Before(inst.HardDeadline)) ||
			(!inst.ExitAt.IsZero() && !now.Before(inst.ExitAt)) {
			continue
		}
		if active := w.itemInstanceForPlayer(p.ID); active != nil && active != inst {
			return
		}
		if !itemInstanceHasMember(inst, p.ID) {
			inst.MemberIDs = append(inst.MemberIDs, p.ID)
		}
		w.setPlayerInstanceIndex(p.ID, runtimeID)
		inst.MemberCharacterUIDs = appendUniqueString(inst.MemberCharacterUIDs, uid)
		if inst.LeaderID == 0 {
			leaderUID := strings.TrimSpace(w.pendingInstanceLeaders[runtimeID])
			if leaderUID == "" || leaderUID == uid {
				inst.LeaderID = p.ID
			}
		}
		if leaderUID := strings.TrimSpace(w.pendingInstanceLeaders[runtimeID]); leaderUID == uid ||
			strings.TrimSpace(inst.LeaderCharacterUID) == uid {
			inst.LeaderID = p.ID
		}
		if x, y, radius, ok := w.stageSpawnPosition(inst); ok {
			if radius < 1 {
				radius = 1
			}
			p.X, p.Y = w.findFreePlayerPositionInInstance(x, y, radius, p, runtimeID)
			p.Char.X, p.Char.Y = p.X, p.Y
		}
		delete(pending, uid)
		if len(pending) == 0 {
			delete(w.pendingInstanceMembers, runtimeID)
		}
		if w.pendingInstanceLeaders[runtimeID] == uid {
			delete(w.pendingInstanceLeaders, runtimeID)
		}
		w.markInstanceStateDirty()
		log.Printf("personagem %q reconectado na instancia %q", p.Char.Name, runtimeID)
		return
	}
}

func (w *World) persistInstanceState() error {
	store, ok := w.store.(instanceRuntimeStore)
	if !ok {
		return nil
	}
	return store.SaveInstanceState(w.instanceStateSnapshot())
}

func (w *World) saveAccountsAndInstanceState(accounts ...*model.Account) error {
	snapshot := w.instanceStateSnapshot()
	// A transaction-capable store still cannot make an unreachable private
	// Water room safe: the snapshot intentionally omits rooms that have no
	// stable CharacterUID. Refuse the account mutation instead of consuming a
	// ticket whose room would disappear after a restart.
	if len(accounts) > 0 {
		for runtimeID, inst := range w.itemInstances {
			if !resumableInstance(inst) {
				continue
			}
			uids, _ := w.instanceCharacterUIDs(runtimeID, inst)
			if len(uids) == 0 {
				// A completed predecessor can be kept in RAM during exit grace
				// solely for cleanup. Once its members are detached it has no
				// durable owner and is intentionally omitted from the snapshot.
				if len(inst.MemberIDs) == 0 && len(inst.MemberCharacterUIDs) == 0 &&
					strings.TrimSpace(inst.LeaderCharacterUID) == "" &&
					len(w.pendingInstanceMembers[runtimeID]) == 0 &&
					strings.TrimSpace(w.pendingInstanceLeaders[runtimeID]) == "" {
					continue
				}
				return fmt.Errorf("resumable instance %q has no stable character UID", runtimeID)
			}
		}
	}
	store, ok := w.store.(instanceRuntimeTransactionStore)
	if ok {
		return store.SaveGameStateWithInstanceState(nil, snapshot,
			accountPersistenceSnapshots(accounts...)...)
	}
	// A mixed operation (ticket/position/account plus a durable instance) must
	// never be split into two commits. The old fallback could consume a ticket
	// and then fail while persisting the room, or vice versa. Lightweight stores
	// may still persist account-only operations; they cannot claim atomic event
	// state without implementing the transaction interface.
	if len(accounts) > 0 {
		for _, inst := range w.itemInstances {
			if inst == nil {
				continue
			}
			if resumableInstance(inst) {
				return fmt.Errorf("store does not support atomic account/instance persistence")
			}
		}
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
		if !found || !resumableConfigOrWater(cfg) {
			continue
		}
		inst := &ItemInstance{
			Config: cfgCopy(cfg), RuntimeID: saved.RuntimeID, State: saved.State,
			CurrentStage: saved.CurrentStage, ScheduleEnd: saved.ScheduleEnd,
			MemberCharacterUIDs: append([]string(nil), saved.MemberCharacterUIDs...),
			LeaderCharacterUID:  saved.LeaderCharacterUID,
			HardDeadline:        saved.HardDeadline, TransitionAt: saved.TransitionAt,
			QuizAt: saved.QuizAt, ExitAt: saved.ExitAt, RewardGranted: saved.RewardGranted,
			MobIDs: make(map[uint16]struct{}),
			NPCIDs: make(map[uint16]struct{}), MobQuadrants: make(map[uint16]uint8),
			HellGateLichIDs:         make(map[uint8]uint16),
			HellGateVariant:         saved.HellGateVariant,
			HellGateClearedMask:     saved.HellGateClearedMask,
			HellGateLichSpawnedMask: saved.HellGateLichSpawned,
			HellGateValidLichMask:   saved.HellGateValidLichMask,
			HellGateWrongLich:       saved.HellGateWrongLich,
		}
		if w.pendingInstanceMembers == nil {
			w.pendingInstanceMembers = make(map[string]map[string]struct{})
		}
		if w.pendingInstanceLeaders == nil {
			w.pendingInstanceLeaders = make(map[string]string)
		}
		if resumableInstance(inst) {
			if len(saved.MemberCharacterUIDs) == 0 && !sharedTimedInstance(inst.Config) {
				// An old snapshot without stable ownership cannot be safely
				// reattached. Do not resurrect an unreachable event room.
				continue
			}
			pending := make(map[string]struct{}, len(saved.MemberCharacterUIDs))
			for _, uid := range saved.MemberCharacterUIDs {
				if strings.TrimSpace(uid) != "" {
					pending[strings.TrimSpace(uid)] = struct{}{}
				}
			}
			if len(pending) > 0 {
				w.pendingInstanceMembers[saved.RuntimeID] = pending
			}
			if leader := strings.TrimSpace(saved.LeaderCharacterUID); leader != "" {
				w.pendingInstanceLeaders[saved.RuntimeID] = leader
			}
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
		if isDurablePrivateWaterInstance(inst) && !inst.ExitAt.IsZero() {
			// A completed room in exit grace has no live content to respawn. The
			// ticket chain remains represented by the durable deadlines/UIDs.
			inst.Remaining = 0
		} else if _, hellGate := hellGateInstance(inst); hellGate {
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
	w.rebuildPlayerInstanceIndex()
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
