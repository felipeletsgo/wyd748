package game

import (
	"encoding/binary"
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type cargoGoldContractStore struct {
	craftStore
	player *Player
	t      *testing.T
	gold   uint32
	cargo  uint32
}

func (s *cargoGoldContractStore) SaveAccount(account *model.Account) error {
	s.t.Helper()
	if s.player.Session.QueuedPacketsForTest() != 0 {
		s.t.Fatal("transfer confirmation was published before persistence")
	}
	s.gold, s.cargo = account.Chars[0].Gold, account.CargoGold
	return s.craftStore.SaveAccount(account)
}

func cargoGoldContractWorld(t *testing.T) (*World, *Player, *cargoGoldContractStore) {
	t.Helper()
	w, p, _ := handlerTestWorld(t)
	banker := &Mob{ID: 1102, X: 2102, Y: 2100, Def: &model.NPCDef{
		Name: "Cargo", Tipo: model.TipoNPC, Score: &model.Score{Merchant: 2},
	}}
	w.registerMobSpatial(banker)
	p.show(banker.ID)
	p.CargoNPC = banker.ID
	p.Char.Gold, p.Account.CargoGold = 1000, 500
	store := &cargoGoldContractStore{player: p, t: t}
	w.store = store
	return w, p, store
}

func cargoGoldRequest(deposit bool, amount uint32) []byte {
	opcode := uint16(wire.OpWithdraw)
	if deposit {
		opcode = wire.OpDeposit
	}
	packet := inboundPacket(opcode, 16)
	binary.LittleEndian.PutUint32(packet[12:16], amount)
	return packet
}

func assertCargoGoldPublication(t *testing.T, p *Player, deposit bool, amount, gold, cargo uint32) {
	t.Helper()
	opcode := uint16(wire.OpWithdraw)
	if deposit {
		opcode = wire.OpDeposit
	}
	for sequence, expected := range []struct {
		opcode uint16
		size   int
		id     uint16
		offset int
		value  uint32
	}{
		{opcode, 16, wire.SceneField, 12, amount},
		{wire.OpUpdateCargoGold, 16, wire.SceneField, 12, cargo},
		{wire.OpUpdateEtc, 36, p.ID, 32, gold},
	} {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok || len(packet) != expected.size || !wire.Decrypt(packet) {
			t.Fatalf("response %d is not the expected encrypted %d-byte frame", sequence, expected.size)
		}
		header := wire.ParseHeader(packet)
		if header.Type != expected.opcode || int(header.Size) != expected.size || header.ID != expected.id ||
			binary.LittleEndian.Uint32(packet[expected.offset:expected.offset+4]) != expected.value {
			t.Fatalf("response %d differs from the committed transfer: % x", sequence, packet)
		}
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("transfer emitted an unexpected extra response")
	}
}

func TestCargoGoldContractPersistenceAndPublication(t *testing.T) {
	for _, deposit := range []bool{true, false} {
		name := "withdraw"
		if deposit {
			name = "deposit"
		}
		t.Run(name, func(t *testing.T) {
			w, p, store := cargoGoldContractWorld(t)
			request := cargoGoldRequest(deposit, 200)
			gold, cargo := uint32(1200), uint32(300)
			if deposit {
				gold, cargo = 800, 700
			}
			w.onCargoGold(p.Session, request, deposit)
			if store.saves != 1 || store.gold != gold || store.cargo != cargo ||
				p.Char.Gold != gold || p.Account.CargoGold != cargo {
				t.Fatal("committed balances differ from the persisted snapshot")
			}
			assertCargoGoldPublication(t, p, deposit, 200, gold, cargo)
			// The native request has no transaction ID: repetition is a new
			// transfer, not an idempotent replay. Each one must persist first.
			w.onCargoGold(p.Session, request, deposit)
			if deposit {
				gold, cargo = 600, 900
			} else {
				gold, cargo = 1400, 100
			}
			if store.saves != 2 || store.gold != gold || store.cargo != cargo ||
				p.Char.Gold != gold || p.Account.CargoGold != cargo {
				t.Fatal("repeated transfer did not persist its own final balances")
			}
			assertCargoGoldPublication(t, p, deposit, 200, gold, cargo)
		})
	}
}

func TestCargoGoldContractRollbackAndRetry(t *testing.T) {
	for _, deposit := range []bool{true, false} {
		name := "withdraw"
		if deposit {
			name = "deposit"
		}
		t.Run(name, func(t *testing.T) {
			w, p, store := cargoGoldContractWorld(t)
			store.err = errors.New("injected persistence failure")
			before := *p.Char
			request := cargoGoldRequest(deposit, 200)
			for attempt := 1; attempt <= 2; attempt++ {
				w.onCargoGold(p.Session, request, deposit)
				if !reflect.DeepEqual(*p.Char, before) || p.Account.CargoGold != 500 || store.saves != attempt ||
					p.Session.QueuedPacketsForTest() != 1 {
					t.Fatal("failed persistence changed balances or published success")
				}
				packet, ok := p.Session.DequeuePacketForTest()
				if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpMessagePanel {
					t.Fatal("failure must publish only a message panel, not a transfer")
				}
			}
			store.err = nil
			w.onCargoGold(p.Session, request, deposit)
			gold, cargo := uint32(1200), uint32(300)
			if deposit {
				gold, cargo = 800, 700
			}
			if store.saves != 3 || p.Char.Gold != gold || p.Account.CargoGold != cargo {
				t.Fatal("retry after rollback applied the transfer more than once")
			}
			assertCargoGoldPublication(t, p, deposit, 200, gold, cargo)
		})
	}
}

func TestCargoGoldContractRejections(t *testing.T) {
	for _, deposit := range []bool{true, false} {
		name := "withdraw"
		if deposit {
			name = "deposit"
		}
		for _, reason := range []string{"zero", "over-limit", "insufficient", "destination-limit", "dead", "out-of-world", "hidden-banker", "distant-banker", "truncated", "oversized"} {
			t.Run(name+"/"+reason, func(t *testing.T) {
				w, p, store := cargoGoldContractWorld(t)
				request := cargoGoldRequest(deposit, 200)
				wantPanel := false
				switch reason {
				case "zero":
					request = cargoGoldRequest(deposit, 0)
				case "over-limit":
					request = cargoGoldRequest(deposit, maxCharacterGold+1)
				case "insufficient":
					request = cargoGoldRequest(deposit, 1001)
					wantPanel = true
				case "destination-limit":
					if deposit {
						p.Account.CargoGold = maxCharacterGold - 199
					} else {
						p.Char.Gold = maxCharacterGold - 199
					}
					wantPanel = true
				case "dead":
					setPlayerCurHP(p.Char, 0)
				case "out-of-world":
					p.InWorld = false
				case "hidden-banker":
					delete(p.Visible, p.CargoNPC)
				case "distant-banker":
					p.X += npcInteractionRange + 20
				case "truncated":
					request = request[:15]
				case "oversized":
					request = append(request, 0)
				}
				before, cargo := *p.Char, p.Account.CargoGold
				w.onCargoGold(p.Session, request, deposit)
				if !reflect.DeepEqual(*p.Char, before) || p.Account.CargoGold != cargo || store.saves != 0 {
					t.Fatal("rejected transfer mutated or persisted account state")
				}
				wantCount := 0
				if wantPanel {
					wantCount = 1
				}
				if p.Session.QueuedPacketsForTest() != wantCount {
					t.Fatal("rejected transfer emitted a success confirmation")
				}
			})
		}
	}
}
