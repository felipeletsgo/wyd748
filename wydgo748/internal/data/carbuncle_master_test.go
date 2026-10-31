package data

import (
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestCarbMastersUseBuffNPCProfile(t *testing.T) {
	npcs, err := LoadNPCs(filepath.Join("..", "..", "data", "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	byName := make(map[string]model.NPCDef, len(npcs))
	for _, npc := range npcs {
		byName[npc.Name] = npc
	}
	for index := 1; index <= 6; index++ {
		name := "Carb_Mstr" + string(rune('0'+index))
		npc, exists := byName[name]
		if !exists {
			t.Fatalf("template %s ausente", name)
		}
		if npc.Tipo != model.TipoNPC || npc.Score == nil || npc.Score.Merchant != 43 ||
			npc.Score.Level < 23 || npc.Score.Level > 26 {
			t.Errorf("perfil de buff invalido em %s: tipo=%q score=%+v", name, npc.Tipo, npc.Score)
		}
	}
}

func TestInactiveNPCDefinitionsAndGeneratorsAreRemoved(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	removed := map[string]bool{"Arch": true, "Compounder": true, "Carbunkle_Anct": true}
	for name := range removed {
		if _, err := os.Stat(filepath.Join(root, "npcs", name+".json")); !os.IsNotExist(err) {
			t.Fatalf("inactive NPC definition %s remains: %v", name, err)
		}
	}
	geners, err := LoadChannelNPCGener(filepath.Join(root, "spawns", "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}
	for _, gener := range geners {
		if removed[gener.Leader] || removed[gener.Follower] {
			t.Fatalf("inactive NPC generator remains: %+v", gener)
		}
	}
}
