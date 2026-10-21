package game

import (
	"log"
	"time"

	"wydgo/internal/wire"
)

// Owner rule: a character with an active ghost (offline) shop earns one honor
// point for every ten minutes the shop stays open, announced by a letter.
// WYD 7.48 and the 7.59 server have no equivalent; this is a WYD-Go mechanic.
const (
	honorCounter           = "honor"
	ghostShopHonorInterval = 10 * time.Minute
	ghostShopHonorLetter   = "!You received 1 honor."
	ghostShopHonorSender   = "Honor"
)

// tickGhostShopHonor keeps its schedule outside GhostShop, so a held shop
// persistence never sees the shop snapshot change.
func (w *World) tickGhostShopHonor(now time.Time) {
	if w.ghostShopHonorAt == nil {
		w.ghostShopHonorAt = make(map[uint16]time.Time)
	}
	for id := range w.ghostShopHonorAt {
		if w.ghostShops[id] == nil {
			delete(w.ghostShopHonorAt, id)
		}
	}
	for id, shop := range w.ghostShops {
		if shop == nil {
			continue
		}
		next, scheduled := w.ghostShopHonorAt[id]
		if !scheduled {
			w.ghostShopHonorAt[id] = now.Add(ghostShopHonorInterval)
			continue
		}
		if now.Before(next) {
			continue
		}
		owner := w.playerByID(shop.OwnerID)
		if owner == nil || owner.GhostShop != shop || owner.Account == nil || owner.Char == nil || owner.Session == nil {
			continue
		}
		if w.criticalPlayerReserved(owner) {
			continue // retried on the next tick
		}
		// Honor lives in the character sidecar, not the account. Never admit a
		// credit on a backend that cannot save that sidecar atomically.
		if w.criticalWrites != nil && !w.criticalStoreHasSidecars() {
			continue
		}
		images := criticalPreimages(owner)
		grantCounters(owner, map[string]uint32{honorCounter: 1})
		publish := func() {
			if w.ghostShops[id] == shop && owner.GhostShop == shop {
				w.ghostShopHonorAt[id] = next.Add(ghostShopHonorInterval)
			}
			if !owner.Session.IsClosed() {
				owner.Session.Send(wire.MessageWhisper(0, ghostShopHonorSender, ghostShopHonorLetter, 0))
			}
			log.Printf("[#%d] GHOST SHOP honor +1 owner=%s total=%d", owner.Session.ID,
				owner.Char.Name, counterBalance(owner, honorCounter))
		}
		if w.criticalWrites != nil {
			if err := w.queueCriticalLiveMutation("ghost_shop_honor", images, publish,
				criticalLiveMutationOptions{sidecar: true}); err != nil {
				log.Printf("[#%d] GHOST SHOP honor admission failed: %v", owner.Session.ID, err)
			}
			continue
		}
		if err := w.saveAccountAndCharStateResult(owner); err != nil {
			w.restoreCriticalPreimages(images)
			log.Printf("[#%d] GHOST SHOP honor persistence failed owner=%s: %v",
				owner.Session.ID, owner.Char.Name, err)
			continue
		}
		publish()
	}
}
