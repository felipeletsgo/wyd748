package store

import (
	"bytes"
	"context"
	_ "embed"
	"encoding/hex"
	"encoding/json"
	"errors"
	"expvar"
	"fmt"
	"io"
	"log"
	"os"
	"strings"
	"sync"
	"sync/atomic"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgconn"
	"github.com/jackc/pgx/v5/pgtype"
	"github.com/jackc/pgx/v5/pgxpool"

	"wydgo/internal/model"
)

//go:embed postgres_schema.sql
var postgresSchema string

//go:embed postgres_progression.sql
var postgresProgressionSchema string

//go:embed postgres_guild_public.sql
var postgresGuildPublicSchema string

//go:embed postgres_staff_ops.sql
var postgresStaffOpsSchema string

//go:embed postgres_account_history.sql
var postgresAccountHistorySchema string

//go:embed postgres_account_notes.sql
var postgresAccountNotesSchema string

//go:embed postgres_npc_definitions.sql
var postgresNPCDefinitionsSchema string

//go:embed postgres_npc_exact_names.sql
var postgresNPCExactNamesSchema string

const postgresWriteQueueSize = 2048
const postgresSchemaVersion = 14
const postgresOperationTimeout = 10 * time.Second
const postgresStartupTimeout = 30 * time.Second
const postgresAutosaveBatchSize = 64
const postgresCharStateBatchSize = 128
const postgresAutosaveCollectWindow = 2 * time.Millisecond

var (
	metricPostgresQueueDepth     = expvar.NewInt("postgres_async_queue_depth")
	metricPostgresPending        = expvar.NewInt("postgres_async_pending_coalesced")
	metricPostgresCoalesced      = expvar.NewInt("postgres_async_snapshots_coalesced_total")
	metricPostgresFailures       = expvar.NewInt("postgres_async_failures_total")
	metricPostgresTxDuration     = expvar.NewInt("postgres_transaction_duration_micros")
	metricPostgresTxDurationMax  = expvar.NewInt("postgres_transaction_duration_max_micros")
	metricPostgresTxRetries      = expvar.NewInt("postgres_transaction_retries_total")
	metricPostgresFenceWaitTotal = expvar.NewInt("postgres_fence_wait_total_micros")
	metricPostgresFenceWaitMax   = expvar.NewInt("postgres_fence_wait_max_micros")
	metricPostgresFenceWaitCount = expvar.NewInt("postgres_fence_wait_count")
	postgresMetricMaxMu          sync.Mutex
)

// Private value of this project. The session lock stops the server and the
// account API from applying DDL at the same time on first boot.
const postgresSchemaAdvisoryLock int64 = 0x575944474F

// PostgresConfig configures persistence only. In production the URL must come
// from an environment variable so the password stays out of the repository and
// the process command line.
type PostgresConfig struct {
	URL              string
	MaxConns         int32
	GuildsTxtPath    string
	OperationTimeout time.Duration
}

type postgresItem struct {
	uid           string
	dbUID         pgtype.UUID
	location      string
	index         uint16
	effects       [6]byte
	activatedUnix int64
	expiresUnix   int64
}

type postgresCharacter struct {
	uid       string
	dbUID     pgtype.UUID
	slot      int
	nameKey   string
	evolution string
	mortalUID *pgtype.UUID
}

// accountSnapshot is immutable once created. This lets autosave leave the game
// loop without observing pointers the World keeps changing.
type accountSnapshot struct {
	key        string
	display    string
	payload    []byte
	characters []postgresCharacter
	items      []postgresItem
}

type postgresCharStateSnapshot struct {
	uid     string
	payload []byte
}

type postgresWriteJob struct {
	label string
	// seq keeps the logical order between the FIFO queue and the coalesced
	// overflow. Without this generation an old snapshot that fell into the
	// overflow could drain after a newer snapshot or a critical barrier.
	seq uint64
	// account marks an account autosave and lets the worker merge several
	// snapshots into one transaction. run is for charstate and other jobs.
	account   *accountSnapshot
	charState *postgresCharStateSnapshot
	run       func() error
	done      chan struct{}
	retry     uint8
}

// PostgresStore is the authoritative production persistence. The account lives
// in JSONB, but names and item instances are also written to constrained
// tables. The JSONB is never committed without the anti-dupe indexes.
type PostgresStore struct {
	pool             *pgxpool.Pool
	guildsTxtPath    string
	writeQueue       chan postgresWriteJob
	mu               sync.RWMutex
	accountFences    postgresFenceSet
	stateFences      postgresFenceSet
	overflowMu       sync.Mutex
	overflowAcc      map[string]postgresWriteJob
	overflowState    map[string]postgresWriteJob
	overflowRuns     map[string]postgresWriteJob
	orderMu          sync.Mutex
	latestAccountSeq map[string]uint64
	latestStateSeq   map[string]uint64
	writeSeq         atomic.Uint64
	coalesced        atomic.Uint64
	guildExportGen   atomic.Uint64
	closeOnce        sync.Once
	closed           bool
	readOnly         bool
	operationTimeout time.Duration
}

// PersistentPlayerSnapshot is the narrow read model exposed to the staff web
// process. It intentionally contains no password hash, inventory or cargo.
type PersistentPlayerSnapshot struct {
	Account       string
	UID           string
	Name          string
	Class         byte
	Level         uint32
	Evolution     string
	HP            uint32
	MaxHP         uint32
	MP            uint32
	MaxMP         uint32
	Attack        uint32
	MagicAttack   uint32
	Defense       uint32
	Str           uint32
	Int           uint32
	Dex           uint32
	Con           uint32
	StatusPoints  uint32
	MasteryPoints uint32
	SkillPoints   uint32
	X             uint16
	Y             uint16
	Gold          uint32
	Exp           uint32
	Hold          uint32
	CP            int16
}

// AccountDirectorySnapshot is the narrow account projection used by the
// administrative directory. Credentials and item payloads never leave SQL.
type AccountDirectorySnapshot struct {
	Key        string
	Name       string
	CreatedAt  time.Time
	UpdatedAt  time.Time
	Characters []AccountDirectoryCharacterSnapshot
}

type AccountDirectoryCharacterSnapshot struct {
	UID       string
	Slot      int
	Name      string
	Class     byte
	Level     uint32
	Evolution string
}

type AccountDirectoryPageSnapshot struct {
	Accounts   []AccountDirectorySnapshot
	NextCursor string
}

// Ping exposes only the light check needed by HTTP readiness. The caller fully
// controls deadline/cancellation through the given context.
func (s *PostgresStore) Ping(ctx context.Context) error {
	if s == nil || s.pool == nil {
		return fmt.Errorf("postgres store closed")
	}
	return s.pool.Ping(ctx)
}

func NewPostgresStore(ctx context.Context, cfg PostgresConfig) (*PostgresStore, error) {
	pool, err := openPostgresPool(ctx, cfg)
	if err != nil {
		return nil, err
	}
	startupContext, cancelStartup := context.WithTimeout(ctx, postgresStartupTimeout)
	defer cancelStartup()
	if err := applyPostgresSchema(startupContext, pool); err != nil {
		pool.Close()
		return nil, fmt.Errorf("store: apply PostgreSQL schema: %w", err)
	}
	s := newPostgresStore(pool, cfg)
	go s.persistLoop()
	return s, nil
}

// NewPostgresReadOnlyStore opens only the read path used to import a template
// account from another database. It applies no migrations, starts no
// persistence worker and cannot lock or change the source database.
func NewPostgresReadOnlyStore(ctx context.Context, cfg PostgresConfig) (*PostgresStore, error) {
	pool, err := openPostgresPool(ctx, cfg)
	if err != nil {
		return nil, err
	}
	return &PostgresStore{pool: pool, readOnly: true, operationTimeout: operationTimeout(cfg)}, nil
}

func openPostgresPool(ctx context.Context, cfg PostgresConfig) (*pgxpool.Pool, error) {
	if strings.TrimSpace(cfg.URL) == "" {
		return nil, errors.New("store: database URL is missing")
	}
	poolConfig, err := pgxpool.ParseConfig(cfg.URL)
	if err != nil {
		return nil, fmt.Errorf("store: parse database URL: %w", err)
	}
	if cfg.MaxConns > 0 {
		poolConfig.MaxConns = cfg.MaxConns
	}
	if poolConfig.ConnConfig.ConnectTimeout == 0 {
		poolConfig.ConnConfig.ConnectTimeout = postgresOperationTimeout
	}
	startupContext, cancelStartup := context.WithTimeout(ctx, postgresStartupTimeout)
	defer cancelStartup()
	pool, err := pgxpool.NewWithConfig(startupContext, poolConfig)
	if err != nil {
		return nil, fmt.Errorf("store: create PostgreSQL pool: %w", err)
	}
	if err := pool.Ping(startupContext); err != nil {
		pool.Close()
		return nil, fmt.Errorf("store: connect to PostgreSQL: %w", err)
	}
	return pool, nil
}

func newPostgresStore(pool *pgxpool.Pool, cfg PostgresConfig) *PostgresStore {
	return &PostgresStore{
		pool:             pool,
		guildsTxtPath:    cfg.GuildsTxtPath,
		writeQueue:       make(chan postgresWriteJob, postgresWriteQueueSize),
		overflowAcc:      make(map[string]postgresWriteJob),
		overflowState:    make(map[string]postgresWriteJob),
		overflowRuns:     make(map[string]postgresWriteJob),
		operationTimeout: operationTimeout(cfg),
	}
}

func operationTimeout(cfg PostgresConfig) time.Duration {
	if cfg.OperationTimeout > 0 {
		return cfg.OperationTimeout
	}
	return postgresOperationTimeout
}

func applyPostgresSchema(ctx context.Context, pool *pgxpool.Pool) error {
	conn, err := pool.Acquire(ctx)
	if err != nil {
		return err
	}
	defer conn.Release()
	if _, err := conn.Exec(ctx, `SELECT pg_advisory_lock($1)`, postgresSchemaAdvisoryLock); err != nil {
		return err
	}
	defer func() {
		_, _ = conn.Exec(context.Background(),
			`SELECT pg_advisory_unlock($1)`, postgresSchemaAdvisoryLock)
	}()
	// The embedded schema includes the initial migration of the JSON payloads.
	// Once done, running it again on every boot only rescans every account and
	// can delay the server by tens of seconds on a load-test database. Check the
	// version under the same advisory lock and run the full DDL only when the
	// table is missing or outdated.
	var currentVersion int
	if err := conn.QueryRow(ctx,
		`SELECT COALESCE(MAX(version),0) FROM schema_migrations`).Scan(&currentVersion); err == nil {
		if currentVersion == postgresSchemaVersion {
			return nil
		}
		if currentVersion > postgresSchemaVersion {
			return fmt.Errorf("schema v%d; this binary requires v%d", currentVersion, postgresSchemaVersion)
		}
	}
	// Upgrading v6+ adds read views, staff tables and their markers. Do not replay the
	// historical account migration or rewrite any live character snapshot.
	if currentVersion < 6 {
		if _, err := conn.Exec(ctx, postgresSchema); err != nil {
			return err
		}
	}
	if currentVersion < 7 {
		if _, err := conn.Exec(ctx, postgresProgressionSchema); err != nil {
			return err
		}
	}
	if _, err := conn.Exec(ctx, postgresGuildPublicSchema); err != nil {
		return err
	}
	if currentVersion < 9 {
		if _, err := conn.Exec(ctx, postgresStaffOpsSchema); err != nil {
			return err
		}
	}
	if currentVersion < 10 {
		if _, err := conn.Exec(ctx, postgresAccountHistorySchema); err != nil {
			return err
		}
	}
	if currentVersion < 11 {
		if _, err := conn.Exec(ctx, postgresAccountNotesSchema); err != nil {
			return err
		}
	}
	if currentVersion < 13 {
		if _, err := conn.Exec(ctx, postgresNPCDefinitionsSchema); err != nil {
			return err
		}
	}
	if currentVersion < 14 {
		if _, err := conn.Exec(ctx, postgresNPCExactNamesSchema); err != nil {
			return err
		}
	}
	var version int
	if err := conn.QueryRow(ctx, `SELECT COALESCE(MAX(version),0) FROM schema_migrations`).Scan(&version); err != nil {
		return err
	}
	if version != postgresSchemaVersion {
		return fmt.Errorf("schema v%d; this binary requires v%d", version, postgresSchemaVersion)
	}
	return nil
}

func (s *PostgresStore) persistLoop() {
	var pending *postgresWriteJob
	for {
		var job postgresWriteJob
		var ok bool
		if pending != nil {
			// The batch may have taken a barrier from the queue while older
			// snapshots were still in the overflow. Drain only the earlier jobs
			// before confirming the barrier/pending job.
			if overflow, exists := s.takeOverflowBefore(pending.seq); exists {
				job, ok = overflow, true
			} else {
				job, pending = *pending, nil
				ok = true
			}
		} else if overflow, exists := s.takeOverflow(); exists {
			job, ok = overflow, true
		} else {
			job, ok = <-s.writeQueue
		}
		if !ok {
			return
		}
		metricPostgresQueueDepth.Set(int64(len(s.writeQueue)))
		if job.account != nil {
			latest := map[string]postgresWriteJob{job.account.key: job}
			timer := time.NewTimer(postgresAutosaveCollectWindow)
			collecting := true
			for collecting && len(latest) < postgresAutosaveBatchSize {
				select {
				case next, open := <-s.writeQueue:
					if !open {
						collecting = false
						break
					}
					if next.account == nil {
						pending = &next
						collecting = false
						break
					}
					// If the database fell behind and there are two autosaves
					// of the same account, only the newest snapshot is written.
					if current, exists := latest[next.account.key]; !exists || next.seq > current.seq {
						latest[next.account.key] = next
					}
				case <-timer.C:
					collecting = false
				}
			}
			if !timer.Stop() {
				select {
				case <-timer.C:
				default:
				}
			}
			keys := make([]string, 0, len(latest))
			for key := range latest {
				keys = append(keys, key)
			}
			unlockFences := s.accountFences.lock(keys...)
			snapshots := make([]*accountSnapshot, 0, len(latest))
			for _, candidate := range latest {
				if s.isLatestAccountJob(candidate) {
					snapshots = append(snapshots, candidate.account)
				}
			}
			if len(snapshots) == 0 {
				unlockFences()
				continue
			}
			err := s.saveSnapshots(snapshots)
			unlockFences()
			if err != nil {
				metricPostgresFailures.Add(1)
				log.Printf("store PostgreSQL: autosave batch (%d account(s)): %v",
					len(snapshots), err)
			}
			continue
		}
		if job.charState != nil {
			latest := map[string]postgresWriteJob{
				job.charState.uid: job,
			}
			timer := time.NewTimer(postgresAutosaveCollectWindow)
			collecting := true
			for collecting && len(latest) < postgresCharStateBatchSize {
				select {
				case next, open := <-s.writeQueue:
					if !open {
						collecting = false
						break
					}
					if next.charState == nil {
						pending = &next
						collecting = false
						break
					}
					if current, exists := latest[next.charState.uid]; !exists || next.seq > current.seq {
						latest[next.charState.uid] = next
					}
				case <-timer.C:
					collecting = false
				}
			}
			if !timer.Stop() {
				select {
				case <-timer.C:
				default:
				}
			}
			keys := make([]string, 0, len(latest))
			for key := range latest {
				keys = append(keys, key)
			}
			unlockFences := s.stateFences.lock(keys...)
			snapshots := make([]*postgresCharStateSnapshot, 0, len(latest))
			for _, candidate := range latest {
				if s.isLatestCharStateJob(candidate) {
					snapshots = append(snapshots, candidate.charState)
				}
			}
			if len(snapshots) == 0 {
				unlockFences()
				continue
			}
			err := s.saveCharStateSnapshots(snapshots)
			unlockFences()
			if err != nil {
				metricPostgresFailures.Add(1)
				log.Printf("store PostgreSQL: charstate batch (%d character(s)): %v",
					len(snapshots), err)
			}
			continue
		}
		if job.run != nil {
			if err := job.run(); err != nil {
				metricPostgresFailures.Add(1)
				log.Printf("store PostgreSQL: %s: %v", job.label, err)
				s.retryDerived(job)
			}
		}
		if job.done != nil {
			close(job.done)
		}
	}
}

func (s *PostgresStore) enqueue(job postgresWriteJob) error {
	if s.closed {
		return errors.New("store: PostgreSQL closed")
	}
	job = s.stampWriteJob(job)
	select {
	case s.writeQueue <- job:
		s.updateAsyncMetrics()
		return nil
	default:
	}
	// A full queue never blocks the World. Only the newest snapshot of each
	// account/character is kept; derived tasks coalesce by label.
	s.overflowMu.Lock()
	switch {
	case job.account != nil:
		if _, replaced := s.overflowAcc[job.account.key]; replaced {
			s.coalesced.Add(1)
			metricPostgresCoalesced.Add(1)
		}
		s.overflowAcc[job.account.key] = job
	case job.charState != nil:
		if _, replaced := s.overflowState[job.charState.uid]; replaced {
			s.coalesced.Add(1)
			metricPostgresCoalesced.Add(1)
		}
		s.overflowState[job.charState.uid] = job
	case job.run != nil:
		s.overflowRuns[job.label] = job
	default:
		s.overflowMu.Unlock()
		return errors.New("store: queue full for a persistence barrier")
	}
	s.overflowMu.Unlock()
	s.updateAsyncMetrics()
	return nil
}

func (s *PostgresStore) stampWriteJob(job postgresWriteJob) postgresWriteJob {
	if job.seq == 0 {
		job.seq = s.writeSeq.Add(1)
	}
	s.orderMu.Lock()
	defer s.orderMu.Unlock()
	if job.account != nil {
		if s.latestAccountSeq == nil {
			s.latestAccountSeq = make(map[string]uint64)
		}
		if job.seq > s.latestAccountSeq[job.account.key] {
			s.latestAccountSeq[job.account.key] = job.seq
		}
	}
	if job.charState != nil {
		if s.latestStateSeq == nil {
			s.latestStateSeq = make(map[string]uint64)
		}
		if job.seq > s.latestStateSeq[job.charState.uid] {
			s.latestStateSeq[job.charState.uid] = job.seq
		}
	}
	return job
}

func (s *PostgresStore) isLatestAccountJob(job postgresWriteJob) bool {
	if job.account == nil {
		return false
	}
	s.orderMu.Lock()
	defer s.orderMu.Unlock()
	return s.latestAccountSeq[job.account.key] == job.seq
}

func (s *PostgresStore) isLatestCharStateJob(job postgresWriteJob) bool {
	if job.charState == nil {
		return false
	}
	s.orderMu.Lock()
	defer s.orderMu.Unlock()
	return s.latestStateSeq[job.charState.uid] == job.seq
}

func (s *PostgresStore) takeOverflow() (postgresWriteJob, bool) {
	return s.takeOverflowBefore(0)
}

// takeOverflowBefore removes the oldest coalesced job before the cutoff. A zero
// cutoff means any generation. Picking the smallest seq keeps the order
// deterministic even though the overflow maps are iterated randomly.
func (s *PostgresStore) takeOverflowBefore(cutoff uint64) (postgresWriteJob, bool) {
	s.overflowMu.Lock()
	defer s.overflowMu.Unlock()
	type selectedJob struct {
		kind byte
		key  string
		job  postgresWriteJob
	}
	var selected selectedJob
	found := false
	consider := func(kind byte, key string, job postgresWriteJob) {
		if cutoff != 0 && job.seq >= cutoff {
			return
		}
		if !found || job.seq < selected.job.seq {
			selected = selectedJob{kind: kind, key: key, job: job}
			found = true
		}
	}
	for key, job := range s.overflowAcc {
		consider('a', key, job)
	}
	for key, job := range s.overflowState {
		consider('s', key, job)
	}
	for key, job := range s.overflowRuns {
		consider('r', key, job)
	}
	if !found {
		return postgresWriteJob{}, false
	}
	switch selected.kind {
	case 'a':
		delete(s.overflowAcc, selected.key)
	case 's':
		delete(s.overflowState, selected.key)
	case 'r':
		delete(s.overflowRuns, selected.key)
	}
	metricPostgresPending.Set(int64(len(s.overflowAcc) + len(s.overflowState) + len(s.overflowRuns)))
	return selected.job, true
}

func (s *PostgresStore) updateAsyncMetrics() {
	metricPostgresQueueDepth.Set(int64(len(s.writeQueue)))
	s.overflowMu.Lock()
	metricPostgresPending.Set(int64(len(s.overflowAcc) + len(s.overflowState) + len(s.overflowRuns)))
	s.overflowMu.Unlock()
}

func (s *PostgresStore) retryDerived(job postgresWriteJob) {
	if job.run == nil {
		return
	}
	if job.retry < 6 {
		job.retry++
	}
	delay := time.Second << job.retry
	if delay > time.Minute {
		delay = time.Minute
	}
	time.AfterFunc(delay, func() {
		s.mu.RLock()
		defer s.mu.RUnlock()
		if !s.closed {
			_ = s.enqueue(job)
		}
	})
}

func (s *PostgresStore) scheduleGuildExport(guilds *model.GuildRegistry) {
	if guilds == nil || s.guildsTxtPath == "" {
		return
	}
	path := s.guildsTxtPath
	payload := append([]byte(nil), guilds.GuildsTxt(0, 0)...)
	generation := s.guildExportGen.Add(1)
	_ = s.enqueue(postgresWriteJob{
		label: "export Guilds.txt",
		run: func() error {
			if s.guildExportGen.Load() != generation {
				return nil // a newer state already replaced this artifact.
			}
			if err := writeFileAtomic(path, payload); err != nil {
				return fmt.Errorf("store: exportar Guilds.txt: %w", err)
			}
			return nil
		},
	})
}

// PostgresAsyncStats exposes backpressure without per-player cardinality.
// Pending measures only the coalesced overflow; len(writeQueue) is the FIFO queue.
func (s *PostgresStore) PostgresAsyncStats() (queued, pending int, coalesced uint64) {
	s.overflowMu.Lock()
	pending = len(s.overflowAcc) + len(s.overflowState) + len(s.overflowRuns)
	s.overflowMu.Unlock()
	return len(s.writeQueue), pending, s.coalesced.Load()
}

func (s *PostgresStore) flushLocked() {
	if s.readOnly || s.writeQueue == nil {
		return
	}
	done := make(chan struct{})
	barrier := s.stampWriteJob(postgresWriteJob{done: done})
	s.writeQueue <- barrier
	<-done
}

func (s *PostgresStore) Flush() {
	s.mu.Lock()
	defer s.mu.Unlock()
	if !s.closed && !s.readOnly {
		s.flushLocked()
	}
}

func (s *PostgresStore) Close() {
	s.closeOnce.Do(func() {
		s.mu.Lock()
		if s.readOnly {
			s.closed = true
			s.mu.Unlock()
			s.pool.Close()
			return
		}
		s.flushLocked()
		s.closed = true
		close(s.writeQueue)
		s.mu.Unlock()
		s.pool.Close()
	})
}

func snapshotAccount(acc *model.Account) (*accountSnapshot, error) {
	if acc == nil {
		return nil, errors.New("store: missing account")
	}
	seenCharacters := make(map[string]int, len(acc.Chars))
	for i := range acc.Chars {
		character := &acc.Chars[i]
		if character.Name == "" {
			continue
		}
		uid, err := model.NormalizeCharacterUID(character.UID)
		if err != nil {
			return nil, fmt.Errorf("store: account %q character[%d]: %w", acc.Name, i, err)
		}
		if uid == "" {
			uid, err = model.NewCharacterUID()
			if err != nil {
				return nil, err
			}
		}
		if previous, exists := seenCharacters[uid]; exists {
			return nil, fmt.Errorf("store: character UID %s repeated in slots %d and %d",
				uid, previous, i)
		}
		character.UID = uid
		seenCharacters[uid] = i
	}
	if err := acc.Validate(); err != nil {
		return nil, fmt.Errorf("store: invalid account %q: %w", acc.Name, err)
	}
	snapshot := &accountSnapshot{
		key:     strings.ToLower(acc.Name),
		display: acc.Name,
	}
	seenUID := make(map[string]string)
	for _, ref := range accountItemRefs(acc) {
		item := ref.item
		if item.Index == 0 {
			if item.UID != "" || item.ActivatedUnix != 0 || item.ExpiresUnix != 0 {
				return nil, fmt.Errorf("store: account %q empty %s has identity/expiry", acc.Name, ref.location)
			}
			continue
		}
		uid, err := model.NormalizeItemUID(item.UID)
		if err != nil {
			return nil, fmt.Errorf("store: account %q %s: %w", acc.Name, ref.location, err)
		}
		if uid == "" {
			uid, err = model.NewItemUID()
			if err != nil {
				return nil, err
			}
		}
		item.UID = uid
		if previous, duplicate := seenUID[uid]; duplicate {
			return nil, fmt.Errorf("store: duplicate UID %s at %s and %s", uid, previous, ref.location)
		}
		seenUID[uid] = ref.location
		rawUID, err := hex.DecodeString(uid)
		if err != nil {
			return nil, err // NormalizeItemUID already makes this path defensive.
		}
		var databaseUID [16]byte
		copy(databaseUID[:], rawUID)
		snapshot.items = append(snapshot.items, postgresItem{
			uid: uid, dbUID: pgtype.UUID{Bytes: databaseUID, Valid: true},
			location: ref.location, index: item.Index, effects: item.Eff,
			activatedUnix: item.ActivatedUnix, expiresUnix: item.ExpiresUnix,
		})
	}
	for i := range acc.Chars {
		character := &acc.Chars[i]
		if character.Name == "" {
			continue
		}
		dbUID, err := postgresUUID(character.UID)
		if err != nil {
			return nil, err
		}
		var mortalUID *pgtype.UUID
		if character.ArchMortalUID != "" {
			value, err := postgresUUID(character.ArchMortalUID)
			if err != nil {
				return nil, err
			}
			mortalUID = &value
		}
		snapshot.characters = append(snapshot.characters, postgresCharacter{
			uid: character.UID, dbUID: dbUID, slot: i,
			nameKey: strings.ToLower(character.Name), evolution: character.Evolution,
			mortalUID: mortalUID,
		})
	}
	payload, err := json.Marshal(acc)
	if err != nil {
		return nil, err
	}
	snapshot.payload = payload
	return snapshot, nil
}

func postgresUUID(uid string) (pgtype.UUID, error) {
	normalized, err := model.NormalizeCharacterUID(uid)
	if err != nil || normalized == "" {
		if err == nil {
			err = errors.New("UID is empty")
		}
		return pgtype.UUID{}, err
	}
	raw, err := hex.DecodeString(normalized)
	if err != nil {
		return pgtype.UUID{}, err
	}
	var databaseUID [16]byte
	copy(databaseUID[:], raw)
	return pgtype.UUID{Bytes: databaseUID, Valid: true}, nil
}

func snapshotAccounts(accounts ...*model.Account) ([]*accountSnapshot, error) {
	result := make([]*accountSnapshot, 0, len(accounts))
	seenAccounts := make(map[string]struct{}, len(accounts))
	seenUID := make(map[string]string)
	for _, acc := range accounts {
		snapshot, err := snapshotAccount(acc)
		if err != nil {
			return nil, err
		}
		if _, duplicate := seenAccounts[snapshot.key]; duplicate {
			return nil, fmt.Errorf("store: account %q repeated in the transaction", snapshot.display)
		}
		seenAccounts[snapshot.key] = struct{}{}
		for _, item := range snapshot.items {
			if previous, duplicate := seenUID[item.uid]; duplicate {
				return nil, fmt.Errorf("store: duplicate UID %s at %s and %s/%s",
					item.uid, previous, snapshot.key, item.location)
			}
			seenUID[item.uid] = snapshot.key + "/" + item.location
		}
		result = append(result, snapshot)
	}
	return result, nil
}

func isRetryablePostgres(err error) bool {
	var pgErr *pgconn.PgError
	return errors.As(err, &pgErr) && (pgErr.Code == "40001" || pgErr.Code == "40P01")
}

func isUniqueViolation(err error) bool {
	var pgErr *pgconn.PgError
	return errors.As(err, &pgErr) && pgErr.Code == "23505"
}

func (s *PostgresStore) postgresContext() (context.Context, context.CancelFunc) {
	timeout := s.operationTimeout
	if timeout <= 0 {
		timeout = postgresOperationTimeout
	}
	return context.WithTimeout(context.Background(), timeout)
}

func setPostgresMetricMax(metric *expvar.Int, value int64) {
	postgresMetricMaxMu.Lock()
	if value > metric.Value() {
		metric.Set(value)
	}
	postgresMetricMaxMu.Unlock()
}

func durationMicros(d time.Duration) int64 {
	micros := d.Microseconds()
	if micros < 0 {
		return 0
	}
	return micros
}

func observePostgresTransaction(started time.Time) {
	micros := durationMicros(time.Since(started))
	metricPostgresTxDuration.Set(micros)
	setPostgresMetricMax(metricPostgresTxDurationMax, micros)
}

func observePostgresFenceWait(wait time.Duration) {
	micros := durationMicros(wait)
	metricPostgresFenceWaitTotal.Add(micros)
	metricPostgresFenceWaitCount.Add(1)
	setPostgresMetricMax(metricPostgresFenceWaitMax, micros)
}

func (s *PostgresStore) withSerializableTx(run func(context.Context, pgx.Tx) error) error {
	started := time.Now()
	defer observePostgresTransaction(started)
	// One deadline covers begin, callback, commit and every retry. Previously
	// each attempt got ten more seconds and a critical operation could freeze
	// the single World goroutine for tens of seconds.
	ctx, cancel := s.postgresContext()
	defer cancel()
	var last error
	for attempt := 0; attempt < 3; attempt++ {
		tx, err := s.pool.BeginTx(ctx, pgx.TxOptions{IsoLevel: pgx.Serializable})
		if err != nil {
			return err
		}
		err = run(ctx, tx)
		if err == nil {
			err = tx.Commit(ctx)
		} else {
			_ = tx.Rollback(ctx)
		}
		if !isRetryablePostgres(err) {
			return err
		}
		metricPostgresTxRetries.Add(1)
		last = err
		delay := time.NewTimer(time.Duration(attempt+1) * 10 * time.Millisecond)
		select {
		case <-delay.C:
		case <-ctx.Done():
			if !delay.Stop() {
				select {
				case <-delay.C:
				default:
				}
			}
			return fmt.Errorf("store: PostgreSQL transaction total deadline: %w", ctx.Err())
		}
	}
	return fmt.Errorf("store: concurrent PostgreSQL transaction exceeded retries: %w", last)
}

func persistAccountSnapshots(ctx context.Context, tx pgx.Tx, snapshots []*accountSnapshot, createOnly bool) error {
	for _, snapshot := range snapshots {
		var err error
		if createOnly {
			_, err = tx.Exec(ctx, `
				INSERT INTO accounts(name_key, payload)
				VALUES ($1,$2)`,
				snapshot.key, snapshot.payload)
		} else {
			_, err = tx.Exec(ctx, `
				INSERT INTO accounts(name_key, payload)
				VALUES ($1,$2)
				ON CONFLICT (name_key) DO UPDATE SET
					payload=EXCLUDED.payload,
					version=accounts.version+1,
					updated_at=now()`,
				snapshot.key, snapshot.payload)
		}
		if err != nil {
			return err
		}
	}
	// Reconcile characters first. Removing missing UIDs fires the charstate
	// ON DELETE CASCADE; the rest are updated by UID without depending on the
	// nickname or the current position on the selection screen.
	for _, snapshot := range snapshots {
		desiredUIDs := make([]pgtype.UUID, 0, len(snapshot.characters))
		for _, character := range snapshot.characters {
			desiredUIDs = append(desiredUIDs, character.dbUID)
		}
		if len(desiredUIDs) == 0 {
			if _, err := tx.Exec(ctx, `DELETE FROM characters WHERE account_key=$1`, snapshot.key); err != nil {
				return err
			}
		} else if _, err := tx.Exec(ctx,
			`DELETE FROM characters WHERE account_key=$1 AND NOT (character_uid=ANY($2))`,
			snapshot.key, desiredUIDs); err != nil {
			return err
		}
	}
	for _, snapshot := range snapshots {
		for _, character := range snapshot.characters {
			tag, err := tx.Exec(ctx, `
				INSERT INTO characters(character_uid,account_key,slot,name_key,evolution,mortal_uid)
				VALUES($1,$2,$3,$4,$5,NULL)
				ON CONFLICT(character_uid) DO UPDATE SET
					slot=EXCLUDED.slot,
					name_key=EXCLUDED.name_key,
					evolution=EXCLUDED.evolution
				WHERE characters.account_key=EXCLUDED.account_key
				  AND (characters.slot IS DISTINCT FROM EXCLUDED.slot
				    OR characters.name_key IS DISTINCT FROM EXCLUDED.name_key
				    OR characters.evolution IS DISTINCT FROM EXCLUDED.evolution)`,
				character.dbUID, snapshot.key, character.slot, character.nameKey,
				character.evolution)
			if err != nil {
				return err
			}
			if tag.RowsAffected() != 1 {
				var owner string
				if err := tx.QueryRow(ctx,
					`SELECT account_key FROM characters WHERE character_uid=$1`,
					character.dbUID).Scan(&owner); err != nil {
					return err
				}
				if owner != snapshot.key {
					return fmt.Errorf("store: character UID %s belongs to another account",
						character.uid)
				}
			}
		}
	}
	for _, snapshot := range snapshots {
		for _, character := range snapshot.characters {
			var mortalUID any
			if character.mortalUID != nil {
				mortalUID = *character.mortalUID
			}
			if _, err := tx.Exec(ctx,
				`UPDATE characters SET mortal_uid=$2
				 WHERE character_uid=$1 AND mortal_uid IS DISTINCT FROM $2`,
				character.dbUID, mortalUID); err != nil {
				return err
			}
		}
	}

	// Names are global reservations per account, not character identities: a
	// Mortal and an Arch with the same name share one reservation.
	for _, snapshot := range snapshots {
		names := make([]string, 0, len(snapshot.characters))
		seen := make(map[string]struct{}, len(snapshot.characters))
		for _, character := range snapshot.characters {
			if _, exists := seen[character.nameKey]; !exists {
				seen[character.nameKey] = struct{}{}
				names = append(names, character.nameKey)
			}
		}
		if len(names) == 0 {
			if _, err := tx.Exec(ctx, `DELETE FROM character_names WHERE account_key=$1`, snapshot.key); err != nil {
				return err
			}
		} else if _, err := tx.Exec(ctx,
			`DELETE FROM character_names WHERE account_key=$1 AND NOT (name_key=ANY($2))`,
			snapshot.key, names); err != nil {
			return err
		}
		for _, name := range names {
			tag, err := tx.Exec(ctx, `
				INSERT INTO character_names(name_key,account_key)
				VALUES ($1,$2)
				ON CONFLICT (name_key) DO NOTHING`, name, snapshot.key)
			if err != nil {
				return err
			}
			if tag.RowsAffected() != 1 {
				var owner string
				if err := tx.QueryRow(ctx,
					`SELECT account_key FROM character_names WHERE name_key=$1`,
					name).Scan(&owner); err != nil {
					return err
				}
				if owner != snapshot.key {
					return fmt.Errorf("store: character %q already belongs to another account", name)
				}
			}
		}
	}

	// Incremental item ledger. First remove every row that is no longer
	// identical; only then insert/update the changed ones. This order lets a
	// UID move between accounts/slots in the same transaction without hitting
	// the global constraint or rewriting thousands of unchanged items.
	type storedItem struct {
		uid, account, location string
		index                  uint16
		effects                [6]byte
		activatedUnix          int64
		expiresUnix            int64
	}
	desired := make(map[string]struct {
		account string
		item    postgresItem
	})
	keys := make([]string, 0, len(snapshots))
	for _, snapshot := range snapshots {
		keys = append(keys, snapshot.key)
		for _, item := range snapshot.items {
			desired[item.uid] = struct {
				account string
				item    postgresItem
			}{snapshot.key, item}
		}
	}
	existing := make(map[string]storedItem)
	if len(keys) != 0 {
		rows, err := tx.Query(ctx, `
			SELECT replace(uid::text,'-',''),account_key,location,item_index,effects,
			       activated_unix,expires_unix
			FROM item_instances WHERE account_key=ANY($1)`, keys)
		if err != nil {
			return err
		}
		for rows.Next() {
			var row storedItem
			var index int
			var effects []byte
			if err := rows.Scan(&row.uid, &row.account, &row.location, &index, &effects,
				&row.activatedUnix, &row.expiresUnix); err != nil {
				rows.Close()
				return err
			}
			row.index = uint16(index)
			copy(row.effects[:], effects)
			existing[row.uid] = row
		}
		if err := rows.Err(); err != nil {
			rows.Close()
			return err
		}
		rows.Close()
	}
	changed := make(map[string]struct{}, len(desired))
	for uid, current := range existing {
		want, ok := desired[uid]
		if ok && current.account == want.account && current.location == want.item.location &&
			current.index == want.item.index && current.effects == want.item.effects &&
			current.activatedUnix == want.item.activatedUnix && current.expiresUnix == want.item.expiresUnix {
			continue
		}
		if _, err := tx.Exec(ctx, `DELETE FROM item_instances WHERE uid=$1`, currentUID(current.uid)); err != nil {
			return err
		}
		if ok {
			changed[uid] = struct{}{}
		}
	}
	for uid := range desired {
		if _, existed := existing[uid]; !existed {
			changed[uid] = struct{}{}
		}
	}
	for uid := range changed {
		want := desired[uid]
		tag, err := tx.Exec(ctx, `
			INSERT INTO item_instances(uid,account_key,location,item_index,effects,activated_unix,expires_unix)
			VALUES ($1,$2,$3,$4,$5,$6,$7)
			ON CONFLICT(uid) DO UPDATE SET
				location=EXCLUDED.location,
				item_index=EXCLUDED.item_index,
				effects=EXCLUDED.effects,
				activated_unix=EXCLUDED.activated_unix,
				expires_unix=EXCLUDED.expires_unix
			WHERE item_instances.account_key=EXCLUDED.account_key`,
			want.item.dbUID, want.account, want.item.location,
			int(want.item.index), want.item.effects[:],
			want.item.activatedUnix, want.item.expiresUnix)
		if err != nil {
			if isUniqueViolation(err) {
				return fmt.Errorf("store: duplicate item UID %s: %w", uid, err)
			}
			return err
		}
		if tag.RowsAffected() != 1 {
			return fmt.Errorf("store: duplicate item UID %s belongs to another account", uid)
		}
	}
	return nil
}

func currentUID(uid string) pgtype.UUID {
	raw, _ := hex.DecodeString(uid)
	var value [16]byte
	copy(value[:], raw)
	return pgtype.UUID{Bytes: value, Valid: true}
}

func (s *PostgresStore) LoadAccount(name string) (*model.Account, error) {
	ctx, cancel := s.postgresContext()
	defer cancel()
	var payload []byte
	err := s.pool.QueryRow(ctx,
		`SELECT payload FROM accounts WHERE name_key=$1`, strings.ToLower(name)).Scan(&payload)
	if errors.Is(err, pgx.ErrNoRows) {
		return nil, fmt.Errorf("%w: account %q", os.ErrNotExist, name)
	}
	if err != nil {
		return nil, err
	}
	var acc model.Account
	if err := decodeAccountJSON(payload, &acc); err != nil {
		return nil, fmt.Errorf("store: corrupted account %q: %w", name, err)
	}
	if err := acc.Validate(); err != nil {
		return nil, fmt.Errorf("store: invalid account %q: %w", name, err)
	}
	return &acc, nil
}

const postgresPersistentPlayerQuery = `
SELECT
    COALESCE(picked.account_name, ''),
    picked.character->>'uid',
    picked.character->>'name',
    COALESCE((picked.character->>'class')::bigint, 0),
    COALESCE((picked.character->'score'->>'level')::bigint, 0),
    COALESCE(picked.character->>'evolution', ''),
    COALESCE((picked.character->'score'->>'curHP')::bigint, 0),
    COALESCE((picked.character->'score'->>'maxHP')::bigint, 0),
    COALESCE((picked.character->'score'->>'curMP')::bigint, 0),
    COALESCE((picked.character->'score'->>'maxMP')::bigint, 0),
    COALESCE((picked.character->'score'->>'attack')::bigint, 0),
    COALESCE((picked.character->'score'->>'magicAttack')::bigint, 0),
    COALESCE((picked.character->'score'->>'defense')::bigint, 0),
    COALESCE((picked.character->'score'->>'str')::bigint, 0),
    COALESCE((picked.character->'score'->>'int')::bigint, 0),
    COALESCE((picked.character->'score'->>'dex')::bigint, 0),
    COALESCE((picked.character->'score'->>'con')::bigint, 0),
    COALESCE((picked.character->'score'->>'statusPoints')::bigint, 0),
    COALESCE((picked.character->'score'->>'masteryPoints')::bigint, 0),
    COALESCE((picked.character->'score'->>'skillPoints')::bigint, 0),
    COALESCE((picked.character->>'x')::bigint, 0),
    COALESCE((picked.character->>'y')::bigint, 0),
    COALESCE((picked.character->>'gold')::bigint, 0),
    COALESCE((picked.character->>'exp')::bigint, 0),
    COALESCE((picked.character->>'hold')::bigint, 0),
    COALESCE((picked.character->>'cp')::bigint, 0)
FROM (
    SELECT
        a.payload->>'name' AS account_name,
        a.payload->'chars'->c.slot AS character
    FROM characters c
    JOIN accounts a ON a.name_key = c.account_key
    WHERE c.account_key=$1 AND c.character_uid=$2
) AS picked
WHERE picked.character->>'uid'=$3
  AND COALESCE(picked.character->>'name', '') <> ''
  AND picked.character->'score' IS NOT NULL
LIMIT 1`

const postgresAccountDirectoryQuery = `
WITH account_page AS (
    SELECT
        a.name_key,
        COALESCE(NULLIF(a.payload->>'name', ''), a.name_key) AS account_name,
        a.created_at,
        a.updated_at
    FROM accounts a
    WHERE a.name_key > $1
      AND (
          $2 = ''
          OR a.name_key LIKE $2 || '%'
          OR EXISTS (
              SELECT 1
              FROM character_names n
              WHERE n.account_key = a.name_key
                AND n.name_key LIKE $2 || '%'
          )
      )
    ORDER BY a.name_key
    LIMIT $3
)
SELECT
    page.name_key,
    page.account_name,
    page.created_at,
    page.updated_at,
    c.slot,
    COALESCE(REPLACE(c.character_uid::text, '-', ''), ''),
    COALESCE(NULLIF(a.payload->'chars'->c.slot->>'name', ''), c.name_key, ''),
    COALESCE((a.payload->'chars'->c.slot->>'class')::bigint, 0),
    COALESCE((a.payload->'chars'->c.slot->'score'->>'level')::bigint, 0),
    COALESCE(NULLIF(a.payload->'chars'->c.slot->>'evolution', ''), c.evolution, '')
FROM account_page page
JOIN accounts a ON a.name_key = page.name_key
LEFT JOIN characters c ON c.account_key = page.name_key
ORDER BY page.name_key, c.slot`

// ReadPersistentPlayer reads only the fields required by the read-only staff
// console. PostgreSQL projects the matching character before any data reaches
// the web process, so account credentials and item payloads are not decoded.
func (s *PostgresStore) ReadPersistentPlayer(ctx context.Context, accountName, uid string) (PersistentPlayerSnapshot, error) {
	if ctx == nil {
		return PersistentPlayerSnapshot{}, errors.New("store: missing context")
	}
	accountName = strings.TrimSpace(accountName)
	if accountName == "" || len(accountName) > 12 {
		return PersistentPlayerSnapshot{}, errors.New("store: invalid account")
	}
	normalizedUID, err := model.NormalizeCharacterUID(uid)
	if err != nil || normalizedUID == "" {
		return PersistentPlayerSnapshot{}, errors.New("store: invalid character UID")
	}
	if s == nil || s.pool == nil {
		return PersistentPlayerSnapshot{}, errors.New("store: PostgreSQL closed")
	}
	timeout := s.operationTimeout
	if timeout <= 0 {
		timeout = postgresOperationTimeout
	}
	queryCtx, cancel := context.WithTimeout(ctx, timeout)
	defer cancel()

	var out PersistentPlayerSnapshot
	err = s.pool.QueryRow(queryCtx, postgresPersistentPlayerQuery,
		strings.ToLower(accountName), currentUID(normalizedUID), normalizedUID).Scan(
		&out.Account, &out.UID, &out.Name, &out.Class, &out.Level, &out.Evolution,
		&out.HP, &out.MaxHP, &out.MP, &out.MaxMP, &out.Attack, &out.MagicAttack,
		&out.Defense, &out.Str, &out.Int, &out.Dex, &out.Con, &out.StatusPoints,
		&out.MasteryPoints, &out.SkillPoints, &out.X, &out.Y, &out.Gold, &out.Exp,
		&out.Hold, &out.CP)
	if errors.Is(err, pgx.ErrNoRows) {
		return PersistentPlayerSnapshot{}, fmt.Errorf("%w: character %q of account %q", os.ErrNotExist, normalizedUID, accountName)
	}
	if err != nil {
		return PersistentPlayerSnapshot{}, err
	}
	return out, nil
}

// ReadAccountDirectory returns at most limit accounts plus a keyset cursor.
// The query projects only display metadata and character summaries; it never
// materializes the account JSON, password hash, inventory or cargo in Go.
func (s *PostgresStore) ReadAccountDirectory(ctx context.Context, search, cursor string, limit int) (AccountDirectoryPageSnapshot, error) {
	if ctx == nil {
		return AccountDirectoryPageSnapshot{}, errors.New("store: context is missing")
	}
	search = strings.ToLower(strings.TrimSpace(search))
	cursor = strings.ToLower(strings.TrimSpace(cursor))
	if !validDirectoryKey(search, true) || !validDirectoryKey(cursor, true) || limit < 1 || limit > 50 {
		return AccountDirectoryPageSnapshot{}, errors.New("store: invalid account query")
	}
	if s == nil || s.pool == nil {
		return AccountDirectoryPageSnapshot{}, errors.New("store: PostgreSQL closed")
	}
	timeout := s.operationTimeout
	if timeout <= 0 {
		timeout = postgresOperationTimeout
	}
	queryCtx, cancel := context.WithTimeout(ctx, timeout)
	defer cancel()

	rows, err := s.pool.Query(queryCtx, postgresAccountDirectoryQuery, cursor, search, limit+1)
	if err != nil {
		return AccountDirectoryPageSnapshot{}, err
	}
	defer rows.Close()

	page := AccountDirectoryPageSnapshot{Accounts: make([]AccountDirectorySnapshot, 0, limit+1)}
	byKey := make(map[string]int, limit+1)
	for rows.Next() {
		var key, name, uid, characterName, evolution string
		var createdAt, updatedAt time.Time
		var slot pgtype.Int2
		var classValue, levelValue int64
		if err := rows.Scan(&key, &name, &createdAt, &updatedAt, &slot, &uid,
			&characterName, &classValue, &levelValue, &evolution); err != nil {
			return AccountDirectoryPageSnapshot{}, err
		}
		index, exists := byKey[key]
		if !exists {
			index = len(page.Accounts)
			byKey[key] = index
			page.Accounts = append(page.Accounts, AccountDirectorySnapshot{
				Key: key, Name: name, CreatedAt: createdAt, UpdatedAt: updatedAt,
				Characters: make([]AccountDirectoryCharacterSnapshot, 0, 4),
			})
		}
		if !slot.Valid {
			continue
		}
		if classValue < 0 || classValue > 255 || levelValue < 0 || levelValue > int64(^uint32(0)) {
			return AccountDirectoryPageSnapshot{}, errors.New("store: character summary out of range")
		}
		page.Accounts[index].Characters = append(page.Accounts[index].Characters, AccountDirectoryCharacterSnapshot{
			UID: uid, Slot: int(slot.Int16), Name: characterName, Class: byte(classValue),
			Level: uint32(levelValue), Evolution: evolution,
		})
	}
	if err := rows.Err(); err != nil {
		return AccountDirectoryPageSnapshot{}, err
	}
	if len(page.Accounts) > limit {
		page.Accounts = page.Accounts[:limit]
		page.NextCursor = page.Accounts[len(page.Accounts)-1].Key
	}
	return page, nil
}

func validDirectoryKey(value string, allowEmpty bool) bool {
	if value == "" {
		return allowEmpty
	}
	if len(value) > 12 {
		return false
	}
	for i := 0; i < len(value); i++ {
		if value[i] < 'a' || value[i] > 'z' {
			if value[i] < '0' || value[i] > '9' {
				return false
			}
		}
	}
	return true
}

func (s *PostgresStore) saveSnapshots(snapshots []*accountSnapshot) error {
	return s.withSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		return persistAccountSnapshots(ctx, tx, snapshots, false)
	})
}

func (s *PostgresStore) SaveAccount(acc *model.Account) error {
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	seq := s.writeSeq.Add(1)
	release, err := s.beginPostgresWrite()
	if err != nil {
		return err
	}
	defer release()
	unlockFences := s.accountFences.lock(accountModelKeys(acc)...)
	defer unlockFences()
	snapshot, err := snapshotAccount(acc)
	if err != nil {
		return err
	}
	s.fenceAccountSnapshots(seq, []*accountSnapshot{snapshot})
	return s.saveSnapshots([]*accountSnapshot{snapshot})
}

func (s *PostgresStore) SaveAccountAsync(acc *model.Account) error {
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	seq := s.writeSeq.Add(1)
	release, err := s.beginPostgresWrite()
	if err != nil {
		return err
	}
	defer release()
	unlockFences := s.accountFences.lock(accountModelKeys(acc)...)
	defer unlockFences()
	snapshot, err := snapshotAccount(acc)
	if err != nil {
		return err
	}
	return s.enqueue(postgresWriteJob{
		label:   "account autosave " + snapshot.display,
		seq:     seq,
		account: snapshot,
	})
}

func (s *PostgresStore) SaveAccounts(accounts ...*model.Account) error {
	return s.SaveGameState(nil, accounts...)
}

func (s *PostgresStore) CreateAccount(acc *model.Account) error {
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	seq := s.writeSeq.Add(1)
	release, err := s.beginPostgresWrite()
	if err != nil {
		return err
	}
	defer release()
	unlockFences := s.accountFences.lock(accountModelKeys(acc)...)
	defer unlockFences()
	snapshot, err := snapshotAccount(acc)
	if err != nil {
		return err
	}
	s.fenceAccountSnapshots(seq, []*accountSnapshot{snapshot})
	err = s.withSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		return persistAccountSnapshots(ctx, tx, []*accountSnapshot{snapshot}, true)
	})
	if isUniqueViolation(err) {
		return ErrAccountExists
	}
	return err
}

func (s *PostgresStore) AccountNameExists(name string) (bool, error) {
	ctx, cancel := s.postgresContext()
	defer cancel()
	var exists bool
	err := s.pool.QueryRow(ctx,
		`SELECT EXISTS(SELECT 1 FROM accounts WHERE name_key=$1)`,
		strings.ToLower(name)).Scan(&exists)
	return exists, err
}

func (s *PostgresStore) CharacterNameExists(name string) (bool, error) {
	ctx, cancel := s.postgresContext()
	defer cancel()
	var exists bool
	err := s.pool.QueryRow(ctx,
		`SELECT EXISTS(SELECT 1 FROM character_names WHERE name_key=$1)`,
		strings.ToLower(name)).Scan(&exists)
	return exists, err
}

func (s *PostgresStore) CharacterNames() (map[string]struct{}, error) {
	ctx, cancel := s.postgresContext()
	defer cancel()
	rows, err := s.pool.Query(ctx, `SELECT name_key FROM character_names`)
	if err != nil {
		return nil, err
	}
	defer rows.Close()
	names := make(map[string]struct{})
	for rows.Next() {
		var name string
		if err := rows.Scan(&name); err != nil {
			return nil, err
		}
		names[name] = struct{}{}
	}
	return names, rows.Err()
}

func (s *PostgresStore) LoadGuilds() (*model.GuildRegistry, error) {
	ctx, cancel := s.postgresContext()
	defer cancel()
	var payload []byte
	err := s.pool.QueryRow(ctx,
		`SELECT payload FROM guild_state WHERE singleton=true`).Scan(&payload)
	if errors.Is(err, pgx.ErrNoRows) {
		return &model.GuildRegistry{Version: model.GuildRegistryVersion}, nil
	}
	if err != nil {
		return nil, err
	}
	var registry model.GuildRegistry
	decoder := json.NewDecoder(bytes.NewReader(payload))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&registry); err != nil {
		return nil, fmt.Errorf("store: corrupt guild state: %w", err)
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		return nil, errors.New("store: guild state contains trailing JSON")
	}
	if err := registry.Validate(); err != nil {
		return nil, err
	}
	return &registry, nil
}

func (s *PostgresStore) LoadInstanceState() (*model.InstanceStateSnapshot, error) {
	ctx, cancel := s.postgresContext()
	defer cancel()
	var payload []byte
	err := s.pool.QueryRow(ctx,
		`SELECT payload FROM instance_state WHERE singleton=true`).Scan(&payload)
	if errors.Is(err, pgx.ErrNoRows) {
		return &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}, nil
	}
	if err != nil {
		return nil, err
	}
	var state model.InstanceStateSnapshot
	decoder := json.NewDecoder(bytes.NewReader(payload))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&state); err != nil {
		return nil, fmt.Errorf("store: corrupt instance state: %w", err)
	}
	if state.Version != model.InstanceStateVersion {
		return nil, fmt.Errorf("store: instance state version %d; expected %d",
			state.Version, model.InstanceStateVersion)
	}
	return &state, nil
}

func (s *PostgresStore) SaveInstanceState(state *model.InstanceStateSnapshot) error {
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	if state == nil {
		state = &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}
	}
	if state.Version != model.InstanceStateVersion {
		return fmt.Errorf("store: instance state version %d; expected %d",
			state.Version, model.InstanceStateVersion)
	}
	b, err := json.Marshal(state)
	if err != nil {
		return err
	}
	release, err := s.beginPostgresWrite()
	if err != nil {
		return err
	}
	defer release()
	return s.withSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		_, err := tx.Exec(ctx, `
			INSERT INTO instance_state(singleton,payload) VALUES(true,$1)
			ON CONFLICT(singleton) DO UPDATE SET
				payload=EXCLUDED.payload,
				version=instance_state.version+1,
				updated_at=now()`, b)
		return err
	})
}

func (s *PostgresStore) SaveGameState(guilds *model.GuildRegistry, accounts ...*model.Account) error {
	return s.saveGameStateWithInstanceState(nil, guilds, accounts...)
}

func (s *PostgresStore) SaveGameStateWithInstanceState(guilds *model.GuildRegistry,
	state *model.InstanceStateSnapshot, accounts ...*model.Account) error {
	return s.saveGameStateWithInstanceState(state, guilds, accounts...)
}

func (s *PostgresStore) saveGameStateWithInstanceState(state *model.InstanceStateSnapshot,
	guilds *model.GuildRegistry, accounts ...*model.Account) error {
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	seq := s.writeSeq.Add(1)
	release, err := s.beginPostgresWrite()
	if err != nil {
		return err
	}
	defer release()
	if state == nil && guilds == nil && len(accounts) == 0 {
		return nil
	}
	if state != nil && state.Version != model.InstanceStateVersion {
		return fmt.Errorf("store: instance state version %d; expected %d",
			state.Version, model.InstanceStateVersion)
	}
	var guildPayload []byte
	if guilds != nil {
		if err := guilds.Validate(); err != nil {
			return fmt.Errorf("store: invalid guilds: %w", err)
		}
		guildPayload, err = json.Marshal(guilds)
		if err != nil {
			return err
		}
	}
	var instancePayload []byte
	if state != nil {
		instancePayload, err = json.Marshal(state)
		if err != nil {
			return err
		}
	}
	unlockFences := s.accountFences.lock(accountModelKeys(accounts...)...)
	defer unlockFences()
	snapshots, err := snapshotAccounts(accounts...)
	if err != nil {
		return err
	}
	s.fenceAccountSnapshots(seq, snapshots)
	err = s.withSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		if err := persistAccountSnapshots(ctx, tx, snapshots, false); err != nil {
			return err
		}
		if guilds != nil {
			_, err := tx.Exec(ctx, `
				INSERT INTO guild_state(singleton,payload) VALUES(true,$1)
				ON CONFLICT(singleton) DO UPDATE SET
					payload=EXCLUDED.payload,
					version=guild_state.version+1,
					updated_at=now()`, guildPayload)
			if err != nil {
				return err
			}
		}
		if state != nil {
			_, err := tx.Exec(ctx, `
				INSERT INTO instance_state(singleton,payload) VALUES(true,$1)
				ON CONFLICT(singleton) DO UPDATE SET
					payload=EXCLUDED.payload,
					version=instance_state.version+1,
					updated_at=now()`, instancePayload)
			return err
		}
		return nil
	})
	if err != nil {
		return err
	}
	if guilds != nil && s.guildsTxtPath != "" {
		// Guilds.txt is a derived artifact: the PostgreSQL commit above is the
		// truth. An export failure is retried and never reaches the handler as
		// if the authoritative transaction had failed.
		s.scheduleGuildExport(guilds)
	}
	return nil
}

// SavePlayerState commits the account, charstate and optionally the guilds in
// the same serializable commit. Crafts/quests that spend items and
// per-character counters use it: there is no window where only half survives.
func (s *PostgresStore) SavePlayerState(guilds *model.GuildRegistry, account *model.Account,
	characterUID string, state *model.CharState) error {
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	seq := s.writeSeq.Add(1)
	stateKey, err := postgresStateFenceKey(characterUID)
	if err != nil {
		return err
	}
	release, err := s.beginPostgresWrite()
	if err != nil {
		return err
	}
	defer release()
	unlockAccounts := s.accountFences.lock(accountModelKeys(account)...)
	defer unlockAccounts()
	unlockState := s.stateFences.lock(stateKey)
	defer unlockState()
	snapshots, err := snapshotAccounts(account)
	if err != nil {
		return err
	}
	statePayload, err := charStatePayload(stateKey, state)
	if err != nil {
		return err
	}
	stateUID, err := postgresUUID(stateKey)
	if err != nil {
		return err
	}
	var guildPayload []byte
	if guilds != nil {
		if err := guilds.Validate(); err != nil {
			return fmt.Errorf("store: invalid guilds: %w", err)
		}
		guildPayload, err = json.Marshal(guilds)
		if err != nil {
			return err
		}
	}
	s.fenceAccountSnapshots(seq, snapshots)
	s.fenceCharStates(seq, stateKey)
	if err := s.withSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		if err := persistAccountSnapshots(ctx, tx, snapshots, false); err != nil {
			return err
		}
		if statePayload == nil {
			if _, err := tx.Exec(ctx,
				`DELETE FROM character_states WHERE character_uid=$1`, stateUID); err != nil {
				return err
			}
		} else if _, err := tx.Exec(ctx, `
			INSERT INTO character_states(character_uid,payload) VALUES($1,$2)
			ON CONFLICT(character_uid) DO UPDATE SET
				payload=EXCLUDED.payload,updated_at=now()`,
			stateUID, statePayload); err != nil {
			return err
		}
		if guilds != nil {
			_, err := tx.Exec(ctx, `
				INSERT INTO guild_state(singleton,payload) VALUES(true,$1)
				ON CONFLICT(singleton) DO UPDATE SET
					payload=EXCLUDED.payload,
					version=guild_state.version+1,
					updated_at=now()`, guildPayload)
			return err
		}
		return nil
	}); err != nil {
		return err
	}
	if guilds != nil && s.guildsTxtPath != "" {
		s.scheduleGuildExport(guilds)
	}
	return nil
}

func emptyCharState(state *model.CharState) bool {
	return state == nil || (len(state.Affects) == 0 && len(state.SpecialCoins) == 0)
}

func charStatePayload(uid string, state *model.CharState) ([]byte, error) {
	if _, err := postgresStateFenceKey(uid); err != nil {
		return nil, err
	}
	if emptyCharState(state) {
		return nil, nil
	}
	clone := *state
	clone.Version = model.CharStateVersion
	return json.Marshal(&clone)
}

func (s *PostgresStore) saveCharStatePayload(uid string, payload []byte) error {
	ctx, cancel := s.postgresContext()
	defer cancel()
	key, err := postgresUUID(uid)
	if err != nil {
		return err
	}
	if payload == nil {
		_, err := s.pool.Exec(ctx,
			`DELETE FROM character_states WHERE character_uid=$1`, key)
		return err
	}
	_, err = s.pool.Exec(ctx, `
		INSERT INTO character_states(character_uid,payload) VALUES($1,$2)
		ON CONFLICT(character_uid) DO UPDATE SET payload=EXCLUDED.payload,updated_at=now()`,
		key, payload)
	return err
}

func (s *PostgresStore) saveCharStateSnapshots(snapshots []*postgresCharStateSnapshot) error {
	return s.withSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		for _, snapshot := range snapshots {
			key, err := postgresUUID(snapshot.uid)
			if err != nil {
				return err
			}
			if snapshot.payload == nil {
				if _, err := tx.Exec(ctx,
					`DELETE FROM character_states WHERE character_uid=$1`, key); err != nil {
					return err
				}
				continue
			}
			if _, err := tx.Exec(ctx, `
				INSERT INTO character_states(character_uid,payload) VALUES($1,$2)
				ON CONFLICT(character_uid) DO UPDATE SET payload=EXCLUDED.payload,updated_at=now()`,
				key, snapshot.payload); err != nil {
				return err
			}
		}
		return nil
	})
}

func (s *PostgresStore) SaveCharState(uid string, state *model.CharState) error {
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	seq := s.writeSeq.Add(1)
	stateKey, err := postgresStateFenceKey(uid)
	if err != nil {
		return err
	}
	release, err := s.beginPostgresWrite()
	if err != nil {
		return err
	}
	defer release()
	unlockFences := s.stateFences.lock(stateKey)
	defer unlockFences()
	payload, err := charStatePayload(stateKey, state)
	if err != nil {
		return err
	}
	s.fenceCharStates(seq, stateKey)
	return s.saveCharStatePayload(stateKey, payload)
}

func (s *PostgresStore) SaveCharStateAsync(uid string, state *model.CharState) error {
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	seq := s.writeSeq.Add(1)
	stateKey, err := postgresStateFenceKey(uid)
	if err != nil {
		return err
	}
	release, err := s.beginPostgresWrite()
	if err != nil {
		return err
	}
	defer release()
	unlockFences := s.stateFences.lock(stateKey)
	defer unlockFences()
	payload, err := charStatePayload(stateKey, state)
	if err != nil {
		return err
	}
	return s.enqueue(postgresWriteJob{
		label:     "charstate autosave " + stateKey,
		seq:       seq,
		charState: &postgresCharStateSnapshot{uid: stateKey, payload: payload},
	})
}

func (s *PostgresStore) LoadCharState(uid string) (*model.CharState, error) {
	ctx, cancel := s.postgresContext()
	defer cancel()
	key, err := postgresUUID(uid)
	if err != nil {
		return nil, err
	}
	var payload []byte
	err = s.pool.QueryRow(ctx,
		`SELECT payload FROM character_states WHERE character_uid=$1`, key).Scan(&payload)
	if errors.Is(err, pgx.ErrNoRows) {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	decoder := json.NewDecoder(bytes.NewReader(payload))
	decoder.DisallowUnknownFields()
	var state model.CharState
	if err := decoder.Decode(&state); err != nil {
		return nil, err
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		return nil, errors.New("store: charstate contains trailing JSON")
	}
	if state.Version != model.CharStateVersion {
		return nil, fmt.Errorf("store: charstate %q version %d; expected %d",
			uid, state.Version, model.CharStateVersion)
	}
	return &state, nil
}
