package game

import (
	"bytes"
	"errors"
	"fmt"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func testMergeStack(uid string, amount byte) model.Item {
	return model.Item{Index: 412, UID: uid, Eff: [6]byte{effectAmount, amount, 2, 7}}
}

func TestStackMergeConservesEveryByteQuantity(t *testing.T) {
	for targetAmount := 1; targetAmount <= 255; targetAmount++ {
		for originAmount := 1; originAmount <= 255; originAmount++ {
			target := testMergeStack("target", byte(targetAmount))
			origin := testMergeStack("origin", byte(originAmount))
			gotTarget, gotOrigin, ok := mergeStackItems(target, origin)
			remaining := 0
			if gotOrigin.Index != 0 {
				remaining = int(itemStackAmount(gotOrigin))
				if gotOrigin.UID != origin.UID {
					t.Fatal("residual stack lost its identity")
				}
			} else if gotOrigin != (model.Item{}) {
				t.Fatal("consumed stack retained instance data")
			}
			if !ok || gotTarget.UID != target.UID || int(itemStackAmount(gotTarget)) != min(255, targetAmount+originAmount) ||
				int(itemStackAmount(gotTarget))+remaining != targetAmount+originAmount || !bytes.Equal(gotTarget.Eff[2:], target.Eff[2:]) {
				t.Fatalf("merge %d + %d lost quantity, effects or identity", targetAmount, originAmount)
			}
		}
	}
}

func TestStackMergeRejectsIncompatibleInstances(t *testing.T) {
	for _, mode := range []string{"index", "empty", "mount", "zero", "missing-amount", "duplicate-amount", "effect", "duplicate-effect", "empty-effect-data", "activation", "expiry", "same-uid"} {
		t.Run(mode, func(t *testing.T) {
			target, origin := testMergeStack("target", 20), testMergeStack("origin", 30)
			switch mode {
			case "index":
				origin.Index++
			case "empty":
				origin.Index = 0
			case "mount":
				target.Index, origin.Index = 2330, 2330
			case "zero":
				origin.Eff[1] = 0
			case "missing-amount":
				origin.Eff[0] = 3
			case "duplicate-amount":
				origin.Eff[4], origin.Eff[5] = effectAmount, 4
			case "effect":
				origin.Eff[3]++
			case "duplicate-effect":
				origin.Eff[4], origin.Eff[5] = 2, 7
			case "empty-effect-data":
				origin.Eff[5] = 7
			case "activation":
				origin.ActivatedUnix = 100
			case "expiry":
				origin.ExpiresUnix = 200
			case "same-uid":
				origin.UID = target.UID
			}
			gotTarget, gotOrigin, ok := mergeStackItems(target, origin)
			if ok || gotTarget != target || gotOrigin != origin {
				t.Fatal("incompatible merge changed authoritative instances")
			}
		})
	}
	for pair := 0; pair < 3; pair++ {
		target, origin := testMergeStack("target", 20), testMergeStack("origin", 30)
		origin.Eff = [6]byte{}
		origin.Eff[pair*2], origin.Eff[pair*2+1] = effectAmount, 30
		other := (pair + 1) % 3
		origin.Eff[other*2], origin.Eff[other*2+1] = 2, 7
		got, empty, ok := mergeStackItems(target, origin)
		if !ok || itemStackAmount(got) != 50 || empty != (model.Item{}) {
			t.Fatal("effect ordering must not prevent equivalent stacks from merging")
		}
	}
}

func TestStackMergeCriticalLifecycleAndNativePublication(t *testing.T) {
	for _, places := range [][2]byte{{placeInv, placeInv}, {placeInv, placeStorage}, {placeStorage, placeInv}, {placeStorage, placeStorage}} {
		for _, partial := range []bool{false, true} {
			for _, outcome := range []string{"commit", "rejected", "unknown"} {
				t.Run(fmt.Sprintf("%d-%d/partial=%t/%s", places[0], places[1], partial, outcome), func(t *testing.T) {
					w, p, fallback, backend, queue, release := criticalCargoFixture(t)
					if outcome == "rejected" {
						backend.failure = criticalCertifiedRejection(t)
					}
					if outcome == "unknown" {
						backend.failure = errors.New("injected uncertain merge")
					}
					st, dt := places[0], places[1]
					target, origin := playerSlotOf(p, st, 3), playerSlotOf(p, dt, 7)
					*target, *origin = testMergeStack("target", 20), testMergeStack("origin", 30)
					if partial {
						target.Eff[1] = 250
					}
					wantTarget, wantOrigin, _ := mergeStackItems(*target, *origin)
					before := accountStateSnapshot(p.Account)
					request := criticalCargoSwapRequest(p, st, 3, dt, 7)
					w.onSwapItem(p.Session, request)
					write := awaitCriticalWrite(t, backend.entered)
					proposal := &Player{Account: write.accounts[0], Char: &write.accounts[0].Chars[p.CharSlot]}
					if *playerSlotOf(proposal, st, 3) != wantTarget || *playerSlotOf(proposal, dt, 7) != wantOrigin {
						t.Fatal("durable merge proposal differs from the intended cells")
					}
					for _, completed := range []bool{false, true} {
						if completed {
							release()
							awaitReadyCriticalResults(t, queue, 1)
						}
						w.onSwapItem(p.Session, request)
						assertCriticalCargoUnchanged(t, p, before, fallback)
						if p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 || len(backend.entered) != 0 {
							t.Fatal("pending merge was published, duplicated or saved inline")
						}
					}
					w.processCriticalCompletions(1)
					if outcome != "commit" {
						assertCriticalCargoUnchanged(t, p, before, fallback)
						for {
							packet, ok := p.Session.DequeuePacketForTest()
							if !ok {
								break
							}
							if !wire.Decrypt(packet) {
								t.Fatal("invalid packet")
							}
							if wire.ParseHeader(packet).Type == wire.OpSwapItem {
								t.Fatal("uncommitted merge was acknowledged")
							}
						}
						if outcome == "unknown" && (!p.PersistencePoisoned || !w.criticalPlayerReserved(p)) {
							t.Fatal("unknown merge lost its fence")
						}
						if outcome == "rejected" && (p.PersistencePoisoned || w.criticalPlayerReserved(p)) {
							t.Fatal("certified rejection retained a false poison/fence")
						}
						return
					}
					if *target != wantTarget || *origin != wantOrigin || w.criticalPlayerReserved(p) || fallback.saves != 0 {
						t.Fatal("committed merge did not install the proposal")
					}
					assertCriticalPickupPacket(t, p, wire.SwapItem(p.ID, st, 3, dt, 7, 1102))
					assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, st, 3, wantTarget))
					assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, dt, 7, wantOrigin))
					if p.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("unexpected merge publication")
					}
				})
			}
		}
	}
}

func TestStackMergeFullTargetAndMissingCoordinatorDoNotSwapOrSave(t *testing.T) {
	for _, mode := range []string{"full", "no-coordinator", "no-warehouse-access", "same-slot"} {
		t.Run(mode, func(t *testing.T) {
			w, p, fallback, _, queue, _ := criticalCargoFixture(t)
			p.Char.Inv[3], p.Char.Inv[7] = testMergeStack("target", 20), testMergeStack("origin", 30)
			request := criticalCargoSwapRequest(p, placeInv, 3, placeInv, 7)
			switch mode {
			case "full":
				p.Char.Inv[3].Eff[1] = 255
			case "no-coordinator":
				w.criticalWrites = nil
			case "no-warehouse-access":
				p.Account.Cargo[7] = p.Char.Inv[7]
				request[14] = placeStorage
				p.CargoNPC = 0
				delete(p.Visible, 1102)
			case "same-slot":
				request[15] = 3
			}
			before := accountStateSnapshot(p.Account)
			w.onSwapItem(p.Session, request)
			assertCriticalCargoUnchanged(t, p, before, fallback)
			if len(queue.pending) != 0 {
				t.Fatal("non-mutating/rejected merge entered persistence")
			}
			if mode == "full" {
				assertCriticalPickupPacket(t, p, wire.SwapItem(p.ID, placeInv, 3, placeInv, 7, 1102))
				assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, 3, p.Char.Inv[3]))
				assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, 7, p.Char.Inv[7]))
			}
		})
	}
}
