package game

import (
	"time"
)

const (
	mobBaseStepInterval = time.Second
	maxMobMoveSpeed     = 4
)

func mobMovementSpeed(attackRun byte) uint32 {
	speed := uint32(attackRun & 0x0F)
	if speed < 1 {
		speed = 1
	} else if speed > maxMobMoveSpeed {
		speed = maxMobMoveSpeed
	}
	return speed
}

func mobMoveDelay(attackRun byte) time.Duration {
	return mobBaseStepInterval / time.Duration(mobMovementSpeed(attackRun))
}

// moveMobToward envia um trecho completo, como GetTargetPos/BASE_GetRoute:
// distancia maxima = 2*Speed. Um unico Action mantem o ciclo de caminhada do
// client ativo ate o fim, em vez de reinicia-lo em toda coordenada.
func (w *World) moveMobToward(m *Mob, targetX, targetY uint16, stopDistance int, now time.Time) int {
	if m == nil || m.Def == nil || m.Def.Extended == nil {
		return 0
	}
	attackRun := effectiveMobAttackRunAt(m, now)
	speed := mobMovementSpeed(attackRun)
	maxSteps := int(speed * 2)
	x, y := m.X, m.Y
	visited := map[uint32]struct{}{uint32(x)<<16 | uint32(y): {}}
	steps := 0
	for steps < maxSteps && chebyshev(x, y, targetX, targetY) > stopDistance {
		nextX, nextY := stepToward(x, y, targetX, targetY)
		if w.mobStepBlockedFrom(m, x, y, nextX, nextY) {
			nextX, nextY = w.freeStepAroundFrom(m, x, y, targetX, targetY, visited)
		}
		if nextX == x && nextY == y {
			break
		}
		key := uint32(nextX)<<16 | uint32(nextY)
		if _, seen := visited[key]; seen {
			break
		}
		visited[key] = struct{}{}
		x, y = nextX, nextY
		steps++
	}
	if steps == 0 {
		m.NextMove = now.Add(mobMoveDelay(attackRun))
		return 0
	}
	oldX, oldY := m.X, m.Y
	m.X, m.Y = x, y
	m.NextMove = now.Add(time.Duration(steps) * mobBaseStepInterval / time.Duration(speed))
	w.publishMobMove(m, oldX, oldY, speed)
	return steps
}

// tickMobRoutes implementa a patrulha basica do NPCGener. RouteType 2 percorre
// os segmentos validos em ida e volta, como CMob::SetSegment no W2PP.
func (w *World) tickMobRoutes(now time.Time, shard, shardCount int) {
	for _, m := range w.activeMobs {
		if shardCount > 1 && int(m.ID)%shardCount != shard {
			continue
		}
		if m.Dead || m.TargetID != 0 || m.RouteType == 0 || now.Before(m.WaitUntil) || now.Before(m.NextMove) {
			continue
		}
		targetIndex, ok := nextSegment(m)
		if !ok {
			continue
		}
		target := m.Segments[targetIndex]
		if m.X == target.X && m.Y == target.Y {
			m.SegmentProgress = targetIndex
			m.WaitUntil = now.Add(time.Duration(target.Wait) * time.Second)
			continue
		}
		w.moveMobToward(m, target.X, target.Y, 0, now)
		if m.X == target.X && m.Y == target.Y {
			m.SegmentProgress = targetIndex
			m.WaitUntil = m.NextMove.Add(time.Duration(target.Wait) * time.Second)
		}
	}
}

func (w *World) freeStepAroundFrom(m *Mob, fromX, fromY, targetX, targetY uint16, visited map[uint32]struct{}) (uint16, uint16) {
	bestX, bestY := fromX, fromY
	bestDist := mobRouteDistance(fromX, fromY, targetX, targetY)
	for dy := -1; dy <= 1; dy++ {
		for dx := -1; dx <= 1; dx++ {
			if dx == 0 && dy == 0 {
				continue
			}
			nx, ny := int(fromX)+dx, int(fromY)+dy
			if nx <= 0 || ny <= 0 || nx > 65535 || ny > 65535 {
				continue
			}
			x, y := uint16(nx), uint16(ny)
			if w.mobStepBlockedFrom(m, fromX, fromY, x, y) {
				continue
			}
			if visited != nil {
				if _, seen := visited[uint32(x)<<16|uint32(y)]; seen {
					continue
				}
			}
			d := mobRouteDistance(x, y, targetX, targetY)
			if d > bestDist {
				continue
			}
			if d < bestDist || bestX == fromX && bestY == fromY {
				bestX, bestY, bestDist = x, y, d
			}
		}
	}
	return bestX, bestY
}

func mobRouteDistance(x, y, tx, ty uint16) int {
	dx, dy := absInt(int(x)-int(tx)), absInt(int(y)-int(ty))
	if dx > dy {
		return dx
	}
	return dy
}

func nextSegment(m *Mob) (int, bool) {
	valid := make([]int, 0, len(m.Segments))
	for i, s := range m.Segments {
		if s.X != 0 && s.Y != 0 {
			valid = append(valid, i)
		}
	}
	if len(valid) < 2 {
		return 0, false
	}
	pos := 0
	for i, idx := range valid {
		if idx == m.SegmentProgress {
			pos = i
			break
		}
	}
	if m.RouteType == 2 || m.RouteType == 3 {
		if m.SegmentDirection == 0 {
			if pos+1 < len(valid) {
				return valid[pos+1], true
			}
			m.SegmentDirection = 1
			return valid[pos-1], true
		}
		if pos > 0 {
			return valid[pos-1], true
		}
		m.SegmentDirection = 0
		return valid[1], true
	}
	if pos+1 < len(valid) {
		return valid[pos+1], true
	}
	if m.RouteType == 4 {
		return valid[0], true
	}
	return 0, false
}

func stepToward(x, y, tx, ty uint16) (uint16, uint16) {
	if x < tx {
		x++
	} else if x > tx {
		x--
	}
	if y < ty {
		y++
	} else if y > ty {
		y--
	}
	return x, y
}
