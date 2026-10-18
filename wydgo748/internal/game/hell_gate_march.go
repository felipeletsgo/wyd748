package game

import "time"

// Defense attackers have an event objective, not an ordinary home leash.
// Membership in MobIDs prevents summons or unrelated actors sharing the map
// from acquiring Dionys. Nightmare and restored version-2 events keep their AI.
func (w *World) hellGateMarchTarget(m *Mob) *Mob {
	inst := w.instanceForMob(m)
	cfg, ok := hellGateInstance(inst)
	if !ok || cfg.Version != 3 || inst.State != "protection" ||
		!w.now().Before(inst.HardDeadline) || m == nil || m.Dead || m.HP == 0 ||
		m.SummonerID != 0 || m.Def == nil || !m.Def.IsMonster() {
		return nil
	}
	if _, owned := inst.MobIDs[m.ID]; !owned {
		return nil
	}
	return w.nightmareProtectorTarget(m, 65535)
}

// A single reverse breadth-first search per protector tile avoids a search
// per attacker per tick. Dynamic occupancy is checked at movement time, so a
// blocked group waits instead of walking through another actor or a wall.
func (w *World) hellGateRoute(inst *ItemInstance, guardian *Mob) map[[2]uint16]uint16 {
	target := [2]uint16{guardian.X, guardian.Y}
	if inst.HellGateRouteDistances != nil && inst.HellGateRouteTarget == target {
		return inst.HellGateRouteDistances
	}
	distances := map[[2]uint16]uint16{target: 0}
	queue := [][2]uint16{target}
	areaX, areaY, radius, ok := instanceMobArea(inst, guardian)
	// Bound both memory and CPU even if an invalid configuration reaches here.
	if !ok || radius <= 0 || radius > 256 {
		return nil
	}
	for cursor := 0; cursor < len(queue); cursor++ {
		from := queue[cursor]
		for dy := -1; dy <= 1; dy++ {
			for dx := -1; dx <= 1; dx++ {
				nx, ny := int(from[0])+dx, int(from[1])+dy
				if nx <= 0 || ny <= 0 || nx > 65535 || ny > 65535 {
					continue
				}
				to := [2]uint16{uint16(nx), uint16(ny)}
				if _, seen := distances[to]; seen || chebyshev(to[0], to[1], areaX, areaY) > radius ||
					!w.terrain.Walkable(to[0], to[1]) ||
					!w.terrain.RouteHeightCompatible(to[0], to[1], from[0], from[1]) {
					continue
				}
				if dx != 0 && dy != 0 && !w.terrain.Walkable(to[0], from[1]) && !w.terrain.Walkable(from[0], to[1]) {
					continue
				}
				distances[to] = distances[from] + 1
				queue = append(queue, to)
			}
		}
	}
	inst.HellGateRouteTarget, inst.HellGateRouteDistances = target, distances
	return distances
}

func (w *World) marchHellGateAttacker(m, guardian *Mob, now time.Time) {
	if chebyshev(m.X, m.Y, guardian.X, guardian.Y) <= mobAttackRange {
		return
	}
	inst := w.instanceForMob(m)
	distances := w.hellGateRoute(inst, guardian)
	current := [2]uint16{m.X, m.Y}
	best, reachable := distances[current]
	if !reachable {
		return
	}
	next := current
	for dy := -1; dy <= 1; dy++ {
		for dx := -1; dx <= 1; dx++ {
			candidate := [2]uint16{uint16(int(m.X) + dx), uint16(int(m.Y) + dy)}
			distance, found := distances[candidate]
			if !found || distance >= best || w.mobStepBlockedFrom(m, m.X, m.Y, candidate[0], candidate[1]) {
				continue
			}
			if dx != 0 && dy != 0 && !w.terrain.Walkable(candidate[0], m.Y) && !w.terrain.Walkable(m.X, candidate[1]) {
				continue
			}
			next, best = candidate, distance
		}
	}
	attackRun := effectiveMobAttackRunAt(m, now)
	speed := mobMovementSpeed(attackRun)
	m.NextMove = now.Add(mobBaseStepInterval / time.Duration(speed))
	if next != current {
		// One adjacent step has an unambiguous native client route, including
		// detours that initially move away from the target around an obstacle.
		m.X, m.Y = next[0], next[1]
		w.publishMobMove(m, current[0], current[1], speed)
	}
}
