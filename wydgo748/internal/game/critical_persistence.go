package game

import (
	"errors"
	"fmt"
	"log"
	"maps"
	"slices"
	"sort"
	"sync"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

// Bound detached participant snapshots independently of the pending-job limit.
const criticalPersistenceMaxAccounts = 64

var (
	errCriticalPersistenceBusy          = errors.New("critical persistence: account already reserved")
	errCriticalPersistenceFull          = errors.New("critical persistence: pending capacity exhausted")
	errCriticalPersistenceStopping      = errors.New("critical persistence: admission closed")
	errCriticalPersistenceUnsupported   = errors.New("critical persistence: atomic operation unsupported")
	errCriticalPersistenceIndeterminate = errors.New("critical persistence: transaction outcome indeterminate")
)

// criticalWrite describes detached durable state, not a gameplay continuation.
// A nonnil character means an atomic account/sidecar operation; nil state means
// deletion of that sidecar. Instances are a serialized global aggregate;
// account/instance writes require the matching atomic adapter. Guilds and
// combined aggregates require the certified game-state adapter.
type criticalWrite struct {
	accounts  []*model.Account
	character *criticalCharacterState
	instances *model.InstanceStateSnapshot
	guilds    *model.GuildRegistry
	// donate marks a premium balance write: one account, the credits it
	// applies and its ledger rows, committed in the same transaction.
	donate *criticalDonateWrite
}

type criticalDonateWrite struct {
	credits []int64
	ledger  []store.DonateLedgerEntry
}

// criticalDonateStore is the certified premium balance adapter (PostgreSQL).
type criticalDonateStore interface {
	SaveDonateAccount(*model.Account, []int64, []store.DonateLedgerEntry) error
}

type criticalCharacterState struct {
	uid   string
	state *model.CharState
}

type criticalPersistenceResult struct {
	id  uint64
	err error
}

type criticalPersistenceJob struct {
	id    uint64
	write criticalWrite
}

// Certified adapters are optional while legacy stores/producers remain staged.
// PostgreSQL provides this account-only path without changing legacy callers.
type criticalAccountStore interface {
	SaveCriticalAccounts(...*model.Account) error
}

type criticalInstanceStore interface {
	SaveCriticalInstanceState(*model.InstanceStateSnapshot, ...*model.Account) error
}

// An empty UID omits the sidecar; a nonempty UID with nil state deletes it.
type criticalGameStateStore interface {
	SaveCriticalGameState(*model.GuildRegistry, *model.InstanceStateSnapshot, string, *model.CharState, ...*model.Account) error
}

type criticalPersistencePending struct {
	keys        []string
	uids        []string
	completed   bool
	instances   bool
	guilds      bool
	quarantined bool
	// submittedAt is real time for diagnostics, never a gameplay deadline.
	submittedAt time.Time
	// background marks a detached autosave snapshot (W7). It holds no gameplay
	// input: a later write of its participants is parked behind it instead.
	background bool
	successors []uint64
	// A parked job goes to a worker only after every predecessor it waits for
	// is acknowledged, so an older snapshot can never commit after it.
	parked *criticalPersistenceJob
	waits  int
	// waited records that this job parked; its predecessor may still be
	// pending, unacknowledged, when this job settles.
	waited bool
}

// criticalPersistence coordinates detached writes for the World actor.
// submit, poll, acknowledge and closeAdmission belong to one actor goroutine.
// Workers only access the store, channels and detached job snapshots. Account
// fences stay held until the owner has applied a completion and acknowledged it.
type criticalPersistence struct {
	store      store.Store
	capacity   int
	lastID     uint64
	stopping   bool
	jobsClosed bool
	// parked counts accepted jobs waiting for a background predecessor. The
	// job channel stays open while any remain, even after admission closes.
	parked     int
	jobs       chan criticalPersistenceJob
	results    chan criticalPersistenceResult
	stopped    chan struct{}
	pending    map[uint64]*criticalPersistencePending
	accounts   map[string]uint64
	characters map[string]uint64
	instances  uint64
	guilds     uint64
	// Failed operations retain ownership, but no longer occupy worker capacity.
	// Separate fences also cover failures after publication released admission.
	quarantined      int
	failedAccounts   map[string]uint64
	failedCharacters map[string]uint64
	failedInstances  uint64
	failedGuilds     uint64
}

func newCriticalPersistence(backend store.Store, workers, capacity int) (*criticalPersistence, error) {
	if backend == nil || workers < 1 || capacity < 1 || workers > capacity {
		return nil, errors.New("critical persistence: invalid store, worker count or capacity")
	}
	q := &criticalPersistence{
		store: backend, capacity: capacity,
		jobs:       make(chan criticalPersistenceJob, capacity),
		results:    make(chan criticalPersistenceResult, capacity),
		stopped:    make(chan struct{}),
		pending:    make(map[uint64]*criticalPersistencePending),
		accounts:   make(map[string]uint64),
		characters: make(map[string]uint64),
	}
	var running sync.WaitGroup
	running.Add(workers)
	for range workers {
		go func() {
			defer running.Done()
			for job := range q.jobs {
				metricCriticalRunning.Add(1)
				started := time.Now()
				result := q.execute(job)
				observeCriticalOutcome(result.err, time.Since(started))
				metricCriticalRunning.Add(-1)
				q.results <- result
			}
		}()
	}
	go func() {
		running.Wait()
		close(q.stopped)
	}()
	return q, nil
}

// submit is nonblocking. Rejections retain no fence and cause no store writes.
// Capacity includes completed work awaiting the actor, but excludes quarantine.
// A participant held only by a background snapshot does not refuse the write:
// the job is parked and starts after that snapshot is acknowledged.
func (q *criticalPersistence) submit(write criticalWrite) (uint64, error) {
	return q.submitOrdered(write, false)
}

// submitBackground accepts one account snapshot only when none of its
// participants has pending work. It never waits behind another write.
func (q *criticalPersistence) submitBackground(write criticalWrite) (uint64, error) {
	if len(write.accounts) != 1 || write.instances != nil || write.guilds != nil || write.donate != nil {
		return 0, errors.New("critical persistence: a background snapshot has exactly one account")
	}
	return q.submitOrdered(write, true)
}

// backgroundHolder reports whether a later write may park behind id.
func (q *criticalPersistence) backgroundHolder(id uint64) bool {
	pending := q.pending[id]
	return pending != nil && pending.background && !pending.quarantined
}

func (q *criticalPersistence) submitOrdered(write criticalWrite, background bool) (uint64, error) {
	if q.stopping {
		return 0, errCriticalPersistenceStopping
	}
	if len(q.pending)-q.quarantined >= q.capacity {
		return 0, errCriticalPersistenceFull
	}
	if (len(write.accounts) == 0 && write.instances == nil && write.guilds == nil) || len(write.accounts) > criticalPersistenceMaxAccounts {
		return 0, errors.New("critical persistence: invalid participant count")
	}
	var predecessors map[uint64]bool
	waitFor := func(holder uint64) bool {
		if background || !q.backgroundHolder(holder) {
			return false
		}
		if predecessors == nil {
			predecessors = make(map[uint64]bool, 1)
		}
		predecessors[holder] = true
		return true
	}
	if write.donate != nil {
		if len(write.accounts) != 1 || write.character != nil || write.instances != nil || write.guilds != nil {
			return 0, errors.New("critical persistence: a donate write has exactly one account")
		}
		if _, ok := q.store.(criticalDonateStore); !ok {
			return 0, errCriticalPersistenceUnsupported
		}
	}
	combined := write.guilds != nil || write.instances != nil && write.character != nil
	if write.guilds != nil && (q.guilds != 0 || q.failedGuilds != 0) {
		return 0, errCriticalPersistenceBusy
	}
	if combined {
		if _, ok := q.store.(criticalGameStateStore); !ok {
			return 0, errCriticalPersistenceUnsupported
		}
	}
	if write.instances != nil {
		if q.instances != 0 || q.failedInstances != 0 {
			return 0, errCriticalPersistenceBusy
		}
		if combined {
			// The combined adapter covers the instance aggregate too.
		} else if _, certified := q.store.(criticalInstanceStore); certified {
			// One certified adapter covers both aggregate-only and atomic forms.
		} else if len(write.accounts) == 0 {
			if _, ok := q.store.(instanceRuntimeStore); !ok {
				return 0, errCriticalPersistenceUnsupported
			}
		} else if _, ok := q.store.(instanceRuntimeTransactionStore); !ok {
			return 0, errCriticalPersistenceUnsupported
		}
	}
	keys := make([]string, 0, len(write.accounts))
	seen := make(map[string]bool, len(write.accounts))
	for _, account := range write.accounts {
		if account == nil || accountSessionKey(account.Name) == "" {
			return 0, errors.New("critical persistence: missing account identity")
		}
		key := accountSessionKey(account.Name)
		if seen[key] {
			return 0, errors.New("critical persistence: duplicate account identity")
		}
		seen[key] = true
		if holder := q.accounts[key]; q.failedAccounts[key] != 0 || holder != 0 && !waitFor(holder) {
			return 0, errCriticalPersistenceBusy
		}
		keys = append(keys, key)
	}
	if write.character != nil {
		if len(write.accounts) == 0 || write.character.uid == "" {
			return 0, errors.New("critical persistence: sidecar requires an account and an explicit UID")
		}
		if combined {
			// The combined adapter covers the sidecar and all participants.
		} else if len(write.accounts) > 1 {
			if _, ok := q.store.(playerStateBatchStore); !ok {
				return 0, errCriticalPersistenceUnsupported
			}
		} else if _, batch := q.store.(playerStateBatchStore); !batch {
			if _, ok := q.store.(playerStateStore); !ok {
				return 0, errCriticalPersistenceUnsupported
			}
		}
	} else if !combined && write.instances == nil && len(write.accounts) > 1 {
		if _, certified := q.store.(criticalAccountStore); !certified {
			if _, ok := q.store.(tradeBatchStore); !ok {
				return 0, errCriticalPersistenceUnsupported
			}
		}
	}
	if q.lastID == ^uint64(0) {
		return 0, errors.New("critical persistence: operation ID exhausted")
	}
	// Reserve every character in the account aggregate and an explicit removed
	// sidecar UID. Standalone sidecar saves must not bypass an account fence.
	uids := make(map[string]string)
	for _, account := range write.accounts {
		for _, character := range account.Chars {
			if character.UID != "" {
				key := accountSessionKey(account.Name)
				if previous, exists := uids[character.UID]; exists && previous != key {
					return 0, errors.New("critical persistence: character UID shared by distinct accounts")
				}
				uids[character.UID] = key
			}
		}
	}
	if write.character != nil {
		key := accountSessionKey(write.accounts[0].Name)
		if previous, exists := uids[write.character.uid]; exists && previous != key {
			return 0, errors.New("critical persistence: sidecar belongs to a different participant")
		}
		uids[write.character.uid] = key
	}
	characterKeys := make([]string, 0, len(uids))
	for uid := range uids {
		if holder := q.characters[uid]; q.failedCharacters[uid] != 0 || holder != 0 && !waitFor(holder) {
			return 0, errCriticalPersistenceBusy
		}
		characterKeys = append(characterKeys, uid)
	}
	sort.Strings(characterKeys)
	// Cloning happens on the actor before publishing the job to any worker.
	snapshot := criticalWrite{accounts: accountPersistenceSnapshots(write.accounts...)}
	if write.donate != nil {
		snapshot.donate = &criticalDonateWrite{
			credits: slices.Clone(write.donate.credits), ledger: slices.Clone(write.donate.ledger),
		}
	}
	if write.guilds != nil {
		snapshot.guilds = write.guilds.Clone()
	}
	if write.instances != nil {
		snapshot.instances = write.instances.Clone()
	}
	if write.character != nil {
		character := *write.character
		if character.state != nil {
			state := *character.state
			state.Affects = slices.Clone(state.Affects)
			state.SpecialCoins = maps.Clone(state.SpecialCoins)
			character.state = &state
		}
		snapshot.character = &character
	}
	sort.Strings(keys)
	id := q.lastID + 1
	job := criticalPersistenceJob{id: id, write: snapshot}
	pending := &criticalPersistencePending{keys: keys, uids: characterKeys, instances: write.instances != nil,
		guilds: write.guilds != nil, submittedAt: time.Now(), background: background}
	if len(predecessors) > 0 {
		pending.parked, pending.waits, pending.waited = &job, len(predecessors), true
		for holder := range predecessors {
			q.pending[holder].successors = append(q.pending[holder].successors, id)
		}
		q.parked++
		metricCriticalParked.Add(1)
	} else {
		select {
		case q.jobs <- job:
		default:
			return 0, errCriticalPersistenceFull
		}
	}
	q.lastID = id
	q.pending[id] = pending
	if write.instances != nil {
		q.instances = id
	}
	if write.guilds != nil {
		q.guilds = id
	}
	// A parked write takes over each key from its background predecessor.
	for _, key := range keys {
		q.accounts[key] = id
	}
	for _, uid := range characterKeys {
		q.characters[uid] = id
	}
	return id, nil
}

// returnKey releases key from id, or hands it back to a predecessor of a
// parked job that is still pending. Only a parked job scans the pending set.
func (q *criticalPersistence) returnKey(holders map[string]uint64, pending *criticalPersistencePending, id uint64, key string, character bool) {
	if pending.waited {
		if other := q.pendingHolding(id, key, character, false); other != 0 {
			holders[key] = other
			return
		}
	}
	delete(holders, key)
}

// pendingHolding returns another pending operation that covers key. The
// pending set is bounded by capacity plus retained quarantine.
func (q *criticalPersistence) pendingHolding(id uint64, key string, character, quarantinedOnly bool) uint64 {
	for other, pending := range q.pending {
		if other == id || quarantinedOnly && !pending.quarantined {
			continue
		}
		keys := pending.keys
		if character {
			keys = pending.uids
		}
		if slices.Contains(keys, key) {
			return other
		}
	}
	return 0
}

// releaseSuccessors starts parked jobs whose last predecessor was just
// acknowledged. Accepted work holds a slot, so the job channel has room.
func (q *criticalPersistence) releaseSuccessors(pending *criticalPersistencePending) {
	successors := pending.successors
	pending.successors = nil
	for _, id := range successors {
		next := q.pending[id]
		if next == nil || next.parked == nil {
			continue
		}
		if next.waits--; next.waits > 0 {
			continue
		}
		job := *next.parked
		next.parked = nil
		q.parked--
		select {
		case q.jobs <- job:
		default:
			q.deliverUnsent(id, "worker queue full")
		}
	}
	q.closeJobsIfDrained()
}

// rejectSuccessors refuses every job parked behind a predecessor that failed.
// That predecessor may still commit later, so no newer write may follow it.
func (q *criticalPersistence) rejectSuccessors(id uint64) {
	pending := q.pending[id]
	if pending == nil {
		return
	}
	successors := pending.successors
	pending.successors = nil
	for _, next := range successors {
		parked := q.pending[next]
		if parked == nil || parked.parked == nil {
			continue
		}
		parked.parked = nil
		q.parked--
		q.deliverUnsent(next, fmt.Sprintf("background write %d did not settle", id))
	}
	q.closeJobsIfDrained()
}

// deliverUnsent completes a parked job that no adapter received. The result
// channel holds one slot per accepted job, so the send cannot block.
func (q *criticalPersistence) deliverUnsent(id uint64, reason string) {
	err := store.RejectUnsentWrite(reason)
	observeCriticalOutcome(err, 0)
	select {
	case q.results <- criticalPersistenceResult{id: id, err: err}:
	default:
		log.Printf("CRITICAL: persistence result channel full; parked write %d stays pending", id)
	}
}

func (q *criticalPersistence) closeJobsIfDrained() {
	if q.stopping && !q.jobsClosed && q.parked == 0 {
		q.jobsClosed = true
		close(q.jobs)
	}
}

// poll records delivery on the owner. Returning a result does not release its
// fence: the owner must first settle live state and publish any confirmation.
func (q *criticalPersistence) poll() (criticalPersistenceResult, bool) {
	select {
	case result := <-q.results:
		if pending := q.pending[result.id]; pending != nil {
			pending.completed = true
		}
		return result, true
	default:
		return criticalPersistenceResult{}, false
	}
}

func (q *criticalPersistence) acknowledge(id uint64) error {
	pending := q.pending[id]
	if pending == nil || !pending.completed || pending.quarantined {
		return errors.New("critical persistence: completion not delivered or already acknowledged")
	}
	// A parked successor may already own a key; release only this job's keys.
	// A failed predecessor still pending keeps the keys this job took over.
	for _, key := range pending.keys {
		if q.accounts[key] == id {
			q.returnKey(q.accounts, pending, id, key, false)
		}
	}
	for _, uid := range pending.uids {
		if q.characters[uid] == id {
			q.returnKey(q.characters, pending, id, uid, true)
		}
	}
	if q.instances == id {
		q.instances = 0
	}
	if q.guilds == id {
		q.guilds = 0
	}
	delete(q.pending, id)
	q.releaseSuccessors(pending)
	return nil
}

// closeAdmission drains accepted jobs without requiring the owner to poll first:
// result capacity equals the total accepted-but-unacknowledged capacity. The
// stopped channel means workers exited, not that World applied all completions.
// A store call that hangs remains pending; callers must report drain timeout.
// Parked jobs keep the job channel open until they start or are refused.
func (q *criticalPersistence) closeAdmission() {
	q.stopping = true
	q.closeJobsIfDrained()
}

func (q *criticalPersistence) execute(job criticalPersistenceJob) (result criticalPersistenceResult) {
	result.id = job.id
	defer func() {
		if failure := recover(); failure != nil {
			// A panic may follow a durable commit. Never describe it as a definite
			// rejection; integration must fence and reconcile these accounts.
			result.err = fmt.Errorf("%w: store panic: %v", errCriticalPersistenceIndeterminate, failure)
		}
		// Only an adapter-certified rejection proves that no commit occurred.
		// Every unclassified error still requires fencing and reconciliation.
		if result.err != nil && !store.IsWriteRejected(result.err) &&
			!errors.Is(result.err, errCriticalPersistenceIndeterminate) {
			result.err = fmt.Errorf("%w: %w", errCriticalPersistenceIndeterminate, result.err)
		}
	}()
	write := job.write
	if write.donate != nil {
		result.err = q.store.(criticalDonateStore).SaveDonateAccount(write.accounts[0],
			write.donate.credits, write.donate.ledger)
	} else if write.guilds != nil || write.instances != nil && write.character != nil {
		var uid string
		var state *model.CharState
		if write.character != nil {
			uid, state = write.character.uid, write.character.state
		}
		result.err = q.store.(criticalGameStateStore).SaveCriticalGameState(write.guilds,
			write.instances, uid, state, write.accounts...)
	} else if write.instances != nil {
		if certified, ok := q.store.(criticalInstanceStore); ok {
			result.err = certified.SaveCriticalInstanceState(write.instances, write.accounts...)
		} else if len(write.accounts) == 0 {
			result.err = q.store.(instanceRuntimeStore).SaveInstanceState(write.instances)
		} else {
			result.err = q.store.(instanceRuntimeTransactionStore).SaveGameStateWithInstanceState(nil,
				write.instances, write.accounts...)
		}
	} else if write.character != nil {
		// Prefer the certified batch adapter even for a single participant.
		// The single-account legacy adapter does not certify transaction errors.
		if batch, ok := q.store.(playerStateBatchStore); ok {
			result.err = batch.SavePlayerStateBatch(nil,
				write.character.uid, write.character.state, write.accounts...)
		} else {
			result.err = q.store.(playerStateStore).SavePlayerState(nil, write.accounts[0],
				write.character.uid, write.character.state)
		}
	} else if certified, ok := q.store.(criticalAccountStore); ok {
		result.err = certified.SaveCriticalAccounts(write.accounts...)
	} else if len(write.accounts) > 1 {
		result.err = q.store.(tradeBatchStore).SaveAccounts(write.accounts...)
	} else {
		result.err = q.store.SaveAccount(write.accounts[0])
	}
	return result
}
