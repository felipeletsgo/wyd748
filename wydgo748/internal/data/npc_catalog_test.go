package data

import (
	"encoding/json"
	"os"
	"path/filepath"
	"regexp"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestNPCCatalogResolvesBothChannelGenerators(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	if err := ValidateNPCItems(npcs, npcCatalogItems(t)); err != nil {
		t.Fatal(err)
	}
	for _, file := range []string{"NPCGener.txt", "NPCGenerTest.txt"} {
		t.Run(file, func(t *testing.T) {
			generators, err := LoadChannelNPCGener(filepath.Join("../../data/spawns", file))
			if err != nil {
				t.Fatal(err)
			}
			if len(generators) == 0 {
				t.Fatal("no generators exercised")
			}
			if err := ValidateNPCGenerators(npcs, generators); err != nil {
				t.Fatal(err)
			}
		})
	}
}

func TestCloneNPCsSeparatesMutableChannelState(t *testing.T) {
	selector := byte(1)
	source := []model.NPCDef{{Name: "Shop", Score: &model.Score{MaxHP: 100},
		ClientMerchant: &selector, Vende: []model.Item{{Index: 1}}, Carry: []model.Item{{Index: 2}}}}
	first, second := CloneNPCs(source), CloneNPCs(source)
	first[0].Score.MaxHP = 200
	first[0].Vende[0].Index = 3
	first[0].Carry[0].Index = 4
	*first[0].ClientMerchant = 5
	for _, catalog := range [][]model.NPCDef{source, second} {
		npc := catalog[0]
		if npc.Score.MaxHP != 100 || npc.Vende[0].Index != 1 || npc.Carry[0].Index != 2 || *npc.ClientMerchant != 1 {
			t.Fatal("channel mutation reached another catalog")
		}
	}
}

func TestNPCGeneratorsRejectMissingTemplate(t *testing.T) {
	generators := []model.NPCGener{{Enabled: true, Leader: "Missing", Follower: "Missing"}}
	if err := ValidateNPCGenerators(nil, generators); err == nil || !strings.Contains(err.Error(), "missing template") {
		t.Fatalf("unresolved generator accepted: %v", err)
	}
	generators[0].Enabled = false
	if err := ValidateNPCGenerators(nil, generators); err != nil {
		t.Fatal(err)
	}
}

func TestNPCItemsRejectUnsupportedItems(t *testing.T) {
	npcs := []model.NPCDef{{Name: "Official", Carry: []model.Item{{Index: 4906}}}}
	if err := ValidateNPCItems(npcs, nil); err == nil {
		t.Fatal("unsupported item admitted")
	}
	npcs[0].Carry[0].Index = 0
	if err := ValidateNPCItems(npcs, nil); err != nil {
		t.Fatal(err)
	}
}

func TestNPCDirectoryRejectsDuplicateTemplate(t *testing.T) {
	fixtures, err := LoadNPCs("../../data/npcs/Aylin.json")
	if err != nil {
		t.Fatal(err)
	}
	raw, err := json.Marshal(fixtures[0])
	if err != nil {
		t.Fatal(err)
	}
	dir := t.TempDir()
	for _, file := range []string{"first.json", "second.json"} {
		if err := os.WriteFile(filepath.Join(dir, file), raw, 0600); err != nil {
			t.Fatal(err)
		}
	}
	if _, err := LoadNPCs(dir); err == nil || !strings.Contains(err.Error(), "duplicate template") {
		t.Fatalf("duplicate identity accepted: %v", err)
	}
}

func TestNPCCatalogConsolidatesIdenticalReimersShop(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	found := map[string]bool{}
	for _, npc := range npcs {
		if npc.Name == "Reimers" || npc.Name == "reimers" {
			found[npc.Name] = true
		}
	}
	if len(found) != 1 || !found["reimers"] {
		t.Fatalf("identical unused Reimers variant retained: %v", found)
	}
}

func TestChannel2ServiceRolesAndVisibleNames(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	byName := make(map[string]model.NPCDef, len(npcs))
	suffixed := regexp.MustCompile(`_[0-9a-f]{4}$`)
	for _, npc := range npcs {
		byName[npc.Name] = npc
		if suffixed.MatchString(npc.Name) {
			t.Errorf("artificial template identifier retained: %q", npc.Name)
		}
	}
	knight := byName["KnightLeader"]
	if knight.Name != "KnightLeader" || knight.VisibleName() != "KnightLeader" || knight.InteractionName() != "Knight_Leader" {
		t.Fatalf("KnightLeader identity or service changed: %+v", knight)
	}
	for _, name := range []string{"AkeGuardTower", "ActKerdanChief", "Aranha_Dourada", "Berserker_Titan", "Griphon_Master", "Dragao_Vermelho"} {
		npc := byName[name]
		if npc.Name != name || npc.VisibleName() != name || npc.DisplayName != "" {
			t.Errorf("template %q does not use its own name directly: %+v", name, npc)
		}
	}
	for _, name := range []string{"Agata", "Conjurer_Ehre", "Aylin", "Compositor", "Tiny", "GateKeeper", "TownWatcher", "TownWatcher_", "Mercenario", "prisang_Nell", "Conjurer_Rassel", "King_Glantuar", "King_Harabard"} {
		npc, ok := byName[name]
		if !ok || npc.Tipo != model.TipoNPC || npc.IsMonster() || npc.IsCityGuard() {
			t.Errorf("service %q is not passive: %+v", name, npc)
		}
	}
	for _, name := range []string{"Bragi", "Heimdall", "Vidar", "Valdor", "Pikeman", "Wizard"} {
		npc, ok := byName[name]
		if !ok || !npc.IsMonster() {
			t.Errorf("legitimate monster %q lost its combat role", name)
		}
	}
}

func TestNPCDisplayNameValidation(t *testing.T) {
	fixtures, err := LoadNPCs("../../data/npcs/Aylin.json")
	if err != nil || len(fixtures) != 1 {
		t.Fatalf("load valid NPC fixture: %v", err)
	}
	npc := fixtures[0]
	for _, name := range []string{"", "KnightLeader", "Town_Watcher", "ActKerdanChief", "Aranha_Dourada", "Berserker_Titan"} {
		npc.DisplayName = name
		if err := validateNPCs("test", []model.NPCDef{npc}); err != nil {
			t.Errorf("valid display name %q: %v", name, err)
		}
	}
	for _, name := range []string{"SixteenBytesName", "Bad\x00Name", "Bad\nName", "Bad\x7fName", "Mágico"} {
		npc.DisplayName = name
		err := validateNPCs("test", []model.NPCDef{npc})
		if err == nil || !strings.Contains(err.Error(), "display name") {
			t.Errorf("invalid display name %q: %v", name, err)
		}
	}
	npc.Name = "InternalCatalogIdentifier"
	npc.DisplayName = "Aranha_Dourada"
	if err := validateNPCs("test", []model.NPCDef{npc}); err != nil {
		t.Fatalf("long internal ID with valid visible name: %v", err)
	}
	npc.DisplayName = ""
	if err := validateNPCs("test", []model.NPCDef{npc}); err == nil {
		t.Fatal("overlong fallback name accepted without a display override")
	}
}

// Item compatibility is validated by the loader before any channel can start.
func npcCatalogItems(t *testing.T) map[uint16]model.ItemDef {
	t.Helper()
	catalog, err := LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	return catalog.Items
}
