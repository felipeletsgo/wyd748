package game

import (
	"errors"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func TestMysticLandGeneratorRespawnConfiguration(t *testing.T) {
	generators, err := data.LoadChannelNPCGener("../../data/spawns/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	count := 0
	for _, g := range generators {
		// Check coordinates and template ownership, not decorative labels alone.
		s := g.Segments[0]
		mystic := s.X >= 2380 && s.X <= 2404 && s.Y >= 1560 && s.Y <= 1568 &&
			(g.Leader == "Bazilisk" || g.Leader == "EfreetBazilisk" || g.Leader == "EfreetDracolich")
		if !mystic {
			if g.RespawnSeconds != 0 {
				t.Fatalf("unrelated generator %d has rapid respawn", g.Label)
			}
			continue
		}
		count++
		if !g.Enabled || g.RespawnSeconds != 5 || g.MaxNumMob != 3 || g.MinGroup != 2 || g.MaxGroup != 2 {
			t.Fatalf("Mystic Land generator %d has wrong respawn or population", g.Label)
		}
	}
	if count != 7 {
		t.Fatalf("found %d Mystic Land monster generators, want 7", count)
	}
}

func TestGeneratorRespawnsEachMobFiveSecondsAfterItsDeath(t *testing.T) {
	w := newZoneTestWorld()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock, w.rng = clock, fixedRNG{}
	leader := &model.NPCDef{Name: "Leader", Tipo: model.TipoMonstro, Score: &model.Score{MaxHP: 100}}
	follower := &model.NPCDef{Name: "Follower", Tipo: model.TipoMonstro, Score: &model.Score{MaxHP: 200}}
	g := &generState{def: model.NPCGener{Index: 5, Enabled: true, MinuteGenerate: 20,
		RespawnSeconds: 5, MaxNumMob: 3, MinGroup: 2, MaxGroup: 2,
		Segments: [model.MaxGenerSegments]model.GenerSegment{{X: 2100, Y: 2100}}}, leader: leader, follower: follower}
	w.spawnGroup(g)
	w.scheduleGenerator(g, clock.Now())
	if g.current != 3 || !g.nextGenerate.IsZero() {
		t.Fatal("rapid respawn retained periodic generation or lost initial population")
	}
	first, second := w.mobs[0], w.mobs[1]
	w.killMobWithoutPlayer(first)
	clock.Advance(2 * time.Second)
	w.killMobWithoutPlayer(second)
	if len(g.respawns) != 2 || g.current != 1 {
		t.Fatal("deaths did not create independent deadlines")
	}
	clock.Advance(3*time.Second - time.Nanosecond)
	w.tickGeneratorRespawns(g, clock.Now())
	if g.current != 1 {
		t.Fatal("respawn happened before five seconds")
	}
	clock.Advance(time.Nanosecond)
	w.tickGeneratorRespawns(g, clock.Now())
	if g.current != 2 || len(g.respawns) != 1 {
		t.Fatal("first death did not respawn alone at five seconds")
	}
	clock.Advance(2 * time.Second)
	w.tickGeneratorRespawns(g, clock.Now())
	if g.current != 3 || len(g.respawns) != 0 {
		t.Fatal("second death did not retain its own five-second deadline")
	}
	for i := 0; i < 3; i++ {
		w.tickGeneratorRespawns(g, clock.Now())
	}
	counts := map[*model.NPCDef]int{}
	for _, m := range w.mobs {
		counts[m.Def]++
		if m.Dead || m.HP != m.Def.Score.MaxHP {
			t.Fatal("replacement did not reset combat state")
		}
	}
	if len(w.mobs) != 3 || counts[leader] != 1 || counts[follower] != 2 {
		t.Fatal("respawn duplicated population or changed monster types")
	}
}

func TestGeneratorRespawnFollowsKillPersistence(t *testing.T) {
	for _, fail := range []bool{false, true} {
		t.Run(fmt.Sprint(fail), func(t *testing.T) {
			var saveError error
			if fail {
				saveError = errors.New("injected save failure")
			}
			w, p, m, _ := killRewardTestWorld(t, saveError)
			clock := newFakeClock(time.Unix(100, 0))
			w.clock = clock
			g := &generState{current: 1, def: model.NPCGener{Enabled: true, MinuteGenerate: 20, RespawnSeconds: 5, MaxNumMob: 1}}
			m.generator = g
			w.killMobState(p, m, 100, 100)
			if fail {
				if g.current != 1 || len(g.respawns) != 0 || m.Dead {
					t.Fatal("failed death scheduled a respawn")
				}
			} else if g.current != 0 || len(g.respawns) != 1 || !g.respawns[0].readyAt.Equal(clock.Now().Add(5*time.Second)) {
				t.Fatal("committed death did not schedule the selected delay")
			}
		})
	}
	// A slow critical write must neither publish nor respawn an unsettled kill.
	w, p, m, _ := killRewardTestWorld(t, nil)
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	g := &generState{current: 1, def: model.NPCGener{Enabled: true, MinuteGenerate: 20, RespawnSeconds: 5, MaxNumMob: 1,
		Segments: [model.MaxGenerSegments]model.GenerSegment{{X: 2100, Y: 2100}}}}
	m.generator = g
	claimCriticalTestPlayers(t, w, p)
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	queue := newCriticalTestQueue(t, &criticalBatchTestStore{backend}, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	w.killMobState(p, m, 100, 100)
	awaitCriticalWrite(t, backend.entered)
	clock.Advance(6 * time.Second)
	w.tickGeneratorRespawns(g, clock.Now())
	// The unsettled kill hides the mob from mobByID but keeps it indexed.
	if g.current != 1 || len(g.respawns) != 0 || w.mobsByID[m.ID] != m {
		t.Fatalf("unsettled death: population=%d pending=%d original=%t dead=%t indexed=%t", g.current, len(g.respawns), w.mobByID(m.ID) == m, m.Dead, w.mobsByID[m.ID] == m)
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if g.current != 0 || len(g.respawns) != 1 || !g.respawns[0].readyAt.Equal(time.Unix(105, 0)) {
		t.Fatal("commit restarted the deadline instead of preserving death time")
	}
	w.tickGeneratorRespawns(g, clock.Now())
	if g.current != 1 || len(g.respawns) != 0 {
		t.Fatal("overdue committed death did not respawn")
	}
}
