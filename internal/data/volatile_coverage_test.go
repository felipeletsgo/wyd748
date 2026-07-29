package data

import (
	"fmt"
	"path/filepath"
	"sort"
	"strings"
	"testing"
)

// TestCadaCodigoVolatileRealTemContratoExplicito cria um subteste independente
// para CADA EF_VOLATILE encontrado no itemlist autoritativo. Isso torna a
// cobertura visivel por codigo no relatorio e impede tres regressões silenciosas:
// item sem regra, contagem divergente e familia implementada voltando ao generic.
func TestCadaCodigoVolatileRealTemContratoExplicito(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := LoadVolatiles(filepath.Join(root, "volatiles.json"),
		catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	itemsByCode := make(map[int][]uint16, len(volatiles.Codes))
	for itemID, code := range volatiles.ItemCodes {
		itemsByCode[code] = append(itemsByCode[code], itemID)
	}
	codes := make([]int, 0, len(volatiles.Codes))
	for code := range volatiles.Codes {
		codes = append(codes, code)
	}
	sort.Ints(codes)
	for _, code := range codes {
		code := code
		t.Run(fmt.Sprintf("volatile_%d", code), func(t *testing.T) {
			items := itemsByCode[code]
			sort.Slice(items, func(i, j int) bool { return items[i] < items[j] })
			if len(items) == 0 || len(items) != volatiles.Codes[code] {
				t.Fatalf("catalogo informa %d item(ns), resolvidos=%v",
					volatiles.Codes[code], items)
			}
			for _, itemID := range items {
				rule, resolvedCode, ok := volatiles.Rule(itemID)
				if !ok || resolvedCode != code || strings.TrimSpace(rule.Action) == "" {
					t.Fatalf("item %d nao resolveu contrato: code=%d ok=%v action=%q",
						itemID, resolvedCode, ok, rule.Action)
				}
				if rule.Action == "generic" {
					t.Fatalf("item %d ainda usa generic", itemID)
				}
			}
		})
	}
}

func TestItensLoveResolvemSkillDataParaAffectsReais(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := LoadVolatiles(filepath.Join(root, "volatiles.json"),
		catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	expected := []struct {
		skillID, affectType, affectValue int
	}{
		{43, 11, 15},
		{44, 9, 90},
		{45, 15, 7},
		{41, 2, 1},
	}
	for _, itemID := range []uint16{1739, 4145} {
		rule, _, ok := volatiles.Rule(itemID)
		if !ok || len(rule.Affects) != len(expected) {
			t.Fatalf("item Love %d: regra=%+v ok=%v", itemID, rule, ok)
		}
		for index, want := range expected {
			configured := rule.Affects[index]
			skill := catalog.Skills[configured.SkillID]
			if configured.SkillID != want.skillID ||
				skill.AffectType != want.affectType ||
				skill.AffectValue != want.affectValue {
				t.Errorf("item %d affect[%d]: skill=%d => type/value=%d/%d, esperado %d=>%d/%d",
					itemID, index, configured.SkillID, skill.AffectType,
					skill.AffectValue, want.skillID, want.affectType, want.affectValue)
			}
		}
	}
}

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
	volateis, err := LoadVolatiles(filepath.Join(raiz, "volatiles.json"),
		catalogo.Items, catalogo.Skills)
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

// Uma sala configurada com nome digitado errado so falharia quando o jogador
// consumisse o ticket. O boot tambem valida essa referencia, mas este teste
// aponta o item exato sem precisar iniciar banco, mundo e rede.
func TestInstanciasDeItemReferenciamNPCExistente(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := LoadVolatiles(filepath.Join(root, "volatiles.json"),
		catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	names := make(map[string]struct{}, len(npcs))
	for _, npc := range npcs {
		names[npc.Name] = struct{}{}
	}
	for itemID, rule := range volatiles.Items {
		if rule.Action != "instance_ticket" || rule.Instance == nil {
			continue
		}
		for _, spawn := range rule.Instance.Spawns {
			if _, ok := names[spawn.NPC]; !ok {
				t.Errorf("item %d instancia %q referencia NPC ausente %q",
					itemID, rule.Instance.ID, spawn.NPC)
			}
		}
		for stageIndex, stage := range rule.Instance.Stages {
			for _, spawn := range stage.Spawns {
				if _, ok := names[spawn.NPC]; !ok {
					t.Errorf("item %d instancia %q sala %d referencia NPC ausente %q",
						itemID, rule.Instance.ID, stageIndex+1, spawn.NPC)
				}
			}
		}
	}
}

func TestCatalogoRealNaoTemVolatileGenerico(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := LoadVolatiles(filepath.Join(root, "volatiles.json"),
		catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	var generic, celestial []uint16
	for itemID := range volatiles.ItemCodes {
		rule, _, ok := volatiles.Rule(itemID)
		if !ok {
			t.Fatalf("item volatile %d nao resolveu regra", itemID)
		}
		switch rule.Action {
		case "generic":
			generic = append(generic, itemID)
		case "celestial_pending":
			celestial = append(celestial, itemID)
		}
	}
	sort.Slice(generic, func(i, j int) bool { return generic[i] < generic[j] })
	sort.Slice(celestial, func(i, j int) bool { return celestial[i] < celestial[j] })
	if len(generic) != 0 {
		t.Fatalf("itens ainda no generic: %v", generic)
	}
	wantCelestial := []uint16{3443, 3455, 5338}
	if len(celestial) != len(wantCelestial) {
		t.Fatalf("celestial_pending=%v, quer %v", celestial, wantCelestial)
	}
	for i := range celestial {
		if celestial[i] != wantCelestial[i] {
			t.Fatalf("celestial_pending=%v, quer %v", celestial, wantCelestial)
		}
	}
}

func TestLojasDaRodadaVendemTodasAsVariantesNaoCelestial(t *testing.T) {
	npcs, err := LoadNPCs(filepath.Join("..", "..", "data", "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	got := make(map[uint16]struct{})
	for _, npc := range npcs {
		if npc.Name != "ShopUse20A" && npc.Name != "ShopUse20B" {
			continue
		}
		for _, item := range npc.Vende {
			got[item.Index] = struct{}{}
		}
	}
	want := []uint16{
		417, 3330, 3336, 3337, 3338, 3339, 3453, 3454, 4003, 4004,
		4005, 4007, 1729, 1730, 4105, 1731, 3171, 3172, 1737, 1772,
		4000, 4001, 3324, 3390, 3325, 3391, 3326, 3392, 3328, 3329,
		3909, 3910, 3974, 3331, 4012, 3393, 3394, 3395, 3396, 3441,
		4114, 4115, 4116, 3439, 3440, 4147, 3444,
	}
	for _, itemID := range want {
		if _, ok := got[itemID]; !ok {
			t.Errorf("item nao-Celestial %d ausente das lojas da rodada", itemID)
		}
	}
	for _, itemID := range []uint16{3443, 3455, 5338} {
		if _, ok := got[itemID]; ok {
			t.Errorf("item Celestial %d entrou nas lojas da rodada", itemID)
		}
	}
	if len(got) != len(want) {
		t.Fatalf("lojas possuem %d itens, esperados %d", len(got), len(want))
	}
}
