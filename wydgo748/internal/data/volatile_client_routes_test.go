package data

import (
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"testing"

	"wydgo/internal/model"
)

// Cross-source contract: read the actual C++ routing tables, not a second copy.
// This checks reachability policy, not execution of the UI or in-game effects.
func TestClientVolatileRoutesMatchAuthoritativeCatalog(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, root, catalog)
	header, err := os.ReadFile(filepath.Join("..", "..", "..", "tmproject", "TMProject748",
		"internal", "application", "NativeVolatileRoutes.h"))
	if err != nil {
		t.Fatal(err)
	}
	parse := func(name string) map[int]string {
		t.Helper()
		_, tail, ok := strings.Cut(string(header), name+"[] = {")
		if !ok {
			t.Fatalf("missing client table %s", name)
		}
		body, _, ok := strings.Cut(tail, "};")
		if !ok {
			t.Fatalf("unterminated client table %s", name)
		}
		rows := regexp.MustCompile(`\{(\d+),\s*(\d+),\s*Route::(\w+)\}`).FindAllStringSubmatch(body, -1)
		if len(rows) == 0 {
			t.Fatalf("empty client table %s", name)
		}
		result := make(map[int]string)
		for _, row := range rows {
			first, err1 := strconv.Atoi(row[1])
			last, err2 := strconv.Atoi(row[2])
			if err1 != nil || err2 != nil || first > last || last > 65535 {
				t.Fatalf("invalid client range %v", row)
			}
			for key := first; key <= last; key++ {
				if _, duplicate := result[key]; duplicate {
					t.Fatalf("overlapping %s route %d", name, key)
				}
				result[key] = row[3]
			}
		}
		return result
	}
	items, codes := parse("ItemRoutes"), parse("CodeRoutes")
	for item, code := range volatiles.ItemCodes {
		t.Run(fmt.Sprintf("item_%d_volatile_%d", item, code), func(t *testing.T) {
			rule, _, ok := volatiles.Rule(item)
			if !ok {
				t.Fatal("missing authoritative rule")
			}
			got, overridden := items[int(item)]
			if !overridden {
				got = codes[code]
			}
			want := expectedVolatileClientRoute(t, rule)
			if got != want {
				t.Fatalf("action=%s mount=%s client route=%q want=%q", rule.Action, rule.MountAction, got, want)
			}
		})
	}
	t.Logf("checked %d items across %d volatile codes", len(volatiles.ItemCodes), len(volatiles.Codes))
}

func expectedVolatileClientRoute(t *testing.T, rule model.VolatileRule) string {
	t.Helper()
	switch rule.Action {
	case "no_direct_use", "ground_gold", "gate_key":
		return "Interaction"
	case "refine", "ore_upgrade", "refine_equipped", "equipment_gem", "tint", "untint", "repliction":
		return "Target"
	case "mount":
		if rule.MountAction == "feed" {
			return "Direct"
		}
		return "Target"
	case "teleport", "warp_saved":
		return "Recall"
	case "hunting_teleport":
		return "Portal"
	case "arch_crystal", "celestial_ideal":
		return "Confirm"
	case "celestial_capsule":
		return "Capsule"
	case "firework":
		if rule.CustomPattern {
			return "CustomFirework"
		}
		return "Direct"
	case "restore", "magical_pill", "grant_next_level", "quest_reward", "buff", "save_position",
		"gold", "instance_ticket", "learn_special_skill", "summon_contract", "face_transform",
		"face_restore", "timed_access", "grant_counter_once", "grant_counter", "territory_pass",
		"mastery_reset", "refine_set", "chaos_remission", "nightmare_ticket", "celestial_fury",
		"celestial_switch", "mount_revive", "loot_box":
		return "Direct"
	default:
		t.Fatalf("unclassified authoritative action %q", rule.Action)
		return ""
	}
}
