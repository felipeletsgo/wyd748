package game

import (
	"encoding/binary"
	"fmt"
	"maps"
	"slices"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// The 7.59 server is the historical algorithm source, not evidence of native
// 7.48 parity. Each room is a shared gameplay space; parties compete inside
// that space, never in separate copies of the same objective.
var runeTrackSlotBase = [7]int{0, 2, 5, 11, 14, 8, 18}

type runeTrackMember struct {
	ID  uint16
	UID string
}

type runeTrackParty struct {
	Slot          int
	Leader        runeTrackMember
	Members       []runeTrackMember
	Score         int
	TowerLost     bool
	PendingReward bool
	Rewarded      bool
	Gold          uint32
	Destination   [2]uint16
}

type runeTrackRuntime struct {
	Start           time.Time
	NextTick        time.Time
	Parties         []runeTrackParty
	Generators      map[uint16]int
	NextGenerate    map[int]time.Time
	Targets         [3]int
	Collected       [3]int
	HiddenGenerator int
	DangerPaused    bool
	ObjectiveWon    bool
	Settled         bool
	Ticks           int
}

func cloneRuneTrack(rt *runeTrackRuntime) *runeTrackRuntime {
	if rt == nil {
		return nil
	}
	copy := *rt
	copy.Parties = slices.Clone(rt.Parties)
	for i := range copy.Parties {
		copy.Parties[i].Members = slices.Clone(rt.Parties[i].Members)
	}
	copy.Generators = maps.Clone(rt.Generators)
	copy.NextGenerate = maps.Clone(rt.NextGenerate)
	return &copy
}

func cloneRuneInstance(inst *ItemInstance) *ItemInstance {
	return cloneCriticalInstanceMembership(&World{itemInstances: map[string]*ItemInstance{inst.RuntimeID: inst}}).itemInstances[inst.RuntimeID]
}

func runeTrackRef(p *Player) runeTrackMember { return runeTrackMember{p.ID, p.Char.UID} }

func (w *World) transferRuneTrackLeader(previous, next *Player) {
	if previous == nil || previous.Char == nil || next == nil || next.Char == nil {
		return
	}
	for _, inst := range w.itemInstances {
		if inst.RuneTrack == nil {
			continue
		}
		for i := range inst.RuneTrack.Parties {
			group := &inst.RuneTrack.Parties[i]
			if group.Leader == runeTrackRef(previous) && slices.Contains(group.Members, runeTrackRef(next)) {
				group.Leader = runeTrackRef(next)
			}
		}
	}
}

func (w *World) runeTrackPlayer(ref runeTrackMember) *Player {
	p := w.playersByID[ref.ID]
	if p == nil || !p.InWorld || p.Char == nil || ref.UID == "" || p.Char.UID != ref.UID {
		return nil
	}
	return p
}

func (w *World) runeTrackMembers(inst *ItemInstance, party *runeTrackParty) []*Player {
	var members []*Player
	for _, ref := range party.Members {
		if p := w.runeTrackPlayer(ref); p != nil && itemInstanceHasMember(inst, p.ID) &&
			w.playerRuntimeInstanceID(p.ID) == inst.RuntimeID {
			members = append(members, p)
		}
	}
	return members
}

func (w *World) runeTrackPartyFor(inst *ItemInstance, p *Player) int {
	if inst == nil || inst.RuneTrack == nil || p == nil || p.Char == nil {
		return -1
	}
	for i := range inst.RuneTrack.Parties {
		for _, ref := range inst.RuneTrack.Parties[i].Members {
			if ref == runeTrackRef(p) && itemInstanceHasMember(inst, p.ID) &&
				w.playerRuntimeInstanceID(p.ID) == inst.RuntimeID {
				return i
			}
		}
	}
	return -1
}

// runeTrackMutation stages membership, character data and positions together.
// The coordinator deep-clones RuneTrack and restores public RAM during I/O.
func (w *World) runeTrackMutation(inst *ItemInstance, members []*Player, operation string,
	mutate func(), publish func()) bool {
	if w.criticalInstanceReserved() {
		return false
	}
	before := cloneRuneInstance(inst)
	type previous struct {
		p    *Player
		ch   model.Char
		x, y uint16
	}
	old := make([]previous, 0, len(members))
	accounts := make([]*model.Account, 0, len(members))
	seen := map[*model.Account]bool{}
	for _, p := range members {
		if p == nil || p.Char == nil || p.Account == nil || w.criticalPlayerReserved(p) {
			return false
		}
		old = append(old, previous{p, cloneCharacterState(p.Char), p.X, p.Y})
		if !seen[p.Account] {
			seen[p.Account] = true
			accounts = append(accounts, p.Account)
		}
	}
	mutate()
	rollback := func() {
		*inst = *before
		for _, prior := range old {
			*prior.p.Char = prior.ch
			prior.p.X, prior.p.Y = prior.x, prior.y
		}
	}
	return w.commitInstanceMutation(operation, accounts, rollback, func() {
		w.rebuildPlayerInstanceIndex()
		w.publishInstanceMemberRelocation(members)
		if publish != nil {
			publish()
		}
	}, nil)
}

func (w *World) registerRuneTrack(s *net.Session, p *Player) {
	now := w.now()
	message := func(text string) { s.Send(wire.MessagePanel(text)) }
	if w.criticalInstanceReserved() || w.criticalPlayerReserved(p) {
		return
	}
	if now.Hour() == 0 || now.Hour() == 23 && now.Minute() >= 30 || !uxmalScheduleAllowed(w.uxmal, now) {
		message("Rune Track registration opens at minutes 16, 36 and 56, except during the daily closure.")
		return
	}
	members, ok := w.uxmalMembers(p)
	if !ok || p.Party == nil || p.Party.leader() != p || len(members) < 2 || len(members) > maxPartyMembers {
		message("The party leader and at least one member must be alive at the entrance.")
		return
	}
	for _, member := range members {
		if member.Char.UID == "" || member.Account == nil || w.itemInstanceForPlayer(member.ID) != nil {
			message("A party member is unavailable or already in another instance.")
			return
		}
		for _, current := range w.itemInstances {
			if current.RuneTrack == nil {
				continue
			}
			for _, group := range current.RuneTrack.Parties {
				for _, ref := range group.Members {
					if ref.UID == member.Char.UID {
						message("A party member is already registered.")
						return
					}
				}
			}
		}
	}
	ticket := -1
	for i := 0; i < 60; i++ {
		if p.Char.Inv[i].Index == w.uxmal.Uxmal.TicketItem {
			ticket = i
			break
		}
	}
	if ticket < 0 {
		message("You need a Rune Track ticket in the first 60 inventory slots.")
		return
	}
	room := itemSanc(p.Char.Inv[ticket])
	if room < 0 || room > 6 {
		message("That Rune Track refinement is invalid.")
		return
	}
	key := fmt.Sprintf("uxmal:%d", room)
	inst := w.itemInstances[key]
	if inst == nil {
		start, allowed := instanceScheduleEnd(w.uxmal, now)
		if !allowed {
			return
		}
		cfg := cfgCopy(*w.uxmal)
		cfg.ID = fmt.Sprintf("uxmal-room-%d", room)
		cfg.Stages = []model.VolatileInstanceStage{cfg.Stages[room]}
		inst = &ItemInstance{Config: cfg, RuntimeID: key, UxmalRoom: room, State: "entry",
			MobIDs: map[uint16]struct{}{}, NPCIDs: map[uint16]struct{}{},
			RuneTrack: &runeTrackRuntime{Start: start, Generators: map[uint16]int{}, NextGenerate: map[int]time.Time{}, Targets: [3]int{100, 100, 100}}}
		if w.itemInstances == nil {
			w.itemInstances = map[string]*ItemInstance{}
		}
		// Retain an empty aggregate on rejection so detached identity checks see
		// the same room pointer before and after admission.
		w.itemInstances[key] = inst
	}
	if inst.RuneTrack == nil || inst.State != "entry" || len(inst.RuneTrack.Parties) >= w.uxmal.Uxmal.MaxParties[room] {
		message("That Rune Track room is full or already running.")
		return
	}
	group := runeTrackParty{Slot: runeTrackSlotBase[room] + len(inst.RuneTrack.Parties), Leader: runeTrackRef(p)}
	for _, member := range members {
		group.Members = append(group.Members, runeTrackRef(member))
	}
	w.runeTrackMutation(inst, []*Player{p}, "Rune Track registration", func() {
		p.Char.Inv[ticket] = model.Item{}
		inst.RuneTrack.Parties = append(inst.RuneTrack.Parties, group)
	}, func() {
		s.Send(wire.SendItem(p.ID, placeInv, byte(ticket), p.Char.Inv[ticket]))
		message("Rune Track registration confirmed. Remain at the entrance with your party until the next 20-minute cycle.")
	})
}

func (w *World) beginRuneTrack(inst *ItemInstance) bool {
	type move struct {
		p    *Player
		x, y uint16
	}
	var moves []move
	groups := slices.Clone(inst.RuneTrack.Parties)
	claimed := map[uint16]bool{}
	reserved := map[uint32]bool{}
	for i := range groups {
		group := &groups[i]
		group.Members = nil
		leader := w.runeTrackPlayer(group.Leader)
		if leader == nil || leader.Party == nil || leader.Party.leader() != leader || playerCurHP(leader.Char) == 0 {
			continue
		}
		members, ok := w.uxmalMembers(leader)
		if !ok {
			continue
		}
		eligible := make([]*Player, 0, len(members))
		for _, p := range members {
			if p.Char.UID != "" && !claimed[p.ID] && w.itemInstanceForPlayer(p.ID) == nil {
				eligible = append(eligible, p)
			}
		}
		if len(eligible) == 0 || eligible[0] != leader {
			continue
		}
		pos := w.uxmal.Uxmal.RoomPositions[inst.UxmalRoom][i]
		ignored := map[*Player]struct{}{}
		for _, p := range eligible {
			ignored[p] = struct{}{}
		}
		// One reservation set covers all parties, including shared +3/+5 arrivals.
		for _, p := range eligible {
			var dest [2]uint16
			found := false
			for radius := 0; radius <= 6 && !found; radius++ {
				for dy := -radius; dy <= radius && !found; dy++ {
					for dx := -radius; dx <= radius; dx++ {
						x, y := int(pos.X)+dx, int(pos.Y)+dy
						if x <= 0 || y <= 0 || x > 65535 || y > 65535 {
							continue
						}
						key := uint32(x)<<16 | uint32(y)
						if reserved[key] || w.positionOccupiedInGameplaySpace(uint16(x), uint16(y), inst.RuntimeID, nil, p, ignored) {
							continue
						}
						dest, found, reserved[key] = [2]uint16{uint16(x), uint16(y)}, true, true
						break
					}
				}
			}
			if !found {
				return false
			}
			claimed[p.ID] = true
			group.Members = append(group.Members, runeTrackRef(p))
			moves = append(moves, move{p, dest[0], dest[1]})
		}
	}
	players := make([]*Player, 0, len(moves))
	for _, m := range moves {
		players = append(players, m.p)
	}
	return w.runeTrackMutation(inst, players, "Rune Track entry", func() {
		inst.RuneTrack.Parties = groups
		inst.State = "preparation"
		for _, m := range moves {
			m.p.X, m.p.Y = m.x, m.y
			m.p.Char.X, m.p.Char.Y = m.x, m.y
			inst.MemberIDs = append(inst.MemberIDs, m.p.ID)
			inst.MemberCharacterUIDs = append(inst.MemberCharacterUIDs, m.p.Char.UID)
		}
		if len(moves) > 0 {
			inst.LeaderID = moves[0].p.ID
			inst.LeaderCharacterUID = moves[0].p.Char.UID
		}
	}, func() {
		// Make the existing 7.48 merchant-9 gold prompt accessible during
		// preparation; no new packet, client menu or client-owned gold state.
		if inst.UxmalRoom == 6 {
			w.spawnRuneGenerator(inst, 5775)
		}
	})
}

func (w *World) tickRuneTrack(inst *ItemInstance, now time.Time) bool {
	if inst == nil || inst.RuneTrack == nil {
		return false
	}
	rt := inst.RuneTrack
	if rt.Settled {
		return false
	}
	if inst.State == "entry" {
		if !now.Before(rt.Start) {
			w.beginRuneTrack(inst)
		}
		return true
	}
	if now.Before(rt.Start.Add(31 * time.Second)) {
		return true
	}
	if inst.State == "preparation" {
		if !w.initializeRuneTrack(inst, now) {
			if !now.Before(rt.Start.Add(15 * time.Minute)) {
				rt.Settled = true
				inst.HardDeadline = now
				return false
			}
			return true
		}
		inst.State = "combat"
	}
	if !now.Before(rt.Start.Add(15*time.Minute)) && inst.State != "settling" {
		inst.State = "settling"
		if inst.UxmalRoom == 1 {
			for i := range rt.Parties {
				g := &rt.Parties[i]
				leader := w.runeTrackPlayer(g.Leader)
				if !g.TowerLost && leader != nil && playerCurHP(leader.Char) > 0 {
					g.PendingReward = true
				}
			}
		}
		if inst.UxmalRoom == 3 {
			best, score := -1, 0
			for i, g := range rt.Parties {
				if g.Score > score {
					best, score = i, g.Score
				}
			}
			if best >= 0 {
				rt.Parties[best].PendingReward = true
			}
		}
	}
	for i := range rt.Parties {
		group := &rt.Parties[i]
		if group.PendingReward && !group.Rewarded {
			members := w.runeTrackMembers(inst, group)
			leader := w.runeTrackPlayer(group.Leader)
			if leader == nil || !slices.Contains(members, leader) {
				group.PendingReward = false
				continue
			}
			ids := make([]uint16, 0, len(members))
			for _, p := range members {
				ids = append(ids, p.ID)
			}
			// Every member draws independently from the complete room pool.
			// Party registration slots and member order never restrict eligibility.
			runes := w.uxmal.Uxmal.Runes[inst.UxmalRoom]
			w.grantRuneRewards(inst, now, ids, leader.ID, runes, group.Gold, func() {
				inst.RuneTrack.Parties[i].Rewarded = true
				inst.RuneTrack.Parties[i].PendingReward = false
			})
			return true
		}
		if group.Destination != [2]uint16{} {
			w.moveRuneParty(inst, i, group.Destination, -1)
			return true
		}
	}
	if inst.State == "settling" {
		rt.Settled = true
		inst.HardDeadline = now
		return false
	}
	if now.Before(rt.NextTick) {
		return true
	}
	rt.NextTick = now.Add(time.Second)
	rt.Ticks++
	w.tickRunePopulation(inst, now)
	return true
}

func (w *World) moveRuneParty(inst *ItemInstance, index int, dest [2]uint16, powder int) bool {
	group := &inst.RuneTrack.Parties[index]
	members := w.runeTrackMembers(inst, group)
	if len(members) == 0 {
		group.Destination = [2]uint16{}
		return false
	}
	ignored := map[*Player]struct{}{}
	for _, p := range members {
		ignored[p] = struct{}{}
	}
	positions, ok := w.planInstancePositionsIgnoringForInstance(members, dest[0], dest[1], ignored, inst.RuntimeID)
	if !ok {
		return false
	}
	leader := w.runeTrackPlayer(group.Leader)
	if powder >= 0 && (leader == nil || !slices.Contains(members, leader) || powder >= 60 || leader.Char.Inv[powder].Index != 4032) {
		return false
	}
	return w.runeTrackMutation(inst, members, "Rune Track party move", func() {
		for i, p := range members {
			p.X, p.Y = positions[i][0], positions[i][1]
			p.Char.X, p.Char.Y = p.X, p.Y
		}
		inst.RuneTrack.Parties[index].Destination = [2]uint16{}
		if powder >= 0 {
			leader.Char.Inv[powder] = model.Item{}
		}
	}, func() {
		if powder >= 0 && leader.Session != nil {
			leader.Session.Send(wire.SendItem(leader.ID, placeInv, byte(powder), leader.Char.Inv[powder]))
		}
	})
}

func (w *World) donateRuneTrackGold(p *Player, pkt []byte) bool {
	inst := w.itemInstanceForPlayer(p.ID)
	if inst == nil || inst.RuneTrack == nil || inst.UxmalRoom != 6 {
		return false
	}
	index := w.runeTrackPartyFor(inst, p)
	if index < 0 || index >= 3 || len(pkt) < 16 {
		return true
	}
	group := &inst.RuneTrack.Parties[index]
	amount := binary.LittleEndian.Uint32(pkt[12:16])
	if group.Leader != runeTrackRef(p) || inst.State != "preparation" || inst.RuneTrack.Targets[index] != 100 ||
		amount == 0 || amount > p.Char.Gold || playerCurHP(p.Char) == 0 {
		return true
	}
	near := false
	for id, g := range inst.RuneTrack.Generators {
		m := w.mobsByID[id]
		if g == 5775 && m != nil && !m.Dead && chebyshev(p.X, p.Y, m.X, m.Y) <= mobActivationRange {
			near = true
		}
	}
	if !near {
		return true
	}
	amount = min(amount, 50_000_000)
	w.runeTrackMutation(inst, []*Player{p}, "Rune Track donation", func() {
		p.Char.Gold -= amount
		inst.RuneTrack.Targets[index] = max(50, 100-int(amount/1_000_000))
	}, func() {
		if p.Session != nil {
			p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		}
	})
	return true
}
