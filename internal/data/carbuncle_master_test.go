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
		if npc.Tipo != model.TipoNPC || npc.Extended == nil || npc.Extended.Merchant != 43 ||
			npc.Extended.Level < 23 || npc.Extended.Level > 26 {
			t.Errorf("perfil de buff invalido em %s: tipo=%q score=%+v", name, npc.Tipo, npc.Extended)
		}
	}
}

func TestArchRemovedAndAncientCarbuncleDisabled(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	if _, err := os.Stat(filepath.Join(root, "npcs", "Arch.json")); !os.IsNotExist(err) {
		t.Fatalf("template Arch deveria ter sido removido, stat err=%v", err)
	}
	geners, err := LoadNPCGener(filepath.Join(root, "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}
	ancientSections := 0
	for _, gener := range geners {
		if gener.Leader == "Arch" || gener.Follower == "Arch" {
			t.Fatalf("gerador residual do NPC Arch: %+v", gener)
		}
		if gener.Leader == "Carbunkle_Anct" {
			ancientSections++
			if gener.Enabled {
				t.Errorf("Carbunkle_Anct continua ativo: %+v", gener)
			}
		}
	}
	if ancientSections != 2 {
		t.Fatalf("secoes preservadas de Carbunkle_Anct=%d, esperado 2", ancientSections)
	}
}
