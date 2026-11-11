package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func switchStoneWorld(t *testing.T) (*World, *Player, *craftStore) {
	t.Helper()
	p, _ := networkedTestPlayer(1, "Carrier", 2100, 2100)
	p.Account.Chars = []model.Char{{Name: "Master"}, {Name: "Slave"}}
	p.Account.Chars[0].Equip[switchStoneSlot] = model.Item{Index: masterSwitchStone}
	p.Account.Chars[1].Equip[switchStoneSlot] = model.Item{Index: slaveSwitchStone}
	p.CharSlot, p.Char = 0, &p.Account.Chars[0]
	w := worldWithNetworkedPlayers(p)
	backend := &craftStore{}
	w.store = backend
	return w, p, backend
}

func TestSendLoginCharacterListHandlesSwitchStoneOutcomes(t *testing.T) {
	t.Run("no complete pair", func(t *testing.T) {
		w, p, backend := switchStoneWorld(t)
		p.Account.Chars[1].Equip[switchStoneSlot] = model.Item{}
		w.sendLoginCharacterList(p.Session, p)
		if backend.saves != 0 || p.Session.QueuedPacketsForTest() != 1 {
			t.Fatal("incomplete exchange changed or withheld the character list")
		}
		if got := wire.ParseHeader(nextPacket(t, p)).Type; got != wire.OpCharList {
			t.Fatalf("packet type=0x%X, want character list", got)
		}
	})

	t.Run("guild restriction", func(t *testing.T) {
		w, p, backend := switchStoneWorld(t)
		p.Account.Chars[0].GuildID = 7
		w.sendLoginCharacterList(p.Session, p)
		if backend.saves != 0 || p.Session.QueuedPacketsForTest() != 2 {
			t.Fatal("guild restriction did not report the unchanged character list")
		}
		if got := wire.ParseHeader(nextPacket(t, p)).Type; got != wire.OpCharList {
			t.Fatalf("packet type=0x%X, want character list", got)
		}
		if got := wire.ParseHeader(nextPacket(t, p)).Type; got != wire.OpMessagePanel {
			t.Fatalf("packet type=0x%X, want restriction notice", got)
		}
	})

	t.Run("duplicate character name", func(t *testing.T) {
		w, p, _ := switchStoneWorld(t)
		p.Account.Chars = append(p.Account.Chars, model.Char{Name: "MASTER"})
		w.sendLoginCharacterList(p.Session, p)
		if p.Session.QueuedPacketsForTest() != 2 {
			t.Fatal("duplicate name did not receive the reason notice")
		}
	})

	t.Run("persist then swap", func(t *testing.T) {
		w, p, backend := switchStoneWorld(t)
		w.sendLoginCharacterList(p.Session, p)
		if backend.saves != 1 || p.Account.Chars[0].Name != "Slave" || p.Account.Chars[1].Name != "Master" ||
			p.Account.Chars[0].Equip[switchStoneSlot].Index != 0 || p.Account.Chars[1].Equip[switchStoneSlot].Index != 0 ||
			p.Session.QueuedPacketsForTest() != 2 {
			t.Fatal("successful exchange did not persist, swap names and consume both stones")
		}
		if got := wire.ParseHeader(nextPacket(t, p)).Type; got != wire.OpCharList {
			t.Fatalf("packet type=0x%X, want character list", got)
		}
		if got := wire.ParseHeader(nextPacket(t, p)).Type; got != wire.OpMessagePanel {
			t.Fatalf("packet type=0x%X, want success notice", got)
		}
	})

	t.Run("rollback failed save", func(t *testing.T) {
		w, p, backend := switchStoneWorld(t)
		backend.err = errors.New("database unavailable")
		w.sendLoginCharacterList(p.Session, p)
		if backend.saves != 1 || p.Account.Chars[0].Name != "Master" || p.Account.Chars[1].Name != "Slave" ||
			p.Account.Chars[0].Equip[switchStoneSlot].Index != masterSwitchStone ||
			p.Account.Chars[1].Equip[switchStoneSlot].Index != slaveSwitchStone || p.Session.QueuedPacketsForTest() != 2 {
			t.Fatal("failed save did not restore both names and stones")
		}
	})
}
