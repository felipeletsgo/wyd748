package game

import (
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func runeGeneratorRoom(id int) int {
	switch {
	case id >= 5662 && id <= 5714:
		return 0
	case id >= 5715 && id <= 5774:
		return 1
	case id >= 5775 && id <= 5797 || id >= 5985 && id <= 5988:
		return 6
	case id >= 5798 && id <= 5857:
		return 2
	case id >= 5858 && id <= 5907:
		return 4
	case id >= 5908 && id <= 5956:
		return 5
	case id >= 5957 && id <= 5984:
		return 3
	}
	return -1
}

func (w *World) runeGeneratorCount(inst *ItemInstance, label int) int {
	count := 0
	for id, g := range inst.RuneTrack.Generators {
		if m := w.mobsByID[id]; g == label && m != nil && !m.Dead && m.HP > 0 {
			count++
		}
	}
	return count
}

// Keep each event template immutable. Only the tower's event-local role is
// changed: players cannot attack their own objective, but invaders can kill it.
func (w *World) spawnRuneGenerator(inst *ItemInstance, label int) bool {
	if inst == nil || inst.RuneTrack == nil || runeGeneratorRoom(label) != inst.UxmalRoom {
		return false
	}
	var selected *model.NPCGener
	for i := range inst.Config.Uxmal.Generators {
		g := &inst.Config.Uxmal.Generators[i]
		if g.Label == label {
			selected = g
			break
		}
	}
	if selected == nil || selected.MaxNumMob <= 0 {
		return false
	}
	g := *selected
	available := g.MaxNumMob - w.runeGeneratorCount(inst, label)
	if available <= 0 {
		return true
	}
	followers := g.MinGroup
	if g.MaxGroup > g.MinGroup {
		followers += w.intn(g.MaxGroup - g.MinGroup + 1)
	}
	count := min(available, 1+followers)
	created := make([]*Mob, 0, count)
	reserved := map[uint32]struct{}{}
	for n := 0; n < count; n++ {
		name := g.Leader
		if n > 0 && g.Follower != "" {
			name = g.Follower
		}
		def := w.npcDefByName(name)
		if def == nil || def.Score == nil || def.Score.MaxHP == 0 {
			break
		}
		if label >= 5715 && label <= 5717 {
			copy := *def
			copy.Tipo = model.TipoNPC
			def = &copy
		}
		start := g.Segments[0]
		var x, y uint16
		found := false
		for radius := 0; radius <= max(4, int(start.Range)); radius++ {
			x, y, found = findHellGatePosition(w, inst.RuntimeID, start.X, start.Y, radius, reserved)
			if found {
				break
			}
		}
		if !found {
			break
		}
		id := w.allocMobID()
		if id == 0 {
			break
		}
		mob := &Mob{ID: id, Def: def, X: x, Y: y, HP: def.Score.MaxHP, GenerIndex: -1, InstanceID: inst.RuntimeID,
			Segments: g.Segments, RouteType: g.RouteType}
		if len(created) > 0 {
			mob.LeaderID = created[0].ID
		}
		created = append(created, mob)
		reserved[uint32(x)<<16|uint32(y)] = struct{}{}
	}
	if len(created) != count {
		return false
	}
	for _, m := range created {
		w.appendMobInstance(m)
		inst.RuneTrack.Generators[m.ID] = label
		if m.Def.IsMonster() {
			inst.MobIDs[m.ID] = struct{}{}
		} else {
			inst.NPCIDs[m.ID] = struct{}{}
		}
		w.publishMobSpawn(m)
	}
	inst.Remaining = len(inst.MobIDs)
	return true
}

func (w *World) removeRuneGenerator(inst *ItemInstance, label int) {
	for id, g := range inst.RuneTrack.Generators {
		if g != label {
			continue
		}
		if m := w.mobsByID[id]; m != nil {
			w.publishMobRemoval(m)
			w.removeMobInstance(m)
		}
		delete(inst.RuneTrack.Generators, id)
		delete(inst.MobIDs, id)
		delete(inst.NPCIDs, id)
	}
	inst.Remaining = len(inst.MobIDs)
}

func (w *World) initializeRuneTrack(inst *ItemInstance, now time.Time) bool {
	rt := inst.RuneTrack
	ready := true
	spawn := func(id int) {
		if !w.spawnRuneGenerator(inst, id) {
			ready = false
		}
	}
	switch inst.UxmalRoom {
	case 0:
		spawn(5662)
		spawn(5663)
	case 1:
		for i, g := range rt.Parties {
			if len(g.Members) > 0 {
				spawn(5715 + i)
			}
		}
	case 4:
		if rt.HiddenGenerator == 0 {
			rt.HiddenGenerator = 5863 + w.intn(40)
		}
		for id := 5863; id <= 5907; id++ {
			spawn(id)
		}
	case 5:
		spawn(5908)
	case 6:
		spawn(5775)
		for id := 5985; id <= 5988; id++ {
			spawn(id)
		}
	}
	for _, g := range inst.Config.Uxmal.Generators {
		if runeGeneratorRoom(g.Label) == inst.UxmalRoom && g.MinuteGenerate > 0 {
			rt.NextGenerate[g.Label] = now // Event-only generators never enter the public map scheduler.
		}
	}
	return ready
}

func (w *World) tickRunePopulation(inst *ItemInstance, now time.Time) {
	rt := inst.RuneTrack
	for _, g := range inst.Config.Uxmal.Generators {
		if due, ok := rt.NextGenerate[g.Label]; ok && !now.Before(due) {
			w.spawnRuneGenerator(inst, g.Label)
			rt.NextGenerate[g.Label] = now.Add(time.Duration(g.MinuteGenerate) * npcGenerMinute)
		}
	}
	switch inst.UxmalRoom {
	case 1:
		for id := 5718; id <= 5774; id++ {
			w.spawnRuneGenerator(inst, id)
		}
	case 2:
		if !inst.BossSpawned && !now.Before(rt.Start.Add(5*time.Minute)) {
			if w.spawnRuneGenerator(inst, 5798) {
				inst.BossSpawned = true
			}
		}
		if !rt.ObjectiveWon {
			for id := 5799; id <= 5857; id++ {
				w.spawnRuneGenerator(inst, id)
			}
		}
		w.restrictRuneBossAccess(inst)
	case 4:
		if rt.Ticks%3 == 0 && !rt.DangerPaused {
			w.runeTrackFloorDamage(inst)
		}
	case 5:
		if rt.Ticks%15 == 0 {
			w.tickRunePortals(inst)
		}
	case 6:
		for id := 5777; id <= 5783; id++ {
			w.spawnRuneGenerator(inst, id)
		}
		for i, g := range rt.Parties {
			for _, p := range w.runeTrackMembers(inst, &g) {
				if p.Session != nil {
					p.Session.Send(wire.MessagePanel(fmtRuneProgress(rt.Collected[i], rt.Targets[i])))
				}
			}
		}
	}
	// Objective marchers must remain active even with no nearby player. This
	// does not wake unrelated monsters or change the public-map AI.
	for id, label := range rt.Generators {
		m := w.mobsByID[id]
		if m != nil && !m.Dead && (label >= 5718 && label <= 5774 || label >= 5799 && label <= 5857 || label >= 5777 && label <= 5783) {
			m.Awake = true
			w.activeMobs[id] = m
		}
	}
}

func (w *World) onRuneTrackKill(killer *Player, m *Mob, now time.Time) {
	if m == nil {
		return
	}
	inst := w.instanceForMob(m)
	if inst == nil || inst.RuneTrack == nil {
		return
	}
	rt := inst.RuneTrack
	label, ok := rt.Generators[m.ID]
	if !ok {
		return
	}
	delete(rt.Generators, m.ID)
	delete(inst.MobIDs, m.ID)
	delete(inst.NPCIDs, m.ID)
	inst.Remaining = len(inst.MobIDs)
	if inst.State != "combat" || !now.Before(rt.Start.Add(15*time.Minute)) {
		return
	}
	if label >= 5715 && label <= 5717 {
		i := label - 5715
		if i < len(rt.Parties) && w.runeGeneratorCount(inst, label) == 0 {
			rt.Parties[i].TowerLost = true
		}
		return
	}
	index := w.runeTrackPartyFor(inst, killer)
	if index < 0 {
		return
	}
	group := &rt.Parties[index]
	switch {
	case label == 5662 || label == 5663:
		if rt.ObjectiveWon {
			return
		}
		if index == label-5662 && w.intn(8) == 0 {
			rt.ObjectiveWon = true
			group.PendingReward = true
			w.removeRuneGenerator(inst, 5662)
			w.removeRuneGenerator(inst, 5663)
		} else {
			w.removeRuneGenerator(inst, label)
			w.spawnRuneGenerator(inst, label)
		}
	case label == 5798 || label == 5908 || label == 5858:
		if !rt.ObjectiveWon {
			rt.ObjectiveWon = true
			group.PendingReward = true
		}
	case label >= 5981 && label <= 5984:
		active := false
		for id := 5957; id <= 5964; id++ {
			active = active || w.runeGeneratorCount(inst, id) > 0
		}
		if !active {
			w.spawnRuneGenerator(inst, 5957+w.intn(8))
		}
	case label >= 5957 && label <= 5964:
		group.Score++
	case label >= 5863 && label <= 5903:
		if w.runeGeneratorCount(inst, label) == 0 {
			if label == rt.HiddenGenerator {
				group.Destination = [2]uint16{3351, 1335}
				w.spawnRuneGenerator(inst, 5858)
			}
			rt.DangerPaused = !rt.DangerPaused
		}
	case label >= 5784 && label <= 5797:
		if rt.Targets[index] == 0 {
			return
		}
		rt.Collected[index] = min(rt.Targets[index], rt.Collected[index]+1)
		if rt.Collected[index] == rt.Targets[index] && rt.Targets[0] != 0 && rt.Targets[1] != 0 && rt.Targets[2] != 0 {
			w.spawnRuneGenerator(inst, 5776)
		}
	case label == 5776:
		if w.runeGeneratorCount(inst, 5776) == 0 && rt.Targets[0] != 0 && rt.Targets[1] != 0 && rt.Targets[2] != 0 {
			group.Gold = uint32(500_000 * (2 * (rt.Collected[0] + rt.Collected[1] + rt.Collected[2] + rt.Targets[index]) / 3))
			group.PendingReward = true
			rt.Targets = [3]int{}
			rt.ObjectiveWon = true
		}
	}
}

// Objective AI bypasses only the ordinary home leash. All movement still uses
// the server's terrain, collision and visibility machinery.
func (w *World) tickRuneMarch(m *Mob, now time.Time, allowMovement bool) bool {
	inst := w.instanceForMob(m)
	if inst == nil || inst.RuneTrack == nil {
		return false
	}
	label := inst.RuneTrack.Generators[m.ID]
	if !(label >= 5718 && label <= 5774 || label >= 5799 && label <= 5857 || label >= 5777 && label <= 5783) {
		return false
	}
	if !w.runeTrackCombatActor(m, now) {
		return true
	}
	var target *Mob
	var x, y uint16
	if inst.UxmalRoom == 1 {
		best := int(^uint(0) >> 1)
		for id, g := range inst.RuneTrack.Generators {
			if g < 5715 || g > 5717 {
				continue
			}
			n := w.mobsByID[id]
			if n != nil && !n.Dead && n.HP > 0 {
				d := chebyshev(m.X, m.Y, n.X, n.Y)
				if d < best || d == best && (target == nil || n.ID < target.ID) {
					target = n
					best = d
				}
			}
		}
		if target == nil {
			return false
		}
		x, y = target.X, target.Y
		m.TargetID = target.ID
		if chebyshev(m.X, m.Y, x, y) <= mobAttackRange {
			if !now.Before(m.NextAttack) && w.combatLineOfSight(m.X, m.Y, x, y) {
				m.NextAttack = now.Add(mobAttackInterval)
				if !w.runeTrackCastAtTower(m, target, now) {
					w.mobAttackNightmareProtector(m, target, now)
				}
			}
			return true
		}
	} else if inst.UxmalRoom == 2 {
		x, y = 3391, 1438
		for id, g := range inst.RuneTrack.Generators {
			if g == 5798 {
				target = w.mobsByID[id]
				break
			}
		}
		if target != nil && runeInside(m.X, m.Y, [4]uint16{3382, 1426, 3394, 1466}) {
			target.HP = min(uint32(1_280_000), target.HP+2000)
			if target.HP > target.Def.Score.MaxHP {
				def, score := *target.Def, *target.Def.Score
				score.MaxHP = target.HP
				def.Score = &score
				target.Def = &def
			}
			w.sendToMobView(target, func() []byte {
				return wire.MobHpMp(target.ID, target.HP, max(target.HP, target.Def.Score.MaxHP), 0, target.Def.Score.MaxMP)
			})
			w.removeRuneGeneratorMob(inst, m)
			return true
		}
	} else {
		x, y = 3425, 1507
		if m.X >= 3422 {
			for i := range inst.RuneTrack.Collected {
				inst.RuneTrack.Collected[i] = max(0, inst.RuneTrack.Collected[i]-1)
			}
			w.removeRuneGeneratorMob(inst, m)
			return true
		}
	}
	if allowMovement && !now.Before(m.NextMove) {
		w.moveMobToward(m, x, y, 0, now)
	}
	return true
}

func (w *World) removeRuneGeneratorMob(inst *ItemInstance, m *Mob) {
	w.publishMobRemoval(m)
	w.removeMobInstance(m)
	delete(inst.RuneTrack.Generators, m.ID)
	delete(inst.MobIDs, m.ID)
	delete(inst.NPCIDs, m.ID)
	inst.Remaining = len(inst.MobIDs)
}

func runeInside(x, y uint16, r [4]uint16) bool {
	return x >= r[0] && y >= r[1] && x <= r[2] && y <= r[3]
}

func (w *World) restrictRuneBossAccess(inst *ItemInstance) {
	if w.runeGeneratorCount(inst, 5798) == 0 {
		return
	}
	for i := range inst.RuneTrack.Parties {
		g := &inst.RuneTrack.Parties[i]
		for _, p := range w.runeTrackMembers(inst, g) {
			if runeTrackRef(p) != g.Leader && runeInside(p.X, p.Y, [4]uint16{3382, 1426, 3394, 1466}) {
				pos := w.uxmal.Uxmal.RoomPositions[2][i]
				// Only the intruder is moved; the party leader keeps fighting.
				w.moveRuneIntruder(inst, p, pos.X, pos.Y)
				return
			}
		}
	}
	for id, g := range inst.RuneTrack.Generators {
		if m := w.mobsByID[id]; g == 5798 && m != nil && !runeInside(m.X, m.Y, [4]uint16{3384, 1428, 3392, 1464}) {
			oldX, oldY := m.X, m.Y
			m.X, m.Y = 3389, 1437
			w.moveMobSpatial(m, oldX, oldY)
			w.publishMobRemoval(m)
			w.publishMobSpawn(m)
		}
	}
}

func (w *World) moveRuneIntruder(inst *ItemInstance, p *Player, x, y uint16) {
	positions, ok := w.planInstancePositionsIgnoringForInstance([]*Player{p}, x, y, map[*Player]struct{}{p: {}}, inst.RuntimeID)
	if !ok {
		return
	}
	w.runeTrackMutation(inst, []*Player{p}, "Rune Track boss boundary", func() {
		p.X, p.Y = positions[0][0], positions[0][1]
		p.Char.X, p.Char.Y = p.X, p.Y
	}, nil)
}
