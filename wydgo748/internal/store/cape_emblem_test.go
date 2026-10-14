package store

import (
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestCapeEmblemSurvivesStoreReloadAndSubsequentSave(t *testing.T) {
	dir := filepath.Join(t.TempDir(), "accounts")
	s := NewJSONStore(dir)
	a := &model.Account{Name: "cosmetic", PasswordHash: "test-only", Chars: []model.Char{{
		Name: "Cosmetic", Score: &model.Score{Version: model.ScoreVersion},
		Citizenship: 1, CapeEmblem: 10,
	}}}
	a.Chars[0].Equip[model.CapeSlot] = model.Item{Index: 545}
	if err := s.SaveAccount(a); err != nil {
		t.Fatal(err)
	}
	for i := 0; i < 2; i++ {
		reloaded := NewJSONStore(dir)
		got, err := reloaded.LoadAccount("cosmetic")
		if err != nil {
			t.Fatal(err)
		}
		if got.Chars[0].CapeEmblem != 10 || got.Chars[0].Citizenship != 1 || got.Chars[0].Equip[15].Index != 545 {
			t.Fatal("store restart or later save lost the independent symbol")
		}
		got.Chars[0].Gold++
		if err := reloaded.SaveAccount(got); err != nil {
			t.Fatal(err)
		}
	}
}
