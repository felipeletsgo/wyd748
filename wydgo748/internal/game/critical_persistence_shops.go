package game

import (
	"fmt"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Merchant proposals are private; taxable transactions also retain the city
// registry and commit treasury revenue together with the account mutation.
func criticalShopProposal(p *Player) (*model.Account, error) {
	if p == nil || p.Account == nil || p.Char == nil || p.CharSlot < 0 ||
		p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] {
		return nil, errCriticalPersistenceIdentity
	}
	return accountStateSnapshot(p.Account), nil
}

func (w *World) queueShopPurchase(p *Player, item model.Item, total, treasury uint64, city int, merchant, sellSlot uint16) error {
	proposed, err := criticalShopProposal(p)
	if err != nil {
		return err
	}
	ch := &proposed.Chars[p.CharSlot]
	if item.Index == 0 || total > uint64(maxCharacterGold) || total > uint64(ch.Gold) {
		return fmt.Errorf("invalid shop purchase")
	}
	dst := addToInv(ch, item)
	if dst < 0 {
		return errShopInventoryFull
	}
	ch.Gold -= uint32(total)
	return w.submitCriticalShopItem(p, proposed, dst, city, treasury, func() {
		p.Session.Send(wire.BuyItem(p.ID, merchant, sellSlot, uint16(dst), p.Char.Gold))
		if p.ShopNPC != merchant {
			// A live edit may invalidate the old grid while persistence runs.
			// Publish the committed item even if the client closed that window.
			p.Session.Send(wire.SendItem(p.ID, placeInv, byte(dst), p.Char.Inv[dst]))
			p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		}
	})
}

func (w *World) queueTicketPurchase(p *Player, ticket model.Item, dst int, total, treasury uint64, city int) error {
	proposed, err := criticalShopProposal(p)
	if err != nil {
		return err
	}
	ch := &proposed.Chars[p.CharSlot]
	if ticket.Index == 0 || ticket.UID == "" || dst < 0 || dst >= model.PlayerCarrySlots ||
		ch.Inv[dst].Index != 0 || total > uint64(maxCharacterGold) || total > uint64(ch.Gold) {
		return fmt.Errorf("invalid ticket purchase")
	}
	ch.Inv[dst], ch.Gold = ticket, ch.Gold-uint32(total)
	return w.submitCriticalShopItem(p, proposed, dst, city, treasury, func() {
		p.Session.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	})
}

func (w *World) queueShopSale(p *Player, src int, net, treasury uint64, city int) error {
	proposed, err := criticalShopProposal(p)
	if err != nil {
		return err
	}
	ch := &proposed.Chars[p.CharSlot]
	if src < 0 || src >= model.PlayerCarrySlots || ch.Inv[src].Index == 0 ||
		net > uint64(maxCharacterGold) || uint64(ch.Gold) > uint64(maxCharacterGold)-net {
		return fmt.Errorf("invalid shop sale")
	}
	ch.Inv[src], ch.Gold = model.Item{}, ch.Gold+uint32(net)
	return w.submitCriticalShopItem(p, proposed, src, city, treasury, func() {
		p.Session.Send(wire.SendItem(p.ID, placeInv, byte(src), p.Char.Inv[src]))
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	})
}

// Publication is actor-owned and never included in the worker's snapshot.
// Install just the economic fields, preserving live identity and positions.
func (w *World) submitCriticalShopItem(p *Player, proposed *model.Account, slot, city int, treasury uint64, publish func()) error {
	item, gold := proposed.Chars[p.CharSlot].Inv[slot], proposed.Chars[p.CharSlot].Gold
	if treasury != 0 {
		if w.guilds == nil || city < 0 || city >= len(cityWarZones) {
			return fmt.Errorf("invalid city treasury")
		}
		guilds := w.criticalGuildSnapshot()
		// Reuse treasury ownership and cap checks on the private registry only.
		if err := (&World{guilds: guilds}).creditCityTreasury(city, treasury); err != nil {
			return err
		}
		return w.queueGuildAccountChange(p, []*Player{p}, guilds, map[*Player]*model.Account{p: proposed},
			"The merchant transaction could not be completed.", func() {
				p.Char.Inv[slot], p.Char.Gold = item, gold
			}, publish)
	}
	_, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{proposed}}, []*Player{p},
		func(criticalPersistenceResult) error {
			p.Char.Inv[slot], p.Char.Gold = item, gold
			if !p.Session.IsClosed() {
				publish()
			}
			return nil
		})
	return err
}
