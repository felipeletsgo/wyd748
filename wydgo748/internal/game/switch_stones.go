package game

import (
	"log"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// Exchange stones: the player equips the Greater Exchange Stone
// (Master_Switch_Stone, 774) on one character and the Lesser Exchange Stone
// (Slave_Switch_Stone, 775) on another character of the same account. At the
// next login the two characters swap names and both stones are consumed.
// Evidence: 7.59 DBSRVIN.exe FUN_00471210 (0x4744d9/0x474513) scans the four
// characters' Equip[13] at account load, swaps the 16-byte names, zeroes
// both stones and logs "etc,name swap".
const (
	masterSwitchStone = 774
	slaveSwitchStone  = 775
	switchStoneSlot   = 13
)

// planSwitchStones returns the two slots to swap, or a reason when stones are
// equipped but the swap cannot happen.
func planSwitchStones(acc *model.Account) (master, slave int, reason string, ok bool) {
	master, slave = -1, -1
	for i := range acc.Chars {
		ch := &acc.Chars[i]
		if ch.Name == "" {
			continue
		}
		switch ch.Equip[switchStoneSlot].Index {
		case masterSwitchStone:
			if master >= 0 && master != i {
				return -1, -1, "Only one character may equip the Greater Exchange Stone.", false
			}
			master = i
		case slaveSwitchStone:
			if slave >= 0 && slave != i {
				return -1, -1, "Only one character may equip the Lesser Exchange Stone.", false
			}
			slave = i
		}
	}
	if master < 0 && slave < 0 {
		return -1, -1, "", false
	}
	if master < 0 || slave < 0 {
		return -1, -1, "", false // waiting for the other stone
	}
	a, b := &acc.Chars[master], &acc.Chars[slave]
	// Guild rosters list members by name; a renamed member would lose its
	// guild. Names shared with a linked evolution would also split.
	if a.GuildID != 0 || b.GuildID != 0 {
		return -1, -1, "Leave your guild before exchanging names.", false
	}
	for i := range acc.Chars {
		if i == master || i == slave || acc.Chars[i].Name == "" {
			continue
		}
		if strings.EqualFold(acc.Chars[i].Name, a.Name) || strings.EqualFold(acc.Chars[i].Name, b.Name) {
			return -1, -1, "These characters share their name with another character.", false
		}
	}
	return master, slave, "", true
}

func applySwitchStones(acc *model.Account, master, slave int) {
	a, b := &acc.Chars[master], &acc.Chars[slave]
	a.Name, b.Name = b.Name, a.Name
	a.Equip[switchStoneSlot], b.Equip[switchStoneSlot] = model.Item{}, model.Item{}
}

// sendLoginCharacterList applies a pending name exchange before the account
// receives its character list.
func (w *World) sendLoginCharacterList(s *net.Session, p *Player) {
	acc := p.Account
	master, slave, reason, ok := planSwitchStones(acc)
	if !ok {
		s.Send(wire.CharList(acc.Name, acc.Chars, acc.Cargo[:], acc.CargoGold))
		if reason != "" {
			s.Send(wire.MessagePanel(reason))
		}
		return
	}
	oldA, oldB := acc.Chars[master].Name, acc.Chars[slave].Name
	done := func() {
		if s.IsClosed() {
			return
		}
		s.Send(wire.CharList(acc.Name, acc.Chars, acc.Cargo[:], acc.CargoGold))
		s.Send(wire.MessagePanel("The Exchange Stones swapped " + oldA + " and " + oldB + "."))
		log.Printf("[#%d] EXCHANGE STONES account=%s %q <-> %q", s.ID, acc.Name, oldA, oldB)
	}
	failed := func() {
		if !s.IsClosed() {
			s.Send(wire.CharList(acc.Name, acc.Chars, acc.Cargo[:], acc.CargoGold))
			s.Send(wire.MessagePanel("The name exchange could not be saved."))
		}
	}
	if w.criticalWrites != nil {
		images := criticalPreimages(p)
		applySwitchStones(acc, master, slave)
		if err := w.queueCriticalLiveMutation("exchange stones", images, done,
			criticalLiveMutationOptions{reject: failed}); err != nil {
			log.Printf("[#%d] exchange stones admission rejected: %v", s.ID, err)
			failed()
		}
		return
	}
	before := accountStateSnapshot(acc)
	applySwitchStones(acc, master, slave)
	if err := w.saveAccount(acc); err != nil {
		installAccountInPlace(acc, before)
		log.Printf("[#%d] save exchange stones: %v", s.ID, err)
		failed()
		return
	}
	done()
}
