package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

// These acceptance tests exposed shared-object settlement defects and now
// guard their corrections. Never accept stranded reservations as success.
func TestClosureAuditGroundSettlement(t *testing.T) {
	for _, operation := range []string{"pickup", "drop"} {
		for _, outcome := range []string{"commit", "rejection", "unknown"} {
			t.Run(operation+"/"+outcome, func(t *testing.T) {
				w, p, ground, _, backend, queue, release := criticalPickupFixture(t)
				w.criticalAccountContainment = true
				w.clock = newFakeClock(time.Unix(100, 0))
				if outcome == "rejection" {
					backend.failure = criticalCertifiedRejection(t)
				} else if outcome == "unknown" {
					backend.failure = errors.New("ground transaction acknowledgement lost")
				}
				if operation == "drop" {
					p.Char.Inv[0] = ground.Item
					w.unregisterGroundItem(ground)
					w.items[ground.Item.Index] = model.ItemDef{Index: ground.Item.Index}
				}
				before := accountStateSnapshot(p.Account)
				if operation == "pickup" {
					w.onGetItem(p.Session, criticalPickupRequest(ground.ID, 0))
				} else {
					w.onDropItem(p.Session, criticalDropRequest(placeInv, 0))
					ground = pendingCriticalDrop(t, w)
				}
				awaitCriticalWrite(t, backend.entered)
				if !ground.criticalReserved() || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
					t.Fatal("pending ground operation lost its reservation or changed live economy")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				switch outcome {
				case "commit":
					if ground.criticalReserved() || w.criticalPlayerReserved(p) || p.PersistencePoisoned {
						t.Fatal("successful ground operation retained a fence or poisoned the account")
					}
					if operation == "pickup" && (w.groundItems[ground.ID] != nil || p.Char.Inv[0] != ground.Item) {
						t.Fatal("committed pickup did not install its item and remove the object")
					}
					if operation == "drop" && (w.groundItems[ground.ID] != ground || p.Char.Inv[0].Index != 0) {
						t.Fatal("committed drop did not publish its allocation and remove the source")
					}
				case "rejection":
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
						t.Fatal("certified rejection changed live economy")
					}
					if p.PersistencePoisoned || p.Session.IsClosed() || w.shuttingDown || w.criticalPlayerReserved(p) {
						t.Error("certified ground rejection was escalated instead of settled safely")
					}
					if operation == "pickup" && (w.groundItems[ground.ID] != ground || ground.criticalReserved()) {
						t.Error("rejected pickup stranded the unchanged ground object")
					}
					if operation == "drop" && w.groundItems[ground.ID] != nil {
						t.Error("rejected drop retained its unpublished allocation")
					}
				case "unknown":
					if !ground.criticalReserved() || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
						t.Fatal("unknown ground outcome was replayed or rolled back unsafely")
					}
					// Shared-object uncertainty must remain contained or be reconciled.
					// An elapsed account-only timer is not object reconciliation.
					w.handle(command{s: p.Session})
					w.tick()
					w.expireGroundItems(w.now().Add(24 * time.Hour))
					if !w.shuttingDown && !w.criticalAccountReserved(p.Account.Name) &&
						w.groundItems[ground.ID] == ground && ground.criticalReserved() {
						t.Fatal("account fence released but its ground object is permanently reserved and cannot expire")
					}
				}
			})
		}
	}
}

func TestClosureAuditWeatherResumesAfterAccountAcknowledgement(t *testing.T) {
	w, p, ground, _, backend, queue, release := criticalPickupFixture(t)
	observer := criticalPickupCompetitor(t, w, ground)
	w.weather = weatherRain
	w.weatherRNG = nil
	if _, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{accountStateSnapshot(p.Account)}},
		[]*Player{p}, func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	origin := time.Unix(200, 0)
	w.tickWeather(origin)
	if p.SentWeather != weatherClear || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("weather published to a persistence-reserved player")
	}
	if observer.SentWeather != weatherRain || observer.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("healthy observer did not receive the current weather")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	w.tickWeather(origin.Add(weatherSyncInterval))
	if p.SentWeather != weatherRain || p.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("acknowledged player did not catch up exactly once")
	}
	w.tickWeather(origin.Add(2 * weatherSyncInterval))
	if p.Session.QueuedPacketsForTest() != 1 || observer.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("unchanged weather was sent again")
	}
}

func TestClosureAuditKillSettlement(t *testing.T) {
	for _, outcome := range []string{"commit", "rejection", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			w, p, mob, _ := killRewardTestWorld(t, nil)
			claimCriticalTestPlayers(t, w, p)
			gate, release := criticalGate(t)
			backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
			if outcome == "rejection" {
				backend.failure = criticalCertifiedRejection(t)
			} else if outcome == "unknown" {
				backend.failure = errors.New("kill reward acknowledgement lost")
			}
			queue := newCriticalTestQueue(t, backend, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			w.criticalAccountContainment = true
			before := accountStateSnapshot(p.Account)
			w.killMobState(p, mob, 500, 37)
			awaitCriticalWrite(t, backend.entered)
			if !mob.Dead || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
				t.Fatal("pending kill did not retain its unpublished death and account preimage")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			switch outcome {
			case "commit":
				if w.mobsByID[mob.ID] == mob || w.criticalPlayerReserved(p) {
					t.Fatal("committed kill did not retire the mob and release its participant")
				}
			case "rejection":
				if mob.Dead || mob.HP != 37 || p.PersistencePoisoned || w.criticalPlayerReserved(p) ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
					t.Fatal("certified kill rejection did not restore the mob and account")
				}
			case "unknown":
				w.handle(command{s: p.Session})
				w.tick()
				if !w.shuttingDown && !w.criticalAccountReserved(p.Account.Name) &&
					w.mobsByID[mob.ID] == mob && mob.Dead {
					t.Fatal("unknown kill released its account but stranded an unpublished dead mob without reconciliation")
				}
			}
		})
	}
}

func TestClosureAuditWeatherScheduleDoesNotConsumeGameplayRNG(t *testing.T) {
	gameplay := &sequenceRNG{values: []int{17, 23}}
	w := &World{rng: gameplay, weatherRNG: &sequenceRNG{values: []int{900, 0, 99}}}
	origin := time.Unix(200, 0)
	w.tickWeather(origin)
	if !w.nextWeatherChange.Equal(origin.Add(weatherMaxDuration)) {
		t.Fatal("inclusive maximum weather duration was not scheduled")
	}
	w.tickWeather(w.nextWeatherChange)
	if w.weather != weatherHeavySnow || !w.nextWeatherChange.Equal(origin.Add(weatherMaxDuration+weatherMinDuration)) {
		t.Fatal("minimum duration or heavy-snow selection changed")
	}
	if !reflect.DeepEqual(gameplay.values, []int{17, 23}) {
		t.Fatal("weather scheduling consumed gameplay randomness")
	}
}
