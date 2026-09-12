package game

import "wydgo/internal/wire"

type visibilityBounds struct {
	minX int
	maxX int
	minY int
	maxY int
}

func playerVisibilityBounds(x, y uint16) visibilityBounds {
	minX := int(x) - viewHalfX
	minY := int(y) - viewHalfY
	if minX < 0 {
		minX = 0
	}
	if minY < 0 {
		minY = 0
	}
	maxX := int(x) + viewHalfX
	maxY := int(y) + viewHalfY
	const maxCoordinate = int(^uint16(0))
	if maxX > maxCoordinate {
		maxX = maxCoordinate
	}
	if maxY > maxCoordinate {
		maxY = maxCoordinate
	}
	return visibilityBounds{minX: minX, maxX: maxX, minY: minY, maxY: maxY}
}

func (r visibilityBounds) contains(x, y uint16) bool {
	xi, yi := int(x), int(y)
	return xi >= r.minX && xi <= r.maxX && yi >= r.minY && yi <= r.maxY
}

func intersectVisibilityBounds(a, b visibilityBounds) (visibilityBounds, bool) {
	intersection := visibilityBounds{
		minX: a.minX,
		maxX: a.maxX,
		minY: a.minY,
		maxY: a.maxY,
	}
	if b.minX > intersection.minX {
		intersection.minX = b.minX
	}
	if b.maxX < intersection.maxX {
		intersection.maxX = b.maxX
	}
	if b.minY > intersection.minY {
		intersection.minY = b.minY
	}
	if b.maxY < intersection.maxY {
		intersection.maxY = b.maxY
	}
	return intersection, intersection.minX <= intersection.maxX && intersection.minY <= intersection.maxY
}

func appendVisibilityBounds(out *[4]visibilityBounds, count *int, minX, maxX, minY, maxY int) {
	if minX > maxX || minY > maxY || *count >= len(out) {
		return
	}
	out[*count] = visibilityBounds{minX: minX, maxX: maxX, minY: minY, maxY: maxY}
	*count++
}

// visibilityBoundsDifference returns a-b as up to four non-overlapping
// rectangles. The fixed-size result keeps normal movement visibility free of
// temporary slices/maps while preserving the exact 65x65 window semantics.
func visibilityBoundsDifference(a, b visibilityBounds) ([4]visibilityBounds, int) {
	var result [4]visibilityBounds
	intersection, overlaps := intersectVisibilityBounds(a, b)
	if !overlaps {
		result[0] = a
		return result, 1
	}
	count := 0
	appendVisibilityBounds(&result, &count, a.minX, a.maxX, a.minY, intersection.minY-1)
	appendVisibilityBounds(&result, &count, a.minX, a.maxX, intersection.maxY+1, a.maxY)
	appendVisibilityBounds(&result, &count, a.minX, intersection.minX-1, intersection.minY, intersection.maxY)
	appendVisibilityBounds(&result, &count, intersection.maxX+1, a.maxX, intersection.minY, intersection.maxY)
	return result, count
}

func (w *World) ensureAuxVisibilitySpatialIndexes() {
	if len(w.groundItemCell) != len(w.groundItems) {
		for _, item := range w.groundItems {
			if _, indexed := w.groundItemCell[item.ID]; !indexed {
				w.registerGroundItem(item)
			}
		}
	}
	if len(w.ghostShopCell) != len(w.ghostShops) {
		for _, shop := range w.ghostShops {
			if _, indexed := w.ghostShopCell[shop.ID]; !indexed {
				w.registerGhostShop(shop)
			}
		}
	}
}

func (w *World) applyMobVisibilityRegions(p *Player, regions [4]visibilityBounds, count int, entering bool) {
	for i := 0; i < count; i++ {
		r := regions[i]
		for cy := r.minY / spatialCellSize; cy <= r.maxY/spatialCellSize; cy++ {
			for cx := r.minX / spatialCellSize; cx <= r.maxX/spatialCellSize; cx++ {
				for _, m := range w.mobCells[uint32(cx)<<16|uint32(cy)] {
					if m == nil || m.Dead || !r.contains(m.X, m.Y) {
						continue
					}
					if entering {
						if w.mobVisibleToPlayer(p, m) {
							w.showMob(p, m)
						}
						continue
					}
					if !inView(p.X, p.Y, m.X, m.Y) || !w.mobVisibleToPlayer(p, m) {
						w.hideMob(p, m, 0)
					}
				}
			}
		}
	}
}

func (w *World) applyGhostShopVisibilityRegions(p *Player, regions [4]visibilityBounds, count int, entering bool) {
	for i := 0; i < count; i++ {
		r := regions[i]
		for cy := r.minY / spatialCellSize; cy <= r.maxY/spatialCellSize; cy++ {
			for cx := r.minX / spatialCellSize; cx <= r.maxX/spatialCellSize; cx++ {
				for _, shop := range w.ghostShopCells[uint32(cx)<<16|uint32(cy)] {
					if shop == nil || w.ghostShops[shop.ID] != shop || !r.contains(shop.X, shop.Y) {
						continue
					}
					if entering {
						w.showGhostShop(p, shop)
						continue
					}
					if !inView(p.X, p.Y, shop.X, shop.Y) {
						w.hideGhostShop(p, shop)
					}
				}
			}
		}
	}
}

func (w *World) applyPlayerVisibilityRegions(p *Player, regions [4]visibilityBounds, count int, entering bool) {
	for i := 0; i < count; i++ {
		r := regions[i]
		for cy := r.minY / spatialCellSize; cy <= r.maxY/spatialCellSize; cy++ {
			for cx := r.minX / spatialCellSize; cx <= r.maxX/spatialCellSize; cx++ {
				for _, other := range w.playerCells[uint32(cx)<<16|uint32(cy)] {
					if other == nil || other == p || !other.InWorld || other.Char == nil || !r.contains(other.X, other.Y) {
						continue
					}
					if entering {
						if w.playersVisibleTogether(p, other) {
							w.showPlayerPair(p, other)
						}
						continue
					}
					if !inView(p.X, p.Y, other.X, other.Y) || !w.playersVisibleTogether(p, other) {
						w.hidePlayerPair(p, other)
					}
				}
			}
		}
	}
}

func (w *World) applyGroundItemVisibilityRegions(p *Player, regions [4]visibilityBounds, count int, entering bool) {
	for i := 0; i < count; i++ {
		r := regions[i]
		for cy := r.minY / spatialCellSize; cy <= r.maxY/spatialCellSize; cy++ {
			for cx := r.minX / spatialCellSize; cx <= r.maxX/spatialCellSize; cx++ {
				for _, g := range w.groundItemCells[uint32(cx)<<16|uint32(cy)] {
					if g == nil || w.groundItems[g.ID] != g || !r.contains(g.X, g.Y) {
						continue
					}
					if entering {
						if w.groundItemVisibleToPlayer(p, g) && !p.hasVisible(g.ID) {
							p.Session.Send(wire.CreateItem(g.X, g.Y, g.ID, g.Item, g.Rotate, g.State, 0, 0, 0))
							p.show(g.ID)
						}
						continue
					}
					if (!inView(p.X, p.Y, g.X, g.Y) || !w.groundItemVisibleToPlayer(p, g)) && p.hasVisible(g.ID) {
						p.Session.Send(wire.RemoveItem(uint32(g.ID)))
						p.hide(g.ID)
					}
				}
			}
		}
	}
}

// refreshPlayerVisibilityAfterMove applies only the geometric delta between
// the old and new authoritative view windows. This keeps PvP/war visibility
// immediate on every movement step without rebuilding all nearby entity sets.
// Full refresh remains the recovery path for non-overlapping jumps and all
// callers outside normal movement (login, teleport, revive, instance changes).
func (w *World) refreshPlayerVisibilityAfterMove(p *Player, oldX, oldY uint16) {
	if p == nil || !p.InWorld {
		return
	}
	oldBounds := playerVisibilityBounds(oldX, oldY)
	newBounds := playerVisibilityBounds(p.X, p.Y)
	if _, overlaps := intersectVisibilityBounds(oldBounds, newBounds); !overlaps {
		w.refreshPlayerVisibility(p)
		return
	}

	w.updatePlayerSpatial(p)
	w.ensureAuxVisibilitySpatialIndexes()

	entered, enteredCount := visibilityBoundsDifference(newBounds, oldBounds)
	left, leftCount := visibilityBoundsDifference(oldBounds, newBounds)

	// Preserve the entity-type ordering of the full refresh. Each region is
	// non-overlapping, so a candidate is examined at most once per type.
	w.applyMobVisibilityRegions(p, entered, enteredCount, true)
	w.applyMobVisibilityRegions(p, left, leftCount, false)
	w.applyGhostShopVisibilityRegions(p, entered, enteredCount, true)
	w.applyGhostShopVisibilityRegions(p, left, leftCount, false)
	w.applyPlayerVisibilityRegions(p, entered, enteredCount, true)
	w.applyPlayerVisibilityRegions(p, left, leftCount, false)
	w.applyGroundItemVisibilityRegions(p, entered, enteredCount, true)
	w.applyGroundItemVisibilityRegions(p, left, leftCount, false)
}
