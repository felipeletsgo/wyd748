package model

import (
	"encoding/json"
	"math"
	"reflect"
	"testing"
	"time"
)

func castleFixture(t *testing.T) (*GuildRegistry, time.Time) {
	t.Helper()
	start := time.Date(2026, 10, 25, 21, 0, 0, 0, time.FixedZone("war", -3*3600))
	r := &GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{
		{ID: 1, Name: "Defender", Fame: 1000, Members: []GuildMember{{Character: "Defender", Rank: GuildRankLeader}}},
		{ID: 2, Name: "Attacker", Fame: 100, Members: []GuildMember{{Character: "Leader", Rank: GuildRankLeader}, {Character: "Member", Rank: GuildRankMember}}},
		{ID: 3, Name: "Bidder", Fame: 500, Members: []GuildMember{{Character: "Bidder", Rank: GuildRankLeader}}},
	}}
	r.Wars.Castle.Owner = 1
	r.Wars.Cities.Territories[0] = CityTerritory{Owner: 2, Challenger: 3}
	if err := r.BeginCastleWar(start); err != nil {
		t.Fatal(err)
	}
	return r, start
}

func completeCastleSeal(r *GuildRegistry, now time.Time) CastleSealProgress {
	var p CastleSealProgress
	for range CastleSealSteps {
		p.Observe(r, "Leader", 1046, 1690, true, now)
	}
	return p
}

func TestCastlePhaseBoundariesAndReplay(t *testing.T) {
	r, start := castleFixture(t)
	for _, tc := range []struct {
		elapsed time.Duration
		phase   CastleWarPhase
		changed bool
	}{
		{-time.Second, CastlePreparing, false},
		{6*time.Minute - time.Nanosecond, CastlePreparing, false},
		{6 * time.Minute, CastleAssault, true},
		{15*time.Minute - time.Nanosecond, CastleAssault, false},
		{15 * time.Minute, CastleSeal, true},
		{50*time.Minute - time.Nanosecond, CastleSeal, false},
		{50 * time.Minute, CastleClosing, true},
		{55*time.Minute - time.Nanosecond, CastleClosing, false},
		{55 * time.Minute, CastleFinished, true},
		{56 * time.Minute, CastleFinished, false},
	} {
		changed, err := r.AdvanceCastleWar(start.Add(tc.elapsed))
		if err != nil || changed != tc.changed || r.Wars.Castle.Phase != tc.phase {
			t.Fatalf("elapsed %v: changed=%t phase=%s error=%v", tc.elapsed, changed, r.Wars.Castle.Phase, err)
		}
	}
	if r.Wars.Castle.Victories != 1 || r.FindByID(1).Fame != 1000 || r.FindByID(2).Fame != 100 {
		t.Fatal("defense incorrectly awarded conquest fame")
	}
	if err := r.BeginCastleWar(start); err != nil || r.Wars.Castle.Phase != CastleFinished {
		t.Fatal("replay reopened the round", err)
	}
	if err := r.BeginCastleWar(start.Add(time.Hour)); err == nil {
		t.Fatal("same-day replacement accepted")
	}
	if err := r.BeginCastleWar(start.AddDate(0, 0, -1)); err == nil {
		t.Fatal("older round accepted")
	}
	if err := r.BeginCastleWar(start.AddDate(0, 0, 7)); err != nil || r.Wars.Castle.Victories != 1 {
		t.Fatal("new authorized round lost ownership history", err)
	}
}

func TestCastleMissedOpeningAndDeadlineRecovery(t *testing.T) {
	for _, elapsed := range []time.Duration{10*time.Minute - time.Nanosecond, 10 * time.Minute, 55 * time.Minute} {
		r, start := castleFixture(t)
		want := CastleCancelled
		if elapsed < 10*time.Minute {
			want = CastleAssault
		}
		if changed, err := r.AdvanceCastleWar(start.Add(elapsed)); err != nil || !changed || r.Wars.Castle.Phase != want {
			t.Fatal("incorrect missed-opening recovery", elapsed, err)
		}
		if want == CastleCancelled && r.Wars.Castle.Victories != 0 {
			t.Fatal("cancelled round counted as a defense")
		}
	}
	r, start := castleFixture(t)
	r.Wars.Castle.Phase = CastleAssault
	if _, err := r.AdvanceCastleWar(start.Add(time.Hour)); err != nil || r.Wars.Castle.Phase != CastleFinished {
		t.Fatal("delayed coordinator extended the battle", err)
	}
	r.Wars.Castle.Phase, r.Wars.Castle.Victories = CastleClosing, math.MaxUint32
	before := r.Wars.Castle
	if _, err := r.AdvanceCastleWar(start.Add(time.Hour)); err == nil || before != r.Wars.Castle {
		t.Fatal("counter overflow partially settled a defense")
	}
}

func TestCastleSealRequiresConsecutiveAuthorizedSamples(t *testing.T) {
	r, start := castleFixture(t)
	now := start.Add(20 * time.Minute)
	r.Wars.Castle.Phase = CastleSeal
	for _, tc := range []struct {
		name      string
		character string
		x, y      uint16
		eligible  bool
		now       time.Time
	}{
		{"member", "Member", 1046, 1690, true, now},
		{"bidder", "Bidder", 1046, 1690, true, now},
		{"defender", "Defender", 1046, 1690, true, now},
		{"unknown", "Unknown", 1046, 1690, true, now},
		{"wrong-x", "Leader", 1047, 1690, true, now},
		{"wrong-y", "Leader", 1046, 1691, true, now},
		{"ineligible", "Leader", 1046, 1690, false, now},
		{"too-early", "Leader", 1046, 1690, true, start.Add(15*time.Minute - time.Nanosecond)},
		{"deadline", "Leader", 1046, 1690, true, start.Add(55 * time.Minute)},
	} {
		t.Run(tc.name, func(t *testing.T) {
			p := completeCastleSeal(r, now)
			if p.Observe(r, tc.character, tc.x, tc.y, tc.eligible, tc.now) || p != (CastleSealProgress{}) {
				t.Fatal("invalid sample retained seal progress")
			}
		})
	}
	var p CastleSealProgress
	for i := 1; i <= CastleSealSteps+1; i++ {
		if done := p.Observe(r, "LEADER", 1046, 1690, true, now); done != (i >= CastleSealSteps) {
			t.Fatalf("incorrect completion at sample %d", i)
		}
	}
	if p.steps != CastleSealSteps {
		t.Fatal("counter did not saturate")
	}
	r.Wars.Cities.Territories[0].Owner = 3
	if p.Observe(r, "Leader", 1046, 1690, true, now) || p.steps != 0 {
		t.Fatal("lost city ownership did not invalidate capture")
	}
	r.Wars.Cities.Territories[0].Owner = 2
	p = completeCastleSeal(r, now)
	r.Wars.Castle.Day, r.Wars.Castle.StartsAt = "2026-11-01", start.AddDate(0, 0, 7)
	if p.Observe(r, "Leader", 1046, 1690, true, now.AddDate(0, 0, 7)) || p.steps != 1 {
		t.Fatal("round change resumed old seal progress")
	}
}

func TestCastleCaptureSettlementAtomicAndReplaySafe(t *testing.T) {
	for _, loserFame := range []uint32{0, 100, 1000} {
		r, start := castleFixture(t)
		now := start.Add(50 * time.Minute)
		r.Wars.Castle.Phase, r.Wars.Castle.Victories = CastleClosing, 20
		r.FindByID(1).Fame = loserFame
		p := completeCastleSeal(r, now)
		if err := r.CaptureCastle(p, now); err != nil {
			t.Fatal(err)
		}
		if r.FindByID(2).Fame != 750 || r.FindByID(1).Fame != loserFame-min(loserFame, CastleWarConquestFame) || r.Wars.Castle.Owner != 2 || r.Wars.Castle.Victories != 1 || !r.Wars.Castle.Captured || r.Wars.Castle.Phase != CastleFinished || len(r.Wars.Medals) != 0 {
			t.Fatal("incorrect conquest settlement")
		}
		encoded, err := json.Marshal(r)
		if err != nil {
			t.Fatal(err)
		}
		var restored GuildRegistry
		if err := json.Unmarshal(encoded, &restored); err != nil {
			t.Fatal(err)
		}
		if err := restored.Wars.Validate(&restored); err != nil {
			t.Fatal(err)
		}
		if err := restored.CaptureCastle(p, now.Add(time.Hour)); err != nil || restored.FindByID(2).Fame != 750 {
			t.Fatal("restored completion awarded the round again", err)
		}
	}
}

func TestCastleCaptureRevalidatesAuthorityAndOverflow(t *testing.T) {
	for _, reason := range []string{"overflow", "rank", "city", "deadline", "incomplete", "phase"} {
		t.Run(reason, func(t *testing.T) {
			r, start := castleFixture(t)
			now := start.Add(20 * time.Minute)
			r.Wars.Castle.Phase = CastleSeal
			p := completeCastleSeal(r, now)
			switch reason {
			case "overflow":
				r.FindByID(2).Fame = math.MaxUint32 - CastleWarConquestFame + 1
			case "rank":
				r.FindByID(2).Members[0].Rank = GuildRankSubLast
			case "city":
				r.Wars.Cities.Territories[0].Owner = 3
			case "deadline":
				now = start.Add(55 * time.Minute)
			case "incomplete":
				p.steps--
			case "phase":
				r.Wars.Castle.Phase = CastleAssault
			}
			before, _ := json.Marshal(r)
			if err := r.CaptureCastle(p, now); err == nil {
				t.Fatal("invalid settlement accepted")
			}
			after, _ := json.Marshal(r)
			if string(before) != string(after) {
				t.Fatal("rejected settlement changed registry")
			}
		})
	}
}

func TestCastleLegacyStateAndInvalidCalendars(t *testing.T) {
	var legacy GuildWarState
	if err := json.Unmarshal([]byte(`{"castle":{"owner":1}}`), &legacy); err != nil || legacy.Castle.validate() != nil {
		t.Fatal("ownership-only schema is no longer readable", err)
	}
	r, start := castleFixture(t)
	valid := r.Wars.Castle
	for _, bad := range []CastleWarState{
		{Day: "bad", StartsAt: start, Phase: CastlePreparing},
		{Day: valid.Day, StartsAt: start.Add(time.Minute), Phase: CastlePreparing},
		{Day: valid.Day, StartsAt: start, Phase: "unknown"},
		{Day: valid.Day, StartsAt: start, Phase: CastleSeal, Captured: true},
		{Phase: CastleSeal},
	} {
		r.Wars.Castle = bad
		if err := r.Wars.Validate(r); err == nil {
			t.Fatal("invalid castle state accepted", bad)
		}
		if err := r.BeginCastleWar(start); err == nil || !reflect.DeepEqual(r.Wars.Castle, bad) {
			t.Fatal("admission silently repaired invalid persisted state")
		}
	}
	if err := (&GuildRegistry{}).BeginCastleWar(start.Add(time.Second)); err == nil {
		t.Fatal("non-hour admission accepted")
	}
}
