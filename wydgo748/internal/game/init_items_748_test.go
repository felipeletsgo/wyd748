package game

import (
	"encoding/binary"
	"path/filepath"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

func TestClient748InitItemsSpawnWithReservedCannonIDs(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	objects, err := data.LoadInitItems(filepath.Join(root, "init_items.csv"), catalog.Items)
	if err != nil {
		t.Fatal(err)
	}
	if len(objects) != 96 {
		t.Fatalf("loaded %d objects; want 96", len(objects))
	}
	w, err := worldComObjetos(objects...)
	if err != nil {
		t.Fatal(err)
	}
	if len(w.groundItems) != len(objects) {
		t.Fatalf("spawned %d objects; want %d", len(w.groundItems), len(objects))
	}
	w.items = catalog.Items
	cannons := 0
	groundMasks := 0
	for id, item := range w.groundItems {
		if !item.Permanent {
			t.Fatalf("object %d is not permanent", id)
		}
		inCannonRange := id >= 15001 && id <= 15100
		if item.Item.Index == 746 {
			cannons++
			if !inCannonRange {
				t.Fatalf("cannon %d has ID %d outside the reserved range", item.Item.Index, id)
			}
		} else if inCannonRange {
			t.Fatalf("non-cannon item %d has reserved ID %d", item.Item.Index, id)
		}
		pkt := w.groundItemCreatePacket(item)
		if len(pkt) != 32 || pkt[26] != item.Rotate {
			t.Fatalf("object %d has invalid CreateItem size or rotation", id)
		}
		def := catalog.Items[item.Item.Index]
		if staticEffect(def, "EF_GROUND") > 0 {
			groundMasks++
			wantState := clientGateClosed
			if staticEffect(def, "EF_KEYID") > 0 {
				wantState = clientGateLocked
			}
			if pkt[27] != wantState || pkt[28] != clientGateHeight {
				t.Fatalf("ground object %d has state/height %d/%d; want %d/%d",
					id, pkt[27], pkt[28], wantState, clientGateHeight)
			}
		} else if pkt[27] != 0 || pkt[28] != 0 {
			t.Fatalf("non-ground object %d has state/height %d/%d; want 0/0", id, pkt[27], pkt[28])
		}
	}
	if cannons != 25 {
		t.Fatalf("spawned %d cannons; want 25", cannons)
	}
	if groundMasks == 0 {
		t.Fatal("no ground-mask objects found in the 96-object catalog")
	}

	// Each CSV fixture must reach the player through the actual visibility
	// path. The client no longer loads a separate local InitItem table.
	session := net.NewTestSession(1, len(objects)+1)
	player := &Player{ID: 1, Session: session, InWorld: true}
	var last *GroundItem
	for _, item := range w.groundItems {
		last = item
		player.X, player.Y = item.X, item.Y
		player.Visible = nil
		w.refreshPlayerVisibility(player)
		found := false
		for session.QueuedPacketsForTest() > 0 {
			pkt, ok := session.DequeuePacketForTest()
			if !ok || !wire.Decrypt(pkt) {
				t.Fatal("visibility sent an undecodable packet")
			}
			if len(pkt) != 32 || wire.ParseHeader(pkt).Type != wire.OpCreateItem {
				t.Fatalf("visibility sent an unexpected packet for object %d", item.ID)
			}
			if binary.LittleEndian.Uint16(pkt[16:18]) != item.ID {
				continue
			}
			found = true
			if binary.LittleEndian.Uint16(pkt[12:14]) != item.X ||
				binary.LittleEndian.Uint16(pkt[14:16]) != item.Y ||
				binary.LittleEndian.Uint16(pkt[18:20]) != item.Item.Index ||
				pkt[26] != item.Rotate {
				t.Fatalf("visibility changed CSV object %d in CreateItem", item.ID)
			}
		}
		if !found || !player.hasVisible(item.ID) {
			t.Fatalf("CSV object %d was not delivered on entering view", item.ID)
		}
	}
	w.refreshPlayerVisibility(player)
	if session.QueuedPacketsForTest() != 0 {
		t.Fatal("stationary visibility resent permanent objects")
	}
	player.X, player.Y = 0, 0
	w.refreshPlayerVisibility(player)
	removed := false
	for session.QueuedPacketsForTest() > 0 {
		pkt, ok := session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpRemoveItem {
			t.Fatal("leaving view did not send a decodable RemoveItem packet")
		}
		if binary.LittleEndian.Uint32(pkt[12:16]) == uint32(last.ID) {
			removed = true
		}
	}
	if !removed || player.hasVisible(last.ID) {
		t.Fatalf("CSV object %d remained visible after leaving its area", last.ID)
	}
	player.X, player.Y = last.X, last.Y
	w.refreshPlayerVisibility(player)
	returned := false
	for session.QueuedPacketsForTest() > 0 {
		pkt, ok := session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpCreateItem {
			t.Fatal("returning to view did not send a decodable CreateItem packet")
		}
		if binary.LittleEndian.Uint16(pkt[16:18]) == last.ID {
			returned = true
		}
	}
	if !returned || !player.hasVisible(last.ID) {
		t.Fatalf("CSV object %d was not restored on returning to its area", last.ID)
	}
}
