package game

import (
	"errors"
	"fmt"
	"strings"
	"testing"
	"time"
	"wydgo/internal/data"

	"wydgo/internal/model"
)

func TestWarNoticesFitNativePanelAndDoNotOverwriteResults(t *testing.T) {
	w, _, a, _, clock := scheduledWarWorld(t, "2026-09-13 20:21")
	message := strings.Repeat("Guilda vencedora ", 12)
	parts := splitWarNotice(message)
	for _, part := range parts {
		if len(part) > 95 {
			t.Fatal("notice truncated by native panel")
		}
	}
	w.warNotice(message)
	before := a.Session.QueuedPacketsForTest()
	w.flushWarNotices(clock.Now())
	w.flushWarNotices(clock.Now())
	if a.Session.QueuedPacketsForTest() != before+1 {
		t.Fatal("same-frame notice overwrite")
	}
	for range parts[1:] {
		clock.Advance(5 * time.Second)
		w.flushWarNotices(clock.Now())
	}
	if a.Session.QueuedPacketsForTest() != before+len(parts) || len(w.warNotices) != 0 {
		t.Fatal("notice lost")
	}
}

func TestScheduledCityAllCountdownsAndCommandHistory(t *testing.T) {
	w, _, a, _, clock := scheduledWarWorld(t, "2026-09-13 19:55")
	start := clock.Now().Add(5 * time.Minute)
	w.guilds.Wars.Cities = model.CityWarState{Day: "2026-09-13", StartsAt: start, Phase: "waiting"}
	for _, step := range []struct {
		minute int
		mask   uint16
	}{
		{-5, 1}, {-1, 3}, {0, 7}, {3, 7}, {5, 15}, {6, 15}, {16, 31}, {20, 63},
	} {
		clock.current = start.Add(time.Duration(step.minute) * time.Minute)
		w.tickGuildWars(clock.Now())
		if got := w.guilds.Wars.Cities.Notices; got != step.mask {
			t.Fatalf("minute %d: notice mask %d, want %d", step.minute, got, step.mask)
		}
	}
	before := a.Session.QueuedPacketsForTest()
	w.guildWarCommand(a, "citywar", "")
	if a.Session.QueuedPacketsForTest() != before+4 {
		t.Fatal("city status must keep all four lines in chat")
	}
}

func TestScheduledWarDeathDoesNotChargeCPOrExperience(t *testing.T) {
	w, st, a, b, clock := scheduledWarWorld(t, "2026-09-13 20:06")
	w.guilds.Wars.Cities = model.CityWarState{Day: "2026-09-13", StartsAt: clock.Now().Add(-6 * time.Minute), Phase: "battle"}
	a.X, a.Y, b.X, b.Y = 205, 220, 228, 220
	w.cityFighters = map[*Player]cityFighter{a: {city: 0, guild: 1}, b: {city: 0, guild: 2}}
	a.Char.CP, b.Char.CP, b.Char.Exp, b.Char.Hold = 75, -10, 50000, 7
	w.applyPvPKills(a, b)
	if a.Char.CP != 75 || b.Char.CP != -10 || b.Char.Exp != 50000 || b.Char.Hold != 7 || st.gameSaves != 0 {
		t.Fatal("war kill charged ordinary PvP penalty")
	}
	if _, exists := w.cityFighters[b]; exists {
		t.Fatal("dead participant retained")
	}
}

func TestScheduledCitySummonLimitAndErionMortalRule(t *testing.T) {
	w, _, _, _, clock := scheduledWarWorld(t, "2026-09-13 20:03")
	w.guilds.Wars.Cities = model.CityWarState{Day: "2026-09-13", StartsAt: clock.Now().Add(-3 * time.Minute), Phase: "preparing"}
	w.guilds.Wars.Cities.Territories[0] = model.CityTerritory{Owner: 1, Challenger: 2}
	for i := 3; i <= 32; i++ {
		p, _ := networkedTestPlayer(uint16(i), fmt.Sprintf("Fighter%02d", i), 2100, 2100)
		p.Char.GuildID = 2
		w.guilds.FindByID(2).Members = append(w.guilds.FindByID(2).Members, model.GuildMember{Character: p.Char.Name, Account: p.Account.Name, Rank: model.GuildRankMember})
		w.players[p.Session], w.playersByID[p.ID] = p, p
		w.indexPlayerCharacter(p)
		w.updatePlayerSpatial(p)
	}
	w.summonCityFighters()
	count := 0
	for _, f := range w.cityFighters {
		if f.guild == 2 {
			count++
		}
	}
	if count != 26 {
		t.Fatalf("summoned %d, expected 26", count)
	}
	if warMortal(&model.Char{Evolution: "arch"}) || !warMortal(&model.Char{Evolution: "mortal"}) {
		t.Fatal("Erion evolution gate")
	}
}

func TestScheduledTowerRejectsOwnerlessAndOutOfArenaDots(t *testing.T) {
	w, _, a, _, clock := scheduledWarWorld(t, "2026-09-14 21:05")
	w.tickGuildWars(clock.Now())
	m := w.warTower()
	if _, _, ok := w.resolveMobAffectSource(m, &model.Affect{}); ok {
		t.Fatal("ownerless tower damage")
	}
	a.X, a.Y = 2494, 1885
	affect := model.Affect{OwnerCharacterUID: a.Char.UID}
	if _, _, ok := w.resolveMobAffectSource(m, &affect); !ok {
		t.Fatal("valid guild dot rejected")
	}
	w.guilds.Wars.Tower.Owner = 1
	if _, _, ok := w.resolveMobAffectSource(m, &affect); ok {
		t.Fatal("owner dot hits own tower")
	}
	w.guilds.Wars.Tower.Owner = 0
	a.X, a.Y = 2100, 2100
	if _, _, ok := w.resolveMobAffectSource(m, &affect); ok {
		t.Fatal("out-of-arena dot hits tower")
	}
}

func scheduledWarWorld(t *testing.T, date string) (*World, *guildFlowStore, *Player, *Player, *fakeClock) {
	t.Helper()
	w, st, a, b := guildWarTestWorld(t)
	w.warConfig = model.DefaultGuildWarConfig()
	w.warLocation, _ = time.LoadLocation(w.warConfig.Timezone)
	now, err := time.ParseInLocation("2006-01-02 15:04", date, w.warLocation)
	if err != nil {
		t.Fatal(err)
	}
	clock := newFakeClock(now)
	w.clock = clock
	w.terrain = loadedFlatTerrain()
	w.npcs = []model.NPCDef{{Name: "Torre", Score: &model.Score{MaxHP: 9000, Defense: 500}}}
	a.Char.Citizenship, b.Char.Citizenship = w.channel, w.channel
	return w, st, a, b, clock
}

func TestScheduledTowerLifecycleNoticesCaptureRewardAndReplay(t *testing.T) {
	w, st, a, b, clock := scheduledWarWorld(t, "2026-09-14 21:00")
	w.tickGuildWars(clock.Now())
	if w.guilds.Wars.Tower.Notices != 1 || w.warTower() != nil {
		t.Fatal("missing prewarning or early tower")
	}
	queued := a.Session.QueuedPacketsForTest()
	saves := st.gameSaves
	clock.Advance(time.Second)
	w.tickGuildWars(clock.Now())
	if a.Session.QueuedPacketsForTest() != queued || st.gameSaves != saves {
		t.Fatal("warning replayed")
	}
	clock.Advance(4*time.Minute - time.Second)
	w.tickGuildWars(clock.Now())
	if w.guilds.Wars.Tower.Notices&2 == 0 {
		t.Fatal("missing one-minute warning")
	}
	clock.Advance(time.Minute)
	w.tickGuildWars(clock.Now())
	m := w.warTower()
	if m == nil || !w.guilds.Wars.Tower.Started {
		t.Fatal("tower did not start")
	}
	a.X, a.Y = 2494, 1885
	b.X, b.Y = 2493, 1885
	if !w.towerTargetAllowed(a, m) {
		t.Fatal("guild cannot attack")
	}
	m.HP = 0
	w.killMobState(a, m, 9000, 9000)
	if w.guilds.Wars.Tower.Owner != 1 || m.HP != 9000 || m.Dead || a.Char.Gold != 0 || w.guilds.FindByID(1).Fame != 0 {
		t.Fatal("capture leaked ordinary kill rewards")
	}
	a.X, a.Y = 2494, 1885
	if w.towerTargetAllowed(a, m) {
		t.Fatal("owner can attack own tower")
	}
	b.X, b.Y = 2493, 1885
	m.HP = 0
	st.err = errors.New("disk unavailable")
	w.killMobState(b, m, 9000, 9000)
	if w.guilds.Wars.Tower.Owner != 1 || m.HP == 0 {
		t.Fatal("failed capture not rolled back")
	}
	st.err = nil
	clock.Advance(25 * time.Minute)
	w.tickGuildWars(clock.Now())
	if w.guilds.Wars.Tower.Notices&4 == 0 {
		t.Fatal("missing end warning")
	}
	clock.Advance(5 * time.Minute)
	st.err = errors.New("disk unavailable")
	w.tickGuildWars(clock.Now())
	if w.guilds.FindByID(1).Fame != 0 || w.guilds.Wars.Tower.Finished {
		t.Fatal("failed reward published")
	}
	st.err = nil
	clock.Advance(time.Second)
	w.tickGuildWars(clock.Now())
	if w.guilds.FindByID(1).Fame != 100 || !w.guilds.Wars.Tower.Finished || w.warTower() != nil {
		t.Fatal("reward or cleanup missing")
	}
	w.warInitialized = false
	clock.Advance(time.Second)
	w.tickGuildWars(clock.Now())
	if w.guilds.FindByID(1).Fame != 100 {
		t.Fatal("restart doubled fame")
	}
}

func TestScheduledCityLifecycleAndCombatGates(t *testing.T) {
	w, st, a, b, clock := scheduledWarWorld(t, "2026-09-12 12:00")
	w.guilds.FindByID(2).Fame = 200
	w.guilds.Wars.Cities.Territories[0].Owner = 1
	if err := w.registerCityWar(b, 0); err != nil {
		t.Fatal(err)
	}
	if w.guilds.FindByID(2).Fame != 100 {
		t.Fatal("fee")
	}
	if err := w.registerCityWar(b, 1); err == nil || w.guilds.FindByID(2).Fame != 100 {
		t.Fatal("duplicate")
	}
	clock.current = w.guilds.Wars.Cities.StartsAt.Add(-5 * time.Minute)
	w.tickGuildWars(clock.Now())
	if w.guilds.Wars.Cities.Phase != "waiting" || w.guilds.Wars.Cities.Notices&1 == 0 {
		t.Fatal("registration closure or warning")
	}
	clock.Advance(5 * time.Minute)
	w.tickGuildWars(clock.Now())
	if w.guilds.Wars.Cities.Notices&4 == 0 {
		t.Fatal("missing start notice")
	}
	clock.Advance(3 * time.Minute)
	w.tickGuildWars(clock.Now())
	if len(w.cityFighters) != 2 || w.guilds.Wars.Cities.Phase != "preparing" {
		t.Fatalf("summon: %d %s", len(w.cityFighters), w.guilds.Wars.Cities.Phase)
	}
	if allowed, handled := w.guildWarPvP(a, b); allowed || !handled {
		t.Fatal("preparation permits PvP")
	}
	if w.guildWarStepAllowed(a, 225, 220) {
		t.Fatal("preparation gate crossed")
	}
	clock.Advance(3 * time.Minute)
	w.tickGuildWars(clock.Now())
	if allowed, handled := w.guildWarPvP(a, b); !allowed || !handled {
		t.Fatal("battle does not permit enemy PvP")
	}
	if !w.guildWarStepAllowed(a, 225, 220) {
		t.Fatal("battle gate closed")
	}
	if !w.teleportPlayer(a, 2086, 2093) || len(w.cityFighters) != 1 {
		t.Fatal("teleport did not forfeit")
	}
	if w.guildWarStepAllowed(a, 205, 220) {
		t.Fatal("outsider can reenter")
	}
	clock.Advance(15 * time.Minute)
	st.err = errors.New("disk unavailable")
	w.tickGuildWars(clock.Now())
	if w.guilds.Wars.Cities.Territories[0].Owner != 1 || w.guilds.Wars.Cities.Phase != "battle" {
		t.Fatal("result published before persistence")
	}
	// Moving/disconnecting after the deadline must not change the frozen score.
	b.X, b.Y = 2100, 2100
	st.err = nil
	clock.Advance(time.Second)
	w.tickGuildWars(clock.Now())
	if w.guilds.Wars.Cities.Territories[0].Owner != 2 || w.guilds.Wars.Cities.Phase != "finished" || len(w.cityFighters) != 0 || cityWarZones[0].arena.contains(b.X, b.Y) {
		t.Fatal("result or cleanup")
	}
	if w.guilds.FindByID(2).Fame != 100 {
		t.Fatal("city invented fame award")
	}
}

func TestScheduledWarPositionsOnInstalledTerrain(t *testing.T) {
	terrain, err := data.LoadTerrain("../../data/maps/HeightMap.dat", "../../data/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	for _, z := range cityWarZones {
		for _, p := range [][2]uint16{{z.exitX, z.exitY}, {z.defendX, z.defendY}, {z.attackX, z.attackY}} {
			if !terrain.Walkable(p[0], p[1]) {
				t.Errorf("%s blocked at %v", z.name, p)
			}
		}
	}
	for _, p := range [][2]uint16{{2495, 1885}, {2450, 1855}} {
		if !terrain.Walkable(p[0], p[1]) {
			t.Errorf("tower blocked at %v", p)
		}
	}
}

func TestScheduledWarRegistrationRollbackAndLeaderAuthority(t *testing.T) {
	w, st, a, _, _ := scheduledWarWorld(t, "2026-09-12 12:00")
	w.tickGuildWars(w.now())
	w.guilds.FindByID(1).Fame = 100
	st.err = errors.New("disk failure")
	if err := w.registerCityWar(a, 0); err == nil || w.guilds.FindByID(1).Fame != 100 || len(w.guilds.Wars.Cities.Entries) != 0 {
		t.Fatal("fee rollback")
	}
	st.err = nil
	w.guilds.FindByID(1).Members[0].Rank = model.GuildRankMember
	if err := w.registerCityWar(a, 0); err == nil {
		t.Fatal("forged leader projection accepted")
	}
}

func TestScheduledCityInterruptedRefundIsAtomicAndOnce(t *testing.T) {
	w, st, _, b, clock := scheduledWarWorld(t, "2026-09-12 12:00")
	w.guilds.FindByID(2).Fame = 100
	if err := w.registerCityWar(b, 0); err != nil {
		t.Fatal(err)
	}
	w.guilds.Wars.Cities.Phase = "battle"
	w.warInitialized = false
	clock.Advance(time.Second)
	st.err = errors.New("disk unavailable")
	w.tickGuildWars(clock.Now())
	if w.guilds.FindByID(2).Fame != 0 || w.guilds.Wars.Cities.Phase != "battle" {
		t.Fatal("refund not atomic")
	}
	st.err = nil
	clock.Advance(time.Second)
	w.tickGuildWars(clock.Now())
	if w.guilds.FindByID(2).Fame != 100 || w.guilds.Wars.Cities.Phase != "finished" {
		t.Fatal("missing refund")
	}
	w.warInitialized = false
	clock.Advance(time.Second)
	w.tickGuildWars(clock.Now())
	if w.guilds.FindByID(2).Fame != 100 {
		t.Fatal("refund replay")
	}
}

func TestScheduledCityTieRetainsDefenderAndDefenseCaps(t *testing.T) {
	w, _, _, _, clock := scheduledWarWorld(t, "2026-09-13 20:21")
	w.guilds.Wars.Cities = model.CityWarState{Day: "2026-09-13", StartsAt: clock.Now().Add(-21 * time.Minute), Phase: "battle"}
	for i := range w.guilds.Wars.Cities.Territories {
		w.guilds.Wars.Cities.Territories[i] = model.CityTerritory{Owner: 1, Challenger: 2, Victories: 4}
	}
	if err := w.finishCityWar(); err != nil {
		t.Fatal(err)
	}
	for _, z := range w.guilds.Wars.Cities.Territories {
		if z.Owner != 1 || z.Victories != 4 || z.Challenger != 0 {
			t.Fatal("tie/cap")
		}
	}
}
