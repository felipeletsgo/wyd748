package game

import (
	"encoding/binary"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestCitizenshipPublishesOnlyAfterCriticalCommit(t *testing.T) {
	for _, emblem := range []byte{0, 7} {
		for _, rejected := range []bool{false, true} {
			name := "commit"
			if rejected {
				name = "rejected"
			}
			t.Run(fmt.Sprintf("%s/emblem%d", name, emblem), func(t *testing.T) {
				w := newZoneTestWorld()
				w.channel = 3
				p := addZonePlayer(w, 1, 2100, 2100, 500)
				p.Char.Gold = 200
				p.Char.CapeEmblem = emblem
				visual := byte(3)
				if emblem != 0 {
					visual = emblem
				}
				p.Char.Equip[0] = model.Item{Index: 1}
				observer := addZonePlayer(w, 2, 2101, 2100, 500)
				observer.show(p.ID)
				claimCriticalTestPlayers(t, w, p)
				gate, release := criticalGate(t)
				backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
				if rejected {
					backend.failure = criticalCertifiedRejection(t)
				}
				queue := newCriticalTestQueue(t, &criticalStateTestStore{&criticalBatchTestStore{backend}}, 1, 2)
				t.Cleanup(release)
				w.criticalWrites = newCriticalPersistenceOwner(queue)
				quest := model.QuestDef{ID: 89, NPC: "Kibita", Repeatable: true,
					Requires: model.QuestRequirements{Gold: 100},
					Messages: model.QuestMessages{Success: "Citizenship granted."}}
				quest.Rewards.Citizenship = true
				w.executeQuest(p.Session, p, &Mob{}, &quest)
				write := awaitCriticalWrite(t, backend.entered)
				if write.accounts[0].Chars[0].Citizenship != 3 || write.accounts[0].Chars[0].Gold != 100 {
					t.Fatal("citizenship and payment were not in the same proposal")
				}
				if p.Char.Citizenship != 0 || p.Char.Gold != 200 || p.Session.QueuedPacketsForTest() != 0 || observer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("pending citizenship mutated or published live state")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				if p.Char.Citizenship != 0 || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("worker completion published before actor acknowledgement")
				}
				w.processCriticalCompletions(1)
				if p.Char.CapeEmblem != emblem || write.accounts[0].Chars[0].CapeEmblem != emblem {
					t.Fatal("citizenship purchase replaced the independently saved cape symbol")
				}
				observerPacket, observerPublished := observer.Session.DequeuePacketForTest()
				if observerPublished && (!wire.Decrypt(observerPacket) || wire.ParseHeader(observerPacket).Type != wire.OpCreateMob || observerPacket[326] != visual) {
					t.Fatal("observer received an invalid citizenship snapshot")
				}
				if observerPublished == rejected {
					t.Fatal("observer publication did not match the persistence outcome")
				}
				face, snapshot := false, false
				for {
					packet, ok := p.Session.DequeuePacketForTest()
					if !ok {
						break
					}
					if !wire.Decrypt(packet) {
						t.Fatal("invalid outbound checksum")
					}
					switch wire.ParseHeader(packet).Type {
					case wire.OpSendItem:
						face = true
					case wire.OpCreateMob:
						if packet[326] != visual {
							t.Fatal("snapshot lost the visual cape projection")
						}
						snapshot = true
					}
				}
				if rejected {
					if p.Char.Citizenship != 0 || p.Char.Gold != 200 || face || snapshot {
						t.Fatal("rejected save published a cape emblem or consumed gold")
					}
				} else {
					if p.Char.Citizenship != 3 || p.Char.Gold != 100 || !face || !snapshot {
						t.Fatal("committed citizenship was not installed and published")
					}
					if _, ok := w.questRequirementsMet(p, &quest); ok {
						t.Fatal("citizenship can be bought twice")
					}
				}
			})
		}
	}
}

// Buying citizenship while transformed re-sends the owner's visual equipment
// after the slot-0 face update, so the BM form does not vanish under its buff.
func TestCitizenshipKeepsActiveTransformationVisible(t *testing.T) {
	w := newZoneTestWorld()
	w.channel = 3
	p := addZonePlayer(w, 1, 2100, 2100, 500)
	p.Char.Gold = 200
	p.Char.Class = 2
	p.Char.Equip[0] = model.Item{Index: 1}
	p.Char.Affects[0] = model.Affect{Type: 16, Value: 1, Level: 200, ExpiresAt: w.now().Add(time.Minute)}
	quest := model.QuestDef{ID: 89, NPC: "Kibita", Repeatable: true,
		Requires: model.QuestRequirements{Gold: 100},
		Messages: model.QuestMessages{Success: "Citizenship granted."}}
	quest.Rewards.Citizenship = true
	w.executeQuest(p.Session, p, &Mob{}, &quest)
	if p.Char.Citizenship != 3 {
		t.Fatal("citizenship was not granted")
	}
	faceAt, visualAt, index := -1, -1, 0
	for {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if !wire.Decrypt(packet) {
			t.Fatal("invalid outbound checksum")
		}
		switch wire.ParseHeader(packet).Type {
		case wire.OpSendItem:
			faceAt = index
		case wire.OpUpdateEquip:
			if binary.LittleEndian.Uint16(packet[12:14])&0x0FFF == 22 {
				visualAt = index
			}
		}
		index++
	}
	if faceAt < 0 || visualAt < faceAt {
		t.Fatalf("transformation was not re-sent after the face update: face=%d visual=%d", faceAt, visualAt)
	}
}
