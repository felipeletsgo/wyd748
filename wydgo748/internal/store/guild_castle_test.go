package store

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestCastleRoundAndConquestPersistTogether(t *testing.T) {
	s, _, _ := newTestStore(t)
	r := sampleGuild()
	r.Guilds[0].Fame = 900
	r.Guilds = append(r.Guilds, model.Guild{ID: 2, Name: "Attacker", Fame: 100,
		Members: []model.GuildMember{{Character: "CastleLeader", Account: "account2", Rank: model.GuildRankLeader}}})
	r.Wars.Castle.Owner = 1
	r.Wars.Cities.Territories[0].Owner = 2
	start := time.Date(2026, 10, 25, 21, 0, 0, 0, time.FixedZone("war", -3*3600))
	if err := r.BeginCastleWar(start); err != nil {
		t.Fatal(err)
	}
	for _, phase := range []model.CastleWarPhase{model.CastlePreparing, model.CastleAssault, model.CastleSeal, model.CastleClosing} {
		r.Wars.Castle.Phase = phase
		if err := s.SaveGameState(r); err != nil {
			t.Fatal(err)
		}
		loaded, err := s.LoadGuilds()
		if err != nil {
			t.Fatal(err)
		}
		if loaded.Wars.Castle.Owner != 1 || loaded.Wars.Castle.Phase != phase || loaded.Wars.Castle.Day != "2026-10-25" || !loaded.Wars.Castle.StartsAt.Equal(start) || loaded.Wars.Castle.StartsAt.Format("2006-01-02 15:04 -0700") != start.Format("2006-01-02 15:04 -0700") {
			t.Fatal("castle round or local calendar lost on reload")
		}
		r = loaded
	}
	now := start.Add(50 * time.Minute)
	var progress model.CastleSealProgress
	for range model.CastleSealSteps {
		progress.Observe(r, "CastleLeader", 1046, 1690, true, now)
	}
	if err := r.CaptureCastle(progress, now); err != nil {
		t.Fatal(err)
	}
	if err := s.SaveGameState(r); err != nil {
		t.Fatal(err)
	}
	loaded, err := s.LoadGuilds()
	if err != nil {
		t.Fatal(err)
	}
	if loaded.FindByID(1).Fame != 250 || loaded.FindByID(2).Fame != 750 || loaded.Wars.Castle.Phase != model.CastleFinished || !loaded.Wars.Castle.Captured || loaded.Wars.Castle.Owner != 2 || loaded.Wars.Castle.Victories != 1 {
		t.Fatal("castle completion was split from ownership or guild fame")
	}
	if err := loaded.CaptureCastle(progress, now); err != nil || loaded.FindByID(2).Fame != 750 {
		t.Fatal("reloaded castle result awarded fame again", err)
	}
	if err := loaded.CaptureCastle(model.CastleSealProgress{}, now); err == nil {
		t.Fatal("empty progress became a valid capture after restart")
	}
}
