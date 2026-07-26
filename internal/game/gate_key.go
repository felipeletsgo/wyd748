package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// A chave e a porta se reconhecem pelo EF_KEYID do itemlist, nao por indice:
// First_Gate_Key (451) tem EF_VOLATILE 3 + EF_KEYID 2, e a porta no chao
// (458) tem EF_GROUND 1 + o MESMO EF_KEYID 2. Sao 38 itens com EF_KEYID no
// catalogo 7.48, entre chaves e portas.
const (
	gateClosed byte = 0
	gateOpen   byte = 1
	// gateReach e o alcance para usar a chave, em Chebyshev. Mesmo raio do
	// recolhimento de item: a porta precisa estar ao alcance da mao.
	gateReach = pickupRange
)

// staticEffect le um efeito do itemlist SEM aplicar refinacao. itemAbility nao
// serve aqui: EF_KEYID nao esta em nonRefinableEffects, entao um item refinado
// teria o id da chave multiplicado.
func staticEffect(def model.ItemDef, name string) int {
	for _, e := range def.StaticEffects {
		if e.Name == name {
			return e.Value
		}
	}
	return 0
}

// gateByKeyID acha a porta FECHADA ao alcance do jogador que casa com o id da
// chave. Devolve nil quando nao ha porta perto, quando ela ja esta aberta ou
// quando o id nao confere -- cada caso tem mensagem propria no chamador.
func (w *World) gateByKeyID(p *Player, keyID int) (*GroundItem, bool) {
	var jaAberta bool
	for _, g := range w.groundItems {
		if !g.Permanent || chebyshev(p.X, p.Y, g.X, g.Y) > gateReach {
			continue
		}
		def, ok := w.items[g.Item.Index]
		if !ok || staticEffect(def, "EF_KEYID") != keyID {
			continue
		}
		if g.State == gateOpen {
			jaAberta = true
			continue
		}
		return g, false
	}
	return nil, jaAberta
}

// useGateKey abre a porta correspondente e GASTA a chave. A ordem importa: a
// chave so some depois que a conta e persistida, senao uma falha de disco
// deixaria o jogador sem a chave e com a porta fechada.
func (w *World) useGateKey(s *net.Session, p *Player, item *model.Item, slot byte, rule model.VolatileRule, code int) {
	def, ok := w.items[item.Index]
	if !ok {
		return
	}
	keyID := staticEffect(def, "EF_KEYID")
	if keyID == 0 {
		log.Printf("[#%d] chave %d sem EF_KEYID no catalogo", s.ID, item.Index)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	porta, jaAberta := w.gateByKeyID(p, keyID)
	if porta == nil {
		if jaAberta {
			s.Send(wire.MessagePanel("This door is already open."))
		} else {
			s.Send(wire.MessagePanel("There is no door for this key here."))
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	anterior := *item
	if rule.Consume {
		consumeOne(item)
	}
	if err := w.saveAccount(p.Account); err != nil {
		*item = anterior
		log.Printf("[#%d] ERRO ao salvar uso da chave %d: %v", s.ID, anterior.Index, err)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	porta.State = gateOpen
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	// A porta e do mundo, nao do jogador: todo mundo que a enxerga precisa
	// ve-la abrir. Quem chegar depois recebe o estado no proprio CreateItem.
	for _, outro := range w.nearbyWorldPlayers(porta.X, porta.Y, viewHalfX) {
		if outro.hasVisible(porta.ID) {
			outro.Session.Send(wire.UpdateItem(porta.ID, uint32(gateOpen)))
		}
	}
	log.Printf("[#%d] abriu a porta %d (item %d, keyid %d) com a chave %d volatile=%d",
		s.ID, porta.ID, porta.Item.Index, keyID, anterior.Index, code)
}
