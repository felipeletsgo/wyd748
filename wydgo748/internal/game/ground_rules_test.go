package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestGroundItemProtectionOwnerPartyAndExpiry(t *testing.T) {
	w, owner, _ := handlerTestWorld(t)
	w.clock = newFakeClock(time.Unix(500, 0))
	w.items[400] = model.ItemDef{Index: 400}
	member, _ := networkedTestPlayer(2, "Member", 2100, 2101)
	stranger, _ := networkedTestPlayer(3, "Stranger", 2101, 2100)
	party := &Party{Members: []*Player{owner, member}}
	owner.Party, member.Party = party, party

	g := w.createGroundDropCell(owner.X, owner.Y, model.Item{Index: 400}, false, "", ownerOf(owner))
	if g == nil || g.OwnerUID != owner.Char.UID || g.ownerParty != party {
		t.Fatal("an owned drop must record its owner and party")
	}
	now := w.now()
	if !g.Expire.Equal(now.Add(groundItemLifetime)) || !g.ProtectedUntil.Equal(now.Add(groundItemProtection)) {
		t.Fatalf("drop timing: expire=%v protected=%v", g.Expire.Sub(now), g.ProtectedUntil.Sub(now))
	}
	if !groundItemClaimable(owner, g, now) || !groundItemClaimable(member, g, now) {
		t.Fatal("the owner and the owner's party must be able to pick up")
	}
	if groundItemClaimable(stranger, g, now) {
		t.Fatal("another player picked up a protected item")
	}
	member.Party = nil
	if groundItemClaimable(member, g, now) {
		t.Fatal("a player who left the party kept the protection right")
	}
	if !groundItemClaimable(stranger, g, now.Add(groundItemProtection)) {
		t.Fatal("the item must become public when the protection ends")
	}
	public := w.createGroundDropForInstance(owner.X+5, owner.Y, model.Item{Index: 400}, false, "")
	if public == nil || public.OwnerUID != "" || !groundItemClaimable(stranger, public, now) {
		t.Fatal("an unowned drop must be public immediately")
	}
}

func TestGroundItemsTakeTheFirstFreeCellAroundTheOrigin(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.items[400] = model.ItemDef{Index: 400}
	first := w.createGroundDropCell(p.X, p.Y, model.Item{Index: 400}, false, "", groundOwner{})
	if first == nil || first.X != p.X || first.Y != p.Y {
		t.Fatal("a free origin cell must be used as is")
	}
	second := w.createGroundDropCell(p.X, p.Y, model.Item{Index: 400}, false, "", groundOwner{})
	if second == nil || (second.X == p.X && second.Y == p.Y) ||
		chebyshev(p.X, p.Y, second.X, second.Y) != 1 {
		t.Fatal("an occupied origin must move to the surrounding 3x3, like GetEmptyItemGrid")
	}
	// The legacy search scans rows top to bottom, left to right.
	if second.X != p.X-1 || second.Y != p.Y-1 {
		t.Fatalf("first free cell (%d,%d), want (%d,%d)", second.X, second.Y, p.X-1, p.Y-1)
	}
	other := w.createGroundDropCell(p.X, p.Y, model.Item{Index: 400}, false, "instance-a", groundOwner{})
	if other == nil || other.X != p.X || other.Y != p.Y {
		t.Fatal("items in another gameplay space must not occupy the cell")
	}
	for range 8 {
		w.createGroundDropCell(p.X, p.Y, model.Item{Index: 400}, false, "", groundOwner{})
	}
	if w.createGroundDropCell(p.X, p.Y, model.Item{Index: 400}, false, "", groundOwner{}) != nil {
		t.Fatal("a full 3x3 must refuse the drop instead of stacking items")
	}
}
