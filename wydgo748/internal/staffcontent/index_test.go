package staffcontent

import (
	"bytes"
	"image/png"
	"strings"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func loadReal(t *testing.T) (*Index, *Zones) {
	t.Helper()
	root := "../../data/"
	catalog, err := data.LoadCatalog(root+"catalog/itemlist.csv", root+"catalog/Itemname.csv", root+"catalog/ItemEffect.h", root+"catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := data.LoadNPCs(root + "npcs")
	if err != nil {
		t.Fatal(err)
	}
	geners, err := data.LoadChannelNPCGener(root + "spawns/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	drops, err := data.LoadDropRates(root + "content/droprate.json")
	if err != nil {
		t.Fatal(err)
	}
	quests, err := data.LoadQuests(root + "content/quests.json")
	if err != nil {
		t.Fatal(err)
	}
	zones, err := data.LoadQuestZones(root + "content/quest_zones.json")
	if err != nil {
		t.Fatal(err)
	}
	terrain, err := data.LoadTerrain(root+"world/maps/HeightMap.dat", root+"world/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	teleports, err := data.LoadTeleports(root + "world/teleports.ini")
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := data.LoadVolatilesWithInstances(root+"content/volatiles.json", root+"content/instances.json", root+"spawns/instances", catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	return Build(npcs, geners, catalog, quests, zones, drops, nil, time.Now()), NewZones(terrain, teleports, zones, volatiles)
}

func TestIndexOverRealData(t *testing.T) {
	ix, _ := loadReal(t)
	dropped, sold, spawned := 0, 0, 0
	for _, refs := range ix.items {
		if len(refs.DroppedBy) > 0 {
			dropped++
		}
		if len(refs.SoldBy) > 0 {
			sold++
		}
	}
	for _, mob := range ix.mobs {
		if len(mob.Spawns) > 0 {
			spawned++
		}
	}
	if dropped == 0 || sold == 0 || spawned == 0 {
		t.Fatal("cross references missing", dropped, sold, spawned)
	}
	for _, issue := range ix.Issues {
		if issue.Severity == "error" {
			t.Fatal("an enabled generator would fail the boot:", issue)
		}
	}
	hits := ix.Search("potion", 5)
	if len(hits) == 0 || len(hits) > 5 {
		t.Fatal(hits)
	}
	t.Logf("%d items dropped, %d sold, %d spawning templates, %d warnings", dropped, sold, spawned, len(ix.Issues))
}

func TestIndexReportsUnresolvedReferences(t *testing.T) {
	catalog := model.Catalog{Items: map[uint16]model.ItemDef{400: {Index: 400, Name: "Small_Potion"}, 401: {Index: 401, Name: "Gem"}}}
	carry := make([]model.Item, 12)
	carry[0], carry[1], carry[11] = model.Item{Index: 400}, model.Item{Index: 9999}, model.Item{Index: 401}
	npcs := []model.NPCDef{{Name: "Wolf", Tipo: model.TipoMonstro, Score: &model.Score{Level: 50}, Carry: carry},
		{Name: "Pup", Tipo: model.TipoMonstro, Carry: []model.Item{{Index: 400}}},
		{Name: "Seller", Vende: []model.Item{{Index: 400}}}}
	geners := []model.NPCGener{{Index: 3, Enabled: true, Leader: "Wolf", Follower: "Ghost"}, {Index: 4, Enabled: false, Leader: "Nobody", Follower: "Wolf"},
		{Index: 5, Enabled: true, Leader: "Ghost", Follower: "Wolf"}, {Index: 6, Enabled: true, Leader: "Wolf", Follower: "Wolf"}}
	var drops [model.MaxCarry]int
	drops[0] = 1000
	ix := Build(npcs, geners, catalog, model.QuestFile{}, model.QuestZoneFile{}, drops, map[int]string{5: "item_instance"}, time.Now())
	item, ok := ix.Item(400)
	if !ok || item.Name != "Small Potion" || len(item.DroppedBy) != 2 || len(item.SoldBy) != 1 {
		t.Fatal(item)
	}
	// Level 50 uses the 8% band (1 in 80); level 0 rounds 4% of 1000 to 40.
	for _, drop := range item.DroppedBy {
		want := map[string]int{"Wolf": 80, "Pup": 40}[drop.Mob]
		if drop.OneIn != want || drop.Always || drop.BaseOneIn != 1000 {
			t.Fatal(drop)
		}
	}
	if gem, _ := ix.Item(401); len(gem.DroppedBy) != 1 || !gem.DroppedBy[0].Always || gem.DroppedBy[0].OneIn != 0 {
		t.Fatal("slot 11 must always drop", gem)
	}
	issues := map[string]Issue{}
	for _, issue := range ix.Issues {
		issues[issue.Kind+":"+issue.Subject] = issue
	}
	if issues["generator_missing_template:NPCGener[3]"].Severity != "error" || issues["generator_missing_template:NPCGener[4]"].Severity != "warning" ||
		issues["drop_unknown_item:Wolf"].Severity != "warning" || ix.Issues[0].Severity != "error" {
		t.Fatal(issues)
	}
	if reserved := issues["generator_missing_template:NPCGener[5]"]; reserved.Severity != "warning" || !strings.Contains(reserved.Detail, "reserved for item instance") {
		t.Fatal("a reserved generator is skipped before the template check", reserved)
	}
	wolf, _ := ix.Mob("Wolf")
	roles := map[int]SpawnRef{}
	for _, spawn := range wolf.Spawns {
		roles[spawn.Generator] = spawn
	}
	if wolf.Kind != "monster" || len(wolf.Spawns) != 4 || roles[3].Role != "leader" || roles[4].Role != "follower" ||
		roles[5].Reserved != "item_instance" || roles[6].Role != "both" || roles[6].Reserved != "" {
		t.Fatal(wolf.Spawns)
	}
}

func TestZonesAndTerrainImage(t *testing.T) {
	_, zones := loadReal(t)
	// The Big Cube return portal carries the native teleport attribute.
	if info := zones.At(1342, 1466); !info.TerrainLoaded || !info.TeleportActivation {
		t.Fatal(info)
	}
	image, err := zones.TerrainPNG()
	if err != nil {
		t.Fatal(err)
	}
	decoded, err := png.Decode(bytes.NewReader(image))
	if err != nil || decoded.Bounds().Dx() != model.AttributeWidth {
		t.Fatal(err)
	}
	if again, _ := zones.TerrainPNG(); &again[0] != &image[0] {
		t.Fatal("terrain image must be rendered once")
	}
	t.Logf("terrain PNG %d KiB", len(image)/1024)
}
