package store

import (
	"strings"
	"testing"

	"wydgo/internal/model"
)

func itemUIDTestAccount(name string) *model.Account {
	return &model.Account{Name: name, Chars: []model.Char{{}}}
}

func TestPrepareAccountItemUIDsGeneratesNormalizesAndReplacesOwners(t *testing.T) {
	const existingUID = "00112233445566778899aabbccddeeff"
	account := itemUIDTestAccount("FIRST")
	account.Chars[0].Inv[0] = model.Item{Index: 100}
	account.Cargo[0] = model.Item{Index: 101, UID: strings.ToUpper(existingUID)}
	store := &JSONStore{itemOwners: map[string]itemUIDOwner{
		"old-item":  {Account: "first", Location: "cargo[0]"},
		existingUID: {Account: "first", Location: "old-slot"},
	}}

	next, changed, err := store.prepareAccountItemUIDs(account)
	if err != nil || changed != 2 {
		t.Fatalf("changed=%d err=%v, want generated and normalized items", changed, err)
	}
	generated := account.Chars[0].Inv[0].UID
	if generated == "" || account.Cargo[0].UID != existingUID || len(next) != 2 {
		t.Fatalf("item identity index: generated=%q cargo=%q owners=%v", generated, account.Cargo[0].UID, next)
	}
	if owner, ok := next[generated]; !ok || owner.Account != "first" || owner.Location != "char-slot[0]/inv[0]" {
		t.Fatalf("generated owner=%+v present=%v", owner, ok)
	}
	if _, kept := next["old-item"]; kept {
		t.Fatal("replaced account retained its former item owner")
	}
}

func TestPrepareAccountItemUIDsAllowsMovesAndRejectsDuplicateOwners(t *testing.T) {
	const uid = "00112233445566778899aabbccddeeff"
	account := itemUIDTestAccount("first")
	account.Chars[0].Inv[0] = model.Item{Index: 100, UID: uid}
	store := &JSONStore{itemOwners: map[string]itemUIDOwner{uid: {Account: "first", Location: "cargo[0]"}}}
	next, changed, err := store.prepareAccountItemUIDs(account)
	if err != nil || changed != 0 || next[uid].Location != "char-slot[0]/inv[0]" {
		t.Fatalf("same-account move: changed=%d owner=%+v err=%v", changed, next[uid], err)
	}

	store.itemOwners = map[string]itemUIDOwner{uid: {Account: "other", Location: "cargo[2]"}}
	if _, _, err := store.prepareAccountItemUIDs(account); err == nil || !strings.Contains(err.Error(), "duplicado") {
		t.Fatalf("duplicate owner accepted: %v", err)
	}

	duplicate := itemUIDTestAccount("first")
	duplicate.Cargo[0] = model.Item{Index: 100, UID: uid}
	duplicate.Cargo[1] = model.Item{Index: 101, UID: uid}
	if _, _, err := (&JSONStore{}).prepareAccountItemUIDs(duplicate); err == nil || !strings.Contains(err.Error(), "duplicado") {
		t.Fatalf("duplicate slots accepted: %v", err)
	}
}

func TestPrepareAccountItemUIDsRejectsMalformedItemsAndRollsBackGeneratedUIDs(t *testing.T) {
	tests := []struct {
		name string
		item model.Item
		want string
	}{
		{name: "empty slot identity", item: model.Item{UID: "unexpected"}, want: "vazio"},
		{name: "empty slot expiry", item: model.Item{ExpiresUnix: 12}, want: "vazio"},
		{name: "invalid UID", item: model.Item{Index: 100, UID: "bad"}, want: "expected 32"},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			account := itemUIDTestAccount("first")
			account.Chars[0].Inv[0] = tc.item
			if _, _, err := (&JSONStore{}).prepareAccountItemUIDs(account); err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("malformed item error=%v, want %q", err, tc.want)
			}
		})
	}

	account := itemUIDTestAccount("first")
	account.Chars[0].Inv[0] = model.Item{Index: 100}
	account.Chars[0].Inv[1] = model.Item{UID: "invalid empty-slot identity"}
	if _, _, err := (&JSONStore{}).prepareAccountItemUIDs(account); err == nil {
		t.Fatal("malformed trailing item was accepted")
	}
	if account.Chars[0].Inv[0].UID != "" {
		t.Fatalf("generated identity remained after rejected batch: %q", account.Chars[0].Inv[0].UID)
	}
}

func TestPrepareAccountItemUIDsRequiresNamedAccounts(t *testing.T) {
	for _, tc := range []struct {
		name    string
		account *model.Account
		want    string
	}{
		{name: "nil account", account: nil, want: "conta ausente"},
		{name: "empty name", account: &model.Account{}, want: "conta invalida"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if _, _, err := (&JSONStore{}).prepareAccountItemUIDs(tc.account); err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("invalid account error=%v, want %q", err, tc.want)
			}
		})
	}
}
