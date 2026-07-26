package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// kefraTicketCounter e o saldo de entradas de Kefra -- o extra.KefraTicket do
// nativo. O Sobrevivente cobra uma por visita; o item 4127 recarrega 100.
const kefraTicketCounter = "kefra_ticket"

// fameCounter e a fama do PERSONAGEM. Vive no charstate como os demais
// contadores; no nativo e extra.Fame, e a fonte e o Warrior's_Seal (4146).
//
// ATENCAO A ORDEM: quem gasta fama junto com itens da conta (crafts do Odin e
// da Lindy) precisa gravar o charstate ANTES do saveAccount, como faz o
// executeQuest. Os dois arquivos nao compartilham transacao.
const fameCounter = "fame"

// Contador nomeado por personagem -- o `KefraTicket` do nativo e o caso de uso
// que motivou isto: um saldo de entradas que o Sobrevivente cobra por visita.
//
// NAO ha campo novo: `Player.SpecialCoins` (persistido em
// data/charstate/<nome>.json) ja e um `map[string]uint32` por personagem, ja e
// zerado por resetCharacterRuntime e ja e SUBSTITUIDO, nunca mesclado, por
// applyCharState. Criar um campo paralelo traria duas armadilhas:
//
//   - CharStateVersion e validado por igualdade exata (store/charstate.go),
//     entao subir a versao invalidaria todo sidecar existente;
//   - o store APAGA o arquivo quando Affects e SpecialCoins estao ambos vazios,
//     entao um campo novo fora dessa condicao sumiria no primeiro autosave.

// counterBalance devolve o saldo do contador. Mapa nil e saldo zero.
func counterBalance(p *Player, name string) uint32 {
	if p == nil || p.SpecialCoins == nil {
		return 0
	}
	return p.SpecialCoins[name]
}

// hasCounters diz se o personagem tem TODOS os saldos pedidos. Devolve tambem o
// nome do primeiro contador em falta, para a mensagem de recusa ser especifica.
func hasCounters(p *Player, required map[string]uint32) (string, bool) {
	for name, amount := range required {
		if counterBalance(p, name) < amount {
			return name, false
		}
	}
	return "", true
}

// spendCounters debita os saldos. Falha sem debitar NADA se algum nao cobrir --
// debitar em ordem e desistir no meio deixaria o personagem pagando parte do
// preco sem receber a recompensa.
func spendCounters(p *Player, cost map[string]uint32) bool {
	if len(cost) == 0 {
		return true
	}
	if _, ok := hasCounters(p, cost); !ok {
		return false
	}
	for name, amount := range cost {
		p.SpecialCoins[name] -= amount
		// Saldo zerado sai do mapa: o charstate omite zeros ao gravar, e manter
		// a chave aqui faria o mapa em memoria divergir do que foi persistido.
		if p.SpecialCoins[name] == 0 {
			delete(p.SpecialCoins, name)
		}
	}
	return true
}

// grantCounters credita saldos, saturando no teto em vez de dar a volta.
func grantCounters(p *Player, reward map[string]uint32) {
	if p == nil || len(reward) == 0 {
		return
	}
	if p.SpecialCoins == nil {
		p.SpecialCoins = make(map[string]uint32, len(reward))
	}
	for name, amount := range reward {
		saldo := p.SpecialCoins[name]
		if saldo > model.MaxQuestCounter-amount {
			saldo = model.MaxQuestCounter
		} else {
			saldo += amount
		}
		p.SpecialCoins[name] = saldo
	}
}

// copyCounters tira um snapshot para o rollback. O mapa e por referencia, entao
// guardar `p.SpecialCoins` direto nao restauraria nada.
func copyCounters(p *Player) map[string]uint32 {
	if p == nil || p.SpecialCoins == nil {
		return nil
	}
	c := make(map[string]uint32, len(p.SpecialCoins))
	for k, v := range p.SpecialCoins {
		c[k] = v
	}
	return c
}

// useCounterGrant e a acao volatile "grant_counter": o item vira saldo. No
// nativo e o 4127, que soma 100 entradas de Kefra.
//
// Mesmo persist-before-confirm do resto: o item so some depois que o sidecar
// esta em disco, senao uma falha de gravacao consumiria o item sem creditar
// nada.
func (w *World) useCounterGrant(s *net.Session, p *Player, item *model.Item, slot byte, rule model.VolatileRule, code int) {
	if len(rule.Counters) == 0 {
		log.Printf("[#%d] grant_counter sem contadores no item %d", s.ID, item.Index)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	anterior := *item
	saldoAnterior := copyCounters(p)

	grantCounters(p, rule.Counters)
	if rule.Consume {
		consumeOne(item)
	}
	if err := w.saveCharStateResult(p); err != nil {
		*item = anterior
		p.SpecialCoins = saldoAnterior
		log.Printf("[#%d] ERRO ao salvar contador do item %d: %v", s.ID, anterior.Index, err)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	for nome, quantidade := range rule.Counters {
		log.Printf("[#%d] contador %q +%d = %d (item %d volatile %d)",
			s.ID, nome, quantidade, counterBalance(p, nome), anterior.Index, code)
	}
}

// rechargeCovers diz se a recarga do NPC consegue suprir o contador em falta.
// Usada na OFERTA: sem isso, quem chega com saldo zero e o selo na mochila
// seria recusado antes de o NPC ter chance de trocar.
func rechargeCovers(p *Player, ch *model.Char, quest *model.QuestDef, faltando string) bool {
	r := quest.Recharge
	if r == nil || r.Counter != faltando {
		return false
	}
	if countInventoryItem(ch, r.Item) < 1 {
		return false
	}
	return counterBalance(p, r.Counter)+r.Amount >= quest.ConsumeCounters[r.Counter]
}

// applyQuestRecharge troca uma unidade do item por saldo, SOMENTE com o
// contador zerado -- e a condicao do nativo (`if KefraTicket < 1`), e ela evita
// que quem ainda tem entradas perca o selo sem precisar.
func applyQuestRecharge(p *Player, quest *model.QuestDef) bool {
	r := quest.Recharge
	if r == nil || p == nil || p.Char == nil {
		return false
	}
	if counterBalance(p, r.Counter) > 0 {
		return false
	}
	if !consumeInventoryItem(p.Char, r.Item, 1) {
		return false
	}
	grantCounters(p, map[string]uint32{r.Counter: r.Amount})
	return true
}
