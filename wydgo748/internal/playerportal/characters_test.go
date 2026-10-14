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

func (b *memoryBackend) CharacterDetail(_ context.Context, name string, slot int) (CharacterDetail, error) {
	b.mu.Lock()
	defer b.mu.Unlock()
	b.profileName = name
	if b.fail {
		return CharacterDetail{}, errors.New("private database details")
	}
	a := b.accounts[strings.ToLower(name)]
	if a == nil || slot < 0 || slot >= len(a.Chars) || a.Chars[slot].Score == nil {
		return CharacterDetail{}, os.ErrNotExist
	}
	c := a.Chars[slot]
	equipment := make([]EquipmentItem, 0)
	for i := 1; i < len(c.Equip); i++ {
		if c.Equip[i].Index != 0 {
			equipment = append(equipment, EquipmentItem{Slot: i, ItemID: c.Equip[i].Index, Effects: c.Equip[i].Eff})
		}
	}
	detail := CharacterDetail{Character: Character{Slot: slot, Name: c.Name, Class: c.Class, Level: c.Score.Level, Evolution: c.Evolution},
		Exp: c.Exp, Gold: c.Gold, CP: c.CP, AsOf: time.Date(2026, 10, 13, 0, 0, 0, 0, time.UTC), Equipment: equipment}
	free := false
	for slot := 0; slot < model.ArchCharacterSlots; slot++ {
		free = free || slot >= len(a.Chars) || a.Chars[slot].Name == ""
	}
	detail.Ascension = archAdvice(detail, c.Evolution, free)
	ideal, space := model.SavedIdealInventoryRequirements(c.Inv[:])
	detail.CelestialAscension = celestialAdvice(c.Evolution, c.Score.Level, c.ArchMortalLevel, ideal)
	var fame uint32
	if state := b.charStates[c.UID]; state != nil {
		if state.Version != model.CharStateVersion {
			return CharacterDetail{}, errors.New("invalid saved state")
		}
		fame = state.SpecialCoins["fame"]
	}
	detail.Fame = fame
	detail.SubCelestialAscension = subCelestialAdvice(detail, c.Evolution, fame, ideal, c.AlternateCelestial == nil, int(c.Equip[0].Index) >= int(c.Class), space)
	return detail, nil
}

func TestSavedCelestialAdviceUsesOriginLevelAndUsableInventory(t *testing.T) {
	h, b := testHandler(t)
	c := model.Char{Name: "ArchOne", Evolution: "arch", Score: &model.Score{Level: 355}, ArchMortalLevel: 399}
	c.Inv[62] = model.Item{Index: 5338, UID: "private-ideal-identity"}
	b.accounts["player1"] = &model.Account{Chars: []model.Char{c}}
	cookie, _ := authenticatedSession(t, h, "Player1")
	read := func() CelestialAscension {
		t.Helper()
		w := request(h, "GET", "/api/player/v1/characters/0?idealStone=true&mortalLevel=400&username=OtherPlayer", "", cookie, "")
		var body struct{ Character CharacterDetail }
		if w.Code != 200 || json.Unmarshal(w.Body.Bytes(), &body) != nil {
			t.Fatalf("advice read failed: %d %s", w.Code, w.Body.String())
		}
		for _, private := range []string{"archMortalLevel", "archMortalUid", "private-ideal-identity", "inv\""} {
			if strings.Contains(w.Body.String(), private) {
				t.Fatalf("Celestial advice leaked %s", private)
			}
		}
		return body.Character.CelestialAscension
	}
	if r := read(); !r.Applicable || !r.RequirementsMet || r.RequiredLevel != 356 || r.RequiredMortalLevel != 400 || len(r.Checks) != 4 {
		t.Fatalf("valid Arch rejected: %+v", r)
	}
	ch := &b.accounts["player1"].Chars[0]
	ch.Inv[62], ch.Inv[63] = model.Item{}, ch.Inv[62]
	if r := read(); r.RequirementsMet || r.Checks[3].Met {
		t.Fatal("reserved inventory slot counted")
	}
	ch.Equip[10], ch.Inv[63] = ch.Inv[63], model.Item{}
	if read().RequirementsMet {
		t.Fatal("equipped Ideal Stone counted as usable inventory")
	}
	ch.Inv[0], ch.Equip[10] = ch.Equip[10], model.Item{}
	ch.ArchMortalLevel = 398
	if read().RequirementsMet {
		t.Fatal("Mortal below displayed level 400 accepted")
	}
	ch.ArchMortalLevel = 399
	ch.Score.Level = 354
	if read().RequirementsMet {
		t.Fatal("Arch below displayed level 356 accepted")
	}
	ch.Score.Level = 355
	for _, evolution := range []string{"", "mortal", "celestial", "subcelestial", "unknown"} {
		ch.Evolution = evolution
		if r := read(); r.Applicable || r.RequirementsMet {
			t.Fatalf("inapplicable evolution accepted: %q", evolution)
		}
	}
}

func TestDisplayedFameIsOwnedSavedCharacterCounter(t *testing.T) {
	h, b := testHandler(t)
	b.accounts["player1"] = &model.Account{Donate: 9000, Chars: []model.Char{
		{UID: "first-private-uid", Name: "First", CP: 75, Score: &model.Score{}},
		{UID: "second-private-uid", Name: "Second", CP: -75, Score: &model.Score{}},
	}}
	b.charStates = map[string]*model.CharState{
		"first-private-uid":  {Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 123456, "private-counter": 999}},
		"second-private-uid": {Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": ^uint32(0)}},
	}
	cookie, _ := authenticatedSession(t, h, "Player1")
	for slot, want := range []uint32{123456, ^uint32(0)} {
		path := "/api/player/v1/characters/" + string(rune('0'+slot)) + "?fame=9000&username=OtherPlayer"
		w := request(h, "GET", path, "", cookie, "")
		var body struct{ Character CharacterDetail }
		if w.Code != 200 || json.Unmarshal(w.Body.Bytes(), &body) != nil || body.Character.Fame != want {
			t.Fatalf("owned Fame read: %d %s", w.Code, w.Body.String())
		}
		if w.Header().Get("Cache-Control") != "no-store" || strings.Contains(w.Body.String(), "private-") || strings.Contains(w.Body.String(), "specialCoins") {
			t.Fatal("private saved Fame cached or raw counter leaked")
		}
	}
	delete(b.charStates, "first-private-uid")
	w := request(h, "GET", "/api/player/v1/characters/0?fame=999", "", cookie, "")
	var body struct{ Character CharacterDetail }
	if w.Code != 200 || json.Unmarshal(w.Body.Bytes(), &body) != nil || body.Character.Fame != 0 {
		t.Fatal("missing counter borrowed Fame from another character, CP or balance")
	}
}

func TestSavedArchAdviceUsesEquippedItemsAndAccountSlots(t *testing.T) {
	h, b := testHandler(t)
	character := model.Char{Name: "Mortal", Score: &model.Score{Level: 370}, CP: -75,
		Equip: [16]model.Item{10: {Index: 1742}, 11: {Index: 1763}}}
	b.accounts["player1"] = &model.Account{Chars: []model.Char{character}}
	cookie, _ := authenticatedSession(t, h, "Player1")
	read := func() ArchAscension {
		t.Helper()
		w := request(h, "GET", "/api/player/v1/characters/0?freeSlot=true&level=400&username=OtherPlayer", "", cookie, "")
		var body struct{ Character CharacterDetail }
		if w.Code != 200 || json.Unmarshal(w.Body.Bytes(), &body) != nil {
			t.Fatalf("advice read failed: %d %s", w.Code, w.Body.String())
		}
		for _, secret := range []string{"rawEvolution", "chars", "uid", "password"} {
			if strings.Contains(w.Body.String(), secret) {
				t.Fatalf("advice leaked %s", secret)
			}
		}
		return body.Character.Ascension
	}
	advice := read()
	if !advice.Applicable || !advice.RequirementsMet || advice.RequiredLevel != 371 || len(advice.Checks) != 5 {
		t.Fatalf("valid Mortal rejected (CP/Fame are not Arch gates): %+v", advice)
	}
	for _, gate := range advice.Checks {
		if !gate.Met {
			t.Fatalf("missing gate: %+v", gate)
		}
	}
	b.accounts["player1"].Chars = []model.Char{character, character, character, character}
	if read().RequirementsMet {
		t.Fatal("full account accepted")
	}
	b.accounts["player1"].Chars[3].Name = ""
	if !read().RequirementsMet {
		t.Fatal("empty fourth slot rejected")
	}
	b.accounts["player1"].Chars[0].Equip[10] = model.Item{}
	b.accounts["player1"].Chars[0].Inv[0] = model.Item{Index: 1742}
	if read().RequirementsMet {
		t.Fatal("inventory stone counted as equipped")
	}
	b.accounts["player1"].Chars[0].Equip[10] = model.Item{Index: 1742}
	b.accounts["player1"].Chars[0].Score.Level = 369
	if read().RequirementsMet {
		t.Fatal("displayed level 370 bypassed internal level gate")
	}
	b.accounts["player1"].Chars[0].Score.Level = 370
	for _, evolution := range []string{"arch", "celestial", "unknown", "mortal"} {
		b.accounts["player1"].Chars[0].Evolution = evolution
		if advice := read(); advice.Applicable || advice.RequirementsMet {
			t.Fatalf("nonempty evolution accepted: %q", evolution)
		}
	}
}

func TestSavedSubCelestialAdviceUsesCharacterFameAndRealSavedGates(t *testing.T) {
	h, b := testHandler(t)
	c := model.Char{UID: "private-character-uid", Name: "CelestialOne", Class: 3, Evolution: "celestial", Score: &model.Score{Level: 120}, CP: 75}
	c.Equip[0] = model.Item{Index: 3}
	c.Equip[11] = model.Item{Index: 1761}
	c.Inv[62] = model.Item{Index: 5338, UID: "private-item-uid"}
	b.accounts["player1"] = &model.Account{Chars: []model.Char{c}}
	b.charStates = map[string]*model.CharState{c.UID: {Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 100, "private-currency": 12345}}}
	cookie, _ := authenticatedSession(t, h, "Player1")
	read := func() SubCelestialAscension {
		t.Helper()
		w := request(h, "GET", "/api/player/v1/characters/0?fame=100&noAlternate=true&username=OtherPlayer", "", cookie, "")
		var body struct{ Character CharacterDetail }
		if w.Code != 200 || json.Unmarshal(w.Body.Bytes(), &body) != nil {
			t.Fatalf("read: %d %s", w.Code, w.Body.String())
		}
		for _, secret := range []string{"private-", `"inventory":`, "specialCoins", "alternateCelestial", "validFace"} {
			if strings.Contains(w.Body.String(), secret) {
				t.Fatalf("leaked %s", secret)
			}
		}
		if w.Header().Get("Cache-Control") != "no-store" {
			t.Fatal("advice cached")
		}
		return body.Character.SubCelestialAscension
	}
	if r := read(); !r.RequirementsMet || !r.Applicable || r.RequiredLevel != 121 || r.RequiredFame != 100 || len(r.Checks) != 8 {
		t.Fatalf("valid Celestial: %+v", r)
	}
	ch := &b.accounts["player1"].Chars[0]
	for _, mutation := range []struct{ apply, restore func() }{
		{func() { b.charStates[c.UID].SpecialCoins["fame"] = 99 }, func() { b.charStates[c.UID].SpecialCoins["fame"] = 100 }},
		{func() { ch.Score.Level = 119 }, func() { ch.Score.Level = 120 }},
		{func() { ch.Equip[11] = model.Item{}; ch.Inv[0] = model.Item{Index: 1761} }, func() { ch.Equip[11] = model.Item{Index: 1761}; ch.Inv[0] = model.Item{} }},
		{func() { ch.AlternateCelestial = &model.CelestialForm{} }, func() { ch.AlternateCelestial = nil }},
		{func() { ch.Equip[0] = model.Item{Index: 2} }, func() { ch.Equip[0] = model.Item{Index: 3} }},
		{func() { ch.Inv[63] = ch.Inv[62]; ch.Inv[62] = model.Item{} }, func() { ch.Inv[62] = ch.Inv[63]; ch.Inv[63] = model.Item{} }},
	} {
		mutation.apply()
		if read().RequirementsMet {
			t.Fatal("missing gate passed")
		}
		mutation.restore()
	}
	delete(b.charStates, c.UID)
	b.charStates["other-character"] = &model.CharState{Version: 1, SpecialCoins: map[string]uint32{"fame": 100}}
	if read().RequirementsMet {
		t.Fatal("missing own Fame borrowed from another character or CP")
	}
	ch.Evolution = "subcelestial"
	if r := read(); r.Applicable || r.RequirementsMet {
		t.Fatal("existing Sub offered creation")
	}
}

func TestCharacterDetailOwnershipPrivacyAndRejections(t *testing.T) {
	h, b := testHandler(t)
	b.accounts["player1"] = &model.Account{Chars: []model.Char{{UID: "character-secret", Name: "Knight", Score: &model.Score{Level: 399, Attack: 90000}, Exp: 42, Gold: 123, CP: -15,
		Equip: [16]model.Item{1: {Index: 100, UID: "item-secret", Eff: [6]byte{43, 9}}, 15: {Index: 101}}}}}
	h.cfg.ItemNames = map[uint16]string{100: "Test_Helmet"}
	if w := request(h, "GET", "/api/player/v1/characters/0", "", nil, ""); w.Code != 401 {
		t.Fatal("anonymous equipment read accepted")
	}
	cookie, _ := authenticatedSession(t, h, "Player1")
	for _, slot := range []string{"4", "-1", "00", "one", "2147483648", "0%20OR%201=1"} {
		if w := request(h, "GET", "/api/player/v1/characters/"+slot, "", cookie, ""); w.Code != 400 {
			t.Fatalf("slot %s: %d %s", slot, w.Code, w.Body.String())
		}
	}
	w := request(h, "GET", "/api/player/v1/characters/0?username=OtherPlayer", "", cookie, "")
	var body struct{ Character CharacterDetail }
	if err := json.Unmarshal(w.Body.Bytes(), &body); err != nil {
		t.Fatal(err)
	}
	if w.Code != 200 || b.profileName != "Player1" || body.Character.Gold != 123 || body.Character.CP != -15 || body.Character.Level != 399 || len(body.Character.Equipment) != 2 || body.Character.Equipment[0].Name != "Test Helmet" || body.Character.AsOf.IsZero() {
		t.Fatalf("invalid projection: %d %s", w.Code, w.Body.String())
	}
	if w.Header().Get("Cache-Control") != "no-store" {
		t.Fatal("private snapshot cached")
	}
	for _, private := range []string{"secret", "uid", "attack", "password", "inventory", "cargo"} {
		if strings.Contains(w.Body.String(), private) {
			t.Fatalf("private field leaked: %s", private)
		}
	}
	if w := request(h, "GET", "/api/player/v1/characters/3", "", cookie, ""); w.Code != 404 {
		t.Fatal("missing slot accepted")
	}
	outsider, _ := authenticatedSession(t, h, "OtherPlayer")
	if w := request(h, "GET", "/api/player/v1/characters/0?username=Player1", "", outsider, ""); w.Code != 404 {
		t.Fatal("cross-account selector accepted")
	}
	b.fail = true
	if w := request(h, "GET", "/api/player/v1/characters/0", "", cookie, ""); w.Code != 503 || strings.Contains(w.Body.String(), "database") {
		t.Fatal("dependency failure leaked or claimed success")
	}
	if w := request(h, "POST", "/api/player/v1/characters/0", `{}`, cookie, "bad"); w.Code == 200 {
		t.Fatal("equipment mutation route exists")
	}
}
