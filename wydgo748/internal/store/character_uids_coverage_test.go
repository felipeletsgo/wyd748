package store

import (
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestPrepareAccountCharacterUIDsNormalizesAndGenerates(t *testing.T) {
	account := &model.Account{Name: "first", Chars: []model.Char{
		{Name: "Arch", UID: "00112233445566778899AABBCCDDEEFF", ArchMortalUID: "FFEEDDCCBBAA99887766554433221100"},
		{Name: "New"},
		{Name: "", UID: "invalid UID is ignored for an empty slot"},
	}}
	changed, err := prepareAccountCharacterUIDs(account)
	if err != nil || changed != 3 {
		t.Fatalf("changed=%d err=%v, want three canonical identities", changed, err)
	}
	if account.Chars[0].UID != "00112233445566778899aabbccddeeff" || account.Chars[0].ArchMortalUID != "ffeeddccbbaa99887766554433221100" {
		t.Fatalf("character identities were not normalized: %+v", account.Chars[0])
	}
	if account.Chars[1].UID == "" || account.Chars[2].UID != "invalid UID is ignored for an empty slot" {
		t.Fatalf("missing/empty-slot identity handling: %+v", account.Chars)
	}
	if changed, err := prepareAccountCharacterUIDs(account); err != nil || changed != 0 {
		t.Fatalf("canonical identities changed again: changed=%d err=%v", changed, err)
	}
}

func TestPrepareAccountCharacterUIDsRejectsInvalidAndDuplicateUIDs(t *testing.T) {
	valid := "00112233445566778899aabbccddeeff"
	for _, tc := range []struct {
		name     string
		accounts []*model.Account
	}{
		{name: "nil account", accounts: []*model.Account{nil}},
		{name: "invalid character UID", accounts: []*model.Account{{Name: "first", Chars: []model.Char{{Name: "Mortal", UID: "bad"}}}}},
		{name: "invalid Arch origin", accounts: []*model.Account{{Name: "first", Chars: []model.Char{{Name: "Arch", UID: valid, ArchMortalUID: "bad"}}}}},
		{name: "duplicate across accounts", accounts: []*model.Account{
			{Name: "first", Chars: []model.Char{{Name: "Mortal", UID: valid}}},
			{Name: "second", Chars: []model.Char{{Name: "Arch", UID: valid}}},
		}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			changed, err := prepareAccountCharacterUIDs(tc.accounts...)
			if err == nil || changed != 0 {
				t.Fatalf("changed=%d err=%v, want rejection without a committed change count", changed, err)
			}
			if !strings.Contains(err.Error(), "store:") {
				t.Fatalf("unexpected validation error: %v", err)
			}
		})
	}
}
