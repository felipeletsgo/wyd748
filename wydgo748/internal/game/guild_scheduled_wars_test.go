package game

import (
	"errors"
	"fmt"
	"math"
	"reflect"
	"strings"
	"testing"
	"time"
	"wydgo/internal/data"

	"wydgo/internal/model"
)

func TestWarNoticesFitNativePanelAndDoNotOverwriteResults(t *testing.T) {
	w, _, a, _, clock := scheduledWarWorld(t, "2026-09-13 20:21")
	message := strings.Repeat("Victorious guild ", 12)
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
		{-5, 1}, {-1, 3}, {0, 7}, {3, 7}, {5, 15}, {6, 15}, {26, 31}, {30, 63},
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
	// The scheduler opens the war before summoning; enrollment requires it.
	w.cityWarRunning = true
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
	a.X, a.Y = towerWarSpawnX-1, towerWarSpawnY
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
	w.guilds.FindByID(1).Kingdom = 7
	w.guilds.FindByID(2).Kingdom = 8
	a.Char.Citizenship, b.Char.Citizenship = w.channel, w.channel
	return w, st, a, b, clock
}

func TestScheduledTowerLifecycleNoticesCaptureRewardAndReplay(t *testing.T) {
	w, st, a, b, clock := scheduledWarWorld(t, "2026-09-14 20:55")
	w.guilds.Wars.Castle.Owner = 1
	prepareWarMedalPlayer(w, a)
	prepareWarMedalPlayer(w, b)
	for _, p := range []*Player{a, b} {
		for i := range p.Char.Inv {
			p.Char.Inv[i] = model.Item{Index: 400}
		}
	}
	w.tickGuildWars(clock.Now())
	if s := w.guilds.Wars.Tower; s.Notices != 1 || w.warTower() != nil || s.Owner != 0 || s.Kingdom != 7 || s.InitialKingdom != 7 || s.DefenderCaptures != 1 {
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
	a.X, a.Y = towerWarSpawnX-1, towerWarSpawnY
	b.X, b.Y = towerWarSpawnX-2, towerWarSpawnY
	if w.towerTargetAllowed(a, m) {
		t.Fatal("defending kingdom may attack its own tower")
	}
	if !w.towerTargetAllowed(b, m) {
		t.Fatal("opposing kingdom cannot attack")
	}
	m.HP = 0
	w.killMobState(b, m, 9000, 9000)
	if s := w.guilds.Wars.Tower; s.Owner != 2 || s.Kingdom != 8 || s.InitialKingdom != 7 || s.DefenderCaptures != 1 || s.Captures != 1 || m.HP != 9000 || m.Dead || b.Char.Gold != 0 || w.guilds.FindByID(2).Fame != 0 || len(w.guilds.Wars.Medals) != 1 {
		t.Fatal("capture leaked ordinary kill rewards")
	}
	a.X, a.Y = towerWarSpawnX-1, towerWarSpawnY
	if !w.towerTargetAllowed(a, m) {
		t.Fatal("opposing kingdom cannot recapture the tower")
	}
	m.HP = 0
	st.err = errors.New("disk unavailable")
	w.killMobState(a, m, 9000, 9000)
	if s := w.guilds.Wars.Tower; s.Owner != 2 || s.Kingdom != 8 || s.DefenderCaptures != 1 || s.Captures != 1 || m.HP == 0 || len(w.guilds.Wars.Medals) != 1 {
		t.Fatal("failed capture not rolled back")
	}
	st.err = nil
	m.HP = 0
	w.killMobState(a, m, 9000, 9000)
	if s := w.guilds.Wars.Tower; s.Owner != 1 || s.Kingdom != 7 || s.DefenderCaptures != 2 || s.Captures != 2 || len(w.guilds.Wars.Medals) != 2 {
		t.Fatal("defending recapture did not increment the native divisor")
	}
	b.X, b.Y = towerWarSpawnX-2, towerWarSpawnY
	if !w.towerTargetAllowed(b, m) {
		t.Fatal("opposing kingdom cannot recapture after a defense")
	}
	m.HP = 0
	w.killMobState(b, m, 9000, 9000)
	if s := w.guilds.Wars.Tower; s.Owner != 2 || s.Kingdom != 8 || s.DefenderCaptures != 2 || s.Captures != 3 || len(w.guilds.Wars.Medals) != 3 {
		t.Fatal("final opposing capture did not preserve the divisor")
	}
	clock.Advance(55 * time.Minute)
	w.tickGuildWars(clock.Now())
	if w.guilds.Wars.Tower.Notices&4 == 0 {
		t.Fatal("missing end warning")
	}
	clock.Advance(5 * time.Minute)
	st.err = errors.New("disk unavailable")
	w.tickGuildWars(clock.Now())
	if w.guilds.FindByID(1).Fame != 0 || w.guilds.FindByID(2).Fame != 0 || w.guilds.Wars.Tower.Finished || w.guilds.Wars.Tower.DefenderCaptures != 2 || len(w.guilds.Wars.Medals) != 3 {
		t.Fatal("failed reward published")
	}
	st.err = nil
	clock.Advance(time.Second)
	w.tickGuildWars(clock.Now())
	if w.guilds.FindByID(1).Fame != 0 || w.guilds.FindByID(2).Fame != 100 || !w.guilds.Wars.Tower.Finished || w.guilds.Wars.Tower.DefenderCaptures != 1 || w.warTower() != nil || len(w.guilds.Wars.Medals) != 4 {
		t.Fatal("reward or cleanup missing")
	}
	w.warInitialized = false
	clock.Advance(time.Second)
	w.tickGuildWars(clock.Now())
	if w.guilds.FindByID(1).Fame != 0 || w.guilds.FindByID(2).Fame != 100 {
		t.Fatal("restart doubled fame")
	}
	w.guilds.Wars.Castle.Owner = 2
	nextStart := time.Date(clock.Now().Year(), clock.Now().Month(), clock.Now().Day()+1, 20, 55, 0, 0, clock.Now().Location())
	clock.current = nextStart
	w.warInitialized = false
	w.tickGuildWars(clock.Now())
	if s := w.guilds.Wars.Tower; s.Owner != 0 || s.Kingdom != 8 || s.InitialKingdom != 8 || s.DefenderCaptures != 1 || s.Captures != 0 || s.Notices != 1 || w.warTower() != nil {
		t.Fatal("next round did not use castle realm, reset owner, or reset changed-realm divisor")
	}
}

func TestScheduledTowerSameRealmRoundPreservesDefenderCaptures(t *testing.T) {
	w, _, _, _, clock := scheduledWarWorld(t, "2026-09-14 20:55")
	w.guilds.Wars.Castle.Owner = 1
	w.guilds.Wars.Tower = model.TowerWarState{
		Day: "2026-09-13", Finished: true, Owner: 1, Kingdom: 7, InitialKingdom: 7,
		DefenderCaptures: 2, Captures: 5,
	}
	w.tickGuildWars(clock.Now())
	if s := w.guilds.Wars.Tower; s.Owner != 0 || s.Kingdom != 7 || s.InitialKingdom != 7 || s.DefenderCaptures != 2 || s.Captures != 0 || s.Notices != 1 || w.warTower() != nil {
		t.Fatal("same-realm round did not preserve the native defender divisor")
	}
}

func TestScheduledCityLifecycleAndCombatGates(t *testing.T) {
	w, st, a, b, clock := scheduledWarWorld(t, "2026-09-12 12:00")
	w.guilds.FindByID(2).Fame = 200
	w.guilds.Wars.Cities.Territories[0].Owner = 1
	if err := w.registerCityWar(b, 0, 100); err != nil {
		t.Fatal(err)
	}
	if w.guilds.FindByID(2).Fame != 100 {
		t.Fatal("bid increase was not deducted")
	}
	if err := w.registerCityWar(b, 0, 100); err != nil || w.guilds.FindByID(2).Fame != 100 || len(w.guilds.Wars.Cities.Entries) != 1 {
		t.Fatal("repeated cumulative bid must not charge or enroll twice")
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
	clock.Advance(25 * time.Minute)
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
	if w.guilds.FindByID(2).Fame != 50 {
		t.Fatal("city conquest must award the reference's 50 guild fame")
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
	for _, p := range [][2]uint16{{towerWarSpawnX, towerWarSpawnY}, {2450, 1855}} {
		if !terrain.Walkable(p[0], p[1]) {
			t.Errorf("tower blocked at %v", p)
		}
	}
}

func TestScheduledWarRegistrationRollbackAndLeaderAuthority(t *testing.T) {
	w, st, a, _, _ := scheduledWarWorld(t, "2026-09-12 12:00")
	w.tickGuildWars(w.now())
	w.guilds.FindByID(1).Fame = 100
	w.guilds.FindByID(1).Kingdom = 0
	if err := w.registerCityWar(a, 0, 100); err == nil || w.guilds.FindByID(1).Fame != 100 || len(w.guilds.Wars.Cities.Entries) != 0 {
		t.Fatal("neutral kingdom was accepted for city registration")
	}
	w.guilds.FindByID(1).Kingdom = 7
	st.err = errors.New("disk failure")
	if err := w.registerCityWar(a, 0, 100); err == nil || w.guilds.FindByID(1).Fame != 100 || len(w.guilds.Wars.Cities.Entries) != 0 {
		t.Fatal("bid rollback")
	}
	st.err = nil
	w.guilds.FindByID(1).Members[0].Rank = model.GuildRankMember
	if err := w.registerCityWar(a, 0, 100); err == nil {
		t.Fatal("forged leader projection accepted")
	}
}

func TestScheduledCityInterruptedRefundIsAtomicAndOnce(t *testing.T) {
	w, st, _, b, clock := scheduledWarWorld(t, "2026-09-12 12:00")
	w.guilds.FindByID(2).Fame = 100
	if err := w.registerCityWar(b, 0, 100); err != nil {
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

func TestScheduledCityWeeklyFameResetAtSundayMidnight(t *testing.T) {
	w, _, leader, _, clock := scheduledWarWorld(t, "2026-09-12 23:59")
	w.tickGuildWars(clock.Now())
	guild := w.guilds.FindByID(1)
	guild.Fame = 300
	if err := w.registerCityWar(leader, 0, 100); err != nil {
		t.Fatal(err)
	}
	if guild.Fame != 200 || len(w.guilds.Wars.Cities.Entries) != 1 {
		t.Fatalf("Saturday bid setup mismatch: fame=%d entries=%+v", guild.Fame, w.guilds.Wars.Cities.Entries)
	}

	clock.Advance(time.Minute + time.Second)
	w.tickGuildWars(clock.Now())
	city := w.guilds.Wars.Cities
	if got := w.guilds.FindByID(1).Fame; got != 0 || w.guilds.Wars.FameResetDay != "2026-09-13" || city.Phase != "waiting" {
		t.Fatalf("Sunday reset mismatch: fame=%d reset=%q phase=%q", got, w.guilds.Wars.FameResetDay, city.Phase)
	}
	if len(city.Entries) != 1 || city.Entries[0].Bid != 100 || city.Territories[0].Challenger != guild.ID || city.Territories[0].Bid != 100 {
		t.Fatalf("Sunday reset did not retain the accepted city bid: entries=%+v territory=%+v", city.Entries, city.Territories[0])
	}

	guild = w.guilds.FindByID(1)
	guild.Fame = 17
	clock.Advance(time.Second)
	w.tickGuildWars(clock.Now())
	if got := w.guilds.FindByID(1).Fame; got != 17 || w.guilds.Wars.FameResetDay != "2026-09-13" {
		t.Fatalf("weekly fame reset replayed within the same Sunday: fame=%d reset=%q", got, w.guilds.Wars.FameResetDay)
	}
}

func TestScheduledCityTieRetainsDefenderAndDefenseCaps(t *testing.T) {
	w, _, _, _, clock := scheduledWarWorld(t, "2026-09-13 20:31")
	w.guilds.Wars.Cities = model.CityWarState{Day: "2026-09-13", StartsAt: clock.Now().Add(-31 * time.Minute), Phase: "battle"}
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

func TestScheduledCityConquestRollbackAndRetry(t *testing.T) {
	for _, failure := range []string{"storage", "late-fame-overflow"} {
		t.Run(failure, func(t *testing.T) {
			w, st, a, _, clock := scheduledWarWorld(t, "2026-09-13 20:21")
			w.guilds.FindByID(1).Fame = 75
			w.guilds.FindByID(2).Fame = 100
			w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: 3, Name: "Third"})
			w.guilds.Wars.Castle.Owner = 1
			w.guilds.Wars.Cities = model.CityWarState{Day: "2026-09-13", StartsAt: clock.Now().Add(-21 * time.Minute), Phase: "battle"}
			w.guilds.Wars.Cities.Territories[0] = model.CityTerritory{Owner: 1, Challenger: 2}
			w.cityWarScores[0] = map[uint16]uint64{2: 1}
			if failure == "storage" {
				st.err = errors.New("city settlement unavailable")
			} else {
				w.guilds.FindByID(3).Fame = math.MaxUint32
				w.guilds.Wars.Cities.Territories[1].Challenger = 3
				w.cityWarScores[1] = map[uint16]uint64{3: 1}
			}
			before := w.criticalGuildSnapshot()
			packets := a.Session.QueuedPacketsForTest()
			if err := w.finishCityWar(); err == nil {
				t.Fatal("failed settlement was accepted")
			}
			if !reflect.DeepEqual(w.criticalGuildSnapshot(), before) || len(w.warNotices) != 0 || a.Session.QueuedPacketsForTest() != packets {
				t.Fatal("failed settlement changed fame, ownership or publications")
			}
			st.err = nil
			w.guilds.FindByID(3).Fame = 0
			if err := w.finishCityWar(); err != nil {
				t.Fatal(err)
			}
			if w.guilds.FindByID(1).Fame != 25 || w.guilds.FindByID(2).Fame != 150 || w.guilds.Wars.Castle.Owner != 2 {
				t.Fatal("retry did not settle frozen scores, fame and castle ownership together")
			}
			saves := st.gameSaves
			if err := w.finishCityWar(); err != nil || st.gameSaves != saves || w.guilds.FindByID(2).Fame != 150 {
				t.Fatal("finished settlement was replayed")
			}
		})
	}
}

func TestScheduledCityConquestCriticalAcknowledgement(t *testing.T) {
	for _, reject := range []bool{false, true} {
		t.Run(fmt.Sprintf("reject-%t", reject), func(t *testing.T) {
			w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 0)
			w.guilds.FindByID(1).Fame = 75
			w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: 2, Name: "Attacker", Fame: 100}, model.Guild{ID: 3, Name: "OtherCityOwner"})
			w.guilds.Wars.Castle.Owner = 1
			w.guilds.Wars.Cities = model.CityWarState{Day: "2026-09-13", StartsAt: w.now().Add(-21 * time.Minute), Phase: "battle"}
			for i := range w.guilds.Wars.Cities.Territories {
				w.guilds.Wars.Cities.Territories[i].Owner = 3
			}
			w.guilds.Wars.Cities.Territories[0] = model.CityTerritory{Owner: 1, Challenger: 2}
			w.cityWarScores[0] = map[uint16]uint64{2: 1}
			w.cityWarRunning = true
			before := w.criticalGuildSnapshot()
			packets := p.Session.QueuedPacketsForTest()
			if reject {
				base.failure = criticalCertifiedRejection(t)
			}
			if err := w.finishCityWar(); err != nil {
				t.Fatal(err)
			}
			proposal := awaitCriticalWrite(t, base.entered)
			if proposal.guilds == nil || len(proposal.accounts) != 0 || proposal.guilds.FindByID(1).Fame != 25 || proposal.guilds.FindByID(2).Fame != 150 || proposal.guilds.Wars.Castle.Owner != 2 || proposal.guilds.Wars.Cities.Phase != "finished" {
				t.Fatal("settlement did not prepare one detached registry proposal")
			}
			assertRetained := func() {
				t.Helper()
				if !reflect.DeepEqual(w.criticalGuildSnapshot(), before) || !w.cityWarRunning || len(w.warNotices) != 0 || p.Session.QueuedPacketsForTest() != packets || fallback.gameSaves != 0 {
					t.Fatal("unacknowledged settlement changed live state or announced a winner")
				}
			}
			assertRetained()
			if err := w.finishCityWar(); !errors.Is(err, errCriticalPersistenceBusy) {
				t.Fatal("pending settlement did not fence a duplicate")
			}
			release()
			result := awaitCriticalResult(t, q)
			assertRetained()
			w.applyCriticalCompletion(result)
			if reject {
				assertRetained()
				if w.criticalGuildReserved() || w.shuttingDown {
					t.Fatal("certified rejection did not release the settlement fence")
				}
				base.failure = nil
				if err := w.finishCityWar(); err != nil {
					t.Fatal(err)
				}
				awaitCriticalWrite(t, base.entered)
				w.applyCriticalCompletion(awaitCriticalResult(t, q))
			}
			if w.shuttingDown || w.criticalGuildReserved() || w.cityWarRunning || len(q.pending) != 0 || !reflect.DeepEqual(w.criticalGuildSnapshot(), proposal.guilds) || len(w.warNotices) == 0 {
				t.Fatal("acknowledgement did not apply and publish exactly the committed settlement")
			}
			if err := w.finishCityWar(); err != nil || len(base.entered) != 0 || w.guilds.FindByID(2).Fame != 150 {
				t.Fatal("acknowledged settlement was awarded again")
			}
		})
	}
}

func TestScheduledWarPrivateSpaceIsolation(t *testing.T) {
	for _, arena := range []string{"city", "tower"} {
		t.Run(arena, func(t *testing.T) {
			w, _, a, b, clock := scheduledWarWorld(t, "2026-09-14 21:05")
			x, y := towerWarSpawnX, towerWarSpawnY
			if arena == "city" {
				x, y = cityWarZones[0].defendX, cityWarZones[0].defendY
				w.guilds.Wars.Cities = model.CityWarState{StartsAt: clock.Now().Add(-6 * time.Minute), Phase: "battle"}
				w.cityWarRunning = true
				w.cityFighters = map[*Player]cityFighter{a: {city: 0, guild: 1}, b: {city: 0, guild: 2}}
			} else {
				w.guilds.Wars.Tower = model.TowerWarState{Day: "2026-09-14", Started: true, EndsAt: clock.Now().Add(55 * time.Minute), Kingdom: 7, InitialKingdom: 7, DefenderCaptures: 1}
			}
			a.X, a.Y, b.X, b.Y = x, y, x+1, y
			w.itemInstances = map[string]*ItemInstance{"private-war-coordinates": {RuntimeID: "private-war-coordinates", MemberIDs: []uint16{a.ID, b.ID}}}
			w.rebuildPlayerInstanceIndex()
			if allowed, handled := w.guildWarPvP(a, b); allowed || handled {
				t.Fatal("public war rules overrode combat inside a shared private runtime")
			}
			if !w.guildWarStepAllowed(a, x, y) {
				t.Fatal("public city gate blocked private instance movement")
			}
			if arena == "tower" {
				m := &Mob{GuildWarTower: true, X: x, Y: y}
				if w.towerTargetAllowed(a, m) {
					t.Fatal("private participant may attack the public tower")
				}
				w.ejectTowerArena()
			} else {
				w.cleanCityArena()
			}
			if a.X != x || a.Y != y || b.X != x+1 || b.Y != y || w.gameplaySpaceForPlayer(a) == "" || w.gameplaySpaceForPlayer(b) == "" {
				t.Fatal("public war cleanup ejected private participants")
			}
			w.itemInstances["private-war-coordinates"].MemberIDs = []uint16{a.ID}
			w.rebuildPlayerInstanceIndex()
			if allowed, _ := w.guildWarPvP(a, b); allowed {
				t.Fatal("public war authorized cross-runtime PvP")
			}
			if arena == "tower" {
				m := &Mob{GuildWarTower: true, InstanceID: "private-war-coordinates", X: x, Y: y}
				if w.towerTargetAllowed(b, m) {
					t.Fatal("public participant may attack a private tower actor")
				}
				w.ejectTowerArena()
			} else {
				w.cleanCityArena()
			}
			if b.X == x+1 && b.Y == y {
				t.Fatal("public arena outsider was not removed")
			}
			if a.X != x || a.Y != y || w.gameplaySpaceForPlayer(a) == "" {
				t.Fatal("public cleanup affected the remaining private participant")
			}
		})
	}
}

func TestCastleOwnerPrunedOnGuildDissolution(t *testing.T) {
	w, _, _, _, _ := scheduledWarWorld(t, "2026-09-13 20:21")
	w.guilds.Wars.Castle.Owner = 1
	w.pruneGuildWarReferences()
	if w.guilds.Wars.Castle.Owner != 1 {
		t.Fatal("existing castle owner was pruned")
	}
	w.guilds.Guilds = w.guilds.Guilds[1:]
	w.pruneGuildWarReferences()
	if w.guilds.Wars.Castle.Owner != 0 {
		t.Fatal("dissolved guild retained castle ownership")
	}
}
