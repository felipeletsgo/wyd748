package game

import (
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func TestSharedNativeTrainersKeepQuestRouting(t *testing.T) {
	npcs, generators := officialNPCDataset(t)
	quests, err := data.LoadQuests("../../data/content/quests.json")
	if err != nil {
		t.Fatal(err)
	}
	definitions := make([]model.NPCDef, 0, len(npcs))
	for _, def := range npcs {
		definitions = append(definitions, *def)
	}
	index, err := indexQuests(quests, definitions)
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	for _, tc := range []struct {
		name     string
		merchant uint32
		questID  int
	}{
		{"Treinador_I", 36, 9}, {"Treinador_II", 40, 15}, {"Treinador_III", 41, 16},
	} {
		t.Run(tc.name, func(t *testing.T) {
			def := npcs[tc.name]
			if def == nil || def.Score.Merchant != tc.merchant || def.MakeScore(0).Merchant&15 != 11 {
				t.Fatal("native function or 7.48 quest projection changed")
			}
			var spawn *model.NPCGener
			for i := range generators {
				if generators[i].Enabled && generators[i].Leader == tc.name {
					if spawn != nil {
						t.Fatal("duplicate trainer spawn")
					}
					spawn = &generators[i]
				}
			}
			if spawn == nil {
				t.Fatal("trainer has no shared spawn")
			}
			w, p, backend := handlerTestWorld(t)
			w.items, w.skills, w.questsByNPC = catalog.Items, catalog.Skills, index
			quest := w.questForNPC(def)
			if quest == nil || quest.ID != tc.questID {
				t.Fatal("quest identity lost")
			}
			p.Char.Score.Level = 1
			p.Char.Evolution = ""
			p.Char.Inv = [64]model.Item{}
			p.Char.Equip[6] = model.Item{Index: 701}
			for i, item := range quest.Requires.Items {
				p.Char.Inv[i] = model.Item{Index: item.Index}
			}
			if quest.Requires.AfterQuest != 0 {
				markQuestCompleted(p.Char, quest.Requires.AfterQuest)
			}
			origin := spawn.Segments[0]
			p.X, p.Y = origin.X-1, origin.Y
			w.updatePlayerSpatial(p)
			mob := &Mob{ID: 1100, X: origin.X, Y: origin.Y, Def: def}
			w.registerMobSpatial(mob)
			p.show(mob.ID)
			w.onUseNPC(p.Session, useNPCPacket(mob.ID))
			if !questCompleted(p.Char, quest.ID) || backend.saves != 1 || p.CraftNPC != 0 {
				t.Fatalf("trainer did not complete and persist quest: completed=%v saves=%d craft=%d", questCompleted(p.Char, quest.ID), backend.saves, p.CraftNPC)
			}
			w.onUseNPC(p.Session, useNPCPacket(mob.ID))
			if backend.saves != 1 {
				t.Fatal("repeated click duplicated the quest reward")
			}
		})
	}
}
