package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// A chave e a porta se acham pelo EF_KEYID, nunca pelo indice: no itemlist a
// First_Gate_Key (451) e a porta (458) trazem o mesmo EF_KEYID 2.

func mundoComPorta(keyIDPorta int, portaX, portaY uint16) (*World, *GroundItem) {
	w := &World{
		groundItems: map[uint16]*GroundItem{},
		items: map[uint16]model.ItemDef{
			451: {Index: 451, Name: "First_Gate_Key",
				StaticEffects: []model.StaticEffect{{Name: "EF_KEYID", Value: 2}}},
			452: {Index: 452, Name: "Second_Gate_Key",
				StaticEffects: []model.StaticEffect{{Name: "EF_KEYID", Value: 3}}},
			458: {Index: 458, Name: "First_Gate",
				StaticEffects: []model.StaticEffect{{Name: "EF_KEYID", Value: keyIDPorta}}},
		},
		volatiles: model.VolatileCatalog{},
	}
	porta := &GroundItem{ID: 10000, Item: model.Item{Index: 458},
		X: portaX, Y: portaY, Permanent: true, State: gateClosed}
	w.groundItems[porta.ID] = porta
	return w, porta
}

func jogadorComChave(w *World, indiceChave uint16, x, y uint16) (*Player, *net.Session) {
	session := net.NewTestSession(1, 64)
	acc := &model.Account{Name: "conta", Chars: []model.Char{{Name: "A",
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 100, CurHP: 100}}}}
	p := &Player{ID: 1, Session: session, Account: acc, Char: &acc.Chars[0],
		InWorld: true, X: x, Y: y, Visible: map[uint16]struct{}{}}
	p.Char.Inv[0] = model.Item{Index: indiceChave}
	w.players = map[*net.Session]*Player{session: p}
	w.store = &craftStore{}
	return p, session
}

func TestChaveAbrePortaEDesaparece(t *testing.T) {
	w, porta := mundoComPorta(2, 2100, 2100)
	p, s := jogadorComChave(w, 451, 2100, 2100)

	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)

	if porta.State != gateOpen {
		t.Error("a porta continuou fechada")
	}
	if p.Char.Inv[0].Index != 0 {
		t.Errorf("a chave sobrou no inventario: %d", p.Char.Inv[0].Index)
	}
}

// TestChaveErradaNaoAbre: e o EF_KEYID que decide. Uma chave de outro id nao
// pode abrir a porta nem ser gasta.
func TestChaveErradaNaoAbre(t *testing.T) {
	w, porta := mundoComPorta(2, 2100, 2100)
	p, s := jogadorComChave(w, 452, 2100, 2100) // EF_KEYID 3, porta e 2

	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)

	if porta.State != gateClosed {
		t.Error("a porta abriu com a chave errada")
	}
	if p.Char.Inv[0].Index != 452 {
		t.Error("a chave errada foi gasta")
	}
}

// TestChaveLongeDaPortaNaoAbre: usar a chave do outro lado do mapa nao pode
// abrir porta nenhuma -- nem gastar a chave.
func TestChaveLongeDaPortaNaoAbre(t *testing.T) {
	w, porta := mundoComPorta(2, 2100, 2100)
	p, s := jogadorComChave(w, 451, 2100+uint16(gateReach)+5, 2100)

	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)

	if porta.State != gateClosed {
		t.Error("a porta abriu de longe")
	}
	if p.Char.Inv[0].Index != 451 {
		t.Error("a chave foi gasta sem abrir nada")
	}
}

// TestChaveNaoSomeSeOSaveFalhar: persist-before-confirm. Se o disco falhar, o
// jogador nao pode ficar sem a chave E com a porta fechada.
func TestChaveNaoSomeSeOSaveFalhar(t *testing.T) {
	w, porta := mundoComPorta(2, 2100, 2100)
	p, s := jogadorComChave(w, 451, 2100, 2100)
	w.store = &craftStore{err: errors.New("disco cheio")}

	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)

	if p.Char.Inv[0].Index != 451 {
		t.Error("a chave sumiu apesar da falha de persistencia")
	}
	if porta.State != gateClosed {
		t.Error("a porta abriu apesar da falha de persistencia")
	}
}

// TestPortaJaAbertaNaoGastaChave.
func TestPortaJaAbertaNaoGastaChave(t *testing.T) {
	w, porta := mundoComPorta(2, 2100, 2100)
	porta.State = gateOpen
	p, s := jogadorComChave(w, 451, 2100, 2100)

	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)

	if p.Char.Inv[0].Index != 451 {
		t.Error("gastou a chave numa porta ja aberta")
	}
}

// TestPortaAbertaChegaAbertaParaQuemChegaDepois: o estado tem que viajar no
// proprio CreateItem, senao quem entrar na area depois veria a porta fechada e
// gastaria outra chave a toa. Confere no byte do pacote: State fica em @27.
func TestPortaAbertaChegaAbertaParaQuemChegaDepois(t *testing.T) {
	w, porta := mundoComPorta(2, 2100, 2100)
	p, s := jogadorComChave(w, 451, 2100, 2100)
	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)

	if porta.State != gateOpen {
		t.Fatal("a porta nao abriu")
	}
	pkt := wire.CreateItem(porta.X, porta.Y, porta.ID, porta.Item, porta.Rotate,
		porta.State, 0, 0, 0)
	if len(pkt) != 32 {
		t.Fatalf("CreateItem com %d bytes, quer 32", len(pkt))
	}
	if pkt[27] != gateOpen {
		t.Errorf("a porta viajou com State=%d, quer %d (aberta)", pkt[27], gateOpen)
	}
}

func updateGroundItemPacket(id uint16, state uint32) []byte {
	pkt := make([]byte, 20)
	binary.LittleEndian.PutUint32(pkt[12:16], uint32(id))
	binary.LittleEndian.PutUint32(pkt[16:20], state)
	return pkt
}

func TestUpdateGroundItemUsesMatchingInventoryKey(t *testing.T) {
	w, porta := mundoComPorta(2, 2100, 2100)
	p, s := jogadorComChave(w, 451, 2100, 2100)
	p.show(porta.ID)

	w.onUpdateGroundItem(s, updateGroundItemPacket(porta.ID, uint32(gateOpen)))

	if porta.State != gateOpen || p.Char.Inv[0].Index != 0 {
		t.Fatalf("0x374 nao abriu/consumiu: state=%d key=%+v", porta.State, p.Char.Inv[0])
	}
}

func TestUpdateGroundItemRejectsForgedStateAndMissingKey(t *testing.T) {
	w, porta := mundoComPorta(2, 2100, 2100)
	p, s := jogadorComChave(w, 452, 2100, 2100)
	p.show(porta.ID)

	w.onUpdateGroundItem(s, updateGroundItemPacket(porta.ID, 6))
	if porta.State != gateClosed || p.Char.Inv[0].Index != 452 {
		t.Fatal("estado forjado alterou a porta")
	}

	w.onUpdateGroundItem(s, updateGroundItemPacket(porta.ID, uint32(gateOpen)))
	if porta.State != gateClosed || p.Char.Inv[0].Index != 452 {
		t.Fatal("chave com EF_KEYID diferente abriu a porta")
	}
}

func TestUpdateGroundItemRollsBackKeyWhenSaveFails(t *testing.T) {
	w, porta := mundoComPorta(2, 2100, 2100)
	p, s := jogadorComChave(w, 451, 2100, 2100)
	p.show(porta.ID)
	w.store = &craftStore{err: errors.New("postgres indisponivel")}

	w.onUpdateGroundItem(s, updateGroundItemPacket(porta.ID, uint32(gateOpen)))

	if porta.State != gateClosed || p.Char.Inv[0].Index != 451 {
		t.Fatalf("falha de save dividiu chave/porta: state=%d key=%+v",
			porta.State, p.Char.Inv[0])
	}
}
