package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestOXUnauthorizedMovementTeleportAndIllusion(t *testing.T) {
	for _, active := range []bool{false, true} {
		t.Run(map[bool]string{false: "inactive", true: "enrollment"}[active], func(t *testing.T) {
			w, clock := colosseumWorld(t, time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC))
			if active {
				runOXEvent(w, "start", true)
			}
			p := oxTestPlayer(w, 1, 1310, 1480)
			if _, _, ok := w.resolvePlayerTeleportDestination(p, 1310, 1482); ok {
				t.Fatal("public teleport admitted a player without a ticket")
			}
			// A route can cross the arena even with an endpoint outside it.
			w.beginPlayerMovement(p, p.X, p.Y, 1310, 1519, []byte("888"), []byte("888"), 0, clock.Now())
			clock.Advance(time.Second)
			w.advancePlayerMovement(p, clock.Now())
			if p.X != 1310 || p.Y != 1480 || p.MovePublished {
				t.Fatal("walking crossed the authoritative arena boundary")
			}
			p.Char.Class, p.Char.LearnedSkill = 3, 1<<1
			setPlayerCurMP(p.Char, 800)
			w.skills = map[int]model.SkillDef{73: {Index: 73, ManaSpent: 10, Range: 8}}
			packet := make([]byte, 28)
			binary.LittleEndian.PutUint16(packet[24:26], 1310)
			binary.LittleEndian.PutUint16(packet[26:28], 1482)
			mp := playerCurMP(p.Char)
			w.onIllusionMove(p, packet)
			if p.Y != 1480 || playerCurMP(p.Char) != mp || !p.SkillReady[73].IsZero() {
				t.Fatal("rejected skill jump changed position, mana or cooldown")
			}
			// A forged/stale occupancy is also evicted by the world loop.
			placeOXTest(w, p, 1310, 1500)
			w.tickOXEvent(clock.Now())
			if oxArena.contains(p.X, p.Y) || !oxNoticeContains(p, "Entrance Ticket") {
				t.Fatal("unregistered arena occupant was not evicted")
			}
		})
	}
}

func TestOXDeadlineFreezesAnswerPositionAndPrivateInstancesAreIndependent(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC))
	runOXEvent(w, "start", true)
	p := oxTestPlayer(w, 1, 1340, 1460)
	enterOXTest(t, w, p)
	placeOXTest(w, p, 1338, 1500)
	clock.Advance(time.Minute)
	w.tickOXEvent(clock.Now())
	clock.Advance(9900 * time.Millisecond)
	w.beginPlayerMovement(p, p.X, p.Y, 1342, 1500, []byte("6666"), []byte("6666"), 0, clock.Now())
	clock.Advance(time.Second)
	// Even a delayed world tick must not promote steps due after the deadline.
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 1338 {
		t.Fatal("post-deadline walking changed the answer")
	}
	if _, _, ok := w.resolvePlayerTeleportDestination(p, 1366, 1500); ok {
		t.Fatal("post-deadline teleport changed the answer")
	}
	w.beginPlayerMovement(p, p.X, p.Y, 1342, 1500, []byte("6666"), []byte("6666"), 0, clock.Now())
	if p.MovePublished {
		t.Fatal("expired question published a new movement plan")
	}
	// Private Cube rooms retain their own admission and movement rules even
	// when their coordinates overlap the public arena.
	w.itemInstances = map[string]*ItemInstance{"private-test": privateTestInstance("private-test", p.ID)}
	w.rebuildPlayerInstanceIndex()
	if !w.oxStepAllowed(p, 1310, 1500) || !w.oxDestinationAllowed(p, 1310, 1500) || w.isOXMember(p) {
		t.Fatal("public OX authority leaked into a private instance")
	}
}

func TestOXExitCannotUseArenaWhenLobbyIsFull(t *testing.T) {
	w, _ := colosseumWorld(t, time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC))
	p := oxTestPlayer(w, 1, 1310, 1500)
	blocker := oxTestPlayer(w, 2, 1340, 1460)
	w.criticalTeleportTiles = make(map[criticalTeleportTile]*Player)
	for dy := -32; dy <= 32; dy++ {
		for dx := -32; dx <= 32; dx++ {
			x, y := uint16(oxExitX+dx), uint16(oxExitY+dy)
			if !oxArena.contains(x, y) {
				w.criticalTeleportTiles[criticalTile(x, y, "")] = blocker
			}
		}
	}
	if _, _, ok := w.findOXExit(p); ok {
		t.Fatal("full lobby selected an exit inside the arena")
	}
	delete(w.criticalTeleportTiles, criticalTile(oxExitX, oxExitY, ""))
	x, y, ok := w.findOXExit(p)
	if !ok || x != oxExitX || y != oxExitY {
		t.Fatal("released lobby tile was not available for departure")
	}
}

func TestOXMembershipCannotSurviveCharacterReplacementOrExit(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC))
	runOXEvent(w, "start", true)
	p := oxTestPlayer(w, 1, 1340, 1460)
	enterOXTest(t, w, p)
	replacement := cloneCharacterState(p.Char)
	p.Char = &replacement
	if w.isOXMember(p) || w.oxStepAllowed(p, 1310, 1500) {
		t.Fatal("replacement character inherited the paid membership")
	}
	w.tickOXEvent(clock.Now())
	if oxArena.contains(p.X, p.Y) || len(w.ox.members) != 0 {
		t.Fatal("invalid membership was retained")
	}
	// A genuine paid member that leaves cannot return on the old admission.
	p.Char = &p.Account.Chars[0]
	placeOXTest(w, p, 1340, 1460)
	enterOXTest(t, w, p)
	placeOXTest(w, p, 1340, 1460)
	w.tickOXEvent(clock.Now())
	if w.isOXMember(p) || w.oxStepAllowed(p, 1310, 1500) {
		t.Fatal("departure retained an arena admission")
	}
}
