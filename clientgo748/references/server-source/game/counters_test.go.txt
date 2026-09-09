package game

import (
	"testing"

	"wydgo/internal/model"
)

// O contador nomeado reusa Player.SpecialCoins, que ja e persistido no
// charstate e ja e zerado na troca de personagem. Estes testes fixam as duas
// propriedades que a reutilizacao precisa manter.

func jogadorComSaldo(saldos map[string]uint32) *Player {
	ch := &model.Char{Name: "A",
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 10, CurHP: 10}}
	return &Player{Char: ch, SpecialCoins: saldos}
}

func TestGastarContadorEhTudoOuNada(t *testing.T) {
	p := jogadorComSaldo(map[string]uint32{"a": 5, "b": 1})

	// "b" nao cobre: nada pode ser debitado, nem o "a" que cobria.
	if spendCounters(p, map[string]uint32{"a": 3, "b": 2}) {
		t.Fatal("gastou com saldo insuficiente")
	}
	if p.SpecialCoins["a"] != 5 || p.SpecialCoins["b"] != 1 {
		t.Errorf("debitou parcialmente: %v", p.SpecialCoins)
	}
}

func TestGastarZeraERemoveAChave(t *testing.T) {
	p := jogadorComSaldo(map[string]uint32{"kefra_ticket": 1})
	if !spendCounters(p, map[string]uint32{"kefra_ticket": 1}) {
		t.Fatal("nao gastou com saldo exato")
	}
	// O charstate omite zeros ao gravar; manter a chave aqui faria a memoria
	// divergir do disco.
	if _, existe := p.SpecialCoins["kefra_ticket"]; existe {
		t.Errorf("saldo zerado continuou no mapa: %v", p.SpecialCoins)
	}
}

func TestCreditarSaturaNoTeto(t *testing.T) {
	p := jogadorComSaldo(map[string]uint32{"x": model.MaxQuestCounter - 1})
	grantCounters(p, map[string]uint32{"x": 100})
	if got := p.SpecialCoins["x"]; got != model.MaxQuestCounter {
		t.Errorf("saldo=%d, quer o teto %d (nao pode dar a volta)", got, model.MaxQuestCounter)
	}
}

func TestCreditarCriaOMapaQuandoNil(t *testing.T) {
	p := jogadorComSaldo(nil)
	grantCounters(p, map[string]uint32{"x": 2})
	if p.SpecialCoins["x"] != 2 {
		t.Fatalf("nao creditou em mapa nil: %v", p.SpecialCoins)
	}
}

// TestSnapshotDeContadorNaoCompartilhaMapa: o rollback depende disso. Guardar
// p.SpecialCoins direto guardaria a MESMA referencia e nao restauraria nada.
func TestSnapshotDeContadorNaoCompartilhaMapa(t *testing.T) {
	p := jogadorComSaldo(map[string]uint32{"x": 5})
	snap := copyCounters(p)
	spendCounters(p, map[string]uint32{"x": 5})
	if snap["x"] != 5 {
		t.Fatalf("o snapshot mudou junto com o original: %v", snap)
	}
}

// --- recarga do NPC (o Sobrevivente) ----------------------------------------

func questDoSobrevivente() *model.QuestDef {
	return &model.QuestDef{
		ID: 18, NPC: "Survivor", Repeatable: true,
		Recharge:        &model.QuestRecharge{Item: 4127, Counter: "kefra_ticket", Amount: 100},
		ConsumeCounters: map[string]uint32{"kefra_ticket": 1},
		Messages:        model.QuestMessages{Offer: "o", Success: "s"},
	}
}

func TestRecargaSoDisparaComSaldoZero(t *testing.T) {
	quest := questDoSobrevivente()

	// Com entradas sobrando, o selo NAO pode ser gasto.
	p := jogadorComSaldo(map[string]uint32{"kefra_ticket": 3})
	p.Char.Inv[0] = model.Item{Index: 4127}
	if applyQuestRecharge(p, quest) {
		t.Error("recarregou com saldo sobrando")
	}
	if p.Char.Inv[0].Index != 4127 {
		t.Error("consumiu o selo de quem ainda tinha entradas")
	}

	// Com saldo zero, troca uma unidade por 100.
	p = jogadorComSaldo(nil)
	p.Char.Inv[0] = model.Item{Index: 4127}
	if !applyQuestRecharge(p, quest) {
		t.Fatal("nao recarregou com saldo zero")
	}
	if p.SpecialCoins["kefra_ticket"] != 100 {
		t.Errorf("saldo=%d, quer 100", p.SpecialCoins["kefra_ticket"])
	}
	if p.Char.Inv[0].Index != 0 {
		t.Error("nao consumiu o selo")
	}
}

func TestRecargaSemSeloNaoCredita(t *testing.T) {
	p := jogadorComSaldo(nil)
	if applyQuestRecharge(p, questDoSobrevivente()) {
		t.Error("recarregou sem ter o selo")
	}
	if len(p.SpecialCoins) != 0 {
		t.Errorf("creditou do nada: %v", p.SpecialCoins)
	}
}

// TestOfertaAceitaQuemVaiRecarregar: sem isso, quem chega com saldo zero e o
// selo na mochila seria recusado ANTES de o NPC ter chance de trocar.
func TestOfertaAceitaQuemVaiRecarregar(t *testing.T) {
	quest := questDoSobrevivente()
	p := jogadorComSaldo(nil)
	p.Char.Inv[0] = model.Item{Index: 4127}

	if !rechargeCovers(p, p.Char, quest, "kefra_ticket") {
		t.Error("a oferta recusou quem tem o selo para recarregar")
	}

	semSelo := jogadorComSaldo(nil)
	if rechargeCovers(semSelo, semSelo.Char, quest, "kefra_ticket") {
		t.Error("a oferta aceitou quem nao tem selo nem saldo")
	}
}
