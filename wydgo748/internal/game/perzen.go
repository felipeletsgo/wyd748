package game

import (
	"log"

	"wydgo/internal/model"
)

type perzenExchange struct {
	sphere   uint16
	reward   uint16
	maxLevel uint32 // Displayed level; zero means no level condition.
}

// MODERNIZACAO_COMPATIVEL: the official 7.59 Quest/08_Perzen_Normal.c
// and Quest/9 - Perzen_Mistico.c recipes use existing 7.48 interaction and
// inventory packets. Merchant 43 also belongs to unrelated quest NPCs.
func perzenExchangeFor(def *model.NPCDef) (perzenExchange, bool) {
	if def == nil || def.Score == nil || def.Score.Merchant != 43 || def.IsMonster() {
		return perzenExchange{}, false
	}
	switch def.InteractionName() {
	case "Perzen_Normal":
		return perzenExchange{sphere: 4128, reward: 3986}, true
	case "Perzen_Myst", "Perzen_Mistico":
		return perzenExchange{sphere: 4129, reward: 3988, maxLevel: 400}, true
	case "Perzen_Arc", "Perzen_Arcano":
		// Project choice confirmed by the owner: Arcane sphere for the
		// 30-day Thoroughbred. The supplied W2PP has no runtime recipe data.
		return perzenExchange{sphere: 4130, reward: 3987}, true
	default:
		// The plain Perzen template has no verified exchange identity.
		return perzenExchange{}, false
	}
}

func (w *World) usePerzen(p *Player, exchange perzenExchange) {
	if exchange.maxLevel != 0 && p.Char.Score.Level >= exchange.maxLevel {
		circleServiceMessage(p, 190)
		return
	}
	if _, ok := w.items[exchange.reward]; !ok {
		circleServiceMessage(p, 190)
		return
	}
	// Keep the recovered sixty-slot search boundary. Consume one sphere,
	// preserving any remaining stack and its identity. Work on a copy until
	// every requirement is satisfied; a consumed slot may hold the reward.
	inventory := p.Char.Inv
	source := -1
	for slot, item := range inventory[:circleCarrySlots] {
		if item.Index == exchange.sphere {
			source = slot
			break
		}
	}
	if source == -1 {
		circleServiceMessage(p, 303) // Item is not suitable (existing EN/PT text).
		return
	}
	if amount := itemStackAmount(inventory[source]); amount > 1 {
		setItemStackAmount(&inventory[source], amount-1)
	} else {
		inventory[source] = model.Item{}
	}
	destination := -1
	for slot, item := range inventory[:circleCarrySlots] {
		if item.Index == 0 {
			destination = slot
			break
		}
	}
	if destination == -1 {
		circleServiceMessage(p, 1) // Not enough space for items.
		return
	}
	reward, err := materializeItem(model.Item{Index: exchange.reward})
	if err != nil {
		log.Printf("[#%d] Perzen reward identity: %v", p.ID, err)
		circleServiceMessage(p, 190)
		return
	}
	// Use the existing premium-mount lifecycle: the 30-day duration starts
	// on first equip, with server-owned timestamps persisted under this UID.
	inventory[destination] = reward
	images := criticalPreimages(p)
	p.Char.Inv = inventory
	w.commitOfficialNPCService(p, images, "Perzen sphere exchange", false, 0)
}
