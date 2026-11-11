package model

import "testing"

const (
	coverageCharacterUID = "11111111111141118111111111111111"
	coverageItemUID      = "0123456789abcdef0123456789abcdef"
	coverageSourceUID    = "fedcba9876543210fedcba9876543210"
)

func validCoverageAccount() *Account {
	return &Account{Name: "coverage-account", PasswordHash: "hash", Chars: []Char{{
		UID: coverageCharacterUID, Name: "Coverage", Score: &Score{
			Version: ScoreVersion, MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100,
		},
	}}}
}

func validCoverageCapsule(id uint16, itemUID, sourceUID string) CelestialCapsule {
	return CelestialCapsule{ID: id, ItemUID: itemUID, SourceUID: sourceUID, CreatedUnix: 1,
		Character: Char{UID: sourceUID, Name: "Celestial", Evolution: "celestial",
			Score: &Score{Version: ScoreVersion, Level: 90}}}
}

func validCoverageCapsuleAccount() *Account {
	a := validCoverageAccount()
	capsule := validCoverageCapsule(7, coverageItemUID, coverageSourceUID)
	a.CelestialCapsules = []CelestialCapsule{capsule}
	a.Cargo[0] = NewCelestialSeal(capsule.ItemUID, capsule.ID)
	return a
}

func TestAccountValidateRejectsInvalidIdentityAndCharacterState(t *testing.T) {
	if err := (*Account)(nil).Validate(); err == nil {
		t.Fatal("nil account was accepted")
	}
	for _, test := range []struct {
		name   string
		mutate func(*Account)
	}{
		{"missing name", func(a *Account) { a.Name = "" }},
		{"missing password hash", func(a *Account) { a.PasswordHash = "" }},
		{"donate balance above limit", func(a *Account) { a.Donate = MaxDonateBalance + 1 }},
		{"too many character slots", func(a *Account) { a.Chars = make([]Char, 5) }},
		{"empty slot contains state", func(a *Account) { a.Chars = append(a.Chars, Char{Gold: 1}) }},
		{"invalid score", func(a *Account) { a.Chars[0].Score.Version = 0 }},
		{"invalid alternate celestial", func(a *Account) {
			a.Chars[0].AlternateCelestial = &CelestialForm{Evolution: "mortal"}
		}},
		{"malformed character UID", func(a *Account) { a.Chars[0].UID = "bad" }},
		{"noncanonical character UID", func(a *Account) { a.Chars[0].UID = "1111111111114111811111111111111A" }},
		{"invalid Arch mortal UID", func(a *Account) { a.Chars[0].ArchMortalUID = "bad" }},
		{"reserved Carry slot", func(a *Account) { a.Chars[0].Inv[PlayerCarrySlots].Index = 100 }},
		{"reserved Cargo slot", func(a *Account) { a.Cargo[PlayerCargoSlots].Index = 100 }},
	} {
		t.Run(test.name, func(t *testing.T) {
			a := validCoverageAccount()
			test.mutate(a)
			if err := a.Validate(); err == nil {
				t.Fatal("invalid account state was accepted")
			}
		})
	}
}

func TestAccountValidateRejectsInvalidItemDeadlines(t *testing.T) {
	for _, test := range []struct {
		name string
		item Item
	}{
		{"negative activation", Item{Index: 3900, ActivatedUnix: -1, ExpiresUnix: 10}},
		{"negative expiration", Item{Index: 3900, ActivatedUnix: 1, ExpiresUnix: -1}},
		{"activation without expiration", Item{Index: 3900, ActivatedUnix: 10}},
		{"expiration before activation", Item{Index: 3900, ActivatedUnix: 20, ExpiresUnix: 10}},
		{"negative pause", Item{Index: 3900, ActivatedUnix: 1, ExpiresUnix: 10, PausedUnix: -1}},
		{"pause before activation", Item{Index: 3900, ActivatedUnix: 10, ExpiresUnix: 30, PausedUnix: 9}},
		{"pause at expiration", Item{Index: 3900, ActivatedUnix: 10, ExpiresUnix: 30, PausedUnix: 30}},
		{"pause on non-timed item", Item{Index: 3899, ActivatedUnix: 10, ExpiresUnix: 30, PausedUnix: 20}},
		{"empty item has UID", Item{UID: coverageItemUID}},
		{"timed item has malformed UID", Item{Index: 3900, UID: "bad", ActivatedUnix: 10, ExpiresUnix: 30}},
		{"timed item has noncanonical UID", Item{Index: 3900, UID: "0123456789ABCDEF0123456789ABCDEF", ActivatedUnix: 10, ExpiresUnix: 30}},
	} {
		t.Run(test.name, func(t *testing.T) {
			a := validCoverageAccount()
			a.Chars[0].Inv[0] = test.item
			if err := a.Validate(); err == nil {
				t.Fatalf("invalid item deadline was accepted: %+v", test.item)
			}
		})
	}
}

func TestAccountValidateChecksCelestialCapsuleAndSealOwnership(t *testing.T) {
	if err := validCoverageCapsuleAccount().Validate(); err != nil {
		t.Fatalf("valid capsule/seal aggregate was rejected: %v", err)
	}

	for _, test := range []struct {
		name   string
		mutate func(*Account)
	}{
		{"invalid capsule", func(a *Account) { a.CelestialCapsules[0].ID = 0 }},
		{"duplicate capsule ID", func(a *Account) {
			a.CelestialCapsules = append(a.CelestialCapsules,
				validCoverageCapsule(7, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"))
		}},
		{"duplicate capsule item UID", func(a *Account) {
			a.CelestialCapsules = append(a.CelestialCapsules,
				validCoverageCapsule(8, coverageItemUID, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"))
		}},
		{"duplicate capsule source UID", func(a *Account) {
			a.CelestialCapsules = append(a.CelestialCapsules,
				validCoverageCapsule(8, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", coverageSourceUID))
		}},
		{"active character also has capsule", func(a *Account) {
			a.CelestialCapsules[0] = validCoverageCapsule(7, coverageItemUID, coverageCharacterUID)
			a.CelestialCapsules[0].Character.Name = "Coverage"
		}},
		{"filled seal without snapshot", func(a *Account) {
			a.CelestialCapsules = nil
			a.Cargo[0] = NewCelestialSeal(coverageItemUID, 7)
		}},
		{"seal item UID does not match capsule", func(a *Account) {
			a.Cargo[0] = NewCelestialSeal("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 7)
		}},
		{"duplicate seal", func(a *Account) {
			a.Chars[0].Inv[0] = a.Cargo[0]
		}},
		{"seal in equipment", func(a *Account) {
			a.Chars[0].Equip[0] = a.Cargo[0]
		}},
		{"capsule has no seal", func(a *Account) { a.Cargo[0] = Item{} }},
	} {
		t.Run(test.name, func(t *testing.T) {
			a := validCoverageCapsuleAccount()
			test.mutate(a)
			if err := a.Validate(); err == nil {
				t.Fatal("inconsistent capsule/seal aggregate was accepted")
			}
		})
	}
}
