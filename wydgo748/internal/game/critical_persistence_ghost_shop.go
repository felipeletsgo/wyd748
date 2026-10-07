package game

import "wydgo/internal/model"

// The seller's account fence also owns this shared shop: every purchase and
// shop mutation checks the owner before touching its listing or Cargo.
func (w *World) queueGhostShopPurchase(buyer, seller *Player, shop *GhostShop, pos int,
	buyerInv [model.MaxCarry]model.Item, sellerCargo [model.MaxCargo]model.Item,
	buyerGold, sellerCargoGold uint32, buyerSlot int) error {
	for _, p := range []*Player{buyer, seller} {
		if p == nil || p.Account == nil || p.Char == nil || p.CharSlot < 0 ||
			p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] {
			return errCriticalPersistenceIdentity
		}
	}
	if shop == nil || pos < 0 || pos >= maxGhostShopItems || seller.GhostShop != shop ||
		w.ghostShops[shop.ID] != shop || shop.OwnerID != seller.ID || buyer.BrowsingGhostShopID != shop.ID {
		return errCriticalPersistenceIdentity
	}
	context := *shop
	item, price, storageSlot := shop.Items[pos], shop.Prices[pos], int(shop.CarryPos[pos])
	buyerAccount, sellerAccount := accountStateSnapshot(buyer.Account), accountStateSnapshot(seller.Account)
	buyerAccount.Chars[buyer.CharSlot].Inv, buyerAccount.Chars[buyer.CharSlot].Gold = buyerInv, buyerGold
	sellerAccount.Cargo, sellerAccount.CargoGold = sellerCargo, sellerCargoGold
	_, err := w.submitCriticalWritePhased(criticalWrite{accounts: []*model.Account{buyerAccount, sellerAccount}},
		[]*Player{buyer, seller}, false, func(criticalPersistenceResult) error {
			if seller.GhostShop != shop || w.ghostShops[context.ID] != shop || *shop != context ||
				buyer.BrowsingGhostShopID != context.ID {
				return errCriticalPersistenceIdentity
			}
			buyer.Char.Inv, buyer.Char.Gold = buyerInv, buyerGold
			seller.Account.Cargo, seller.Account.CargoGold = sellerCargo, sellerCargoGold
			shop.Items[pos], shop.CarryPos[pos], shop.Prices[pos] = model.Item{}, -1, 0
			return nil
		}, func(criticalPersistenceResult) error {
			w.finishGhostShopPurchase(buyer, seller, shop, pos, item, price, storageSlot, buyerSlot)
			return nil
		})
	return err
}
