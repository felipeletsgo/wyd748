package game

import (
	"path/filepath"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

// A fama e o unico contador que tem DOIS consumidores caros (Lindy, 1, para o
// nivel 370 do Arch; Odin, 200, para o nivel 40 do Celestial) e uma unica
// fonte. Ate esta rodada ela era gasta em ambos e concedida em lugar nenhum --
// um preco impagavel. Estes testes fixam a ponta que faltava.

// TestSeloDoGuerreiroConcedeFama percorre a cadeia real de dados: o item
// carrega EF_VOLATILE, o codigo cai numa regra grant_counter e a regra credita
// fama. Um teste so no handler passaria com o itemlist desligado.
func TestSeloDoGuerreiroConcedeFama(t *testing.T) {
	const warriorsSeal = 4146
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := data.LoadVolatilesWithInstances(filepath.Join(root, "volatiles.json"),
		filepath.Join(root, "instances.json"), catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	code, temVolatile := volatiles.ItemCodes[warriorsSeal]
	if !temVolatile {
		t.Fatalf("item %d (Warrior's_Seal) esta sem EF_VOLATILE no itemlist -- sem ele "+
			"o consumivel nunca chega ao dispatcher e a fama fica sem fonte", warriorsSeal)
	}
	rule, temRegra := volatiles.Rules[code]
	if !temRegra {
		t.Fatalf("volatile %d do item %d nao tem regra em volatiles.json", code, warriorsSeal)
	}
	if rule.Action != "grant_counter" {
		t.Fatalf("volatile %d: action=%q, quer grant_counter", code, rule.Action)
	}
	if !rule.Consume {
		t.Errorf("volatile %d nao consome o selo: a fama seria infinita", code)
	}
	// 10 por selo e o valor do nativo (_MSG_UseItem.cpp:7147, extra.Fame += 10).
	if got := rule.Counters[fameCounter]; got != 10 {
		t.Errorf("volatile %d concede %d de %q, quer 10", code, got, fameCounter)
	}
	if len(rule.Counters) != 1 {
		t.Errorf("volatile %d concede contadores demais: %v", code, rule.Counters)
	}
}

// TestNenhumContadorEhGastoSemFonte e a guarda geral: qualquer contador que o
// codigo debite precisa ter pelo menos uma regra que o credite. Foi assim que a
// fama passou despercebida.
func TestNenhumContadorEhGastoSemFonte(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := data.LoadVolatilesWithInstances(filepath.Join(root, "volatiles.json"),
		filepath.Join(root, "instances.json"), catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	quests, err := data.LoadQuests(filepath.Join(root, "quests.json"))
	if err != nil {
		t.Fatal(err)
	}
	// Uma regra volatile so e fonte de verdade se ALGUM item do catalogo cair
	// nela. Uma regra grant_counter sem item que a alcance e letra morta -- e foi
	// exatamente esse o estado da fama ate esta rodada.
	fontes := map[string]bool{}
	for id, code := range volatiles.ItemCodes {
		regra, ok := volatiles.Items[id]
		if !ok {
			regra, ok = volatiles.Rules[code]
		}
		if !ok {
			continue
		}
		for name := range regra.Counters {
			fontes[name] = true
		}
	}
	for _, quest := range quests.Quests {
		for name := range quest.Rewards.Counters {
			fontes[name] = true
		}
		// A recarga tambem e fonte: o Sobrevivente troca o selo 4127 por 100
		// entradas de Kefra sem que o item seja consumivel.
		if quest.Recharge != nil && quest.Recharge.Amount > 0 {
			fontes[quest.Recharge.Counter] = true
		}
	}
	// Os contadores que o codigo Go debita hoje. Adicionar um debito novo sem
	// adicionar aqui nao quebra o teste, mas adicionar aqui sem uma fonte sim.
	for _, name := range []string{fameCounter, kefraTicketCounter} {
		if !fontes[name] {
			t.Errorf("o contador %q e gasto pelo servidor mas nenhuma regra volatile "+
				"ou recompensa de quest o concede", name)
		}
	}
}

// TestFamePersisteNoCharstateENaoNaConta: a fama e por PERSONAGEM. Se ela
// voltasse para model.Char, dois personagens da mesma conta dividiriam o saldo.
func TestFamePersisteNoCharstateENaoNaConta(t *testing.T) {
	p := jogadorComSaldo(nil)
	grantCounters(p, map[string]uint32{fameCounter: 10})

	estado := model.CharState{Version: model.CharStateVersion, SpecialCoins: p.SpecialCoins}
	if estado.SpecialCoins[fameCounter] != 10 {
		t.Fatalf("a fama nao entrou no charstate: %v", estado.SpecialCoins)
	}
	// applyCharState SUBSTITUI o mapa, nunca mescla: trocar de personagem tem
	// de trazer o saldo do novo, nao somar ao do anterior.
	outro := jogadorComSaldo(map[string]uint32{fameCounter: 999})
	outro.SpecialCoins = copyCounters(p)
	if outro.SpecialCoins[fameCounter] != 10 {
		t.Errorf("saldo do personagem anterior vazou: %v", outro.SpecialCoins)
	}
	grantCounters(outro, map[string]uint32{fameCounter: 5})
	if p.SpecialCoins[fameCounter] != 10 {
		t.Errorf("os dois personagens compartilham o MESMO mapa: %v", p.SpecialCoins)
	}
}
