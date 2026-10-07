package game

import (
	"errors"
	"math"
	"testing"
	"time"

	"wydgo/internal/model"
)

func kefraTestWorld(t *testing.T) (*World, *guildFlowStore, *Player, *Player, *fakeClock, *Mob) {
	t.Helper()
	// 2026-09-14 is a Monday; the weekly reset is the next day at 12:00.
	w, st, alpha, beta, clock := scheduledWarWorld(t, "2026-09-14 21:00")
	kefra := &Mob{ID: 4000, X: 2372, Y: 3932, GenerIndex: -1, HP: 1,
		Def: &model.NPCDef{Name: kefraBossName, Tipo: model.TipoMonstro,
			Score: &model.Score{MaxHP: 28000, Level: 390}}}
	w.registerMobSpatial(kefra)
	w.teleports = []model.Teleport{{SourceX: 2366, SourceY: 3925, DestX: 3250, DestY: 1703,
		Price: 700, Name: "Hall Kefra para Cidade Kefra"}}
	return w, st, alpha, beta, clock, kefra
}

func TestKefraResetIsTheNextTuesdayNoon(t *testing.T) {
	location, _ := time.LoadLocation(model.DefaultGuildWarConfig().Timezone)
	at := func(s string) time.Time {
		v, err := time.ParseInLocation("2006-01-02 15:04", s, location)
		if err != nil {
			t.Fatal(err)
		}
		return v
	}
	for defeated, want := range map[string]string{
		"2026-09-14 21:00": "2026-09-15 12:00", // Monday -> next day
		"2026-09-15 11:59": "2026-09-15 12:00", // Tuesday before noon
		"2026-09-15 12:00": "2026-09-22 12:00", // exactly at reset -> next week
		"2026-09-15 13:00": "2026-09-22 12:00",
		"2026-09-20 08:00": "2026-09-22 12:00", // Sunday
	} {
		if got := kefraResetAt(at(defeated), location); !got.Equal(at(want)) {
			t.Fatalf("defeat %s resets at %v; want %s", defeated, got, want)
		}
	}
}

func TestKefraDeathCreditsGuildAndOpensThePassageToUxmal(t *testing.T) {
	w, st, alpha, beta, _, kefra := kefraTestWorld(t)
	beta.X, beta.Y = 2366, 3925
	beta.Char.X, beta.Char.Y = beta.X, beta.Y
	beta.Char.Gold = 10_000

	w.onReqTeleport(beta.Session, make([]byte, 16))
	if beta.X != 2366 || beta.Y != 3925 || beta.Char.Gold != 10_000 {
		t.Fatalf("closed passage moved or charged the player: (%d,%d) gold=%d", beta.X, beta.Y, beta.Char.Gold)
	}
	drainPlayerPackets(beta)

	fame, saves := w.guilds.FindByID(1).Fame, st.gameSaves
	w.onKefraKilled(alpha, kefra)
	state := w.guilds.Wars.Kefra
	if !state.Defeated || state.Guild != 1 || state.DefeatedAt.IsZero() {
		t.Fatalf("defeat not recorded: %+v", state)
	}
	if got := w.guilds.FindByID(1).Fame; got != fame+kefraGuildFame || st.gameSaves != saves+1 {
		t.Fatalf("fame=%d saves=%d; want +%d fame in one commit", got, st.gameSaves, kefraGuildFame)
	}
	if beta.Session.QueuedPacketsForTest() == 0 {
		t.Fatal("the defeat was not announced")
	}
	drainPlayerPackets(beta)

	w.onKefraKilled(alpha, kefra)
	if w.guilds.FindByID(1).Fame != fame+kefraGuildFame || st.gameSaves != saves+1 {
		t.Fatal("a second death while defeated credited the guild again")
	}

	w.onReqTeleport(beta.Session, make([]byte, 16))
	if chebyshev(beta.X, beta.Y, 3250, 1703) > 3 || beta.Char.Gold != 9_300 {
		t.Fatalf("open passage did not lead to Uxmal: (%d,%d) gold=%d", beta.X, beta.Y, beta.Char.Gold)
	}
}

func TestKefraKilledWithoutGuildIsDefeatedWithoutFame(t *testing.T) {
	w, st, alpha, _, _, kefra := kefraTestWorld(t)
	w.guilds.Guilds[0].Members = nil // alpha no longer belongs to a guild
	fame := w.guilds.FindByID(1).Fame
	w.onKefraKilled(alpha, kefra)
	if s := w.guilds.Wars.Kefra; !s.Defeated || s.Guild != 0 || w.guilds.FindByID(1).Fame != fame {
		t.Fatalf("guild-less defeat = %+v fame=%d", s, w.guilds.FindByID(1).Fame)
	}
	if st.durable == nil || !st.durable.Wars.Kefra.Defeated || st.durable.Wars.Kefra.Guild != 0 {
		t.Fatal("the guild-less defeat was not persisted")
	}
}

// restartGuilds replaces the in-memory guild state with what the store would
// load after a restart.
func restartGuilds(t *testing.T, w *World, st *guildFlowStore) {
	t.Helper()
	reg, err := st.LoadGuilds()
	if err != nil {
		t.Fatal(err)
	}
	w.guilds = reg
}

// Audit A01: a Kefra kill whose commit fails must leave no defeat or fame in
// memory that a restart would not load, and the next successful kill must
// credit the guild exactly once.
func TestKefraKillCommitFailureRollsBackAndRecoversAfterRestart(t *testing.T) {
	w, st, alpha, beta, _, kefra := kefraTestWorld(t)
	kefra.X, kefra.Y = alpha.X+1, alpha.Y
	fame := w.guilds.FindByID(1).Fame
	st.durable = cloneGuildRegistry(w.guilds) // the state loaded at boot
	st.err = errors.New("disk full")
	drainPlayerPackets(beta)

	kefra.HP = 0
	w.killMobState(alpha, kefra, 1, 1)
	if w.kefraDefeated() || w.guilds.FindByID(1).Fame != fame {
		t.Fatalf("failed commit left state=%+v fame=%d in memory", w.guilds.Wars.Kefra, w.guilds.FindByID(1).Fame)
	}
	if kefra.Dead {
		t.Fatal("a failed commit must roll the kill back")
	}
	if st.durable.Wars.Kefra.Defeated || st.durable.FindByID(1).Fame != fame {
		t.Fatal("a failed commit reached the store")
	}
	def := *kefra.Def
	if w.generatorBlockedByKefra(&generState{leader: &def, follower: &def}) {
		t.Fatal("an undurable defeat blocked Kefra's generator")
	}

	restartGuilds(t, w, st) // a restart loads the same undefeated state
	if w.kefraDefeated() {
		t.Fatal("restart loaded a defeat that was never persisted")
	}

	st.err = nil // storage is back
	kefra.HP = 0
	w.killMobState(alpha, kefra, 1, 1)
	if !kefra.Dead || !w.kefraDefeated() || w.guilds.Wars.Kefra.Guild != 1 {
		t.Fatalf("recovered kill state=%+v dead=%v", w.guilds.Wars.Kefra, kefra.Dead)
	}
	if got := w.guilds.FindByID(1).Fame; got != fame+kefraGuildFame {
		t.Fatalf("fame=%d; want exactly one grant (+%d)", got, kefraGuildFame)
	}
	saves := st.gameSaves
	w.onKefraKilled(alpha, kefra) // repeated death notifications
	w.killMobWithoutPlayer(kefra)
	if st.gameSaves != saves || w.guilds.FindByID(1).Fame != fame+kefraGuildFame {
		t.Fatal("a repeated death credited or saved again")
	}

	restartGuilds(t, w, st)
	if s := w.guilds.Wars.Kefra; !s.Defeated || s.Guild != 1 || s.DefeatedAt.IsZero() ||
		w.guilds.FindByID(1).Fame != fame+kefraGuildFame {
		t.Fatalf("restart after recovery loaded %+v fame=%d", s, w.guilds.FindByID(1).Fame)
	}
}

func TestKefraDeathWithoutKillerIsDefeatedOnlyWhenPersisted(t *testing.T) {
	w, st, _, beta, clock, kefra := kefraTestWorld(t)
	st.err = errors.New("disk full")
	drainPlayerPackets(beta)
	w.onKefraKilled(nil, kefra)
	if w.kefraDefeated() || beta.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("an unpersisted defeat was kept or announced")
	}
	w.tickKefra(clock.Now()) // the weekly reset has nothing to undo
	if w.kefraDefeated() {
		t.Fatal("reset tick invented a defeat")
	}
	st.err = nil
	w.onKefraKilled(nil, kefra)
	if !w.kefraDefeated() || st.durable == nil || !st.durable.Wars.Kefra.Defeated {
		t.Fatal("the defeat was not persisted once storage recovered")
	}
	clock.Advance(15 * time.Hour) // past Tuesday noon
	w.tickKefra(clock.Now())
	if w.kefraDefeated() {
		t.Fatal("weekly reset did not restore Kefra after recovery")
	}
}

func TestKefraDefeatAtTheFameLimitKeepsTheGuildWithoutOverflow(t *testing.T) {
	w, _, alpha, _, _, kefra := kefraTestWorld(t)
	w.guilds.FindByID(1).Fame = math.MaxUint32 - 1
	w.onKefraKilled(alpha, kefra)
	if s := w.guilds.Wars.Kefra; !s.Defeated || s.Guild != 1 || w.guilds.FindByID(1).Fame != math.MaxUint32-1 {
		t.Fatalf("state=%+v fame=%d", s, w.guilds.FindByID(1).Fame)
	}
}

func TestKefraGeneratorWaitsForTheWeeklyReset(t *testing.T) {
	w, _, alpha, _, clock, kefra := kefraTestWorld(t)
	def := *kefra.Def
	g := &generState{def: model.NPCGener{Index: 3856, MaxNumMob: 1,
		Segments: [model.MaxGenerSegments]model.GenerSegment{{X: 2372, Y: 3932}}},
		leader: &def, follower: &def}
	w.onKefraKilled(alpha, kefra)

	w.spawnGroup(g)
	if g.current != 0 {
		t.Fatal("defeated Kefra respawned through his generator")
	}
	clock.Advance(14*time.Hour + 59*time.Minute) // Tuesday 11:59
	w.tickKefra(clock.Now())
	w.spawnGroup(g)
	if !w.kefraDefeated() || g.current != 0 {
		t.Fatal("Kefra returned before Tuesday noon")
	}
	clock.Advance(time.Minute)
	w.tickKefra(clock.Now())
	if w.kefraDefeated() || w.guilds.Wars.Kefra != (model.KefraState{}) {
		t.Fatalf("weekly reset did not restore Kefra: %+v", w.guilds.Wars.Kefra)
	}
	w.spawnGroup(g)
	if g.current != 1 {
		t.Fatal("Kefra did not respawn after the reset")
	}
	alpha.X, alpha.Y = 2366, 3925
	alpha.Char.X, alpha.Char.Y = alpha.X, alpha.Y
	alpha.Char.Gold = 1_000
	w.onReqTeleport(alpha.Session, make([]byte, 16))
	if alpha.X != 2366 || alpha.Y != 3925 {
		t.Fatal("the passage stayed open after Kefra returned")
	}
}

func TestKefraKillThroughTheRewardCommitRecordsTheGuild(t *testing.T) {
	w, st, alpha, _, _, kefra := kefraTestWorld(t)
	kefra.X, kefra.Y = alpha.X+1, alpha.Y
	kefra.HP = 0
	w.killMobState(alpha, kefra, 1, 1)
	if !kefra.Dead {
		t.Fatal("Kefra did not die")
	}
	if s := w.guilds.Wars.Kefra; !s.Defeated || s.Guild != 1 || st.gameSaves == 0 {
		t.Fatalf("kill path did not record the guild: %+v saves=%d", s, st.gameSaves)
	}
}

// NPCGener.txt reuses index 3856 for Kefra and a Grim_Lock group. A death must
// lower the counter of the entry that spawned the mob, not the last entry that
// claimed the index, or Kefra never returns after the weekly reset.
func TestKefraDeathLowersItsOwnGeneratorDespiteADuplicateIndex(t *testing.T) {
	w, _, _, _, clock, kefra := kefraTestWorld(t)
	grim := model.NPCDef{Name: "Grim_Lock", Tipo: model.TipoMonstro, Score: &model.Score{MaxHP: 10}}
	w.generators = []generState{
		{def: model.NPCGener{Index: 3856, MaxNumMob: 1,
			Segments: [model.MaxGenerSegments]model.GenerSegment{{X: 2372, Y: 3932}}},
			leader: kefra.Def, follower: kefra.Def},
		{def: model.NPCGener{Index: 3856, MaxNumMob: 1,
			Segments: [model.MaxGenerSegments]model.GenerSegment{{X: 816, Y: 3988}}},
			leader: &grim, follower: &grim},
	}
	w.generatorByIndex = map[int]int{3856: 1}
	w.spawnGroup(&w.generators[0])
	w.spawnGroup(&w.generators[1])
	var spawned *Mob
	for _, m := range w.mobs {
		if m.Def == kefra.Def && m != kefra {
			spawned = m
		}
	}
	if spawned == nil || w.generators[0].current != 1 || w.generators[1].current != 1 {
		t.Fatal("generators did not spawn")
	}
	w.killMobWithoutPlayer(spawned)
	if w.generators[0].current != 0 || w.generators[1].current != 1 {
		t.Fatalf("counters kefra=%d grim=%d; want 0 and 1", w.generators[0].current, w.generators[1].current)
	}
	clock.Advance(7 * 24 * time.Hour)
	w.tickKefra(clock.Now())
	w.spawnGroup(&w.generators[0])
	if w.generators[0].current != 1 {
		t.Fatal("Kefra did not return after the reset")
	}
}

func kefraGuardGenerators(w *World) []*generState {
	crunt := &model.NPCDef{Name: "Lich_Crunt", Tipo: model.TipoMonstro, Score: &model.Score{MaxHP: 10}}
	vatama := &model.NPCDef{Name: "Lich_Vatama", Tipo: model.TipoMonstro, Score: &model.Score{MaxHP: 10}}
	w.generators = []generState{
		{def: model.NPCGener{Index: 5152, MaxNumMob: 1,
			Segments: [model.MaxGenerSegments]model.GenerSegment{{X: 2372, Y: 3924}}}, leader: crunt, follower: crunt},
		{def: model.NPCGener{Index: 5153, MaxNumMob: 1,
			Segments: [model.MaxGenerSegments]model.GenerSegment{{X: 2378, Y: 3928}}}, leader: vatama, follower: vatama},
	}
	gens := []*generState{&w.generators[0], &w.generators[1]}
	for _, g := range gens {
		w.spawnGroup(g)
	}
	return gens
}

func liveKefraGuards(w *World) []*Mob {
	var guards []*Mob
	for _, m := range w.mobs {
		if isKefraGuard(m) && !m.Dead {
			guards = append(guards, m)
		}
	}
	return guards
}

func TestKefraMagosRespawnAtOnceWhileKefraLives(t *testing.T) {
	w, _, alpha, _, _, _ := kefraTestWorld(t)
	gens := kefraGuardGenerators(w)
	guards := liveKefraGuards(w)
	if len(guards) != 2 || gens[0].current != 1 || gens[1].current != 1 {
		t.Fatalf("guards did not spawn: %d", len(guards))
	}
	killed := guards[0]
	killed.X, killed.Y = alpha.X+1, alpha.Y
	killed.HP = 0
	w.killMobState(alpha, killed, 1, 1)
	if !killed.Dead {
		t.Fatal("guard did not die")
	}
	after := liveKefraGuards(w)
	if len(after) != 2 || gens[0].current != 1 || gens[1].current != 1 {
		t.Fatalf("killed guard did not return at once: live=%d counters=%d/%d",
			len(after), gens[0].current, gens[1].current)
	}
	for _, m := range after {
		if m == killed {
			t.Fatal("the dead guard is still listed")
		}
	}

	w.killMobWithoutPlayer(after[1])
	if len(liveKefraGuards(w)) != 2 {
		t.Fatal("guard killed without a player did not return")
	}
}

func TestKefraMagosLeaveWithKefraAndReturnWithHim(t *testing.T) {
	w, _, alpha, _, clock, kefra := kefraTestWorld(t)
	gens := kefraGuardGenerators(w)
	w.onKefraKilled(alpha, kefra)
	if len(liveKefraGuards(w)) != 0 || gens[0].current != 0 || gens[1].current != 0 {
		t.Fatalf("guards stayed after Kefra died: live=%d", len(liveKefraGuards(w)))
	}
	for _, g := range gens {
		w.spawnGroup(g)
	}
	if len(liveKefraGuards(w)) != 0 {
		t.Fatal("guards respawned while Kefra is defeated")
	}
	clock.Advance(15 * time.Hour) // past Tuesday 12:00
	w.tickKefra(clock.Now())
	for _, g := range gens {
		w.spawnGroup(g)
	}
	if len(liveKefraGuards(w)) != 2 {
		t.Fatal("guards did not return with Kefra")
	}
}

func TestKefraDeathWithoutKillerIsRecorded(t *testing.T) {
	w, _, _, _, _, kefra := kefraTestWorld(t)
	w.killMobWithoutPlayer(kefra)
	if s := w.guilds.Wars.Kefra; !s.Defeated || s.Guild != 0 {
		t.Fatalf("death without killer = %+v", s)
	}
	other := &Mob{ID: 4001, GenerIndex: -1, Def: &model.NPCDef{Name: "Gold_Spider", Score: &model.Score{}}}
	w.guilds.Wars.Kefra = model.KefraState{}
	w.onKefraKilled(nil, other)
	instanced := &Mob{ID: 4002, InstanceID: "room", Def: kefra.Def}
	w.onKefraKilled(nil, instanced)
	if w.kefraDefeated() {
		t.Fatal("another mob or an instance copy defeated the world Kefra")
	}
}

func TestKefraStateValidation(t *testing.T) {
	registry := &model.GuildRegistry{Guilds: []model.Guild{{ID: 1, Name: "Alpha"}}}
	now := time.Now()
	for name, state := range map[string]model.KefraState{
		"defeated without date": {Defeated: true},
		"date while alive":      {DefeatedAt: now},
		"guild while alive":     {Guild: 1},
		"unknown guild":         {Defeated: true, DefeatedAt: now, Guild: 9},
	} {
		if err := (model.GuildWarState{Kefra: state}).Validate(registry); err == nil {
			t.Fatalf("%s accepted", name)
		}
	}
	for _, state := range []model.KefraState{{}, {Defeated: true, DefeatedAt: now}, {Defeated: true, DefeatedAt: now, Guild: 1}} {
		if err := (model.GuildWarState{Kefra: state}).Validate(registry); err != nil {
			t.Fatalf("valid state %+v rejected: %v", state, err)
		}
	}
}
