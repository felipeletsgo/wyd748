package store

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgtype"
	"wydgo/internal/model"
)

// SaveCriticalGameState commits the optional global aggregates, owner sidecar
// and participant accounts as one certified transaction. An empty character UID
// omits the sidecar; a nonempty UID with an empty state deletes it. Callers must
// reserve every included aggregate until the actor acknowledges the result.
func (s *PostgresStore) SaveCriticalGameState(guilds *model.GuildRegistry,
	instances *model.InstanceStateSnapshot, characterUID string, state *model.CharState,
	accounts ...*model.Account) (err error) {
	writeStarted := false
	defer func() {
		if err != nil && !writeStarted {
			err = rejectWrite(err)
		}
	}()
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	if guilds == nil && instances == nil {
		return errors.New("store: critical game-state write requires a global aggregate")
	}
	for _, account := range accounts {
		if account == nil {
			return errors.New("store: critical game-state write has a nil participant")
		}
	}
	var guildPayload, instancePayload, statePayload []byte
	if guilds != nil {
		if err := guilds.Validate(); err != nil {
			return fmt.Errorf("store: invalid guild registry: %w", err)
		}
		guildPayload, err = json.Marshal(guilds)
		if err != nil {
			return err
		}
	}
	if instances != nil {
		if instances.Version != model.InstanceStateVersion {
			return fmt.Errorf("store: instance state version %d; expected %d", instances.Version, model.InstanceStateVersion)
		}
		instancePayload, err = json.Marshal(instances)
		if err != nil {
			return err
		}
	}
	var stateKey string
	var stateUID pgtype.UUID
	if characterUID == "" {
		if state != nil {
			return errors.New("store: sidecar requires an explicit character UID")
		}
	} else {
		if len(accounts) == 0 {
			return errors.New("store: sidecar requires an owner account")
		}
		stateKey, err = postgresStateFenceKey(characterUID)
		if err != nil {
			return err
		}
		owned := false
		for i, account := range accounts {
			for _, character := range account.Chars {
				key, keyErr := postgresStateFenceKey(character.UID)
				if keyErr == nil && key == stateKey {
					if i != 0 {
						return errors.New("store: sidecar belongs to a different participant")
					}
					owned = true
				}
			}
		}
		if !emptyCharState(state) && !owned {
			return errors.New("store: sidecar does not belong to the owner account")
		}
		statePayload, err = charStatePayload(stateKey, state)
		if err != nil {
			return err
		}
		stateUID, err = postgresUUID(stateKey)
		if err != nil {
			return err
		}
	}
	seq := s.writeSeq.Add(1)
	release, err := s.beginPostgresWrite()
	if err != nil {
		return err
	}
	defer release()
	unlockAccounts := s.accountFences.lock(accountModelKeys(accounts...)...)
	defer unlockAccounts()
	if stateKey != "" {
		unlockState := s.stateFences.lock(stateKey)
		defer unlockState()
	}
	snapshots, err := snapshotAccounts(accounts...)
	if err != nil {
		return err
	}
	s.fenceAccountSnapshots(seq, snapshots)
	if stateKey != "" {
		s.fenceCharStates(seq, stateKey)
	}
	writeStarted = true
	if err := s.withCertifiedSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		if err := persistAccountSnapshots(ctx, tx, snapshots, false); err != nil {
			return err
		}
		if stateKey != "" {
			if statePayload == nil {
				if _, err := tx.Exec(ctx, `DELETE FROM character_states WHERE character_uid=$1`, stateUID); err != nil {
					return err
				}
			} else if _, err := tx.Exec(ctx, `
				INSERT INTO character_states(character_uid,payload) VALUES($1,$2)
				ON CONFLICT(character_uid) DO UPDATE SET
					payload=EXCLUDED.payload,updated_at=now()`, stateUID, statePayload); err != nil {
				return err
			}
		}
		if guilds != nil {
			if _, err := tx.Exec(ctx, `
				INSERT INTO guild_state(singleton,payload) VALUES(true,$1)
				ON CONFLICT(singleton) DO UPDATE SET
					payload=EXCLUDED.payload,version=guild_state.version+1,updated_at=now()`, guildPayload); err != nil {
				return err
			}
		}
		if instances != nil {
			if _, err := tx.Exec(ctx, `
				INSERT INTO instance_state(singleton,payload) VALUES(true,$1)
				ON CONFLICT(singleton) DO UPDATE SET
					payload=EXCLUDED.payload,version=instance_state.version+1,updated_at=now()`, instancePayload); err != nil {
				return err
			}
		}
		return nil
	}); err != nil {
		return err
	}
	if guilds != nil {
		s.scheduleGuildExport(guilds)
	}
	return nil
}
