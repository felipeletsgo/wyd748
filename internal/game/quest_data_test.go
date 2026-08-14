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
		"Gravedigger": {39, 115, 2398, 2105},
		"Gardener":    {115, 189, 2234, 1714},
		"Scout":       {190, 264, 464, 3902},
		"Guard__":     {265, 319, 668, 3756},
		"Envoy":       {319, 349, 1322, 4041},
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
		// No client 7.48 estas cinco quests de level sao portais por faixa:
		// nenhum item 4038..4042 pode bloquear ou ser consumido na entrada.
		if len(quest.Requires.Items) != 0 || len(quest.Consumes) != 0 {
			t.Errorf("quest %s voltou a exigir item de entrada: requires=%+v consumes=%+v",
				quest.NPC, quest.Requires.Items, quest.Consumes)
		}
		// Reproduz o bug real: personagem na faixa, inventario completamente
		// vazio. A validacao runtime precisa autorizar a entrada.
		player := questTestPlayer(want.min, 0)
		if reason, allowed := (&World{}).questRequirementsMet(player, quest); !allowed {
			t.Errorf("quest %s recusou entrada sem item na faixa correta: %s",
				quest.NPC, reason)
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
	volatiles, err := data.LoadVolatilesWithInstances(filepath.Join(root, "volatiles.json"),
		filepath.Join(root, "instances.txt"), catalog.Items, catalog.Skills)
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
		"Aparicao": 4117, "Grande_Carb": 4118, "Kaizen_Kn": 4119,
		"Gold_Hydra1": 4120, "Elf_Master": 4121,
	}
	followers := map[string]bool{
		"Skeleton": true, "Carb_Servant": true, "Servant_Kn": true,
		"Immort_Hydra": true, "Elf_Servant1": true,
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
	royal := byNPC["Drill_Master"]
	if !royal.Repeatable || !royal.Requires.MortalOnly ||
		royal.Requires.MinLevel != 199 || royal.Requires.MaxLevel != 253 ||
		royal.Rewards.Teleport == nil || royal.Rewards.Teleport.X != 1740 ||
		royal.Rewards.Teleport.Y != 1725 {
		t.Fatalf("QUEST_CAPAREAL divergiu do W2PP: %+v", royal)
	}
	ameria := byNPC["Priest_Ameria"]
	if !ameria.Repeatable || !ameria.Requires.MortalOnly ||
		ameria.Requires.MinLevel != 119 || ameria.Requires.MaxLevel != 123 ||
		len(ameria.Consumes) != 1 || ameria.Consumes[0].Index != 4125 ||
		len(ameria.Rewards.Items) != 1 || ameria.Rewards.Items[0].Index != 4126 {
		t.Fatalf("AMELIA/Priest Ameria divergiu do W2PP: %+v", ameria)
	}
}

// TestItensDeEntradaTemFonteViva protege as OUTRAS quests que realmente exigem
// itens. As cinco quests de level 1/3/4/5/6 sao intencionalmente entrada livre
// dentro da faixa e, portanto, nao aparecem nesta varredura.
// lacunasDeFonte sao itens consumidos por quest que hoje NAO tem como ser
// obtidos. Ficam listados em vez de silenciados: o teste registra cada um no
// log, e RECLAMA se algum ganhar fonte e ninguem tirar daqui -- assim a lista
// so encolhe.
//
// Esta vazia. O 4125 (Balance's_Piece) esteve aqui por engano meu: eu havia
// procurado o item no MobDropList do W2PP pelo nome sem o acento de
// "Equilibrio" e concluido que nao existia fonte. Ele dropa do InionTrollChief,
// nos slots 9 e 10, exatamente como o 4123 dropa do NainTrollChief.
var lacunasDeFonte = map[uint16]string{}

func TestItensDeEntradaTemFonteViva(t *testing.T) {
	root := filepath.Join("..", "..", "data")

	quests, err := data.LoadQuests(filepath.Join(root, "quests.json"))
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := data.LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	geners, err := data.LoadNPCGener(filepath.Join(root, "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}

	// Quem realmente nasce no mundo: lider e follower de gerador habilitado.
	vivos := make(map[string]bool)
	for _, g := range geners {
		if !g.Enabled || g.MaxNumMob <= 0 {
			continue
		}
		for _, nome := range []string{g.Leader, g.Follower} {
			if nome != "" {
				vivos[generName(nome)] = true
			}
		}
	}
	if len(vivos) == 0 {
		t.Fatal("nenhum mob vivo lido do NPCGener; o teste nao provaria nada")
	}

	// Vale para o que a quest CONSOME e para o que ela apenas EXIGE: nos dois
	// casos o jogador precisa conseguir o item em algum lugar.
	for i := range quests.Quests {
		quest := &quests.Quests[i]
		exigidos := append(append([]model.QuestItem(nil), quest.Consumes...),
			quest.Requires.Items...)
		vistos := make(map[uint16]bool, len(exigidos))
		for _, consumido := range exigidos {
			if vistos[consumido.Index] {
				continue
			}
			vistos[consumido.Index] = true
			var fontes, fontesVivas []string
			for j := range npcs {
				for _, carregado := range npcs[j].Carry {
					if carregado.Index != consumido.Index {
						continue
					}
					nome := generName(npcs[j].Name)
					fontes = append(fontes, nome)
					if vivos[nome] {
						fontesVivas = append(fontesVivas, nome)
					}
					break
				}
			}
			motivo, conhecida := lacunasDeFonte[consumido.Index]
			temFonte := len(fontesVivas) > 0

			if conhecida && temFonte {
				t.Errorf("o item %d ganhou fonte (%v): tire-o de lacunasDeFonte",
					consumido.Index, fontesVivas)
				continue
			}
			if conhecida {
				t.Logf("LACUNA CONHECIDA: quest %d (%s) consome o item %d -- %s",
					quest.ID, quest.NPC, consumido.Index, motivo)
				continue
			}
			if len(fontes) == 0 {
				t.Errorf("quest %d (%s) consome o item %d, que NENHUM mob dropa",
					quest.ID, quest.NPC, consumido.Index)
				continue
			}
			if !temFonte {
				t.Errorf("quest %d (%s) consome o item %d, que so dropa de mob que "+
					"nao nasce no mundo: %v", quest.ID, quest.NPC, consumido.Index, fontes)
			}
		}
	}
}
