package game

// Scene publication under output pressure (DOCS/server/server-correction-plan.md,
// W3/D3). A full view refresh can create hundreds of entities at once. Each
// optional creation first checks room in the receiver's output queue; when the
// queue is nearly full, the entity stays unpublished and the next tick tries
// again. Removals and updates are never deferred. Updates only reach entities
// already in Visible, so a deferred entity receives nothing until created.

// sceneQueueReserve keeps a quarter of the queue for control and gameplay
// packets. The dense-arrival probe showed one tick of ordinary traffic can
// add about 30 packets to a crowded receiver.
func sceneQueueReserve(capacity int) int {
	return max(capacity/4, 1)
}

// sceneRoom reports whether p can take another optional creation now. A
// refusal marks the scene incomplete; continueIncompleteScenes resumes it.
func (w *World) sceneRoom(p *Player) bool {
	if p == nil || p.Session == nil {
		return false
	}
	if p.Session.QueueFree() > sceneQueueReserve(p.Session.QueueCapacity()) {
		return true
	}
	if !p.sceneIncomplete {
		p.sceneIncomplete = true
		metricSceneDeferred.Add(1)
	}
	return false
}

// continueIncompleteScenes repeats one full view refresh per waiting player.
// The refresh recomputes the desired set, so a teleport, runtime change or
// disconnect drops obsolete creations without losing any removal.
func (w *World) continueIncompleteScenes() {
	for _, p := range w.players {
		if p == nil || !p.sceneIncomplete {
			continue
		}
		p.sceneIncomplete = false
		if p.InWorld && p.Session != nil && !p.Session.IsClosed() {
			w.refreshPlayerVisibility(p)
		}
	}
}
