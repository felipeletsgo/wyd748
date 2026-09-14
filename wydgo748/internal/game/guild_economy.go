package game

import (
	"errors"
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

const (
	cityWarInteractionTTL = 30 * time.Second
	cityWarTaxRate        = uint32(10)
	cityWarChequeItem     = uint16(4011)
	cityWarChequeValue    = uint64(1_000_000_000)
)

var errShopInventoryFull = errors.New("shop inventory full")

// cityCollectorCity identifies only the active city-tax NPCs. The city is
// taken from the NPC definition and its position is checked against the same
// four 7.48 city rectangles used by the war scheduler; a client cannot select
// a collector ID from another map or city.
func cityCollectorCity(m *Mob) (int, bool) {
	if m == nil || m.Def == nil || m.Def.Score == nil || m.Def.Score.Merchant != 6 {
		return 0, false
	}
	city := int(m.Def.Score.Level)
	if city < 0 || city >= len(cityWarZones) || !cityWarZones[city].city.contains(m.X, m.Y) {
		return 0, false
	}
	return city, true
}

// cityShopTax returns the rate for an ordinary owned-city shop. Collectors
// are deliberately excluded: their click is a treasury interaction, not a
// purchasable shop. Missing or stale owners fail closed and do not create tax
// revenue outside a valid guild territory.
func (w *World) cityShopTax(m *Mob) (city int, rate uint32, ok bool) {
	if w == nil || w.guilds == nil || m == nil || m.Def == nil || m.Def.Score == nil {
		return 0, 0, false
	}
	if _, collector := cityCollectorCity(m); collector {
		return 0, 0, false
	}
	if _, shop := shopTypeForMerchant(m.Def.Score.Merchant); !shop {
		return 0, 0, false
	}
	for city := range cityWarZones {
		if !cityWarZones[city].city.contains(m.X, m.Y) {
			continue
		}
		owner := w.guilds.Wars.Cities.Territories[city].Owner
		if owner == 0 || w.guilds.FindByID(owner) == nil {
			return 0, 0, false
		}
		return city, cityWarTaxRate, true
	}
	return 0, 0, false
}

// cityPriceForNPC applies the displayed city tax to a base price. One quarter
// of the surcharge becomes city treasury revenue; the rest remains a server
// fee and is intentionally not minted into any player balance.
func (w *World) cityPriceForNPC(m *Mob, base uint64) (total, treasury uint64, city int, rate uint32) {
	total, city = base, -1
	city, rate, ok := w.cityShopTax(m)
	if !ok || base == 0 {
		return total, 0, -1, 0
	}
	tax := base * uint64(rate) / 100
	return base + tax, tax / 4, city, rate
}

func clearCityWarContext(p *Player) {
	if p == nil {
		return
	}
	p.CityWarNPC = 0
	p.CityWarCity = 0
	p.CityWarGuild = 0
	p.CityWarUntil = time.Time{}
}

func setCityWarContext(p *Player, npc uint16, city int, guild uint16, now time.Time) {
	if p == nil {
		return
	}
	p.CityWarNPC = npc
	p.CityWarCity = city
	p.CityWarGuild = guild
	p.CityWarUntil = now.Add(cityWarInteractionTTL)
}

func (w *World) cityWarLocal(now time.Time) time.Time {
	if w != nil && w.warLocation != nil {
		return now.In(w.warLocation)
	}
	return now
}

// guildEconomyCommit commits a player-account mutation and the city treasury
// in the existing SaveGameState transaction. The live account and guild
// registry are restored together if either the change or persistence fails,
// preventing gold/item duplication and phantom tax revenue.
func (w *World) guildEconomyCommit(p *Player, change func() error) error {
	if w == nil || w.guilds == nil || p == nil || p.Account == nil || p.Char == nil || change == nil {
		return fmt.Errorf("guild economy transaction unavailable")
	}
	accountBefore := accountStateSnapshot(p.Account)
	guildsBefore := w.snapshotGuilds()
	warsBefore := w.guilds.Wars.Clone()
	restore := func() {
		w.restoreGuilds(guildsBefore)
		w.guilds.Wars = warsBefore
		restoreAccountState(p.Account, accountBefore)
		if p.CharSlot >= 0 && p.CharSlot < len(p.Account.Chars) {
			p.Char = &p.Account.Chars[p.CharSlot]
		}
	}
	if err := change(); err != nil {
		restore()
		return err
	}
	if err := w.saveGuildState(p.Account); err != nil {
		restore()
		return err
	}
	return nil
}

func (w *World) creditCityTreasury(city int, amount uint64) error {
	if amount == 0 {
		return nil
	}
	if w == nil || w.guilds == nil || city < 0 || city >= len(cityWarZones) {
		return fmt.Errorf("invalid city treasury")
	}
	t := &w.guilds.Wars.Cities.Territories[city]
	if t.Owner == 0 || w.guilds.FindByID(t.Owner) == nil {
		return fmt.Errorf("city has no valid owner")
	}
	if t.Treasury > model.GuildWarTreasuryCap || amount > model.GuildWarTreasuryCap-t.Treasury {
		return fmt.Errorf("city treasury cap exceeded")
	}
	t.Treasury += amount
	return nil
}

// commitShopPurchase performs the normal account-only save when no city is
// owned, and upgrades to the guild transaction when tax revenue is present.
func (w *World) commitShopPurchase(p *Player, item model.Item, total uint64, city int, treasury uint64) (int, error) {
	if p == nil || p.Char == nil || total > uint64(maxCharacterGold) || total > uint64(p.Char.Gold) {
		return -1, fmt.Errorf("invalid shop purchase")
	}
	oldGold := p.Char.Gold
	dst := -1
	apply := func() error {
		dst = addToInv(p.Char, item)
		if dst < 0 {
			return errShopInventoryFull
		}
		p.Char.Gold -= uint32(total)
		return w.creditCityTreasury(city, treasury)
	}
	if treasury != 0 {
		if err := w.guildEconomyCommit(p, apply); err != nil {
			return -1, err
		}
		return dst, nil
	}
	if err := apply(); err != nil {
		return -1, err
	}
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[dst] = model.Item{}
		p.Char.Gold = oldGold
		return -1, err
	}
	return dst, nil
}

func (w *World) commitTicketPurchase(p *Player, ticket model.Item, dst int, total uint64, city int, treasury uint64) error {
	if p == nil || p.Char == nil || dst < 0 || dst >= model.PlayerCarrySlots || total > uint64(maxCharacterGold) || total > uint64(p.Char.Gold) {
		return fmt.Errorf("invalid ticket purchase")
	}
	oldItem, oldGold := p.Char.Inv[dst], p.Char.Gold
	apply := func() error {
		if p.Char.Inv[dst].Index != 0 {
			return fmt.Errorf("inventory slot is no longer empty")
		}
		p.Char.Inv[dst] = ticket
		p.Char.Gold -= uint32(total)
		return w.creditCityTreasury(city, treasury)
	}
	if treasury != 0 {
		return w.guildEconomyCommit(p, apply)
	}
	if err := apply(); err != nil {
		return err
	}
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[dst], p.Char.Gold = oldItem, oldGold
		return err
	}
	return nil
}

func (w *World) commitShopSale(p *Player, src *model.Item, net uint64, city int, treasury uint64) error {
	if p == nil || p.Char == nil || src == nil || net > uint64(maxCharacterGold) || uint64(p.Char.Gold) > uint64(maxCharacterGold)-net {
		return fmt.Errorf("invalid shop sale")
	}
	oldItem, oldGold := *src, p.Char.Gold
	apply := func() error {
		*src = model.Item{}
		p.Char.Gold += uint32(net)
		return w.creditCityTreasury(city, treasury)
	}
	if treasury != 0 {
		return w.guildEconomyCommit(p, apply)
	}
	if err := apply(); err != nil {
		return err
	}
	if err := w.saveAccount(p.Account); err != nil {
		*src, p.Char.Gold = oldItem, oldGold
		return err
	}
	return nil
}

// withdrawCityTreasury follows the native collector split: a balance below a
// billion becomes character gold; whole billions become cheque item 4011.
// Limited carry space withdraws only what fits and leaves the remainder in the
// treasury for a later collection.
func (w *World) withdrawCityTreasury(p *Player, city int) error {
	if w == nil || w.guilds == nil || p == nil || p.Char == nil || city < 0 || city >= len(cityWarZones) {
		return fmt.Errorf("invalid city treasury")
	}
	local := w.cityWarLocal(w.now())
	if local.Weekday() == time.Saturday {
		return fmt.Errorf("treasury collection is closed during Saturday registration")
	}
	g, err := w.requireGuildLeader(p.Char)
	if err != nil {
		return err
	}
	t := &w.guilds.Wars.Cities.Territories[city]
	if t.Owner != g.ID {
		return fmt.Errorf("only the city owner can collect its treasury")
	}
	if t.Treasury == 0 {
		return fmt.Errorf("city treasury is empty")
	}
	if t.Treasury < cityWarChequeValue {
		if uint64(p.Char.Gold) > uint64(maxCharacterGold)-t.Treasury {
			return fmt.Errorf("not enough room for the treasury gold")
		}
		amount := t.Treasury
		if err := w.guildEconomyCommit(p, func() error {
			p.Char.Gold += uint32(amount)
			t.Treasury = 0
			return nil
		}); err != nil {
			return err
		}
		if p.Session != nil {
			p.Session.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
			p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
			p.Session.Send(wire.MessagePanel(fmt.Sprintf("Collected %d gold from the city treasury.", amount)))
		}
		return nil
	}

	units := t.Treasury / cityWarChequeValue
	slots := make([]int, 0, units)
	for slot := 0; slot < model.PlayerCarrySlots && uint64(len(slots)) < units; slot++ {
		if p.Char.Inv[slot].Index == 0 {
			slots = append(slots, slot)
		}
	}
	if len(slots) == 0 {
		return fmt.Errorf("inventory is full for treasury cheques")
	}
	items := make([]model.Item, len(slots))
	for i := range items {
		items[i], err = materializeItem(model.Item{Index: cityWarChequeItem})
		if err != nil {
			return fmt.Errorf("create treasury cheque: %w", err)
		}
	}
	amount := uint64(len(items)) * cityWarChequeValue
	if err := w.guildEconomyCommit(p, func() error {
		for i, slot := range slots {
			if p.Char.Inv[slot].Index != 0 {
				return fmt.Errorf("inventory changed during treasury collection")
			}
			p.Char.Inv[slot] = items[i]
		}
		t.Treasury -= amount
		return nil
	}); err != nil {
		return err
	}
	sessionID := int64(0)
	if p.Session != nil {
		sessionID = p.Session.ID
		p.Session.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		p.Session.Send(wire.MessagePanel(fmt.Sprintf("Collected %d billion-gold cheque(s) from the city treasury.", len(items))))
	}
	log.Printf("[#%d] coletou %d cheque(s) da cidade %s; restante=%d", sessionID, len(items), cityWarZones[city].name, t.Treasury)
	return nil
}
