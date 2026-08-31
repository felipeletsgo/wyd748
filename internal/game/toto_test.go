package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
)

type totoStore struct {
	saves         int
	err           error
	snapshot      *model.Account
	packetsAtSave int
	player        *Player
}

func (s *totoStore) LoadAccount(string) (*model.Account, error) {
	return nil, errors.New("not used")
}

func (s *totoStore) SaveAccount(account *model.Account) error {
	s.saves++
	s.snapshot = account
	if s.player != nil {
		s.packetsAtSave = s.player.Session.QueuedPacketsForTest()
	}
	return s.err
}

func (s *totoStore) CharacterNameExists(string) (bool, error) { return false, nil }

func totoTestWorld(t *testing.T) (*World, *Player, *totoStore, *Mob) {
	t.Helper()
	w, p, _ := handlerTestWorld(t)
	store := &totoStore{player: p}
	w.store = store
	w.items[4147] = model.ItemDef{Index: 4147, Price: 3_000}
	shop := &Mob{
		ID: 1100, X: p.X + 1, Y: p.Y,
		Def: &model.NPCDef{
			Name: "TOTO Merchant", Tipo: model.TipoNPC,
			Score: &model.Score{Merchant: nativeShopMerchant},
			Vende: []model.Item{{Index: 4147}},
		},
	}
	w.registerMobSpatial(shop)
	p.show(shop.ID)
	p.ShopNPC = shop.ID
	p.Char.Gold = 10_000
	return w, p, store, shop
}

func totoBuyPacket(targetID uint16, shopSlot, dst int16, coin uint32, match, scoreA, scoreB int32) []byte {
	pkt := make([]byte, 36)
	binary.LittleEndian.PutUint16(pkt[0:2], uint16(len(pkt)))
	binary.LittleEndian.PutUint16(pkt[4:6], 0x3CE)
	binary.LittleEndian.PutUint16(pkt[12:14], targetID)
	binary.LittleEndian.PutUint16(pkt[14:16], uint16(shopSlot))
	binary.LittleEndian.PutUint16(pkt[16:18], uint16(dst))
	binary.LittleEndian.PutUint32(pkt[20:24], coin)
	binary.LittleEndian.PutUint32(pkt[24:28], uint32(match))
	binary.LittleEndian.PutUint32(pkt[28:32], uint32(scoreA))
	binary.LittleEndian.PutUint32(pkt[32:36], uint32(scoreB))
	return pkt
}

func TestBuyTotoUsesAuthoritativePriceAndPersistsTicketBeforePublishing(t *testing.T) {
	w, p, store, shop := totoTestWorld(t)
	p.Char.Inv[0] = model.Item{Index: 400, UID: "11111111111141118111111111110400"}
	pkt := totoBuyPacket(shop.ID, 0, 5, 1, 80, 12, 127)

	w.onBuyToto(p.Session, pkt)

	ticket := p.Char.Inv[5]
	wantEffects := [6]byte{64, 80, 65, 12, 66, 127}
	if ticket.Index != 4147 || ticket.UID == "" || ticket.Eff != wantEffects {
		t.Fatalf("bilhete TOTO incorreto: %+v", ticket)
	}
	if _, err := model.NormalizeItemUID(ticket.UID); err != nil {
		t.Fatalf("bilhete sem UID persistente valido: %q err=%v", ticket.UID, err)
	}
	if p.Char.Inv[0].Index != 400 || p.Char.Gold != 7_000 {
		t.Fatalf("destino/gold incorretos: inv0=%+v gold=%d", p.Char.Inv[0], p.Char.Gold)
	}
	if store.saves != 1 || store.snapshot == nil || store.packetsAtSave != 0 {
		t.Fatalf("persist-before-publish violado: saves=%d snapshot=%v packetsAtSave=%d",
			store.saves, store.snapshot != nil, store.packetsAtSave)
	}
	persisted := store.snapshot.Chars[p.CharSlot]
	if persisted.Inv[5] != ticket || persisted.Gold != 7_000 {
		t.Fatalf("snapshot TOTO incorreto: ticket=%+v gold=%d", persisted.Inv[5], persisted.Gold)
	}
	if got := p.Session.QueuedPacketsForTest(); got != 2 {
		t.Fatalf("sucesso deve publicar somente UpdateCarry+UpdateEtc: packets=%d", got)
	}
}

func TestBuyTotoRejectsForgedContextAndValues(t *testing.T) {
	tests := []struct {
		name   string
		mutate func(*World, *Player, *Mob, []byte)
	}{
		{"merchant not open", func(_ *World, p *Player, _ *Mob, _ []byte) { p.ShopNPC = 0 }},
		{"wrong merchant", func(_ *World, _ *Player, shop *Mob, pkt []byte) {
			binary.LittleEndian.PutUint16(pkt[12:14], shop.ID+1)
		}},
		{"negative shop slot", func(_ *World, _ *Player, _ *Mob, pkt []byte) {
			binary.LittleEndian.PutUint16(pkt[14:16], uint16(0xFFFF))
		}},
		{"occupied destination", func(_ *World, p *Player, _ *Mob, _ []byte) {
			p.Char.Inv[5] = model.Item{Index: 400}
		}},
		{"hidden destination", func(_ *World, _ *Player, _ *Mob, pkt []byte) {
			binary.LittleEndian.PutUint16(pkt[16:18], model.PlayerCarrySlots)
		}},
		{"match zero", func(_ *World, _ *Player, _ *Mob, pkt []byte) {
			binary.LittleEndian.PutUint32(pkt[24:28], 0)
		}},
		{"match above table", func(_ *World, _ *Player, _ *Mob, pkt []byte) {
			binary.LittleEndian.PutUint32(pkt[24:28], 81)
		}},
		{"negative score", func(_ *World, _ *Player, _ *Mob, pkt []byte) {
			binary.LittleEndian.PutUint32(pkt[28:32], uint32(0xFFFFFFFF))
		}},
		{"score above wire contract", func(_ *World, _ *Player, _ *Mob, pkt []byte) {
			binary.LittleEndian.PutUint32(pkt[32:36], 128)
		}},
		{"insufficient gold", func(_ *World, p *Player, _ *Mob, _ []byte) { p.Char.Gold = 2_999 }},
		{"visual item is not ticket", func(_ *World, _ *Player, shop *Mob, _ []byte) {
			shop.Def.Vende[0] = model.Item{Index: 400}
		}},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			w, p, store, shop := totoTestWorld(t)
			pkt := totoBuyPacket(shop.ID, 0, 5, 0xFFFFFFFF, 1, 0, 0)
			tt.mutate(w, p, shop, pkt)
			beforeGold, beforeInv := p.Char.Gold, p.Char.Inv

			w.onBuyToto(p.Session, pkt)

			if p.Char.Gold != beforeGold || p.Char.Inv != beforeInv || store.saves != 0 {
				t.Fatalf("rejeicao alterou estado: gold=%d/%d saves=%d inv5=%+v",
					p.Char.Gold, beforeGold, store.saves, p.Char.Inv[5])
			}
			if got := p.Session.QueuedPacketsForTest(); got != 1 {
				t.Fatalf("rejeicao deve encerrar com um MessagePanel: packets=%d", got)
			}
		})
	}
}

func TestBuyTotoRollsBackGoldAndTicketWhenSaveFails(t *testing.T) {
	w, p, store, shop := totoTestWorld(t)
	store.err = errors.New("disk unavailable")
	p.Char.Inv[2] = model.Item{Index: 400, UID: "11111111111141118111111111110400"}
	beforeGold, beforeInv := p.Char.Gold, p.Char.Inv

	w.onBuyToto(p.Session, totoBuyPacket(shop.ID, 0, 5, 123_456_789, 7, 2, 3))

	if p.Char.Gold != beforeGold || p.Char.Inv != beforeInv {
		t.Fatalf("falha de save nao fez rollback integral: gold=%d/%d inv5=%+v",
			p.Char.Gold, beforeGold, p.Char.Inv[5])
	}
	if store.saves != 1 || store.packetsAtSave != 0 {
		t.Fatalf("tentativa de persistencia incorreta: saves=%d packetsAtSave=%d",
			store.saves, store.packetsAtSave)
	}
	if got := p.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("falha de save deve enviar somente MessagePanel: packets=%d", got)
	}
}
