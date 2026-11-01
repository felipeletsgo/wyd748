package game

import (
	"fmt"
	"math/rand"
	"os"
	"runtime"
	"slices"
	"sort"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// Repeatable actor load probes (DOCS/server/server-correction-plan.md, W0).
// They run only with WYD_LOAD_PROBE=1 and measure in-process actor work with
// test sessions. They are not TCP capacity or disconnect evidence.
//
//	WYD_LOAD_PROBE=1 go test -run 'TestLoadProbe' -v ./internal/game
const loadProbeSeed = 1

func loadProbeWorld(t *testing.T, seed int64) (*World, *fakeClock) {
	t.Helper()
	if os.Getenv("WYD_LOAD_PROBE") != "1" {
		t.Skip("set WYD_LOAD_PROBE=1 to run the load probe")
	}
	npcs, err := data.LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	generators, err := data.LoadChannelNPCGener("../../data/spawns/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	f, err := eventFixtures()
	if err != nil {
		t.Fatal(err)
	}
	clock := newFakeClock(time.Date(2026, 10, 31, 12, 0, 0, 0, time.UTC))
	w, err := NewWorld(&craftStore{}, npcs, generators, model.Catalog{Items: f.items}, [model.MaxCarry]int{}, f.volatiles,
		model.CharacterTemplateFile{}, f.terrain, WithOperationalConfig(DefaultOperationalConfig()),
		WithInitItems(f.initItems), WithNPCGenerLog("quiet"), WithGameplayLog("quiet"),
		WithClock(clock), WithRNG(rand.New(rand.NewSource(seed))))
	if err != nil {
		t.Fatal(err)
	}
	t.Logf("environment: %s %s/%s cpus=%d gomaxprocs=%d seed=%d mobs=%d generators=%d",
		runtime.Version(), runtime.GOOS, runtime.GOARCH, runtime.NumCPU(), runtime.GOMAXPROCS(0),
		seed, len(w.mobs), len(w.generators))
	return w, clock
}

type loadProbeBlock struct {
	x, y  uint16
	count int
}

// loadProbeDenseBlocks ranks 64x64 blocks by monster count with a fixed tie order.
func loadProbeDenseBlocks(w *World, n int) []loadProbeBlock {
	counts := map[[2]uint16]int{}
	for _, m := range w.mobs {
		if m != nil && m.Def != nil && m.Def.Tipo == model.TipoMonstro {
			counts[[2]uint16{m.X / 64, m.Y / 64}]++
		}
	}
	blocks := make([]loadProbeBlock, 0, len(counts))
	for key, count := range counts {
		blocks = append(blocks, loadProbeBlock{x: key[0]*64 + 32, y: key[1]*64 + 32, count: count})
	}
	sort.Slice(blocks, func(i, j int) bool {
		a, b := blocks[i], blocks[j]
		if a.count != b.count {
			return a.count > b.count
		}
		if a.x != b.x {
			return a.x < b.x
		}
		return a.y < b.y
	})
	return blocks[:min(n, len(blocks))]
}

func loadProbeWalkable(w *World, x, y uint16) (uint16, uint16) {
	for r := 0; r < 12; r++ {
		for dx := -r; dx <= r; dx++ {
			for dy := -r; dy <= r; dy++ {
				nx, ny := uint16(int(x)+dx), uint16(int(y)+dy)
				if w.terrain.Walkable(nx, ny) {
					return nx, ny
				}
			}
		}
	}
	return x, y
}

func loadProbePlayer(w *World, id uint16, x, y uint16) *Player {
	return loadProbePlayerWithQueue(w, id, x, y, 1<<16)
}

func loadProbePlayerWithQueue(w *World, id uint16, x, y uint16, queue int) *Player {
	session := net.NewTestSession(int64(id), queue)
	account := &model.Account{Name: fmt.Sprintf("probe%04d", id), Chars: []model.Char{{
		Name: fmt.Sprintf("Probe%04d", id), Class: uint8(id % 4),
		Score: &model.Score{Version: model.ScoreVersion, Level: 300, MaxHP: 2_000_000, CurHP: 2_000_000,
			MaxMP: 100000, CurMP: 100000, Defense: 3000},
	}}}
	p := &Player{ID: id, Session: session, Account: account, Char: &account.Chars[0], InWorld: true,
		X: x, Y: y, Visible: map[uint16]struct{}{}}
	p.Char.X, p.Char.Y = x, y
	w.players[session] = p
	w.playersByID[id] = p
	w.indexPlayerCharacter(p)
	w.refreshPlayerVisibility(p)
	return p
}

func loadProbeDrain(players []*Player) (total, peak int) {
	for _, p := range players {
		n := 0
		for {
			if _, ok := p.Session.DequeuePacketForTest(); !ok {
				break
			}
			n++
		}
		total, peak = total+n, max(peak, n)
	}
	return total, peak
}

func loadProbePercentile(samples []time.Duration, q float64) time.Duration {
	if len(samples) == 0 {
		return 0
	}
	sorted := slices.Clone(samples)
	slices.Sort(sorted)
	return sorted[int(float64(len(sorted)-1)*q)]
}

// TestLoadProbeMovementAndTick separates movement visibility work from tick
// work for fixed populations placed in the densest monster blocks.
func TestLoadProbeMovementAndTick(t *testing.T) {
	for _, population := range []int{50, 150, 300} {
		w, clock := loadProbeWorld(t, loadProbeSeed)
		blocks := loadProbeDenseBlocks(w, 8)
		placement := rand.New(rand.NewSource(loadProbeSeed))
		players := make([]*Player, 0, population)
		for i := range population {
			b := blocks[i%len(blocks)]
			x, y := loadProbeWalkable(w, b.x+uint16(placement.Intn(24))-12, b.y+uint16(placement.Intn(24))-12)
			players = append(players, loadProbePlayer(w, uint16(i+1), x, y))
		}
		loadProbeDrain(players)
		for range 20 {
			clock.Advance(worldTickInterval)
			w.tick()
		}
		loadProbeDrain(players)
		const ticks = 240
		var tickDurations, cycleDurations []time.Duration
		var moveTime time.Duration
		moves, packets, peak := 0, 0, 0
		var before, after runtime.MemStats
		runtime.GC()
		runtime.ReadMemStats(&before)
		for i := range ticks {
			began := time.Now()
			for j, p := range players {
				if (i+j)%2 != 0 { // half the population steps each tick
					continue
				}
				oldX, oldY := p.X, p.Y
				nx, ny := uint16(int(p.X)+placement.Intn(3)-1), uint16(int(p.Y)+placement.Intn(3)-1)
				if !w.terrain.Walkable(nx, ny) {
					continue
				}
				p.X, p.Y, p.Char.X, p.Char.Y = nx, ny, nx, ny
				w.refreshPlayerVisibilityAfterMove(p, oldX, oldY)
				subject := p
				w.sendToPlayerView(subject, func() []byte { return wire.ActionStop(subject.ID, subject.X, subject.Y) })
				moves++
			}
			movePhase := time.Since(began)
			moveTime += movePhase
			clock.Advance(worldTickInterval)
			began = time.Now()
			w.tick()
			tickDurations = append(tickDurations, time.Since(began))
			// P1: one channel's whole cycle is the wait it adds to the other
			// channel's commands on the shared cluster goroutine.
			cycleDurations = append(cycleDurations, movePhase+tickDurations[len(tickDurations)-1])
			total, most := loadProbeDrain(players)
			packets, peak = packets+total, max(peak, most)
		}
		runtime.ReadMemStats(&after)
		t.Logf("population=%d measured=refreshPlayerVisibilityAfterMove+sendToPlayerView,tick "+
			"move_mean=%s moves=%d tick_p50=%s tick_p99=%s tick_max=%s alloc_per_tick=%dKB gc=%d "+
			"packets_per_player_per_second=%.1f peak_packets_per_player_per_tick=%d",
			population, moveTime/time.Duration(max(moves, 1)), moves,
			loadProbePercentile(tickDurations, 0.5), loadProbePercentile(tickDurations, 0.99), loadProbePercentile(tickDurations, 1),
			(after.TotalAlloc-before.TotalAlloc)/ticks/1024, after.NumGC-before.NumGC,
			float64(packets)/float64(population)/(float64(ticks)*worldTickInterval.Seconds()), peak)
		t.Logf("population=%d channel_cycle_p50=%s channel_cycle_p99=%s channel_cycle_max=%s (wait added to the other channel)",
			population, loadProbePercentile(cycleDurations, 0.5), loadProbePercentile(cycleDurations, 0.99), loadProbePercentile(cycleDurations, 1))
	}
}

// TestLoadProbeArrivalBurst counts packets queued at once for one arrival in
// the densest block. The session queue here has no concurrent TCP writer.
func TestLoadProbeArrivalBurst(t *testing.T) {
	for _, crowd := range []int{0, 50, 100, 200} {
		w, _ := loadProbeWorld(t, loadProbeSeed)
		block := loadProbeDenseBlocks(w, 1)[0]
		placement := rand.New(rand.NewSource(loadProbeSeed + 1))
		players := make([]*Player, 0, crowd)
		for i := range crowd {
			x, y := loadProbeWalkable(w, block.x+uint16(placement.Intn(30))-15, block.y+uint16(placement.Intn(30))-15)
			players = append(players, loadProbePlayer(w, uint16(i+1), x, y))
		}
		loadProbeDrain(players)
		x, y := loadProbeWalkable(w, block.x, block.y)
		began := time.Now()
		arrival := loadProbePlayer(w, 900, x, y)
		cost := time.Since(began)
		observers, _ := loadProbeDrain(players)
		t.Logf("crowd=%d block_mobs=%d measured=refreshPlayerVisibility arrival_packets=%d default_queue=256 observer_packets=%d cost=%s",
			crowd, block.count, arrival.Session.QueuedPacketsForTest(), observers, cost)
	}
}

// TestLoadProbeArrivalWithDefaultQueue repeats the dense arrival with the
// production 256-packet queue. The writer is modeled as draining between
// ticks; the probe reports ticks to a complete scene and any closure.
func TestLoadProbeArrivalWithDefaultQueue(t *testing.T) {
	for _, crowd := range []int{0, 50, 100, 200} {
		w, clock := loadProbeWorld(t, loadProbeSeed)
		block := loadProbeDenseBlocks(w, 1)[0]
		placement := rand.New(rand.NewSource(loadProbeSeed + 1))
		players := make([]*Player, 0, crowd)
		for i := range crowd {
			x, y := loadProbeWalkable(w, block.x+uint16(placement.Intn(30))-15, block.y+uint16(placement.Intn(30))-15)
			players = append(players, loadProbePlayer(w, uint16(i+1), x, y))
		}
		loadProbeDrain(players)
		x, y := loadProbeWalkable(w, block.x, block.y)
		arrival := loadProbePlayerWithQueue(w, 900, x, y, 256)
		peak := arrival.Session.QueuedPacketsForTest()
		total, ticks := 0, 0
		for ; arrival.sceneIncomplete && ticks < 50; ticks++ {
			n, _ := loadProbeDrain([]*Player{arrival})
			total += n
			loadProbeDrain(players)
			clock.Advance(worldTickInterval)
			w.tick()
			peak = max(peak, arrival.Session.QueuedPacketsForTest())
		}
		n, _ := loadProbeDrain([]*Player{arrival})
		total += n
		t.Logf("crowd=%d block_mobs=%d queue=256 ticks_to_complete=%d peak_queued=%d arrival_packets=%d closed=%t visible=%d",
			crowd, block.count, ticks, peak, total, arrival.Session.IsClosed(), len(arrival.Visible))
	}
}
