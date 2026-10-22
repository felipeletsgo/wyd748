// Package staffcontent builds the staff-only cross-reference index of the
// static game data (DOCS/server/staff-operations.md): which NPC sells an item,
// which monster drops it and at which slot rate, which quest uses it, where a
// monster spawns, and which references do not resolve.
//
// It is built once at server start from the same loaded inputs the World
// receives, then never changes. It is served only through the authenticated
// panel API: the panel's static assets are also copied to the public site, so
// this data must not be bundled into them.
package staffcontent

import (
	"fmt"
	"sort"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/webcatalog"
)

type SaleRef struct {
	NPC  string `json:"npc"`
	Slot int    `json:"slot"`
}

// DropRef names a monster carry slot. OneIn is the effective rate the World
// rolls for this monster (model.DropRateForSlot: level band, special slots
// and cap) before the killer's drop bonus; each kill drops the slot when
// rand()%OneIn == 0. Always marks a slot that drops on every kill (slot 11,
// or an effective rate of zero). BaseOneIn is the unadjusted droprate.json
// value.
type DropRef struct {
	Mob       string `json:"mob"`
	Slot      int    `json:"slot"`
	OneIn     int    `json:"oneIn"`
	Always    bool   `json:"always"`
	BaseOneIn int    `json:"baseOneIn"`
}

type QuestRef struct {
	ID   int    `json:"id"`
	Name string `json:"name"`
	Role string `json:"role"`
}

type ItemRefs struct {
	ID        uint16     `json:"id"`
	Name      string     `json:"name"`
	SoldBy    []SaleRef  `json:"soldBy"`
	DroppedBy []DropRef  `json:"droppedBy"`
	Quests    []QuestRef `json:"quests"`
}

// SpawnRef is one NPCGener entry using the template. Role is "leader",
// "follower" or "both" when the generator uses the template for both.
// Reserved names the feature that owns an enabled generator the World skips
// at boot ("item_instance" or "colosseum"); such entries never spawn in the
// open world.
type SpawnRef struct {
	Generator int    `json:"generator"`
	Enabled   bool   `json:"enabled"`
	Reserved  string `json:"reserved,omitempty"`
	Role      string `json:"role"`
	X         uint16 `json:"x"`
	Y         uint16 `json:"y"`
	MaxMobs   int    `json:"maxMobs"`
}

type MobRefs struct {
	Name     string     `json:"name"`
	Kind     string     `json:"kind"`
	Level    uint32     `json:"level"`
	Gold     uint32     `json:"gold"`
	Drops    []DropItem `json:"drops"`
	Sells    []DropItem `json:"sells"`
	Spawns   []SpawnRef `json:"spawns"`
	Merchant bool       `json:"merchant"`
}

type DropItem struct {
	ItemID uint16 `json:"itemId"`
	Name   string `json:"name"`
	Slot   int    `json:"slot"`
	OneIn  int    `json:"oneIn,omitempty"`
	Always bool   `json:"always,omitempty"`
	// BaseOneIn is the unadjusted droprate.json value; see DropRef.
	BaseOneIn int `json:"baseOneIn,omitempty"`
}

// Issue is one unresolved reference. Severity "error" would also fail the
// boot for an enabled, unreserved generator; "warning" is reachable data that
// is unused or inconsistent but does not stop the server.
type Issue struct {
	Severity string `json:"severity"`
	Kind     string `json:"kind"`
	Subject  string `json:"subject"`
	Detail   string `json:"detail"`
}

type Index struct {
	BuiltAt time.Time
	items   map[uint16]*ItemRefs
	mobs    map[string]*MobRefs
	names   []nameEntry
	Issues  []Issue
}

type nameEntry struct {
	lower string
	kind  string
	key   string
}

func itemName(catalog model.Catalog, id uint16) string {
	if def, ok := catalog.Items[id]; ok && def.Name != "NULL" {
		return strings.ReplaceAll(def.Name, "_", " ")
	}
	return ""
}

func generName(s string) string { return strings.ReplaceAll(s, "_", " ") }

// Build indexes the loaded data. It reports issues instead of failing: the
// World's own loaders remain the authority for what may boot. reserved is the
// World's set of enabled generators skipped before the template check
// (game.World.ReservedGenerators); nil means none.
func Build(npcs []model.NPCDef, geners []model.NPCGener, catalog model.Catalog, quests model.QuestFile,
	zones model.QuestZoneFile, dropRates [model.MaxCarry]int, reserved map[int]string, now time.Time) *Index {
	ix := &Index{BuiltAt: now.UTC(), items: map[uint16]*ItemRefs{}, mobs: map[string]*MobRefs{}, Issues: []Issue{}}
	item := func(id uint16) *ItemRefs {
		refs := ix.items[id]
		if refs == nil {
			refs = &ItemRefs{ID: id, Name: itemName(catalog, id), SoldBy: []SaleRef{}, DroppedBy: []DropRef{}, Quests: []QuestRef{}}
			ix.items[id] = refs
		}
		return refs
	}
	for id, def := range catalog.Items {
		if id != 0 && def.Name != "NULL" && strings.TrimSpace(def.Name) != "" {
			item(id)
		}
	}
	templates := map[string]*MobRefs{}
	for _, npc := range npcs {
		// The persisted type values are legacy identifiers; the API uses English.
		kind := "npc"
		if npc.Tipo == model.TipoMonstro {
			kind = "monster"
		}
		mob := &MobRefs{Name: npc.Name, Kind: kind, Gold: npc.Gold, Drops: []DropItem{}, Sells: []DropItem{}, Spawns: []SpawnRef{}, Merchant: len(npc.Vende) > 0}
		if npc.Score != nil {
			mob.Level = npc.Score.Level
		}
		for slot, carried := range npc.Carry {
			if carried.Index == 0 || slot >= model.MaxCarry {
				continue
			}
			oneIn, always := model.DropRateForSlot(dropRates, slot, int(mob.Level))
			always = always || oneIn <= 0
			if _, known := catalog.Items[carried.Index]; !known {
				ix.Issues = append(ix.Issues, Issue{"warning", "drop_unknown_item", npc.Name, fmt.Sprintf("carry slot %d holds unknown item %d", slot, carried.Index)})
				continue
			}
			mob.Drops = append(mob.Drops, DropItem{ItemID: carried.Index, Name: itemName(catalog, carried.Index), Slot: slot,
				OneIn: oneIn, Always: always, BaseOneIn: dropRates[slot]})
			item(carried.Index).DroppedBy = append(item(carried.Index).DroppedBy,
				DropRef{Mob: npc.Name, Slot: slot, OneIn: oneIn, Always: always, BaseOneIn: dropRates[slot]})
		}
		for slot, sold := range npc.Vende {
			if sold.Index == 0 {
				continue
			}
			if _, known := catalog.Items[sold.Index]; !known {
				ix.Issues = append(ix.Issues, Issue{"warning", "sale_unknown_item", npc.Name, fmt.Sprintf("sale slot %d holds unknown item %d", slot, sold.Index)})
				continue
			}
			mob.Sells = append(mob.Sells, DropItem{ItemID: sold.Index, Name: itemName(catalog, sold.Index), Slot: slot})
			item(sold.Index).SoldBy = append(item(sold.Index).SoldBy, SaleRef{NPC: npc.Name, Slot: slot})
		}
		if _, duplicate := ix.mobs[npc.Name]; duplicate {
			ix.Issues = append(ix.Issues, Issue{"warning", "duplicate_template", npc.Name, "two NPC templates share this name"})
		}
		ix.mobs[npc.Name] = mob
		templates[npc.Name] = mob
		templates[generName(npc.Name)] = mob
	}
	for _, g := range geners {
		reservedBy := ""
		if g.Enabled {
			reservedBy = reserved[g.Index]
		}
		var leader *MobRefs
		for _, part := range []struct{ role, name string }{{"leader", g.Leader}, {"follower", g.Follower}} {
			mob := templates[part.name]
			if mob == nil {
				mob = templates[generName(part.name)]
			}
			if mob == nil {
				// Only an enabled generator the World actually loads fails the boot.
				severity, detail := "warning", fmt.Sprintf("%s %q has no NPC template", part.role, part.name)
				if g.Enabled && reservedBy == "" {
					severity = "error"
				}
				if reservedBy != "" {
					detail += fmt.Sprintf(" (reserved for %s; skipped at boot)", strings.ReplaceAll(reservedBy, "_", " "))
				}
				ix.Issues = append(ix.Issues, Issue{severity, "generator_missing_template", fmt.Sprintf("NPCGener[%d]", g.Index), detail})
				continue
			}
			if part.role == "follower" && mob == leader {
				mob.Spawns[len(mob.Spawns)-1].Role = "both"
				continue
			}
			if part.role == "leader" {
				leader = mob
			}
			mob.Spawns = append(mob.Spawns, SpawnRef{Generator: g.Index, Enabled: g.Enabled, Reserved: reservedBy, Role: part.role,
				X: g.Segments[0].X, Y: g.Segments[0].Y, MaxMobs: g.MaxNumMob})
		}
	}
	if projected, err := webcatalog.ProjectQuests(quests, zones, catalogItems(catalog)); err != nil {
		ix.Issues = append(ix.Issues, Issue{"warning", "quest_projection", "quests.json", err.Error()})
	} else {
		for _, quest := range projected {
			for _, part := range []struct {
				role  string
				items []webcatalog.QuestItem
			}{{"required", quest.RequiredItems}, {"consumed", quest.ConsumedItems}, {"reward", quest.RewardItems}} {
				for _, used := range part.items {
					if _, known := catalog.Items[used.ID]; !known {
						ix.Issues = append(ix.Issues, Issue{"warning", "quest_unknown_item", quest.Name, fmt.Sprintf("%s item %d is not in the catalog", part.role, used.ID)})
						continue
					}
					refs := item(used.ID)
					refs.Quests = append(refs.Quests, QuestRef{ID: quest.ID, Name: quest.Name, Role: part.role})
				}
			}
		}
	}
	for id, refs := range ix.items {
		ix.names = append(ix.names, nameEntry{lower: strings.ToLower(refs.Name), kind: "item", key: fmt.Sprint(id)})
	}
	for name := range ix.mobs {
		ix.names = append(ix.names, nameEntry{lower: strings.ToLower(name), kind: "mob", key: name})
	}
	sort.Slice(ix.names, func(i, j int) bool { return ix.names[i].lower < ix.names[j].lower })
	sort.Slice(ix.Issues, func(i, j int) bool {
		if ix.Issues[i].Severity != ix.Issues[j].Severity {
			return ix.Issues[i].Severity == "error"
		}
		return ix.Issues[i].Subject < ix.Issues[j].Subject
	})
	return ix
}

func catalogItems(catalog model.Catalog) []webcatalog.Item {
	items := make([]webcatalog.Item, 0, len(catalog.Items))
	for id, def := range catalog.Items {
		if id != 0 && def.Name != "NULL" {
			items = append(items, webcatalog.Item{ID: id, Name: itemName(catalog, id)})
		}
	}
	sort.Slice(items, func(i, j int) bool { return items[i].ID < items[j].ID })
	return items
}

func (ix *Index) Item(id uint16) (ItemRefs, bool) {
	refs, ok := ix.items[id]
	if !ok {
		return ItemRefs{}, false
	}
	return *refs, true
}

func (ix *Index) Mob(name string) (MobRefs, bool) {
	refs, ok := ix.mobs[name]
	if !ok {
		return MobRefs{}, false
	}
	return *refs, true
}

type SearchHit struct {
	Kind string `json:"kind"`
	Key  string `json:"key"`
	Name string `json:"name"`
}

// Search returns up to limit items and monsters whose name contains query.
func (ix *Index) Search(query string, limit int) []SearchHit {
	query = strings.ToLower(strings.TrimSpace(query))
	hits := []SearchHit{}
	if query == "" {
		return hits
	}
	for _, entry := range ix.names {
		if !strings.Contains(entry.lower, query) {
			continue
		}
		name := entry.key
		if entry.kind == "item" {
			name = ix.items[parseID(entry.key)].Name
		}
		hits = append(hits, SearchHit{Kind: entry.kind, Key: entry.key, Name: name})
		if len(hits) == limit {
			break
		}
	}
	return hits
}

func parseID(key string) uint16 {
	var id uint16
	_, _ = fmt.Sscan(key, &id)
	return id
}
