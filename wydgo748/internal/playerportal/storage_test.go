package playerportal

import (
	"context"
	"encoding/json"
	"errors"
	"os"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
)

func (b *memoryBackend) Inventory(_ context.Context, name string, slot int) (StorageSnapshot, error) {
	b.mu.Lock()
	defer b.mu.Unlock()
	b.profileName = name
	if b.fail {
		return StorageSnapshot{}, errors.New("private database failure")
	}
	a := b.accounts[strings.ToLower(name)]
	if a == nil || slot < 0 || slot > 3 || slot >= len(a.Chars) || a.Chars[slot].Score == nil || a.Chars[slot].Name == "" {
		return StorageSnapshot{}, os.ErrNotExist
	}
	result := storageFixture(a.Chars[slot].Inv[:model.PlayerCarrySlots])
	result.CharacterSlot = &slot
	return result, nil
}

func (b *memoryBackend) Cargo(_ context.Context, name string) (StorageSnapshot, error) {
	b.mu.Lock()
	defer b.mu.Unlock()
	b.profileName = name
	if b.fail {
		return StorageSnapshot{}, errors.New("private database failure")
	}
	a := b.accounts[strings.ToLower(name)]
	if a == nil {
		return StorageSnapshot{}, os.ErrNotExist
	}
	result := storageFixture(a.Cargo[:model.PlayerCargoSlots])
	gold := a.CargoGold
	result.Gold = &gold
	return result, nil
}

func storageFixture(items []model.Item) StorageSnapshot {
	result := StorageSnapshot{Capacity: len(items), Items: []StoredItem{}, AsOf: time.Date(2026, 10, 13, 0, 0, 0, 0, time.UTC)}
	for slot, item := range items {
		if item.Index != 0 {
			result.Items = append(result.Items, StoredItem{Slot: slot, ItemID: item.Index, Effects: item.Eff})
		}
	}
	return result
}

func TestStorageOwnershipPrivacyAndReadOnly(t *testing.T) {
	h, b := testHandler(t)
	a := &model.Account{Name: "Player1", PasswordHash: "password-secret", CargoGold: 4000000000,
		Chars: []model.Char{{Name: "Knight", UID: "character-secret", Score: &model.Score{}}}}
	a.Chars[0].Inv[0] = model.Item{Index: 412, UID: "inventory-secret", Eff: [6]byte{61, 255}, ExpiresUnix: 999999}
	a.Chars[0].Inv[62] = model.Item{Index: 2330, Eff: [6]byte{61, 200}}
	a.Chars[0].Inv[63] = model.Item{Index: 999}
	a.Cargo[119] = model.Item{Index: 413, UID: "cargo-secret", Eff: [6]byte{43, 9, 61, 10}}
	a.Cargo[120] = model.Item{Index: 999}
	b.accounts["player1"] = a
	h.cfg.ItemNames = map[uint16]string{412: "Stack_Item"}
	before, _ := json.Marshal(a)
	cookie, _ := authenticatedSession(t, h, "Player1")
	outsider, _ := authenticatedSession(t, h, "OtherPlayer")
	for _, path := range []string{"/characters/0/inventory", "/cargo"} {
		url := "/api/player/v1" + path
		if w := request(h, "GET", url, "", nil, ""); w.Code != 401 {
			t.Fatal("anonymous storage read accepted")
		}
		if w := request(h, "GET", url+"?username=Player1", "", outsider, ""); w.Code != 404 {
			t.Fatal("another account selected the owner")
		}
		w := request(h, "GET", url+"?username=OtherPlayer", "", cookie, "")
		var body struct {
			Version int
			Source  string
			Storage StorageSnapshot
		}
		if err := json.Unmarshal(w.Body.Bytes(), &body); err != nil {
			t.Fatal(err)
		}
		if w.Code != 200 || b.profileName != "Player1" || body.Version != 1 || body.Source != "persisted snapshot" || body.Storage.AsOf.IsZero() || w.Header().Get("Cache-Control") != "no-store" {
			t.Fatalf("invalid storage response: %d %s", w.Code, w.Body.String())
		}
		for _, private := range []string{"secret", "uid", "password", "expiresUnix", "activatedUnix", "999"} {
			if strings.Contains(w.Body.String(), private) {
				t.Fatalf("private or reserved field leaked: %s", private)
			}
		}
		if path == "/cargo" {
			if body.Storage.Capacity != 120 || body.Storage.CharacterSlot != nil || body.Storage.Gold == nil || *body.Storage.Gold != 4000000000 || len(body.Storage.Items) != 1 || body.Storage.Items[0].Quantity != 10 || body.Storage.Items[0].Slot != 119 {
				t.Fatal("invalid Cargo projection")
			}
		} else if body.Storage.Capacity != 63 || body.Storage.Gold != nil || body.Storage.CharacterSlot == nil || *body.Storage.CharacterSlot != 0 || len(body.Storage.Items) != 2 || body.Storage.Items[0].Quantity != 255 || body.Storage.Items[0].Name != "Stack Item" || body.Storage.Items[1].Quantity != 1 {
			t.Fatal("invalid inventory projection")
		}
		if w := request(h, "POST", url, `{}`, cookie, "bad"); w.Code == 200 {
			t.Fatal("storage mutation accepted")
		}
		b.fail = true
		w = request(h, "GET", url, "", cookie, "")
		if w.Code != 503 || strings.Contains(w.Body.String(), "database") {
			t.Fatal("storage failure leaked or claimed success")
		}
		b.fail = false
	}
	for _, slot := range []string{"4", "-1", "00", "one", "0%20OR%201=1"} {
		if w := request(h, "GET", "/api/player/v1/characters/"+slot+"/inventory", "", cookie, ""); w.Code != 400 {
			t.Fatalf("malformed inventory selector accepted: %s", slot)
		}
	}
	if w := request(h, "GET", "/api/player/v1/characters/3/inventory", "", cookie, ""); w.Code != 404 {
		t.Fatal("empty character selected")
	}
	after, _ := json.Marshal(a)
	if string(before) != string(after) {
		t.Fatal("read-only storage changed the account")
	}
}

func TestStoredQuantityUsesInstanceEffectsNotMountHP(t *testing.T) {
	for _, id := range []uint16{2329, 2330, 2389, 2390, 412} {
		for pair := 0; pair < 3; pair++ {
			for value := 0; value <= 255; value++ {
				item := StoredItem{ItemID: id}
				item.Effects[pair*2], item.Effects[pair*2+1] = 61, byte(value)
				want := uint16(max(1, value))
				if id >= 2330 && id <= 2389 {
					want = 1
				}
				if got := storedQuantity(item); got != want {
					t.Fatalf("id=%d pair=%d value=%d: got %d want %d", id, pair, value, got, want)
				}
			}
		}
	}
	if storedQuantity(StoredItem{ItemID: 412}) != 1 {
		t.Fatal("unstacked item is not one unit")
	}
}
