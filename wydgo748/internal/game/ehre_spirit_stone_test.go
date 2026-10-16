package game

import (
	"errors"
	"fmt"
	"reflect"
	"testing"

	"wydgo/internal/model"
)

func spiritStoneRequest(first, second uint16) combineRequest {
	return combineRequest{Items: [combineSlots]model.Item{
		{Index: first}, {Index: second}, {Index: 633, UID: "spirit-stone", Eff: [6]byte{43, 9}},
	}, Pos: [combineSlots]int8{0, 1, 2}}
}

func TestEhreSpiritStoneAdditions(t *testing.T) {
	for first := uint16(661); first <= 663; first++ {
		for second := uint16(661); second <= 663; second++ {
			t.Run(fmt.Sprintf("%d-%d", first, second), func(t *testing.T) {
				req := spiritStoneRequest(first, second)
				result, ok := ehreSpiritStone(req)
				if !ok || result.UID != req.Items[2].UID || itemSanc(result) != 7 {
					t.Fatal("valid composition lost identity or refinement")
				}
				want := map[byte]int{}
				for _, index := range []uint16{first, second} {
					effect, amount := byte(70), 2
					if index == 662 {
						effect = 69
					}
					if index == 663 {
						effect, amount = 71, 10
					}
					want[effect] += amount
				}
				got := map[byte]int{}
				for _, i := range []int{2, 4} {
					if result.Eff[i] != 0 {
						got[result.Eff[i]] += int(result.Eff[i+1])
					}
				}
				if !reflect.DeepEqual(got, want) {
					t.Fatalf("additions %v, want %v", got, want)
				}
			})
		}
	}
	for _, eff := range [][6]byte{{43, 9, 69, 20}, {43, 9, 70, 20}, {43, 9, 71, 100},
		{43, 9, 69, 10, 70, 10}, {43, 9, 69, 10, 71, 50}, {43, 9, 70, 10, 71, 50},
		{43, 9, 69, 2, 71, 10}, {43, 8}} {
		req := spiritStoneRequest(661, 661)
		req.Items[2].Eff = eff
		if _, ok := ehreSpiritStone(req); ok {
			t.Fatalf("accepted blocked effects %v", eff)
		}
	}
	req := spiritStoneRequest(662, 662)
	req.Items[2].Eff = [6]byte{43, 9, 69, 18}
	result, ok := ehreSpiritStone(req)
	if !ok || result.Eff != [6]byte{43, 7, 69, 20} {
		t.Fatal("HP addition did not clamp at 20")
	}
	req.Items[3].Index = 413
	if _, ok := ehreSpiritStone(req); ok {
		t.Fatal("accepted extra ingredient")
	}
	req = spiritStoneRequest(660, 661)
	if _, ok := ehreSpiritStone(req); ok {
		t.Fatal("accepted non-Ankh ingredient")
	}
}

func TestEhreSpiritStoneHandler(t *testing.T) {
	for _, mode := range []string{"success", "failure", "rollback", "forged", "duplicate"} {
		t.Run(mode, func(t *testing.T) {
			w, p, session, store := newCraftWorld(t, "Ehre", map[uint16]model.ItemDef{633: {Index: 633}}, 123)
			req := spiritStoneRequest(661, 662)
			placeItems(p.Char, req.Items, req.Pos)
			before := p.Char.Inv
			rng := &advancedCraftRNG{roll: 50}
			w.rng = rng
			switch mode {
			case "failure":
				rng.roll = 51
			case "rollback":
				store.err = errors.New("save rejected")
			case "forged":
				req.Items[0].Index = 663
			case "duplicate":
				req.Pos[1] = req.Pos[0]
			}
			packet := buildCombinePacket(req.Items, req.Pos)
			w.onCombineEhre(session, packet)
			if mode == "rollback" || mode == "forged" || mode == "duplicate" {
				if p.Char.Inv != before {
					t.Fatal("rejected composition changed inventory")
				}
				return
			}
			if store.saves != 1 || rng.calls != 1 || p.Char.Gold != 123 {
				t.Fatal("incorrect cost, save count or roll count")
			}
			if p.Char.Inv[0].Index != 0 || p.Char.Inv[1].Index != 0 {
				t.Fatal("Ankhs were not consumed")
			}
			if mode == "success" {
				if p.Char.Inv[2].Eff != [6]byte{43, 7, 70, 2, 69, 2} || p.Char.Inv[2].UID != before[2].UID {
					t.Fatal("incorrect success result")
				}
			} else if p.Char.Inv[2].Index != 0 {
				t.Fatal("failure retained the stone")
			}
			w.onCombineEhre(session, packet)
			if store.saves != 1 || rng.calls != 1 {
				t.Fatal("replay charged or rolled again")
			}
		})
	}
}

func TestCriticalEhreSpiritStone(t *testing.T) {
	for _, roll := range []int{50, 51} {
		t.Run(fmt.Sprint(roll), func(t *testing.T) {
			tc := advancedCraftCase{name: "spirit", artisan: "Ehre", items: []uint16{661, 662, 633}, roll: roll, level: 39}
			w, p, fallback, backend, queue, req, rng, release := criticalAdvancedFixture(t, tc)
			req.Items[2].Eff = [6]byte{43, 9}
			p.Char.Inv[int(req.Pos[2])] = req.Items[2]
			before := accountStateSnapshot(p.Account)
			if err := w.queueAdvancedComposition(p, req, "Ehre"); err != nil {
				t.Fatal(err)
			}
			proposal := awaitCriticalWrite(t, backend.entered)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
				t.Fatal("proposal mutated live state")
			}
			if err := w.queueAdvancedComposition(p, req, "Ehre"); err == nil {
				t.Fatal("pending duplicate admitted")
			}
			want := accountStateSnapshot(before)
			for i := 0; i < 3; i++ {
				want.Chars[p.CharSlot].Inv[int(req.Pos[i])] = model.Item{}
			}
			if roll == 50 {
				item := req.Items[2]
				item.Eff = [6]byte{43, 7, 70, 2, 69, 2}
				want.Chars[p.CharSlot].Inv[int(req.Pos[2])] = item
			}
			if !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) {
				t.Fatal("wrong detached result")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), want) || fallback.saves != 0 || rng.calls != 1 {
				t.Fatal("commit did not apply exactly once")
			}
		})
	}
}
