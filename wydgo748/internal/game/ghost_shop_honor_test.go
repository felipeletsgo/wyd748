package game

import (
	"errors"
	"testing"
	"time"
)

// An active ghost shop earns its owner one honor every ten minutes.
func TestGhostShopEarnsHonorEveryTenMinutes(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	start := time.Unix(1_800_000_000, 0)
	shop := &GhostShop{ID: ghostShopIDBase + p.ID, OwnerID: p.ID}
	p.GhostShop = shop
	w.ghostShops[shop.ID] = shop

	w.tickGhostShopHonor(start)
	w.tickGhostShopHonor(start.Add(9 * time.Minute))
	if got := counterBalance(p, honorCounter); got != 0 {
		t.Fatalf("honor before ten minutes = %d", got)
	}
	w.tickGhostShopHonor(start.Add(10 * time.Minute))
	w.tickGhostShopHonor(start.Add(20 * time.Minute))
	if got := counterBalance(p, honorCounter); got != 2 {
		t.Fatalf("honor after twenty minutes = %d, want 2", got)
	}
	if st.saves != 2 {
		t.Fatalf("honor grants persisted %d times, want 2", st.saves)
	}

	packetsBefore := p.Session.QueuedPacketsForTest()
	st.err = errors.New("disk unavailable")
	w.tickGhostShopHonor(start.Add(30 * time.Minute))
	if got := counterBalance(p, honorCounter); got != 2 || p.Session.QueuedPacketsForTest() != packetsBefore {
		t.Fatalf("failed honor persistence changed live state: honor=%d packets=%d/%d",
			got, p.Session.QueuedPacketsForTest(), packetsBefore)
	}
	st.err = nil
	w.tickGhostShopHonor(start.Add(31 * time.Minute))
	if got := counterBalance(p, honorCounter); got != 3 {
		t.Fatalf("honor was not retried after persistence failure: %d", got)
	}

	p.GhostShop = nil // closed shop earns nothing
	w.tickGhostShopHonor(start.Add(50 * time.Minute))
	if got := counterBalance(p, honorCounter); got != 3 {
		t.Fatalf("closed shop earned honor: %d", got)
	}
}

func TestGhostShopHonorCriticalCommitAndRetry(t *testing.T) {
	for _, rejected := range []bool{false, true} {
		t.Run(map[bool]string{false: "commit", true: "rejection_then_retry"}[rejected], func(t *testing.T) {
			w, p, fallback := handlerTestWorld(t)
			p.Char.UID = "honor-owner"
			w.playersByCharacterUID[p.Char.UID] = p
			w.claimAccountSession(p.Session, p.Account.Name)
			gate, release := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			if rejected {
				base.failure = criticalCertifiedRejection(t)
			}
			q := newCriticalTestQueue(t, &criticalStateTestStore{&criticalBatchTestStore{base}}, 1, 4)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(q)
			shop := &GhostShop{ID: ghostShopIDBase + p.ID, OwnerID: p.ID}
			p.GhostShop = shop
			w.ghostShops[shop.ID] = shop
			start := time.Unix(1_800_000_000, 0)
			due := start.Add(ghostShopHonorInterval)
			w.tickGhostShopHonor(start)
			packets := p.Session.QueuedPacketsForTest()
			w.tickGhostShopHonor(due)
			write := awaitCriticalWrite(t, base.entered)
			if write.character == nil || write.character.uid != p.Char.UID || write.character.state.SpecialCoins[honorCounter] != 1 {
				t.Fatal("honor was not included in the atomic character sidecar")
			}
			w.tickGhostShopHonor(due.Add(time.Second))
			if counterBalance(p, honorCounter) != 0 || p.Session.QueuedPacketsForTest() != packets || !w.ghostShopHonorAt[shop.ID].Equal(due) || len(base.entered) != 0 {
				t.Fatal("held honor write changed live state or admitted a duplicate")
			}
			release()
			awaitReadyCriticalResults(t, q, 1)
			w.processCriticalCompletions(1)
			if rejected {
				if counterBalance(p, honorCounter) != 0 || !w.ghostShopHonorAt[shop.ID].Equal(due) || p.Session.QueuedPacketsForTest() != packets {
					t.Fatal("rejected honor write published a grant or advanced its deadline")
				}
				base.failure = nil
				w.tickGhostShopHonor(due.Add(2 * time.Second))
				awaitCriticalWrite(t, base.entered)
				awaitReadyCriticalResults(t, q, 1)
				w.processCriticalCompletions(1)
			}
			if counterBalance(p, honorCounter) != 1 || !w.ghostShopHonorAt[shop.ID].Equal(due.Add(ghostShopHonorInterval)) || p.Session.QueuedPacketsForTest() != packets+1 || fallback.saves != 0 {
				t.Fatal("committed honor did not publish exactly once through the coordinator")
			}
			w.tickGhostShopHonor(due.Add(3 * time.Second))
			if len(base.entered) != 0 || counterBalance(p, honorCounter) != 1 {
				t.Fatal("committed interval was awarded again")
			}
		})
	}
}
