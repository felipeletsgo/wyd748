package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

type criticalGuildRowsTestStore struct{ *criticalTestStore }

func (s *criticalGuildRowsTestStore) SaveCriticalGameState(guilds *model.GuildRegistry,
	instances *model.InstanceStateSnapshot, uid string, state *model.CharState, accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts, guilds: guilds, instances: instances})
}

func (s *criticalGuildRowsTestStore) SaveCriticalGameStateRows(guilds *model.GuildRegistry,
	guildKeys []store.GuildParticipant, instances *model.InstanceStateSnapshot, instanceKeys []store.InstanceParticipant,
	uid string, state *model.CharState, accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts, guilds: guilds, guildKeys: guildKeys, instances: instances})
}

// W8: writes of different guilds are admitted together; a second write of
// the same guild waits; a scoped install keeps the other guild's live state.
func TestCriticalGuildPlansReserveOnlyTheirGuilds(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	q := newCriticalTestQueue(t, &criticalGuildRowsTestStore{backend}, 2, 4)
	t.Cleanup(release)
	w := &World{guilds: &model.GuildRegistry{Version: model.GuildRegistryVersion,
		Guilds: []model.Guild{{ID: 1, Name: "One", Fame: 10}, {ID: 2, Name: "Two", Fame: 20}}}}
	w.criticalWrites = newCriticalPersistenceOwner(q)
	planFor := func(id uint16, fame uint32) *criticalGuildPlan {
		retained, proposed := w.criticalGuildSnapshot(), w.criticalGuildSnapshot()
		proposed.FindByID(id).Fame = fame
		return newCriticalGuildPlan(w.guilds, retained, proposed, true)
	}
	one, two := planFor(1, 11), planFor(2, 22)
	if keys := one.keys(); len(keys) != 1 || keys[0].GuildID != 1 {
		t.Fatalf("plan keys %+v, want guild 1 only", keys)
	}
	if _, err := q.submit(criticalWrite{guilds: one.proposed, guildKeys: one.keys()}); err != nil {
		t.Fatal(err)
	}
	if _, err := q.submit(criticalWrite{guilds: two.proposed, guildKeys: two.keys()}); err != nil {
		t.Fatalf("unrelated guild write refused: %v", err)
	}
	if _, err := q.submit(criticalWrite{guilds: one.proposed, guildKeys: one.keys()}); !errors.Is(err, errCriticalPersistenceBusy) {
		t.Fatalf("second write of guild 1: %v", err)
	}
	if !w.criticalGuildIDReserved(1) || !w.criticalGuildIDReserved(2) || w.criticalGuildIDReserved(3) || w.criticalGuildWholeReserved() {
		t.Fatal("reservations are not per guild")
	}
	// Guild 2's plan installs first; guild 1's scoped install must keep it.
	two.install(w)
	if !one.unchanged(w) {
		t.Fatal("a change of guild 2 invalidated the plan of guild 1")
	}
	one.install(w)
	if w.guilds.FindByID(1).Fame != 11 || w.guilds.FindByID(2).Fame != 22 {
		t.Fatalf("scoped installs: one=%d two=%d", w.guilds.FindByID(1).Fame, w.guilds.FindByID(2).Fame)
	}
	if !one.installed(w) || !two.installed(w) {
		t.Fatal("installed participants do not match their proposals")
	}
}

// A changed guild that references another guild pulls the target into scope.
func TestCriticalGuildPlanIncludesReferencedGuild(t *testing.T) {
	w := &World{guilds: &model.GuildRegistry{Version: model.GuildRegistryVersion,
		Guilds: []model.Guild{{ID: 1, Name: "One"}, {ID: 2, Name: "Two"}}}}
	retained, proposed := w.criticalGuildSnapshot(), w.criticalGuildSnapshot()
	proposed.FindByID(1).Ally = 2
	keys := newCriticalGuildPlan(w.guilds, retained, proposed, true).keys()
	if len(keys) != 2 || keys[0].GuildID != 1 || keys[1].GuildID != 2 {
		t.Fatalf("alliance plan keys %+v, want guilds 1 and 2", keys)
	}
}
