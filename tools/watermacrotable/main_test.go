package main

import (
	"bytes"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestBuildWaterTableExcludesNonWaterAndSorts(t *testing.T) {
	catalog := model.VolatileCatalog{
		ItemCodes: map[uint16]int{5: 1, 9: 2, 10: 3, 20: 4, 21: 4},
		Rules: map[int]model.VolatileRule{
			4: {Action: "instance_ticket", Instance: &model.VolatileInstance{
				ID: "water-normal-rule-only", EntryAreas: []model.VolatileInstanceEntryArea{{MinX: 20, MinY: 21, MaxX: 22, MaxY: 23}},
			}},
		},
		Items: map[uint16]model.VolatileRule{
			9: {Action: "teleport"},
			20: {Action: "instance_ticket", Instance: &model.VolatileInstance{
				ID: "water-normal-2", EntryAreas: []model.VolatileInstanceEntryArea{{MinX: 2, MinY: 3, MaxX: 4, MaxY: 5}},
			}},
			10: {Action: "instance_ticket", Instance: &model.VolatileInstance{
				ID: "nightmare-normal", EntryAreas: []model.VolatileInstanceEntryArea{{MinX: 1, MinY: 1, MaxX: 1, MaxY: 1}},
			}},
			5: {Action: "instance_ticket", Instance: &model.VolatileInstance{
				ID: "water-normal-1", EntryAreas: []model.VolatileInstanceEntryArea{{MinX: 10, MinY: 11, MaxX: 12, MaxY: 13}},
			}},
		},
	}
	entries, err := buildWaterTable(catalog)
	if err != nil {
		t.Fatal(err)
	}
	if len(entries) != 3 || entries[0].ItemID != 5 || entries[1].ItemID != 20 || entries[2].ItemID != 21 {
		t.Fatalf("tabela Water incorreta: %+v", entries)
	}
}

func TestBuildWaterTableRejectsMissingOrInvalidArea(t *testing.T) {
	for name, rule := range map[string]model.VolatileRule{
		"missing": {Action: "instance_ticket", Instance: &model.VolatileInstance{ID: "water-normal-1"}},
		"inverted": {Action: "instance_ticket", Instance: &model.VolatileInstance{
			ID: "water-normal-1", EntryAreas: []model.VolatileInstanceEntryArea{{MinX: 4, MinY: 1, MaxX: 3, MaxY: 2}},
		}},
	} {
		t.Run(name, func(t *testing.T) {
			_, err := buildWaterTable(model.VolatileCatalog{
				ItemCodes: map[uint16]int{1: 1},
				Items:     map[uint16]model.VolatileRule{1: rule},
			})
			if err == nil {
				t.Fatal("tabela invalida aceita")
			}
		})
	}
}

func TestRealWaterTableIsDeterministicAndResolved(t *testing.T) {
	root := filepath.Join("..", "..")
	catalog, err := loadCatalog(root)
	if err != nil {
		t.Fatal(err)
	}
	first, err := buildWaterTable(catalog)
	if err != nil {
		t.Fatal(err)
	}
	second, err := buildWaterTable(catalog)
	if err != nil {
		t.Fatal(err)
	}
	one, err := encodeWaterTable(first)
	if err != nil {
		t.Fatal(err)
	}
	two, err := encodeWaterTable(second)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(one, two) {
		t.Fatal("a tabela Water nao e deterministica")
	}
	if len(first) == 0 {
		t.Fatal("nenhum Water Scroll resolvido")
	}
	for _, entry := range first {
		if entry.ItemID == 0 || len(entry.Areas) == 0 {
			t.Fatalf("entrada Water incompleta: %+v", entry)
		}
	}
}
