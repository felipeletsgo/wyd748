package game

const spatialCellSize = 16

func spatialKey(x, y uint16) uint32 {
	return uint32(x/spatialCellSize)<<16 | uint32(y/spatialCellSize)
}

func spatialCellXY(key uint32) (int, int) { return int(key >> 16), int(key & 0xffff) }

func (w *World) registerMobSpatial(m *Mob) {
	if m == nil {
		return
	}
	if w.mobsByID == nil {
		w.mobsByID = make(map[uint16]*Mob)
	}
	if w.mobCells == nil {
		w.mobCells = make(map[uint32]map[uint16]*Mob)
	}
	if w.mobCell == nil {
		w.mobCell = make(map[uint16]uint32)
	}
	if w.activeMobs == nil {
		w.activeMobs = make(map[uint16]*Mob)
	}
	key := spatialKey(m.X, m.Y)
	cell := w.mobCells[key]
	if cell == nil {
		cell = make(map[uint16]*Mob)
		w.mobCells[key] = cell
	}
	cell[m.ID], w.mobsByID[m.ID], w.mobCell[m.ID] = m, m, key
	if m.SummonerID != 0 {
		if w.summons == nil {
			w.summons = make(map[uint16]*Mob)
		}
		w.summons[m.ID] = m
	}
	if m.SummonKind == summonKindThornWall {
		if w.sephiraObjects == nil {
			w.sephiraObjects = make(map[uint16]*Mob)
		}
		w.sephiraObjects[m.ID] = m
	}
	w.recomputeMobActive(m)
}

func (w *World) unregisterMobSpatial(m *Mob) {
	if m == nil {
		return
	}
	if key, ok := w.mobCell[m.ID]; ok {
		delete(w.mobCells[key], m.ID)
		if len(w.mobCells[key]) == 0 {
			delete(w.mobCells, key)
		}
	}
	delete(w.mobCell, m.ID)
	delete(w.mobsByID, m.ID)
	delete(w.activeMobs, m.ID)
	delete(w.summons, m.ID)
	delete(w.sephiraObjects, m.ID)
}

func (w *World) moveMobSpatial(m *Mob, oldX, oldY uint16) {
	if m == nil {
		return
	}
	oldKey, newKey := spatialKey(oldX, oldY), spatialKey(m.X, m.Y)
	if oldKey != newKey {
		delete(w.mobCells[oldKey], m.ID)
		if len(w.mobCells[oldKey]) == 0 {
			delete(w.mobCells, oldKey)
		}
		cell := w.mobCells[newKey]
		if cell == nil {
			cell = make(map[uint16]*Mob)
			w.mobCells[newKey] = cell
		}
		cell[m.ID], w.mobCell[m.ID] = m, newKey
	}
	w.recomputeMobActive(m)
}

func (w *World) nearbyMobs(x, y uint16, radius int) []*Mob {
	if radius < 0 {
		return nil
	}
	cx, cy := int(x)/spatialCellSize, int(y)/spatialCellSize
	cr := radius/spatialCellSize + 1
	result := make([]*Mob, 0, 64)
	for yy := cy - cr; yy <= cy+cr; yy++ {
		for xx := cx - cr; xx <= cx+cr; xx++ {
			if xx < 0 || yy < 0 {
				continue
			}
			for _, m := range w.mobCells[uint32(xx)<<16|uint32(yy)] {
				if !m.Dead && chebyshev(x, y, m.X, m.Y) <= radius {
					result = append(result, m)
				}
			}
		}
	}
	return result
}

func (w *World) nearbyPlayers(x, y uint16, radius int) []*Player {
	all := w.nearbyWorldPlayers(x, y, radius)
	result := all[:0]
	for _, p := range all {
		if validMobTarget(p) {
			result = append(result, p)
		}
	}
	return result
}

// nearbyWorldPlayers atende visibilidade/transporte e, ao contrario da busca
// de alvo da IA, inclui jogadores mortos ou ocultos que ainda estao no mundo.
func (w *World) nearbyWorldPlayers(x, y uint16, radius int) []*Player {
	cx, cy := int(x)/spatialCellSize, int(y)/spatialCellSize
	cr := radius/spatialCellSize + 1
	result := make([]*Player, 0, 16)
	for yy := cy - cr; yy <= cy+cr; yy++ {
		for xx := cx - cr; xx <= cx+cr; xx++ {
			if xx < 0 || yy < 0 {
				continue
			}
			for _, p := range w.playerCells[uint32(xx)<<16|uint32(yy)] {
				if p != nil && p.InWorld && p.Char != nil && chebyshev(x, y, p.X, p.Y) <= radius {
					result = append(result, p)
				}
			}
		}
	}
	return result
}

func (w *World) recomputeMobActive(m *Mob) {
	if m == nil || m.Dead || m.Def == nil || !m.Def.IsMonster() || m.SummonerID != 0 {
		if m != nil {
			m.Awake = false
			m.TargetID = 0
			delete(w.activeMobs, m.ID)
		}
		return
	}
	awake := len(w.nearbyPlayers(m.X, m.Y, mobActivationRange)) != 0
	m.Awake = awake
	if awake {
		w.activeMobs[m.ID] = m
	} else {
		m.TargetID = 0
		delete(w.activeMobs, m.ID)
	}
}

func (w *World) updatePlayerSpatial(p *Player) {
	if p == nil || !p.InWorld || p.ID == 0 {
		return
	}
	if w.playerCells == nil {
		w.playerCells = make(map[uint32]map[uint16]*Player)
	}
	if w.playerCell == nil {
		w.playerCell = make(map[uint16]uint32)
	}
	newKey := spatialKey(p.X, p.Y)
	oldKey, existed := w.playerCell[p.ID]
	var affected = make(map[uint16]*Mob)
	if existed && oldKey == newKey {
		for _, m := range w.nearbyMobsAtCell(newKey, mobActivationRange) {
			w.recomputeMobActive(m)
		}
		return
	}
	if existed {
		for _, m := range w.nearbyMobsAtCell(oldKey, mobActivationRange) {
			affected[m.ID] = m
		}
		delete(w.playerCells[oldKey], p.ID)
		if len(w.playerCells[oldKey]) == 0 {
			delete(w.playerCells, oldKey)
		}
	}
	cell := w.playerCells[newKey]
	if cell == nil {
		cell = make(map[uint16]*Player)
		w.playerCells[newKey] = cell
	}
	cell[p.ID], w.playerCell[p.ID] = p, newKey
	for _, m := range w.nearbyMobsAtCell(newKey, mobActivationRange) {
		affected[m.ID] = m
	}
	for _, m := range affected {
		w.recomputeMobActive(m)
	}
}

func (w *World) nearbyMobsAtCell(key uint32, radius int) []*Mob {
	cx, cy := spatialCellXY(key)
	cr := radius/spatialCellSize + 1
	result := make([]*Mob, 0, 64)
	for yy := cy - cr; yy <= cy+cr; yy++ {
		for xx := cx - cr; xx <= cx+cr; xx++ {
			if xx < 0 || yy < 0 {
				continue
			}
			for _, m := range w.mobCells[uint32(xx)<<16|uint32(yy)] {
				result = append(result, m)
			}
		}
	}
	return result
}

func (w *World) unregisterPlayerSpatial(p *Player) {
	if p == nil || p.ID == 0 {
		return
	}
	key, ok := w.playerCell[p.ID]
	if !ok {
		return
	}
	affected := w.nearbyMobsAtCell(key, mobActivationRange)
	delete(w.playerCells[key], p.ID)
	if len(w.playerCells[key]) == 0 {
		delete(w.playerCells, key)
	}
	delete(w.playerCell, p.ID)
	for _, m := range affected {
		w.recomputeMobActive(m)
	}
}
