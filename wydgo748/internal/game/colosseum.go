package game

import (
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// The Colosseum follows the 7.59 TMSRV scheduler (sub_567780): every day at
// EventHour+1 (N, mortals only), +2 (M, no Celestials) and +3 (A, consumes an
// Elixir_Notice) the steel windows close at minute 5 and waves spawn from
// minute 6 to 15. Project rule: the inner gates open with wave 1 at minute 6;
// minute 17 empties and resets the arena. One transition runs per one-second
// tick, like the native scheduler.
const (
	colosseumElixirItem = 3170
	colosseumExitX      = 2615
	colosseumExitY      = 1727
	steelWindowItem     = 472
	// colosseumElixirSlots is the carry range sub_565F50 scans.
	colosseumElixirSlots = 60
	// colosseumStartWindow is the last minute an hour may still start:
	// wave 1 and the inner gates start at minute 6.
	colosseumStartWindow = 6
	// colosseumIndexBase keeps event generators apart from NPCGener indices.
	colosseumIndexBase = 1 << 20
)

var (
	// colosseumArea bounds the entry filters (sub_565E30/sub_565F50) and the
	// fixtures; the native elixir check stops one row earlier, at 1744.
	colosseumArea       = warRect{2604, 1706, 2651, 1745}
	colosseumElixirArea = warRect{2604, 1706, 2651, 1744}
	// colosseumMoveArea is the movement filter while N or M runs.
	colosseumMoveArea = warRect{2604, 1708, 2648, 1744}
	// colosseumExitArea is emptied to (2615,1727) at minute 17 (sub_566230).
	colosseumExitArea = warRect{2622, 1708, 2651, 1745}
	// colosseumFixtureArea holds the steel windows (x 2603) and inner gates.
	colosseumFixtureArea = warRect{2600, 1706, 2651, 1745}
)

// colosseumWaves lists, per tier, the generator labels of the five waves at
// minutes 6, 9, 11, 13 and 15.
var colosseumWaves = [model.ColosseumTiers][5][]int{
	model.ColosseumTierN: {{5}, {6}, {5, 6}, {6, 7}, {7, 7}},
	model.ColosseumTierM: {{0}, {1}, {0, 1}, {1, 2}, {2, 2}},
	model.ColosseumTierA: {{103}, {104}, {103, 104}, {104, 105}, {106, 105, 105}},
}

type colosseumRuntime struct {
	config     model.ColosseumConfig
	generators [model.ColosseumTiers]map[int]*generState
	// stage mirrors the native dword_647378: 0 idle, 1 windows locked, 2..7
	// waves and gates, 8 finished until minute 0..4 of a later event hour.
	stage int
	// filter mirrors dword_61AA114: N and M recall ineligible classes that
	// move inside the arena until the event ends.
	filter bool
	// unpaid holds Colosseum A players whose charge or recall could not run
	// yet (pending critical write) or whose elixir write was rejected.
	unpaid map[*Player]bool
	// exits survives clock resets until every admitted arena exit commits.
	exits    map[*Player]bool
	endsAt   time.Time
	nextTick time.Time
	// A manual start uses the same timeline relative to startedAt, independent
	// of the wall-clock hour. Day claims and admin receipts are process-local.
	tier       int
	startedAt  time.Time
	claimed    [model.ColosseumTiers]string
	epoch      string
	revision   uint64
	receipts   map[string]colosseumReceipt
	notices    [model.ColosseumTiers]colosseumNoticeState
	countdowns map[*Player]time.Time
}

func WithColosseum(config model.ColosseumConfig, generators model.ColosseumGenerators) WorldOption {
	return func(w *World) {
		c := &colosseumRuntime{config: config}
		for tier, geners := range generators {
			c.generators[tier] = make(map[int]*generState, len(geners))
			for _, g := range geners {
				g.Index = colosseumIndexBase + tier*1000 + g.Label
				c.generators[tier][g.Label] = &generState{def: g}
			}
		}
		w.colosseum = c
	}
}

// bindColosseumTemplates resolves the event generators once NPC templates are
// known; a missing template fails startup like an NPCGener entry.
func (w *World) bindColosseumTemplates(templates map[string]*model.NPCDef) error {
	if w.colosseum == nil {
		return nil
	}
	for _, gens := range w.colosseum.generators {
		for _, g := range gens {
			g.leader, g.follower = templates[g.def.Leader], templates[g.def.Follower]
			if g.leader == nil {
				g.leader = templates[generName(g.def.Leader)]
			}
			if g.follower == nil {
				g.follower = templates[generName(g.def.Follower)]
			}
			if g.leader == nil || g.follower == nil {
				return fmt.Errorf("colosseum generator %d: missing template (Leader=%q Follower=%q)",
					g.def.Label, g.def.Leader, g.def.Follower)
			}
		}
	}
	return nil
}

// generatorReservedForColosseum drops the base NPCGener arena spawns
// (MinuteGenerate -1 inside the arena): only the event populates it.
func generatorReservedForColosseum(g model.NPCGener) bool {
	start := g.Segments[0]
	return g.MinuteGenerate < 0 && colosseumArea.contains(start.X, start.Y)
}

func (w *World) tickColosseum(now time.Time) {
	c := w.colosseum
	if c == nil {
		return
	}
	if !c.config.Enabled {
		w.clearColosseumCountdowns(now)
		return
	}
	if now.Before(c.nextTick) {
		return
	}
	c.nextTick = now.Add(time.Second)
	defer w.syncColosseumCountdowns(now)
	w.retryColosseumExits()
	location, err := w.serverLocation()
	if err != nil {
		log.Printf("colosseum timezone: %v", err)
		return
	}
	local := now.In(location)
	w.tickColosseumNotices(local)
	minute := local.Minute()
	if !c.endsAt.IsZero() && !now.Before(c.endsAt) && c.stage >= 1 && c.stage <= 7 {
		w.finishColosseum()
		c.stage, c.filter, c.unpaid = 8, false, nil
		return
	}
	// The reset window belongs to the event clock, not to a valid tier. After
	// tier A finishes, the next event starts at EventHour+1, so this check must
	// run before the tier guard or stage 8 would survive until the next day.
	if minute < 5 && c.stage == 8 {
		c.stage, c.filter, c.unpaid = 0, false, nil
	}
	tier := c.tier
	if c.stage == 0 || c.stage == 8 {
		tier = local.Hour() - int(c.config.EventHour) - 1
		if tier < 0 || tier >= model.ColosseumTiers || c.claimed[tier] == local.Format("2006-01-02") {
			return
		}
		if c.stage != 0 || minute < 5 {
			return
		}
		if minute > colosseumStartWindow || len(c.exits) != 0 {
			c.stage = 8
			return
		}
		w.startColosseum(tier, time.Date(local.Year(), local.Month(), local.Day(), local.Hour(), 5, 0, 0, location))
		return
	}
	minute = 5 + int(now.Sub(c.startedAt)/time.Minute)
	if c.filter {
		w.enforceColosseumClass(tier, colosseumMoveArea)
	}
	if tier == model.ColosseumTierA && c.stage >= 1 && c.stage <= 7 {
		w.retryColosseumElixir()
	}
	// Persistence delays must not grant unpaid entry or extend the event past
	// its existing end time. Pending exits remain retryable after finalization.
	if minute >= 17 && c.stage >= 1 && c.stage <= 7 {
		w.finishColosseum()
		c.stage, c.filter, c.unpaid = 8, false, nil
		log.Printf("colosseum %c: finished", "NMA"[tier])
		return
	}
	if tier == model.ColosseumTierA && len(c.unpaid) != 0 {
		return
	}
	switch {
	case minute >= 6 && c.stage == 1:
		w.setColosseumFixtures(arenaGateItem, gateOpen)
		w.spawnColosseumWave(tier, 0)
		c.stage = 2
	case minute >= 7 && c.stage == 2:
		// Preserve the stage numbers exposed by staff status and countdowns.
		c.stage = 3
	case minute >= 9 && c.stage == 3:
		w.spawnColosseumWave(tier, 1)
		c.stage = 4
	case minute >= 11 && c.stage == 4:
		w.spawnColosseumWave(tier, 2)
		c.stage = 5
	case minute >= 13 && c.stage == 5:
		w.spawnColosseumWave(tier, 3)
		c.stage = 6
	case minute >= 15 && c.stage == 6:
		w.spawnColosseumWave(tier, 4)
		c.stage = 7
	}
}

// MODERNIZACAO_COMPATIVEL: scheduled and staff-triggered runs share all
// admission, persistence, wave and cleanup rules. Starting early consumes
// today's scheduled tier in this process; it is not an extra reward session.
func (w *World) startColosseum(tier int, start time.Time) {
	c := w.colosseum
	c.tier, c.startedAt, c.endsAt = tier, start, start.Add(12*time.Minute)
	c.stage, c.filter, c.unpaid = 1, false, nil
	c.claimed[tier] = start.Format("2006-01-02")
	c.revision++
	w.setColosseumFixtures(steelWindowItem, gateClosed)
	switch tier {
	case model.ColosseumTierN, model.ColosseumTierM:
		c.filter = true
		w.enforceColosseumClass(tier, colosseumArea)
	case model.ColosseumTierA:
		c.unpaid = make(map[*Player]bool)
		for _, p := range w.colosseumPlayers(colosseumElixirArea) {
			w.chargeColosseumElixir(p)
		}
	}
	log.Printf("colosseum %c: started", "NMA"[tier])
	w.colosseumNotice(fmt.Sprintf("Colosseum %c has started. Entrance gates are now closed.", "NMA"[tier]))
	w.syncColosseumCountdowns(w.now())
}

// spawnColosseumWave ports sub_566770: each label runs GenerateMob 3..5 times.
func (w *World) spawnColosseumWave(tier, wave int) {
	for _, label := range colosseumWaves[tier][wave] {
		g := w.colosseum.generators[tier][label]
		if g == nil {
			continue
		}
		for i, n := 0, 3+w.intn(3); i < n; i++ {
			w.spawnGroup(g)
		}
	}
	message := fmt.Sprintf("Colosseum %c: wave %d/5 has begun.", "NMA"[tier], wave+1)
	if wave == 0 {
		w.colosseumNotice(fmt.Sprintf("Colosseum %c: combat has begun! Wave 1/5 is now active.", "NMA"[tier]))
	} else {
		for _, p := range w.colosseumPlayers(colosseumArea) {
			if w.gameplaySpaceForPlayer(p) == "" && p.Session != nil && !p.Session.IsClosed() {
				p.Session.Send(wire.MessagePanel(message))
				p.Session.Send(wire.MessageWhisper(0, "[Colosseum]", message, 7))
			}
		}
	}
}

// setColosseumFixtures drives the steel windows (sub_566470) or the inner
// gates (sub_5665F0); both are KEYID 9 fixtures only events change.
func (w *World) setColosseumFixtures(item uint16, state byte) {
	for _, g := range w.nearbyGroundItems(2626, 1726, 26) {
		if g != nil && g.Permanent && g.Item.Index == item && colosseumFixtureArea.contains(g.X, g.Y) {
			w.setFixtureState(g, state)
		}
	}
}

// colosseumPlayers lists live world players inside the rectangle.
func (w *World) colosseumPlayers(area warRect) []*Player {
	var out []*Player
	for _, p := range w.nearbyWorldPlayers(2627, 1726, 30) {
		if p != nil && p.InWorld && p.Char != nil && area.contains(p.X, p.Y) {
			out = append(out, p)
		}
	}
	return out
}

// enforceColosseumClass ports sub_565E30: N recalls every Arch or Celestial,
// M recalls Celestials (Is2stClass 2).
func (w *World) enforceColosseumClass(tier int, area warRect) {
	for _, p := range w.colosseumPlayers(area) {
		// A player with a pending write (such as the previous recall) is checked
		// again on the next tick instead of queueing a duplicate recall.
		if w.criticalWrites != nil && w.criticalPlayerReserved(p) {
			continue
		}
		celestial := isCelestialEvolution(p.Char)
		if celestial || tier == model.ColosseumTierN && isArch(p.Char) {
			w.recallPlayer(p, "colosseum class")
		}
	}
}

// chargeColosseumElixir ports sub_565F50: a player in the arena spends one
// Elixir_Notice (3170) or is told it is required (_NN_Need_Elixer 347) and
// recalled. A player whose charge or recall cannot run yet stays unpaid and is
// retried every tick until Colosseum A ends.
func (w *World) chargeColosseumElixir(p *Player) {
	c := w.colosseum
	c.unpaid[p] = true
	if w.criticalWrites != nil && w.criticalPlayerReserved(p) {
		c.unpaid[p] = true
		return
	}
	slots := min(colosseumElixirSlots, model.PlayerCarrySlots)
	for i := 0; i < slots; i++ {
		if p.Char.Inv[i].Index == colosseumElixirItem {
			if w.consumeColosseumElixir(p, byte(i)) && w.criticalWrites == nil {
				delete(c.unpaid, p)
			}
			return
		}
	}
	if p.Session != nil {
		p.Session.Send(wire.MessagePanel("You need an Elixir Notice to enter the Colosseum."))
	}
	if w.recallPlayer(p, "colosseum elixir") && w.criticalWrites == nil {
		delete(c.unpaid, p)
	}
}

// retryColosseumElixir charges unpaid players still inside Colosseum A.
func (w *World) retryColosseumElixir() {
	for p := range w.colosseum.unpaid {
		if w.players[p.Session] != p || !p.InWorld || p.Char == nil || !colosseumElixirArea.contains(p.X, p.Y) {
			delete(w.colosseum.unpaid, p)
			continue
		}
		w.chargeColosseumElixir(p)
	}
}

// consumeColosseumElixir reports whether the charge was taken or durably
// queued; a rejected critical write marks the player unpaid again.
func (w *World) consumeColosseumElixir(p *Player, slot byte) bool {
	if w.criticalWrites != nil {
		proposed := accountStateSnapshot(p.Account)
		ch := &proposed.Chars[p.CharSlot]
		consumeOne(&ch.Inv[slot])
		err := w.queueCriticalAccountWrite(p, proposed, criticalWrite{}, "colosseum_elixir",
			"The Colosseum entry could not be saved.", func() {
				p.Char.Inv[slot] = ch.Inv[slot]
				delete(w.colosseum.unpaid, p)
			},
			func() {
				if p.Session != nil {
					resyncUsedItem(p.Session, p, slot)
				}
			}, false, criticalAccountMutationHooks{reject: func() {
				if c := w.colosseum; c != nil && c.unpaid != nil {
					c.unpaid[p] = true
				}
			}})
		return err == nil
	}
	before := p.Char.Inv[slot]
	consumeOne(&p.Char.Inv[slot])
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[slot] = before
		log.Printf("colosseum elixir save: %v", err)
		return false
	}
	if p.Session != nil {
		resyncUsedItem(p.Session, p, slot)
	}
	return true
}

// finishColosseum ports minute 17: revive and move players out of the arena
// (sub_566230), reopen the windows, lock the inner gates and delete the event
// mobs that are still alive (sub_5667F0).
func (w *World) finishColosseum() {
	w.colosseum.revision++
	w.clearColosseumCountdowns(w.now())
	if w.colosseum.exits == nil {
		w.colosseum.exits = make(map[*Player]bool)
	}
	for _, p := range w.colosseumPlayers(colosseumExitArea) {
		w.colosseum.exits[p] = true
	}
	w.retryColosseumExits()
	w.setColosseumFixtures(steelWindowItem, gateOpen)
	w.setColosseumFixtures(arenaGateItem, gateClosed)
	w.removeColosseumMobs()
}

func (w *World) retryColosseumExits() {
	c := w.colosseum
	for p := range c.exits {
		if p == nil || p.Char == nil || !p.InWorld || w.players[p.Session] != p ||
			!colosseumExitArea.contains(p.X, p.Y) {
			delete(c.exits, p)
			continue
		}
		if w.criticalWrites != nil && w.criticalPlayerReserved(p) {
			continue
		}
		if w.moveOutOfArenaWithAck(p, colosseumExitX, colosseumExitY, func() { delete(c.exits, p) }) &&
			w.criticalWrites == nil {
			delete(c.exits, p)
		}
	}
}

// moveOutOfArena ports sub_566230 for one player: the dead get 1 HP, their
// corpse leaves every observer's view and they are teleported, through the
// critical persistence coordinator when it is active.
func (w *World) moveOutOfArena(p *Player, x, y uint16) bool {
	return w.moveOutOfArenaWithAck(p, x, y, nil)
}

func (w *World) moveOutOfArenaWithAck(p *Player, x, y uint16, afterAck func()) bool {
	if p == nil || p.Char == nil || p.Account == nil {
		return false
	}
	dead := playerCurHP(p.Char) == 0
	if w.criticalWrites != nil {
		proposed := accountStateSnapshot(p.Account)
		if dead {
			setPlayerCurHP(&proposed.Chars[p.CharSlot], 1)
		}
		return w.queuePlayerTeleportState(p, proposed, x, y, "arena exit", nil, afterAck, nil, dead) == nil
	}
	if !dead {
		moved := w.teleportPlayer(p, x, y)
		if moved && afterAck != nil {
			afterAck()
		}
		return moved
	}
	if _, _, ok := w.resolvePlayerTeleportDestination(p, x, y); !ok {
		return false
	}
	oldHP := playerCurHP(p.Char)
	oldDeadAt := p.DeadAt
	observers := w.nearbyWorldPlayers(p.X, p.Y, viewHalfX)
	for _, observer := range observers {
		if observer != p && observer.Session != nil && observer.hasVisible(p.ID) {
			observer.Session.Send(wire.RemoveMob(p.ID, 3))
			observer.hide(p.ID)
		}
	}
	setPlayerCurHP(p.Char, 1)
	p.DeadAt = time.Time{}
	moved := w.teleportPlayer(p, x, y)
	if !moved {
		// teleportPlayer restores the coordinates when persistence fails, but it
		// cannot know that this caller also staged a revival and corpse removal.
		// Roll back those local changes and rematerialize the corpse for observers.
		setPlayerCurHP(p.Char, oldHP)
		p.DeadAt = oldDeadAt
		for _, observer := range observers {
			if observer != p && observer.InWorld && w.playersVisibleTogether(observer, p) &&
				inView(observer.X, observer.Y, p.X, p.Y) {
				w.showPlayerPair(observer, p)
			}
		}
	}
	w.syncPlayerVitals(p)
	if moved && afterAck != nil {
		afterAck()
	}
	return moved
}

func (w *World) colosseumGenerator(g *generState) bool {
	if w.colosseum == nil || g == nil {
		return false
	}
	for _, gens := range w.colosseum.generators {
		if gens[g.def.Label] == g {
			return true
		}
	}
	return false
}

func (w *World) removeColosseumMobs() {
	var doomed []*Mob
	for _, m := range w.mobs {
		if m != nil && w.colosseumGenerator(m.generator) {
			doomed = append(doomed, m)
		}
	}
	for _, m := range doomed {
		w.publishMobRemoval(m)
		w.removeMobInstance(m)
	}
	for _, gens := range w.colosseum.generators {
		for _, g := range gens {
			g.current = 0
		}
	}
}
