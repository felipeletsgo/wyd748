package data

import (
	"encoding/json"
	"os"
	"path/filepath"
	"testing"
)

// Renomear um NPC quebra tudo que o referencia PELO NOME. As quests ja tinham
// guarda; os bosses e as zonas nao tinham, e por isso um `npc = "Arq_Ciclope"`
// sobreviveu a traducao e so apareceu como falha de boot:
//
//	criar mundo: boss "arquimago_negro": NPC base "Arq_Ciclope" nao existe
//
// Estes testes trazem essa deteccao para a suite. Um nome de NPC quebrado passa
// a falhar em segundos, e nao no arranque do servidor.

func nomesDeNPC(t *testing.T) map[string]bool {
	t.Helper()
	npcs, err := LoadNPCs(filepath.Join("..", "..", "data", "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	nomes := make(map[string]bool, len(npcs))
	for _, npc := range npcs {
		nomes[npc.Name] = true
	}
	return nomes
}

func TestBossesReferenciamNPCsExistentes(t *testing.T) {
	nomes := nomesDeNPC(t)
	catalogo, err := LoadBossCatalog(filepath.Join("..", "..", "data", "boss"))
	if err != nil {
		t.Fatal(err)
	}
	if len(catalogo.Bosses) == 0 {
		t.Fatal("nenhum encontro de boss em data/boss")
	}
	for _, boss := range catalogo.Bosses {
		// O NPC base do boss e o NPC de cada grupo de adds invocavel: os dois
		// sao resolvidos pelo nome no catalogo, e os dois quebram o boot.
		refs := []string{boss.NPC}
		for _, summon := range boss.Summons {
			refs = append(refs, summon.NPC)
		}
		for _, ref := range refs {
			if ref != "" && !nomes[ref] {
				t.Errorf("boss %q referencia o NPC %q, que nao existe em data/npcs",
					boss.ID, ref)
			}
		}
	}
}

func TestZonasDeQuestReferenciamNPCsExistentes(t *testing.T) {
	nomes := nomesDeNPC(t)
	b, err := os.ReadFile(filepath.Join("..", "..", "data", "quest_zones.json"))
	if err != nil {
		t.Fatal(err)
	}
	var arquivo struct {
		Zones []struct {
			Name string `json:"name"`
		} `json:"zones"`
	}
	if err := json.Unmarshal(b, &arquivo); err != nil {
		t.Fatal(err)
	}
	if len(arquivo.Zones) == 0 {
		t.Fatal("nenhuma zona em quest_zones.json")
	}
	for _, zona := range arquivo.Zones {
		if !nomes[zona.Name] {
			t.Errorf("zona %q nao corresponde a nenhum NPC em data/npcs", zona.Name)
		}
	}
}
