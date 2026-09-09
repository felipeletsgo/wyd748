package game

import (
	"container/heap"
	"time"
)

type groundItemExpiry struct {
	id     uint16
	expire time.Time
}

type groundItemExpiryHeap []groundItemExpiry

func (h groundItemExpiryHeap) Len() int           { return len(h) }
func (h groundItemExpiryHeap) Less(i, j int) bool { return h[i].expire.Before(h[j].expire) }
func (h groundItemExpiryHeap) Swap(i, j int)      { h[i], h[j] = h[j], h[i] }
func (h *groundItemExpiryHeap) Push(value any)    { *h = append(*h, value.(groundItemExpiry)) }
func (h *groundItemExpiryHeap) Pop() any {
	old := *h
	last := old[len(old)-1]
	*h = old[:len(old)-1]
	return last
}

func (w *World) scheduleGroundItemExpiry(item *GroundItem) {
	if w == nil || item == nil || item.Permanent || item.Expire.IsZero() {
		return
	}
	if w.groundExpiryByID == nil {
		w.groundExpiryByID = make(map[uint16]time.Time)
	}
	if scheduled, ok := w.groundExpiryByID[item.ID]; ok && scheduled.Equal(item.Expire) {
		return
	}
	w.groundExpiryByID[item.ID] = item.Expire
	heap.Push(&w.groundExpiry, groundItemExpiry{id: item.ID, expire: item.Expire})
}

func (w *World) ensureGroundExpiryIndex() {
	if w.groundExpiryByID != nil {
		return
	}
	// Compatibilidade para restores/test fixtures construidos antes do registry.
	// O World normal inicializa o mapa e nunca executa este scan em tick.
	w.groundExpiryByID = make(map[uint16]time.Time)
	for _, item := range w.groundItems {
		w.scheduleGroundItemExpiry(item)
	}
}

func (w *World) expireGroundItems(now time.Time) {
	if w == nil {
		return
	}
	w.ensureGroundExpiryIndex()
	for len(w.groundExpiry) > 0 && !w.groundExpiry[0].expire.After(now) {
		due := heap.Pop(&w.groundExpiry).(groundItemExpiry)
		scheduled, active := w.groundExpiryByID[due.id]
		if !active || !scheduled.Equal(due.expire) {
			continue
		}
		item := w.groundItems[due.id]
		if item == nil || item.Permanent || !item.Expire.Equal(due.expire) {
			delete(w.groundExpiryByID, due.id)
			continue
		}
		w.publishItemRemove(item)
		w.unregisterGroundItem(item)
	}
}
