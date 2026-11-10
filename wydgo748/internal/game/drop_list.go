package game

import (
	"math"
	"sort"
	"strconv"
	"strings"
	"time"
	"unicode"

	"golang.org/x/text/encoding/charmap"
	"golang.org/x/text/unicode/norm"
	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

type dropListRelation struct {
	monster, nameKey, itemKey string
	item                      uint16
	rates                     []int
	fixed                     []float64
	flags                     uint16
}

func dropSearchKey(s string) string {
	s = strings.ReplaceAll(strings.ToLower(s), "_", " ")
	folded := strings.Map(func(r rune) rune {
		if unicode.Is(unicode.Mn, r) {
			return -1
		}
		return r
	}, norm.NFD.String(s))
	return strings.Join(strings.Fields(folded), " ")
}

func (w *World) buildDropListIndex() []dropListRelation {
	result := make([]dropListRelation, 0)
	appendMob := func(def *model.NPCDef, label string, extras []model.BossDrop) {
		if def == nil || def.Score == nil || def.Tipo != model.TipoMonstro {
			return
		}
		byItem := map[uint16]*dropListRelation{}
		get := func(id uint16) *dropListRelation {
			if id == 0 || id >= 6500 {
				return nil
			}
			item, ok := w.items[id]
			if !ok {
				return nil
			}
			if byItem[id] == nil {
				byItem[id] = &dropListRelation{monster: strings.ReplaceAll(label, "_", " "), nameKey: dropSearchKey(label + " " + def.Name), itemKey: dropSearchKey(item.Name), item: id}
			}
			return byItem[id]
		}
		for slot, item := range def.Carry {
			if slot >= model.MaxCarry {
				break
			}
			r := get(item.Index)
			if r == nil {
				continue
			}
			rate, _ := model.DropRateForSlot(w.dropRates, slot, int(def.Score.Level))
			r.rates = append(r.rates, rate)
			r.flags |= 1
		}
		for _, extra := range extras {
			if r := get(extra.Item); r != nil {
				r.fixed = append(r.fixed, float64(extra.ChancePercent)/100)
				r.flags |= 2
			}
		}
		for _, r := range byItem {
			result = append(result, *r)
		}
	}
	for i := range w.npcs {
		if w.npcDefinitionState == nil || !w.npcDefinitionState.spawned[w.npcs[i].Name] {
			continue
		}
		appendMob(&w.npcs[i], w.npcs[i].VisibleName(), nil)
	}
	for _, boss := range w.bossCatalog.Bosses {
		base := w.npcDefByName(boss.NPC)
		if base == nil {
			continue
		}
		def := bossDefFrom(base, boss)
		name := def.VisibleName()
		appendMob(def, name+" [boss]", boss.Drops)
		if boss.AreaReward.Item != 0 && boss.AreaReward.Amount > 0 {
			if item, ok := w.items[boss.AreaReward.Item]; ok {
				result = append(result, dropListRelation{monster: strings.ReplaceAll(name, "_", " ") + " [area]", nameKey: dropSearchKey(name + " [area] " + def.Name), itemKey: dropSearchKey(item.Name), item: boss.AreaReward.Item, fixed: []float64{1}, flags: 8})
			}
		}
	}
	sort.Slice(result, func(i, j int) bool {
		a, b := result[i], result[j]
		if a.nameKey != b.nameKey {
			return a.nameKey < b.nameKey
		}
		return a.item < b.item
	})
	return result
}

func dropListName(name string) string {
	encoded := make([]byte, 0, 47)
	for _, r := range name {
		value, ok := charmap.Windows1252.EncodeRune(r)
		if !ok || r < 32 || r == 127 {
			value = '?'
		}
		encoded = append(encoded, value)
		if len(encoded) == 47 {
			break
		}
	}
	return string(encoded)
}

func (w *World) dropListRelations(now time.Time) []dropListRelation {
	if w.dropListIndex == nil {
		w.dropListIndex = w.buildDropListIndex()
		w.dropListEvent = nil
	}
	e := w.globalDrop
	if e == nil || !e.status.Active || (e.status.EndsAt != nil && !now.Before(*e.status.EndsAt)) ||
		(e.status.MaxDrops > 0 && e.status.Dropped+e.reserved >= e.status.MaxDrops) {
		return w.dropListIndex
	}
	item, known := w.items[e.status.ItemID]
	if !known || e.status.ItemID == 0 || e.status.ItemID >= 6500 {
		return w.dropListIndex
	}
	if w.dropListEvent == e {
		return w.dropListEventIndex
	}
	rows := append([]dropListRelation(nil), w.dropListIndex...)
	names := map[string]bool{}
	chance := float64(e.status.RatePercent) / 100
	for i := range rows {
		r := &rows[i]
		if r.flags&8 != 0 {
			continue
		}
		if r.item == e.status.ItemID {
			r.fixed = append(append([]float64(nil), r.fixed...), chance)
			r.flags |= 4
			names[r.monster] = true
		}
	}
	add := func(name, alias string) {
		name = strings.ReplaceAll(name, "_", " ")
		if names[name] {
			return
		}
		names[name] = true
		rows = append(rows, dropListRelation{monster: name, nameKey: dropSearchKey(name + " " + alias), itemKey: dropSearchKey(item.Name), item: e.status.ItemID, fixed: []float64{chance}, flags: 4})
	}
	for _, npc := range w.npcs {
		if w.npcDefinitionState != nil && w.npcDefinitionState.spawned[npc.Name] && npc.Tipo == model.TipoMonstro {
			add(npc.VisibleName(), npc.Name)
		}
	}
	for _, boss := range w.bossCatalog.Bosses {
		if base := w.npcDefByName(boss.NPC); base != nil {
			def := bossDefFrom(base, boss)
			add(def.VisibleName()+" [boss]", def.Name)
		}
	}
	sort.Slice(rows, func(i, j int) bool {
		if rows[i].nameKey != rows[j].nameKey {
			return rows[i].nameKey < rows[j].nameKey
		}
		return rows[i].item < rows[j].item
	})
	w.dropListEvent, w.dropListEventIndex = e, rows
	return rows
}

func dropListChance(r dropListRelation, bonus int) uint32 {
	miss := 1.0
	for _, base := range r.rates {
		rate := applyDropBonus(base, bonus)
		if rate <= 1 {
			return wire.DropListChanceScale
		}
		miss *= 1 - 1/float64(rate)
	}
	for _, chance := range r.fixed {
		miss *= 1 - chance
	}
	return uint32(math.Round((1 - miss) * wire.DropListChanceScale))
}

func (w *World) onDropListQuery(s *gameNet.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld {
		return
	}
	q, ok := wire.ParseDropListQuery(pkt)
	if !ok {
		return
	}
	now := w.now()
	if !p.LastDropList.IsZero() && now.Sub(p.LastDropList) < 250*time.Millisecond {
		q.Offset = 0
		s.Send(wire.DropListResult(q, 0, 0, 1, nil))
		return
	}
	p.LastDropList = now
	decoded, _ := charmap.Windows1252.NewDecoder().String(q.Query)
	key := dropSearchKey(strings.TrimSpace(decoded))
	bonus := w.dropBonusFor(p)
	relations := w.dropListRelations(now)
	numeric, numericErr := strconv.ParseUint(key, 10, 16)
	match := func(r dropListRelation) bool {
		match := key == "" || strings.Contains(r.nameKey, key)
		if q.Mode == 1 {
			match = key == "" || strings.Contains(r.itemKey, key) || (numericErr == nil && uint64(r.item) == numeric) || q.Items[r.item/8]&(1<<uint(r.item%8)) != 0
		}
		return match
	}
	var total uint32
	page := make([]dropListRelation, 0, wire.DropListPageSize)
	for _, r := range relations {
		if match(r) {
			if total >= q.Offset && len(page) < wire.DropListPageSize {
				page = append(page, r)
			}
			total++
		}
	}
	if q.Offset >= total {
		q.Offset = 0
		page = page[:0]
		for _, r := range relations {
			if match(r) {
				page = append(page, r)
				if len(page) == wire.DropListPageSize {
					break
				}
			}
		}
	}
	rows := make([]wire.DropListRow, 0, wire.DropListPageSize)
	for _, r := range page {
		name := dropListName(r.monster)
		rows = append(rows, wire.DropListRow{Item: r.item, Flags: r.flags, Base: dropListChance(r, 0), Current: dropListChance(r, bonus), Monster: name})
	}
	s.Send(wire.DropListResult(q, total, int32(bonus), 0, rows))
}
