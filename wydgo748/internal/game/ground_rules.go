package game

import (
	"strings"
	"time"

	"wydgo/internal/model"
)

// Ground-item rules (DOCS/server/ground-items.md).
//
// The native 7.48 client draws a ground item at its cell + 0.5 and ignores the
// Owner field of 0x26E, so lifetime, placement and pickup rights are server
// rules. Placement follows the legacy GetEmptyItemGrid: the requested cell if
// no item occupies it, otherwise the first free cell of the 3x3 ring around
// it. Ownership is an owner decision (2026-10-07): for groundItemProtection
// only the owner and the owner's party may pick the item up; afterwards anyone
// can, until groundItemLifetime removes it.
const (
	groundItemLifetime   = 2 * time.Minute
	groundItemProtection = 60 * time.Second
)

const groundProtectedMessage = "This item belongs to another player for now."

// groundOwner identifies who may pick an item up while it is protected.
type groundOwner struct {
	uid   string
	party *Party
}

func ownerOf(p *Player) groundOwner {
	if p == nil || p.Char == nil {
		return groundOwner{}
	}
	return groundOwner{uid: p.Char.UID, party: p.Party}
}

// groundCellOccupied reports an item already on the exact cell in the same
// gameplay space. Permanent fixtures occupy their cell for every space.
func (w *World) groundCellOccupied(x, y uint16, instanceID string) bool {
	for _, item := range w.nearbyGroundItems(x, y, 0) {
		if item.X != x || item.Y != y {
			continue
		}
		if item.Permanent || strings.TrimSpace(item.InstanceID) == instanceID {
			return true
		}
	}
	return false
}

// emptyGroundCell ports the legacy GetEmptyItemGrid: the cell itself when it
// is free, else the first free walkable cell of the surrounding 3x3 in row
// order. ok is false when every candidate is taken.
func (w *World) emptyGroundCell(x, y uint16, instanceID string) (uint16, uint16, bool) {
	instanceID = strings.TrimSpace(instanceID)
	if !w.groundCellOccupied(x, y, instanceID) {
		return x, y, true
	}
	for yy := int(y) - 1; yy <= int(y)+1; yy++ {
		for xx := int(x) - 1; xx <= int(x)+1; xx++ {
			if xx < 1 || yy < 1 || xx >= model.TerrainWidth || yy >= model.TerrainHeight {
				continue
			}
			cx, cy := uint16(xx), uint16(yy)
			if w.terrain.Walkable(cx, cy) && !w.groundCellOccupied(cx, cy, instanceID) {
				return cx, cy, true
			}
		}
	}
	return 0, 0, false
}

// groundItemClaimable applies the protection window. Permanent fixtures are
// rejected elsewhere; items without an owner are public.
func groundItemClaimable(p *Player, g *GroundItem, now time.Time) bool {
	if g == nil || g.OwnerUID == "" || !now.Before(g.ProtectedUntil) {
		return true
	}
	if p == nil || p.Char == nil {
		return false
	}
	if p.Char.UID == g.OwnerUID {
		return true
	}
	return g.ownerParty != nil && p.Party == g.ownerParty
}
