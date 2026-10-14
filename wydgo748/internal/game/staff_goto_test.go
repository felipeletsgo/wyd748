package game

import (
	"fmt"
	"path/filepath"
	"testing"

	"wydgo/internal/control"
	"wydgo/internal/data"
)

func gotoTestWorld(t *testing.T, authorized bool) (*World, *Player) {
	t.Helper()
	staff, _ := networkedTestPlayer(1, "Staff", 2100, 2100)
	w := worldWithNetworkedPlayers(staff)
	w.store = &craftStore{}
	w.commandAuthorizer = func(account, capability string) bool {
		return authorized && account == "staffaccount" && capability == control.GotoCapability
	}
	return w, staff
}

func TestStaffGotoTeleportsAuthorizedAccount(t *testing.T) {
	w, p := gotoTestWorld(t, true)
	w.onMessageWhisper(p.Session, whisperPacket("goto", "2300 2200"))
	if p.X != 2300 || p.Y != 2200 {
		t.Fatalf("whisper /goto left the player at (%d,%d)", p.X, p.Y)
	}
	w.onMessageChat(p.Session, chatPacket("/goto 2310,2210"))
	if p.X != 2310 || p.Y != 2210 {
		t.Fatalf("chat /goto left the player at (%d,%d)", p.X, p.Y)
	}
	for _, arg := range []string{"", "2100", "0 2100", "4096 2100", "x y", "1 2 3"} {
		w.onMessageWhisper(p.Session, whisperPacket("goto", arg))
		if p.X != 2310 || p.Y != 2210 {
			t.Fatalf("invalid /goto %q moved the player to (%d,%d)", arg, p.X, p.Y)
		}
	}
}

func TestStaffGotoRequiresCapabilityAndPublicMap(t *testing.T) {
	w, p := gotoTestWorld(t, false)
	if w.staffGoto(p.Session, p, "2300 2200") || p.X != 2100 {
		t.Fatal("/goto ran for an account without the capability")
	}
	w.commandAuthorizer = nil
	if w.staffGoto(p.Session, p, "2300 2200") || p.X != 2100 {
		t.Fatal("/goto ran without a staff authorizer")
	}

	w, p = gotoTestWorld(t, true)
	w.itemInstances = map[string]*ItemInstance{"room": {RuntimeID: "room", MemberIDs: []uint16{p.ID}}}
	w.setPlayerInstanceIndex(p.ID, "room")
	if !w.staffGoto(p.Session, p, "2300 2200") || p.X != 2100 {
		t.Fatal("/goto must refuse, not bypass, an instance")
	}
}

func TestStaffGotoBlockedTargetUsesNearestWalkableTile(t *testing.T) {
	root := filepath.Join("..", "..", "data", "maps")
	terrain, err := data.LoadTerrain(filepath.Join(root, "HeightMap.dat"), filepath.Join(root, "AttributeMap.dat"))
	if err != nil {
		t.Skipf("installed maps unavailable: %v", err)
	}
	w, p := gotoTestWorld(t, true)
	w.terrain = terrain
	var bx, by uint16
	for y := uint16(2000); y < 2200 && bx == 0; y++ {
		for x := uint16(2000); x < 2200; x++ {
			if !terrain.Walkable(x, y) && terrain.Walkable(x+1, y) {
				bx, by = x, y
				break
			}
		}
	}
	if bx == 0 {
		t.Fatal("no blocked tile next to a walkable one near Armia")
	}
	if !w.staffGoto(p.Session, p, formatGoto(bx, by)) {
		t.Fatal("/goto was not consumed")
	}
	if !terrain.Walkable(p.X, p.Y) || chebyshev(p.X, p.Y, bx, by) > gotoSearchRadius+3 {
		t.Fatalf("blocked target (%d,%d) moved the player to (%d,%d)", bx, by, p.X, p.Y)
	}
}

func formatGoto(x, y uint16) string {
	return fmt.Sprintf("%d %d", x, y)
}

// The KR attribute map split the city-war arena with an invisible strip at
// y 188..191. The native 7.48 map left the central corridor open; only that
// interior corridor is reopened, while the arena's outer walls stay blocked.
func TestCityWarArenaCentralCorridorIsOpen(t *testing.T) {
	root := filepath.Join("..", "..", "data", "maps")
	terrain, err := data.LoadTerrain(filepath.Join(root, "HeightMap.dat"), filepath.Join(root, "AttributeMap.dat"))
	if err != nil {
		t.Skipf("installed maps unavailable: %v", err)
	}
	for _, x := range []uint16{140, 160, 219, 230, 240} {
		for y := uint16(187); y < 192; y++ {
			if !terrain.RouteHeightCompatible(x, y+1, x, y) {
				t.Fatalf("corridor step (%d,%d)->(%d,%d) is blocked", x, y+1, x, y)
			}
		}
	}
	for _, x := range []uint16{128, 132, 252} {
		if terrain.Walkable(x, 189) {
			t.Fatalf("arena wall at (%d,189) was opened", x)
		}
	}
}
