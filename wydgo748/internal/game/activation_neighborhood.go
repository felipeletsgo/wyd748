package game

import "time"

// activationNeighborhood serves one recomputeMobsNearCell pass (W6 item 5).
// The pass used to scan the player cell maps once per nearby mob. It now reads
// each cell map once into a window and memoizes player eligibility. Each mob
// still checks only the cells its own scan covers, so the result is exactly
// the previous one. Nothing survives the pass: no invalidation is needed.
type activationNeighborhood struct {
	originX, originY, side int
	built                  bool
	cells                  [][]*Player
	eligible               map[*Player]bool
	now                    time.Time
}

// reset prepares the World's scratch window around one cell. The window is
// filled on first use, so a pass without public-world mobs reads no players.
func (n *activationNeighborhood) reset(cx, cy, reach int, now time.Time) {
	n.originX, n.originY, n.side, n.built, n.now = cx-reach, cy-reach, 2*reach+1, false, now
	if cells := n.side * n.side; cap(n.cells) < cells {
		n.cells = make([][]*Player, cells)
	} else {
		n.cells = n.cells[:cells]
	}
	if n.eligible == nil {
		n.eligible = make(map[*Player]bool)
	}
	clear(n.eligible)
}

func (n *activationNeighborhood) index(xx, yy int) int {
	dx, dy := xx-n.originX, yy-n.originY
	if dx < 0 || dy < 0 || dx >= n.side || dy >= n.side {
		return -1
	}
	return dy*n.side + dx
}

func (n *activationNeighborhood) build(w *World) {
	for i := range n.cells {
		n.cells[i] = n.cells[i][:0]
	}
	for dy := range n.side {
		for dx := range n.side {
			xx, yy := n.originX+dx, n.originY+dy
			if xx < 0 || yy < 0 {
				continue
			}
			i := dy*n.side + dx
			for _, p := range w.playerCells[uint32(xx)<<16|uint32(yy)] {
				n.cells[i] = append(n.cells[i], p)
			}
		}
	}
	n.built = true
}

// hasNearbyPublicPlayer equals hasNearbyPlayerInGameplaySpace(x, y, radius, "")
// for any mob whose scan window lies inside the neighborhood.
func (w *World) hasNearbyPublicPlayer(n *activationNeighborhood, x, y uint16, radius int) bool {
	if radius < 0 {
		return false
	}
	cx, cy := int(x)/spatialCellSize, int(y)/spatialCellSize
	cr := radius/spatialCellSize + 1
	if n.index(max(cx-cr, 0), max(cy-cr, 0)) < 0 || n.index(cx+cr, cy+cr) < 0 {
		return w.hasNearbyPlayerInGameplaySpace(x, y, radius, "") // stale mob index: scan as before
	}
	if !n.built {
		n.build(w)
	}
	for yy := cy - cr; yy <= cy+cr; yy++ {
		for xx := cx - cr; xx <= cx+cr; xx++ {
			if xx < 0 || yy < 0 {
				continue
			}
			for _, p := range n.cells[n.index(xx, yy)] {
				if p == nil || chebyshev(x, y, p.X, p.Y) > radius {
					continue
				}
				eligible, known := n.eligible[p]
				if !known {
					eligible = validMobTargetAt(p, n.now) && w.gameplaySpaceForPlayer(p) == ""
					n.eligible[p] = eligible
				}
				if eligible {
					return true
				}
			}
		}
	}
	return false
}
