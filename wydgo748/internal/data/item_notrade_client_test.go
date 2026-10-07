package data

import (
	"encoding/binary"
	"os"
	"path/filepath"
	"testing"
)

// The client 7.48 item list marks untradeable items with EF_NOTRADE and hides
// the trade action for them; the server is authoritative for trades, so every
// such item must carry the same flag in itemlist.csv (validateTradableItem).
func TestItemListNoTradeMatchesClient748(t *testing.T) {
	dataRoot := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(dataRoot, "itemlist.csv"),
		filepath.Join(dataRoot, "Itemname.csv"), filepath.Join(dataRoot, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	noTradeID := -1
	for id, name := range catalog.ItemEffects {
		if name == "EF_NOTRADE" {
			noTradeID = int(id)
		}
	}
	if noTradeID < 0 {
		t.Fatal("ItemEffect.h does not define EF_NOTRADE")
	}
	encoded, err := os.ReadFile(filepath.Join("..", "..", "..", "tmproject", "client748", "ItemList.bin"))
	if err != nil {
		t.Fatal(err)
	}
	const itemCount, recordSize = 6500, 140
	if len(encoded) != itemCount*recordSize+4 {
		t.Fatalf("client item list has %d bytes; want %d", len(encoded), itemCount*recordSize+4)
	}
	checked := 0
	for index := 0; index < itemCount; index++ {
		var row [recordSize]byte
		for i := range row {
			row[i] = encoded[index*recordSize+i] ^ 0x5a
		}
		clientValue := 0
		for slot := 0; slot < 12; slot++ {
			offset := 80 + slot*4
			if int(binary.LittleEndian.Uint16(row[offset:offset+2])) == noTradeID {
				clientValue = int(int16(binary.LittleEndian.Uint16(row[offset+2 : offset+4])))
			}
		}
		if clientValue == 0 {
			continue
		}
		checked++
		definition, ok := catalog.Items[uint16(index)]
		if !ok {
			t.Errorf("item %d is untradeable in the client but missing from itemlist.csv", index)
			continue
		}
		serverValue := 0
		for _, effect := range definition.StaticEffects {
			if effect.Name == "EF_NOTRADE" {
				serverValue = effect.Value
			}
		}
		if serverValue != clientValue {
			t.Errorf("item %d %s EF_NOTRADE: client %d, server %d", index, definition.Name, clientValue, serverValue)
		}
	}
	if checked == 0 {
		t.Fatal("no EF_NOTRADE item found in the client list")
	}
}
