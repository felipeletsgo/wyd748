package game

import (
	"errors"
	"reflect"
	"sync"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

type criticalInstanceTestStore struct{ *criticalTestStore }

func (*criticalInstanceTestStore) LoadInstanceState() (*model.InstanceStateSnapshot, error) {
	return nil, nil
}

func (s *criticalInstanceTestStore) SaveInstanceState(state *model.InstanceStateSnapshot) error {
	return s.record(criticalWrite{instances: state})
}

type criticalInstanceAtomicTestStore struct{ *criticalInstanceTestStore }

func (s *criticalInstanceAtomicTestStore) SaveGameStateWithInstanceState(_ *model.GuildRegistry,
	state *model.InstanceStateSnapshot, accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts, instances: state})
}

func TestCriticalInstanceAggregateDetachedAndSerializedThroughAcknowledgement(t *testing.T) {
	for _, atomic := range []bool{false, true} {
		t.Run(map[bool]string{false: "instance-only", true: "atomic-accounts"}[atomic], func(t *testing.T) {
			gate, release := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			backend := &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}
			queue := newCriticalTestQueue(t, backend, 2, 3)
			t.Cleanup(release)
			state := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion,
				NightmarePartyRuns: map[string]int{"window": 2}, GamblePool: 100,
				Instances: []model.InstanceRuntimeState{{RuntimeID: "water-room", MemberCharacterUIDs: []string{"owner"}}}}
			write := criticalWrite{instances: state}
			if atomic {
				write.accounts = []*model.Account{{Name: "owner"}, {Name: "member"}}
			}
			id, err := queue.submit(write)
			if err != nil {
				t.Fatal(err)
			}
			stored := awaitCriticalWrite(t, base.entered)
			stored.instances.NightmarePartyRuns["window"] = 9
			stored.instances.Instances[0].MemberCharacterUIDs[0] = "worker-change"
			stored.instances.Instances[0].RewardGranted = true
			if state.NightmarePartyRuns["window"] != 2 || state.Instances[0].MemberCharacterUIDs[0] != "owner" || state.Instances[0].RewardGranted {
				t.Fatal("worker received aliases into the actor's nested instance state")
			}
			if len(stored.accounts) != len(write.accounts) || atomic &&
				!reflect.DeepEqual(stored.accounts, accountPersistenceSnapshots(write.accounts...)) {
				t.Fatal("atomic form lost accounts")
			}
			if _, err := queue.submit(criticalWrite{instances: state}); !errors.Is(err, errCriticalPersistenceBusy) {
				t.Fatalf("overlapping global aggregate accepted: %v", err)
			}
			release()
			result := awaitCriticalResult(t, queue)
			if result.err != nil || result.id != id || queue.instances != id {
				t.Fatal("completion released the global instance reservation before acknowledgement")
			}
			if _, err := queue.submit(criticalWrite{instances: state}); !errors.Is(err, errCriticalPersistenceBusy) {
				t.Fatal("completed-but-unacknowledged aggregate admitted a competing writer")
			}
			if err := queue.acknowledge(id); err != nil || queue.instances != 0 {
				t.Fatalf("successful acknowledgement retained the instance reservation: %v", err)
			}
			if _, err := queue.submit(criticalWrite{instances: state}); err != nil {
				t.Fatalf("released aggregate could not be resubmitted: %v", err)
			}
			awaitCriticalResult(t, queue)
		})
	}
}

func TestCriticalInstanceUnsupportedFormsRejectBeforeWriting(t *testing.T) {
	for _, scenario := range []string{"no-instance-adapter", "no-atomic-adapter", "combined-sidecar"} {
		t.Run(scenario, func(t *testing.T) {
			base := &criticalTestStore{entered: make(chan criticalWrite, 2)}
			queue := newCriticalTestQueue(t, base, 1, 2)
			write := criticalWrite{instances: &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}}
			if scenario != "no-instance-adapter" {
				queue.store = &criticalInstanceTestStore{base}
				write.accounts = []*model.Account{{Name: "owner"}}
			}
			if scenario == "combined-sidecar" {
				queue.store = &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}
				write.character = &criticalCharacterState{uid: "owner"}
			}
			if id, err := queue.submit(write); id != 0 || !errors.Is(err, errCriticalPersistenceUnsupported) ||
				queue.instances != 0 || len(queue.pending) != 0 || len(base.entered) != 0 {
				t.Fatalf("unsupported form wrote or reserved: id=%d err=%v", id, err)
			}
		})
	}
}

func TestCriticalInstanceFlushRetainsLaterDirtyGenerationAndRejectsInlineOverlap(t *testing.T) {
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 3), release: gate}
	queue := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}, 1, 2)
	t.Cleanup(release)
	w, fallback := newShutdownWorld()
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	w.gamblePool = 100
	w.markInstanceStateDirty()
	w.flushInstanceStateIfDirty()
	first := awaitCriticalWrite(t, base.entered)
	w.gamblePool = 200
	w.markInstanceStateDirty()
	for range 3 {
		w.flushInstanceStateIfDirty()
	}
	if first.instances.GamblePool != 100 || !w.instanceStateDirty || len(base.entered) != 0 {
		t.Fatal("pending snapshot changed or admitted an overlapping flush")
	}
	if err := w.persistInstanceState(); !errors.Is(err, errCriticalPersistenceBusy) {
		t.Fatalf("inline instance writer bypassed reservation: %v", err)
	}
	if err := w.saveAccountsAndInstanceState(&model.Account{Name: "other"}); !errors.Is(err, errCriticalPersistenceBusy) || fallback.saves != 0 {
		t.Fatalf("inline atomic instance writer bypassed reservation: %v", err)
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if !w.instanceStateDirty || w.criticalInstanceReserved() {
		t.Fatal("older completion erased later dirty state or retained a successful reservation")
	}
	w.flushInstanceStateIfDirty()
	second := awaitCriticalWrite(t, base.entered)
	if second.instances.GamblePool != 200 {
		t.Fatal("follow-up flush did not snapshot the later mutation")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if w.instanceStateDirty || len(queue.pending) != 0 {
		t.Fatal("current generation was not acknowledged")
	}
}

func TestCriticalInstanceFlushRejectedAdmissionKeepsDirtyWithoutInlineFallback(t *testing.T) {
	for _, scenario := range []string{"full", "stopping", "unsupported"} {
		t.Run(scenario, func(t *testing.T) {
			gate, release := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 3), release: gate}
			queue := newCriticalTestQueue(t, &criticalInstanceTestStore{base}, 1, 1)
			t.Cleanup(release)
			w, fallback := newShutdownWorld()
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			switch scenario {
			case "full":
				if _, err := w.criticalWrites.submit(criticalWrite{accounts: []*model.Account{{Name: "occupied"}}},
					func(criticalPersistenceResult) error { return nil }); err != nil {
					t.Fatal(err)
				}
				awaitCriticalWrite(t, base.entered)
			case "stopping":
				queue.closeAdmission()
			case "unsupported":
				queue.store = base
			}
			w.markInstanceStateDirty()
			generation := w.criticalInstanceGeneration
			for range 3 {
				w.flushInstanceStateIfDirty()
			}
			if !w.instanceStateDirty || w.criticalInstanceGeneration != generation ||
				w.criticalInstanceReserved() || fallback.saves != 0 || len(base.entered) != 0 {
				t.Fatal("rejected flush lost dirty state, reserved, wrote inline or replayed")
			}
			if scenario == "full" {
				if w.criticalInstanceAdmissionErr != nil {
					t.Fatal("temporary saturation became a permanent rejection")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				w.flushInstanceStateIfDirty()
				awaitCriticalWrite(t, base.entered)
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if w.instanceStateDirty {
					t.Fatal("freed capacity did not persist the retained dirty state")
				}
			} else if w.criticalInstanceAdmissionErr == nil {
				t.Fatal("permanent rejection was not retained")
			}
		})
	}
}

// Account writes settle normally; only the instance write is held or fails.
type criticalDisconnectInstanceStore struct {
	*criticalInstanceTestStore
	instanceRelease <-chan struct{}
	instanceFailure error
	instancePanic   bool
}

func (s *criticalDisconnectInstanceStore) SaveInstanceState(state *model.InstanceStateSnapshot) error {
	writer := criticalTestStore{entered: s.entered, release: s.instanceRelease,
		failure: s.instanceFailure, panicAfterWrite: s.instancePanic}
	return writer.record(criticalWrite{instances: state})
}

func addCriticalWaterRoom(w *World, p *Player) *ItemInstance {
	room := &ItemInstance{RuntimeID: "water-test", Config: model.VolatileInstance{ID: "water-test"},
		MemberIDs: []uint16{p.ID}, LeaderID: p.ID, MobIDs: make(map[uint16]struct{})}
	if w.itemInstances == nil {
		w.itemInstances = make(map[string]*ItemInstance)
	}
	w.itemInstances[room.RuntimeID] = room
	w.setPlayerInstanceIndex(p.ID, room.RuntimeID)
	return room
}

func TestCriticalDisconnectInstanceWorldLoopContinuesDuringMembershipSave(t *testing.T) {
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 4)}
	backend := &criticalDisconnectInstanceStore{criticalInstanceTestStore: &criticalInstanceTestStore{base}, instanceRelease: gate}
	w, err := NewWorld(backend, nil, nil, model.Catalog{}, [model.MaxCarry]int{}, model.VolatileCatalog{},
		model.CharacterTemplateFile{}, model.TerrainMap{})
	if err != nil {
		t.Fatal(err)
	}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	p, _ := networkedTestPlayer(1, "Departing", 2100, 2100)
	unrelated, _ := networkedTestPlayer(2, "Unrelated", 2110, 2100)
	for _, player := range []*Player{p, unrelated} {
		w.players[player.Session], w.playersByID[player.ID] = player, player
		w.indexPlayerCharacter(player)
		w.updatePlayerSpatial(player)
		if !w.claimAccountSession(player.Session, player.Account.Name) {
			t.Fatal("fixture account admission failed")
		}
		player.NextCPRecovery = w.now().Add(time.Hour)
		player.NextRegen = w.now().Add(time.Hour)
	}
	setPlayerCurHP(unrelated.Char, 500)
	unrelated.NextRegen = w.now().Add(-time.Second)
	room := addCriticalWaterRoom(w, p)
	uid, unrelatedUID := p.Char.UID, unrelated.Char.UID
	// Autosave is a separate migration gate, not evidence supplied by this test.
	w.nextAutoSave = w.now().Add(time.Hour)
	ticks, done, stopped := make(chan time.Time), make(chan struct{}), make(chan struct{})
	go func() {
		defer close(stopped)
		w.run(ticks, done)
	}()
	var once sync.Once
	stop := func() {
		once.Do(func() { close(done) })
		select {
		case <-stopped:
		case <-time.After(2 * time.Second):
			t.Error("World loop did not stop")
		}
	}
	t.Cleanup(stop)
	select {
	case w.commands <- command{s: p.Session}:
	case <-time.After(time.Second):
		t.Fatal("World did not accept disconnect")
	}
	awaitCriticalWrite(t, base.entered)
	write := awaitCriticalWrite(t, base.entered)
	if write.instances == nil || len(write.instances.Instances) != 1 ||
		!reflect.DeepEqual(write.instances.Instances[0].MemberCharacterUIDs, []string{uid}) {
		t.Fatal("actual disconnect did not persist resumable membership")
	}
	for range 3 {
		deliverCriticalLoopTick(t, ticks)
		overview := readCriticalLoopOverview(t, w)
		regenerated, retained := false, false
		for _, player := range overview.Players {
			regenerated = regenerated || player.UID == unrelatedUID && player.HP > 500
			retained = retained || player.UID == uid
		}
		if overview.State != "running" || !regenerated || !retained || !p.Session.IsClosed() ||
			p.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("held instance save blocked ticks/read commands or released the departing owner")
		}
	}
	if len(base.entered) != 0 {
		t.Fatal("repeated ticks replayed the held write")
	}
	release()
	removed := false
	deadline := time.Now().Add(2 * time.Second)
	for time.Now().Before(deadline) {
		found := false
		for _, player := range readCriticalLoopOverview(t, w).Players {
			found = found || player.UID == uid
		}
		if !found {
			removed = true
			break
		}
	}
	stop()
	// Actor-owned maps are inspected only after the loop has joined.
	if !removed || w.players[p.Session] != nil || w.playersByCharacterUID[uid] != nil ||
		w.accountSessions[accountSessionKey(p.Account.Name)] != nil || len(queue.pending) != 0 ||
		len(w.criticalDisconnects) != 0 || w.instanceStateDirty || w.mobTickCounter < 3 || len(base.entered) != 0 {
		t.Fatal("actual loop failed to acknowledge membership and release disconnect ownership exactly once")
	}
	if _, ok := w.pendingInstanceMembers[room.RuntimeID][uid]; !ok {
		t.Fatal("durable resumable membership disappeared during cleanup")
	}
}

func TestCriticalDisconnectInstanceOwnershipHeldUntilCoveringAcknowledgement(t *testing.T) {
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 4)}
	backend := &criticalDisconnectInstanceStore{criticalInstanceTestStore: &criticalInstanceTestStore{base}, instanceRelease: gate}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w, p, fallback := criticalLifecycleTestWorld(t, queue)
	room := addCriticalWaterRoom(w, p)
	session, id, uid := p.Session, p.ID, p.Char.UID
	w.handle(command{s: session})
	awaitCriticalWrite(t, base.entered)
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	write := awaitCriticalWrite(t, base.entered)
	if write.instances == nil || len(write.instances.Instances) != 1 ||
		!reflect.DeepEqual(write.instances.Instances[0].MemberCharacterUIDs, []string{uid}) ||
		write.instances.Instances[0].LeaderCharacterUID != uid || len(room.MemberIDs) != 0 || room.LeaderID != 0 {
		t.Fatal("detached Water membership lost stable UID/leader identity")
	}
	for _, phase := range []string{"held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.handle(command{s: session})
		w.resumeCriticalDisconnects(8)
		if w.players[session] != p || w.playersByID[id] != p || w.playersByCharacterUID[uid] != p ||
			w.accountSessions["held"] != session || !w.criticalPlayerReserved(p) || !w.criticalInstanceReserved() ||
			len(base.entered) != 0 || fallback.saves != 0 || w.claimAccountSession(net.NewTestSession(2, 8), "held") {
			t.Fatalf("%s released ownership, repeated detachment/write or fell back inline", phase)
		}
	}
	w.processCriticalCompletions(1)
	if w.players[session] != nil || w.playersByID[id] != nil || w.playersByCharacterUID[uid] != nil ||
		len(w.criticalDisconnects) != 0 || w.instanceStateDirty || len(queue.pending) != 0 || fallback.saves != 0 ||
		!w.claimAccountSession(net.NewTestSession(3, 8), "held") {
		t.Fatal("covering acknowledgement did not release disconnect ownership")
	}
	if _, retained := w.pendingInstanceMembers[room.RuntimeID][uid]; !retained {
		t.Fatal("cleanup removed resumable membership after its durable acknowledgement")
	}
}

func TestCriticalDisconnectInstanceUnknownOutcomeDoesNotReplayOrOverwrite(t *testing.T) {
	for _, scenario := range []string{"error", "panic", "identity"} {
		t.Run(scenario, func(t *testing.T) {
			gate, release := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 4)}
			backend := &criticalDisconnectInstanceStore{criticalInstanceTestStore: &criticalInstanceTestStore{base}, instanceRelease: gate}
			if scenario == "error" {
				backend.instanceFailure = errors.New("instance commit acknowledgement lost")
			}
			backend.instancePanic = scenario == "panic"
			queue := newCriticalTestQueue(t, backend, 1, 2)
			t.Cleanup(release)
			w, p, fallback := criticalLifecycleTestWorld(t, queue)
			addCriticalWaterRoom(w, p)
			w.handle(command{s: p.Session})
			awaitCriticalWrite(t, base.entered)
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			awaitCriticalWrite(t, base.entered)
			if scenario == "identity" {
				p.criticalLifecycle = &criticalPlayerLifecycle{}
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			w.resumeCriticalDisconnects(8)
			w.flushInstanceStateIfDirty()
			done := make(chan error, 1)
			w.runShutdown(done)
			if err := <-done; err == nil || fallback.saves != 0 || fallback.flushes != 0 ||
				w.players[p.Session] != p || w.accountSessions["held"] != p.Session || !p.PersistencePoisoned ||
				!w.instanceStateDirty || !w.criticalInstanceReserved() || len(queue.pending) != 1 || len(base.entered) != 0 {
				t.Fatalf("unknown instance outcome replayed, overwrote or released identity: %v", err)
			}
		})
	}
}

type criticalInstanceShutdownCapture struct {
	atomicShutdownStore
	state *model.InstanceStateSnapshot
}

func (s *criticalInstanceShutdownCapture) SaveInstanceState(state *model.InstanceStateSnapshot) error {
	s.state = state
	return s.atomicShutdownStore.SaveInstanceState(state)
}

func TestCriticalDisconnectInstanceShutdownWaitsAndPersistsLaterGeneration(t *testing.T) {
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 4)}
	queue := newCriticalTestQueue(t, &criticalDisconnectInstanceStore{
		criticalInstanceTestStore: &criticalInstanceTestStore{base}, instanceRelease: gate}, 1, 2)
	t.Cleanup(release)
	w, p, fallback := criticalLifecycleTestWorld(t, queue)
	terminal := &criticalInstanceShutdownCapture{}
	w.store = terminal
	addCriticalWaterRoom(w, p)
	w.handle(command{s: p.Session})
	awaitCriticalWrite(t, base.entered)
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	awaitCriticalWrite(t, base.entered)
	w.gamblePool = 300
	w.markInstanceStateDirty()
	done := make(chan error, 1)
	w.runShutdown(done)
	select {
	case <-done:
		t.Fatal("shutdown bypassed the accepted instance transaction")
	default:
	}
	if terminal.playerStateSaves != 0 || terminal.instanceSaves != 0 || terminal.flushes != 0 {
		t.Fatal("terminal state overwrote a held instance transaction")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	select {
	case err := <-done:
		if err != nil {
			t.Fatal(err)
		}
	case <-time.After(time.Second):
		t.Fatal("shutdown did not finish after instance acknowledgement")
	}
	if terminal.playerStateSaves != 1 || terminal.instanceSaves != 1 || terminal.flushes != 1 ||
		terminal.state == nil || terminal.state.GamblePool != 300 ||
		w.instanceStateDirty || w.players[p.Session] != p || fallback.saves != 0 {
		t.Fatal("terminal shutdown lost later dirty state or removed retained identity")
	}
}

func TestCriticalDisconnectLaterMembershipWaitsForItsOwnSnapshot(t *testing.T) {
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 8)}
	backend := &criticalDisconnectInstanceStore{criticalInstanceTestStore: &criticalInstanceTestStore{base}, instanceRelease: gate}
	queue := newCriticalTestQueue(t, backend, 2, 2)
	t.Cleanup(release)
	w, first, fallback := criticalLifecycleTestWorld(t, queue)
	account := newShutdownAccount("second")
	account.Chars[0].UID = "second-character"
	second := addShutdownPlayer(w, 2, account)
	w.indexPlayerCharacter(second)
	if !w.claimAccountSession(second.Session, account.Name) {
		t.Fatal("second fixture account admission failed")
	}
	room := addCriticalWaterRoom(w, first)
	room.MemberIDs = append(room.MemberIDs, second.ID)
	w.setPlayerInstanceIndex(second.ID, room.RuntimeID)
	w.handle(command{s: first.Session})
	awaitCriticalWrite(t, base.entered)
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	awaitCriticalWrite(t, base.entered)
	w.handle(command{s: second.Session})
	accountWrite := awaitCriticalWrite(t, base.entered)
	if len(accountWrite.accounts) != 1 || accountWrite.accounts[0].Name != "second" {
		t.Fatal("unrelated account could not progress behind the instance write")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	// The second account committed, but membership retained by the first
	// in-flight instance snapshot cannot change until that write settles.
	if len(room.MemberIDs) != 1 || w.criticalDisconnects[second.Session].instancesDetached ||
		w.criticalDisconnects[second.Session].instancesSaved || len(base.entered) != 0 {
		t.Fatal("later membership changed behind the in-flight instance snapshot")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	covering := awaitCriticalWrite(t, base.entered)
	if covering.instances == nil || w.players[first.Session] != nil || w.players[second.Session] != second ||
		!w.criticalPlayerReserved(second) || fallback.saves != 0 {
		t.Fatal("older acknowledgement released a later disconnect or failed to submit its covering snapshot")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if w.players[second.Session] != nil || len(w.criticalDisconnects) != 0 || w.instanceStateDirty || len(queue.pending) != 0 {
		t.Fatal("later covering acknowledgement did not settle the second disconnect")
	}
}

func TestCriticalDisconnectUnsupportedInstanceAdmissionRetainsForShutdown(t *testing.T) {
	base := &criticalTestStore{entered: make(chan criticalWrite, 4)}
	queue := newCriticalTestQueue(t, base, 1, 1)
	w, p, fallback := criticalLifecycleTestWorld(t, queue)
	addCriticalWaterRoom(w, p)
	w.handle(command{s: p.Session})
	awaitCriticalWrite(t, base.entered)
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	for range 3 {
		w.resumeCriticalDisconnects(8)
		w.flushInstanceStateIfDirty()
	}
	if !errors.Is(w.criticalInstanceAdmissionErr, errCriticalPersistenceUnsupported) ||
		w.players[p.Session] != p || !w.instanceStateDirty || !w.criticalPlayerReserved(p) ||
		len(queue.pending) != 0 || len(base.entered) != 0 || fallback.saves != 0 || w.shuttingDown {
		t.Fatal("known admission rejection wrote, repeated, released ownership or claimed an unknown commit")
	}
	done := make(chan error, 1)
	w.runShutdown(done)
	if err := <-done; err != nil || fallback.saves != 1 || fallback.flushes != 1 || w.instanceStateDirty {
		t.Fatalf("known instance rejection lost the explicit terminal snapshot: %v", err)
	}
}
