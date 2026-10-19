package game

import (
	"fmt"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func (w *World) startTowerRound(day string, start time.Time) {
	startTowerRound(w.guilds, day, start)
}

func startTowerRound(registry *model.GuildRegistry, day string, start time.Time) {
	var kingdom byte
	if owner := registry.FindByID(registry.Wars.Castle.Owner); owner != nil {
		kingdom = owner.Kingdom
	}
	// The reference spawns each daily tower for the castle's defending realm,
	// with no capturing guild yet. Its divisor survives unchanged-realm rounds.
	divisor := max(registry.Wars.Tower.DefenderCaptures, 1)
	registry.Wars.Tower = model.TowerWarState{Day: day, EndsAt: start.Add(time.Hour), Kingdom: kingdom, InitialKingdom: kingdom, DefenderCaptures: divisor}
}

func (w *World) publishTowerFinish() {
	s := w.guilds.Wars.Tower
	w.removeWarTower()
	w.ejectTowerOpponents(s.Kingdom)
	if s.Kingdom == 0 {
		w.warNotice("Tower war over without a winning kingdom.")
	} else if s.Kingdom != s.InitialKingdom && w.guilds.FindByID(s.Owner) != nil {
		w.warNotice(fmt.Sprintf("Tower war over! Kingdom %d won; guild %s receives 100 fame. Eligible players receive a random medal.", s.Kingdom, w.warGuildName(s.Owner)))
	} else {
		w.warNotice(fmt.Sprintf("Tower war over! Kingdom %d defended; no guild fame awarded. Eligible players receive a random medal.", s.Kingdom))
	}
}

func (w *World) ejectTowerOpponents(kingdom byte) {
	for _, p := range w.players {
		if !validPlayerMobParticipant(p) || w.gameplaySpaceForPlayer(p) != "" || !towerWarArena.contains(p.X, p.Y) {
			continue
		}
		g, _ := w.guildOf(p.Char)
		if g != nil && g.Kingdom != kingdom {
			w.requestPlayerTeleport(p, 2453, 2000, nil, nil)
		}
	}
}

// Select each color with exactly one of the three equally likely Intn results.
// The roll and recipient identity are persisted with the event, not at delivery.
func (w *World) addTowerMedals(registry *model.GuildRegistry, phase string) {
	s := registry.Wars.Tower
	if s.Kingdom != 7 && s.Kingdom != 8 {
		return
	}
	event := fmt.Sprintf("tower/%s/%s/%d", s.Day, phase, s.Captures)
	for _, p := range w.players {
		if !validPlayerMobParticipant(p) || p.Account == nil || w.gameplaySpaceForPlayer(p) != "" || !towerWarArena.contains(p.X, p.Y) {
			continue
		}
		g, _ := w.guildOf(p.Char)
		if g == nil || g.Kingdom != s.Kingdom {
			continue
		}
		pending := false
		for _, claim := range registry.Wars.Medals {
			if claim.Event == event && claim.CharacterUID == p.Char.UID {
				pending = true
				break
			}
		}
		if pending {
			continue
		}
		item := [...]uint16{model.WarMedalGreen, model.WarMedalGold, model.WarMedalRed}[w.intn(3)]
		registry.Wars.Medals = append(registry.Wars.Medals, model.WarMedalClaim{Event: event, Account: p.Account.Name, CharacterUID: p.Char.UID, Item: item})
	}
}

// Deliver one available claim per tick. The durable claim queue allows any
// number of recipients without one all-player transaction or inventory loss.
func (w *World) deliverWarMedal() error {
	for i, claim := range w.guilds.Wars.Medals {
		p := w.playerByCharacterUID(claim.CharacterUID)
		if p == nil || !p.InWorld || p.Account == nil || p.Char == nil || p.Session == nil || p.Session.IsClosed() ||
			p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] ||
			accountSessionKey(p.Account.Name) != accountSessionKey(claim.Account) || p.Trade != nil || w.criticalTradeReserved(p) {
			continue
		}
		slot := -1
		for j := 0; j < model.PlayerCarrySlots; j++ {
			if p.Char.Inv[j].Index == 0 {
				slot = j
				break
			}
		}
		if slot < 0 {
			continue
		}
		item, err := materializeItem(model.Item{Index: claim.Item})
		if err != nil {
			return err
		}
		publish := func() {
			p.Session.Send(wire.SendItem(p.ID, placeInv, byte(slot), item))
			p.Session.Send(wire.MessagePanel("Tower war reward: one random green, gold or red medal. Each color has equal chance."))
		}
		if w.criticalWrites != nil {
			proposed, account := w.criticalGuildSnapshot(), accountStateSnapshot(p.Account)
			proposed.Wars.Medals = append(proposed.Wars.Medals[:i], proposed.Wars.Medals[i+1:]...)
			account.Chars[p.CharSlot].Inv[slot] = item
			return w.queueGuildAccountChange(p, []*Player{p}, proposed, map[*Player]*model.Account{p: account},
				"Medal delivery could not be saved. Your reward remains pending.", func() { p.Char.Inv[slot] = item }, publish)
		}
		if err := w.guildEconomyCommit(p, func() error {
			p.Char.Inv[slot] = item
			w.guilds.Wars.Medals = append(w.guilds.Wars.Medals[:i], w.guilds.Wars.Medals[i+1:]...)
			return nil
		}); err != nil {
			return err
		}
		publish()
		return nil
	}
	return nil
}
