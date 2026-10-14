package game

import (
	"testing"

	netpkg "wydgo/internal/net"
	"wydgo/internal/model"
)

// W2PP GetEnemyFromView: a kingdom guard skips players of its own kingdom
// when choosing a target, but still targets the other kingdom.
func TestKingdomGuardDoesNotAggroOwnKingdom(t *testing.T) {
	s1, blue := livingPlayer(1, 102, 100)
	blue.Char.Equip[model.CapeSlot] = model.Item{Index: 543} // Hekalotia
	s2, red := livingPlayer(2, 105, 100)
	red.Char.Equip[model.CapeSlot] = model.Item{Index: 544} // Akelonia
	w := testSpatialWorld(nil, blue, red)
	w.players = map[*netpkg.Session]*Player{s1: blue, s2: red}

	guard := &Mob{ID: 2000, X: 100, Y: 100, Def: &model.NPCDef{}}
	guard.Def.Equip.Capa.Index = 734 // Hekalotia (blue) guard
	if !sameKingdomGuard(guard, blue) || sameKingdomGuard(guard, red) {
		t.Fatal("blue guard must treat only blue capes as its own kingdom")
	}
	accept := func(p *Player) bool { return !sameKingdomGuard(guard, p) }
	if got := w.nearestPlayerMatching(100, 100, 8, "", accept); got != red {
		t.Fatalf("blue guard targeted %v, want the red-cape player", got)
	}
	red.Char.Equip[model.CapeSlot] = model.Item{Index: 543}
	if got := w.nearestPlayerMatching(100, 100, 8, "", accept); got != nil {
		t.Fatalf("blue guard targeted a blue-cape player: id=%d", got.ID)
	}
	guard.Def.Equip.Capa.Index = 0 // a neutral monster attacks everyone
	if got := w.nearestPlayerMatching(100, 100, 8, "", func(p *Player) bool { return !sameKingdomGuard(guard, p) }); got != blue {
		t.Fatalf("neutral mob targeted %v, want the nearest player", got)
	}
}
