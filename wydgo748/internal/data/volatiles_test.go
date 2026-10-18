package data

import (
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestSplitVolatileFilesEnforceInstanceBoundary(t *testing.T) {
	dir := t.TempDir()
	volatilesPath := filepath.Join(dir, "volatiles.json")
	instancesPath := filepath.Join(dir, "instances.json")
	write := func(path, contents string) {
		t.Helper()
		if err := os.WriteFile(path, []byte(contents), 0o600); err != nil {
			t.Fatal(err)
		}
	}

	write(volatilesPath, `{
		"default":{"action":"generic"},
		"rules":{"51":{"action":"instance_ticket","instanceRef":"cube"}}
	}`)
	write(instancesPath, `{"instances":{},"rules":{},"items":{}}`)
	if _, err := LoadVolatilesWithInstances(volatilesPath, instancesPath, nil, nil); err == nil {
		t.Fatal("volatiles.json accepted an instance rule")
	}

	write(volatilesPath, `{"default":{"action":"generic"},"rules":{},"items":{}}`)
	write(instancesPath, `{
		"instances":{},
		"rules":{"1":{"action":"restore","consume":true}},
		"items":{}
	}`)
	if _, err := LoadVolatilesWithInstances(volatilesPath, instancesPath, nil, nil); err == nil {
		t.Fatal("instances.json accepted an ordinary volatile rule")
	}
}

func TestLoadVolatilesDiscoversCatalogItems(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "volatiles.json")
	if err := os.WriteFile(path, []byte(`{
		"default":{"action":"generic"},
		"rules":{"1":{"action":"restore","consume":true,"valueSource":"item_effects"}},
		"items":{"401":{"action":"restore","consume":true,"hp":999}}
	}`), 0600); err != nil {
		t.Fatal(err)
	}
	items := map[uint16]model.ItemDef{
		400: {Index: 400, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 1}}},
		401: {Index: 401, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 1}}},
		500: {Index: 500},
	}
	catalog, err := LoadVolatiles(path, items, nil)
	if err != nil {
		t.Fatal(err)
	}
	if len(catalog.ItemCodes) != 2 || catalog.Codes[1] != 2 {
		t.Fatalf("incorrect discovery: items=%v codes=%v", catalog.ItemCodes, catalog.Codes)
	}
	rule, code, ok := catalog.Rule(401)
	if !ok || code != 1 || rule.HP != 999 {
		t.Fatalf("override not applied: rule=%+v code=%d ok=%v", rule, code, ok)
	}
}

func TestLoadVolatilesRejectsInvalidEquipmentTransforms(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "volatiles.json")
	items := map[uint16]model.ItemDef{
		578:  {Index: 578, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 9}}},
		3386: {Index: 3386, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 180}}},
	}
	for _, body := range []string{
		`{"default":{"action":"generic"},"rules":{"180":{"action":"equipment_gem","consume":true,"variant":4}}}`,
		`{"default":{"action":"generic"},"rules":{"180":{"action":"equipment_gem","variant":0}}}`,
		`{"default":{"action":"generic"},"rules":{"9":{"action":"ore_upgrade","consume":true,"variant":3,"successPercent":0}}}`,
		`{"default":{"action":"generic"},"rules":{"9":{"action":"ore_upgrade","consume":true,"variant":3,"successPercent":101}}}`,
	} {
		if err := os.WriteFile(path, []byte(body), 0600); err != nil {
			t.Fatal(err)
		}
		if _, err := LoadVolatiles(path, items, nil); err == nil {
			t.Fatalf("invalid equipment transform was accepted: %s", body)
		}
	}
}

func TestLoadVolatilesValidatesQuestReward(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "volatiles.json")
	items := map[uint16]model.ItemDef{
		4117: {Index: 4117, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 191}}},
	}
	for _, body := range []string{
		`{"default":{"action":"generic"},"items":{"4117":{"action":"quest_reward","consume":true,"minLevel":39,"maxLevelExclusive":115}}}`,
		`{"default":{"action":"generic"},"items":{"4117":{"action":"quest_reward","consume":true,"exp":5000,"minLevel":39,"maxLevelExclusive":39}}}`,
	} {
		if err := os.WriteFile(path, []byte(body), 0600); err != nil {
			t.Fatal(err)
		}
		if _, err := LoadVolatiles(path, items, nil); err == nil {
			t.Fatalf("invalid quest_reward was accepted: %s", body)
		}
	}
}

func TestLoadVolatilesRejectsIncompleteRefineSet(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "volatiles.json")
	items := map[uint16]model.ItemDef{
		4122: {Index: 4122, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 194}}},
	}
	for _, body := range []string{
		`{"default":{"action":"generic"},"items":{"4122":{"action":"refine_set","consume":true,"refineMax":6}}}`,
		`{"default":{"action":"generic"},"items":{"4122":{"action":"refine_set","consume":true,"mortalOnly":true,"refineMax":6,"onceQuestId":-194,"minLevel":256,"maxLevelExclusive":200}}}`,
		`{"default":{"action":"generic"},"items":{"4122":{"action":"refine","consume":true,"refineMax":6,"onceQuestId":-194}}}`,
	} {
		if err := os.WriteFile(path, []byte(body), 0600); err != nil {
			t.Fatal(err)
		}
		if _, err := LoadVolatiles(path, items, nil); err == nil {
			t.Fatalf("invalid refine_set was accepted: %s", body)
		}
	}
}

func TestLoadVolatilesRejectsAmbiguousOrInvalidBuffs(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "volatiles.json")
	items := map[uint16]model.ItemDef{
		100: {Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 1}}},
	}
	skills := map[int]model.SkillDef{
		43: {Index: 43, AffectType: 11, AffectValue: 15},
	}
	bodies := []string{
		`{"default":{"action":"generic"},"rules":{"1":{"action":"buff","affectType":2}}}`,
		`{"default":{"action":"generic"},"rules":{"1":{"action":"buff","affectType":256,"durationUnits":10}}}`,
		`{"default":{"action":"generic"},"rules":{"1":{"action":"buff","affectType":2,"durationUnits":10,"affects":[{"skillId":43,"durationUnits":10}]}}}`,
		`{"default":{"action":"generic"},"rules":{"1":{"action":"buff","affects":[{"skillId":999,"durationUnits":10}]}}}`,
		`{"default":{"action":"generic"},"rules":{"1":{"action":"buff","affects":[{"type":2,"skillId":43,"durationUnits":10}]}}}`,
	}
	for _, body := range bodies {
		if err := os.WriteFile(path, []byte(body), 0600); err != nil {
			t.Fatal(err)
		}
		if _, err := LoadVolatiles(path, items, skills); err == nil {
			t.Fatalf("invalid buff was accepted: %s", body)
		}
	}
}

func TestLoadVolatilesAllowsSharedInstanceIDOnlyForSameLayout(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "volatiles.json")
	items := map[uint16]model.ItemDef{
		100: {Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 1}}},
		101: {Index: 101, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 2}}},
	}
	const base = `"action":"instance_ticket","consume":true,"instance":{` +
		`"id":"shared","name":"Room","x":100,"y":100,"spawnX":101,"spawnY":101,` +
		`"areaRadius":5,"spawns":[{"npc":"Mob","count":1}],` +
		`"durationSeconds":60,"exitX":200,"exitY":200`

	same := `{"default":{"action":"generic"},"rules":{` +
		`"1":{` + base + `}},` +
		`"2":{` + base + `}}}}`
	if err := os.WriteFile(path, []byte(same), 0600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadVolatiles(path, items, nil); err != nil {
		t.Fatalf("identical shared room was rejected: %v", err)
	}

	divergent := `{"default":{"action":"generic"},"rules":{` +
		`"1":{` + base + `}},` +
		`"2":{` + base + `,"durationSeconds":90}}}}`
	if err := os.WriteFile(path, []byte(divergent), 0600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadVolatiles(path, items, nil); err == nil {
		t.Fatal("same ID with conflicting layouts was accepted")
	}
}

func TestLoadVolatilesRejectsWaterChainCrossTier(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "volatiles.json")
	items := map[uint16]model.ItemDef{
		100: {Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 1}}},
		101: {Index: 101, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 2}}},
	}
	body := `{"default":{"action":"generic"},"items":{` +
		`"100":{"action":"instance_ticket","consume":true,"instance":{"id":"water-normal-boss","name":"boss","x":100,"y":100,"spawnX":101,"spawnY":101,"areaRadius":5,"spawns":[{"npc":"Mob","count":1}],"durationSeconds":60,"chainNextItem":101}},` +
		`"101":{"action":"instance_ticket","consume":true,"instance":{"id":"water-mystic-1","name":"room","x":200,"y":200,"spawnX":201,"spawnY":201,"areaRadius":5,"spawns":[{"npc":"Mob","count":1}],"durationSeconds":60}}}}`
	if err := os.WriteFile(path, []byte(body), 0600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadVolatiles(path, items, nil); err == nil {
		t.Fatal("cross-tier Water chain normal->mystic was accepted")
	}
}

// TestVolatilesRealConfigDifferentiatesByItemIndex loads production data and
// verifies distinct Index overrides for items sharing EF_VOLATILE. Production
// configuration must load against the real catalog.
func TestVolatilesRealConfigDifferentiatesByItemIndex(t *testing.T) {
	catalog, err := LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	vc, err := LoadVolatilesWithInstances("../../data/volatiles.json",
		"../../data/instances.json", catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatalf("production volatiles.json does not load against the catalog: %v", err)
	}
	for _, itemID := range []uint16{3173, 777, 3182} {
		rule, _, ok := vc.Rule(itemID)
		if !ok || rule.Instance == nil || !rule.Instance.ChainStart {
			t.Fatalf("Water root %d is not marked ChainStart: %+v", itemID, rule)
		}
	}
	for _, itemID := range []uint16{3174, 3181, 778, 785, 3183, 3190} {
		rule, _, ok := vc.Rule(itemID)
		if !ok || rule.Instance == nil || rule.Instance.ChainStart {
			t.Fatalf("Water non-root %d incorrectly received ChainStart: %+v", itemID, rule)
		}
	}

	// Sefirah 7-day and 30-day items share client-supported code 67, but resolve
	// distinct durations through their Index overrides.
	r7, c7, ok7 := vc.Rule(3361)
	r30, c30, ok30 := vc.Rule(3363)
	if !ok7 || !ok30 || c7 != 67 || c30 != 67 {
		t.Fatalf("unexpected Sefirah codes: c7=%d c30=%d ok=%v/%v", c7, c30, ok7, ok30)
	}
	if r7.DurationUnits == r30.DurationUnits {
		t.Fatalf("Sefirah 7d and 30d resolved the same duration (%d): Index overrides failed",
			r7.DurationUnits)
	}
	if r30.DurationUnits <= r7.DurationUnits {
		t.Fatalf("Sefirah 30d (%d) must last longer than 7d (%d)", r30.DurationUnits, r7.DurationUnits)
	}

	// Code-keyed damage buff (Kappa 787 -> Type 4 scaling by percentage).
	if r, _, _ := vc.Rule(787); r.Action != "buff" || r.AffectType != 4 || r.AffectValue == 0 {
		t.Fatalf("Kappa (787) did not resolve a scaled Type 4 buff: %+v", r)
	}
	// Fairy Dust uses the next native level threshold, never fixed EXP.
	if r, _, _ := vc.Rule(414); r.Action != "grant_next_level" {
		t.Fatalf("Fairy_Dust (414) did not resolve grant_next_level: %+v", r)
	}
	// Star Gem / Warp (12/13) and EXP chest (198) resolve their intended actions.
	if vc.Rules[12].Action != "save_position" {
		t.Fatalf("code 12 must resolve save_position: %q", vc.Rules[12].Action)
	}
	if vc.Rules[13].Action != "warp_saved" {
		t.Fatalf("code 13 must resolve warp_saved: %q", vc.Rules[13].Action)
	}
	if r := vc.Rules[198]; r.Action != "buff" || r.AffectType != 39 || !r.Accumulate {
		t.Fatalf("code 198 must resolve an accumulating affect 39 buff: %+v", r)
	}
	uxmal, ok := vc.Instances["uxmal"]
	if !ok || uxmal.StateMachine != "uxmal" || uxmal.Uxmal == nil ||
		len(uxmal.Stages) != 7 || len(uxmal.Uxmal.RoomPositions) != 7 ||
		len(uxmal.Uxmal.Runes) != 7 || uxmal.Uxmal.TicketItem != 5134 {
		t.Fatalf("Uxmal did not resolve its named configuration: ok=%v cfg=%+v", ok, uxmal)
	}
	if uxmal.Uxmal.MaxParties[0] != 2 || uxmal.Uxmal.MaxParties[1] != 3 {
		t.Fatalf("native Uxmal room capacity was lost: %v", uxmal.Uxmal.MaxParties)
	}
	for itemID, expected := range map[uint16]struct {
		code, variant int
	}{
		3386: {180, 0}, 3387: {181, 1}, 3388: {182, 2}, 3389: {183, 3},
	} {
		r, code, ok := vc.Rule(itemID)
		if !ok || code != expected.code || r.Action != "equipment_gem" ||
			r.Variant != expected.variant || !r.Consume {
			t.Fatalf("Gem %d: code=%d rule=%+v ok=%v", itemID, code, r, ok)
		}
	}
	if r, code, ok := vc.Rule(578); !ok || code != 9 || r.Action != "ore_upgrade" ||
		r.Variant != 3 || r.SuccessPercent != 50 || !r.Consume {
		t.Fatalf("Adamantite: code=%d rule=%+v ok=%v", code, r, ok)
	}
	// Index-multiplexed _MSG_UseItem rules must not silently regress to the
	// code-level no_direct_use fallback.
	for itemID, action := range map[uint16]string{
		3453: "mount_revive", 4003: "loot_box",
		1731: "instance_ticket", 3171: "instance_ticket", 3172: "instance_ticket",
		3324: "instance_ticket", 3390: "instance_ticket",
		3328: "instance_ticket", 3329: "instance_ticket",
		3909: "timed_access", 3439: "timed_access",
		3393: "grant_counter", 4114: "grant_counter",
	} {
		r, _, ok := vc.Rule(itemID)
		if !ok || r.Action != action {
			t.Fatalf("item %d must resolve %s: %+v ok=%v", itemID, action, r, ok)
		}
	}
	for _, itemID := range []uint16{1772, 4000, 4001, 3328, 3329} {
		r, _, ok := vc.Rule(itemID)
		if !ok || r.Action != "instance_ticket" || r.Instance == nil ||
			len(r.Instance.Stages) == 0 {
			t.Fatalf("dungeon %d did not resolve a complete instance: %+v", itemID, r)
		}
	}
	if r, _, ok := vc.Rule(1737); !ok || r.Action != "instance_ticket" || !r.Consume ||
		r.Instance == nil || r.Instance.StateMachine != "big_cube" ||
		len(r.Instance.Stages) != 1 || r.Instance.Stages[0].Quiz == nil {
		t.Fatalf("Big Cube must resolve the O/X state machine: %+v ok=%v", r, ok)
	}
	if cube, _, _ := vc.Rule(1772); len(cube.Instance.Stages) != 25 {
		t.Fatalf("Cube must have 25 rooms, got %d", len(cube.Instance.Stages))
	}
	if hellSolo, _, _ := vc.Rule(3328); hellSolo.Instance.ActiveDurationSeconds != 1080 ||
		hellSolo.Instance.DurationSeconds != 1200 || hellSolo.Instance.Schedule[0].DurationSeconds != 120 {
		t.Fatalf("Hell Gate must retain the approved 2-minute admission and 18-minute encounter: %+v", hellSolo.Instance)
	}
	if hellSolo, _, _ := vc.Rule(3328); hellSolo.Instance.ID != "hell-gate" {
		t.Fatalf("Hell Gate solo resolved an unexpected ID: %q", hellSolo.Instance.ID)
	}
	if hellParty, _, _ := vc.Rule(3329); hellParty.Instance.ID != "hell-gate" {
		t.Fatalf("Hell Gate party must share occupancy: %q", hellParty.Instance.ID)
	}
	if nightmare, _, _ := vc.Rule(3324); len(nightmare.Instance.Schedule) != 3 ||
		len(nightmare.Instance.EntryAreas) != 1 || nightmare.Instance.NightmareTier != "normal" ||
		nightmare.Instance.PartyRunLimit != 3 {
		t.Fatalf("Nightmare Normal lost its entry window/area: %+v", nightmare.Instance)
	}
	if nightmare, _, _ := vc.Rule(3325); nightmare.Instance.NightmareTier != "mystic" {
		t.Fatalf("Nightmare Mystic lost its tier: %+v", nightmare.Instance)
	}
	if nightmare, _, _ := vc.Rule(3326); nightmare.Instance.NightmareTier != "arcane" {
		t.Fatalf("Nightmare Arcane lost its tier: %+v", nightmare.Instance)
	}
	if hell, _, _ := vc.Rule(3328); len(hell.Instance.Schedule) != 2 {
		t.Fatalf("Hell Gate must have two scheduled windows: %+v", hell.Instance)
	}
	if magic, _, _ := vc.Rule(3172); magic.PartyMode != "" {
		t.Fatalf("Magic Chamber must accept solo and leader-led parties: partyMode=%q", magic.PartyMode)
	}
	for _, itemID := range []uint16{1731, 3171, 3172} {
		magic, _, ok := vc.Rule(itemID)
		if !ok || magic.Instance == nil || len(magic.Instance.Stages) != 4 ||
			magic.Instance.ExclusiveGroup != "" ||
			len(magic.Instance.Stages[3].CompletionSpawns) == 0 {
			t.Fatalf("Magic Chamber %d must allow independent runs with four rooms and a final boss: %+v", itemID, magic.Instance)
		}
	}
	for itemID, wantID := range map[uint16]string{1772: "cube", 4000: "cube-mystic", 4001: "cube-arcane"} {
		cube, _, ok := vc.Rule(itemID)
		if !ok || cube.Instance == nil || cube.Instance.ID != wantID ||
			!cube.Instance.SharedEntry || len(cube.Instance.Stages) != 25 {
			t.Fatalf("Cube %d did not resolve its independent variant: id=%q shared=%v stages=%d",
				itemID, cube.Instance.ID, cube.Instance.SharedEntry, len(cube.Instance.Stages))
		}
	}
	for _, itemID := range []uint16{3324, 3325, 3326} {
		nightmare, _, ok := vc.Rule(itemID)
		if !ok || nightmare.Instance == nil || nightmare.Instance.Mode != "shared_timed_zone" ||
			nightmare.Instance.FinishPolicy != "respawn_until_timeout" ||
			nightmare.Instance.SharedGroup == "" {
			t.Fatalf("Nightmare %d is not a shared zone with respawn: %+v", itemID, nightmare.Instance)
		}
	}
	if hell, _, ok := vc.Rule(3328); !ok || hell.Instance == nil ||
		hell.Instance.StateMachine != "hell_gate" || hell.Instance.HellGate == nil ||
		len(hell.Instance.HellGate.Quadrants) != 4 || hell.Instance.FinishPolicy != "state_machine" {
		t.Fatalf("Hell Gate did not resolve its state machine: %+v ok=%v", hell.Instance, ok)
	}
	for _, itemID := range []uint16{646, 647, 3378} {
		r, _, _ := vc.Rule(itemID)
		if r.Action != "buff" || r.AffectType != 30 {
			t.Fatalf("Courage %d does not use affect 30: %+v", itemID, r)
		}
	}
	for _, itemID := range []uint16{1739, 4145} {
		r, _, _ := vc.Rule(itemID)
		if len(r.Affects) != 4 {
			t.Fatalf("Love item %d does not have all four affects: %+v", itemID, r)
		}
	}
	if expBox := vc.Rules[198]; expBox.MaxDurationUnits != 10800 {
		t.Fatalf("EXP chest lost its 24-hour cap: %+v", expBox)
	}
	if r, _, ok := vc.Rule(3443); !ok || r.Action != "celestial_capsule" || !r.Consume {
		t.Fatalf("Spirit's Seal must perform encapsulation: %+v ok=%v", r, ok)
	}
	if r, _, ok := vc.Rule(3455); !ok || r.Action != "no_direct_use" || r.Consume {
		t.Fatalf("Extraction of Magical Power must not allow direct use: %+v ok=%v", r, ok)
	}
	if r, _, ok := vc.Rule(5338); !ok || r.Action != "celestial_ideal" || !r.Consume {
		t.Fatalf("Ideal Stone must create Celestial/SubCelestial: %+v ok=%v", r, ok)
	}
	if r, _, ok := vc.Rule(3020); !ok || r.Action != "celestial_fury" || !r.Consume {
		t.Fatalf("Fury Stone must perform Celestial unlocks: %+v ok=%v", r, ok)
	}
	if r, _, ok := vc.Rule(4148); !ok || r.Action != "celestial_switch" || !r.Consume {
		t.Fatalf("Mysterious Stone must switch forms: %+v ok=%v", r, ok)
	}
	// Refinement: Ori(4)=cap +6, Lac(5)=cap +9, Molar(194)=set +6.
	if r := vc.Rules[4]; r.Action != "refine" || r.RefineMax != 6 {
		t.Fatalf("code 4 must resolve refine with cap 6: %+v", r)
	}
	if r := vc.Rules[5]; r.Action != "refine" || r.RefineMax != 9 {
		t.Fatalf("code 5 must resolve refine with cap 9: %+v", r)
	}
	if r := vc.Rules[194]; r.Action != "refine_set" || r.RefineMax != 6 ||
		!r.MortalOnly || r.MinLevel != 200 || r.MaxLevelExclusive != 256 || r.OnceQuestID != -194 {
		t.Fatalf("code 194 must resolve refine_set with cap 6: %+v", r)
	}
	// The 24 regular rooms and three Nessus rooms are distinct instances.
	// Each regular room awards the next scroll; the boss ends the chain.
	waterItems := []uint16{
		777, 778, 779, 780, 781, 782, 783, 784, 785,
		3173, 3174, 3175, 3176, 3177, 3178, 3179, 3180, 3181,
		3182, 3183, 3184, 3185, 3186, 3187, 3188, 3189, 3190,
	}
	instanceIDs := make(map[string]struct{}, len(waterItems))
	for _, itemID := range waterItems {
		r, _, ok := vc.Rule(itemID)
		if !ok || r.Action != "instance_ticket" || r.Instance == nil {
			t.Fatalf("Water scroll %d has no instance: %+v ok=%v", itemID, r, ok)
		}
		instanceIDs[r.Instance.ID] = struct{}{}
	}
	if len(instanceIDs) != len(waterItems) {
		t.Fatalf("duplicate Water rooms: ids=%d items=%d", len(instanceIDs), len(waterItems))
	}
	chains := [][]uint16{
		{777, 778, 779, 780, 781, 782, 783, 784, 785},
		{3173, 3174, 3175, 3176, 3177, 3178, 3179, 3180, 3181},
		{3182, 3183, 3184, 3185, 3186, 3187, 3188, 3189, 3190},
	}
	for _, chain := range chains {
		for index, itemID := range chain {
			r, _, _ := vc.Rule(itemID)
			want := uint16(0)
			if index+1 < len(chain) {
				want = chain[index+1]
			}
			if r.Instance.RewardItem != want {
				t.Fatalf("Water %d reward %d, want %d",
					itemID, r.Instance.RewardItem, want)
			}
		}
	}
	// Each ticket has an exact origin. The first accepts external entry and the
	// boss room (closing the cycle); Rooms 2..8 accept only the previous platform;
	// the boss accepts only Room 8. The loader expands the data-owned table.
	waterSets := []struct {
		items  []uint16
		prefix string
	}{
		{items: chains[0], prefix: "water-mystic"},
		{items: chains[1], prefix: "water-normal"},
		{items: chains[2], prefix: "water-arcane"},
	}
	for _, family := range waterSets {
		for index, itemID := range family.items {
			r, _, ok := vc.Rule(itemID)
			if !ok || r.Instance == nil {
				t.Fatalf("Water %d has no instance after entryAreaSet expansion", itemID)
			}
			wantSet, wantAreas := family.prefix+"-room1-entry", 2
			if index > 0 {
				wantSet, wantAreas = fmt.Sprintf("%s-room%d", family.prefix, index), 1
			}
			if index == len(family.items)-1 {
				wantSet = family.prefix + "-room8"
			}
			if r.Instance.EntryAreaSet != wantSet || len(r.Instance.EntryAreas) != wantAreas {
				t.Fatalf("Water %d incorrect entry areas: set=%q areas=%d want=%q/%d",
					itemID, r.Instance.EntryAreaSet, len(r.Instance.EntryAreas), wantSet, wantAreas)
			}
			if !r.Instance.AllowChainDuringExitGrace {
				t.Fatalf("Water %d lost AllowChainDuringExitGrace", itemID)
			}
			if index == len(family.items)-1 {
				if r.Instance.RewardItem != 0 || r.Instance.ChainNextItem != family.items[0] {
					t.Fatalf("Water boss %d must accept Room1 without a reward: reward=%d next=%d",
						itemID, r.Instance.RewardItem, r.Instance.ChainNextItem)
				}
			} else if r.Instance.ChainNextItem != 0 {
				t.Fatalf("regular Water %d must not have ChainNextItem=%d", itemID, r.Instance.ChainNextItem)
			}
		}
	}
}

func TestRealCatalogPreservesAncientGemFamilies(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	families := 0
	for itemID, def := range catalog.Items {
		if def.Grade != 5 || (def.Pos != 64 && def.Pos != 192) {
			continue
		}
		complete := true
		for variant := 0; variant < 4; variant++ {
			candidate, ok := catalog.Items[itemID+uint16(variant)]
			if !ok || candidate.Pos != def.Pos || candidate.Grade != 5+variant {
				complete = false
				break
			}
		}
		if complete {
			families++
		}
	}
	if families == 0 {
		t.Fatal("catalog has no complete Ancient family (contiguous grades 5..8)")
	}
}

func TestRealCatalogMapsAdamantiteToMatchingLegendVariant(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	adamantiteUnique := map[int]bool{8: true, 10: true, 17: true, 20: true,
		27: true, 30: true, 37: true, 40: true}
	checked := 0
	for itemID, def := range catalog.Items {
		if def.Pos == 0 || def.Grade < 1 || def.Grade > 3 || !adamantiteUnique[def.Unique] {
			continue
		}
		if def.Extra <= 0 || def.Extra > int(^uint16(0)) {
			t.Errorf("Adamantite-eligible equipment %d has no Le variant target", itemID)
			continue
		}
		legend, ok := catalog.Items[uint16(def.Extra)]
		if !ok || legend.Pos != def.Pos || legend.Grade != 4 || legend.Unique != def.Unique {
			t.Errorf("equipment %d points Extra=%d to an invalid Le variant: %+v",
				itemID, def.Extra, legend)
		}
		checked++
	}
	if checked == 0 {
		t.Fatal("catalog has no Adamantite-eligible equipment")
	}

	// Real example: Golden Embersed Helmet (Mystic) -> the same helmet (Legend).
	input, output := catalog.Items[1207], catalog.Items[2186]
	if input.Extra != 2186 || input.Pos != output.Pos || input.Unique != output.Unique ||
		input.Grade != 2 || output.Grade != 4 {
		t.Fatalf("Golden Embersed (M)->(Le) lost its link: input=%+v output=%+v",
			input, output)
	}
}

func TestLoadVolatilesAppliesGenericToEveryUnconfiguredCode(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "volatiles.json")
	if err := os.WriteFile(path, []byte(`{"default":{"action":"generic","consume":false}}`), 0600); err != nil {
		t.Fatal(err)
	}
	items := map[uint16]model.ItemDef{
		700: {Index: 700, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 77}}},
	}
	catalog, err := LoadVolatiles(path, items, nil)
	if err != nil {
		t.Fatal(err)
	}
	rule, code, ok := catalog.Rule(700)
	if !ok || code != 77 || rule.Action != "generic" || rule.Consume {
		t.Fatalf("incorrect generic fallback: rule=%+v code=%d ok=%v", rule, code, ok)
	}
}

func TestVolatileInstancesReferenceExistingHostileNPCs(t *testing.T) {
	catalog, err := LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := LoadVolatilesWithInstances("../../data/volatiles.json",
		"../../data/instances.json", catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	byName := make(map[string]model.NPCDef, len(npcs))
	for _, npc := range npcs {
		byName[npc.Name] = npc
	}
	check := func(label string, rule model.VolatileRule) {
		t.Helper()
		if rule.Action != "instance_ticket" || rule.Instance == nil {
			return
		}
		stages := rule.Instance.Stages
		if len(stages) == 0 {
			stages = []model.VolatileInstanceStage{{Spawns: rule.Instance.Spawns}}
		}
		for stageIndex, stage := range stages {
			for _, spawn := range stage.Spawns {
				npc, exists := byName[spawn.NPC]
				if !exists {
					t.Errorf("%s room %d references missing NPC %q",
						label, stageIndex+1, spawn.NPC)
				} else if npc.Tipo != model.TipoMonstro {
					t.Errorf("%s room %d uses non-hostile NPC %q",
						label, stageIndex+1, spawn.NPC)
				}
			}
		}
	}
	for code, rule := range volatiles.Rules {
		check(fmt.Sprintf("volatile %d", code), rule)
	}
	for itemID, rule := range volatiles.Items {
		check(fmt.Sprintf("item %d", itemID), rule)
	}
}
