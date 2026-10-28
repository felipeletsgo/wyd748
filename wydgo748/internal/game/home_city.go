package game

import (
	"math/rand/v2"

	"wydgo/internal/model"
)

// MODERNIZACAO_COMPATIVEL: preserve the existing wire and Merchant bits.
// TMSRVIN 7.59 (0x63A060) and native client 7.48 (0x5BE158) contain these
// same village bounds and return coordinates. Keep war and login policy separate.
var homeCityZones = [...]struct {
	area                       warRect
	returnX, returnY           uint16
	guildReturnX, guildReturnY uint16
}{
	{warRect{2052, 2052, 2171, 2163}, 2086, 2093, 2088, 2148},
	{warRect{2432, 1536, 2675, 1767}, 2500, 1707, 2540, 1700},
	{warRect{2448, 1966, 2476, 2024}, 2453, 2000, 2460, 1976},
	{warRect{3500, 3100, 3700, 3200}, 3650, 3118, 3615, 3122},
	{warRect{1036, 1700, 1075, 1775}, 1050, 1706, 1066, 1760},
}

// recallSpawn follows DoRecall (0x572AE0): randomize the bound city's return
// within 15x15 tiles, then prefer the first territory owned by the player's
// guild. Noatum can override a return but cannot fit in the two hometown bits.
func (w *World) recallSpawn(ch *model.Char) (int, uint16, uint16) {
	city := playerHomeCity(ch)
	zone := homeCityZones[city]
	x, y := zone.returnX+uint16(rand.IntN(15)), zone.returnY+uint16(rand.IntN(15))
	if ch != nil && ch.GuildID != 0 && w.guilds != nil {
		for i, zone := range homeCityZones {
			owner := w.guilds.Wars.Castle.Owner
			if i < len(w.guilds.Wars.Cities.Territories) {
				owner = w.guilds.Wars.Cities.Territories[i].Owner
			}
			if owner == ch.GuildID {
				return city, zone.guildReturnX, zone.guildReturnY
			}
		}
	}
	return city, x, y
}

func stageHomeCity(ch *model.Char, city int) {
	merchant := (ch.Score.Merchant &^ playerHomeCityMask) | uint32(city<<playerHomeCityShift)
	ch.Score.Merchant = merchant
	if ch.RuntimeScore != nil {
		ch.RuntimeScore.Merchant = merchant
	}
}
