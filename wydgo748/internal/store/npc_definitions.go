package store

import (
	"context"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"reflect"
	"sort"
	"strings"
	"time"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
)

var ErrNPCDefinitionStale = errors.New("store: NPC definition version is stale")

type NPCDefinitionSeed struct {
	Definition model.NPCDef
	Active     bool
}

type NPCDefinitionRecord struct {
	Channel    uint8
	Name       string
	Version    uint32
	Active     bool
	Overridden bool
	Definition json.RawMessage
	Source     json.RawMessage
	UpdatedBy  string
	UpdatedAt  time.Time
	ChangeID   int64
}

type NPCDefinitionStore interface {
	SyncNPCDefinitions(context.Context, uint8, []NPCDefinitionSeed) error
	LoadNPCDefinitions(context.Context, uint8) ([]NPCDefinitionRecord, error)
	LoadNPCDefinition(context.Context, uint8, string) (NPCDefinitionRecord, error)
	SaveNPCDefinition(context.Context, uint8, string, uint32, json.RawMessage, bool, string, string) (NPCDefinitionRecord, error)
	NPCDefinitionChangeCursor(context.Context, uint8) (int64, error)
	LoadNPCDefinitionChanges(context.Context, uint8, int64) ([]NPCDefinitionRecord, error)
}

type NPCDefinitionUpdate struct {
	Channel            uint8
	Name               string
	ExpectedVersion    uint32
	Definition         json.RawMessage
	RestoreSource      bool
	ExpectedSourceHash string
}

// NPCDefinitionBatchStore commits definitions and audit entries together.
type NPCDefinitionBatchStore interface {
	SaveNPCDefinitions(context.Context, []NPCDefinitionUpdate, string, string) ([]NPCDefinitionRecord, error)
}

func npcDefinitionSourceHash(raw []byte) string {
	sum := sha256.Sum256(raw)
	return hex.EncodeToString(sum[:])
}

// Serialize revision allocation through commit, independently for each channel.
// Every writer must hold this lock before changing rows or allocating an ID.
func lockNPCDefinitions(ctx context.Context, tx pgx.Tx, channel uint8) error {
	_, err := tx.Exec(ctx, `SELECT pg_advisory_xact_lock(748015, $1::integer)`, int(channel))
	return err
}

func (s *PostgresStore) SyncNPCDefinitions(ctx context.Context, channel uint8, seeds []NPCDefinitionSeed) error {
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	if channel == 0 {
		return errors.New("store: invalid NPC definition channel")
	}
	type row struct {
		Name       string          `json:"name"`
		Definition json.RawMessage `json:"definition"`
		Effective  json.RawMessage `json:"effective"`
		SourceHash string          `json:"source_hash"`
		Active     bool            `json:"active"`
		Changed    bool            `json:"changed"`
		Overridden bool            `json:"overridden"`
	}
	rows := make([]row, 0, len(seeds))
	seen := make(map[string]bool, len(seeds))
	for _, seed := range seeds {
		name := strings.TrimSpace(seed.Definition.Name)
		key := name
		if name == "" || len(name) > 64 || seen[key] {
			return fmt.Errorf("store: invalid or duplicate NPC definition %q", name)
		}
		seen[key] = true
		raw, err := json.Marshal(seed.Definition)
		if err != nil {
			return fmt.Errorf("store: encode NPC definition %q: %w", name, err)
		}
		rows = append(rows, row{Name: name, Definition: raw, SourceHash: npcDefinitionSourceHash(raw), Active: seed.Active})
	}
	tx, err := s.pool.BeginTx(ctx, pgx.TxOptions{})
	if err != nil {
		return err
	}
	defer tx.Rollback(ctx)
	if err := lockNPCDefinitions(ctx, tx, channel); err != nil {
		return err
	}
	type previousDefinition struct {
		definition model.NPCDef
		overridden bool
		document   any
		raw        json.RawMessage
	}
	previous := make(map[string]previousDefinition)
	stored, err := tx.Query(ctx, `SELECT npc_name, definition_json, overridden FROM npc_definitions WHERE channel_id=$1 FOR UPDATE`, int16(channel))
	if err != nil {
		return err
	}
	for stored.Next() {
		var name string
		var raw []byte
		var entry previousDefinition
		if err := stored.Scan(&name, &raw, &entry.overridden); err != nil {
			stored.Close()
			return err
		}
		if err := json.Unmarshal(raw, &entry.definition); err != nil {
			stored.Close()
			return fmt.Errorf("store: decode saved NPC %q: %w", name, err)
		}
		if err := json.Unmarshal(raw, &entry.document); err != nil {
			stored.Close()
			return err
		}
		entry.raw = append(json.RawMessage(nil), raw...)
		previous[name] = entry
	}
	err = stored.Err()
	stored.Close()
	if err != nil {
		return err
	}
	for i := range rows {
		effective := seeds[i].Definition
		if old, exists := previous[rows[i].Name]; exists {
			rows[i].Overridden = old.overridden
			if old.overridden {
				effective = old.definition.WithSourceSelectors(effective)
			}
		}
		rows[i].Effective, err = json.Marshal(effective)
		if err != nil {
			return err
		}
		if old, exists := previous[rows[i].Name]; exists {
			// Preserve legacy JSON spelling and omitted defaults when no selector
			// changed. Canonicalizing a staff override must not create a new edit.
			if old.overridden && reflect.DeepEqual(old.definition, effective) {
				rows[i].Effective = old.raw
			}
			var document any
			if err := json.Unmarshal(rows[i].Effective, &document); err != nil {
				return err
			}
			rows[i].Changed = !reflect.DeepEqual(old.document, document)
		}
	}
	payload, err := json.Marshal(rows)
	if err != nil {
		return err
	}
	if _, err := tx.Exec(ctx, `UPDATE npc_definitions SET active=false WHERE channel_id=$1`, int16(channel)); err != nil {
		return err
	}
	_, err = tx.Exec(ctx, `
		INSERT INTO npc_definitions(channel_id, npc_key, npc_name, source_json, definition_json,
			source_hash, active, version, overridden, updated_by, updated_at, reason)
		SELECT $1, x.name, x.name, x.definition, x.effective, x.source_hash,
			x.active, 1, x.overridden, 'source-sync', now(), 'Imported source definition'
		FROM jsonb_to_recordset($2::jsonb) AS x(name text, definition jsonb, effective jsonb, source_hash text, active boolean, overridden boolean)
		ON CONFLICT (channel_id, npc_key) DO UPDATE SET
			npc_name=EXCLUDED.npc_name,
			source_json=EXCLUDED.source_json,
			source_hash=EXCLUDED.source_hash,
			definition_json=EXCLUDED.definition_json,
			active=EXCLUDED.active,
			version=CASE WHEN npc_definitions.definition_json<>EXCLUDED.definition_json
				THEN npc_definitions.version+1 ELSE npc_definitions.version END,
			updated_by=CASE WHEN npc_definitions.definition_json<>EXCLUDED.definition_json
				THEN 'source-sync' ELSE npc_definitions.updated_by END,
			updated_at=CASE WHEN npc_definitions.definition_json<>EXCLUDED.definition_json
				THEN now() ELSE npc_definitions.updated_at END,
			reason=CASE WHEN npc_definitions.definition_json<>EXCLUDED.definition_json
				THEN 'Source definition changed' ELSE npc_definitions.reason END`, int16(channel), payload)
	if err != nil {
		return err
	}
	_, err = tx.Exec(ctx, `INSERT INTO npc_definition_revisions(channel_id, npc_key, npc_name, version,
		definition_json, actor, reason, restored_source)
		SELECT d.channel_id, d.npc_key, d.npc_name, d.version, d.definition_json, 'source-sync',
			'Source selectors or definition changed', NOT d.overridden
		FROM npc_definitions d JOIN jsonb_to_recordset($2::jsonb) AS x(name text, changed boolean)
		ON d.npc_key=x.name WHERE d.channel_id=$1 AND x.changed`, int16(channel), payload)
	if err != nil {
		return err
	}
	return tx.Commit(ctx)
}

func (s *PostgresStore) LoadNPCDefinitions(ctx context.Context, channel uint8) ([]NPCDefinitionRecord, error) {
	rows, err := s.pool.Query(ctx, `
		SELECT channel_id, npc_name, version, active, overridden, definition_json, source_json, updated_by, updated_at
		FROM npc_definitions WHERE channel_id=$1 AND active=true ORDER BY npc_name`, int16(channel))
	if err != nil {
		return nil, err
	}
	defer rows.Close()
	out := make([]NPCDefinitionRecord, 0)
	for rows.Next() {
		var entry NPCDefinitionRecord
		var channelID, version int64
		if err := rows.Scan(&channelID, &entry.Name, &version, &entry.Active, &entry.Overridden,
			&entry.Definition, &entry.Source, &entry.UpdatedBy, &entry.UpdatedAt); err != nil {
			return nil, err
		}
		if channelID < 1 || channelID > 255 || version < 1 || version > int64(^uint32(0)) {
			return nil, fmt.Errorf("store: NPC definition %q has out-of-range identity or version", entry.Name)
		}
		entry.Channel, entry.Version = uint8(channelID), uint32(version)
		out = append(out, entry)
	}
	return out, rows.Err()
}

func (s *PostgresStore) LoadNPCDefinition(ctx context.Context, channel uint8, name string) (NPCDefinitionRecord, error) {
	var entry NPCDefinitionRecord
	var channelID, version int64
	err := s.pool.QueryRow(ctx, `
		SELECT channel_id, npc_name, version, active, overridden, definition_json, source_json, updated_by, updated_at
		FROM npc_definitions WHERE channel_id=$1 AND npc_key=$2`, int16(channel), name).Scan(
		&channelID, &entry.Name, &version, &entry.Active, &entry.Overridden,
		&entry.Definition, &entry.Source, &entry.UpdatedBy, &entry.UpdatedAt)
	if err != nil {
		return NPCDefinitionRecord{}, err
	}
	if channelID < 1 || channelID > 255 || version < 1 || version > int64(^uint32(0)) {
		return NPCDefinitionRecord{}, errors.New("store: NPC definition identity or version out of range")
	}
	entry.Channel, entry.Version = uint8(channelID), uint32(version)
	return entry, nil
}

func (s *PostgresStore) SaveNPCDefinition(ctx context.Context, channel uint8, name string, expected uint32, definition json.RawMessage,
	restoreSource bool, actor, reason string) (NPCDefinitionRecord, error) {
	entries, err := s.SaveNPCDefinitions(ctx, []NPCDefinitionUpdate{{Channel: channel, Name: name,
		ExpectedVersion: expected, Definition: definition, RestoreSource: restoreSource}}, actor, reason)
	if err != nil {
		return NPCDefinitionRecord{}, err
	}
	return entries[0], nil
}

// SaveNPCDefinitions locks channels in ascending order and rolls back every
// definition and revision if any precondition or write fails.
func (s *PostgresStore) SaveNPCDefinitions(ctx context.Context, updates []NPCDefinitionUpdate, actor, reason string) ([]NPCDefinitionRecord, error) {
	if s.readOnly {
		return nil, errors.New("store: PostgreSQL is read-only")
	}
	reason = strings.TrimSpace(reason)
	if len(updates) == 0 || len(updates) > 255 || actor == "" || len(actor) > 128 || len([]rune(reason)) < 3 || len([]rune(reason)) > 200 {
		return nil, errors.New("store: invalid NPC definition batch or audit metadata")
	}
	changes := append([]NPCDefinitionUpdate(nil), updates...)
	for i := range changes {
		changes[i].Name = strings.TrimSpace(changes[i].Name)
	}
	sort.Slice(changes, func(i, j int) bool {
		if changes[i].Channel == changes[j].Channel {
			return changes[i].Name < changes[j].Name
		}
		return changes[i].Channel < changes[j].Channel
	})
	for i := range changes {
		change := &changes[i]
		if change.Channel == 0 || change.Name == "" || len(change.Name) > 64 || change.ExpectedVersion == 0 || change.ExpectedVersion == ^uint32(0) ||
			(i > 0 && changes[i-1].Channel == change.Channel && changes[i-1].Name == change.Name) {
			return nil, errors.New("store: invalid NPC definition update")
		}
		if change.RestoreSource {
			if len(change.Definition) != 0 {
				return nil, errors.New("store: restore cannot include a definition")
			}
		} else {
			var identity struct {
				Name string `json:"name"`
			}
			if len(change.Definition) > 64*1024 || json.Unmarshal(change.Definition, &identity) != nil || identity.Name != change.Name {
				return nil, errors.New("store: NPC definition name does not match its key")
			}
		}
	}
	tx, err := s.pool.BeginTx(ctx, pgx.TxOptions{})
	if err != nil {
		return nil, err
	}
	defer tx.Rollback(ctx)
	var lockedChannel uint8
	for _, change := range changes {
		if change.Channel != lockedChannel {
			if err := lockNPCDefinitions(ctx, tx, change.Channel); err != nil {
				return nil, err
			}
			lockedChannel = change.Channel
		}
	}
	// Validate all versions before allocating revisions or changing rows.
	for i := range changes {
		change := &changes[i]
		var source []byte
		var sourceHash string
		var currentVersion int64
		err := tx.QueryRow(ctx, `SELECT source_json, version, source_hash FROM npc_definitions
			WHERE channel_id=$1 AND npc_key=$2 AND active=true FOR UPDATE`, int16(change.Channel), change.Name).
			Scan(&source, &currentVersion, &sourceHash)
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrNPCDefinitionStale
		}
		if err != nil {
			return nil, err
		}
		if currentVersion != int64(change.ExpectedVersion) || change.ExpectedSourceHash != "" && change.ExpectedSourceHash != sourceHash {
			return nil, ErrNPCDefinitionStale
		}
		if change.RestoreSource {
			change.Definition = json.RawMessage(source)
		}
	}
	entries := make([]NPCDefinitionRecord, 0, len(changes))
	for _, change := range changes {
		var entry NPCDefinitionRecord
		var channelID, version int64
		err = tx.QueryRow(ctx, `UPDATE npc_definitions SET definition_json=$1::jsonb, version=version+1,
		overridden=$2, updated_by=$3, updated_at=now(), reason=$4
		WHERE channel_id=$5 AND npc_key=$6 AND version=$7 AND active=true
		RETURNING channel_id, npc_name, version, active, overridden, definition_json, source_json, updated_by, updated_at`,
			[]byte(change.Definition), !change.RestoreSource, actor, reason, int16(change.Channel), change.Name, int64(change.ExpectedVersion)).Scan(
			&channelID, &entry.Name, &version, &entry.Active, &entry.Overridden,
			&entry.Definition, &entry.Source, &entry.UpdatedBy, &entry.UpdatedAt)
		if err != nil {
			if errors.Is(err, pgx.ErrNoRows) {
				return nil, ErrNPCDefinitionStale
			}
			return nil, err
		}
		entry.Channel, entry.Version = uint8(channelID), uint32(version)
		err = tx.QueryRow(ctx, `INSERT INTO npc_definition_revisions(channel_id, npc_key, npc_name, version,
		definition_json, actor, reason, restored_source)
		VALUES ($1, $2, $2, $3, $4::jsonb, $5, $6, $7) RETURNING id`,
			int16(change.Channel), change.Name, version, []byte(change.Definition), actor, reason, change.RestoreSource).Scan(&entry.ChangeID)
		if err != nil {
			return nil, err
		}
		entries = append(entries, entry)
	}
	if err := tx.Commit(ctx); err != nil {
		return nil, err
	}
	return entries, nil
}

func (s *PostgresStore) NPCDefinitionChangeCursor(ctx context.Context, channel uint8) (int64, error) {
	var id int64
	err := s.pool.QueryRow(ctx, `SELECT COALESCE(MAX(id),0) FROM npc_definition_revisions WHERE channel_id=$1`, int16(channel)).Scan(&id)
	return id, err
}

func (s *PostgresStore) LoadNPCDefinitionChanges(ctx context.Context, channel uint8, after int64) ([]NPCDefinitionRecord, error) {
	rows, err := s.pool.Query(ctx, `SELECT id, channel_id, npc_name, version, definition_json, actor, created_at
		FROM npc_definition_revisions WHERE channel_id=$1 AND id>$2 ORDER BY id LIMIT 100`, int16(channel), after)
	if err != nil {
		return nil, err
	}
	defer rows.Close()
	out := make([]NPCDefinitionRecord, 0)
	for rows.Next() {
		var entry NPCDefinitionRecord
		var channelID, version int64
		if err := rows.Scan(&entry.ChangeID, &channelID, &entry.Name, &version, &entry.Definition, &entry.UpdatedBy, &entry.UpdatedAt); err != nil {
			return nil, err
		}
		if channelID < 1 || channelID > 255 || version < 1 || version > int64(^uint32(0)) {
			return nil, fmt.Errorf("store: NPC revision %q has out-of-range identity or version", entry.Name)
		}
		entry.Channel, entry.Version, entry.Active, entry.Overridden = uint8(channelID), uint32(version), true, true
		out = append(out, entry)
	}
	return out, rows.Err()
}

var _ NPCDefinitionStore = (*PostgresStore)(nil)
var _ NPCDefinitionBatchStore = (*PostgresStore)(nil)
