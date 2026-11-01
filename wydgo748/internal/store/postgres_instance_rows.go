package store

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"maps"
	"reflect"
	"slices"
	"strings"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
)

// Instance participant rows (DOCS/server/server-correction-plan.md, W8).
// Each runtime instance is one row keyed by channel and runtime ID; each
// channel has one row for its globals (Nightmare run counters and the Gamble
// jackpot and pool). A write commits only the rows it declares, so writes of
// different runtimes do not share a row. The former singleton row stays
// current through an export after each commit and is the rollback source for
// an older binary.

const postgresInstanceRowsSchema = `
CREATE TABLE IF NOT EXISTS instance_runtime_state (
    channel     smallint NOT NULL CHECK (channel BETWEEN 1 AND 255),
    runtime_id  text NOT NULL CHECK (length(runtime_id) BETWEEN 1 AND 160),
    payload     jsonb NOT NULL,
    version     bigint NOT NULL DEFAULT 1,
    updated_at  timestamptz NOT NULL DEFAULT now(),
    PRIMARY KEY (channel, runtime_id)
);
CREATE TABLE IF NOT EXISTS instance_channel_state (
    channel     smallint PRIMARY KEY CHECK (channel BETWEEN 1 AND 255),
    payload     jsonb NOT NULL,
    version     bigint NOT NULL DEFAULT 1,
    updated_at  timestamptz NOT NULL DEFAULT now()
);`

// InstanceParticipant names one durable instance row. An empty RuntimeID
// names the channel's globals row.
type InstanceParticipant struct {
	Channel   byte
	RuntimeID string
}

type instanceChannelGlobals struct {
	NightmarePartyRuns map[string]int `json:"nightmarePartyRuns,omitempty"`
	GambleJackpot      uint32         `json:"gambleJackpot,omitempty"`
	GamblePool         uint64         `json:"gamblePool,omitempty"`
}

// instanceChannels maps the snapshot root to channels: the top level is
// channel 1 and Channels holds every other channel.
func instanceChannels(state *model.InstanceStateSnapshot) map[byte]*model.InstanceStateSnapshot {
	channels := make(map[byte]*model.InstanceStateSnapshot, 1+len(state.Channels))
	channels[1] = state
	for channel, snapshot := range state.Channels {
		if channel != 1 && snapshot != nil {
			channels[channel] = snapshot
		}
	}
	return channels
}

func channelGlobals(snapshot *model.InstanceStateSnapshot) instanceChannelGlobals {
	globals := instanceChannelGlobals{GambleJackpot: snapshot.GambleJackpot, GamblePool: snapshot.GamblePool}
	if len(snapshot.NightmarePartyRuns) > 0 {
		globals.NightmarePartyRuns = maps.Clone(snapshot.NightmarePartyRuns)
	}
	return globals
}

func validInstanceRuntimeID(id string) bool {
	return id != "" && len(id) <= 160 && strings.TrimSpace(id) == id
}

// writeInstanceRows commits the declared participants of state. Nil keys
// write every channel present in state and delete runtimes absent from it.
func writeInstanceRows(ctx context.Context, tx pgx.Tx, state *model.InstanceStateSnapshot, keys []InstanceParticipant) error {
	if state == nil || state.Version != model.InstanceStateVersion {
		return errors.New("store: instance rows need a current snapshot")
	}
	channels := instanceChannels(state)
	if keys == nil {
		for channel, snapshot := range channels {
			keys = append(keys, InstanceParticipant{Channel: channel})
			for _, runtime := range snapshot.Instances {
				keys = append(keys, InstanceParticipant{Channel: channel, RuntimeID: runtime.RuntimeID})
			}
			present := make([]string, 0, len(snapshot.Instances))
			for _, runtime := range snapshot.Instances {
				present = append(present, runtime.RuntimeID)
			}
			if _, err := tx.Exec(ctx, `DELETE FROM instance_runtime_state
				WHERE channel=$1 AND NOT (runtime_id = ANY($2::text[]))`, int16(channel), present); err != nil {
				return err
			}
		}
	}
	slices.SortFunc(keys, func(a, b InstanceParticipant) int {
		if a.Channel != b.Channel {
			return int(a.Channel) - int(b.Channel)
		}
		return strings.Compare(a.RuntimeID, b.RuntimeID)
	})
	for _, key := range slices.Compact(keys) {
		snapshot := channels[key.Channel]
		if key.Channel == 0 {
			return errors.New("store: instance participant without a channel")
		}
		if key.RuntimeID == "" {
			globals := instanceChannelGlobals{}
			if snapshot != nil {
				globals = channelGlobals(snapshot)
			}
			payload, err := json.Marshal(globals)
			if err != nil {
				return err
			}
			if _, err := tx.Exec(ctx, `INSERT INTO instance_channel_state(channel,payload) VALUES($1,$2)
				ON CONFLICT(channel) DO UPDATE SET payload=EXCLUDED.payload,
					version=instance_channel_state.version+1,updated_at=now()`, int16(key.Channel), payload); err != nil {
				return err
			}
			continue
		}
		if !validInstanceRuntimeID(key.RuntimeID) {
			return fmt.Errorf("store: invalid instance runtime ID %q", key.RuntimeID)
		}
		var runtime *model.InstanceRuntimeState
		if snapshot != nil {
			for i := range snapshot.Instances {
				if snapshot.Instances[i].RuntimeID == key.RuntimeID {
					runtime = &snapshot.Instances[i]
					break
				}
			}
		}
		if runtime == nil {
			if _, err := tx.Exec(ctx, `DELETE FROM instance_runtime_state WHERE channel=$1 AND runtime_id=$2`,
				int16(key.Channel), key.RuntimeID); err != nil {
				return err
			}
			continue
		}
		payload, err := json.Marshal(runtime)
		if err != nil {
			return err
		}
		if _, err := tx.Exec(ctx, `INSERT INTO instance_runtime_state(channel,runtime_id,payload) VALUES($1,$2,$3)
			ON CONFLICT(channel,runtime_id) DO UPDATE SET payload=EXCLUDED.payload,
				version=instance_runtime_state.version+1,updated_at=now()`,
			int16(key.Channel), key.RuntimeID, payload); err != nil {
			return err
		}
	}
	return nil
}

type instanceRowQuerier interface {
	Query(context.Context, string, ...any) (pgx.Rows, error)
}

// loadInstanceRows rebuilds the snapshot root from participant rows.
func loadInstanceRows(ctx context.Context, q instanceRowQuerier) (*model.InstanceStateSnapshot, error) {
	root := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}
	channel := func(id byte) *model.InstanceStateSnapshot {
		if id == 1 {
			return root
		}
		if root.Channels == nil {
			root.Channels = make(map[byte]*model.InstanceStateSnapshot)
		}
		if root.Channels[id] == nil {
			root.Channels[id] = &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}
		}
		return root.Channels[id]
	}
	rows, err := q.Query(ctx, `SELECT channel,payload FROM instance_channel_state ORDER BY channel`)
	if err != nil {
		return nil, err
	}
	for rows.Next() {
		var id int16
		var payload []byte
		if err := rows.Scan(&id, &payload); err != nil {
			rows.Close()
			return nil, err
		}
		var globals instanceChannelGlobals
		if err := json.Unmarshal(payload, &globals); err != nil {
			rows.Close()
			return nil, fmt.Errorf("store: corrupt instance channel %d: %w", id, err)
		}
		target := channel(byte(id))
		target.NightmarePartyRuns = globals.NightmarePartyRuns
		target.GambleJackpot, target.GamblePool = globals.GambleJackpot, globals.GamblePool
	}
	rows.Close()
	if err := rows.Err(); err != nil {
		return nil, err
	}
	rows, err = q.Query(ctx, `SELECT channel,runtime_id,payload FROM instance_runtime_state ORDER BY channel,runtime_id`)
	if err != nil {
		return nil, err
	}
	defer rows.Close()
	for rows.Next() {
		var id int16
		var runtimeID string
		var payload []byte
		if err := rows.Scan(&id, &runtimeID, &payload); err != nil {
			return nil, err
		}
		var runtime model.InstanceRuntimeState
		if err := json.Unmarshal(payload, &runtime); err != nil {
			return nil, fmt.Errorf("store: corrupt instance %d/%s: %w", id, runtimeID, err)
		}
		if runtime.RuntimeID != runtimeID {
			return nil, fmt.Errorf("store: instance row %d/%s holds runtime %q", id, runtimeID, runtime.RuntimeID)
		}
		target := channel(byte(id))
		target.Instances = append(target.Instances, runtime)
	}
	return root, rows.Err()
}

// normalizeInstanceState gives equal snapshots one representation: sorted
// runtimes, nil empty collections and no empty channels.
func normalizeInstanceState(state *model.InstanceStateSnapshot) *model.InstanceStateSnapshot {
	if state == nil {
		return &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}
	}
	normalizeOne := func(s *model.InstanceStateSnapshot) *model.InstanceStateSnapshot {
		out := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion,
			GambleJackpot: s.GambleJackpot, GamblePool: s.GamblePool}
		if len(s.NightmarePartyRuns) > 0 {
			out.NightmarePartyRuns = maps.Clone(s.NightmarePartyRuns)
		}
		if len(s.Instances) > 0 {
			out.Instances = slices.Clone(s.Instances)
			slices.SortFunc(out.Instances, func(a, b model.InstanceRuntimeState) int {
				return strings.Compare(a.RuntimeID, b.RuntimeID)
			})
		}
		return out
	}
	root := normalizeOne(state)
	for channel, snapshot := range state.Channels {
		if channel == 1 || snapshot == nil {
			continue
		}
		one := normalizeOne(snapshot)
		if one.Instances == nil && one.NightmarePartyRuns == nil && one.GambleJackpot == 0 && one.GamblePool == 0 {
			continue
		}
		if root.Channels == nil {
			root.Channels = make(map[byte]*model.InstanceStateSnapshot)
		}
		root.Channels[channel] = one
	}
	return root
}

// equalInstanceStates compares through JSON so time values compare by the
// instant they persist, not by monotonic readings or location pointers.
func equalInstanceStates(a, b *model.InstanceStateSnapshot) bool {
	left, errA := json.Marshal(normalizeInstanceState(a))
	right, errB := json.Marshal(normalizeInstanceState(b))
	if errA != nil || errB != nil {
		return false
	}
	var x, y any
	return json.Unmarshal(left, &x) == nil && json.Unmarshal(right, &y) == nil && reflect.DeepEqual(x, y)
}

// migrateInstanceRows splits the singleton payload into participant rows in
// one transaction and requires the rebuilt aggregate to equal the original.
// The singleton row is kept unchanged as the rollback source.
func migrateInstanceRows(ctx context.Context, tx pgx.Tx) error {
	if _, err := tx.Exec(ctx, postgresInstanceRowsSchema); err != nil {
		return err
	}
	var existing int
	if err := tx.QueryRow(ctx, `SELECT (SELECT count(*) FROM instance_runtime_state) +
		(SELECT count(*) FROM instance_channel_state)`).Scan(&existing); err != nil {
		return err
	}
	var payload []byte
	err := tx.QueryRow(ctx, `SELECT payload FROM instance_state WHERE singleton=true`).Scan(&payload)
	if errors.Is(err, pgx.ErrNoRows) {
		return nil
	}
	if err != nil {
		return err
	}
	if existing > 0 {
		return errors.New("store: instance rows exist before the v15 migration; refusing to merge")
	}
	var original model.InstanceStateSnapshot
	if err := json.Unmarshal(payload, &original); err != nil {
		return fmt.Errorf("store: corrupt instance state before migration: %w", err)
	}
	if original.Version != model.InstanceStateVersion {
		return fmt.Errorf("store: instance state version %d; expected %d", original.Version, model.InstanceStateVersion)
	}
	if err := writeInstanceRows(ctx, tx, &original, nil); err != nil {
		return err
	}
	rebuilt, err := loadInstanceRows(ctx, tx)
	if err != nil {
		return err
	}
	if !equalInstanceStates(&original, rebuilt) {
		return errors.New("store: instance rows do not rebuild the singleton state; migration refused")
	}
	return nil
}

// scheduleInstanceExport keeps the singleton row current for rollback. The
// newest request wins; an export never blocks or fails a gameplay write.
func (s *PostgresStore) scheduleInstanceExport() {
	generation := s.instanceExportGen.Add(1)
	_ = s.enqueue(postgresWriteJob{
		label: "export instance rollback state",
		run: func() error {
			if s.instanceExportGen.Load() != generation {
				return nil
			}
			return s.exportInstanceSingleton()
		},
	})
}

func (s *PostgresStore) exportInstanceSingleton() error {
	ctx, cancel := s.postgresContext()
	defer cancel()
	tx, err := s.pool.BeginTx(ctx, pgx.TxOptions{IsoLevel: pgx.RepeatableRead})
	if err != nil {
		return err
	}
	defer func() { _ = tx.Rollback(context.Background()) }()
	state, err := loadInstanceRows(ctx, tx)
	if err != nil {
		return err
	}
	payload, err := json.Marshal(normalizeInstanceState(state))
	if err != nil {
		return err
	}
	if _, err := tx.Exec(ctx, `INSERT INTO instance_state(singleton,payload) VALUES(true,$1)
		ON CONFLICT(singleton) DO UPDATE SET payload=EXCLUDED.payload,
			version=instance_state.version+1,updated_at=now()`, payload); err != nil {
		return err
	}
	return tx.Commit(ctx)
}
