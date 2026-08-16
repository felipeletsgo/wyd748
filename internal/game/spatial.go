package game

import "strings"

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

func (w *World) nearbyPlayersInGameplaySpace(x, y uint16, radius int, space string) []*Player {
	space = strings.TrimSpace(space)
	all := w.nearbyWorldPlayers(x, y, radius)
	result := all[:0]
	now := w.now()
	for _, p := range all {
		if validMobTargetAt(p, now) && w.gameplaySpaceForPlayer(p) == space {
			result = append(result, p)
		}
	}
	return result
}

// hasNearbyPlayerInGameplaySpace answers the activation question without
// building the temporary []Player used by target-selection callers. Filters
// and exact Chebyshev radius intentionally match nearbyPlayersInGameplaySpace.
func (w *World) hasNearbyPlayerInGameplaySpace(x, y uint16, radius int, space string) bool {
	if radius < 0 {
		return false
	}
	space = strings.TrimSpace(space)
	cx, cy := int(x)/spatialCellSize, int(y)/spatialCellSize
	cr := radius/spatialCellSize + 1
	now := w.now()
	for yy := cy - cr; yy <= cy+cr; yy++ {
		for xx := cx - cr; xx <= cx+cr; xx++ {
			if xx < 0 || yy < 0 {
				continue
			}
			for _, p := range w.playerCells[uint32(xx)<<16|uint32(yy)] {
				if validMobTargetAt(p, now) && w.gameplaySpaceForPlayer(p) == space &&
					chebyshev(x, y, p.X, p.Y) <= radius {
					return true
				}
			}
		}
	}
	return false
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
	awake := false
	if m.InstanceID != "" {
		// Conteudo de instancia nunca adquire jogadores externos. O teste usa
		// MemberIDs e a area da sala, nao apenas proximidade espacial.
		awake = w.instanceMobHasNearbyMember(m, mobActivationRange)
	} else {
		awake = w.hasNearbyPlayerInGameplaySpace(m.X, m.Y, mobActivationRange, "")
	}
	m.Awake = awake
	if awake {
		w.activeMobs[m.ID] = m
	} else {
		m.TargetID = 0
		delete(w.activeMobs, m.ID)
	}
}

// recomputeMobsNearCell walks the same cell neighborhood as nearbyMobsAtCell
// without materializing a temporary slice. Player movement inside one 16-tile
// cell is the common path, so keeping this walk allocation-free avoids a burst
// of short-lived slices while preserving immediate mob wake/sleep updates.
func (w *World) recomputeMobsNearCell(key uint32, radius int) {
	cx, cy := spatialCellXY(key)
	cr := radius/spatialCellSize + 1
	for yy := cy - cr; yy <= cy+cr; yy++ {
		for xx := cx - cr; xx <= cx+cr; xx++ {
			if xx < 0 || yy < 0 {
				continue
			}
			for _, m := range w.mobCells[uint32(xx)<<16|uint32(yy)] {
				w.recomputeMobActive(m)
			}
		}
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
	if existed && oldKey == newKey {
		w.recomputeMobsNearCell(newKey, mobActivationRange)
		return
	}
	// Crossing a cell is much rarer than moving inside one. Keep the de-dup map
	// here so mobs present in both neighborhoods are still recomputed only once.
	affected := make(map[uint16]*Mob)
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

func (w *World) registerGroundItem(item *GroundItem) {
	if item == nil {
		return
	}
	if w.groundItems == nil {
		w.groundItems = make(map[uint16]*GroundItem)
	}
	if w.groundItemCells == nil {
		w.groundItemCells = make(map[uint32]map[uint16]*GroundItem)
	}
	if w.groundItemCell == nil {
		w.groundItemCell = make(map[uint16]uint32)
	}
	key := spatialKey(item.X, item.Y)
	if oldKey, indexed := w.groundItemCell[item.ID]; indexed && oldKey != key {
		delete(w.groundItemCells[oldKey], item.ID)
		if len(w.groundItemCells[oldKey]) == 0 {
			delete(w.groundItemCells, oldKey)
		}
	}
	cell := w.groundItemCells[key]
	if cell == nil {
		cell = make(map[uint16]*GroundItem)
		w.groundItemCells[key] = cell
	}
	w.groundItems[item.ID], cell[item.ID], w.groundItemCell[item.ID] = item, item, key
	w.scheduleGroundItemExpiry(item)
}

func (w *World) unregisterGroundItem(item *GroundItem) {
	if item == nil {
		return
	}
	if current := w.groundItems[item.ID]; current != nil && current != item {
		return
	}
	if key, ok := w.groundItemCell[item.ID]; ok {
		delete(w.groundItemCells[key], item.ID)
		if len(w.groundItemCells[key]) == 0 {
			delete(w.groundItemCells, key)
		}
	}
	delete(w.groundItemCell, item.ID)
	delete(w.groundExpiryByID, item.ID)
	delete(w.groundItems, item.ID)
}

func (w *World) nearbyGroundItems(x, y uint16, radius int) []*GroundItem {
	// Test fixtures and import repair may populate the canonical map directly.
	// Rebuild only on cardinality mismatch; the production hot path remains a
	// pure local-cell query.
	if len(w.groundItemCell) != len(w.groundItems) {
		for _, item := range w.groundItems {
			if _, indexed := w.groundItemCell[item.ID]; !indexed {
				w.registerGroundItem(item)
			}
		}
	}
	cx, cy := int(x)/spatialCellSize, int(y)/spatialCellSize
	cr := radius/spatialCellSize + 1
	result := make([]*GroundItem, 0, 16)
	for yy := cy - cr; yy <= cy+cr; yy++ {
		for xx := cx - cr; xx <= cx+cr; xx++ {
			if xx < 0 || yy < 0 {
				continue
			}
			for _, item := range w.groundItemCells[uint32(xx)<<16|uint32(yy)] {
				if w.groundItems[item.ID] == item && chebyshev(x, y, item.X, item.Y) <= radius {
					result = append(result, item)
				}
			}
		}
	}
	return result
}

func (w *World) registerGhostShop(shop *GhostShop) {
	if shop == nil {
		return
	}
	if w.ghostShops == nil {
		w.ghostShops = make(map[uint16]*GhostShop)
	}
	if w.ghostShopCells == nil {
		w.ghostShopCells = make(map[uint32]map[uint16]*GhostShop)
	}
	if w.ghostShopCell == nil {
		w.ghostShopCell = make(map[uint16]uint32)
	}
	key := spatialKey(shop.X, shop.Y)
	if oldKey, indexed := w.ghostShopCell[shop.ID]; indexed && oldKey != key {
		delete(w.ghostShopCells[oldKey], shop.ID)
		if len(w.ghostShopCells[oldKey]) == 0 {
			delete(w.ghostShopCells, oldKey)
		}
	}
	cell := w.ghostShopCells[key]
	if cell == nil {
		cell = make(map[uint16]*GhostShop)
		w.ghostShopCells[key] = cell
	}
	w.ghostShops[shop.ID], cell[shop.ID], w.ghostShopCell[shop.ID] = shop, shop, key
}

func (w *World) unregisterGhostShop(shop *GhostShop) {
	if shop == nil {
		return
	}
	if current := w.ghostShops[shop.ID]; current != nil && current != shop {
		return
	}
	if key, ok := w.ghostShopCell[shop.ID]; ok {
		delete(w.ghostShopCells[key], shop.ID)
		if len(w.ghostShopCells[key]) == 0 {
			delete(w.ghostShopCells, key)
		}
	}
	delete(w.ghostShopCell, shop.ID)
	delete(w.ghostShops, shop.ID)
}

func (w *World) nearbyGhostShops(x, y uint16, radius int) []*GhostShop {
	if len(w.ghostShopCell) != len(w.ghostShops) {
		for _, shop := range w.ghostShops {
			if _, indexed := w.ghostShopCell[shop.ID]; !indexed {
				w.registerGhostShop(shop)
			}
		}
	}
	cx, cy := int(x)/spatialCellSize, int(y)/spatialCellSize
	cr := radius/spatialCellSize + 1
	result := make([]*GhostShop, 0, 8)
	for yy := cy - cr; yy <= cy+cr; yy++ {
		for xx := cx - cr; xx <= cx+cr; xx++ {
			if xx < 0 || yy < 0 {
				continue
			}
			for _, shop := range w.ghostShopCells[uint32(xx)<<16|uint32(yy)] {
				if w.ghostShops[shop.ID] == shop && chebyshev(x, y, shop.X, shop.Y) <= radius {
					result = append(result, shop)
				}
			}
		}
	}
	return result
}
