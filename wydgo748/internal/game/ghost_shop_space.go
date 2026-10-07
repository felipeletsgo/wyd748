package game

// Ghost shops are stationary public-world entities, independent of their
// owner's subsequent runtime membership. Opening one requires public Armia;
// viewing or buying from one requires the observer to remain in public space.
func (w *World) ghostShopVisibleToPlayer(p *Player, shop *GhostShop) bool {
	return p != nil && shop != nil && w.gameplaySpaceForPlayer(p) == ""
}
