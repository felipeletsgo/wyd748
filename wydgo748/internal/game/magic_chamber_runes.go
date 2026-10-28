package game

import (
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Magic Chamber rune slates. The native 7.48 InitItem rows 20..35
// (data/world/init_items.csv) are Rune_Slate objects (1733..1736, meshes 989..992),
// one 2x2 block per room. The 7.48 client draws a slate sunk below the floor
// while open and on the floor with its glow while locked. The 7.59 TMSRV
// (sub_561F90) rolls a color 1..4 when each room starts, and sub_561E80 shows
// in every block cell k as the rune rolled for room k (item 1732+color,
// locked); the other slates stay open, sunk below the floor. Runs are
// independent here, so the slates are
// projected per viewer from that viewer's run instead of mutating the shared
// world objects.
const (
	runeSlateBase = 1732
	runeSlateLast = 1736
	// Voucher_Courage (1732) in the leader's slot 10 limits the roll to colors
	// 1..3; without it the Arcane chamber always rolls color 4 (no bonus).
	runeVoucherItem = 1732
	runeVoucherSlot = 10
	runeNoBonus     = 4
)

// magicChamberRun is process-local per-run state: the rune rolled for each
// room and whether the rune bonus boss has already appeared.
type magicChamberRun struct {
	Runes        [4]byte
	BonusSpawned bool
}

// runeSlateBlocks are the north-west cells of each room's block, in room order.
var runeSlateBlocks = [4][2]uint16{{786, 3676}, {844, 3676}, {844, 3620}, {786, 3634}}

// runeSlateCell maps a slate position to its cell 0..3 in InitItem order:
// (x,y), (x,y+1), (x+1,y), (x+1,y+1).
func runeSlateCell(x, y uint16) (int, bool) {
	for _, block := range runeSlateBlocks {
		if x >= block[0] && x <= block[0]+1 && y >= block[1] && y <= block[1]+1 {
			return int(x-block[0])*2 + int(y-block[1]), true
		}
	}
	return 0, false
}

func isMagicChamberInstance(inst *ItemInstance) bool {
	return inst != nil && strings.HasPrefix(strings.ToLower(strings.TrimSpace(inst.Config.ID)), "magic-chamber-")
}

func itemInstanceKey(inst *ItemInstance) string {
	if inst.RuntimeID != "" {
		return inst.RuntimeID
	}
	return inst.Config.ID
}

// rollMagicChamberRune draws the room's rune once per run, as the native
// server does on room entry, and refreshes slates the members already see.
func (w *World) rollMagicChamberRune(inst *ItemInstance, stage int) {
	if !isMagicChamberInstance(inst) || stage < 0 || stage >= len(runeSlateBlocks) {
		return
	}
	if w.magicChamberRunes == nil {
		w.magicChamberRunes = make(map[string]magicChamberRun)
	}
	for key := range w.magicChamberRunes {
		if _, live := w.itemInstances[key]; !live {
			delete(w.magicChamberRunes, key)
		}
	}
	key := itemInstanceKey(inst)
	run := w.magicChamberRunes[key]
	if run.Runes[stage] != 0 {
		return
	}
	run.Runes[stage] = w.rollRuneColor(inst)
	w.magicChamberRunes[key] = run
	for _, id := range inst.MemberIDs {
		p := w.playersByID[id]
		if p == nil || p.Session == nil || p.Session.IsClosed() {
			continue
		}
		for _, g := range w.nearbyGroundItems(p.X, p.Y, viewHalfX) {
			if _, slate := runeSlateCell(g.X, g.Y); !slate || !g.Permanent || !p.hasVisible(g.ID) {
				continue
			}
			p.Session.Send(wire.RemoveItem(uint32(g.ID)))
			p.Session.Send(w.groundItemCreatePacketFor(p, g))
		}
	}
}

// groundItemCreatePacketFor is groundItemCreatePacket as seen by one player:
// a Magic Chamber member sees the runes of their own run on the slates.
func (w *World) groundItemCreatePacketFor(p *Player, g *GroundItem) []byte {
	if g == nil || !g.Permanent || g.Item.Index <= runeSlateBase || g.Item.Index > runeSlateLast {
		return w.groundItemCreatePacket(g)
	}
	cell, slate := runeSlateCell(g.X, g.Y)
	if !slate {
		return w.groundItemCreatePacket(g)
	}
	var runes [4]byte
	if inst := w.itemInstances[w.gameplaySpaceForPlayer(p)]; isMagicChamberInstance(inst) {
		runes = w.magicChamberRunes[itemInstanceKey(inst)].Runes
	}
	color := runes[cell]
	if color == 0 {
		return w.groundItemCreatePacket(g) // open: sunk below the floor
	}
	view := *g
	view.Item.Index = runeSlateBase + uint16(color)
	// Locked and on the floor; height 0 keeps the base terrain walkable.
	return wire.CreateItem(view.X, view.Y, view.ID, view.Item, view.Rotate, clientGateLocked, 0, 0, 0)
}

// rollRuneColor follows the 7.59 roll: the leader's Voucher_Courage limits it
// to colors 1..3; without it the Arcane chamber always rolls color 4, and the
// Normal and Mystic chambers roll 1..4.
func (w *World) rollRuneColor(inst *ItemInstance) byte {
	voucher := false
	if leader := w.playersByID[inst.LeaderID]; leader != nil && leader.Char != nil {
		voucher = leader.Char.Equip[runeVoucherSlot].Index == runeVoucherItem
	}
	switch {
	case voucher:
		return byte(w.intn(3) + 1)
	case strings.EqualFold(strings.TrimSpace(inst.Config.ID), "magic-chamber-arcane"):
		return runeNoBonus
	default:
		return byte(w.intn(4) + 1)
	}
}

// runeBonusEarned reports the 7.59 condition: colors 1, 2 and 3 are all
// among the four rooms' runes.
func runeBonusEarned(runes [4]byte) bool {
	var seen [runeNoBonus]bool
	for _, color := range runes {
		if color > 0 && color < runeNoBonus {
			seen[color] = true
		}
	}
	return seen[1] && seen[2] && seen[3]
}

// spawnMagicChamberRuneBonus summons the rune bonus boss once, after the last
// room and its completion boss are cleared. The run finishes when it dies.
func (w *World) spawnMagicChamberRuneBonus(inst *ItemInstance, stage model.VolatileInstanceStage,
	now time.Time) bool {
	if !isMagicChamberInstance(inst) || len(stage.RuneBonusSpawns) == 0 {
		return false
	}
	key := itemInstanceKey(inst)
	run := w.magicChamberRunes[key]
	if run.BonusSpawned || !runeBonusEarned(run.Runes) {
		return false
	}
	bonus := stage
	bonus.CompletionSpawns = stage.RuneBonusSpawns
	if !w.spawnItemInstanceCompletionWave(inst, bonus, now) {
		return false
	}
	run.BonusSpawned = true
	w.magicChamberRunes[key] = run
	for _, id := range inst.MemberIDs {
		if member := w.playersByID[id]; member != nil && member.InWorld && member.Session != nil {
			member.Session.Send(wire.MessagePanel("The runes awaken a hidden guardian!"))
		}
	}
	log.Printf("INSTANCE %q rune bonus boss runes=%v", inst.Config.ID, run.Runes)
	return true
}
