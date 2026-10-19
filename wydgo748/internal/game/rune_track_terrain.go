package game

import (
	"fmt"

	"wydgo/internal/wire"
)

var runeTrackPortals = [10][4]uint16{
	{3429, 1197, 3432, 1200}, {3379, 1178, 3381, 1180}, {3379, 1166, 3381, 1168},
	{3362, 1223, 3364, 1225}, {3351, 1223, 3354, 1225}, {3340, 1223, 3342, 1225},
	{3402, 1237, 3404, 1239}, {3394, 1237, 3396, 1239}, {3385, 1237, 3388, 1239}, {3378, 1237, 3380, 1239},
}

// Decoded from the 7.59 executable's table at VA 0x646FA8. These are inclusive
// bounds, not a rectangular approximation of the entire maze.
var runeTrackFloorTiles = func() [][4]uint16 {
	tiles := [][4]uint16{{3436, 1302, 3450, 1317}, {3436, 1335, 3450, 1351}, {3436, 1370, 3450, 1385}}
	y := [][2]uint16{{1284, 1300}, {1300, 1316}, {1316, 1335}, {1335, 1352}, {1352, 1368}, {1368, 1384}, {1384, 1400}}
	for _, x := range [][2]uint16{{3418, 3436}, {3385, 3418}, {3368, 3385}} {
		for _, row := range y {
			tiles = append(tiles, [4]uint16{x[0], row[0], x[1], row[1]})
		}
	}
	for _, x := range [][2]uint16{{3349, 3368}, {3332, 3349}} {
		for _, row := range [][2]uint16{{1284, 1300}, {1300, 1316}, {1352, 1368}, {1368, 1384}, {1384, 1400}} {
			tiles = append(tiles, [4]uint16{x[0], row[0], x[1], row[1]})
		}
	}
	return tiles
}()

func fmtRuneProgress(current, target int) string {
	return fmt.Sprintf("Rune Track: %d / %d", current, target)
}

func (w *World) runePortalDestination(portal int) [2]uint16 {
	switch {
	case portal == 0:
		if w.intn(2) == 0 {
			return [2]uint16{3428, 1208}
		}
		return [2]uint16{3388, 1175}
	case portal <= 2:
		if w.intn(4) == 0 {
			return [2]uint16{3356, 1209}
		}
		return [2]uint16{3388, 1175}
	case portal <= 5:
		switch w.intn(7) {
		case 0:
			return [2]uint16{3390, 1250}
		case 6:
			return [2]uint16{3428, 1208}
		}
		return [2]uint16{3356, 1209}
	default:
		switch w.intn(10) {
		case 0:
			return [2]uint16{3396, 1216}
		case 3:
			return [2]uint16{3428, 1208}
		case 8:
			return [2]uint16{3388, 1175}
		}
		return [2]uint16{3390, 1250}
	}
}

func (w *World) tickRunePortals(inst *ItemInstance) {
	for i, group := range inst.RuneTrack.Parties {
		leader := w.runeTrackPlayer(group.Leader)
		if leader == nil || w.runeTrackPartyFor(inst, leader) != i || playerCurHP(leader.Char) == 0 {
			continue
		}
		for portal, rect := range runeTrackPortals {
			if !runeInside(leader.X, leader.Y, rect) {
				continue
			}
			powder := -1
			for slot := 0; slot < 60; slot++ {
				if leader.Char.Inv[slot].Index == 4032 {
					powder = slot
				}
			}
			if powder >= 0 {
				w.moveRuneParty(inst, i, w.runePortalDestination(portal), powder)
			}
			// One durable move per tick; subsequent parties retain their powder
			// and are processed on the next portal pulse.
			if w.criticalInstanceReserved() {
				return
			}
			break
		}
	}
}

func (w *World) runeTrackFloorDamage(inst *ItemInstance) {
	for _, rect := range runeTrackFloorTiles {
		protected := false
		for i := range inst.RuneTrack.Parties {
			group := &inst.RuneTrack.Parties[i]
			leader := w.runeTrackPlayer(group.Leader)
			if leader != nil && w.runeTrackPartyFor(inst, leader) == i && runeInside(leader.X, leader.Y, rect) {
				protected = true
				break
			}
		}
		if protected {
			continue
		}
		centerX, centerY := (rect[0]+rect[2])/2, (rect[1]+rect[3])/2
		for i := range inst.RuneTrack.Parties {
			for _, p := range w.runeTrackMembers(inst, &inst.RuneTrack.Parties[i]) {
				if p.Session != nil && inView(p.X, p.Y, centerX, centerY) {
					p.Session.Send(wire.EnvironmentEffect(rect, 32))
				}
				if !runeInside(p.X, p.Y, rect) || playerCurHP(p.Char) == 0 || w.criticalPlayerReserved(p) {
					continue
				}
				// Preserve the historical nonlethal 1,000-HP floor, including its
				// recovery below that floor, bounded by the actual character cap.
				old := playerCurHP(p.Char)
				next := int64(old) - int64(playerMaxHP(p.Char)/4)
				setPlayerCurHP(p.Char, uint32(max(int64(1000), next)))
				w.sendToPlayerView(p, func() []byte { return wire.SetHpMp(p.ID, effectiveScore(p.Char)) })
			}
		}
	}
}
