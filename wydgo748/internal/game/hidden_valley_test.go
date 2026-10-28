package game

import (
	"encoding/json"
	"errors"
	"os"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func hiddenValleyPortal(t *testing.T) model.Teleport {
	t.Helper()
	routes, err := data.LoadTeleports("../../data/world/teleports.ini")
	if err != nil {
		t.Fatal(err)
	}
	portal, ok := teleportAt(routes, 2548, 1740)
	if !ok || portal.Price != 1000 || !inHiddenValley(portal.DestX, portal.DestY) {
		t.Fatalf("missing or invalid Hidden Valley route: %+v", portal)
	}
	return portal
}

func TestHiddenValleyPortalRequiresActivatedMap(t *testing.T) {
	for _, access := range []string{"absent", "carried", "fairy", "expired", "deadline", "active", "active-save-failure"} {
		t.Run(access, func(t *testing.T) {
			w, p, st := handlerTestWorld(t)
			w.clock = newFakeClock(time.Unix(2_000_000_000, 0))
			portal := hiddenValleyPortal(t)
			w.teleports = []model.Teleport{portal}
			p.X, p.Y, p.Char.X, p.Char.Y = 2548, 1740, 2548, 1740
			p.Char.Gold = 2000
			p.SpecialCoins = map[string]uint32{}
			switch access {
			case "carried":
				p.Char.Inv[0] = model.Item{Index: 3909}
			case "fairy":
				p.Char.Equip[13] = model.Item{Index: 3916}
			case "expired":
				p.SpecialCoins[hiddenValleyAccessCounter] = uint32(w.now().Unix() - 1)
			case "deadline":
				p.SpecialCoins[hiddenValleyAccessCounter] = uint32(w.now().Unix())
			case "active", "active-save-failure":
				p.SpecialCoins[hiddenValleyAccessCounter] = uint32(w.now().Unix() + 60)
			}
			if access == "active-save-failure" {
				st.err = errors.New("storage unavailable")
			}
			before := *p.Char
			w.onReqTeleport(p.Session, make([]byte, 16))
			if access == "active" {
				if !inHiddenValley(p.X, p.Y) || p.Char.Gold != 1000 || st.saves != 1 {
					t.Fatalf("authorized entry failed: position=(%d,%d), gold=%d, saves=%d", p.X, p.Y, p.Char.Gold, st.saves)
				}
			} else if p.X != 2548 || p.Y != 1740 || p.Char.Gold != before.Gold || p.Char.Inv != before.Inv {
				t.Fatal("denied/failed entry changed position, gold or inventory")
			} else if access != "active-save-failure" && st.saves != 0 {
				t.Fatal("denied entry submitted a write")
			}
		})
	}
}

func TestHiddenValleyMapActivationAndRelogExpiry(t *testing.T) {
	contents, err := os.ReadFile("../../data/content/volatiles.json")
	if err != nil {
		t.Fatal(err)
	}
	var configured struct {
		Items map[uint16]model.VolatileRule `json:"items"`
	}
	if err := json.Unmarshal(contents, &configured); err != nil {
		t.Fatal(err)
	}
	for id, duration := range map[uint16]int{3909: 86400, 3910: 604800, 3974: 28800} {
		t.Run(configured.Items[id].AccessKey+"-"+time.Duration(duration*int(time.Second)).String(), func(t *testing.T) {
			rule := configured.Items[id]
			if rule.Action != "timed_access" || rule.AccessKey != "hidden_ruins" || int(rule.DurationSeconds) != duration || !rule.Consume {
				t.Fatalf("invalid map %d rule: %+v", id, rule)
			}
			w, p, _ := useItemWorld(rule)
			st := &atomicCharStateMemoryStore{}
			w.store = st
			clock := newFakeClock(time.Unix(2_000_000_000, 0))
			w.clock = clock
			w.onUseItem(p.Session, useItemPacket(0, 0))
			deadline := clock.Now().Add(time.Duration(duration) * time.Second)
			if p.Char.Inv[0].Index != 0 || st.atomicSaves != 1 || st.state.SpecialCoins[hiddenValleyAccessCounter] != uint32(deadline.Unix()) {
				t.Fatal("activation did not atomically consume one map and save its absolute expiry")
			}
			p.Char.Inv[0] = model.Item{Index: 100}
			w.onUseItem(p.Session, useItemPacket(0, 0))
			if st.atomicSaves != 1 || p.Char.Inv[0].Index != 100 {
				t.Fatal("duplicate activation consumed another map or extended access")
			}
			fresh, _ := networkedTestPlayer(2, "Restored", 2100, 2100)
			clock.Advance(time.Hour)
			w.applyCharState(fresh, st.state, clock.Now())
			if !hiddenValleyAccessActive(fresh, clock.Now()) || fresh.SpecialCoins[hiddenValleyAccessCounter] != uint32(deadline.Unix()) {
				t.Fatal("relogin lost or extended an unexpired access")
			}
			if hiddenValleyAccessActive(fresh, deadline) || hiddenValleyAccessActive(fresh, deadline.Add(1<<32*time.Second)) {
				t.Fatal("deadline equality or Unix wrapping resurrected access")
			}
		})
	}
}

func TestHiddenValleyAlternativeTeleportMovementAndExpiryGuard(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(2_000_000_000, 0))
	w.clock = clock
	if _, _, ok := w.resolvePlayerTeleportDestination(p, 2281, 3688); ok {
		t.Fatal("an alternative teleport bypassed map activation")
	}
	if _, _, ok := w.resolvePlayerTeleportDestination(p, 2100, 2100); !ok {
		t.Fatal("ordinary teleports were blocked")
	}
	p.X, p.Y, p.Char.X, p.Char.Y = 2190, 3650, 2190, 3650
	w.beginPlayerMovement(p, p.X, p.Y, p.X+1, p.Y, []byte("6"), []byte("6"), 0, clock.Now())
	w.advancePlayerMovement(p, clock.Now().Add(time.Second))
	if p.X != 2190 || p.MovePublished {
		t.Fatal("walking bypassed map activation")
	}
	p.SpecialCoins = map[string]uint32{hiddenValleyAccessCounter: uint32(clock.Now().Unix() + 60)}
	if !w.teleportPlayer(p, 2281, 3688) {
		t.Fatal("active access rejected a teleport")
	}
	w.tickHiddenValleyAccess(clock.Now())
	if !inHiddenValley(p.X, p.Y) {
		t.Fatal("guard recalled an authorized player")
	}
	clock.Advance(time.Minute)
	w.tickHiddenValleyAccess(clock.Now())
	if inHiddenValley(p.X, p.Y) || p.Char.X != p.X || p.Char.Y != p.Y {
		t.Fatal("expired access did not recall the player to a synchronized home position")
	}
}

func TestHiddenValleyCriticalActivationSettlement(t *testing.T) {
	for _, rejected := range []bool{false, true} {
		t.Run(map[bool]string{false: "commit", true: "rejection"}[rejected], func(t *testing.T) {
			w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
			w.warConfig.Enabled = false
			rule, packet := configureCriticalConsumableSettlement(w, p, "timed_access")
			rule.AccessKey = "hidden_ruins"
			w.volatiles.Items[900] = rule
			if rejected {
				base.failure = criticalCertifiedRejection(t)
			}
			w.onUseItem(p.Session, packet)
			proposal := awaitCriticalWrite(t, base.entered)
			if hiddenValleyAccessActive(p, w.now()) || p.Char.Inv[0].Index != 900 || proposal.character == nil ||
				proposal.character.state.SpecialCoins[hiddenValleyAccessCounter] == 0 {
				t.Fatal("activation escaped settlement or omitted its atomic sidecar")
			}
			w.onUseItem(p.Session, packet)
			if len(base.entered) != 0 {
				t.Fatal("a repeated packet submitted a second activation")
			}
			release()
			awaitReadyCriticalResults(t, q, 1)
			w.processCriticalCompletions(1)
			if hiddenValleyAccessActive(p, w.now()) == rejected || (p.Char.Inv[0].Index == 0) == rejected || fallback.saves != 0 {
				t.Fatal("activation commit/rejection violated consumption, access or the S2 boundary")
			}
		})
	}
}

func TestHiddenValleyCriticalPortalCrossesExpiryWithoutContainment(t *testing.T) {
	w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
	w.warConfig.Enabled = false
	portal := hiddenValleyPortal(t)
	w.teleports = []model.Teleport{portal}
	p.X, p.Y, p.Char.X, p.Char.Y = 2548, 1740, 2548, 1740
	p.Char.Gold = 2000
	p.SpecialCoins = map[string]uint32{hiddenValleyAccessCounter: uint32(w.now().Unix() + 1)}
	w.onReqTeleport(p.Session, make([]byte, 16))
	proposal := awaitCriticalWrite(t, base.entered)
	// Persisted coordinates intentionally use the neutral login position; the
	// acknowledged callback owns the live destination, not the stored X/Y.
	if proposal.accounts[0].Chars[p.CharSlot].Gold != 1000 || p.Char.Gold != 2000 || p.X != 2548 {
		t.Fatal("portal did not retain live state and propose its fee until commit")
	}
	w.clock.(*fakeClock).Advance(2 * time.Second)
	w.tickHiddenValleyAccess(w.now())
	if len(base.entered) != 0 {
		t.Fatal("guard submitted a concurrent recall during portal settlement")
	}
	release()
	awaitReadyCriticalResults(t, q, 1)
	w.processCriticalCompletions(1)
	if !inHiddenValley(p.X, p.Y) || p.Char.Gold != 1000 || w.criticalPlayerReserved(p) || p.Session.IsClosed() {
		t.Fatal("expiry crossing incorrectly rejected or contained a committed teleport")
	}
	w.tickHiddenValleyAccess(w.now())
	awaitCriticalWrite(t, base.entered)
	awaitReadyCriticalResults(t, q, 1)
	w.processCriticalCompletions(1)
	if inHiddenValley(p.X, p.Y) || p.Char.Gold != 1000 || w.criticalPlayerReserved(p) || fallback.saves != 0 {
		t.Fatal("expired access did not settle a safe S2 recall")
	}
}

func TestHiddenValleyInstalledTerrainBoundary(t *testing.T) {
	terrain, err := data.LoadTerrain("../../data/world/maps/HeightMap.dat", "../../data/world/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	portal := hiddenValleyPortal(t)
	if !terrain.Walkable(portal.DestX, portal.DestY) {
		t.Fatal("Hidden Valley destination is blocked in the installed terrain")
	}
	type point struct{ x, y uint16 }
	start := point{portal.DestX, portal.DestY}
	queue, seen := []point{start}, map[point]bool{start: true}
	for head := 0; head < len(queue); head++ {
		current := queue[head]
		if !inHiddenValley(current.x, current.y) {
			t.Fatalf("the installed connected valley escaped its access boundary at %+v", current)
		}
		for _, direction := range routeDirections {
			x, y := int(current.x)+direction[0], int(current.y)+direction[1]
			if x <= 0 || y <= 0 || x >= model.TerrainWidth || y >= model.TerrainHeight {
				continue
			}
			next := point{uint16(x), uint16(y)}
			if !seen[next] && terrain.RouteHeightCompatible(current.x, current.y, next.x, next.y) {
				seen[next] = true
				queue = append(queue, next)
			}
		}
	}
	if len(seen) < 1000 {
		t.Fatalf("destination is not a substantial connected area: %d cells", len(seen))
	}
	generators, err := data.LoadChannelNPCGener("../../data/spawns/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	count := 0
	for _, generator := range generators {
		if generator.Enabled && inHiddenValley(generator.Segments[0].X, generator.Segments[0].Y) {
			count++
		}
	}
	t.Logf("installed connected terrain: %d cells; enabled generators starting inside: %d", len(seen), count)
}
