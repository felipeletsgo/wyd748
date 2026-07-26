package data

import (
	"path/filepath"
	"strings"
	"testing"
)

// Durante o beta TODO volatile precisa ser alcancavel por uma loja: sem item na
// mao nao ha como exercitar a regra in-game, e um volatile nunca testado e um
// volatile que so falha quando o servidor abre.
//
// Este teste e o que impede a cobertura de regredir. Implementar um volatile
// novo sem colocar o item numa loja passa a QUEBRAR aqui, com o codigo e a
// acao no erro -- e o mesmo padrao do TestNenhumContadorEhGastoSemFonte.
func TestTodoVolatileTemLoja(t *testing.T) {
	raiz := filepath.Join("..", "..", "data")
	catalogo, err := LoadCatalog(filepath.Join(raiz, "itemlist.csv"),
		filepath.Join(raiz, "Itemname.csv"), filepath.Join(raiz, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volateis, err := LoadVolatiles(filepath.Join(raiz, "volatiles.json"), catalogo.Items)
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := LoadNPCs(filepath.Join(raiz, "npcs"))
	if err != nil {
		t.Fatal(err)
	}

	vendedorDoCodigo := make(map[int]string, len(volateis.Codes))
	for _, npc := range npcs {
		for _, item := range npc.Vende {
			if code, ok := volateis.ItemCodes[item.Index]; ok {
				if _, visto := vendedorDoCodigo[code]; !visto {
					vendedorDoCodigo[code] = npc.Name
				}
			}
		}
	}

	semLoja := make([]int, 0)
	for code := range volateis.Codes {
		if _, ok := vendedorDoCodigo[code]; !ok {
			semLoja = append(semLoja, code)
		}
	}
	for _, code := range semLoja {
		acao := "(cai no default generic)"
		if regra, ok := volateis.Rules[code]; ok {
			acao = regra.Action
		}
		t.Errorf("volatile %d [%s] nao e vendido por nenhum NPC: nao ha como "+
			"testa-lo in-game", code, acao)
	}
}

// TestLojasDeVolatileCabemNaJanela cobre SO as lojas de teste de volatile.
//
// As lojas nativas passam bem de 27 itens de proposito e o servidor as trunca,
// registrando quantos caiu (handlers.go, clientShopSlots). Isso e aceitavel
// numa loja de venda comum, mas nao aqui: um volatile que cai fora da janela
// nao pode ser testado, que e a unica razao destas lojas existirem.
func TestLojasDeVolatileCabemNaJanela(t *testing.T) {
	// Espelha clientShopSlots de internal/game/handlers.go. Nao da para
	// importar (game depende de data, nao o contrario), entao o valor e
	// repetido aqui de proposito.
	const slotsDoClient = 27
	npcs, err := LoadNPCs(filepath.Join("..", "..", "data", "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	achou := 0
	for _, npc := range npcs {
		if !strings.HasPrefix(npc.Name, "ShopVol") {
			continue
		}
		achou++
		if len(npc.Vende) > slotsDoClient {
			t.Errorf("%s vende %d itens: o client so mostra %d e o resto "+
				"seria truncado sem chegar ao jogador",
				npc.Name, len(npc.Vende), slotsDoClient)
		}
	}
	if achou == 0 {
		t.Error("nenhuma loja ShopVol* encontrada: a cobertura de volatile do beta sumiu")
	}
}
