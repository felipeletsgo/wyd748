package model

import (
	"strings"
	"testing"
)

const (
	validCapsuleItemUID   = "0123456789abcdef0123456789abcdef"
	validCapsuleSourceUID = "fedcba9876543210fedcba9876543210"
)

func validCelestialCapsule() CelestialCapsule {
	return CelestialCapsule{
		ID: 7, ItemUID: validCapsuleItemUID, SourceUID: validCapsuleSourceUID,
		Character: Char{
			UID: validCapsuleSourceUID, Name: "Celestial", Evolution: "celestial",
			Score: &Score{Version: ScoreVersion, Level: 90},
		},
		CreatedUnix: 1,
	}
}

func TestCelestialSealEncodeAndDecode(t *testing.T) {
	item := NewCelestialSeal(validCapsuleItemUID, 0x1234)
	if item.Index != CelestialSealItem || item.UID != validCapsuleItemUID ||
		item.Eff != [6]byte{CelestialSealEffect, 0x12, CelestialSealEffect, 0x34, 0, 0} {
		t.Fatalf("encoded seal = %+v", item)
	}
	if id, ok := CelestialSealID(item); !ok || id != 0x1234 {
		t.Fatalf("decoded seal = (%#x, %t), want (0x1234, true)", id, ok)
	}
	for _, item := range []Item{
		{},
		{Index: CelestialSealItem, Eff: [6]byte{CelestialSealEffect, 0, CelestialSealEffect, 0}},
		{Index: CelestialSealItem, Eff: [6]byte{1, 0, CelestialSealEffect, 1}},
		{Index: CelestialSealItem, Eff: [6]byte{CelestialSealEffect, 0, 1, 1}},
	} {
		if id, ok := CelestialSealID(item); ok || id != 0 {
			t.Errorf("invalid or empty seal decoded as (%d, %t): %+v", id, ok, item)
		}
	}
}

func TestCelestialCapsuleValidationRejectsInvalidSnapshots(t *testing.T) {
	for _, test := range []struct {
		name   string
		mutate func(*CelestialCapsule)
	}{
		{"zero capsule ID", func(c *CelestialCapsule) { c.ID = 0 }},
		{"malformed item UID", func(c *CelestialCapsule) { c.ItemUID = "short" }},
		{"noncanonical item UID", func(c *CelestialCapsule) { c.ItemUID = strings.ToUpper(c.ItemUID) }},
		{"malformed source UID", func(c *CelestialCapsule) { c.SourceUID = "bad" }},
		{"noncanonical source UID", func(c *CelestialCapsule) { c.SourceUID = strings.ToUpper(c.SourceUID) }},
		{"missing name", func(c *CelestialCapsule) { c.Character.Name = "" }},
		{"missing character UID", func(c *CelestialCapsule) { c.Character.UID = "" }},
		{"source UID mismatch", func(c *CelestialCapsule) { c.Character.UID = validCapsuleItemUID }},
		{"invalid evolution", func(c *CelestialCapsule) { c.Character.Evolution = "mortal" }},
		{"invalid score", func(c *CelestialCapsule) { c.Character.Score.Version = 0 }},
	} {
		t.Run(test.name, func(t *testing.T) {
			capsule := validCelestialCapsule()
			test.mutate(&capsule)
			if err := capsule.Validate(); err == nil {
				t.Fatal("invalid capsule snapshot was accepted")
			}
		})
	}
	if err := (CelestialCapsule{}).Validate(); err == nil {
		t.Fatal("empty capsule was accepted")
	}
}

func TestCelestialCapsuleValidationAcceptsSubCelestialSnapshot(t *testing.T) {
	capsule := validCelestialCapsule()
	capsule.Character.Evolution = " SubCelestial "
	if err := capsule.Validate(); err != nil {
		t.Fatalf("valid SubCelestial snapshot was rejected: %v", err)
	}
}
