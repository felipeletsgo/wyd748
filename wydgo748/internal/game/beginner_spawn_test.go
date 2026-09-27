package game

import (
	"testing"
	"time"
)

func TestMortalBeginnerSpawnOnLoginAndRestart(t *testing.T) {
	cases := []struct {
		name      string
		evolution string
		level     uint32
		beginner  bool
	}{
		{name: "new mortal", level: 0, beginner: true},
		{name: "mortal below limit", evolution: "mortal", level: 34, beginner: true},
		{name: "mortal at limit", evolution: "mortal", level: 35},
		{name: "arch below limit", evolution: "arch", level: 0},
		{name: "celestial below limit", evolution: "celestial", level: 0},
		{name: "subcelestial below limit", evolution: "subcelestial", level: 0},
	}

	const boundCity = 2 // Erion.
	const merchant = uint32(boundCity<<playerHomeCityShift) | 5
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			wantX, wantY := cityWarZones[boundCity].exitX, cityWarZones[boundCity].exitY
			if tc.beginner {
				wantX, wantY = mortalBeginnerSpawnX, mortalBeginnerSpawnY
			}

			entryWorld, entryPlayer, session := newEnterWorldPlayer(t, 100, 1000)
			entryChar := &entryPlayer.Account.Chars[0]
			entryChar.Score.Level = tc.level
			entryChar.Score.Merchant = merchant
			entryChar.Evolution = tc.evolution
			entryWorld.onEnterWorld(session, enterWorldPacket(0))
			if !entryPlayer.InWorld || chebyshev(entryPlayer.X, entryPlayer.Y, wantX, wantY) > 8 {
				t.Fatalf("login spawned at (%d,%d), expected near (%d,%d)", entryPlayer.X, entryPlayer.Y, wantX, wantY)
			}
			if playerHomeCity(entryPlayer.Char) != boundCity || entryPlayer.Char.Score.Merchant != merchant {
				t.Fatal("login changed the bound city")
			}

			world, player, store := handlerTestWorld(t)
			player.Char.Score.Level = tc.level
			player.Char.RuntimeScore.Level = tc.level
			player.Char.Score.Merchant = merchant
			player.Char.RuntimeScore.Merchant = merchant
			player.Char.Evolution = tc.evolution
			player.X, player.Y = 2200, 2200
			world.updatePlayerSpatial(player)
			setPlayerCurHP(player.Char, 0)
			player.DeadAt = time.Now().Add(-5 * time.Second)
			world.onRestart(player.Session)
			if playerCurHP(player.Char) == 0 || chebyshev(player.X, player.Y, wantX, wantY) > 8 || store.saves != 1 {
				t.Fatalf("restart state: HP=%d position=(%d,%d), saves=%d; expected near (%d,%d)",
					playerCurHP(player.Char), player.X, player.Y, store.saves, wantX, wantY)
			}
			if playerHomeCity(player.Char) != boundCity || player.Char.Score.Merchant != merchant ||
				player.Char.RuntimeScore.Merchant != merchant {
				t.Fatal("restart changed the bound city")
			}
		})
	}
}
