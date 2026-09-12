package data

import (
	"testing"

	"wydgo/internal/model"
)

// Os sete compositores sao entidades permanentes do NPCGener. Os templates
// apenas descrevem aparencia/funcao: nenhum deles pode carregar recompensa de
// combate, estoque de loja comum ou Merchant diferente da janela de craft.
func TestCraftingNPCDataset(t *testing.T) {
	const craftingMerchant = 8
	expected := map[string]uint16{
		"Agatha":     56,
		"Aylin":      55,
		"Compositor": 54,
		"Ehre":       68,
		"Lindy":      67,
		"Odin":       67,
		"Tiny":       68,
	}

	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	byName := make(map[string]model.NPCDef, len(npcs))
	for _, npc := range npcs {
		byName[npc.Name] = npc
	}
	for name, face := range expected {
		npc, ok := byName[name]
		if !ok {
			t.Errorf("template do compositor %q ausente", name)
			continue
		}
		if npc.Tipo != model.TipoNPC || npc.Score == nil ||
			npc.Score.Merchant&0x0F != craftingMerchant {
			t.Errorf("%s: tipo=%q merchant=%v; esperado npc/8",
				name, npc.Tipo, npc.Score)
		}
		if npc.Equip.Rosto.Index != face {
			t.Errorf("%s: face=%d, esperado %d para seletor do client",
				name, npc.Equip.Rosto.Index, face)
		}
		if npc.ExpReward != 0 || npc.Gold != 0 || len(npc.Carry) != 0 || len(npc.Vende) != 0 {
			t.Errorf("%s carrega estado de combate/loja: exp=%d gold=%d carry=%d vende=%d",
				name, npc.ExpReward, npc.Gold, len(npc.Carry), len(npc.Vende))
		}
		if npc.IsMonster() {
			t.Errorf("%s foi classificado como monstro", name)
		}
	}

	geners, err := LoadNPCGener("../../data/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	refs := make(map[string]int, len(expected))
	for _, gener := range geners {
		if _, ok := expected[gener.Leader]; ok {
			refs[gener.Leader]++
		}
	}
	for name := range expected {
		if refs[name] != 1 {
			t.Errorf("%s possui %d geradores; esperado exatamente 1", name, refs[name])
		}
	}
}
