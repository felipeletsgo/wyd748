package game

import (
	"errors"
	"sync"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// activatedTestStore records which adapter served each write. The certified
// methods are the only ones an activated World may use outside the terminal
// shutdown snapshot.
type activatedTestStore struct {
	mu        sync.Mutex
	certified int
	legacy    int
}

func (s *activatedTestStore) LoadAccount(string) (*model.Account, error) { return nil, nil }
func (s *activatedTestStore) CharacterNameExists(string) (bool, error)   { return false, nil }
func (s *activatedTestStore) SaveAccount(*model.Account) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.legacy++
	return nil
}
func (s *activatedTestStore) count() (int, int) {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.certified, s.legacy
}
func (s *activatedTestStore) certify() error {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.certified++
	return nil
}
func (s *activatedTestStore) SaveCriticalAccounts(...*model.Account) error { return s.certify() }
func (s *activatedTestStore) SavePlayerStateBatch(*model.GuildRegistry, string, *model.CharState, ...*model.Account) error {
	return s.certify()
}
func (s *activatedTestStore) SaveCriticalInstanceState(*model.InstanceStateSnapshot, ...*model.Account) error {
	return s.certify()
}
func (s *activatedTestStore) SaveCriticalGameState(*model.GuildRegistry, *model.InstanceStateSnapshot, string,
	*model.CharState, ...*model.Account) error {
	return s.certify()
}

func TestCriticalActivationRejectsUncertifiedStore(t *testing.T) {
	_, err := NewWorld(&flushCraftStore{}, nil, nil, model.Catalog{}, [model.MaxCarry]int{}, model.VolatileCatalog{},
		model.CharacterTemplateFile{}, model.TerrainMap{}, WithCriticalPersistence(1, 8))
	if !errors.Is(err, errCriticalPersistenceStoreUnsupported) {
		t.Fatalf("activation accepted a store without certified adapters: %v", err)
	}
}

// The activated production path: NewWorld starts the coordinator and the
// actual actor loop processes ticks, packets and disconnects without any
// legacy inline write until the terminal shutdown snapshot.
func TestCriticalActivatedWorldNeverWritesInlineDuringGameplay(t *testing.T) {
	backend := &activatedTestStore{}
	w, err := NewWorld(backend, nil, nil, model.Catalog{}, [model.MaxCarry]int{}, model.VolatileCatalog{},
		model.CharacterTemplateFile{}, model.TerrainMap{}, WithCriticalPersistence(2, 64), WithCriticalAccountContainment())
	if err != nil {
		t.Fatal(err)
	}
	if w.criticalWrites == nil || !w.criticalAccountContainment {
		t.Fatal("NewWorld did not activate the coordinator")
	}
	rejectedBefore := metricSyncPersistenceRejected.Value()
	stay, _ := networkedTestPlayer(1, "Stay", 2100, 2100)
	leave, _ := networkedTestPlayer(2, "Leave", 2110, 2100)
	for _, p := range []*Player{stay, leave} {
		w.players[p.Session], w.playersByID[p.ID] = p, p
		w.updatePlayerSpatial(p)
		claimCriticalTestPlayers(t, w, p)
		p.NextRegen = w.now().Add(time.Hour)
	}
	stay.Char.CP, stay.NextCPRecovery = -3, w.now().Add(-time.Second)
	w.nextAutoSave = w.now().Add(-time.Second)

	ticks, stop, stopped := make(chan time.Time), make(chan struct{}), make(chan struct{})
	go func() {
		defer close(stopped)
		w.run(ticks, stop)
	}()
	send := func(cmd command) {
		select {
		case w.commands <- cmd:
		case <-time.After(time.Second):
			t.Fatal("World did not accept a command")
		}
	}
	for range 8 {
		deliverCriticalLoopTick(t, ticks)
		send(command{s: stay.Session, pkt: criticalPKModePacket(true)})
		send(command{s: stay.Session, pkt: inboundPacket(wire.OpSysQuit, 12)})
	}
	send(command{s: leave.Session, pkt: inboundPacket(wire.OpCharacterLogout, 12)})
	send(command{s: leave.Session}) // physical disconnect after logout
	deadline := time.Now().Add(3 * time.Second)
	for time.Now().Before(deadline) {
		overview := readCriticalLoopOverview(t, w)
		if overview.State != "running" {
			t.Fatalf("activated World left running state: %s", overview.State)
		}
		if certified, _ := backend.count(); certified >= 4 && len(overview.Players) == 1 {
			break
		}
		deliverCriticalLoopTick(t, ticks)
	}
	if certified, legacy := backend.count(); certified < 4 || legacy != 0 {
		t.Fatalf("gameplay writes: certified=%d legacy=%d; want certified writes only", certified, legacy)
	}
	done := make(chan bool, 1)
	go func() { done <- w.Shutdown(2 * time.Second) }()
	for {
		select {
		case ok := <-done:
			close(stop)
			<-stopped
			if !ok {
				t.Fatal("activated shutdown did not complete")
			}
			if delta := metricSyncPersistenceRejected.Value() - rejectedBefore; delta != 0 || len(w.syncPersistenceRejected) != 0 {
				t.Fatalf("activated gameplay attempted %d unmigrated inline write(s): %v", delta, w.syncPersistenceRejected)
			}
			if w.players[leave.Session] != nil || !stay.PKMode || stay.Char.CP == -3 {
				t.Fatal("activated gameplay did not apply deferred packets, logout or CP recovery")
			}
			return
		case <-time.After(10 * time.Millisecond):
			select {
			case ticks <- time.Now():
			default:
			}
		}
	}
}
