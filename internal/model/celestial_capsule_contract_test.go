package model

import (
	"strings"
	"testing"
)

func validCapsuleContract() CelestialCapsule {
	itemUID := "11111111111141118111111111111111"
	charUID := "22222222222242228222222222222222"
	return CelestialCapsule{
		ID: 513, ItemUID: itemUID, SourceUID: charUID,
		Character: Char{
			UID: charUID, Name: "CapsuleChar", Evolution: "celestial",
			Extended: &ExtendedScore{Version: ExtendedScoreVersion, MaxHP: 100, CurHP: 100, MaxMP: 50, CurMP: 50},
		},
	}
}

func TestCelestialSealRoundTripAndRejectsMalformedEffects(t *testing.T) {
	uid := "aaaaaaaaaaaa4aaa8aaaaaaaaaaaaaaa"
	item := NewCelestialSeal(uid, 0xBEEF)
	if item.Index != CelestialSealItem || item.UID != uid {
		t.Fatalf("NewCelestialSeal perdeu identidade: %+v", item)
	}
	id, ok := CelestialSealID(item)
	if !ok || id != 0xBEEF {
		t.Fatalf("CelestialSealID=(%04X,%v), esperado BEEF,true", id, ok)
	}

	for _, mutate := range []func(*Item){
		func(it *Item) { it.Index++ },
		func(it *Item) { it.Eff[0] = 0 },
		func(it *Item) { it.Eff[2] = 0 },
		func(it *Item) { it.Eff[1], it.Eff[3] = 0, 0 },
	} {
		bad := item
		mutate(&bad)
		if id, ok := CelestialSealID(bad); ok || id != 0 {
			t.Fatalf("selo malformado foi aceito: %+v => %04X,%v", bad, id, ok)
		}
	}
}

func TestCelestialCapsuleValidateCoversIdentityEvolutionAndScoreContracts(t *testing.T) {
	tests := []struct {
		name   string
		mutate func(*CelestialCapsule)
	}{
		{"zero id", func(c *CelestialCapsule) { c.ID = 0 }},
		{"missing item uid", func(c *CelestialCapsule) { c.ItemUID = "" }},
		{"invalid item uid", func(c *CelestialCapsule) { c.ItemUID = "xyz" }},
		{"missing source uid", func(c *CelestialCapsule) { c.SourceUID = "" }},
		{"invalid source uid", func(c *CelestialCapsule) { c.SourceUID = "xyz" }},
		{"missing character name", func(c *CelestialCapsule) { c.Character.Name = "" }},
		{"missing character uid", func(c *CelestialCapsule) { c.Character.UID = "" }},
		{"source mismatch", func(c *CelestialCapsule) { c.Character.UID = "33333333333343338333333333333333" }},
		{"invalid evolution", func(c *CelestialCapsule) { c.Character.Evolution = "arch" }},
		{"nil score", func(c *CelestialCapsule) { c.Character.Extended = nil }},
		{"invalid score", func(c *CelestialCapsule) { c.Character.Extended.CurHP = 101 }},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			capsule := validCapsuleContract()
			tc.mutate(&capsule)
			if err := capsule.Validate(); err == nil {
				t.Fatal("capsula invalida foi aceita")
			}
		})
	}
}

func TestCelestialCapsuleValidateAcceptsNormalizedIdentityAndBothNativeEvolutions(t *testing.T) {
	for _, evolution := range []string{"celestial", " subcelestial "} {
		capsule := validCapsuleContract()
		capsule.Character.Evolution = evolution
		capsule.ItemUID = strings.ToUpper(capsule.ItemUID)
		capsule.SourceUID = strings.ToUpper(capsule.SourceUID)
		capsule.Character.UID = capsule.SourceUID
		if err := capsule.Validate(); err != nil {
			t.Fatalf("evolucao/identidade normalizavel %q foi recusada: %v", evolution, err)
		}
	}
}
