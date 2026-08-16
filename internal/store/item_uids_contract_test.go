package store

import (
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestAccountItemRefsCoversNilStableCharacterAndLegacyLocations(t *testing.T) {
	if refs := accountItemRefs(nil); refs != nil {
		t.Fatalf("nil account refs=%v", refs)
	}
	acc := &model.Account{Name: "acc"}
	acc.Chars[0].Name = "Legacy"
	acc.Chars[0].Equip[1] = model.Item{Index: 10}
	acc.Chars[1].Name = "Stable"
	acc.Chars[1].UID = "11111111111141118111111111111111"
	acc.Chars[1].Inv[2] = model.Item{Index: 11}
	acc.Cargo[3] = model.Item{Index: 12}
	refs := accountItemRefs(acc)
	want := len(acc.Chars)*(model.MaxEquipSlots+model.MaxCarry) + len(acc.Cargo)
	if len(refs) != want {
		t.Fatalf("refs=%d, esperado=%d", len(refs), want)
	}
	foundLegacy, foundStable, foundCargo := false, false, false
	for _, ref := range refs {
		switch ref.location {
		case "char-slot[0]/equip[1]":
			foundLegacy = ref.item == &acc.Chars[0].Equip[1]
		case "char[11111111111141118111111111111111]/inv[2]":
			foundStable = ref.item == &acc.Chars[1].Inv[2]
		case "cargo[3]":
			foundCargo = ref.item == &acc.Cargo[3]
		}
	}
	if !foundLegacy || !foundStable || !foundCargo {
		t.Fatalf("locations incompletas legacy=%v stable=%v cargo=%v", foundLegacy, foundStable, foundCargo)
	}
}

func TestCloneItemOwnersProducesIndependentMap(t *testing.T) {
	src := map[string]itemUIDOwner{"uid": {Account: "a", Location: "x"}}
	dst := cloneItemOwners(src)
	dst["uid"] = itemUIDOwner{Account: "b", Location: "y"}
	if src["uid"].Account != "a" || src["uid"].Location != "x" {
		t.Fatal("cloneItemOwners compartilhou estado")
	}
}

func TestPrepareAccountItemUIDsGeneratesCanonicalizesAndReplacesOwners(t *testing.T) {
	canonical := "aaaaaaaaaaaa4aaa8aaaaaaaaaaaaaaa"
	upper := strings.ToUpper(canonical)
	store := &JSONStore{itemOwners: map[string]itemUIDOwner{
		"bbbbbbbbbbbb4bbb8bbbbbbbbbbbbbbb": {Account: "acc", Location: "old"},
		"cccccccccccc4ccc8ccccccccccccccc": {Account: "other", Location: "cargo[0]"},
	}}
	acc := &model.Account{Name: "Acc"}
	acc.Chars[0].Name = "Char"
	acc.Chars[0].UID = "11111111111141118111111111111111"
	acc.Chars[0].Inv[0] = model.Item{Index: 4011}
	acc.Chars[0].Inv[1] = model.Item{Index: 4012, UID: upper}

	next, changed, err := store.prepareAccountItemUIDs(acc)
	if err != nil {
		t.Fatal(err)
	}
	if changed != 2 {
		t.Fatalf("changed=%d, esperado 2", changed)
	}
	generated := acc.Chars[0].Inv[0].UID
	if generated == "" || generated == canonical {
		t.Fatalf("UID novo invalido/colidiu: %q", generated)
	}
	if acc.Chars[0].Inv[1].UID != canonical {
		t.Fatalf("UID nao foi canonicalizado: %q", acc.Chars[0].Inv[1].UID)
	}
	if _, stale := next["bbbbbbbbbbbb4bbb8bbbbbbbbbbbbbbb"]; stale {
		t.Fatal("owner antigo da conta substituida permaneceu")
	}
	if owner := next["cccccccccccc4ccc8ccccccccccccccc"]; owner.Account != "other" {
		t.Fatal("owner de outra conta foi removido")
	}
	if next[generated].Account != "acc" || next[canonical].Account != "acc" {
		t.Fatalf("owners novos incorretos: %+v", next)
	}

	store.itemOwners = next
	next2, changed2, err := store.prepareAccountItemUIDs(acc)
	if err != nil || changed2 != 0 || len(next2) != len(next) {
		t.Fatalf("segunda preparacao deveria ser idempotente: changed=%d len=%d/%d err=%v", changed2, len(next2), len(next), err)
	}
}

func TestPrepareAccountItemUIDsRejectsInvalidAccountsEmptyMetadataUIDAndDuplicates(t *testing.T) {
	store := &JSONStore{itemOwners: make(map[string]itemUIDOwner)}
	if _, _, err := store.prepareAccountItemUIDs((*model.Account)(nil)); err == nil {
		t.Fatal("conta nil foi aceita")
	}
	if _, _, err := store.prepareAccountItemUIDs(&model.Account{}); err == nil {
		t.Fatal("conta sem nome foi aceita")
	}

	emptyMetadata := &model.Account{Name: "empty"}
	emptyMetadata.Cargo[0] = model.Item{UID: "11111111111141118111111111111111"}
	if _, _, err := store.prepareAccountItemUIDs(emptyMetadata); err == nil {
		t.Fatal("slot vazio com UID foi aceito")
	}
	emptyDeadline := &model.Account{Name: "deadline"}
	emptyDeadline.Cargo[0] = model.Item{ExpiresUnix: 1}
	if _, _, err := store.prepareAccountItemUIDs(emptyDeadline); err == nil {
		t.Fatal("slot vazio com prazo foi aceito")
	}

	invalid := &model.Account{Name: "invalid"}
	invalid.Cargo[0] = model.Item{Index: 1, UID: "bad"}
	if _, _, err := store.prepareAccountItemUIDs(invalid); err == nil {
		t.Fatal("item UID invalido foi aceito")
	}

	dupUID := "22222222222242228222222222222222"
	dup := &model.Account{Name: "dup"}
	dup.Cargo[0] = model.Item{Index: 1, UID: dupUID}
	dup.Cargo[1] = model.Item{Index: 2, UID: dupUID}
	if _, _, err := store.prepareAccountItemUIDs(dup); err == nil || !strings.Contains(err.Error(), "UID duplicado") {
		t.Fatalf("duplicidade intra-conta nao foi recusada: %v", err)
	}

	store.itemOwners[dupUID] = itemUIDOwner{Account: "other", Location: "cargo[9]"}
	foreign := &model.Account{Name: "new"}
	foreign.Cargo[0] = model.Item{Index: 3, UID: dupUID}
	if _, _, err := store.prepareAccountItemUIDs(foreign); err == nil || !strings.Contains(err.Error(), "UID duplicado") {
		t.Fatalf("duplicidade inter-conta nao foi recusada: %v", err)
	}
}

func TestGeneratedUIDsRollbackWhenLaterItemValidationFails(t *testing.T) {
	store := &JSONStore{itemOwners: make(map[string]itemUIDOwner)}
	acc := &model.Account{Name: "rollback"}
	acc.Cargo[0] = model.Item{Index: 1}
	acc.Cargo[1] = model.Item{Index: 2, UID: "bad"}
	if _, _, err := store.prepareAccountItemUIDs(acc); err == nil {
		t.Fatal("segunda entrada invalida deveria falhar")
	}
	if acc.Cargo[0].UID != "" {
		t.Fatalf("UID gerado nao foi revertido: %q", acc.Cargo[0].UID)
	}
}
