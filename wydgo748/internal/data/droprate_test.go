package data

import (
	"encoding/json"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestLoadDropRatesRequiresExactly64Slots(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "droprate.json")
	rates := make([]int, model.MaxCarry)
	for i := range rates {
		rates[i] = i + 1
	}
	writeRates := func(values []int) {
		t.Helper()
		b, err := json.Marshal(map[string]any{"rates": values})
		if err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(path, b, 0o600); err != nil {
			t.Fatal(err)
		}
	}

	writeRates(rates)
	got, err := LoadDropRates(path)
	if err != nil || got[0] != 1 || got[63] != 64 {
		t.Fatalf("tabela valida: rates[0]=%d rates[63]=%d err=%v", got[0], got[63], err)
	}
	writeRates(rates[:63])
	if _, err := LoadDropRates(path); err == nil {
		t.Fatal("tabela com 63 slots foi aceita")
	}
}

func TestNPCRejectsCarryAboveNativeLimit(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "mob.json")
	npc := model.NPCDef{Name: "carry-invalido", Carry: make([]model.Item, model.MaxCarry+1)}
	b, err := json.Marshal(npc)
	if err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(path, b, 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadNPCs(path); err == nil {
		t.Fatal("NPC com 65 slots de carry foi aceito")
	}
}
