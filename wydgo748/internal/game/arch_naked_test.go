package game

import (
	"fmt"
	"testing"

	"wydgo/internal/model"
)

// A new Arch starts naked: only the face survives from the class base set,
// as in DBSrv _MSG_DBCreateArchCharacter.
func TestArchStartsWithoutEquipment(t *testing.T) {
	for class := byte(0); class < 4; class++ {
		t.Run(fmt.Sprintf("class-%d", class), func(t *testing.T) {
			w, p, _ := newAscensionWorld(t)
			// Give every class a starting set so the strip is actually exercised.
			for class, template := range w.charTemplates {
				template.Equip = model.CharacterEquipment{
					Rosto: model.Item{Index: 1}, Elmo: model.Item{Index: 1104}, Armadura: model.Item{Index: 1116},
					Arma: model.Item{Index: 856}, Capa: model.Item{Index: 543},
				}
				template.Inventory = []model.Item{{Index: 400}, {Index: 401}}
				w.charTemplates[class] = template
			}
			prepareArchCandidate(p, int(class))
			if !w.createArch(p.Session, p) {
				t.Fatal("ascension was not handled")
			}
			if len(p.Account.Chars) < 2 {
				t.Fatal("the Arch was not created")
			}
			arch := p.Account.Chars[1]
			if arch.Equip[0].Index == 0 {
				t.Fatal("the Arch lost its face")
			}
			for slot := 1; slot < len(arch.Equip); slot++ {
				if arch.Equip[slot] != (model.Item{}) {
					t.Fatalf("Arch equipment slot %d holds item %d", slot, arch.Equip[slot].Index)
				}
			}
			for slot := 0; slot < len(arch.Inv)-4; slot++ {
				if arch.Inv[slot] != (model.Item{}) {
					t.Fatalf("Arch inventory slot %d holds item %d", slot, arch.Inv[slot].Index)
				}
			}
		})
	}
}

func TestStripArchStartingItemsPreservesFaceAndReservedInventory(t *testing.T) {
	var equip [16]model.Item
	var inv [64]model.Item
	item := model.Item{Index: 543, UID: "starting-item", Eff: [6]byte{1, 2, 3, 4, 5, 6}}
	for i := range equip {
		equip[i] = item
	}
	for i := range inv {
		inv[i] = item
	}
	stripArchStartingItems(&equip, &inv)
	for i := range equip {
		want := model.Item{}
		if i == 0 {
			want = item
		}
		if equip[i] != want {
			t.Fatalf("equipment slot %d: %+v", i, equip[i])
		}
	}
	for i := range inv {
		want := model.Item{}
		if i >= 60 {
			want = item
		}
		if inv[i] != want {
			t.Fatalf("inventory slot %d: %+v", i, inv[i])
		}
	}
}

func TestCriticalArchCreationStartsNakedAfterCommitOnly(t *testing.T) {
	for _, rejected := range []bool{false, true} {
		name := "commit"
		if rejected {
			name = "rejected"
		}
		t.Run(name, func(t *testing.T) {
			w, p, _ := newAscensionWorld(t)
			template := w.charTemplates[0]
			template.Equip.Arma = model.Item{Index: 856, UID: "starter-weapon"}
			template.Inventory = []model.Item{{Index: 400, UID: "starter-potion"}}
			w.charTemplates[0] = template
			prepareArchCandidate(p, 0)
			claimCriticalTestPlayers(t, w, p)
			before := p.Char.Equip
			gate, release := criticalGate(t)
			backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			if rejected {
				backend.failure = criticalCertifiedRejection(t)
			}
			queue := newCriticalTestQueue(t, backend, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			w.createArch(p.Session, p)
			write := awaitCriticalWrite(t, backend.entered)
			arch := write.accounts[0].Chars[1]
			for slot := 1; slot < len(arch.Equip); slot++ {
				if arch.Equip[slot] != (model.Item{}) {
					t.Fatalf("proposed Arch equipment slot %d is not empty", slot)
				}
			}
			for slot := 0; slot < 60; slot++ {
				if arch.Inv[slot] != (model.Item{}) {
					t.Fatalf("proposed Arch inventory slot %d is not empty", slot)
				}
			}
			if p.Account.Chars[1].Name != "" || p.Char.Equip != before || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("Arch creation was installed or published before persistence")
			}
			release()
			w.applyCriticalCompletion(awaitCriticalResult(t, queue))
			if rejected {
				if p.Account.Chars[1].Name != "" || p.Char.Equip != before {
					t.Fatal("rejected Arch creation consumed items")
				}
			} else if p.Account.Chars[1].Name == "" || p.Account.Chars[1].Equip != arch.Equip {
				t.Fatal("committed Arch differs from the persisted naked proposal")
			}
		})
	}
}

func TestArchIsBornInArmia(t *testing.T) {
	w, p, _ := newAscensionWorld(t)
	// The pre-fix template stored hometown bits 3 (Nippleheim, the ice city).
	for class, template := range w.charTemplates {
		template.Score.Merchant = 0xC0
		w.charTemplates[class] = template
	}
	prepareArchCandidate(p, 0)
	if !w.createArch(p.Session, p) || len(p.Account.Chars) < 2 {
		t.Fatal("the Arch was not created")
	}
	arch := &p.Account.Chars[1]
	if city := playerHomeCity(arch); city != 0 {
		t.Fatalf("Arch bound to %s, want Armia", cityWarZones[city].name)
	}
	if city, x, y := w.homeCitySpawn(arch); city != 0 || absInt(int(x)-2091) > armiaSpawnSpread || absInt(int(y)-2100) > armiaSpawnSpread {
		t.Fatalf("Arch spawn %s (%d,%d), want within ±5 of Armia 2091,2100", cityWarZones[city].name, x, y)
	}
}
