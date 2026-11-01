package game

import (
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

// W5/D5: a tick panic fails only this World closed; it no longer ends the
// process. Its sessions close with the panic cause and nothing is saved.
func TestTickPanicFailsWorldClosedWithoutCrashing(t *testing.T) {
	w := testSpatialWorld(nil)
	p, session := networkedTestPlayer(1, "Observer", 2100, 2100)
	w.players[session], w.playersByID[p.ID] = p, p
	w.players[gameNet.NewTestSession(99, 1)] = nil // corrupt actor state
	before := metricPanicsTotal.Value()
	func() {
		defer func() {
			if r := recover(); r != nil {
				t.Fatalf("tick panic escaped the World: %v", r)
			}
		}()
		// Stop at the first contained panic.
		for range 4 {
			if w.tick(); metricPanicsTotal.Value() != before || w.shuttingDown {
				break
			}
		}
	}()
	if metricPanicsTotal.Value() != before+1 {
		t.Skip("fixture did not reach a panicking tick path")
	}
	if !w.shuttingDown || !p.PersistencePoisoned || !session.IsClosed() || session.CloseCause() != gameNet.CloseCausePanic {
		t.Fatalf("maintenance=%t poisoned=%t closed=%t cause=%s",
			w.shuttingDown, p.PersistencePoisoned, session.IsClosed(), session.CloseCause())
	}
}

func TestChannelsReadyReflectsHeartbeatAndMaintenance(t *testing.T) {
	// The registry is process-wide; other tests leave their own channels in
	// maintenance. Production has only its real channels.
	channelMetricsRegistry.Lock()
	saved := channelMetricsRegistry.channels
	channelMetricsRegistry.channels = make(map[byte]*channelMetrics)
	channelMetricsRegistry.Unlock()
	t.Cleanup(func() {
		channelMetricsRegistry.Lock()
		channelMetricsRegistry.channels = saved
		channelMetricsRegistry.Unlock()
	})
	w := &World{channel: 231}
	now := time.Now()
	w.observeHeartbeat(now)
	if err := ChannelsReady(now, time.Second); err != nil {
		t.Fatalf("live channel not ready: %v", err)
	}
	if err := ChannelsReady(now.Add(2*time.Second), time.Second); err == nil {
		t.Fatal("a stalled actor heartbeat was ready")
	}
	w.shuttingDown = true
	w.observeHeartbeat(now)
	if err := ChannelsReady(now, time.Second); err == nil {
		t.Fatal("a channel in maintenance was ready")
	}
}

// slowSafeStore is goroutine-safe and costs a fixed delay per final write.
type slowSafeStore struct {
	craftStore
	delay       time.Duration
	saves       atomic.Int64
	mu          sync.Mutex
	inFlight    int
	maxInFlight int
}

func (s *slowSafeStore) SavePlayerState(_ *model.GuildRegistry, _ *model.Account, _ string, _ *model.CharState) error {
	s.mu.Lock()
	s.inFlight++
	s.maxInFlight = max(s.maxInFlight, s.inFlight)
	s.mu.Unlock()
	time.Sleep(s.delay)
	s.mu.Lock()
	s.inFlight--
	s.mu.Unlock()
	s.saves.Add(1)
	return nil
}

// W5/D11: final saves use bounded workers on detached snapshots.
func TestTerminalSavesRunOnBoundedWorkers(t *testing.T) {
	const players = 24
	elapsed := map[int]time.Duration{}
	for _, workers := range []int{1, 4} {
		st := &slowSafeStore{delay: 10 * time.Millisecond}
		list := make([]*Player, 0, players)
		for i := range players {
			p, _ := networkedTestPlayer(uint16(i+1), "Final"+string(rune('A'+i)), 2100, 2100)
			list = append(list, p)
		}
		w := worldWithNetworkedPlayers(list...)
		w.store, w.shutdownSaveWorkers = st, workers
		started := time.Now()
		w.persistShutdownState()
		elapsed[workers] = time.Since(started)
		if w.shutdownErr != nil || st.saves.Load() != players || st.maxInFlight > workers {
			t.Fatalf("workers=%d err=%v saves=%d maxInFlight=%d", workers, w.shutdownErr, st.saves.Load(), st.maxInFlight)
		}
	}
	if elapsed[4]*2 > elapsed[1] {
		t.Fatalf("four workers took %s against %s sequentially", elapsed[4], elapsed[1])
	}
	t.Logf("%d final saves: sequential %s, four workers %s", players, elapsed[1], elapsed[4])
}

// W5/D5: a contained panic requests one supervised restart for the process.
func TestContainedPanicRequestsOneSupervisedRestart(t *testing.T) {
	panicRestartRequested.Store(false)
	t.Cleanup(func() { panicRestartRequested.Store(false) })
	requests := make(chan string, 4)
	first, second := &World{panicRestart: true}, &World{panicRestart: true}
	for _, w := range []*World{first, second} {
		w.staffOps.request = func(mode string) { requests <- mode }
	}
	first.failClosedAfterHandlerPanic()
	second.failClosedAfterHandlerPanic()
	select {
	case mode := <-requests:
		if mode != control.ShutdownModeRestart {
			t.Fatalf("restart mode = %q", mode)
		}
	case <-time.After(time.Second):
		t.Fatal("a contained panic did not request a supervised restart")
	}
	select {
	case <-requests:
		t.Fatal("a second panic requested another restart")
	case <-time.After(50 * time.Millisecond):
	}
	panicRestartRequested.Store(false)
	quiet := &World{}
	quiet.staffOps.request = func(mode string) { requests <- mode }
	quiet.failClosedAfterHandlerPanic()
	select {
	case <-requests:
		t.Fatal("a World without WithPanicRestart requested a restart")
	case <-time.After(50 * time.Millisecond):
	}
}
