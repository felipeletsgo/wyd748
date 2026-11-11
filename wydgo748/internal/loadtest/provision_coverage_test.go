package loadtest

import (
	"errors"
	"os"
	"testing"

	"wydgo/internal/model"
)

type scriptedAccountStore struct {
	accounts     map[string]*model.Account
	loadErrors   map[string]error
	createErrors map[string]error
}

func (s *scriptedAccountStore) LoadAccount(name string) (*model.Account, error) {
	if err := s.loadErrors[name]; err != nil {
		return nil, err
	}
	return s.accounts[name], nil
}

func (s *scriptedAccountStore) CreateAccount(acc *model.Account) error {
	if err := s.createErrors[acc.Name]; err != nil {
		return err
	}
	if s.accounts[acc.Name] != nil {
		return errAccountExists
	}
	s.accounts[acc.Name] = acc
	return nil
}

func validProvisionConfig() ProvisionConfig {
	return ProvisionConfig{
		SourceAccount: "felipe", BotCount: 1, AccountPrefix: "bot",
		CharacterPrefix: "Bot", BotPassword: "1234",
	}
}

func provisioningStores() (*scriptedAccountStore, *scriptedAccountStore) {
	source := &scriptedAccountStore{accounts: map[string]*model.Account{"felipe": testSourceAccount()}}
	target := &scriptedAccountStore{
		accounts:   map[string]*model.Account{},
		loadErrors: map[string]error{"felipe": os.ErrNotExist},
	}
	return target, source
}

func TestProvisionConfigDefaultsAndValidation(t *testing.T) {
	defaults, err := (ProvisionConfig{BotPassword: "1234"}).normalize()
	if err != nil {
		t.Fatal(err)
	}
	if defaults.SourceAccount != "felipe" || defaults.BotCount != 990 || defaults.AccountPrefix != "bot" || defaults.CharacterPrefix != "Bot" {
		t.Fatalf("defaults: %+v", defaults)
	}
	for name, mutate := range map[string]func(*ProvisionConfig){
		"bot count too small":              func(c *ProvisionConfig) { c.BotCount = -1 },
		"bot count too large":              func(c *ProvisionConfig) { c.BotCount = 991 },
		"account prefix too long":          func(c *ProvisionConfig) { c.AccountPrefix = "botabcdef" },
		"account prefix nonletter":         func(c *ProvisionConfig) { c.AccountPrefix = "bot1" },
		"account prefix outside namespace": func(c *ProvisionConfig) { c.AccountPrefix = "load" },
		"character prefix too long":        func(c *ProvisionConfig) { c.CharacterPrefix = "Character" },
		"character prefix nonletter":       func(c *ProvisionConfig) { c.CharacterPrefix = "Bot1" },
		"invalid password":                 func(c *ProvisionConfig) { c.BotPassword = "bad\n" },
	} {
		t.Run(name, func(t *testing.T) {
			cfg := validProvisionConfig()
			mutate(&cfg)
			if _, err := cfg.normalize(); err == nil {
				t.Fatal("invalid provisioning configuration was accepted")
			}
		})
	}
}

func TestProvisionBotsRejectsStoreAndSourceFailures(t *testing.T) {
	cfg := validProvisionConfig()
	if _, err := ProvisionBotsFromSource(nil, nil, cfg); err == nil {
		t.Fatal("missing stores were accepted")
	}
	t.Run("source load error", func(t *testing.T) {
		source := &scriptedAccountStore{accounts: map[string]*model.Account{}, loadErrors: map[string]error{"felipe": errors.New("read failed")}}
		if _, err := ProvisionBotsFromSource(&scriptedAccountStore{accounts: map[string]*model.Account{}}, source, cfg); err == nil {
			t.Fatal("source load failure was ignored")
		}
	})
	t.Run("source missing", func(t *testing.T) {
		if _, err := ProvisionBotsFromSource(&scriptedAccountStore{accounts: map[string]*model.Account{}},
			&scriptedAccountStore{accounts: map[string]*model.Account{}}, cfg); err == nil {
			t.Fatal("missing source account was accepted")
		}
	})
	t.Run("target load error", func(t *testing.T) {
		source := &scriptedAccountStore{accounts: map[string]*model.Account{"felipe": testSourceAccount()}}
		target := &scriptedAccountStore{accounts: map[string]*model.Account{}, loadErrors: map[string]error{"felipe": errors.New("destination read failed")}}
		if _, err := ProvisionBotsFromSource(target, source, cfg); err == nil {
			t.Fatal("target load failure was ignored")
		}
	})
	t.Run("source account create error", func(t *testing.T) {
		target, source := provisioningStores()
		target.createErrors = map[string]error{"felipe": errors.New("destination write failed")}
		if _, err := ProvisionBotsFromSource(target, source, cfg); err == nil {
			t.Fatal("target create failure was ignored")
		}
	})
	t.Run("source character missing", func(t *testing.T) {
		target, source := provisioningStores()
		cfg := cfg
		cfg.SourceCharacter = "Missing"
		if _, err := ProvisionBotsFromSource(target, source, cfg); err == nil {
			t.Fatal("missing source character was accepted")
		}
	})
	t.Run("source character has no score", func(t *testing.T) {
		target, source := provisioningStores()
		delete(target.loadErrors, "felipe")
		target.accounts["felipe"] = testSourceAccount()
		account := source.accounts["felipe"]
		account.Chars[0].Score = nil
		if _, err := ProvisionBotsFromSource(target, source, cfg); err == nil {
			t.Fatal("source character without score was accepted")
		}
	})
}

func TestProvisionBotsHandlesCreateRacesAndBotFailures(t *testing.T) {
	t.Run("missing source copied after not found", func(t *testing.T) {
		target, source := provisioningStores()
		result, err := ProvisionBotsFromSource(target, source, validProvisionConfig())
		if err != nil || !result.SourceCreated || result.Created != 1 {
			t.Fatalf("result=%+v err=%v", result, err)
		}
	})
	t.Run("source copy race", func(t *testing.T) {
		target, source := provisioningStores()
		target.createErrors = map[string]error{"felipe": errAccountExists}
		result, err := ProvisionBotsFromSource(target, source, validProvisionConfig())
		if err != nil || result.SourceCreated || result.Created != 1 {
			t.Fatalf("result=%+v err=%v", result, err)
		}
	})
	t.Run("existing source account preserved", func(t *testing.T) {
		target, source := provisioningStores()
		target.accounts["felipe"] = testSourceAccount()
		result, err := ProvisionBotsFromSource(target, source, validProvisionConfig())
		if err != nil || result.SourceCreated || result.Created != 1 {
			t.Fatalf("result=%+v err=%v", result, err)
		}
	})
	t.Run("existing nil source account filled", func(t *testing.T) {
		target, source := provisioningStores()
		delete(target.loadErrors, "felipe")
		target.accounts["felipe"] = nil
		result, err := ProvisionBotsFromSource(target, source, validProvisionConfig())
		if err != nil || !result.SourceCreated || result.Created != 1 {
			t.Fatalf("result=%+v err=%v", result, err)
		}
	})
	t.Run("existing nil source account create error", func(t *testing.T) {
		target, source := provisioningStores()
		delete(target.loadErrors, "felipe")
		target.accounts["felipe"] = nil
		target.createErrors = map[string]error{"felipe": errors.New("destination write failed")}
		if _, err := ProvisionBotsFromSource(target, source, validProvisionConfig()); err == nil {
			t.Fatal("failed source copy was ignored")
		}
	})
	t.Run("existing bot skipped", func(t *testing.T) {
		target, source := provisioningStores()
		target.createErrors = map[string]error{"bot0001": errAccountExists}
		result, err := ProvisionBotsFromSource(target, source, validProvisionConfig())
		if err != nil || result.Skipped != 1 || result.Created != 0 {
			t.Fatalf("result=%+v err=%v", result, err)
		}
	})
	t.Run("bot create error", func(t *testing.T) {
		target, source := provisioningStores()
		target.createErrors = map[string]error{"bot0001": errors.New("write failed")}
		result, err := ProvisionBotsFromSource(target, source, validProvisionConfig())
		if err == nil || result.Created != 0 {
			t.Fatalf("result=%+v err=%v", result, err)
		}
	})
}

func TestSelectCharacterErrorBranches(t *testing.T) {
	empty := &model.Account{Chars: []model.Char{{}, {Name: "Hero"}}}
	negative, outOfRange, blank := -2, 3, 0
	for _, tc := range []struct {
		name       string
		acc        *model.Account
		nameFilter string
		slot       *int
	}{
		{name: "nil account"},
		{name: "slot less than minus one", acc: empty, slot: &negative},
		{name: "slot out of range", acc: empty, slot: &outOfRange},
		{name: "empty slot", acc: empty, slot: &blank},
		{name: "slot name mismatch", acc: empty, nameFilter: "Other", slot: func() *int { i := 1; return &i }()},
		{name: "name missing", acc: empty, nameFilter: "Missing"},
		{name: "no character", acc: &model.Account{}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if _, err := selectCharacter(tc.acc, tc.nameFilter, tc.slot); err == nil {
				t.Fatal("invalid character selection was accepted")
			}
		})
	}
	selected, err := selectCharacter(empty, "", nil)
	if err != nil || selected.Name != "Hero" {
		t.Fatalf("fallback selection=%+v err=%v", selected, err)
	}
	selected, err = selectCharacter(empty, "hero", nil)
	if err != nil || selected.Name != "Hero" {
		t.Fatalf("case-insensitive name selection=%+v err=%v", selected, err)
	}
}

func TestCloneCopiesAlternateCelestialAndRejectsMissingScore(t *testing.T) {
	if _, err := cloneForLoadtest(nil, "bot0001", "BotAAAAA", "hash"); err == nil {
		t.Fatal("nil source character was accepted")
	}
	withoutScore := &model.Char{}
	if _, err := cloneForLoadtest(withoutScore, "bot0001", "BotAAAAA", "hash"); err == nil {
		t.Fatal("source without score was accepted")
	}
	origin := testSourceAccount().Chars[0]
	origin.Score.CurHP, origin.Score.CurMP = 1, 2
	origin.AlternateCelestial = &model.CelestialForm{
		Evolution: "celestial", Score: &model.Score{MaxHP: 50, CurHP: 1, MaxMP: 60, CurMP: 2},
	}
	origin.Equip[0].UID, origin.Inv[0].UID = "equip", "inventory"
	clone, err := cloneForLoadtest(&origin, "bot0001", "BotAAAAA", "hash")
	if err != nil {
		t.Fatal(err)
	}
	if clone.Chars[0].AlternateCelestial == origin.AlternateCelestial ||
		clone.Chars[0].AlternateCelestial.Score == origin.AlternateCelestial.Score ||
		clone.Chars[0].AlternateCelestial.Score.CurHP != 50 || clone.Chars[0].AlternateCelestial.Score.CurMP != 60 {
		t.Fatalf("alternate form was not detached and restored: %+v", clone.Chars[0].AlternateCelestial)
	}
	if clone.Chars[0].Equip[0].UID != "" || clone.Chars[0].Inv[0].UID != "" ||
		origin.AlternateCelestial.Score.CurHP != 1 || origin.AlternateCelestial.Score.CurMP != 2 {
		t.Fatal("clone mutated or retained identity from source")
	}
}

func TestCopyAccountAndAlreadyExistsClassification(t *testing.T) {
	if _, err := copyAccount(nil); err == nil {
		t.Fatal("nil account copied")
	}
	clone, err := copyAccount(testSourceAccount())
	if err != nil || clone.Name != "felipe" || len(clone.Chars) == 0 {
		t.Fatalf("clone=%+v err=%v", clone, err)
	}
	for _, tc := range []struct {
		err  error
		want bool
	}{
		{err: nil, want: false},
		{err: errors.New("ja existe"), want: true},
		{err: errors.New("already exists"), want: true},
		{err: errors.New("duplicate key"), want: true},
		{err: errors.New("connection failed"), want: false},
	} {
		if got := isAlreadyExists(tc.err); got != tc.want {
			t.Fatalf("isAlreadyExists(%v)=%t, want %t", tc.err, got, tc.want)
		}
	}
}
