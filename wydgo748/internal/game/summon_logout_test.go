package game

import (
	"encoding/binary"
	"fmt"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestOwnerExitRemovesSummonsAndNotifiesObservers(t *testing.T) {
	for _, staged := range []bool{false, true} {
		for _, selection := range []bool{false, true} {
			name := "disconnect"
			if selection {
				name = "character-selection"
			}
			if staged {
				name += "/staged-save"
			} else {
				name += "/synchronous-save"
			}
			t.Run(name, func(t *testing.T) {
				w, owner, _ := handlerTestWorld(t)
				var queue *criticalPersistence
				var backend *criticalTestStore
				var release func()
				if staged {
					gate, unlock := criticalGate(t)
					release = unlock
					backend = &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
					queue = newCriticalTestQueue(t, &criticalStateTestStore{
						criticalBatchTestStore: &criticalBatchTestStore{criticalTestStore: backend},
					}, 1, 1)
					t.Cleanup(release)
					w.criticalWrites = newCriticalPersistenceOwner(queue)
					if !w.claimAccountSession(owner.Session, owner.Account.Name) {
						t.Fatal("owner account admission failed")
					}
				}
				observers := make([]*Player, 0, 2)
				for _, id := range []uint16{2, 3} {
					observer, _ := networkedTestPlayer(id, "Observer", owner.X+id, owner.Y)
					w.players[observer.Session], w.playersByID[id] = observer, observer
					w.updatePlayerSpatial(observer)
					observers = append(observers, observer)
				}
				owned := make(map[uint16]*Mob)
				for id := uint16(1450); id < 1455; id++ {
					mob := &Mob{ID: id, X: owner.X + 1, Y: owner.Y, HP: 100,
						SummonerID: owner.ID, SummonKind: summonKindBM,
						Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
					mob.Def.Name, mob.Def.Equip.Rosto.Index = "Tiger^", 244
					w.appendMobInstance(mob)
					w.registerMobSpatial(mob)
					owned[id] = mob
				}
				// A different owner's summon and an ordinary mob must remain.
				retained := []*Mob{
					{ID: 1460, X: owner.X + 1, Y: owner.Y, HP: 100,
						SummonerID: observers[0].ID, SummonKind: summonKindBM,
						Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})},
					{ID: 1470, X: owner.X + 1, Y: owner.Y, HP: 100,
						Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})},
				}
				for _, mob := range retained {
					w.appendMobInstance(mob)
					w.registerMobSpatial(mob)
				}
				for _, observer := range observers {
					for _, mob := range w.mobs {
						w.showMob(observer, mob)
						if !observer.hasVisible(mob.ID) {
							t.Fatalf("fixture mob %d was not visible", mob.ID)
						}
					}
					for observer.Session.QueuedPacketsForTest() > 0 {
						observer.Session.DequeuePacketForTest()
					}
				}
				if selection {
					w.onCharacterLogout(owner.Session, inboundPacket(wire.OpCharacterLogout, 12))
				} else {
					owner.Session.Close()
					w.onDisconnect(owner.Session)
				}
				if staged {
					awaitCriticalWrite(t, backend.entered)
					for id := range owned {
						if w.summons[id] == nil {
							t.Fatalf("summon %d removed before save acknowledgement", id)
						}
					}
					release()
					awaitReadyCriticalResults(t, queue, 1)
					w.processCriticalCompletions(1)
				}
				if w.playersByID[1] != nil {
					t.Fatal("owner exit did not complete")
				}
				if len(w.mobs) != len(retained) {
					t.Fatalf("remaining mobs = %d, want %d", len(w.mobs), len(retained))
				}
				for id, mob := range owned {
					_, hasCell := w.mobCell[id]
					_, hasListIndex := w.mobListIndex[id]
					if !mob.Dead || w.summons[id] != nil || w.mobsByID[id] != nil ||
						w.activeMobs[id] != nil || hasCell || hasListIndex {
						t.Fatalf("departed owner's summon %d retained runtime references", id)
					}
					for _, cell := range w.mobCells {
						if cell[id] != nil {
							t.Fatalf("summon %d retained in spatial cell", id)
						}
					}
				}
				for _, observer := range observers {
					removals := make(map[uint16]int)
					for {
						packet, ok := observer.Session.DequeuePacketForTest()
						if !ok {
							break
						}
						if !wire.Decrypt(packet) {
							t.Fatal("observer received an invalid packet")
						}
						header := wire.ParseHeader(packet)
						if header.Type == wire.OpRemoveMob {
							if len(packet) != 16 || binary.LittleEndian.Uint32(packet[12:16]) != 0 {
								t.Fatal("observer received an invalid summon removal")
							}
							removals[header.ID]++
						}
					}
					if len(removals) != len(owned) {
						t.Fatalf("observer %d removals = %v", observer.ID, removals)
					}
					for id := range owned {
						if observer.hasVisible(id) || removals[id] != 1 {
							t.Fatalf("observer %d retained summon %d or missed its removal", observer.ID, id)
						}
					}
					for _, mob := range retained {
						if !observer.hasVisible(mob.ID) || w.mobsByID[mob.ID] != mob || mob.Dead {
							t.Fatalf("unrelated mob %d was removed", mob.ID)
						}
					}
				}
			})
		}
	}
}

func TestOwnerExitSummonCleanupStaysInOwningChannel(t *testing.T) {
	for channel := 1; channel <= 2; channel++ {
		for _, selection := range []bool{false, true} {
			t.Run(fmt.Sprintf("channel-%d/selection-%t", channel, selection), func(t *testing.T) {
				cluster, one, two, players := channelIsolationFixture(t)
				worlds := []*World{one, two}
				summons := make([]*Mob, 2)
				for i, w := range worlds {
					owner, observer := players[i*2], players[i*2+1]
					mob := &Mob{ID: 1450, X: owner.X + 1, Y: owner.Y, HP: 100,
						SummonerID: owner.ID, SummonKind: summonKindBM,
						Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
					w.appendMobInstance(mob)
					w.registerMobSpatial(mob)
					w.showMob(observer, mob)
					if !observer.hasVisible(mob.ID) {
						t.Fatal("fixture summon was not visible")
					}
					channelIsolationPackets(t, observer)
					summons[i] = mob
				}
				source, remote := channel-1, 2-channel
				w, owner := worlds[source], players[source*2]
				cluster.inWorld(w, func() {
					if selection {
						w.onCharacterLogout(owner.Session, inboundPacket(wire.OpCharacterLogout, 12))
					} else {
						owner.Session.Close()
						w.onDisconnect(owner.Session)
					}
				})
				if w.playersByID[1] != nil || w.summons[1450] != nil || w.mobsByID[1450] != nil || !summons[source].Dead {
					t.Fatal("owner exit retained its channel's summon")
				}
				if !sentRemoveMob(players[source*2+1], 1450, 0) || players[source*2+1].hasVisible(1450) {
					t.Fatal("local observer did not receive summon removal")
				}
				remoteObserver := players[remote*2+1]
				if worlds[remote].summons[1450] != summons[remote] || worlds[remote].mobsByID[1450] != summons[remote] ||
					summons[remote].Dead || !remoteObserver.hasVisible(1450) ||
					len(channelIsolationPackets(t, remoteObserver)) != 0 {
					t.Fatal("cleanup removed or notified the other channel's identically numbered summon")
				}
			})
		}
	}
}
