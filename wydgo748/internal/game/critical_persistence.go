package game

import (
	"errors"
	"fmt"
	"maps"
	"slices"
	"sort"
	"sync"

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
	keys      []string
	uids      []string
	completed bool
}

// criticalPersistence is the staged S2 coordinator, not yet enabled by World.
// submit, poll, acknowledge and closeAdmission belong to one actor goroutine.
// Workers only access the store, channels and detached job snapshots. Account
// fences stay held until the owner has applied a completion and acknowledged it.
type criticalPersistence struct {
	store      store.Store
	capacity   int
	lastID     uint64
	stopping   bool
	jobs       chan criticalPersistenceJob
	results    chan criticalPersistenceResult
	stopped    chan struct{}
	pending    map[uint64]*criticalPersistencePending
	accounts   map[string]uint64
	characters map[string]uint64
	instances  uint64
	guilds     uint64
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
				q.results <- q.execute(job)
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
// Capacity includes completed work not yet applied by the owning actor.
func (q *criticalPersistence) submit(write criticalWrite) (uint64, error) {
	if q.stopping {
		return 0, errCriticalPersistenceStopping
	}
	if len(q.pending) >= q.capacity {
		return 0, errCriticalPersistenceFull
	}
	if (len(write.accounts) == 0 && write.instances == nil && write.guilds == nil) || len(write.accounts) > criticalPersistenceMaxAccounts {
		return 0, errors.New("critical persistence: invalid participant count")
	}
	combined := write.guilds != nil || write.instances != nil && write.character != nil
	if write.guilds != nil && q.guilds != 0 {
		return 0, errCriticalPersistenceBusy
	}
	if combined {
		if _, ok := q.store.(criticalGameStateStore); !ok {
			return 0, errCriticalPersistenceUnsupported
		}
	}
	if write.instances != nil {
		if q.instances != 0 {
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
		if _, reserved := q.accounts[key]; reserved {
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
		if _, reserved := q.characters[uid]; reserved {
			return 0, errCriticalPersistenceBusy
		}
		characterKeys = append(characterKeys, uid)
	}
	sort.Strings(characterKeys)
	// Cloning happens on the actor before publishing the job to any worker.
	snapshot := criticalWrite{accounts: accountPersistenceSnapshots(write.accounts...)}
	if write.guilds != nil {
		registry := *write.guilds
		registry.Guilds = slices.Clone(registry.Guilds)
		for i := range registry.Guilds {
			registry.Guilds[i].Members = slices.Clone(registry.Guilds[i].Members)
		}
		registry.Wars = registry.Wars.Clone()
		snapshot.guilds = &registry
	}
	if write.instances != nil {
		instances := *write.instances
		instances.NightmarePartyRuns = maps.Clone(instances.NightmarePartyRuns)
		instances.Instances = slices.Clone(instances.Instances)
		for i := range instances.Instances {
			instances.Instances[i].MemberCharacterUIDs = slices.Clone(instances.Instances[i].MemberCharacterUIDs)
		}
		snapshot.instances = &instances
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
	select {
	case q.jobs <- criticalPersistenceJob{id: id, write: snapshot}:
		q.lastID = id
		q.pending[id] = &criticalPersistencePending{keys: keys, uids: characterKeys}
		if write.instances != nil {
			q.instances = id
		}
		if write.guilds != nil {
			q.guilds = id
		}
		for _, key := range keys {
			q.accounts[key] = id
		}
		for _, uid := range characterKeys {
			q.characters[uid] = id
		}
		return id, nil
	default:
		return 0, errCriticalPersistenceFull
	}
}

// poll records delivery on the owner. Returning a result does not release its
// fence: the owner must first settle live state and publish any confirmation.
func (q *criticalPersistence) poll() (criticalPersistenceResult, bool) {
	select {
	case result := <-q.results:
		q.pending[result.id].completed = true
		return result, true
	default:
		return criticalPersistenceResult{}, false
	}
}

func (q *criticalPersistence) acknowledge(id uint64) error {
	pending := q.pending[id]
	if pending == nil || !pending.completed {
		return errors.New("critical persistence: completion not delivered or already acknowledged")
	}
	for _, key := range pending.keys {
		delete(q.accounts, key)
	}
	for _, uid := range pending.uids {
		delete(q.characters, uid)
	}
	if q.instances == id {
		q.instances = 0
	}
	if q.guilds == id {
		q.guilds = 0
	}
	delete(q.pending, id)
	return nil
}

// closeAdmission drains accepted jobs without requiring the owner to poll first:
// result capacity equals the total accepted-but-unacknowledged capacity. The
// stopped channel means workers exited, not that World applied all completions.
// A store call that hangs remains pending; callers must report drain timeout.
func (q *criticalPersistence) closeAdmission() {
	if !q.stopping {
		q.stopping = true
		close(q.jobs)
	}
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
	if write.guilds != nil || write.instances != nil && write.character != nil {
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
