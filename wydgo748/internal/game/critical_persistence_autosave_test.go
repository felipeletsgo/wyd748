package game

import (
	"errors"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

// These adapters deliberately lack the atomic form. Admission must reject
// before invoking either an account write or a standalone sidecar write.
type criticalAutosaveSidecarOnly struct{ *criticalTestStore }

func (*criticalAutosaveSidecarOnly) LoadCharState(string) (*model.CharState, error) {
	panic("unexpected standalone sidecar read")
}
func (*criticalAutosaveSidecarOnly) SaveCharState(string, *model.CharState) error {
	panic("unexpected standalone sidecar write")
}

type criticalAutosaveAsyncSidecarOnly struct{ *criticalTestStore }

func (*criticalAutosaveAsyncSidecarOnly) SaveCharStateAsync(string, *model.CharState) error {
	panic("unexpected asynchronous sidecar write")
}

// Hold the first post-autosave economy write independently. Polling actor
// readiness must not accidentally commit repeated fresh deposit intentions.
type criticalAutosaveLoopStore struct {
	*criticalStateTestStore
	transferRelease <-chan struct{}
}

func (s *criticalAutosaveLoopStore) SaveAccount(account *model.Account) error {
	s.entered <- criticalWrite{accounts: []*model.Account{account}}
	<-s.transferRelease
	return nil
}

func criticalAutosaveFixture(t *testing.T, slot int) (*World, *Player, *batchGameStore, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, other, fallback, backend, queue, release := criticalTradeFixture(t, slot, 0, "a")
	p.Trade, other.Trade, other.InWorld = nil, nil, false
	p.SpecialCoins = map[string]uint32{"token": 19, "zero": 0}
	p.Char.Affects[0] = model.Affect{Type: 7, Value: 31, SourceItemUID: "buff-item", ExpiresAt: w.now().Add(time.Hour)}
	return w, p, fallback, backend, queue, release
}

func runCriticalAutosaveSlice(w *World, p *Player, now time.Time) {
	w.autoSaveBucket = accountAutoSaveBucket(p.Account.Name)
	w.autoSaveAccounts(now)
}

func TestCriticalAutosaveDetachedAtomicSnapshotAndFreshNextSave(t *testing.T) {
	for slot := 0; slot < 4; slot++ {
		t.Run(fmt.Sprintf("slot-%d", slot), func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalAutosaveFixture(t, slot)
			now := w.now()
			before, want := accountStateSnapshot(p.Account), accountPersistenceSnapshot(p.Account)
			state := buildCharState(p, now)
			account, character := p.Account, p.Char
			runCriticalAutosaveSlice(w, p, now)
			write := awaitCriticalWrite(t, backend.entered)
			if len(write.accounts) != 1 || write.accounts[0] == account ||
				&write.accounts[0].Chars[slot] == character || !reflect.DeepEqual(write.accounts[0], want) ||
				write.character == nil || write.character.uid != p.Char.UID || !reflect.DeepEqual(write.character.state, state) ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 || fallback.batchSaves != 0 {
				t.Fatal("autosave changed live state, wrote inline or lost the detached atomic snapshot")
			}
			for _, char := range p.Account.Chars {
				if char.UID != "" && !w.criticalCharacterReserved(char.UID) {
					t.Fatal("autosave failed to reserve a persisted character")
				}
			}
			for range 3 {
				runCriticalAutosaveSlice(w, p, now)
			}
			if err := w.queueAccountAutoSave(p, now); !errors.Is(err, errCriticalPersistenceBusy) {
				t.Fatalf("direct autosave bypassed its account reservation: %v", err)
			}
			if len(queue.pending) != 1 || len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 ||
				!w.nextAutoSave.Equal(now.Add(accountAutoSaveSliceInterval)) {
				t.Fatal("held autosave repeated, published success or lost its cadence")
			}
			// Direct actor-owned mutation here is a detachment probe, not a
			// gameplay consumer: it must neither alter worker data nor be rolled
			// back by this durability-only completion.
			p.Char.Gold++
			p.Account.CelestialCapsules[0].Character.Name = "NewCapsuleState"
			p.SpecialCoins["token"]++
			p.Char.Affects[0].Value++
			newLive := accountStateSnapshot(p.Account)
			if !reflect.DeepEqual(write.accounts[0], want) || !reflect.DeepEqual(write.character.state, state) {
				t.Fatal("worker snapshot aliases nested live state")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			runCriticalAutosaveSlice(w, p, now)
			if !w.criticalPlayerReserved(p) || len(queue.pending) != 1 || len(backend.entered) != 0 {
				t.Fatal("completed autosave released ownership before acknowledgement")
			}
			w.processCriticalCompletions(1)
			if p.Account != account || p.Char != character || !reflect.DeepEqual(accountStateSnapshot(p.Account), newLive) ||
				p.SpecialCoins["token"] != 20 || p.Char.Affects[0].Value != 32 ||
				w.criticalPlayerReserved(p) || len(queue.pending) != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("autosave acknowledgement restored old state, leaked ownership or published an economic response")
			}
			runCriticalAutosaveSlice(w, p, now)
			fresh := awaitCriticalWrite(t, backend.entered)
			if !reflect.DeepEqual(fresh.accounts[0], accountPersistenceSnapshot(p.Account)) ||
				!reflect.DeepEqual(fresh.character.state, buildCharState(p, now)) {
				t.Fatal("next autosave reused the previous snapshot")
			}
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
		})
	}
}

func TestCriticalAutosaveRejectsUnsupportedSidecarsBeforeAnyWrite(t *testing.T) {
	for _, async := range []bool{false, true} {
		t.Run(fmt.Sprintf("async-%t", async), func(t *testing.T) {
			w, p, fallback, backend, queue, _ := criticalAutosaveFixture(t, 0)
			if async {
				queue.store = &criticalAutosaveAsyncSidecarOnly{backend}
			} else {
				queue.store = &criticalAutosaveSidecarOnly{backend}
			}
			before := accountStateSnapshot(p.Account)
			if err := w.queueAccountAutoSave(p, w.now()); !errors.Is(err, errCriticalPersistenceUnsupported) {
				t.Fatalf("sidecar-only adapter accepted split autosave: %v", err)
			}
			runCriticalAutosaveSlice(w, p, w.now())
			if len(queue.pending) != 0 || len(backend.entered) != 0 || len(w.criticalWrites.continuations) != 0 ||
				p.criticalLifecycle != nil || fallback.saves != 0 || fallback.batchSaves != 0 ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
				t.Fatal("unsupported autosave wrote, mutated state or retained a rejected operation")
			}
		})
	}
}

func TestCriticalAutosaveSaturationSkipsWithoutFallbackThenUsesFreshState(t *testing.T) {
	w, p, fallback, backend, queue, release := criticalAutosaveFixture(t, 0)
	for _, name := range []string{"held-one", "held-two"} {
		if _, err := w.criticalWrites.submit(criticalWrite{accounts: []*model.Account{{Name: name}}},
			func(criticalPersistenceResult) error { return nil }); err != nil {
			t.Fatal(err)
		}
	}
	awaitCriticalWrite(t, backend.entered)
	before := accountStateSnapshot(p.Account)
	if err := w.queueAccountAutoSave(p, w.now()); !errors.Is(err, errCriticalPersistenceFull) {
		t.Fatalf("saturated autosave admitted: %v", err)
	}
	runCriticalAutosaveSlice(w, p, w.now())
	if len(queue.pending) != 2 || w.criticalPlayerReserved(p) || fallback.saves != 0 || fallback.batchSaves != 0 ||
		!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.criticalLifecycle != nil {
		t.Fatal("saturated autosave retained work, changed state or used an inline fallback")
	}
	p.Char.Gold = 4321
	release()
	awaitCriticalWrite(t, backend.entered)
	awaitReadyCriticalResults(t, queue, 2)
	w.processCriticalCompletions(2)
	runCriticalAutosaveSlice(w, p, w.now())
	write := awaitCriticalWrite(t, backend.entered)
	if write.accounts[0].Chars[p.CharSlot].Gold != 4321 {
		t.Fatal("next eligible autosave replayed the rejected old snapshot")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
}

func TestCriticalAutosaveRejectsInvalidIdentityAndStoppedAdmission(t *testing.T) {
	for _, reason := range []string{"closed-session", "account-owner", "character-slot", "out-of-world", "poisoned", "stopping", "missing-owner"} {
		t.Run(reason, func(t *testing.T) {
			w, p, fallback, backend, queue, _ := criticalAutosaveFixture(t, 0)
			wantErr := errCriticalPersistenceIdentity
			switch reason {
			case "closed-session":
				p.Session.Close()
			case "account-owner":
				delete(w.accountSessions, accountSessionKey(p.Account.Name))
			case "character-slot":
				p.CharSlot = 1
			case "out-of-world":
				p.InWorld = false
			case "poisoned":
				p.PersistencePoisoned = true
			case "stopping":
				queue.closeAdmission()
				wantErr = errCriticalPersistenceStopping
			case "missing-owner":
				w.criticalWrites = nil
				wantErr = errCriticalPersistenceUnsupported
			}
			before := accountStateSnapshot(p.Account)
			if err := w.queueAccountAutoSave(p, w.now()); !errors.Is(err, wantErr) {
				t.Fatalf("invalid autosave admission %s: %v", reason, err)
			}
			if len(queue.pending) != 0 || len(backend.entered) != 0 || p.criticalLifecycle != nil ||
				fallback.saves != 0 || fallback.batchSaves != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
				t.Fatal("rejected autosave changed state, saved inline or retained ownership")
			}
		})
	}
}

func TestCriticalAutosaveAccountOnlyAdapterUsesCoordinator(t *testing.T) {
	w, p, fallback, backend, queue, release := criticalCargoFixture(t)
	before := accountStateSnapshot(p.Account)
	runCriticalAutosaveSlice(w, p, w.now())
	write := awaitCriticalWrite(t, backend.entered)
	if write.character != nil || !reflect.DeepEqual(write.accounts[0], accountPersistenceSnapshot(p.Account)) ||
		fallback.saves != 0 || !w.criticalPlayerReserved(p) {
		t.Fatal("account-only autosave used an inline fallback or lost its reservation")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if w.criticalPlayerReserved(p) || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("account-only autosave acknowledgement mutated, published or leaked ownership")
	}
}

type criticalAutosaveTerminalStore struct {
	*batchGameStore
	final   []criticalWrite
	flushes int
}

func (s *criticalAutosaveTerminalStore) SavePlayerState(_ *model.GuildRegistry, account *model.Account,
	uid string, state *model.CharState) error {
	s.final = append(s.final, criticalWrite{accounts: []*model.Account{accountStateSnapshot(account)},
		character: &criticalCharacterState{uid: uid, state: state}})
	return nil
}
func (s *criticalAutosaveTerminalStore) Flush() { s.flushes++ }

func TestCriticalAutosaveShutdownDrainsBeforeFinalSnapshotAndCachesResult(t *testing.T) {
	w, p, fallback, backend, queue, release := criticalAutosaveFixture(t, 0)
	terminal := &criticalAutosaveTerminalStore{batchGameStore: fallback}
	w.store = terminal
	runCriticalAutosaveSlice(w, p, w.now())
	first := awaitCriticalWrite(t, backend.entered)
	waiters := []chan error{make(chan error, 1), make(chan error, 1)}
	for _, done := range waiters {
		w.runShutdown(done)
		select {
		case <-done:
			t.Fatal("shutdown replied before autosave acknowledgement")
		default:
		}
	}
	if len(terminal.final) != 0 || terminal.flushes != 0 || w.shutdownFinished || !queue.stopping {
		t.Fatal("shutdown performed terminal persistence before autosave settled")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	for _, done := range waiters {
		select {
		case err := <-done:
			if err != nil {
				t.Fatal(err)
			}
		default:
			t.Fatal("shutdown did not complete after autosave acknowledgement")
		}
	}
	if len(terminal.final) != 1 || !reflect.DeepEqual(terminal.final[0].accounts, first.accounts) ||
		!reflect.DeepEqual(terminal.final[0].character.state, buildCharState(p, w.now())) ||
		terminal.flushes != 1 || fallback.saves != 0 || fallback.batchSaves != 0 || len(queue.pending) != 0 {
		t.Fatal("terminal save omitted the atomic snapshot or persisted twice")
	}
	third := make(chan error, 1)
	w.runShutdown(third)
	if err := <-third; err != nil || len(terminal.final) != 1 || terminal.flushes != 1 {
		t.Fatal("repeated shutdown replayed persistence instead of sharing the cached result")
	}
}

func TestCriticalAutosaveDisconnectRetainsOwnershipUntilFinalAtomicAcknowledgement(t *testing.T) {
	w, p, fallback, backend, queue, release := criticalAutosaveFixture(t, 1)
	now := w.now()
	runCriticalAutosaveSlice(w, p, now)
	first := awaitCriticalWrite(t, backend.entered)
	w.handle(command{s: p.Session})
	w.handle(command{s: p.Session})
	if !p.Session.IsClosed() || w.players[p.Session] != p || len(backend.entered) != 0 ||
		fallback.saves != 0 || w.claimAccountSession(gameNet.NewTestSession(98, 4), p.Account.Name) {
		t.Fatal("disconnect released ownership or saved before pending autosave acknowledgement")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	final := awaitCriticalWrite(t, backend.entered)
	if !reflect.DeepEqual(final.accounts, first.accounts) || final.character == nil ||
		!reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) {
		t.Fatal("disconnect lost account or sidecar contents after autosave")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.handle(command{s: p.Session})
	if w.players[p.Session] != p || !w.criticalPlayerReserved(p) || len(backend.entered) != 0 {
		t.Fatal("unacknowledged final save released ownership or repeated persistence")
	}
	w.processCriticalCompletions(1)
	if w.players[p.Session] != nil || len(w.criticalDisconnects) != 0 || len(queue.pending) != 0 ||
		!w.claimAccountSession(gameNet.NewTestSession(99, 4), p.Account.Name) {
		t.Fatal("final acknowledgement did not release ownership for readmission")
	}
}

func TestCriticalAutosaveUnknownOutcomeRetainsFencesAndPreventsOldShutdownSave(t *testing.T) {
	for _, reason := range []string{"error", "panic", "identity"} {
		t.Run(reason, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalAutosaveFixture(t, 0)
			if reason == "error" {
				backend.failure = errors.New("unknown autosave outcome")
			}
			backend.panicAfterWrite = reason == "panic"
			before := accountStateSnapshot(p.Account)
			runCriticalAutosaveSlice(w, p, w.now())
			awaitCriticalWrite(t, backend.entered)
			if reason == "identity" {
				p.criticalLifecycle = &criticalPlayerLifecycle{}
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			runCriticalAutosaveSlice(w, p, w.now())
			done := make(chan error, 1)
			w.runShutdown(done)
			if !w.shuttingDown || !p.PersistencePoisoned || !p.Session.IsClosed() ||
				!w.criticalPlayerReserved(p) || len(queue.pending) != 1 || len(backend.entered) != 0 ||
				fallback.saves != 0 || fallback.batchSaves != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
				p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("unknown autosave was replayed, unfenced, applied or overwritten by shutdown")
			}
			select {
			case err := <-done:
				if err == nil {
					t.Fatal("unknown autosave shutdown reported success")
				}
			default:
				t.Fatal("unknown autosave shutdown did not report its failure")
			}
		})
	}
}

func TestCriticalAutosaveActualDueTickKeepsUnrelatedWorldProgress(t *testing.T) {
	for _, failed := range []bool{false, true} {
		t.Run(fmt.Sprintf("unknown-%t", failed), func(t *testing.T) {
			var failure error
			if failed {
				failure = errors.New("unknown actor autosave outcome")
			}
			transferGate, releaseTransfer := criticalGate(t)
			w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, failure,
				func(w *World, p *Player) {
					w.criticalWrites.queue.store = &criticalAutosaveLoopStore{
						criticalStateTestStore: &criticalStateTestStore{&criticalBatchTestStore{w.store.(*criticalTestStore)}},
						transferRelease:        transferGate,
					}
					w.nextAutoSave = w.now().Add(-time.Second)
					w.autoSaveBucket = accountAutoSaveBucket(p.Account.Name)
					p.SpecialCoins = map[string]uint32{"token": 7}
					for _, other := range w.players {
						if other != p && accountAutoSaveBucket(other.Account.Name) == w.autoSaveBucket {
							w.releaseAccountSession(other.Session, other.Account)
							other.Account.Name += "-other-bucket"
							for accountAutoSaveBucket(other.Account.Name) == w.autoSaveBucket {
								other.Account.Name += "x"
							}
							if !w.claimAccountSession(other.Session, other.Account.Name) {
								t.Fatal("unrelated fixture ownership failed")
							}
						}
					}
				})
			t.Cleanup(releaseTransfer)
			deliverCriticalLoopTick(t, ticks)
			write := awaitCriticalWrite(t, backend.entered)
			if write.character == nil || write.character.state.SpecialCoins["token"] != 7 || write.accounts[0].Name != p.Account.Name {
				t.Fatal("due actor tick did not submit atomic autosave")
			}
			for range 3 {
				deliverCriticalLoopPing(t, w, p)
				deliverCriticalLoopTick(t, ticks)
				overview := readCriticalLoopOverview(t, w)
				regenerated := false
				for _, player := range overview.Players {
					if player.UID == unrelatedUID && player.HP > 500 {
						regenerated = true
					}
				}
				if overview.State != "running" || !regenerated || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("held autosave blocked unrelated ticks or admitted a reserved economic request")
				}
			}
			release()
			deadline := time.Now().Add(2 * time.Second)
			requested := false
			for {
				overview := readCriticalLoopOverview(t, w)
				if failed && overview.State != "running" {
					break
				}
				if !failed {
					// One economic request sent while autosave may still be held
					// waits behind that reservation and runs after acknowledgement.
					// Its transfer is the actor-visible proof of release.
					if !requested {
						deliverCargoLoopRequest(t, w, p, true)
						requested = true
					}
					select {
					case transfer := <-backend.entered:
						if transfer.character != nil || transfer.accounts[0].Chars[0].Gold != 800 || transfer.accounts[0].CargoGold != 700 {
							t.Fatal("fresh economic request used incorrect post-autosave state")
						}
						goto completed
					default:
					}
				}
				if time.Now().After(deadline) {
					t.Fatal("actor did not acknowledge autosave")
				}
			}
		completed:
			readCriticalLoopOverview(t, w)
			stop()
			if failed {
				if !p.PersistencePoisoned || !p.Session.IsClosed() || len(queue.pending) != 1 || !errors.Is(w.shutdownErr, errCriticalPersistenceIndeterminate) {
					t.Fatal("unknown actor autosave failed to contain its outcome")
				}
			} else {
				releaseTransfer()
				// The actor may stop with a committed transfer awaiting delivery.
				// Complete it synchronously only after joining that actor.
				if len(queue.pending) != 0 {
					awaitReadyCriticalResults(t, queue, 1)
					w.processCriticalCompletions(1)
				}
				if p.Char.Gold != 800 || p.Account.CargoGold != 700 || w.shuttingDown || len(queue.pending) != 0 {
					t.Fatal("post-autosave economic commit failed")
				}
			}
		})
	}
}
