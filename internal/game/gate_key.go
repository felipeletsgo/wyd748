package game

import (
	"encoding/binary"
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
	w.openGateWithKey(s, p, porta, slot, rule.Consume, code)
}

// openGateWithKey e a transacao comum dos dois fluxos nativos: usar a chave
// pelo 0x373 ou clicar diretamente no objeto pelo 0x374. A chave e localizada
// e consumida no inventario autoritativo; a porta so muda depois do save.
func (w *World) openGateWithKey(s *net.Session, p *Player, porta *GroundItem, slot byte, consume bool, code int) bool {
	if s == nil || p == nil || p.Char == nil || p.Account == nil || porta == nil ||
		int(slot) >= model.PlayerCarrySlots || porta.State == gateOpen {
		return false
	}
	item := &p.Char.Inv[slot]
	keyDef, keyOK := w.items[item.Index]
	gateDef, gateOK := w.items[porta.Item.Index]
	keyID := staticEffect(keyDef, "EF_KEYID")
	if !keyOK || !gateOK || keyID == 0 || staticEffect(gateDef, "EF_KEYID") != keyID {
		return false
	}

	anterior := *item
	if consume {
		consumeOne(item)
	}
	if err := w.saveAccount(p.Account); err != nil {
		*item = anterior
		log.Printf("[#%d] ERRO ao salvar uso da chave %d: %v", s.ID, anterior.Index, err)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return false
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
	return true
}

func (w *World) gateKeySlot(p *Player, keyID int) int {
	if p == nil || p.Char == nil || keyID == 0 {
		return -1
	}
	for slot := 0; slot < model.PlayerCarrySlots; slot++ {
		item := p.Char.Inv[slot]
		if def, ok := w.items[item.Index]; ok && staticEffect(def, "EF_KEYID") == keyID {
			return slot
		}
	}
	return -1
}

// onUpdateGroundItem trata o MSG_UpdateItem 0x374 (ItemID@12, State@16).
// O client apenas pede para abrir: ID, proximidade, visibilidade, estado,
// EF_KEYID e posse da chave sao todos revalidados no mundo.
func (w *World) onUpdateGroundItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) != 20 || playerCurHP(p.Char) == 0 {
		return
	}
	itemIDRaw := binary.LittleEndian.Uint32(pkt[12:16])
	state := binary.LittleEndian.Uint32(pkt[16:20])
	// A source aceita State 0..5 e, depois da validacao, sempre transiciona o
	// objeto para OPEN. O campo e uma intencao/estado visual do client, nunca o
	// novo estado autoritativo da porta.
	if itemIDRaw == 0 || itemIDRaw > uint32(^uint16(0)) || state > 5 {
		w.recordSecurityViolation(s, wire.OpUpdateItem, "id/estado de objeto invalido")
		return
	}
	porta := w.groundItems[uint16(itemIDRaw)]
	if porta == nil || !porta.Permanent || !p.hasVisible(porta.ID) ||
		chebyshev(p.X, p.Y, porta.X, porta.Y) > gateReach {
		w.recordSecurityViolation(s, wire.OpUpdateItem, "objeto inexistente ou fora de alcance")
		return
	}
	if porta.State == gateOpen {
		s.Send(wire.UpdateItem(porta.ID, uint32(gateOpen)))
		return
	}
	gateDef, ok := w.items[porta.Item.Index]
	keyID := staticEffect(gateDef, "EF_KEYID")
	if !ok || keyID == 0 {
		w.recordSecurityViolation(s, wire.OpUpdateItem, "objeto nao e um portao com chave")
		return
	}
	slot := w.gateKeySlot(p, keyID)
	if slot < 0 {
		s.Send(wire.MessagePanel("You do not have the required key."))
		s.Send(wire.UpdateItem(porta.ID, uint32(porta.State)))
		return
	}
	w.openGateWithKey(s, p, porta, byte(slot), true, 3)
}
