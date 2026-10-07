package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func newGuardTestWorld() *World {
	w := newZoneTestWorld()
	w.questsByNPC = map[string]*model.QuestDef{
		"gravedigger": {ID: 1, NPC: "Gravedigger", Repeatable: true,
			Requires: model.QuestRequirements{MinLevel: 39, MaxLevel: 115}},
	}
	return w
}

func TestQuestZoneGuardRecallsPlayerWithoutEntry(t *testing.T) {
	w := newGuardTestWorld()
	intruder := addZonePlayer(w, 1, 2400, 2100, 500)
	intruder.Char.Score.Level = 50

	w.tickQuestZoneGuard(time.Now())

	if questZoneInterior(w.questZones[0], intruder.X, intruder.Y) {
		t.Fatalf("player inside the zone without the NPC entry stayed at (%d,%d)", intruder.X, intruder.Y)
	}
}

func TestQuestZoneGuardKeepsEnteredPlayerInRange(t *testing.T) {
	w := newGuardTestWorld()
	p := addZonePlayer(w, 1, 2400, 2100, 500)
	p.Char.Score.Level = 50
	w.grantQuestZoneEntry(p, p.X, p.Y)

	w.tickQuestZoneGuard(time.Now())

	if p.X != 2400 || p.Y != 2100 || p.QuestZoneEntry != "Gravedigger" {
		t.Fatalf("entered player in range was moved to (%d,%d), entry %q", p.X, p.Y, p.QuestZoneEntry)
	}
}

func TestQuestZoneGuardRecallsPlayerPastLevelLimit(t *testing.T) {
	w := newGuardTestWorld()
	p := addZonePlayer(w, 1, 2400, 2100, 500)
	p.Char.Score.Level = 115
	w.grantQuestZoneEntry(p, p.X, p.Y)

	w.tickQuestZoneGuard(time.Now())
	if p.X != 2400 || p.Y != 2100 {
		t.Fatalf("player at the cap (internal 115, shown 116) was expelled")
	}

	p.Char.Score.Level = 116
	w.tickQuestZoneGuard(time.Now())
	if questZoneInterior(w.questZones[0], p.X, p.Y) || p.QuestZoneEntry != "" {
		t.Fatalf("player past the cap stayed at (%d,%d), entry %q", p.X, p.Y, p.QuestZoneEntry)
	}
}

func TestQuestZoneGuardIgnoresBorderAndOutside(t *testing.T) {
	w := newGuardTestWorld()
	border := addZonePlayer(w, 1, 2379, 2100, 500)
	outside := addZonePlayer(w, 2, 100, 100, 500)

	w.tickQuestZoneGuard(time.Now())

	if border.X != 2379 || border.Y != 2100 {
		t.Fatalf("border tile player was expelled")
	}
	if outside.X != 100 || outside.Y != 100 {
		t.Fatalf("outside player was moved")
	}
}

func TestQuestZoneResetClearsEntries(t *testing.T) {
	w := newGuardTestWorld()
	p := addZonePlayer(w, 1, 2400, 2100, 500)
	w.grantQuestZoneEntry(p, p.X, p.Y)
	far := addZonePlayer(w, 2, 100, 100, 500)
	far.QuestZoneEntry = "Gravedigger"

	w.tickQuestZoneReset(time.Now())

	if p.QuestZoneEntry != "" || far.QuestZoneEntry != "" {
		t.Fatalf("reset kept entries: %q %q", p.QuestZoneEntry, far.QuestZoneEntry)
	}
}
