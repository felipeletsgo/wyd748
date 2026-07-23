package game

import (
	"path/filepath"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
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

func TestCadeiaPrincipalTemRecompensasEDropsNativos(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	quests, err := data.LoadQuests(filepath.Join(root, "quests.json"))
	if err != nil {
		t.Fatal(err)
	}
	type questRange struct {
		min, max uint32
		x, y     uint16
	}
	wantQuests := map[string]questRange{
		"Coveiro":       {39, 115, 2398, 2105},
		"Jardineiro":    {115, 189, 2234, 1714},
		"Batedor":       {190, 264, 464, 3902},
		"Guarda":        {265, 319, 668, 3756},
		"Representante": {319, 349, 1322, 4041},
	}
	seenQuests := make(map[string]bool)
	for i := range quests.Quests {
		quest := &quests.Quests[i]
		want, ok := wantQuests[quest.NPC]
		if !ok {
			continue // Kibita e independente da cadeia principal.
		}
		seenQuests[quest.NPC] = true
		if !quest.Repeatable || !quest.Requires.MortalOnly ||
			quest.Requires.MinLevel != want.min || quest.Requires.MaxLevel != want.max ||
			quest.Rewards.Teleport == nil || quest.Rewards.Teleport.X != want.x ||
			quest.Rewards.Teleport.Y != want.y {
			t.Fatalf("quest %s divergiu da tabela nativa: %+v", quest.NPC, *quest)
		}
	}
	if len(seenQuests) != len(wantQuests) {
		t.Fatalf("quests principais encontradas=%v", seenQuests)
	}

	catalog, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := data.LoadVolatiles(filepath.Join(root, "volatiles.json"), catalog.Items)
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := data.LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	rates, err := data.LoadDropRates(filepath.Join(root, "droprate.json"))
	if err != nil {
		t.Fatal(err)
	}
	if rates[56] != 1 {
		t.Fatalf("slot 56 deveria ser garantido, rate=%d", rates[56])
	}

	leaders := map[string]uint16{
		"Aparicao": 4117, "Grande_Carb": 4118, "Cav._Kaizen": 4119,
		"Hidra_Dourada": 4120, "Mestre_Elfo": 4121,
	}
	followers := map[string]bool{
		"Esqueleto": true, "Servo_Carbuncle": true, "Cav._Servo": true,
		"Hidra_Imortal": true, "Servo_Elfo": true,
	}
	seen := make(map[string]bool)
	for i := range npcs {
		npc := &npcs[i]
		if item, ok := leaders[npc.Name]; ok {
			seen[npc.Name] = true
			if len(npc.Carry) <= 56 || npc.Carry[56].Index != item {
				t.Fatalf("%s deveria carregar %d no slot 56", npc.Name, item)
			}
			rule, code, registered := volatiles.Rule(item)
			if !registered || code != 191 || rule.Action != "quest_reward" || !rule.Consume {
				t.Fatalf("item %d sem quest_reward Vol 191: %+v code=%d", item, rule, code)
			}
		}
		if followers[npc.Name] {
			for _, carried := range npc.Carry {
				if carried.Index >= 4117 && carried.Index <= 4121 {
					t.Fatalf("follower %s vazou caixa %d", npc.Name, carried.Index)
				}
			}
		}
	}
	if len(seen) != len(leaders) {
		t.Fatalf("lideres encontrados=%v, esperava %d", seen, len(leaders))
	}

}

func TestQuestsComplementaresConfirmadasNaSource(t *testing.T) {
	quests, err := data.LoadQuests(filepath.Join("..", "..", "data", "quests.json"))
	if err != nil {
		t.Fatal(err)
	}
	byNPC := make(map[string]model.QuestDef, len(quests.Quests))
	for _, quest := range quests.Quests {
		byNPC[quest.NPC] = quest
	}
	royal := byNPC["Chefe_de_Treino"]
	if !royal.Repeatable || !royal.Requires.MortalOnly ||
		royal.Requires.MinLevel != 199 || royal.Requires.MaxLevel != 253 ||
		royal.Rewards.Teleport == nil || royal.Rewards.Teleport.X != 1740 ||
		royal.Rewards.Teleport.Y != 1725 {
		t.Fatalf("QUEST_CAPAREAL divergiu do W2PP: %+v", royal)
	}
	helen := byNPC["Helen"]
	if !helen.Repeatable || !helen.Requires.MortalOnly ||
		helen.Requires.MinLevel != 119 || helen.Requires.MaxLevel != 123 ||
		len(helen.Consumes) != 1 || helen.Consumes[0].Index != 4125 ||
		len(helen.Rewards.Items) != 1 || helen.Rewards.Items[0].Index != 4126 {
		t.Fatalf("AMELIA/Helen divergiu do W2PP: %+v", helen)
	}
}
