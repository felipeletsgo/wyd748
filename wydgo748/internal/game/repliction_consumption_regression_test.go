package game

import (
	"encoding/binary"
	"fmt"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Exercise the installed rules, not a synthetic Consume=true override. The
// empty source slot must be published after the account save succeeds.
func TestReplictionRealCatalogConsumesAndPublishesSource(t *testing.T) {
	catalog, err := data.LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/ItemEffect.h", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := data.LoadVolatilesWithInstances("../../data/volatiles.json", "../../data/instances.json", catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	volatiles.Repliction, err = data.LoadRepliction("../../data/repliction.json", catalog.Items)
	if err != nil {
		t.Fatal(err)
	}
	for id := uint16(4016); id <= 4025; id++ {
		for _, amount := range []byte{0, 1, 2} {
			t.Run(fmt.Sprintf("item-%d/amount-%d", id, amount), func(t *testing.T) {
				w, p, st := useItemWorld(model.VolatileRule{})
				w.items, w.volatiles = catalog.Items, volatiles
				var target model.Item
				for itemID, def := range catalog.Items {
					candidate := model.Item{Index: itemID, Eff: [6]byte{43, 0, 60, 1, 71, 1}}
					if def.Pos == 4 && itemAbility(candidate, def, "EF_ITEMLEVEL") == volatiles.Repliction.Items[id].ItemLevel && itemAbility(candidate, def, "EF_MOBTYPE") == 0 {
						target = candidate
						break
					}
				}
				if target.Index == 0 {
					t.Fatal("missing defensive equipment fixture")
				}
				p.Char.Equip[2] = target
				p.Char.Inv[0] = model.Item{}
				const slot = 37 // Also exercise rows beyond the first modern bag.
				p.Char.Inv[slot] = model.Item{Index: id}
				if amount > 0 {
					p.Char.Inv[slot].Eff = [6]byte{61, amount}
				}
				packet := useItemPacket(slot, 2)
				binary.LittleEndian.PutUint32(packet[20:24], placeEquip)
				w.onUseItem(p.Session, packet)
				want := model.Item{}
				if amount > 1 {
					want = model.Item{Index: id, Eff: [6]byte{61, amount - 1}}
				}
				if p.Char.Inv[slot] != want || p.Char.Equip[2] == target || st.saves != 1 {
					t.Fatalf("use did not commit: source=%+v target=%+v saves=%d", p.Char.Inv[slot], p.Char.Equip[2], st.saves)
				}
				found := false
				for {
					out, ok := p.Session.DequeuePacketForTest()
					if !ok {
						break
					}
					if !wire.Decrypt(out) {
						t.Fatal("invalid outbound checksum")
					}
					if wire.ParseHeader(out).Type != wire.OpSendItem || binary.LittleEndian.Uint16(out[12:14]) != placeInv || binary.LittleEndian.Uint16(out[14:16]) != slot {
						continue
					}
					found = true
					if len(out) != 24 || binary.LittleEndian.Uint16(out[16:18]) != want.Index || string(out[18:24]) != string(want.Eff[:]) {
						t.Fatalf("wrong authoritative source frame: % X", out)
					}
				}
				if !found {
					t.Fatal("missing source slot confirmation")
				}
				if amount <= 1 {
					before := p.Char.Equip[2]
					w.onUseItem(p.Session, packet)
					if st.saves != 1 || p.Char.Equip[2] != before {
						t.Fatal("replayed use changed equipment after consumption")
					}
				}
			})
		}
	}
}
