package store

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
)

// SavePlayerStateBatch commits the owner's sidecar and all reserved accounts in
// one transaction. It never falls back to separately saving the participants.
func (s *PostgresStore) SavePlayerStateBatch(guilds *model.GuildRegistry,
	characterUID string, state *model.CharState, accounts ...*model.Account) (err error) {
	writeStarted := false
	defer func() {
		if err != nil && !writeStarted {
			err = rejectWrite(err)
		}
	}()
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	if len(accounts) == 0 || accounts[0] == nil {
		return errors.New("store: player state batch requires an owner account")
	}
	stateKey, err := postgresStateFenceKey(characterUID)
	if err != nil {
		return err
	}
	// Nonempty sidecars must belong to the first participant. Nil sidecars may
	// delete a removed character's state, as in the single-account interface.
	if !emptyCharState(state) {
		owned := false
		for _, character := range accounts[0].Chars {
			key, err := postgresStateFenceKey(character.UID)
			if err == nil && key == stateKey {
				owned = true
			}
		}
		if !owned {
			return errors.New("store: sidecar does not belong to the owner account")
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
	unlockState := s.stateFences.lock(stateKey)
	defer unlockState()
	snapshots, err := snapshotAccounts(accounts...)
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
	writeStarted = true
	if err := s.withCertifiedSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		if err := persistAccountSnapshots(ctx, tx, snapshots, false); err != nil {
			return err
		}
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
		if guilds != nil {
			_, err := tx.Exec(ctx, `
				INSERT INTO guild_state(singleton,payload) VALUES(true,$1)
				ON CONFLICT(singleton) DO UPDATE SET
					payload=EXCLUDED.payload,version=guild_state.version+1,updated_at=now()`, guildPayload)
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
