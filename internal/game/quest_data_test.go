package game

import (
	"path/filepath"
	"testing"

	"wydgo/internal/data"
)

// TestQuestsDeDadosReaisSaoValidas carrega o data/quests.json e o data/npcs de
// verdade e roda o mesmo cruzamento do boot. Uma quest apontando para NPC
// inexistente ou reservado quebra AQUI, no `go test`, em vez de derrubar o
// servidor na frente do jogador.
func TestQuestsDeDadosReaisSaoValidas(t *testing.T) {
	root := filepath.Join("..", "..", "data")

	quests, err := data.LoadQuests(filepath.Join(root, "quests.json"))
	if err != nil {
		t.Fatalf("data/quests.json invalido: %v", err)
	}
	npcs, err := data.LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatalf("carregar data/npcs: %v", err)
	}

	index, err := indexQuests(quests, npcs)
	if err != nil {
		t.Fatalf("quests nao casam com os NPCs reais: %v", err)
	}
	if len(index) != len(quests.Quests) {
		t.Fatalf("indice com %d quests, arquivo tem %d", len(index), len(quests.Quests))
	}

	// Toda quest configurada precisa cair no fallback generico do client, que e
	// o unico caminho que gera o 0x28B. Se o NPC fosse de um tipo com handler
	// proprio, indexQuests ja teria falhado acima -- este check documenta a
	// intencao e protege se o vocabulario de tipos mudar.
	for i := range quests.Quests {
		quest := &quests.Quests[i]
		var found bool
		for j := range npcs {
			if npcs[j].Name == quest.NPC || generName(npcs[j].Name) == quest.NPC {
				found = true
				if _, reserved := reservedNPCKind(&npcs[j]); reserved {
					t.Fatalf("quest %d esta em npc reservado %q", quest.ID, quest.NPC)
				}
			}
		}
		if !found {
			t.Fatalf("quest %d referencia npc inexistente %q", quest.ID, quest.NPC)
		}
	}
}
