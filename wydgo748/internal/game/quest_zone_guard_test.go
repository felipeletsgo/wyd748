package game

import (
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func TestQuestZoneExplicitNPCAndLegacyBinding(t *testing.T) {
	quest := &model.QuestDef{ID: 77, NPC: "Guard__"}
	w := &World{questsByNPC: map[string]*model.QuestDef{model.NormalizeQuestNPC(quest.NPC): quest}}
	if got := w.questZoneQuest(&model.QuestZone{Name: "Training area", NPC: quest.NPC}); got != quest {
		t.Fatal("explicit guard ignored")
	}
	if got := w.questZoneQuest(&model.QuestZone{Name: quest.NPC}); got != quest {
		t.Fatal("legacy binding changed")
	}
	if got := w.questZoneQuest(&model.QuestZone{Name: quest.NPC, NPC: "Missing"}); got != nil {
		t.Fatal("explicit missing guard fell back to area name")
	}
}

func TestMolarConfiguredAreaGuard(t *testing.T) {
	zones, err := data.LoadQuestZones("../../data/content/quest_zones.json")
	if err != nil {
		t.Fatal(err)
	}
	quests, err := data.LoadQuests("../../data/content/quests.json")
	if err != nil {
		t.Fatal(err)
	}
	for _, scenario := range []string{"wall jump", "relogin", "authorized", "below minimum", "above maximum", "reset", "outside NPC"} {
		t.Run(scenario, func(t *testing.T) {
			w := newZoneTestWorld()
			w.questZones = zones.Zones
			w.questsByNPC = make(map[string]*model.QuestDef)
			for i := range quests.Quests {
				q := &quests.Quests[i]
				w.questsByNPC[model.NormalizeQuestNPC(q.NPC)] = q
			}
			w.nextQuestZoneReset = w.now().Add(questZoneResetInterval)
			p := addZonePlayer(w, 1, 817, 4062, 500)
			p.Char.Score.Level = 200
			zone := w.questZoneAt(p.X, p.Y)
			if zone == nil || zone.Name != "Gargoyle_Watcher" || w.questZoneQuest(zone) == nil {
				t.Fatal("Molar destination has no guard linked to its admission quest")
			}
			switch scenario {
			case "authorized", "below minimum", "above maximum", "reset":
				w.grantQuestZoneEntry(p, p.X, p.Y)
			case "relogin":
				// Persisted character coordinates do not grant a runtime entry flag.
				p.Char.X, p.Char.Y = p.X, p.Y
			case "outside NPC":
				p.X, p.Y = 787, 4058
			}
			if scenario == "below minimum" {
				p.Char.Score.Level = 199
			} else if scenario == "above maximum" {
				p.Char.Score.Level = 205
			}
			if scenario == "reset" {
				w.tickQuestZoneReset(w.now())
			} else {
				w.tickQuestZoneGuard(w.now())
			}
			switch scenario {
			case "authorized":
				if p.X != 817 || p.Y != 4062 || p.QuestZoneEntry != zone.Name || !p.QuestZoneTimer {
					t.Fatal("authorized player did not retain the guarded stay and countdown")
				}
			case "outside NPC":
				if p.X != 787 || p.Y != 4058 {
					t.Fatal("guard moved a player outside the entrance")
				}
			default:
				// The shared guard retires a recalled area's HUD on its next sweep.
				w.tickQuestZoneGuard(w.now().Add(questZoneGuardInterval))
				if !inArmiaCity(p.X, p.Y) || p.QuestZoneEntry != "" || p.QuestZoneTimer {
					t.Fatalf("invalid or expired stay was not recalled: position=(%d,%d) entry=%q", p.X, p.Y, p.QuestZoneEntry)
				}
			}
		})
	}
}

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
