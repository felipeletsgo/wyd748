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

const postgresWriteQueueSize = 2048
const postgresSchemaVersion = 2
const postgresOperationTimeout = 10 * time.Second
const postgresStartupTimeout = 30 * time.Second
const postgresAutosaveBatchSize = 64
const postgresCharStateBatchSize = 128
const postgresAutosaveCollectWindow = 2 * time.Millisecond

var (
	metricPostgresQueueDepth = expvar.NewInt("postgres_async_queue_depth")
	metricPostgresPending    = expvar.NewInt("postgres_async_pending_coalesced")
	metricPostgresCoalesced  = expvar.NewInt("postgres_async_snapshots_coalesced_total")
	metricPostgresFailures   = expvar.NewInt("postgres_async_failures_total")
)

// Valor privado deste projeto. O lock de sessao impede server e account-api de
// tentarem aplicar DDL simultaneamente no primeiro boot.
const postgresSchemaAdvisoryLock int64 = 0x575944474F

// PostgresConfig configura apenas persistencia. A URL deve vir de variavel de
// ambiente em producao para a senha nao parar no repositorio nem na linha de
// comando do processo.
type PostgresConfig struct {
	URL           string
	MaxConns      int32
	GuildsTxtPath string
}

type postgresItem struct {
	uid      string
	dbUID    pgtype.UUID
	location string
	index    uint16
	effects  [6]byte
}

type postgresCharacter struct {
	uid       string
	dbUID     pgtype.UUID
	slot      int
	nameKey   string
	evolution string
	mortalUID *pgtype.UUID
}

// accountSnapshot e imutavel depois de criado. Isso permite ao autosave sair
// do game-loop sem observar ponteiros que continuam sendo alterados pelo World.
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
	// account identifica autosave de conta e permite ao worker consolidar
	// varios snapshots em uma unica transacao. run fica para charstate/outros.
	account   *accountSnapshot
	charState *postgresCharStateSnapshot
	run       func() error
	done      chan struct{}
	retry     uint8
}

// PostgresStore e a persistencia autoritativa de producao. A ficha fica em
// JSONB, mas nomes e instancias de item tambem sao gravados em tabelas com
// constraints. O JSONB nunca e confirmado sem os indices anti-dupe.
type PostgresStore struct {
	pool           *pgxpool.Pool
	guildsTxtPath  string
	writeQueue     chan postgresWriteJob
	mu             sync.Mutex
	overflowMu     sync.Mutex
	overflowAcc    map[string]postgresWriteJob
	overflowState  map[string]postgresWriteJob
	overflowRuns   map[string]postgresWriteJob
	coalesced      atomic.Uint64
	guildExportGen atomic.Uint64
	closeOnce      sync.Once
	closed         bool
}

func NewPostgresStore(ctx context.Context, cfg PostgresConfig) (*PostgresStore, error) {
	if strings.TrimSpace(cfg.URL) == "" {
		return nil, errors.New("store: database URL ausente")
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
		return nil, fmt.Errorf("store: criar pool PostgreSQL: %w", err)
	}
	if err := pool.Ping(startupContext); err != nil {
		pool.Close()
		return nil, fmt.Errorf("store: conectar PostgreSQL: %w", err)
	}
	if err := applyPostgresSchema(startupContext, pool); err != nil {
		pool.Close()
		return nil, fmt.Errorf("store: aplicar schema PostgreSQL: %w", err)
	}
	s := &PostgresStore{
		pool:          pool,
		guildsTxtPath: cfg.GuildsTxtPath,
		writeQueue:    make(chan postgresWriteJob, postgresWriteQueueSize),
		overflowAcc:   make(map[string]postgresWriteJob),
		overflowState: make(map[string]postgresWriteJob),
		overflowRuns:  make(map[string]postgresWriteJob),
	}
	go s.persistLoop()
	return s, nil
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
	if _, err := conn.Exec(ctx, postgresSchema); err != nil {
		return err
	}
	var version int
	if err := conn.QueryRow(ctx, `SELECT COALESCE(MAX(version),0) FROM schema_migrations`).Scan(&version); err != nil {
		return err
	}
	if version != postgresSchemaVersion {
		return fmt.Errorf("schema v%d; binario exige v%d", version, postgresSchemaVersion)
	}
	return nil
}

func (s *PostgresStore) persistLoop() {
	var pending *postgresWriteJob
	for {
		var job postgresWriteJob
		var ok bool
		if pending != nil {
			job, pending = *pending, nil
			ok = true
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
			latest := map[string]*accountSnapshot{job.account.key: job.account}
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
					// Se o banco ficou atrasado e ha dois autosaves da mesma
					// conta, somente o snapshot mais novo precisa ser escrito.
					latest[next.account.key] = next.account
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
			snapshots := make([]*accountSnapshot, 0, len(latest))
			for _, snapshot := range latest {
				snapshots = append(snapshots, snapshot)
			}
			if err := s.saveSnapshots(snapshots); err != nil {
				metricPostgresFailures.Add(1)
				log.Printf("store PostgreSQL: lote de autosave (%d conta(s)): %v",
					len(snapshots), err)
			}
			continue
		}
		if job.charState != nil {
			latest := map[string]*postgresCharStateSnapshot{
				job.charState.uid: job.charState,
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
					latest[next.charState.uid] = next.charState
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
			snapshots := make([]*postgresCharStateSnapshot, 0, len(latest))
			for _, snapshot := range latest {
				snapshots = append(snapshots, snapshot)
			}
			if err := s.saveCharStateSnapshots(snapshots); err != nil {
				metricPostgresFailures.Add(1)
				log.Printf("store PostgreSQL: lote de charstate (%d personagem(ns)): %v",
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
		if next, ok := s.takeOverflow(); ok {
			pending = &next
		}
	}
}

func (s *PostgresStore) enqueue(job postgresWriteJob) error {
	if s.closed {
		return errors.New("store: PostgreSQL fechado")
	}
	select {
	case s.writeQueue <- job:
		s.updateAsyncMetrics()
		return nil
	default:
	}
	// A fila cheia nunca bloqueia o World. Mantemos somente o snapshot mais
	// novo de cada conta/personagem; tarefas derivadas coalescem pelo label.
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
		return errors.New("store: fila cheia para barreira de persistencia")
	}
	s.overflowMu.Unlock()
	s.updateAsyncMetrics()
	return nil
}

func (s *PostgresStore) takeOverflow() (postgresWriteJob, bool) {
	s.overflowMu.Lock()
	defer s.overflowMu.Unlock()
	for key, job := range s.overflowAcc {
		delete(s.overflowAcc, key)
		metricPostgresPending.Set(int64(len(s.overflowAcc) + len(s.overflowState) + len(s.overflowRuns)))
		return job, true
	}
	for key, job := range s.overflowState {
		delete(s.overflowState, key)
		metricPostgresPending.Set(int64(len(s.overflowAcc) + len(s.overflowState) + len(s.overflowRuns)))
		return job, true
	}
	for key, job := range s.overflowRuns {
		delete(s.overflowRuns, key)
		metricPostgresPending.Set(int64(len(s.overflowAcc) + len(s.overflowState) + len(s.overflowRuns)))
		return job, true
	}
	return postgresWriteJob{}, false
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
		s.mu.Lock()
		defer s.mu.Unlock()
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
		label: "exportar Guilds.txt",
		run: func() error {
			if s.guildExportGen.Load() != generation {
				return nil // um estado mais novo ja substituiu este artefato.
			}
			if err := writeFileAtomic(path, payload); err != nil {
				return fmt.Errorf("store: exportar Guilds.txt: %w", err)
			}
			return nil
		},
	})
}

// PostgresAsyncStats expoe backpressure sem cardinalidade por jogador. Pending
// mede apenas o overflow coalescido; len(writeQueue) continua sendo a fila FIFO.
func (s *PostgresStore) PostgresAsyncStats() (queued, pending int, coalesced uint64) {
	s.overflowMu.Lock()
	pending = len(s.overflowAcc) + len(s.overflowState) + len(s.overflowRuns)
	s.overflowMu.Unlock()
	return len(s.writeQueue), pending, s.coalesced.Load()
}

func (s *PostgresStore) flushLocked() {
	done := make(chan struct{})
	s.writeQueue <- postgresWriteJob{done: done}
	<-done
}

func (s *PostgresStore) Flush() {
	s.mu.Lock()
	defer s.mu.Unlock()
	if !s.closed {
		s.flushLocked()
	}
}

func (s *PostgresStore) Close() {
	s.closeOnce.Do(func() {
		s.mu.Lock()
		s.flushLocked()
		s.closed = true
		close(s.writeQueue)
		s.mu.Unlock()
		s.pool.Close()
	})
}

func snapshotAccount(acc *model.Account) (*accountSnapshot, error) {
	if acc == nil {
		return nil, errors.New("store: conta ausente")
	}
	seenCharacters := make(map[string]int, len(acc.Chars))
	for i := range acc.Chars {
		character := &acc.Chars[i]
		if character.Name == "" {
			continue
		}
		uid, err := model.NormalizeCharacterUID(character.UID)
		if err != nil {
			return nil, fmt.Errorf("store: conta %q personagem[%d]: %w", acc.Name, i, err)
		}
		if uid == "" {
			uid, err = model.NewCharacterUID()
			if err != nil {
				return nil, err
			}
		}
		if previous, exists := seenCharacters[uid]; exists {
			return nil, fmt.Errorf("store: UID de personagem %s repetido nos slots %d e %d",
				uid, previous, i)
		}
		character.UID = uid
		seenCharacters[uid] = i
	}
	if err := acc.Validate(); err != nil {
		return nil, fmt.Errorf("store: conta %q invalida: %w", acc.Name, err)
	}
	snapshot := &accountSnapshot{
		key:     strings.ToLower(acc.Name),
		display: acc.Name,
	}
	seenUID := make(map[string]string)
	for _, ref := range accountItemRefs(acc) {
		item := ref.item
		if item.Index == 0 {
			if item.UID != "" {
				return nil, fmt.Errorf("store: conta %q %s vazio possui UID", acc.Name, ref.location)
			}
			continue
		}
		uid, err := model.NormalizeItemUID(item.UID)
		if err != nil {
			return nil, fmt.Errorf("store: conta %q %s: %w", acc.Name, ref.location, err)
		}
		if uid == "" {
			uid, err = model.NewItemUID()
			if err != nil {
				return nil, err
			}
		}
		item.UID = uid
		if previous, duplicate := seenUID[uid]; duplicate {
			return nil, fmt.Errorf("store: UID duplicado %s em %s e %s", uid, previous, ref.location)
		}
		seenUID[uid] = ref.location
		rawUID, err := hex.DecodeString(uid)
		if err != nil {
			return nil, err // NormalizeItemUID ja torna este caminho defensivo.
		}
		var databaseUID [16]byte
		copy(databaseUID[:], rawUID)
		snapshot.items = append(snapshot.items, postgresItem{
			uid: uid, dbUID: pgtype.UUID{Bytes: databaseUID, Valid: true},
			location: ref.location, index: item.Index, effects: item.Eff,
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
			err = errors.New("UID vazio")
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
			return nil, fmt.Errorf("store: conta %q duplicada na transacao", snapshot.display)
		}
		seenAccounts[snapshot.key] = struct{}{}
		for _, item := range snapshot.items {
			if previous, duplicate := seenUID[item.uid]; duplicate {
				return nil, fmt.Errorf("store: UID duplicado %s em %s e %s/%s",
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

func postgresContext() (context.Context, context.CancelFunc) {
	return context.WithTimeout(context.Background(), postgresOperationTimeout)
}

func (s *PostgresStore) withSerializableTx(run func(context.Context, pgx.Tx) error) error {
	var last error
	for attempt := 0; attempt < 3; attempt++ {
		ctx, cancel := postgresContext()
		tx, err := s.pool.BeginTx(ctx, pgx.TxOptions{IsoLevel: pgx.Serializable})
		if err != nil {
			cancel()
			return err
		}
		err = run(ctx, tx)
		if err == nil {
			err = tx.Commit(ctx)
		} else {
			_ = tx.Rollback(ctx)
		}
		cancel()
		if !isRetryablePostgres(err) {
			return err
		}
		last = err
		time.Sleep(time.Duration(attempt+1) * 10 * time.Millisecond)
	}
	return fmt.Errorf("store: transacao PostgreSQL concorrente excedeu retries: %w", last)
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
	// Reconciliacao de personagens primeiro. Remover UIDs ausentes aciona o
	// ON DELETE CASCADE do charstate; os restantes sao atualizados por UID sem
	// depender do nickname ou da posicao atual na tela de selecao.
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
					return fmt.Errorf("store: UID de personagem %s pertence a outra conta",
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

	// Nomes sao reservas globais por conta, nao identidades de personagem:
	// Mortal e Arch homonimos compartilham uma unica reserva.
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
					return fmt.Errorf("store: personagem %q ja pertence a outra conta", name)
				}
			}
		}
	}

	// Ledger incremental de itens. Primeiro remove todas as linhas que deixaram
	// de ser identicas; so depois insere/atualiza as alteradas. Essa ordem
	// permite mover um UID entre contas/slots na mesma transacao sem colidir
	// com a constraint global nem regravar os milhares de itens inalterados.
	type storedItem struct {
		uid, account, location string
		index                  uint16
		effects                [6]byte
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
			SELECT replace(uid::text,'-',''),account_key,location,item_index,effects
			FROM item_instances WHERE account_key=ANY($1)`, keys)
		if err != nil {
			return err
		}
		for rows.Next() {
			var row storedItem
			var index int
			var effects []byte
			if err := rows.Scan(&row.uid, &row.account, &row.location, &index, &effects); err != nil {
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
			current.index == want.item.index && current.effects == want.item.effects {
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
			INSERT INTO item_instances(uid,account_key,location,item_index,effects)
			VALUES ($1,$2,$3,$4,$5)
			ON CONFLICT(uid) DO UPDATE SET
				location=EXCLUDED.location,
				item_index=EXCLUDED.item_index,
				effects=EXCLUDED.effects
			WHERE item_instances.account_key=EXCLUDED.account_key`,
			want.item.dbUID, want.account, want.item.location,
			int(want.item.index), want.item.effects[:])
		if err != nil {
			if isUniqueViolation(err) {
				return fmt.Errorf("store: UID de item duplicado %s: %w", uid, err)
			}
			return err
		}
		if tag.RowsAffected() != 1 {
			return fmt.Errorf("store: UID de item duplicado %s pertence a outra conta", uid)
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
	ctx, cancel := postgresContext()
	defer cancel()
	var payload []byte
	err := s.pool.QueryRow(ctx,
		`SELECT payload FROM accounts WHERE name_key=$1`, strings.ToLower(name)).Scan(&payload)
	if errors.Is(err, pgx.ErrNoRows) {
		return nil, fmt.Errorf("%w: conta %q", os.ErrNotExist, name)
	}
	if err != nil {
		return nil, err
	}
	var acc model.Account
	if err := decodeAccountJSON(payload, &acc); err != nil {
		return nil, fmt.Errorf("store: conta %q corrompida: %w", name, err)
	}
	if err := acc.Validate(); err != nil {
		return nil, fmt.Errorf("store: conta %q invalida: %w", name, err)
	}
	return &acc, nil
}

func (s *PostgresStore) saveSnapshots(snapshots []*accountSnapshot) error {
	return s.withSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		return persistAccountSnapshots(ctx, tx, snapshots, false)
	})
}

func (s *PostgresStore) SaveAccount(acc *model.Account) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	snapshot, err := snapshotAccount(acc)
	if err != nil {
		return err
	}
	s.flushLocked()
	return s.saveSnapshots([]*accountSnapshot{snapshot})
}

func (s *PostgresStore) SaveAccountAsync(acc *model.Account) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	snapshot, err := snapshotAccount(acc)
	if err != nil {
		return err
	}
	return s.enqueue(postgresWriteJob{
		label:   "autosave da conta " + snapshot.display,
		account: snapshot,
	})
}

func (s *PostgresStore) SaveAccounts(accounts ...*model.Account) error {
	return s.SaveGameState(nil, accounts...)
}

func (s *PostgresStore) CreateAccount(acc *model.Account) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	snapshot, err := snapshotAccount(acc)
	if err != nil {
		return err
	}
	s.flushLocked()
	err = s.withSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		return persistAccountSnapshots(ctx, tx, []*accountSnapshot{snapshot}, true)
	})
	if isUniqueViolation(err) {
		return ErrAccountExists
	}
	return err
}

func (s *PostgresStore) AccountNameExists(name string) (bool, error) {
	ctx, cancel := postgresContext()
	defer cancel()
	var exists bool
	err := s.pool.QueryRow(ctx,
		`SELECT EXISTS(SELECT 1 FROM accounts WHERE name_key=$1)`,
		strings.ToLower(name)).Scan(&exists)
	return exists, err
}

func (s *PostgresStore) CharacterNameExists(name string) (bool, error) {
	ctx, cancel := postgresContext()
	defer cancel()
	var exists bool
	err := s.pool.QueryRow(ctx,
		`SELECT EXISTS(SELECT 1 FROM character_names WHERE name_key=$1)`,
		strings.ToLower(name)).Scan(&exists)
	return exists, err
}

func (s *PostgresStore) CharacterNames() (map[string]struct{}, error) {
	ctx, cancel := postgresContext()
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
	ctx, cancel := postgresContext()
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
		return nil, fmt.Errorf("store: guild state corrompido: %w", err)
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		return nil, errors.New("store: guild state possui JSON adicional")
	}
	if err := registry.Validate(); err != nil {
		return nil, err
	}
	return &registry, nil
}

func (s *PostgresStore) SaveGameState(guilds *model.GuildRegistry, accounts ...*model.Account) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	if guilds == nil && len(accounts) == 0 {
		return nil
	}
	var guildPayload []byte
	var err error
	if guilds != nil {
		if err := guilds.Validate(); err != nil {
			return fmt.Errorf("store: guilds invalidas: %w", err)
		}
		guildPayload, err = json.Marshal(guilds)
		if err != nil {
			return err
		}
	}
	snapshots, err := snapshotAccounts(accounts...)
	if err != nil {
		return err
	}
	s.flushLocked()
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
			return err
		}
		return nil
	})
	if err != nil {
		return err
	}
	if guilds != nil && s.guildsTxtPath != "" {
		// Guilds.txt e artefato derivado: o commit PostgreSQL acima ja e a
		// verdade. Falha de exportacao entra em retry e jamais volta ao handler
		// como se a transacao autoritativa tivesse falhado.
		s.scheduleGuildExport(guilds)
	}
	return nil
}

// SavePlayerState confirma conta, charstate e opcionalmente guild no mesmo
// commit serializavel. E a fronteira usada por crafts/quests que gastam itens e
// contadores por personagem: nao existe janela em que apenas metade sobreviva.
func (s *PostgresStore) SavePlayerState(guilds *model.GuildRegistry, account *model.Account,
	characterUID string, state *model.CharState) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	snapshots, err := snapshotAccounts(account)
	if err != nil {
		return err
	}
	statePayload, err := charStatePayload(characterUID, state)
	if err != nil {
		return err
	}
	stateUID, err := postgresUUID(characterUID)
	if err != nil {
		return err
	}
	var guildPayload []byte
	if guilds != nil {
		if err := guilds.Validate(); err != nil {
			return fmt.Errorf("store: guilds invalidas: %w", err)
		}
		guildPayload, err = json.Marshal(guilds)
		if err != nil {
			return err
		}
	}
	s.flushLocked()
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
	normalized, err := model.NormalizeCharacterUID(uid)
	if err != nil {
		return nil, fmt.Errorf("store: charstate: %w", err)
	}
	if normalized == "" {
		return nil, errors.New("store: charstate sem UID de personagem")
	}
	if emptyCharState(state) {
		return nil, nil
	}
	clone := *state
	clone.Version = model.CharStateVersion
	return json.Marshal(&clone)
}

func (s *PostgresStore) saveCharStatePayload(uid string, payload []byte) error {
	ctx, cancel := postgresContext()
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
	s.mu.Lock()
	defer s.mu.Unlock()
	payload, err := charStatePayload(uid, state)
	if err != nil {
		return err
	}
	s.flushLocked()
	return s.saveCharStatePayload(uid, payload)
}

func (s *PostgresStore) SaveCharStateAsync(uid string, state *model.CharState) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	payload, err := charStatePayload(uid, state)
	if err != nil {
		return err
	}
	return s.enqueue(postgresWriteJob{
		label:     "autosave do charstate " + uid,
		charState: &postgresCharStateSnapshot{uid: uid, payload: payload},
	})
}

func (s *PostgresStore) LoadCharState(uid string) (*model.CharState, error) {
	ctx, cancel := postgresContext()
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
		return nil, errors.New("store: charstate possui JSON adicional")
	}
	if state.Version != model.CharStateVersion {
		return nil, fmt.Errorf("store: charstate %q versao %d; esperado %d",
			uid, state.Version, model.CharStateVersion)
	}
	return &state, nil
}
