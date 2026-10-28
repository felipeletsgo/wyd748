package game

import (
	"errors"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func recallArea(city int) warRect {
	z := homeCityZones[city]
	return warRect{z.returnX, z.returnY, z.returnX + 14, z.returnY + 14}
}

func TestRecallScrollUsesBoundCity(t *testing.T) {
	for city := 0; city < 4; city++ {
		t.Run(cityWarZones[city].name, func(t *testing.T) {
			w, p, st := handlerTestWorld(t)
			p.Char.Score.Merchant = uint32(city<<6) | 0x105
			p.Char.RuntimeScore.Merchant = p.Char.Score.Merchant
			// Even a mortal beginner returns to its city through DoRecall.
			p.Char.Score.Level = 0
			w.volatiles = model.VolatileCatalog{Rules: map[int]model.VolatileRule{11: {Action: "recall", Consume: true}},
				ItemCodes: map[uint16]int{900: 11}}
			w.items[900] = model.ItemDef{Index: 900}
			p.Char.Inv[0] = model.Item{Index: 900, UID: "return-scroll", Eff: [6]byte{61, 2}}
			before := p.Char.Score.Merchant
			w.onUseItem(p.Session, useItemPacket(0, 0))
			if !recallArea(city).contains(p.X, p.Y) || st.saves != 1 ||
				p.Char.Inv[0].Index != 900 || p.Char.Inv[0].Eff[1] != 1 ||
				p.Char.Score.Merchant != before || p.Char.RuntimeScore.Merchant != before {
				t.Fatalf("recall lost city, consumption, or hometown: position=(%d,%d) item=%+v merchant=%x saves=%d",
					p.X, p.Y, p.Char.Inv[0], p.Char.Score.Merchant, st.saves)
			}
		})
	}
}

func TestRecallGuildTerritoryPriority(t *testing.T) {
	for territory, z := range homeCityZones {
		t.Run(fmt.Sprint(territory), func(t *testing.T) {
			w, p, st := handlerTestWorld(t)
			p.Char.GuildID = 9
			p.Char.Score.Merchant, p.Char.RuntimeScore.Merchant = 0x185, 0x187
			w.guilds = &model.GuildRegistry{}
			if territory < 4 {
				w.guilds.Wars.Cities.Territories[territory].Owner = 9
			} else {
				w.guilds.Wars.Castle.Owner = 9
			}
			// A later owned territory must not override the first one.
			w.guilds.Wars.Castle.Owner = 9
			city, x, y := w.recallSpawn(p.Char)
			if city != 2 || x != z.guildReturnX || y != z.guildReturnY {
				t.Fatalf("wrong guild return: city=%d destination=(%d,%d)", city, x, y)
			}
			w.volatiles = model.VolatileCatalog{Rules: map[int]model.VolatileRule{11: {Action: "recall", Consume: true}},
				ItemCodes: map[uint16]int{900: 11}}
			w.items[900] = model.ItemDef{Index: 900}
			p.Char.Inv[0] = model.Item{Index: 900, UID: "guild-return"}
			before := accountStateSnapshot(p.Account)
			oldX, oldY := p.X, p.Y
			st.err = errors.New("save rejected")
			w.onUseItem(p.Session, useItemPacket(0, 0))
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.X != oldX || p.Y != oldY {
				t.Fatal("rejected recall changed the item, position, or hometown")
			}
			st.err = nil
			w.onUseItem(p.Session, useItemPacket(0, 0))
			if p.X != x || p.Y != y || p.Char.Inv[0].Index != 0 ||
				p.Char.Score.Merchant != 0x185 || p.Char.RuntimeScore.Merchant != 0x187 {
				t.Fatal("guild return changed the hometown or lost atomic consumption")
			}
		})
	}
}

func TestHomeCityNativeBounds(t *testing.T) {
	for _, tc := range []struct {
		x, y uint16
		city int
	}{
		{2432, 1536, 1}, {2675, 1767, 1}, {2432, 1535, -1},
		{3500, 3100, 3}, {3700, 3200, 3}, {3606, 3099, -1}, {3650, 3201, -1},
		{2448, 1966, 2}, {2476, 2024, 2}, {2447, 1966, -1},
		{2052, 2052, 0}, {2171, 2163, 0}, {2172, 2163, -1},
		{1066, 1760, -1}, // Noatum never replaces the two-bit hometown.
	} {
		city, ok := bindableCityAt(tc.x, tc.y)
		if ok != (tc.city >= 0) || ok && city != tc.city {
			t.Errorf("(%d,%d): city=%d ok=%v, want=%d", tc.x, tc.y, city, ok, tc.city)
		}
	}
}

func TestCriticalHomeCityMovementStableSnapshot(t *testing.T) {
	for city := 0; city < 4; city++ {
		for _, steps := range []int{1, 2} {
			for _, rejected := range []bool{false, true} {
				t.Run(fmt.Sprintf("%d/%d/rejected=%v", city, steps, rejected), func(t *testing.T) {
					w, p, _, _, base, q, release := criticalTreasuryFixture(t, 0)
					w.criticalAccountContainment = true
					if rejected {
						base.failure = criticalCertifiedRejection(t)
					}
					oldMerchant := uint32((city+3)%4<<6) | 0x105
					p.Char.Score.Merchant, p.Char.RuntimeScore.Merchant = oldMerchant, oldMerchant
					area := homeCityZones[city].area
					p.X, p.Y = area.x1-1, area.y1+5
					p.Char.X, p.Char.Y = p.X, p.Y
					w.updatePlayerSpatial(p)
					now := w.now()
					route := directMovementRoute(p.X, p.Y, p.X+uint16(steps), p.Y)
					w.beginPlayerMovement(p, p.X, p.Y, p.X+uint16(steps), p.Y, route, route, 0, now)
					w.advancePlayerMovement(p, now.Add(2*time.Second))
					proposal := awaitCriticalWrite(t, base.entered)
					ch := &proposal.accounts[0].Chars[p.CharSlot]
					if playerHomeCity(ch) != city || ch.X != playerEntryX || ch.Y != playerEntryY ||
						p.Char.X != p.X || p.Char.Y != p.Y || p.Char.Score.Merchant != oldMerchant {
						t.Fatal("hometown proposal did not capture the final movement state")
					}
					before := accountStateSnapshot(p.Account)
					for _, ready := range []bool{false, true} {
						if ready {
							release()
							awaitReadyCriticalResults(t, q, 1)
						}
						w.bindPlayerHomeCity(p)
						w.advancePlayerMovement(p, now.Add(3*time.Second))
						if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(base.entered) != 0 {
							t.Fatal("pending or unacknowledged write allowed a repeated mutation")
						}
					}
					w.processCriticalCompletions(1)
					want := uint32(city<<6) | 0x105
					if rejected {
						want = oldMerchant
					}
					if p.Char.Score.Merchant != want || p.Char.RuntimeScore.Merchant != want ||
						p.Session.IsClosed() || p.PersistencePoisoned || w.shuttingDown || w.criticalPlayerReserved(p) {
						t.Fatal("walking caused containment, lost rollback, or retained a reservation")
					}
				})
			}
		}
	}
}

func TestCriticalHomeCityMovementCrossedOrStopped(t *testing.T) {
	for _, stopped := range []bool{false, true} {
		t.Run(fmt.Sprint(stopped), func(t *testing.T) {
			w, p, _, _, base, q, release := criticalTreasuryFixture(t, 0)
			p.Char.Score.Merchant, p.Char.RuntimeScore.Merchant = 0xC5, 0xC5
			p.X, p.Y, p.Char.X, p.Char.Y = 2447, 1966, 2447, 1966
			w.updatePlayerSpatial(p)
			route := []byte("63")
			if stopped {
				mob := &Mob{ID: 1000, X: 2449, Y: 1965, HP: 100,
					Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100, Merchant: 1})}
				w.appendMobInstance(mob)
				w.registerMobSpatial(mob)
			}
			now := w.now()
			w.beginPlayerMovement(p, p.X, p.Y, 2449, 1965, route, route, 0, now)
			w.advancePlayerMovement(p, now.Add(2*time.Second))
			proposal := awaitCriticalWrite(t, base.entered)
			ch := &proposal.accounts[0].Chars[p.CharSlot]
			if playerHomeCity(ch) != 2 || p.Char.X != p.X || p.Char.Y != p.Y || stopped && p.X != 2448 {
				t.Fatal("crossed city or accepted movement was lost before a stopped route")
			}
			release()
			awaitReadyCriticalResults(t, q, 1)
			w.processCriticalCompletions(1)
			if playerHomeCity(p.Char) != 2 || p.PersistencePoisoned || p.Session.IsClosed() {
				t.Fatal("crossed/stopped route failed its hometown continuation")
			}
		})
	}
}
