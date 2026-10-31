package game

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func configureCelestialShortcutForms(p *Player, first, second byte) ([20]byte, [20]byte) {
	ch := p.Char
	ch.Class, ch.Evolution = first, "celestial"
	ch.Equip[0] = model.Item{Index: 41 + uint16(first)}
	ch.LearnedSkill = celestialSoulBit | 1<<0 | 1<<5 | 1<<12
	ch.ShortSkill = emptyShortSkills()
	ch.ShortSkill[0], ch.ShortSkill[9], ch.ShortSkill[10], ch.ShortSkill[19] = 0, 5, first*24+12, 102
	ch.Inv[0] = model.Item{Index: mysteriousStoneItem, Eff: [6]byte{effectAmount, 10}}
	other := emptyShortSkills()
	other[1], other[10], other[18], other[19] = 3, second*24+7, 19, 102
	score := *ch.Score
	ch.AlternateCelestial = &model.CelestialForm{
		Evolution: "subcelestial", Class: second, Face: model.Item{Index: 41 + uint16(second)},
		Score: &score, LearnedSkill: celestialSoulBit | 1<<3 | 1<<7 | 1<<19, ShortSkill: other,
	}
	p.X, p.Y = 2100, 2100
	return ch.ShortSkill, other
}

func assertCelestialShortcutPublication(t *testing.T, p *Player) {
	t.Helper()
	face, learned, shortcuts := false, false, false
	for {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if !wire.Decrypt(packet) {
			t.Fatal("invalid form publication")
		}
		switch wire.ParseHeader(packet).Type {
		case wire.OpSendItem:
			if binary.LittleEndian.Uint16(packet[12:14]) == placeEquip && binary.LittleEndian.Uint16(packet[14:16]) == 0 {
				want := wire.SendItem(p.ID, placeEquip, 0, wire.CharacterFaceItem(*p.Char))
				if !bytes.Equal(packet[12:], want[12:]) {
					t.Fatal("face differs from the committed class or cosmetic projection")
				}
				face = true
			}
		case wire.OpUpdateEtc:
			if !face || binary.LittleEndian.Uint32(packet[20:24]) != p.Char.LearnedSkill {
				t.Fatal("learned skills arrived before class or differ from the active form")
			}
			learned = true
		case wire.OpSetShortSkill:
			if !face || !learned || !bytes.Equal(packet[12:], p.Char.ShortSkill[:]) {
				t.Fatal("shortcuts arrived before class/learned skills or differ from the active form")
			}
			shortcuts = true
		}
	}
	if !face || !learned || !shortcuts {
		t.Fatal("incomplete class, learned-skill or shortcut publication")
	}
}

func TestMysteriousStoneRestoresBothShortcutPagesAcrossClasses(t *testing.T) {
	for first := byte(0); first < 4; first++ {
		for second := byte(0); second < 4; second++ {
			t.Run(fmt.Sprintf("%d-to-%d", first, second), func(t *testing.T) {
				w, p, _ := handlerTestWorld(t)
				initial, alternate := configureCelestialShortcutForms(p, first, second)
				for i := 0; i < 4; i++ {
					w.useMysteriousStone(p.Session, p, &p.Char.Inv[0], 0)
					want, class := alternate, second
					evolution, faceType := "subcelestial", subCelestialFaceType
					if i%2 == 1 {
						want, class = initial, first
						evolution, faceType = "celestial", celestialFaceType
					}
					if p.Char.ShortSkill != want || p.Char.Class != class || p.Char.Evolution != evolution ||
						p.Char.Equip[0].Eff[3] != faceType || itemStackAmount(p.Char.Inv[0]) != uint32(9-i) {
						t.Fatalf("switch %d lost shortcuts or class: %+v", i, p.Char.ShortSkill)
					}
					assertCelestialShortcutPublication(t, p)
					// Rebuild the persisted account representation before switching back.
					loaded := accountStateSnapshot(p.Account)
					*p.Char = loaded.Chars[p.CharSlot]
				}
			})
		}
	}
}

func TestCriticalMysteriousStonePublishesShortcutsOnlyAfterCommit(t *testing.T) {
	for _, fail := range []bool{false, true} {
		t.Run(fmt.Sprintf("failure-%t", fail), func(t *testing.T) {
			w, p, _, backend, queue, _, _, release := criticalTinyFixture(t, true, true)
			_, alternate := configureCelestialShortcutForms(p, 0, 3)
			before := accountStateSnapshot(p.Account)
			if fail {
				backend.failure = errors.New("injected form save failure")
			}
			if err := w.queueCelestialConsumable(p, 0, "celestial_switch"); err != nil {
				t.Fatal(err)
			}
			proposal := awaitCriticalWrite(t, backend.entered)
			if proposal.accounts[0].Chars[p.CharSlot].ShortSkill != alternate ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("uncommitted form changed live state or published shortcuts")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if fail {
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
					t.Fatal("failed commit changed form or shortcuts")
				}
				for {
					packet, ok := p.Session.DequeuePacketForTest()
					if !ok {
						break
					}
					if !wire.Decrypt(packet) {
						t.Fatal("invalid rejection frame")
					}
					if wire.ParseHeader(packet).Type == wire.OpSetShortSkill {
						t.Fatal("failed commit published new shortcuts")
					}
				}
				return
			}
			if p.Char.ShortSkill != alternate || p.Char.Class != 3 {
				t.Fatal("committed form lost shortcuts or class")
			}
			assertCelestialShortcutPublication(t, p)
		})
	}
}
