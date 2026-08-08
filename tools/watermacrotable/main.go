// Command watermacrotable generates the client-side lookup used by the
// optional Water Scroll macro. The server's data/volatiles.json is the only
// source of item IDs and entry rectangles; this command merely serializes the
// resolved catalog in a deterministic order for the client patch step.
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

type waterArea struct {
	MinX uint16 `json:"minX"`
	MinY uint16 `json:"minY"`
	MaxX uint16 `json:"maxX"`
	MaxY uint16 `json:"maxY"`
}

type waterScroll struct {
	ItemID uint16      `json:"item"`
	Areas  []waterArea `json:"areas"`
}

func isWaterScroll(rule model.VolatileRule) bool {
	return rule.Action == "instance_ticket" && rule.Instance != nil &&
		strings.HasPrefix(strings.ToLower(strings.TrimSpace(rule.Instance.ID)), "water-")
}

func buildWaterTable(catalog model.VolatileCatalog) ([]waterScroll, error) {
	ids := make([]uint16, 0)
	// ItemCodes is the authoritative set of every item carrying EF_VOLATILE.
	// Rule resolves the item override first, then the code rule, then the
	// default. Iterating Items alone silently omitted valid rule-only tickets.
	for itemID := range catalog.ItemCodes {
		rule, _, ok := catalog.Rule(itemID)
		if !ok {
			continue
		}
		if isWaterScroll(rule) {
			ids = append(ids, itemID)
		}
	}
	sort.Slice(ids, func(i, j int) bool { return ids[i] < ids[j] })
	entries := make([]waterScroll, 0, len(ids))
	for _, itemID := range ids {
		rule, _, ok := catalog.Rule(itemID)
		if !ok {
			return nil, fmt.Errorf("volatile item %d has no resolved rule", itemID)
		}
		areas := rule.Instance.EntryAreas
		if len(areas) == 0 {
			return nil, fmt.Errorf("water item %d has no resolved entry area", itemID)
		}
		entry := waterScroll{ItemID: itemID, Areas: make([]waterArea, 0, len(areas))}
		for index, area := range areas {
			if area.MinX > area.MaxX || area.MinY > area.MaxY {
				return nil, fmt.Errorf("water item %d area %d has inverted bounds", itemID, index)
			}
			entry.Areas = append(entry.Areas, waterArea{
				MinX: area.MinX, MinY: area.MinY, MaxX: area.MaxX, MaxY: area.MaxY,
			})
		}
		entries = append(entries, entry)
	}
	return entries, nil
}

func encodeWaterTable(entries []waterScroll) ([]byte, error) {
	return json.Marshal(entries)
}

func loadCatalog(root string) (model.VolatileCatalog, error) {
	root = filepath.Clean(root)
	catalog, err := data.LoadCatalog(
		filepath.Join(root, "data", "itemlist.csv"),
		filepath.Join(root, "data", "Itemname.csv"),
		filepath.Join(root, "data", "ItemEffect.h"),
		filepath.Join(root, "data", "SkillData.csv"),
	)
	if err != nil {
		return model.VolatileCatalog{}, err
	}
	return data.LoadVolatiles(filepath.Join(root, "data", "volatiles.json"),
		catalog.Items, catalog.Skills)
}

func main() {
	root := flag.String("root", ".", "wyd-go repository root")
	out := flag.String("out", "", "output JSON path; stdout when omitted")
	flag.Parse()
	catalog, err := loadCatalog(*root)
	if err != nil {
		fmt.Fprintf(os.Stderr, "watermacrotable: %v\n", err)
		os.Exit(1)
	}
	entries, err := buildWaterTable(catalog)
	if err != nil {
		fmt.Fprintf(os.Stderr, "watermacrotable: %v\n", err)
		os.Exit(1)
	}
	b, err := encodeWaterTable(entries)
	if err != nil {
		fmt.Fprintf(os.Stderr, "watermacrotable: encode: %v\n", err)
		os.Exit(1)
	}
	if *out == "" {
		_, _ = os.Stdout.Write(append(b, '\n'))
		return
	}
	if err := os.WriteFile(*out, append(b, '\n'), 0o644); err != nil {
		fmt.Fprintf(os.Stderr, "watermacrotable: write %s: %v\n", *out, err)
		os.Exit(1)
	}
	fmt.Printf("water scroll table: %d items -> %s\n", len(entries), *out)
}
