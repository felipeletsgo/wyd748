package data

import (
	"regexp"
	"testing"
)

// wireNameBytes e quanto CreateMob copia do nome (wire/codec.go:366,
// `copy(b[18:18+12], name)`). Nome maior chega CORTADO no client, sem aviso --
// era o caso de Servo_Carbuncle, Gargula_Gigante e Cavaleiro_Negro antes da
// traducao.
const wireNameBytes = 12

// O client 7.48 e global: todo texto que o jogador le e em ingles, e o nome do
// NPC e o texto mais visivel que existe -- fica flutuando sobre a cabeca dele.
var marcasPortuguesas = regexp.MustCompile(
	`(?i)(guarda|mestre|treinador|ferreiro|sabio|sobreviv|guardiao|gargula|` +
		`cavaleiro|alquimista|arqueiro|coveiro|jardineiro|batedor|` +
		`representante|montaria|^loja|chefe|esqueleto|hidra|servo_|imortal|` +
		`dourada|negro|caveira|gigante)`)

// nomesLongosHerdados e a divida que ja existia antes da traducao: mobs cujo
// nome sempre chegou cortado ao client. Nao e alvo desta rodada -- o teste os
// CONTA para que o numero nao cresca, e falha se algum nome novo estourar.
//
// Baixar este numero e trabalho legitimo: cada nome encurtado e um mob que
// passa a ser lido por inteiro no jogo.
const nomesLongosHerdados = 55

func TestNomesDeNPCCabemNoWire(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	longos := make([]string, 0, nomesLongosHerdados)
	for _, npc := range npcs {
		if len(npc.Name) > wireNameBytes {
			longos = append(longos, npc.Name)
		}
	}
	if len(longos) > nomesLongosHerdados {
		t.Errorf("%d nomes passam de %d bytes, herdados eram %d -- algum nome NOVO "+
			"chega cortado ao client: %v", len(longos), wireNameBytes,
			nomesLongosHerdados, longos)
	}
	if len(longos) < nomesLongosHerdados {
		t.Errorf("agora sao %d nomes longos e nao %d: baixe nomesLongosHerdados "+
			"para travar o ganho", len(longos), nomesLongosHerdados)
	}
}

func TestNomesDeNPCEstaoEmIngles(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	for _, npc := range npcs {
		if marcasPortuguesas.MatchString(npc.Name) {
			t.Errorf("%q parece portugues -- o client 7.48 e global e o nome do "+
				"NPC aparece sobre a cabeca dele", npc.Name)
		}
	}
}

// TestNomesDeNPCSaoUnicos: o nome e a CHAVE usada por quests.json, pelo
// NPCGener e por npcDefByName. Dois NPCs com o mesmo nome fazem a busca
// devolver um deles em silencio.
func TestNomesDeNPCSaoUnicos(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	visto := make(map[string]bool, len(npcs))
	for _, npc := range npcs {
		if visto[npc.Name] {
			t.Errorf("nome duplicado: %q", npc.Name)
		}
		visto[npc.Name] = true
	}
}
