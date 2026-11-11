package game

import (
	"context"
	"errors"
	"net/netip"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

type worldGuildLoadFailureStore struct{ *craftStore }

func (*worldGuildLoadFailureStore) LoadGuilds() (*model.GuildRegistry, error) {
	return nil, errors.New("guild load failed")
}

func (*worldGuildLoadFailureStore) SaveGameState(*model.GuildRegistry, ...*model.Account) error {
	return nil
}

type worldGuildRowsFailureStore struct{ *craftStore }

func (*worldGuildRowsFailureStore) LoadGuildRowStates([]uint16) ([]store.GuildRowState, error) {
	return nil, errors.New("guild rows failed")
}

type worldCharacterNamesFailureStore struct{ *craftStore }

func (*worldCharacterNamesFailureStore) CharacterNames() (map[string]struct{}, error) {
	return nil, errors.New("character index failed")
}

type worldNPCDefinitionsFailureStore struct {
	*craftStore
	store.NPCDefinitionStore
}

func (*worldNPCDefinitionsFailureStore) SyncNPCDefinitions(context.Context, uint8, []store.NPCDefinitionSeed) error {
	return errors.New("NPC synchronization failed")
}

func TestNewWorldRejectsInvalidStartupOptions(t *testing.T) {
	colosseum := model.DefaultColosseumConfig()
	colosseum.EventHour = 21
	wars := model.DefaultGuildWarConfig()
	wars.Timezone = ""
	operational := DefaultOperationalConfig()
	operational.ChannelID = 0

	tests := []struct {
		name string
		want string
		opt  WorldOption
	}{
		{name: "colosseum", want: "colosseum configuration", opt: WithColosseum(colosseum, model.ColosseumGenerators{})},
		{name: "guild wars", want: "guild wars:", opt: WithGuildWars(wars)},
		{name: "operational", want: "operational configuration", opt: WithOperationalConfig(operational)},
		{name: "network admission", want: "politica de admissao", opt: WithNetworkAdmission(model.NetworkAdmissionFile{})},
		{name: "uxmal", want: "Uxmal configuration", opt: WithUxmal(model.VolatileInstance{})},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			npcs, geners, catalog, templates := minimalWorldInputs()
			_, err := NewWorld(&craftStore{}, npcs, geners, catalog, [model.MaxCarry]int{},
				model.VolatileCatalog{}, templates, model.TerrainMap{}, tc.opt)
			if err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("invalid %s configuration error=%v, want substring %q", tc.name, err, tc.want)
			}
		})
	}
}

func TestNewWorldInitializesGuildWarColosseumAndNetworkOptions(t *testing.T) {
	npcs, geners, catalog, templates := minimalWorldInputs()
	operational := DefaultOperationalConfig()
	operational.ChannelID = 2
	admission := model.NetworkAdmissionFile{
		Version: model.NetworkAdmissionVersion,
		Rules: []model.NetworkAdmissionRule{{
			CIDR: netip.MustParsePrefix("192.0.2.0/24"), Action: model.NetworkAdmissionAllow,
		}},
	}
	clock := newFakeClock(time.Date(2026, 7, 26, 15, 0, 0, 0, time.UTC))

	w, err := NewWorld(&craftStore{}, npcs, geners, catalog, [model.MaxCarry]int{},
		model.VolatileCatalog{}, templates, model.TerrainMap{},
		WithClock(clock), WithOperationalConfig(operational), WithNetworkAdmission(admission),
		WithGuildWars(model.DefaultGuildWarConfig()),
		WithColosseum(model.DefaultColosseumConfig(), model.ColosseumGenerators{}),
		WithNPCGenerLog("quiet"),
	)
	if err != nil {
		t.Fatalf("NewWorld with valid event and admission options: %v", err)
	}
	if w.channel != 2 || w.commands == nil || cap(w.commands) != operational.WorldCommandQueueCapacity {
		t.Fatalf("operational options not initialized: channel=%d queue=%v", w.channel, w.commands)
	}
	if w.colosseum == nil || w.warLocation == nil {
		t.Fatalf("event configuration not initialized: colosseum=%v war location=%v", w.colosseum, w.warLocation)
	}
	if _, ok := w.networkAdmission.match(netip.MustParseAddr("192.0.2.42")); !ok {
		t.Fatal("valid network admission prefix was not installed")
	}
}

func TestNewWorldReturnsPersistenceInitializationFailures(t *testing.T) {
	tests := []struct {
		name  string
		store store.Store
		want  string
		opt   WorldOption
	}{
		{name: "guilds", store: &worldGuildLoadFailureStore{&craftStore{}}, want: "load guilds: guild load failed"},
		{name: "guild rows", store: &worldGuildRowsFailureStore{&craftStore{}}, want: "load guild row versions: guild rows failed", opt: WithGuildRowVersions()},
		{name: "character names", store: &worldCharacterNamesFailureStore{&craftStore{}}, want: "index character names: character index failed"},
		{name: "NPC definitions", store: &worldNPCDefinitionsFailureStore{craftStore: &craftStore{}}, want: "load PostgreSQL NPC definitions: NPC synchronization failed"},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			npcs, geners, catalog, templates := minimalWorldInputs()
			var options []WorldOption
			if tc.opt != nil {
				options = append(options, tc.opt)
			}
			_, err := NewWorld(tc.store, npcs, geners, catalog, [model.MaxCarry]int{},
				model.VolatileCatalog{}, templates, model.TerrainMap{}, options...)
			if err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("startup error=%v, want substring %q", err, tc.want)
			}
		})
	}
}
