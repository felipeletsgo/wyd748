package webadmin

import (
	"context"
	"errors"
	"os"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

type persistentAccounts struct {
	player    store.PersistentPlayerSnapshot
	directory store.AccountDirectoryPageSnapshot
	err       error
}

func (s persistentAccounts) ReadAccountDirectory(context.Context, string, string, int) (store.AccountDirectoryPageSnapshot, error) {
	if s.err != nil {
		return store.AccountDirectoryPageSnapshot{}, s.err
	}
	return s.directory, nil
}

func (s persistentAccounts) ReadPersistentPlayer(context.Context, string, string) (store.PersistentPlayerSnapshot, error) {
	if s.err != nil {
		return store.PersistentPlayerSnapshot{}, s.err
	}
	return s.player, nil
}

func TestAccountPersistentReaderProjectsCharacter(t *testing.T) {
	uid, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	wantTime := time.Date(2026, 9, 13, 20, 0, 0, 0, time.UTC)
	player := store.PersistentPlayerSnapshot{
		Account: "fixture", UID: uid, Name: "Sentinela", Class: 2, Level: 120,
		Evolution: "arch", HP: 400, MaxHP: 500, MP: 200, MaxMP: 300,
		Attack: 90, MagicAttack: 80, Defense: 70, Str: 60, Int: 50, Dex: 40, Con: 30,
		StatusPoints: 20, MasteryPoints: 10, SkillPoints: 5, X: 2101, Y: 2102,
		Gold: 1234, Exp: 9876, Hold: 50, CP: -3,
	}
	reader := NewAccountPersistentReader(persistentAccounts{player: player}).(*accountPersistentReader)
	reader.now = func() time.Time { return wantTime }
	got, err := reader.Player(context.Background(), "fixture", uid)
	if err != nil {
		t.Fatal(err)
	}
	if got.Version != 1 || got.AsOf != wantTime || got.Account != "fixture" || got.UID != uid || got.Name != "Sentinela" || got.Class != 2 || got.Level != 120 || got.Evolution != "arch" {
		t.Fatalf("identity projection mismatch: %+v", got)
	}
	if got.HP != 400 || got.MaxHP != 500 || got.MP != 200 || got.MaxMP != 300 || got.Attack != 90 || got.MagicAttack != 80 || got.Defense != 70 {
		t.Fatalf("score projection mismatch: %+v", got)
	}
	if got.Str != 60 || got.Int != 50 || got.Dex != 40 || got.Con != 30 || got.StatusPoints != 20 || got.MasteryPoints != 10 || got.SkillPoints != 5 {
		t.Fatalf("attribute projection mismatch: %+v", got)
	}
	if got.X != 2101 || got.Y != 2102 || got.Gold != 1234 || got.Exp != 9876 || got.Hold != 50 || got.CP != -3 {
		t.Fatalf("persistent fields mismatch: %+v", got)
	}
}

func TestAccountPersistentReaderRejectsInvalidAndMissingTargets(t *testing.T) {
	reader := NewAccountPersistentReader(persistentAccounts{})
	if _, err := reader.Player(context.Background(), "fixture", "not-a-character-uid"); !errors.Is(err, ErrInvalidPersistentTarget) {
		t.Fatalf("invalid UID: %v", err)
	}
	uid, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	if _, err := reader.Player(context.Background(), "fixture", uid); !errors.Is(err, ErrPersistentPlayerNotFound) {
		t.Fatalf("missing character: %v", err)
	}
	reader = NewAccountPersistentReader(persistentAccounts{err: os.ErrNotExist})
	if _, err := reader.Player(context.Background(), "fixture", uid); !errors.Is(err, ErrPersistentPlayerNotFound) {
		t.Fatalf("missing account: %v", err)
	}
}

func TestAccountPersistentReaderProjectsDirectory(t *testing.T) {
	now := time.Date(2026, 9, 20, 12, 0, 0, 0, time.UTC)
	reader := NewAccountPersistentReader(persistentAccounts{directory: store.AccountDirectoryPageSnapshot{
		NextCursor: "fixture",
		Accounts: []store.AccountDirectorySnapshot{{
			Key: "fixture", Name: "Fixture", CreatedAt: now.Add(-time.Hour), UpdatedAt: now,
			Characters: []store.AccountDirectoryCharacterSnapshot{{UID: "abcd", Slot: 1, Name: "Sentinela", Class: 2, Level: 120, Evolution: "arch"}},
		}},
	}}).(*accountPersistentReader)
	reader.now = func() time.Time { return now }
	got, err := reader.Accounts(context.Background(), "FIX", "", 20)
	if err != nil {
		t.Fatal(err)
	}
	if got.Version != 1 || got.AsOf != now || got.NextCursor != "fixture" || len(got.Accounts) != 1 {
		t.Fatalf("directory projection mismatch: %+v", got)
	}
	account := got.Accounts[0]
	if account.Username != "Fixture" || len(account.Characters) != 1 || account.Characters[0].Name != "Sentinela" || account.Characters[0].Level != 120 {
		t.Fatalf("account projection mismatch: %+v", account)
	}
	if _, err := reader.Accounts(context.Background(), "inválido", "", 20); !errors.Is(err, ErrInvalidAccountDirectory) {
		t.Fatalf("invalid directory search: %v", err)
	}
	if _, err := reader.Accounts(context.Background(), "", "", 51); !errors.Is(err, ErrInvalidAccountDirectory) {
		t.Fatalf("invalid directory limit: %v", err)
	}
}
