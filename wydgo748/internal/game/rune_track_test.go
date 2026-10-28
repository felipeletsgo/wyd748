package game

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"reflect"
	"slices"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func runeTrackTestWorld(t *testing.T) (*World, *Player, *Player, *guildFlowStore, *fakeClock) {
	t.Helper()
	w, leader, member, st, clock := uxmalTestWorld()
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := data.LoadVolatilesWithInstances("../../data/content/volatiles.json", "../../data/content/instances.json", "../../data/spawns/instances", catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	w.npcs, err = data.LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	cfg := volatiles.Instances["uxmal"]
	w.uxmal, w.items = &cfg, catalog.Items
	for i, p := range []*Player{leader, member} {
		p.Char.UID = fmt.Sprintf("00000000-0000-0000-0000-%012d", i+1)
		w.indexPlayerCharacter(p)
	}
	if err := w.validateUxmalConfig(); err != nil {
		t.Fatal(err)
	}
	return w, leader, member, st, clock
}

func enterRuneTrack(t *testing.T, w *World, p *Player, clock *fakeClock, room int) *ItemInstance {
	t.Helper()
	p.Char.Inv[0] = model.Item{Index: 5134}
	if !setItemSanc(&p.Char.Inv[0], room) {
		t.Fatal("cannot set ticket refinement")
	}
	w.startUxmal(p.Session, p)
	inst := w.itemInstances[fmt.Sprintf("uxmal:%d", room)]
	if inst == nil || len(inst.RuneTrack.Parties) != 1 || p.Char.Inv[0].Index != 0 {
		t.Fatal("registration failed")
	}
	clock.Advance(4 * time.Minute)
	w.tickItemInstances(clock.Now())
	if inst.State != "preparation" || w.itemInstanceForPlayer(p.ID) != inst {
		t.Fatal("entry did not attach members")
	}
	return inst
}

func runeMob(t *testing.T, w *World, inst *ItemInstance, label int) *Mob {
	t.Helper()
	for id, g := range inst.RuneTrack.Generators {
		if g == label {
			return w.mobsByID[id]
		}
	}
	if !w.spawnRuneGenerator(inst, label) {
		t.Fatalf("generator %d failed", label)
	}
	for id, g := range inst.RuneTrack.Generators {
		if g == label {
			return w.mobsByID[id]
		}
	}
	t.Fatalf("generator %d produced no actor", label)
	return nil
}

func finishRuneMob(t *testing.T, w *World, inst *ItemInstance, p *Player, label int, now time.Time) {
	t.Helper()
	// Use the actual population size: follower deaths must not complete a group.
	for attempts := 0; attempts < 64; attempts++ {
		m := runeMob(t, w, inst, label)
		m.Dead, m.HP = true, 0
		w.onRuneTrackKill(p, m, now)
		w.removeMobInstance(m)
		if w.runeGeneratorCount(inst, label) == 0 {
			return
		}
	}
	t.Fatalf("generator %d never cleared", label)
}

func TestRuneTrackProductionPopulationIsIndependent(t *testing.T) {
	w, _, _, _, _ := runeTrackTestWorld(t)
	if len(w.uxmal.Uxmal.Generators) != 327 {
		t.Fatal("incomplete population")
	}
	counts := [7]int{}
	names := map[string]bool{}
	for _, g := range w.uxmal.Uxmal.Generators {
		counts[runeGeneratorRoom(g.Label)]++
		for _, name := range []string{g.Leader, g.Follower} {
			if name == "" {
				continue
			}
			if w.npcDefByName(name) == nil {
				t.Fatalf("event references a missing template: %s", name)
			}
			names[name] = true
		}
	}
	if counts != [7]int{53, 60, 60, 28, 50, 49, 27} || len(names) != 30 {
		t.Fatalf("population mismatch: %v, %d templates", counts, len(names))
	}
	lich := w.npcDefByName("RT_LichKing")
	if lich.ExpReward != 8500 || lich.Score.Level != 499 || lich.Score.Defense != 5350 || lich.Score.Attack != 5600 || lich.Score.MaxHP != 32000 {
		t.Fatalf("historical stats changed: %+v", lich)
	}
	broken := cfgCopy(*w.uxmal)
	broken.Uxmal.Generators[0].Leader = "MissingRuneNPC"
	w.uxmal = &broken
	if err := w.validateUxmalConfig(); err == nil {
		t.Fatal("accepted a missing monster reference")
	}
}

func TestRuneTrackBootRejectsRenumberedPublicCopies(t *testing.T) {
	fixtures, err := eventFixtures()
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := data.LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	cfg := fixtures.volatiles.Instances["uxmal"]
	var copies []model.NPCGener
	for _, g := range cfg.Uxmal.Generators {
		if g.Label == 5662 || g.Label == 5985 || g.Label == 5986 {
			g.Index = len(copies)
			g.Label += 10000 // Imports can assign a fresh decorative label.
			if g.Leader == "RT_LichKing" {
				g.Leader, g.Follower = "LichCruntKing", "LichCruntKing"
			}
			copies = append(copies, g)
		}
	}
	general, err := data.LoadChannelNPCGener("../../data/spawns/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	for _, g := range general {
		if g.Leader == "Uxmal" {
			g.Index = len(copies)
			copies = append(copies, g)
			break
		}
	}
	if len(copies) != 4 {
		t.Fatal("missing event/public guard fixtures")
	}
	w, err := NewWorld(&craftStore{}, npcs, copies, model.Catalog{Items: fixtures.items},
		[model.MaxCarry]int{}, fixtures.volatiles, model.CharacterTemplateFile{}, fixtures.terrain,
		WithUxmal(cfg), WithNPCGenerLog("quiet"))
	if err != nil {
		t.Fatal(err)
	}
	for _, g := range copies[:3] {
		if w.reservedGenerators[g.Index] != "rune_track" {
			t.Fatalf("renumbered generator %d entered public population", g.Label)
		}
	}
	if len(w.generators) != 1 || len(w.mobsByID) != 1 {
		t.Fatalf("public startup population includes event actors: generators=%d mobs=%d", len(w.generators), len(w.mobsByID))
	}
	for _, mob := range w.mobsByID {
		if mob.Def.Name != "Uxmal" || mob.InstanceID != "" {
			t.Fatal("public entrance NPC was lost")
		}
	}
	// A loaded event table does not create any room until registration/activation.
	if len(w.itemInstances) != 0 {
		t.Fatal("Rune Track started without player entry")
	}
}

func TestRuneTrackRegistrationEntryAndRollback(t *testing.T) {
	for _, reject := range []bool{false, true} {
		t.Run(fmt.Sprint(reject), func(t *testing.T) {
			w, p, member, st, clock := runeTrackTestWorld(t)
			old := p.Char.Inv[0]
			if reject {
				st.err = errors.New("registration rejected")
			}
			w.startUxmal(p.Session, p)
			inst := w.itemInstances["uxmal:0"]
			if inst == nil {
				t.Fatal("missing room aggregate")
			}
			if reject {
				if len(inst.RuneTrack.Parties) != 0 || p.Char.Inv[0] != old {
					t.Fatal("failed registration consumed ticket")
				}
				return
			}
			if p.X != 3280 || member.X != 3281 || len(inst.MemberIDs) != 0 {
				t.Fatal("registration teleported early")
			}
			clock.Advance(4 * time.Minute)
			st.err = errors.New("entry rejected")
			w.tickItemInstances(clock.Now())
			if inst.State != "entry" || p.X != 3280 || len(inst.MemberIDs) != 0 {
				t.Fatal("entry failure leaked membership or position")
			}
			st.err = nil
			w.tickItemInstances(clock.Now())
			if inst.State != "preparation" || w.runeTrackPartyFor(inst, member) != 0 {
				t.Fatal("entry retry failed")
			}
			member.Char.UID = "reused-id"
			if w.runeTrackPartyFor(inst, member) != -1 {
				t.Fatal("reused entity ID inherited event rights")
			}
		})
	}
}

func TestRuneTrackEmptyEntryStaysPending(t *testing.T) {
	w, leader, _, _, _ := runeTrackTestWorld(t)
	leader.Char.Inv[0] = model.Item{Index: 5134}
	if !setItemSanc(&leader.Char.Inv[0], 0) {
		t.Fatal("cannot set ticket refinement")
	}
	w.startUxmal(leader.Session, leader)
	inst := w.itemInstances["uxmal:0"]
	if inst == nil {
		t.Fatal("registration did not create the Rune Track room")
	}

	// The registered party becomes invalid before the cycle begins. No member
	// may be attached and the room must remain retryable in the entry state.
	leader.Char.UID = ""
	if w.beginRuneTrack(inst) {
		t.Fatal("an empty Rune Track entry was reported as committed")
	}
	if inst.State != "entry" || len(inst.MemberIDs) != 0 || len(inst.RuneTrack.Parties) != 1 {
		t.Fatalf("empty Rune Track entry mutated the room: state=%q members=%d parties=%d",
			inst.State, len(inst.MemberIDs), len(inst.RuneTrack.Parties))
	}
}

func TestRuneTrackSharedArrivalsAndFourPartyCapacity(t *testing.T) {
	for _, room := range []int{3, 4, 5} {
		t.Run(fmt.Sprint(room), func(t *testing.T) {
			w, p, member, _, clock := runeTrackTestWorld(t)
			p.Char.Inv[0] = model.Item{Index: 5134}
			setItemSanc(&p.Char.Inv[0], room)
			w.startUxmal(p.Session, p)
			inst := w.itemInstances[fmt.Sprintf("uxmal:%d", room)]
			capacity := w.uxmal.Uxmal.MaxParties[room]
			// Distinct registered references use the same native +3/+5 destination.
			for n := 1; n < capacity; n++ {
				a, _ := networkedTestPlayer(uint16(n*2+1), fmt.Sprintf("Leader%d", n), 3280, uint16(1680+n))
				b, _ := networkedTestPlayer(uint16(n*2+2), fmt.Sprintf("Member%d", n), 3281, uint16(1680+n))
				a.Char.UID, b.Char.UID = fmt.Sprint("leader-", n), fmt.Sprint("member-", n)
				party := &Party{Members: []*Player{a, b}}
				a.Party, b.Party = party, party
				for _, player := range []*Player{a, b} {
					w.players[player.Session] = player
					w.playersByID[player.ID] = player
					w.updatePlayerSpatial(player)
				}
				a.Char.Inv[0] = model.Item{Index: 5134}
				setItemSanc(&a.Char.Inv[0], room)
				w.startUxmal(a.Session, a)
			}
			if len(inst.RuneTrack.Parties) != capacity {
				t.Fatalf("capacity %d not admitted: %d", capacity, len(inst.RuneTrack.Parties))
			}
			clock.Advance(4 * time.Minute)
			w.tickItemInstances(clock.Now())
			seen := map[[2]uint16]bool{}
			for _, id := range inst.MemberIDs {
				player := w.playersByID[id]
				tile := [2]uint16{player.X, player.Y}
				if seen[tile] {
					t.Fatal("arrival collision")
				}
				seen[tile] = true
			}
			if len(seen) != capacity*2 || w.runeTrackPartyFor(inst, p) != 0 || w.runeTrackPartyFor(inst, member) != 0 {
				t.Fatal("shared room membership mismatch")
			}
			w.transferRuneTrackLeader(p, member)
			if inst.RuneTrack.Parties[0].Leader != runeTrackRef(member) {
				t.Fatal("leader transfer lost event slot")
			}
		})
	}
}

func TestRuneTrackRoomObjectives(t *testing.T) {
	for room := 0; room < 7; room++ {
		t.Run(fmt.Sprint(room), func(t *testing.T) {
			w, p, member, _, clock := runeTrackTestWorld(t)
			inst := enterRuneTrack(t, w, p, clock, room)
			clock.Advance(31 * time.Second)
			w.tickItemInstances(clock.Now())
			if inst.State != "combat" {
				t.Fatal("population did not initialize")
			}
			w.rng = zeroRNG{}
			switch room {
			case 0:
				finishRuneMob(t, w, inst, p, 5662, clock.Now())
			case 1:
				if m := runeMob(t, w, inst, 5715); m.Def.IsMonster() {
					t.Fatal("tower is hostile to players")
				}
				clock.Advance(15 * time.Minute)
				w.tickItemInstances(clock.Now())
			case 2:
				if inst.BossSpawned {
					t.Fatal("Amon spawned before five minutes")
				}
				clock.Advance(5 * time.Minute)
				w.tickItemInstances(clock.Now())
				if !inst.BossSpawned {
					t.Fatal("Amon did not spawn")
				}
				finishRuneMob(t, w, inst, p, 5798, clock.Now())
			case 3:
				finishRuneMob(t, w, inst, p, 5981, clock.Now())
				finishRuneMob(t, w, inst, p, 5957, clock.Now())
				if inst.RuneTrack.Parties[0].Score != 1 {
					t.Fatal("mage kill did not score")
				}
				clock.Advance(15 * time.Minute)
				w.tickItemInstances(clock.Now())
			case 4:
				label := inst.RuneTrack.HiddenGenerator
				finishRuneMob(t, w, inst, p, label, clock.Now())
				if inst.RuneTrack.Parties[0].Destination != [2]uint16{3351, 1335} {
					t.Fatal("hidden maze group did not open passage")
				}
				finishRuneMob(t, w, inst, p, 5858, clock.Now())
			case 5:
				finishRuneMob(t, w, inst, p, 5908, clock.Now())
			case 6:
				inst.RuneTrack.Targets = [3]int{1, 100, 100}
				finishRuneMob(t, w, inst, p, 5784, clock.Now())
				finishRuneMob(t, w, inst, p, 5776, clock.Now())
				if inst.RuneTrack.Targets != [3]int{} || inst.RuneTrack.Parties[0].Gold != 500000 {
					t.Fatal("rabbit settlement formula mismatch")
				}
			}
			w.tickItemInstances(clock.Now())
			if !inst.RuneTrack.Parties[0].Rewarded {
				t.Fatal("objective did not reward winning party")
			}
			for _, player := range []*Player{p, member} {
				found := false
				for _, item := range player.Char.Inv {
					found = found || slices.Contains(w.uxmal.Uxmal.Runes[room], item.Index)
				}
				if !found {
					t.Fatal("member did not receive a rune from the room pool")
				}
			}
			if room < 6 && (p.Char.Inv[1].Index != 5134 || itemSanc(p.Char.Inv[1]) != room+1) {
				t.Fatal("leader did not receive next ticket")
			}
		})
	}
}

type runeTrackRewardRNG struct {
	t     *testing.T
	pool  int
	draws []int
	calls int
}

func (r *runeTrackRewardRNG) Intn(n int) int {
	r.t.Helper()
	if n != r.pool || r.calls >= len(r.draws) {
		r.t.Fatalf("unexpected reward draw: pool=%d, calls=%d", n, r.calls)
	}
	value := r.draws[r.calls]
	r.calls++
	return value
}

func TestRuneTrackRewardsUseFullRoomPoolForSmallParties(t *testing.T) {
	// Independent literal pools guard against changing the configured reward set.
	pools := [][]uint16{
		{5110, 5111, 5112, 5113, 5114},
		{5112, 5113, 5114, 5115, 5116, 5117},
		{5115, 5116, 5117, 5118, 5119, 5120},
		{5121, 5122, 5123, 5124, 5125, 5126},
		{5124, 5125, 5126, 5127, 5128, 5129, 5130},
		{5118, 5119, 5120, 5121, 5122, 5123},
		{5128, 5129, 5130, 5131, 5132, 5133},
	}
	for room, pool := range pools {
		t.Run(fmt.Sprintf("room+%d", room), func(t *testing.T) {
			w, leader, member, _, clock := runeTrackTestWorld(t)
			if !slices.Equal(w.uxmal.Uxmal.Runes[room], pool) {
				t.Fatal("configured room pool changed")
			}
			third, _ := networkedTestPlayer(3, "Third", 3282, 1680)
			third.Char.UID, third.Party = "reward-third", leader.Party
			leader.Party.Members = append(leader.Party.Members, third)
			w.players[third.Session], w.playersByID[third.ID] = third, third
			w.updatePlayerSpatial(third)
			players := []*Player{leader, member, third}
			inst := enterRuneTrack(t, w, leader, clock, room)
			clock.Advance(31 * time.Second)
			w.tickItemInstances(clock.Now())
			base := cloneRuneInstance(inst)
			chars := make([]model.Char, len(players))
			for i, p := range players {
				chars[i] = cloneCharacterState(p.Char)
			}
			for size := 1; size <= len(players); size++ {
				for slot := 0; slot < w.uxmal.Uxmal.MaxParties[room]; slot++ {
					// Every position can draw every rune; the final round allows duplicates.
					for roll := 0; roll <= len(pool); roll++ {
						*inst = *cloneRuneInstance(base)
						for i, p := range players {
							*p.Char = cloneCharacterState(&chars[i])
						}
						group := &inst.RuneTrack.Parties[0]
						group.Slot = runeTrackSlotBase[room] + slot
						group.PendingReward = true
						group.Members = nil
						ordered := make([]*Player, size)
						for i := range ordered {
							ordered[i] = players[(i+roll)%size]
							group.Members = append(group.Members, runeTrackRef(ordered[i]))
						}
						rng := &runeTrackRewardRNG{t: t, pool: len(pool)}
						for i := range ordered {
							draw := (roll + i) % len(pool)
							if roll == len(pool) {
								draw = len(pool) - 1
							}
							rng.draws = append(rng.draws, draw)
						}
						w.rng = rng
						w.tickRuneTrack(inst, clock.Now())
						if !inst.RuneTrack.Parties[0].Rewarded || rng.calls != size {
							t.Fatalf("size=%d slot=%d: missing independent member draws", size, slot)
						}
						for i, p := range ordered {
							if p.Char.Inv[0].Index != pool[rng.draws[i]] {
								t.Fatalf("size=%d slot=%d roll=%d member=%d: rune=%d, want=%d",
									size, slot, roll, i, p.Char.Inv[0].Index, pool[rng.draws[i]])
							}
						}
						for _, p := range players[size:] {
							if !reflect.DeepEqual(*p.Char, chars[p.ID-1]) {
								t.Fatal("nonparticipating player received a reward")
							}
						}
						before := rng.calls
						w.tickRuneTrack(inst, clock.Now())
						if rng.calls != before {
							t.Fatal("completed party drew another reward")
						}
					}
				}
			}
		})
	}
}

func TestRuneTrackFreyjaNativeGoldRequest(t *testing.T) {
	w, p, member, _, clock := runeTrackTestWorld(t)
	inst := enterRuneTrack(t, w, p, clock, 6)
	var freyja *Mob
	for id, label := range inst.RuneTrack.Generators {
		if label == 5775 {
			freyja = w.mobsByID[id]
			break
		}
	}
	if freyja == nil || freyja.Dead || freyja.ID < 1000 {
		t.Fatal("Freyja was not spawned as an NPC during preparation")
	}
	p.X, p.Y = freyja.X+1, freyja.Y
	p.Char.X, p.Char.Y = p.X, p.Y
	w.updatePlayerSpatial(p)
	for p.Session.QueuedPacketsForTest() > 0 {
		p.Session.DequeuePacketForTest()
	}
	p.hide(freyja.ID)
	w.showMob(p, freyja)
	spawn, ok := p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(spawn) || wire.ParseHeader(spawn).Type != wire.OpCreateMob ||
		binary.LittleEndian.Uint16(spawn[16:18]) != freyja.ID ||
		binary.LittleEndian.Uint16(spawn[34:36]) != 51 ||
		binary.LittleEndian.Uint32(spawn[276:280])&15 != 9 {
		t.Fatal("Freyja publication lacks the head-51/merchant-9 gold prompt identity")
	}
	request := func(actor *Player, amount uint32) {
		t.Helper()
		packet := inboundPacket(wire.OpStoreGoldRequest, 16)
		binary.LittleEndian.PutUint16(packet[6:8], actor.ID)
		binary.LittleEndian.PutUint32(packet[12:16], amount)
		if !w.validateInboundCommand(actor.Session, packet) {
			t.Fatal("native 16-byte 0x3D1 rejected at ingress")
		}
		w.handle(command{s: actor.Session, pkt: packet})
	}
	p.Char.Gold, member.Char.Gold = 60_000_000, 60_000_000
	request(member, 50_000_000)
	if member.Char.Gold != 60_000_000 || inst.RuneTrack.Targets[0] != 100 {
		t.Fatal("a non-leader donation changed gold or target")
	}
	request(p, 60_000_001)
	if p.Char.Gold != 60_000_000 || inst.RuneTrack.Targets[0] != 100 {
		t.Fatal("a donation exceeding carried gold was accepted")
	}
	request(p, 50_000_000)
	if p.Char.Gold != 10_000_000 || inst.RuneTrack.Targets[0] != 50 {
		t.Fatal("native gold confirmation did not reach the room donation handler")
	}
	goldUpdates := 0
	for p.Session.QueuedPacketsForTest() > 0 {
		reply, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(reply) {
			t.Fatal("invalid donation publication")
		}
		if wire.ParseHeader(reply).Type == wire.OpUpdateEtc {
			goldUpdates++
		}
	}
	if goldUpdates != 1 {
		t.Fatalf("committed donation published %d gold updates, want one", goldUpdates)
	}
	request(p, 1_000_000)
	if p.Char.Gold != 10_000_000 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("repeated confirmation charged or published gold twice")
	}
}

func TestRuneTrackDonationAndRewardRollback(t *testing.T) {
	w, p, _, st, clock := runeTrackTestWorld(t)
	inst := enterRuneTrack(t, w, p, clock, 6)
	freyja := runeMob(t, w, inst, 5775)
	p.X, p.Y = freyja.X+1, freyja.Y
	p.Char.X, p.Char.Y = p.X, p.Y
	w.updatePlayerSpatial(p)
	p.Char.Gold = 60_000_000
	packet := make([]byte, 16)
	binary.LittleEndian.PutUint32(packet[12:], 50_000_000)
	st.err = errors.New("donation rejected")
	w.donateRuneTrackGold(p, packet)
	if p.Char.Gold != 60_000_000 || inst.RuneTrack.Targets[0] != 100 {
		t.Fatal("failed donation leaked gold or target")
	}
	st.err = nil
	w.donateRuneTrackGold(p, packet)
	if p.Char.Gold != 10_000_000 || inst.RuneTrack.Targets[0] != 50 {
		t.Fatal("donation did not lower target")
	}
	w.donateRuneTrackGold(p, packet)
	if p.Char.Gold != 10_000_000 {
		t.Fatal("repeat donation charged gold")
	}
	inst.RuneTrack.Parties[0].PendingReward = true
	inst.RuneTrack.Parties[0].Gold = 5_000_000
	clock.Advance(31 * time.Second)
	before := cloneCharacterState(p.Char)
	st.err = errors.New("reward rejected")
	w.tickItemInstances(clock.Now())
	if inst.RuneTrack.Parties[0].Rewarded || !reflect.DeepEqual(*p.Char, before) {
		t.Fatal("failed reward leaked state")
	}
	st.err = nil
	w.tickItemInstances(clock.Now())
	if !inst.RuneTrack.Parties[0].Rewarded || p.Char.Gold != 15_000_000 {
		t.Fatal("reward retry did not settle")
	}
	before = cloneCharacterState(p.Char)
	w.tickItemInstances(clock.Now())
	if !reflect.DeepEqual(*p.Char, before) {
		t.Fatal("duplicate reward")
	}
}

type zeroRNG struct{}

func (zeroRNG) Intn(int) int { return 0 }

func TestRuneTrackInstalledTerrainAndCleanup(t *testing.T) {
	terrain, err := data.LoadTerrain("../../data/world/maps/HeightMap.dat", "../../data/world/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	for room := 0; room < 7; room++ {
		t.Run(fmt.Sprint(room), func(t *testing.T) {
			w, p, member, _, clock := runeTrackTestWorld(t)
			w.terrain = terrain
			if err := w.validateUxmalConfig(); err != nil {
				t.Fatal(err)
			}
			inst := enterRuneTrack(t, w, p, clock, room)
			if room == 0 {
				for _, label := range []int{5662, 5663} {
					if w.runeGeneratorCount(inst, label) != 0 {
						t.Fatalf("Lich King group %d spawned before room combat", label)
					}
				}
			}
			clock.Advance(31 * time.Second)
			w.tickItemInstances(clock.Now())
			if inst.State != "combat" {
				t.Fatalf("installed population failed to initialize room %d", room)
			}
			if room == 0 {
				for _, label := range []int{5662, 5663} {
					if got := w.runeGeneratorCount(inst, label); got != 9 {
						t.Fatalf("Lich King group %d has %d actors, want 9", label, got)
					}
				}
			}
			for id := range inst.RuneTrack.Generators {
				m := w.mobsByID[id]
				if m == nil || !terrain.Walkable(m.X, m.Y) || m.InstanceID != inst.RuntimeID {
					t.Fatal("spawn has no valid event terrain or identity")
				}
			}
			clock.Advance(16 * time.Minute)
			for range 8 {
				w.tickItemInstances(clock.Now())
			}
			if w.itemInstances[inst.RuntimeID] != nil || w.itemInstanceForPlayer(p.ID) != nil ||
				w.itemInstanceForPlayer(member.ID) != nil {
				t.Fatal("expired room retained membership")
			}
			for _, m := range w.mobsByID {
				if m.InstanceID == inst.RuntimeID {
					t.Fatal("expired room leaked population")
				}
			}
			if !uxmalInsideEntry(w.uxmal, p.X, p.Y) || !uxmalInsideEntry(w.uxmal, member.X, member.Y) {
				t.Fatal("cleanup did not return members to the entrance")
			}
		})
	}
}

func TestRuneTrackFloorPulseAndPortalRollback(t *testing.T) {
	w, p, member, st, clock := runeTrackTestWorld(t)
	inst := enterRuneTrack(t, w, p, clock, 4)
	p.X, p.Y = 3455, 1310
	member.X, member.Y = 3440, 1310
	effectiveScore(member.Char).MaxHP = 8000
	setPlayerCurHP(member.Char, 8000)
	w.runeTrackFloorDamage(inst)
	if playerCurHP(member.Char) != 6000 {
		t.Fatalf("floor pulse HP=%d", playerCurHP(member.Char))
	}
	found := false
	for member.Session.QueuedPacketsForTest() > 0 {
		packet, _ := member.Session.DequeuePacketForTest()
		decoded, checksum, err := wire.ReadPacket(bytes.NewReader(packet))
		if err != nil || !checksum {
			t.Fatalf("invalid outbound frame: %v", err)
		}
		found = found || wire.ParseHeader(decoded).Type == 0x3A2
	}
	if !found {
		t.Fatal("floor damage omitted its native visual")
	}
	p.X, p.Y = member.X+1, member.Y
	w.runeTrackFloorDamage(inst)
	if playerCurHP(member.Char) != 6000 {
		t.Fatal("leader did not protect rectangle")
	}
	// A public-space player at the same tile cannot be damaged by this event.
	inst.MemberIDs = []uint16{p.ID}
	w.rebuildPlayerInstanceIndex()
	p.X = 3455
	w.runeTrackFloorDamage(inst)
	if playerCurHP(member.Char) != 6000 {
		t.Fatal("floor pulse crossed runtime boundary")
	}
	// The Balrog portal consumes the complete powder slot only after commit.
	w, p, member, st, clock = runeTrackTestWorld(t)
	inst = enterRuneTrack(t, w, p, clock, 5)
	p.X, p.Y = 3363, 1224
	p.Char.X, p.Char.Y = p.X, p.Y
	p.Char.Inv[3] = model.Item{Index: 4032}
	beforeP, beforeM := cloneCharacterState(p.Char), cloneCharacterState(member.Char)
	st.err = errors.New("portal rejected")
	w.tickRunePortals(inst)
	if !reflect.DeepEqual(*p.Char, beforeP) || !reflect.DeepEqual(*member.Char, beforeM) || p.X != 3363 {
		t.Fatal("failed portal consumed powder or moved players")
	}
	st.err, w.rng = nil, zeroRNG{}
	w.tickRunePortals(inst)
	if p.Char.Inv[3].Index != 0 || chebyshev(p.X, p.Y, 3390, 1250) > 4 || chebyshev(member.X, member.Y, 3390, 1250) > 4 {
		t.Fatal("portal did not move the complete party and consume powder")
	}
}

func TestRuneTrackAllBalrogPortalRoutesOnInstalledTerrain(t *testing.T) {
	terrain, err := data.LoadTerrain("../../data/world/maps/HeightMap.dat", "../../data/world/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	// Keep the historical inputs independent of the production table and switch.
	rects := [10][4]uint16{
		{3429, 1197, 3432, 1200}, {3379, 1178, 3381, 1180}, {3379, 1166, 3381, 1168},
		{3362, 1223, 3364, 1225}, {3351, 1223, 3354, 1225}, {3340, 1223, 3342, 1225},
		{3402, 1237, 3404, 1239}, {3394, 1237, 3396, 1239}, {3385, 1237, 3388, 1239}, {3378, 1237, 3380, 1239},
	}
	if runeTrackPortals != rects {
		t.Fatal("Balrog portal rectangles differ from the historical configuration")
	}
	for portal, rect := range rects {
		t.Run(fmt.Sprintf("portal_%d", portal), func(t *testing.T) {
			w, p, member, _, clock := runeTrackTestWorld(t)
			w.terrain = terrain
			inst := enterRuneTrack(t, w, p, clock, 5)
			clock.Advance(31 * time.Second)
			w.tickItemInstances(clock.Now())
			// Isolate portal draws from unrelated monster generation randomness.
			clear(inst.RuneTrack.NextGenerate)
			var source [2]uint16
			for y := rect[1]; y <= rect[3] && source == [2]uint16{}; y++ {
				for x := rect[0]; x <= rect[2]; x++ {
					if terrain.Walkable(x, y) {
						source = [2]uint16{x, y}
						break
					}
				}
			}
			if source == [2]uint16{} {
				t.Fatal("portal has no walkable activation tile")
			}
			var destinations [][2]uint16
			switch portal {
			case 0:
				destinations = [][2]uint16{{3428, 1208}, {3388, 1175}}
			case 1, 2:
				destinations = [][2]uint16{{3356, 1209}, {3388, 1175}, {3388, 1175}, {3388, 1175}}
			case 3, 4, 5:
				destinations = [][2]uint16{{3390, 1250}, {3356, 1209}, {3356, 1209}, {3356, 1209}, {3356, 1209}, {3356, 1209}, {3428, 1208}}
			default:
				destinations = [][2]uint16{{3396, 1216}, {3390, 1250}, {3390, 1250}, {3428, 1208}, {3390, 1250}, {3390, 1250}, {3390, 1250}, {3390, 1250}, {3388, 1175}, {3390, 1250}}
			}
			for roll, dest := range destinations {
				t.Run(fmt.Sprintf("roll_%d", roll), func(t *testing.T) {
					p.X, p.Y = source[0], source[1]
					p.Char.X, p.Char.Y = p.X, p.Y
					p.Char.Inv[59] = model.Item{Index: 4032}
					w.updatePlayerSpatial(p)
					for _, player := range []*Player{p, member} {
						for player.Session.QueuedPacketsForTest() > 0 {
							player.Session.DequeuePacketForTest()
						}
					}
					rng := &runeTrackRewardRNG{t: t, pool: len(destinations), draws: []int{roll}}
					w.rng = rng
					inst.RuneTrack.Ticks = 13
					clock.Advance(time.Second)
					w.tickItemInstances(clock.Now())
					if p.X != source[0] || p.Y != source[1] || p.Char.Inv[59].Index != 4032 || rng.calls != 0 {
						t.Fatal("portal activated before its scheduled pulse")
					}
					clock.Advance(time.Second)
					w.tickItemInstances(clock.Now())
					if rng.calls != 1 || p.Char.Inv[59].Index != 0 {
						t.Fatal("scheduled portal did not draw once and consume the powder")
					}
					for _, player := range []*Player{p, member} {
						if chebyshev(player.X, player.Y, dest[0], dest[1]) > 4 || !terrain.Walkable(player.X, player.Y) ||
							player.Char.X != player.X || player.Char.Y != player.Y || w.itemInstanceForPlayer(player.ID) != inst {
							t.Fatalf("player %d did not reach valid destination %v: (%d,%d)", player.ID, dest, player.X, player.Y)
						}
						found := make(map[uint16]bool)
						for player.Session.QueuedPacketsForTest() > 0 {
							packet, _ := player.Session.DequeuePacketForTest()
							decoded, checksum, err := wire.ReadPacket(bytes.NewReader(packet))
							if err != nil || !checksum {
								t.Fatalf("invalid relocation frame: %v", err)
							}
							for _, subject := range []*Player{p, member} {
								want := wire.ActionStop(subject.ID, subject.X, subject.Y)
								// Framing, encryption keys, and send timestamps are transport-owned.
								if len(decoded) == len(want) && bytes.Equal(decoded[4:8], want[4:8]) && bytes.Equal(decoded[12:], want[12:]) {
									found[subject.ID] = true
								}
							}
						}
						if !found[p.ID] || !found[member.ID] {
							t.Fatalf("player %d did not receive both party relocation packets: %v", player.ID, found)
						}
					}
					if p.X == member.X && p.Y == member.Y {
						t.Fatal("portal stacked party members on the same tile")
					}
				})
			}
		})
	}
}

func TestRuneTrackBalrogPortalRejectsIneligibleActivation(t *testing.T) {
	for _, scenario := range []string{"no_powder", "powder_outside_first_60_slots", "dead_leader", "member_only"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, member, _, clock := runeTrackTestWorld(t)
			inst := enterRuneTrack(t, w, p, clock, 5)
			p.X, p.Y = 3363, 1224
			p.Char.Inv[0] = model.Item{Index: 4032}
			switch scenario {
			case "no_powder":
				p.Char.Inv[0] = model.Item{}
			case "powder_outside_first_60_slots":
				p.Char.Inv[60], p.Char.Inv[0] = p.Char.Inv[0], model.Item{}
			case "dead_leader":
				setPlayerCurHP(p.Char, 0)
			case "member_only":
				p.X, p.Y = 3428, 1208
				member.X, member.Y = 3363, 1224
			}
			beforeP, beforeM := cloneCharacterState(p.Char), cloneCharacterState(member.Char)
			x, y := p.X, p.Y
			w.tickRunePortals(inst)
			if !reflect.DeepEqual(*p.Char, beforeP) || !reflect.DeepEqual(*member.Char, beforeM) || p.X != x || p.Y != y {
				t.Fatal("ineligible portal activation changed the party or inventory")
			}
		})
	}
}

func TestRuneTrackMazePassageMovesPartyOnInstalledTerrain(t *testing.T) {
	w, p, member, st, clock := runeTrackTestWorld(t)
	var err error
	w.terrain, err = data.LoadTerrain("../../data/world/maps/HeightMap.dat", "../../data/world/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	inst := enterRuneTrack(t, w, p, clock, 4)
	clock.Advance(31 * time.Second)
	w.tickItemInstances(clock.Now())
	finishRuneMob(t, w, inst, p, inst.RuneTrack.HiddenGenerator, clock.Now())
	beforeP, beforeM := cloneCharacterState(p.Char), cloneCharacterState(member.Char)
	st.err = errors.New("maze teleport rejected")
	w.tickItemInstances(clock.Now())
	if !reflect.DeepEqual(*p.Char, beforeP) || !reflect.DeepEqual(*member.Char, beforeM) ||
		inst.RuneTrack.Parties[0].Destination != [2]uint16{3351, 1335} {
		t.Fatal("failed maze teleport changed the party or lost the pending destination")
	}
	st.err = nil
	w.tickItemInstances(clock.Now())
	if inst.RuneTrack.Parties[0].Destination != [2]uint16{} || w.runeGeneratorCount(inst, 5858) == 0 {
		t.Fatal("maze passage did not finish or Shadow is missing")
	}
	for _, player := range []*Player{p, member} {
		if chebyshev(player.X, player.Y, 3351, 1335) > 4 || !w.terrain.Walkable(player.X, player.Y) ||
			player.Char.X != player.X || player.Char.Y != player.Y || w.itemInstanceForPlayer(player.ID) != inst {
			t.Fatalf("maze member %d did not reach Shadow", player.ID)
		}
	}
	if p.X == member.X && p.Y == member.Y {
		t.Fatal("maze passage stacked party members")
	}
}

func TestRuneTrackQueuedRewardVisibility(t *testing.T) {
	for _, outcome := range []string{"commit", "rejected", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			w, p, member, _, clock := runeTrackTestWorld(t)
			inst := enterRuneTrack(t, w, p, clock, 0)
			claimCriticalTestPlayers(t, w, p, member)
			clock.Advance(31 * time.Second)
			w.tickItemInstances(clock.Now())
			inst.RuneTrack.Parties[0].PendingReward = true
			for _, player := range []*Player{p, member} {
				w.refreshPlayerVisibility(player)
				for player.Session.QueuedPacketsForTest() > 0 {
					player.Session.DequeuePacketForTest()
				}
			}
			beforeP, beforeM := cloneCharacterState(p.Char), cloneCharacterState(member.Char)
			gate, release := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate}
			if outcome == "rejected" {
				base.failure = criticalCertifiedRejection(t)
			} else if outcome == "unknown" {
				base.failure = errors.New("reward acknowledgement lost")
			}
			q := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(q)
			w.tickItemInstances(clock.Now())
			proposal := awaitCriticalWrite(t, base.entered)
			if proposal.instances == nil || len(proposal.accounts) != 2 {
				t.Fatal("reward was not one account-and-instance proposal")
			}
			assertHidden := func() {
				t.Helper()
				if inst.RuneTrack.Parties[0].Rewarded || !inst.RuneTrack.Parties[0].PendingReward ||
					!reflect.DeepEqual(*p.Char, beforeP) || !reflect.DeepEqual(*member.Char, beforeM) ||
					p.Session.QueuedPacketsForTest() != 0 || member.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("unconfirmed reward escaped detached state")
				}
			}
			assertHidden()
			w.tickItemInstances(clock.Now()) // Must not submit a duplicate while fenced.
			release()
			awaitReadyCriticalResults(t, q, 1)
			assertHidden()
			w.processCriticalCompletions(1)
			switch outcome {
			case "commit":
				if !inst.RuneTrack.Parties[0].Rewarded || len(q.pending) != 0 || p.Session.QueuedPacketsForTest() == 0 {
					t.Fatal("confirmed reward was not installed and published")
				}
			case "rejected":
				assertHidden()
				if len(q.pending) != 0 || w.criticalInstanceReserved() || w.shuttingDown {
					t.Fatal("certified rejection prevented a safe retry")
				}
			case "unknown":
				assertHidden()
				if len(q.pending) != 1 || !w.criticalInstanceReserved() || !w.shuttingDown {
					t.Fatal("unknown outcome escaped containment")
				}
			}
		})
	}
}

func TestRuneTrackObjectiveMarchers(t *testing.T) {
	for _, room := range []int{1, 2, 6} {
		t.Run(fmt.Sprint(room), func(t *testing.T) {
			w, p, _, _, clock := runeTrackTestWorld(t)
			inst := enterRuneTrack(t, w, p, clock, room)
			clock.Advance(31 * time.Second)
			w.tickItemInstances(clock.Now())
			switch room {
			case 1:
				tower := runeMob(t, w, inst, 5715)
				invader := runeMob(t, w, inst, 5718)
				invader.X, invader.Y = tower.X+1, tower.Y
				tower.HP = 1
				w.tickRuneMarch(invader, clock.Now(), true)
				if !inst.RuneTrack.Parties[0].TowerLost {
					t.Fatal("invader did not destroy the tower")
				}
			case 2:
				boss := runeMob(t, w, inst, 5798)
				invader := runeMob(t, w, inst, 5799)
				oldHP, templateHP := boss.HP, boss.Def.Score.MaxHP
				invader.X, invader.Y = 3390, 1438
				w.tickRuneMarch(invader, clock.Now(), true)
				if w.mobsByID[invader.ID] != nil || boss.HP != oldHP+2000 || w.npcDefByName(boss.Def.Name).Score.MaxHP != templateHP {
					t.Fatal("Amon absorption changed the shared template or missed its heal")
				}
			case 6:
				invader := runeMob(t, w, inst, 5777)
				inst.RuneTrack.Collected = [3]int{5, 1, 0}
				invader.X = 3422
				w.tickRuneMarch(invader, clock.Now(), true)
				if w.mobsByID[invader.ID] != nil || inst.RuneTrack.Collected != [3]int{4, 0, 0} {
					t.Fatal("Zakum crossing did not decrement counters safely")
				}
			}
		})
	}
}
