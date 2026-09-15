package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

type homeCityStore struct {
	craftStore
	saved *model.Account
}

func (s *homeCityStore) SaveAccount(acc *model.Account) error {
	if err := s.craftStore.SaveAccount(acc); err != nil {
		return err
	}
	s.saved = accountStateSnapshot(acc)
	return nil
}

func TestHomeCityMovementBindsWithoutRepeatedClientNotice(t *testing.T) {
	for city, zone := range cityWarZones {
		t.Run(zone.name, func(t *testing.T) {
			w, p, _ := handlerTestWorld(t)
			st := &homeCityStore{}
			w.store = st
			p.Char.Score.Merchant = uint32((city+3)%4<<playerHomeCityShift) | 5
			p.Char.RuntimeScore.Merchant = p.Char.Score.Merchant
			p.X, p.Y = zone.city.x1-1, zone.city.y1+5
			p.Char.X, p.Char.Y = p.X, p.Y
			w.updatePlayerSpatial(p)
			now := time.Unix(100, 0)
			route := directMovementRoute(p.X, p.Y, p.X+2, p.Y)
			w.beginPlayerMovement(p, p.X, p.Y, p.X+2, p.Y, route, route, 0, now)

			// The visual client can announce entry before authority crosses the
			// boundary. It does not repeat this notice on every movement step.
			pkt := make([]byte, 16)
			binary.LittleEndian.PutUint32(pkt[12:], uint32(city))
			w.onChangeCity(p.Session, pkt)
			if st.saves != 0 || playerHomeCity(p.Char) == city {
				t.Fatal("unreached movement target bound a city")
			}
			w.advancePlayerMovement(p, now.Add(2*time.Second))
			want := uint32(city<<playerHomeCityShift) | 5
			if p.Char.Score.Merchant != want || p.Char.RuntimeScore.Merchant != want || st.saves != 1 {
				t.Fatalf("confirmed entry did not bind once: merchant=%x runtime=%x saves=%d",
					p.Char.Score.Merchant, p.Char.RuntimeScore.Merchant, st.saves)
			}
			w.onChangeCity(p.Session, pkt)
			if st.saves != 1 {
				t.Fatal("duplicate notice saved the same binding again")
			}

			// Reload the actual saved character, not the actor's live position.
			entryWorld, entryPlayer, session := newEnterWorldPlayer(t, 100, 1000)
			entryPlayer.Account = accountStateSnapshot(st.saved)
			entryWorld.onEnterWorld(session, enterWorldPacket(0))
			if !entryPlayer.InWorld || chebyshev(entryPlayer.X, entryPlayer.Y, zone.exitX, zone.exitY) > 8 {
				t.Fatalf("saved city not used at login: (%d,%d)", entryPlayer.X, entryPlayer.Y)
			}
			p.X, p.Y = 2200, 2200
			w.updatePlayerSpatial(p)
			setPlayerCurHP(p.Char, 0)
			p.DeadAt = time.Now().Add(-5 * time.Second)
			w.onRestart(p.Session)
			if playerCurHP(p.Char) == 0 || chebyshev(p.X, p.Y, zone.exitX, zone.exitY) > 8 {
				t.Fatalf("bound city not used at restart: (%d,%d)", p.X, p.Y)
			}
		})
	}
}

func TestHomeCityTeleportPersistsBindingAtomically(t *testing.T) {
	for _, fail := range []bool{false, true} {
		t.Run(map[bool]string{false: "success", true: "rollback"}[fail], func(t *testing.T) {
			w, p, _ := handlerTestWorld(t)
			st := &homeCityStore{}
			w.store = st
			p.Char.Score.Merchant, p.Char.RuntimeScore.Merchant = 0xC5, 0xC7
			oldX, oldY := p.X, p.Y
			if fail {
				st.err = errors.New("disk unavailable")
			}
			zone := cityWarZones[0]
			ok := w.teleportPlayer(p, zone.exitX, zone.exitY)
			if ok == fail || st.saves != 1 {
				t.Fatalf("teleport result=%v saves=%d", ok, st.saves)
			}
			if fail {
				if p.X != oldX || p.Y != oldY || p.Char.X != oldX || p.Char.Y != oldY ||
					p.Char.Score.Merchant != 0xC5 || p.Char.RuntimeScore.Merchant != 0xC7 || st.saved != nil {
					t.Fatal("failed teleport did not roll back position and both scores")
				}
				st.err = nil
				if !w.teleportPlayer(p, zone.exitX, zone.exitY) {
					t.Fatal("retry failed")
				}
			}
			if p.Char.Score.Merchant != 5 || p.Char.RuntimeScore.Merchant != 5 ||
				st.saved.Chars[0].Score.Merchant != 5 {
				t.Fatal("Armia binding was not included in teleport commit")
			}
			// Field teleports must retain the last visited city.
			if !w.teleportPlayer(p, 2200, 2200) || playerHomeCity(p.Char) != 0 {
				t.Fatal("field teleport lost the bound city")
			}
		})
	}
}

func TestHomeCityMovementRetryAfterSaveFailure(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Score.Merchant, p.Char.RuntimeScore.Merchant = 0x1C5, 0x1C7
	st.err = errors.New("disk unavailable")
	now := time.Unix(100, 0)
	route := []byte("66")
	w.beginPlayerMovement(p, p.X, p.Y, p.X+2, p.Y, route, route, 0, now)
	firstStep := movementStepDeadline(p, 1)
	w.advancePlayerMovement(p, firstStep)
	if p.X != 2101 || p.Char.Score.Merchant != 0x1C5 || p.Char.RuntimeScore.Merchant != 0x1C7 || st.saves != 1 {
		t.Fatal("failed binding must restore both scores without rewinding accepted movement")
	}
	st.err = nil
	w.advancePlayerMovement(p, now.Add(2*time.Second))
	if p.X != 2102 || p.Char.Score.Merchant != 0x105 || p.Char.RuntimeScore.Merchant != 0x105 || st.saves != 2 {
		t.Fatal("next confirmed step did not retry binding or damaged unrelated Merchant bits")
	}
}

func TestHomeCityUnreachedTargetDoesNotBind(t *testing.T) {
	for _, cancel := range []bool{false, true} {
		t.Run(map[bool]string{false: "occupied", true: "cancelled"}[cancel], func(t *testing.T) {
			w, p, st := handlerTestWorld(t)
			p.Char.Score.Merchant = 0xC5
			p.X, p.Y = 2051, 2057
			w.updatePlayerSpatial(p)
			now := time.Unix(100, 0)
			route := []byte("6")
			w.beginPlayerMovement(p, p.X, p.Y, p.X+1, p.Y, route, route, 0, now)
			if cancel {
				clearPublishedPlayerMove(p)
			} else {
				mob := &Mob{ID: 1000, X: 2052, Y: 2057, HP: 100,
					Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100, Merchant: 1})}
				w.appendMobInstance(mob)
				w.registerMobSpatial(mob)
			}
			w.advancePlayerMovement(p, now.Add(2*time.Second))
			w.onChangeCity(p.Session, make([]byte, 16))
			if p.X != 2051 || p.Char.Score.Merchant != 0xC5 || st.saves != 0 {
				t.Fatal("unreached city bound through cancelled/blocked movement or forged notice")
			}
		})
	}
}

func TestHomeCityCrossedDuringMovementBatchIsRetained(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Score.Merchant = 0xC5
	p.X, p.Y = 2447, 1966
	w.updatePlayerSpatial(p)
	now := time.Unix(100, 0)
	route := []byte("63") // Enter Erion, then leave its southern edge.
	w.beginPlayerMovement(p, p.X, p.Y, 2449, 1965, route, route, 0, now)
	w.advancePlayerMovement(p, now.Add(2*time.Second))
	if p.X != 2449 || p.Y != 1965 || playerHomeCity(p.Char) != 2 || st.saves != 1 {
		t.Fatal("batch ending outside town lost the city actually visited")
	}
}

func TestHomeCityQuestTeleportSharesAtomicCommit(t *testing.T) {
	for _, fail := range []bool{false, true} {
		t.Run(map[bool]string{false: "success", true: "rollback"}[fail], func(t *testing.T) {
			w := newZoneTestWorld()
			st := &atomicCharStateMemoryStore{}
			w.store = st
			p := addZonePlayer(w, 1, 2200, 2200, 500)
			p.Char.Score.Merchant = 0xC5
			p.Char.RuntimeScore = &model.Score{Merchant: 0xC7}
			quest := simpleQuest(77, "Keeper")
			quest.Rewards.Teleport = &model.QuestTeleport{X: 2086, Y: 2093}
			if fail {
				st.atomicErr = errors.New("database unavailable")
			}
			w.executeQuest(p.Session, p, &Mob{}, &quest)
			if st.atomicSaves != 1 || st.syncSaves != 0 || st.saves != 0 {
				t.Fatal("city binding must share the quest's only commit")
			}
			if fail {
				if p.X != 2200 || p.Y != 2200 || p.Char.Score.Merchant != 0xC5 || p.Char.RuntimeScore.Merchant != 0xC7 {
					t.Fatal("quest failure left a changed position or hometown")
				}
			} else if p.X != 2086 || p.Y != 2093 || p.Char.Score.Merchant != 5 || st.accountSnapshot.Chars[0].Score.Merchant != 5 {
				t.Fatal("quest teleport did not persist Armia in its own transaction")
			}
		})
	}
}
