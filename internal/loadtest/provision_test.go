package loadtest

import (
	"testing"

	"wydgo/internal/model"
)

type memoryAccountStore struct {
	accounts map[string]*model.Account
}

func (s *memoryAccountStore) LoadAccount(name string) (*model.Account, error) {
	return s.accounts[name], nil
}

func (s *memoryAccountStore) CreateAccount(acc *model.Account) error {
	if s.accounts[acc.Name] != nil {
		return errAccountExists
	}
	s.accounts[acc.Name] = acc
	return nil
}

var errAccountExists = &accountExistsError{}

type accountExistsError struct{}

func (*accountExistsError) Error() string { return "ja existe" }

func testSourceAccount() *model.Account {
	mortal := model.Char{
		UID:  "11111111111141118111111111111111",
		Name: "Felipe", Class: 0,
		Score: &model.Score{
			Version: model.ScoreVersion,
			Level:   399, MaxHP: 100000, CurHP: 90000,
			MaxMP: 90000, CurMP: 80000, Attack: 5000,
		},
		LearnedSkill: 123, Equip: [16]model.Item{{Index: 100}},
	}
	arch := mortal
	arch.UID = "22222222222242228222222222222222"
	arch.Evolution = "arch"
	arch.Class = 2
	arch.ArchMortalUID = mortal.UID
	arch.Equip[0] = model.Item{Index: 9000}
	return &model.Account{
		Name:         "felipe",
		PasswordHash: "hash",
		Chars:        []model.Char{mortal, arch},
	}
}

func TestGeneratedCharacterNamesAreAlphabeticAndUnique(t *testing.T) {
	seen := map[string]struct{}{}
	for i := 0; i < 990; i++ {
		name := generatedCharacterName("Bot", i)
		if len(name) > 13 || name == "" {
			t.Fatalf("nome invalido %q", name)
		}
		for _, c := range name {
			if c < 'A' || c > 'Z' {
				t.Fatalf("nome contem caractere nao alfabetico: %q", name)
			}
		}
		if _, ok := seen[name]; ok {
			t.Fatalf("nome repetido %q", name)
		}
		seen[name] = struct{}{}
	}
}

func TestCloneForLoadtestRegeneratesIdentityAndRestoresVitals(t *testing.T) {
	source := testSourceAccount().Chars[0]
	clone, err := cloneForLoadtest(&source, "bot0001", "BotAAAAA", "hash")
	if err != nil {
		t.Fatal(err)
	}
	ch := clone.Chars[0]
	if ch.UID != "" || ch.Name != "BotAAAAA" || ch.Score.CurHP != ch.Score.MaxHP ||
		ch.Score.CurMP != ch.Score.MaxMP {
		t.Fatalf("clone incorreto: %+v", ch)
	}
	if ch.Equip[0].UID != "" || ch.LearnedSkill != source.LearnedSkill {
		t.Fatalf("clone perdeu habilidades ou manteve UID: %+v", ch)
	}
}

func TestProvisionBotsCreatesRequestedCountAndSkipsExisting(t *testing.T) {
	st := &memoryAccountStore{accounts: map[string]*model.Account{"felipe": testSourceAccount()}}
	result, err := ProvisionBotsFromSource(st, st, ProvisionConfig{
		SourceAccount: "felipe", BotCount: 3, AccountPrefix: "bot",
		CharacterPrefix: "Bot", BotPassword: "123456",
	})
	if err != nil {
		t.Fatal(err)
	}
	if result.Created != 3 || len(st.accounts) != 4 {
		t.Fatalf("provisionamento=%+v contas=%d", result, len(st.accounts))
	}
	result, err = ProvisionBotsFromSource(st, st, ProvisionConfig{
		SourceAccount: "felipe", BotCount: 3, AccountPrefix: "bot",
		CharacterPrefix: "Bot", BotPassword: "123456",
	})
	if err != nil {
		t.Fatal(err)
	}
	if result.Skipped != 3 || result.Created != 0 {
		t.Fatalf("repeticao=%+v", result)
	}
}

func TestSelectCharacterByExactSlotWhenNicknamesRepeat(t *testing.T) {
	account := testSourceAccount()
	slot := 1
	character, err := selectCharacter(account, "Felipe", &slot)
	if err != nil {
		t.Fatal(err)
	}
	if character.Evolution != "arch" || character.Equip[0].Index != 9000 {
		t.Fatalf("slot Arch incorreto: evolution=%q item=%d", character.Evolution, character.Equip[0].Index)
	}
	legacy := -1
	character, err = selectCharacter(account, "", &legacy)
	if err != nil || character.Evolution != "" {
		t.Fatalf("selecao legada incorreta: character=%+v err=%v", character, err)
	}
}

func TestSelectCharacterRejectsEmptyOrOutOfRangeSlot(t *testing.T) {
	account := testSourceAccount()
	for _, slot := range []int{2, -2} {
		slot := slot
		if _, err := selectCharacter(account, "", &slot); err == nil {
			t.Fatalf("slot %d deveria ser recusado", slot)
		}
	}
}

func TestProvisionBotsCopiesModelIntoEmptyDisposableStore(t *testing.T) {
	source := &memoryAccountStore{accounts: map[string]*model.Account{"felipe": testSourceAccount()}}
	target := &memoryAccountStore{accounts: map[string]*model.Account{}}
	result, err := ProvisionBotsFromSource(target, source, ProvisionConfig{
		SourceAccount: "felipe", BotCount: 1, AccountPrefix: "bot",
		CharacterPrefix: "Bot", BotPassword: "123456",
	})
	if err != nil {
		t.Fatal(err)
	}
	if !result.SourceCreated || result.Created != 1 {
		t.Fatalf("resultado=%+v", result)
	}
	if target.accounts["felipe"] == nil || target.accounts["bot0001"] == nil {
		t.Fatalf("contas materializadas: %#v", target.accounts)
	}
	if target.accounts["felipe"].Chars[0].Name != "Felipe" ||
		target.accounts["bot0001"].Chars[0].Name == "Felipe" {
		t.Fatal("modelo/clone incorretos")
	}
	if source.accounts["felipe"].Chars[0].UID == "" {
		t.Fatal("a conta fonte foi alterada durante a copia")
	}
}

func TestProvisionBotsRejectsNonBotPrefix(t *testing.T) {
	st := &memoryAccountStore{accounts: map[string]*model.Account{"felipe": testSourceAccount()}}
	if _, err := ProvisionBotsFromSource(st, st, ProvisionConfig{
		SourceAccount: "felipe", BotCount: 1, AccountPrefix: "load",
		CharacterPrefix: "Bot", BotPassword: "123456",
	}); err == nil {
		t.Fatal("prefixo fora de bot deveria ser recusado")
	}
}
