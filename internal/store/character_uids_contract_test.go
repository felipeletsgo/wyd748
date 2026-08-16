package store

import (
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestPrepareAccountCharacterUIDsCoversGenerationCanonicalizationAndArchOrigin(t *testing.T) {
	var nilAccount *model.Account
	if _, err := prepareAccountCharacterUIDs(nilAccount); err == nil {
		t.Fatal("conta nil foi aceita")
	}

	account := &model.Account{Name: "acc"}
	account.Chars[0] = model.Char{Name: "Generated"}
	account.Chars[1] = model.Char{
		Name: "Canonical",
		UID:  "AAAAAAAAAAAA4AAA8AAAAAAAAAAAAAAA",
		ArchMortalUID: "BBBBBBBBBBBB4BBB8BBBBBBBBBBBBBBB",
	}
	changed, err := prepareAccountCharacterUIDs(account)
	if err != nil {
		t.Fatal(err)
	}
	if changed != 3 {
		t.Fatalf("changed=%d, esperado 3", changed)
	}
	for slot := 0; slot < 2; slot++ {
		uid, err := model.NormalizeCharacterUID(account.Chars[slot].UID)
		if err != nil || uid == "" || uid != account.Chars[slot].UID {
			t.Fatalf("slot %d UID nao canonico: %q err=%v", slot, account.Chars[slot].UID, err)
		}
	}
	if account.Chars[1].UID != strings.ToLower("AAAAAAAAAAAA4AAA8AAAAAAAAAAAAAAA") ||
		account.Chars[1].ArchMortalUID != strings.ToLower("BBBBBBBBBBBB4BBB8BBBBBBBBBBBBBBB") {
		t.Fatalf("canonicalizacao divergente: %+v", account.Chars[1])
	}

	changed, err = prepareAccountCharacterUIDs(account)
	if err != nil || changed != 0 {
		t.Fatalf("segunda preparacao deveria ser idempotente: changed=%d err=%v", changed, err)
	}
}

func TestPrepareAccountCharacterUIDsRejectsInvalidAndDuplicateIdentities(t *testing.T) {
	invalid := &model.Account{Name: "bad"}
	invalid.Chars[0] = model.Char{Name: "Bad", UID: "not-a-uid"}
	if _, err := prepareAccountCharacterUIDs(invalid); err == nil {
		t.Fatal("UID invalido foi aceito")
	}

	invalidArch := &model.Account{Name: "arch"}
	invalidArch.Chars[0] = model.Char{
		Name: "Arch", UID: "11111111111141118111111111111111", ArchMortalUID: "bad",
	}
	if _, err := prepareAccountCharacterUIDs(invalidArch); err == nil {
		t.Fatal("ArchMortalUID invalido foi aceito")
	}

	dup := "22222222222242228222222222222222"
	a := &model.Account{Name: "a"}
	b := &model.Account{Name: "b"}
	a.Chars[0] = model.Char{Name: "One", UID: dup}
	b.Chars[3] = model.Char{Name: "Two", UID: dup}
	if _, err := prepareAccountCharacterUIDs(a, b); err == nil || !strings.Contains(err.Error(), "duplicado") {
		t.Fatalf("UID duplicado nao foi recusado corretamente: %v", err)
	}
}

func TestPrepareAccountCharacterUIDsSkipsEmptySlots(t *testing.T) {
	account := &model.Account{Name: "empty"}
	account.Chars[0].UID = "this stale field is ignored with empty name"
	changed, err := prepareAccountCharacterUIDs(account)
	if err != nil || changed != 0 {
		t.Fatalf("slot vazio deveria ser ignorado: changed=%d err=%v", changed, err)
	}
}
