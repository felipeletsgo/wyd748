package game

import (
	"encoding/binary"
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
)

type capsuleNameErrorStore struct{ err error }

func (s capsuleNameErrorStore) LoadAccount(string) (*model.Account, error) { return nil, s.err }
func (s capsuleNameErrorStore) SaveAccount(*model.Account) error           { return s.err }
func (s capsuleNameErrorStore) CharacterNameExists(string) (bool, error)   { return false, s.err }

func TestCelestialExtractionRejectsInvalidStateAndRequests(t *testing.T) {
	tests := []struct {
		name  string
		setup func(*World, *Player, model.Item, []byte) []byte
	}{
		{name: "packet length", setup: func(_ *World, _ *Player, _ model.Item, pkt []byte) []byte { return pkt[:len(pkt)-1] }},
		{name: "grid x outside client range", setup: func(_ *World, _ *Player, _ model.Item, pkt []byte) []byte {
			binary.LittleEndian.PutUint16(pkt[28:30], capsuleGridLimit)
			return pkt
		}},
		{name: "grid y outside client range", setup: func(_ *World, _ *Player, _ model.Item, pkt []byte) []byte {
			binary.LittleEndian.PutUint16(pkt[30:32], capsuleGridLimit)
			return pkt
		}},
		{name: "unsupported source type", setup: func(_ *World, _ *Player, _ model.Item, pkt []byte) []byte {
			binary.LittleEndian.PutUint32(pkt[12:16], 99)
			return pkt
		}},
		{name: "inventory position outside range", setup: func(_ *World, _ *Player, _ model.Item, pkt []byte) []byte {
			binary.LittleEndian.PutUint32(pkt[16:20], model.PlayerCarrySlots)
			return pkt
		}},
		{name: "storage position outside range", setup: func(_ *World, _ *Player, _ model.Item, pkt []byte) []byte {
			binary.LittleEndian.PutUint32(pkt[12:16], placeStorage)
			binary.LittleEndian.PutUint32(pkt[16:20], model.PlayerCargoSlots)
			return pkt
		}},
		{name: "storage requires nearby cargo access", setup: func(_ *World, p *Player, seal model.Item, pkt []byte) []byte {
			p.Char.Inv[0] = model.Item{}
			p.Account.Cargo[0] = seal
			binary.LittleEndian.PutUint32(pkt[12:16], placeStorage)
			return pkt
		}},
		{name: "source item is not a seal", setup: func(_ *World, p *Player, _ model.Item, pkt []byte) []byte {
			p.Char.Inv[0] = model.Item{Index: 1}
			return pkt
		}},
		{name: "seal identifier is invalid", setup: func(_ *World, p *Player, seal model.Item, pkt []byte) []byte {
			p.Char.Inv[0] = model.NewCelestialSeal(seal.UID, 0)
			return pkt
		}},
		{name: "capsule snapshot is missing", setup: func(_ *World, p *Player, seal model.Item, pkt []byte) []byte {
			p.Char.Inv[0] = model.NewCelestialSeal(seal.UID, 2)
			return pkt
		}},
		{name: "capsule item uid does not match", setup: func(_ *World, p *Player, _ model.Item, pkt []byte) []byte {
			p.Account.CelestialCapsules[0].ItemUID = "foreign-item"
			return pkt
		}},
		{name: "invalid extracted character name", setup: func(_ *World, _ *Player, _ model.Item, pkt []byte) []byte {
			clear(pkt[34:50])
			copy(pkt[34:50], "abc")
			return pkt
		}},
		{name: "character name is already reserved", setup: func(w *World, _ *Player, _ model.Item, pkt []byte) []byte {
			w.charNames["restored"] = struct{}{}
			return pkt
		}},
		{name: "name lookup fails", setup: func(w *World, _ *Player, _ model.Item, pkt []byte) []byte {
			w.charNames = nil
			w.store = capsuleNameErrorStore{err: errors.New("name index unavailable")}
			return pkt
		}},
		{name: "account has no free character slot", setup: func(_ *World, p *Player, _ model.Item, pkt []byte) []byte {
			chars := make([]model.Char, maxCharactersPerAccount)
			chars[0] = *p.Char
			for i := 1; i < len(chars); i++ {
				chars[i] = model.Char{Name: "Full" + string(rune('A'+i)), Score: testScore(model.Score{Level: 1, MaxHP: 100, CurHP: 100})}
			}
			p.Account.Chars = chars
			p.CharSlot = 0
			p.Char = &p.Account.Chars[0]
			return pkt
		}},
		{name: "player is not in world", setup: func(_ *World, p *Player, _ model.Item, pkt []byte) []byte {
			p.InWorld = false
			return pkt
		}},
		{name: "player has no current hp", setup: func(_ *World, p *Player, _ model.Item, pkt []byte) []byte {
			setPlayerCurHP(p.Char, 0)
			return pkt
		}},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w, p, seal := celestialExtractionFixture(t)
			pkt := putoutSealPacket(placeInv, 0, 2100, 2100, "Restored")
			pkt = tc.setup(w, p, seal, pkt)
			beforeChars := append([]model.Char(nil), p.Account.Chars...)
			beforeCargo := p.Account.Cargo
			beforeCapsules := append([]model.CelestialCapsule(nil), p.Account.CelestialCapsules...)
			beforeInWorld, beforeSlot := p.InWorld, p.CharSlot

			w.onPutoutSeal(p.Session, pkt)

			if p.InWorld != beforeInWorld || p.CharSlot != beforeSlot || p.Char == nil ||
				!reflect.DeepEqual(p.Account.Chars, beforeChars) || p.Account.Cargo != beforeCargo ||
				!reflect.DeepEqual(p.Account.CelestialCapsules, beforeCapsules) {
				t.Fatalf("rejected request changed account state: inWorld=%v slot=%d chars=%+v cargo=%+v capsules=%+v",
					p.InWorld, p.CharSlot, p.Account.Chars, p.Account.Cargo, p.Account.CelestialCapsules)
			}
		})
	}
}
