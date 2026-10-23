package store

import (
	"encoding/json"
	"testing"

	"wydgo/internal/model"
)

func TestSnapshotAccountPreservesPausedFairy(t *testing.T) {
	item := model.Item{Index: 3914, UID: "11111111111141118111111111111111", ActivatedUnix: 100, ExpiresUnix: 300, PausedUnix: 200}
	account := postgresTestAccount("fairy", "Fairy", item)
	snapshot, err := snapshotAccount(account)
	if err != nil {
		t.Fatal(err)
	}
	var restored model.Account
	if err := json.Unmarshal(snapshot.payload, &restored); err != nil {
		t.Fatal(err)
	}
	if restored.Chars[0].Inv[0] != item || len(snapshot.items) != 1 || snapshot.items[0].uid != item.UID {
		t.Fatal("durable snapshot lost the fairy UID or frozen lifetime")
	}
}
