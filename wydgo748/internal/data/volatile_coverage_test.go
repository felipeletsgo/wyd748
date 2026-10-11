package data

import (
	"fmt"
	"path/filepath"
	"sort"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func loadRealVolatiles(t *testing.T, root string, catalog model.Catalog) model.VolatileCatalog {
	t.Helper()
	volatiles, err := LoadVolatilesWithInstances(filepath.Join(root, "volatiles.json"),
		filepath.Join(root, "instances.json"), catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	return volatiles
}

// TestEveryRealVolatileCodeHasExplicitContract creates an independent subtest
// for EVERY EF_VOLATILE found in the authoritative item list. This makes the
// coverage visible per code in the report and prevents three silent
// regressions: an item without a rule, a diverging count and an implemented
// family falling back to generic.
func TestEveryRealVolatileCodeHasExplicitContract(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, root, catalog)
	itemsByCode := make(map[int][]uint16, len(volatiles.Codes))
	for itemID, code := range volatiles.ItemCodes {
		itemsByCode[code] = append(itemsByCode[code], itemID)
	}
	codes := make([]int, 0, len(volatiles.Codes))
	for code := range volatiles.Codes {
		codes = append(codes, code)
	}
	sort.Ints(codes)
	for _, code := range codes {
		code := code
		t.Run(fmt.Sprintf("volatile_%d", code), func(t *testing.T) {
			items := itemsByCode[code]
			sort.Slice(items, func(i, j int) bool { return items[i] < items[j] })
			if len(items) == 0 || len(items) != volatiles.Codes[code] {
				t.Fatalf("catalog reports %d item(s), resolved=%v",
					volatiles.Codes[code], items)
			}
			for _, itemID := range items {
				rule, resolvedCode, ok := volatiles.Rule(itemID)
				if !ok || resolvedCode != code || strings.TrimSpace(rule.Action) == "" {
					t.Fatalf("item %d did not resolve a contract: code=%d ok=%v action=%q",
						itemID, resolvedCode, ok, rule.Action)
				}
				if rule.Action == "generic" {
					t.Fatalf("item %d still uses generic", itemID)
				}
			}
		})
	}
}

// itemlist.csv is the source of truth for the EF_VOLATILE universe. These
// numbers are an audit sentinel: if a conversion deletes a line, changes the
// delimiter or adds an item without updating the contract, the tests fail
// instead of hiding the regression in the generic handler.
func TestVolatileCatalogKeepsAuthoritativeCount(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, root, catalog)
	if got, want := len(volatiles.ItemCodes), 355; got != want {
		t.Fatalf("items with EF_VOLATILE=%d, want %d", got, want)
	}
	if got, want := len(volatiles.Codes), 124; got != want {
		t.Fatalf("EF_VOLATILE codes=%d, want %d", got, want)
	}
}

func TestMountRecoveryAndCatalystsResolveNativeContracts(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, root, catalog)
	tests := []struct {
		item             uint16
		code             int
		action           string
		minLevel, amount int
	}{
		{item: 3315, code: 90, action: "longevity_restore", amount: 1},
		{item: 3316, code: 91, action: "level_set", amount: 100},
		{item: 3317, code: 92, action: "level_set", minLevel: 100, amount: 120},
	}
	for _, want := range tests {
		rule, code, ok := volatiles.Rule(want.item)
		if !ok || code != want.code || rule.Action != "mount" ||
			rule.MountAction != want.action || rule.MountMinLevel != want.minLevel ||
			rule.Amount != want.amount || !rule.Consume {
			t.Errorf("item %d resolved code=%d ok=%v rule=%+v", want.item, code, ok, rule)
		}
	}
}

func TestPremiumFirecrackerHasCustomContractWithoutChangingCommonFireworks(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, root, catalog)
	premium, code, ok := volatiles.Rule(3442)
	if !ok || code != 19 || premium.Action != "firework" ||
		!premium.Consume || !premium.CustomPattern {
		t.Fatalf("Premium Firecracker resolved a wrong contract: code=%d ok=%v rule=%+v",
			code, ok, premium)
	}
	ordinary, code, ok := volatiles.Rule(1728)
	if !ok || code != 19 || ordinary.Action != "firework" ||
		!ordinary.Consume || ordinary.CustomPattern {
		t.Fatalf("the premium changed the common firework: code=%d ok=%v rule=%+v",
			code, ok, ordinary)
	}
}

func TestLoveItemsResolveSkillDataToRealAffects(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, root, catalog)
	expected := []struct {
		skillID, affectType, affectValue int
	}{
		{43, 11, 15},
		{44, 9, 5},
		{45, 15, 7},
		{41, 2, 1},
	}
	for _, itemID := range []uint16{1739, 4145} {
		rule, _, ok := volatiles.Rule(itemID)
		if !ok || len(rule.Affects) != len(expected) {
			t.Fatalf("Love item %d: rule=%+v ok=%v", itemID, rule, ok)
		}
		for index, want := range expected {
			configured := rule.Affects[index]
			skill := catalog.Skills[configured.SkillID]
			if configured.SkillID != want.skillID ||
				skill.AffectType != want.affectType ||
				skill.AffectValue != want.affectValue {
				t.Errorf("item %d affect[%d]: skill=%d => type/value=%d/%d, expected %d=>%d/%d",
					itemID, index, configured.SkillID, skill.AffectType,
					skill.AffectValue, want.skillID, want.affectType, want.affectValue)
			}
		}
	}
}

// During the beta EVERY volatile must be reachable from a shop: without the
// item in hand the rule cannot be exercised in game, and a volatile never
// tested is one that only fails when the server opens.
//
// This test stops coverage from regressing. Implementing a new volatile
// without putting the item in a shop BREAKS here, with the code and action in
// the error -- the same pattern as the counter source test.
//
// Code 184 (Wyden coins) is exempt: the coins hold donate and are sold only by
// the premium shop for donate (DOCS/protocol/premium-shop.md). Selling them
// for gold in an NPC shop would convert gold into donate.
func TestEveryVolatileHasShop(t *testing.T) {
	const premiumOnlyCode = 184
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, root, catalog)
	npcs, err := LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}

	sellerOfCode := make(map[int]string, len(volatiles.Codes))
	for _, npc := range npcs {
		for _, item := range npc.Vende {
			if code, ok := volatiles.ItemCodes[item.Index]; ok {
				// Merchant 1 and low nibble 3 (except the skill master 19)
				// are the purchasable shops; other inventories are quest data.
				merchant := uint32(0)
				if npc.Score != nil {
					merchant = npc.Score.Merchant
				}
				purchasable := merchant == 1 || merchant&0xF == 3 && merchant != 19
				if code == premiumOnlyCode && purchasable {
					t.Errorf("%s sells Wyden coin %d for gold; it is premium-shop only", npc.Name, item.Index)
				}
				if _, seen := sellerOfCode[code]; !seen {
					sellerOfCode[code] = npc.Name
				}
			}
		}
	}

	withoutShop := make([]int, 0)
	for code := range volatiles.Codes {
		if _, ok := sellerOfCode[code]; !ok && code != premiumOnlyCode {
			withoutShop = append(withoutShop, code)
		}
	}
	for _, code := range withoutShop {
		action := "(falls back to the generic default)"
		if rule, ok := volatiles.Rules[code]; ok {
			action = rule.Action
		}
		t.Errorf("volatile %d [%s] is not sold by any NPC: it cannot be "+
			"tested in game", code, action)
	}
}

// TestVolatileShopsFitTheWindow covers ONLY the volatile test shops.
//
// Native shops exceed 27 items on purpose and the server truncates them,
// logging how many were dropped (handlers.go, clientShopSlots). That is fine
// for an ordinary shop, but not here: a volatile outside the window cannot be
// tested, which is the only reason these shops exist.
func TestVolatileShopsFitTheWindow(t *testing.T) {
	// Mirrors clientShopSlots in internal/game/handlers.go. It cannot be
	// imported (game depends on data, not the other way), so the value is
	// repeated here on purpose.
	const clientSlots = 27
	npcs, err := LoadNPCs(filepath.Join("..", "..", "data", "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	found := 0
	for _, npc := range npcs {
		if !strings.HasPrefix(npc.Name, "ShopVol") {
			continue
		}
		found++
		if len(npc.Vende) > clientSlots {
			t.Errorf("%s sells %d items: the client only shows %d and the rest "+
				"would be truncated before reaching the player",
				npc.Name, len(npc.Vende), clientSlots)
		}
	}
	if found == 0 {
		t.Error("no ShopVol* shop found: the beta volatile coverage is gone")
	}
}

// A room configured with a misspelled name would only fail when a player used
// the ticket. Boot also validates this reference, but this test points at the
// exact item without starting the database, world and network.
func TestItemInstancesReferenceExistingNPC(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, root, catalog)
	npcs, err := LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	names := make(map[string]struct{}, len(npcs))
	for _, npc := range npcs {
		names[npc.Name] = struct{}{}
	}
	for itemID, rule := range volatiles.Items {
		if rule.Action != "instance_ticket" || rule.Instance == nil {
			continue
		}
		for _, spawn := range rule.Instance.Spawns {
			if _, ok := names[spawn.NPC]; !ok {
				t.Errorf("item %d instance %q references missing NPC %q",
					itemID, rule.Instance.ID, spawn.NPC)
			}
		}
		for stageIndex, stage := range rule.Instance.Stages {
			for _, spawn := range stage.Spawns {
				if _, ok := names[spawn.NPC]; !ok {
					t.Errorf("item %d instance %q room %d references missing NPC %q",
						itemID, rule.Instance.ID, stageIndex+1, spawn.NPC)
				}
			}
		}
	}
}

func TestRealCatalogHasNoGenericVolatile(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, root, catalog)
	var generic, celestial []uint16
	for itemID := range volatiles.ItemCodes {
		rule, _, ok := volatiles.Rule(itemID)
		if !ok {
			t.Fatalf("volatile item %d did not resolve a rule", itemID)
		}
		switch rule.Action {
		case "generic":
			generic = append(generic, itemID)
		case "celestial_capsule":
			celestial = append(celestial, itemID)
		}
	}
	sort.Slice(generic, func(i, j int) bool { return generic[i] < generic[j] })
	sort.Slice(celestial, func(i, j int) bool { return celestial[i] < celestial[j] })
	if len(generic) != 0 {
		t.Fatalf("items still on generic: %v", generic)
	}
	wantCelestial := []uint16{3443}
	if len(celestial) != len(wantCelestial) {
		t.Fatalf("celestial_capsule=%v, want %v", celestial, wantCelestial)
	}
	for i := range celestial {
		if celestial[i] != wantCelestial[i] {
			t.Fatalf("celestial_capsule=%v, want %v", celestial, wantCelestial)
		}
	}
}

func TestRoundShopsSellEveryNonCelestialVariant(t *testing.T) {
	npcs, err := LoadNPCs(filepath.Join("..", "..", "data", "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	got := make(map[uint16]struct{})
	for _, npc := range npcs {
		if npc.Name != "ShopUse20A" && npc.Name != "ShopUse20B" {
			continue
		}
		for _, item := range npc.Vende {
			got[item.Index] = struct{}{}
		}
	}
	want := []uint16{
		417, 3330, 3336, 3337, 3338, 3339, 3453, 3454, 4003, 4004,
		4005, 4007, 1729, 1730, 4105, 1731, 3171, 3172, 1737, 1772,
		4000, 4001, 3324, 3390, 3325, 3391, 3326, 3392, 3328, 3329,
		3909, 3910, 3974, 3331, 4012,
		4114, 4115, 4116, 3439, 3440, 4147, 3444,
	}
	for _, itemID := range want {
		if _, ok := got[itemID]; !ok {
			t.Errorf("non-Celestial item %d missing from the round shops", itemID)
		}
	}
	// Celestial items stay out; Wyden coins are premium-shop only.
	for _, itemID := range []uint16{3443, 3455, 5338, 3393, 3394, 3395, 3396, 3441} {
		if _, ok := got[itemID]; ok {
			t.Errorf("item %d must not be in the round shops", itemID)
		}
	}
	if len(got) != len(want) {
		t.Fatalf("shops have %d items, expected %d", len(got), len(want))
	}
}
