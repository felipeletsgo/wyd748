package model

import (
	"encoding/json"
	"testing"
)

func TestTowerRealmCaptureAndConditionalFame(t *testing.T) {
	r := GuildRegistry{Guilds: []Guild{{ID: 1, Kingdom: 7, Fame: 2500}, {ID: 2, Kingdom: 8, Fame: 10}, {ID: 3, Kingdom: 7, Fame: 200}}}
	r.Wars.Tower = TowerWarState{Kingdom: 7, InitialKingdom: 7, Owner: 1}
	r.Wars.Cities.Territories[0].Owner = 1
	r.Wars.Cities.Territories[1].Owner = 1
	r.Wars.Cities.Territories[2].Owner = 3
	if err := r.CaptureTower(3); err == nil {
		t.Fatal("same-realm capture accepted")
	}
	for _, owner := range []uint16{2, 1, 2, 1, 2} {
		if err := r.CaptureTower(owner); err != nil {
			t.Fatal(err)
		}
	}
	if r.Wars.Tower.DefenderCaptures != 3 || r.Wars.Tower.Captures != 5 {
		t.Fatal("defending recaptures counted incorrectly")
	}
	if err := r.ResolveTowerWar(); err != nil {
		t.Fatal(err)
	}
	if r.Guilds[0].Fame != 1834 || r.Guilds[1].Fame != 110 || r.Guilds[2].Fame != 0 || r.Wars.Tower.DefenderCaptures != 1 {
		t.Fatalf("incorrect winner/territory penalty: %+v", r.Guilds)
	}
	if err := r.ResolveTowerWar(); err != nil || r.Guilds[1].Fame != 110 {
		t.Fatal("result replay awarded fame twice", err)
	}
	if err := r.CaptureTower(1); err == nil {
		t.Fatal("capture after settlement accepted")
	}
	r.Wars.Tower = TowerWarState{Kingdom: 8, InitialKingdom: 8, Owner: 2}
	if err := r.ResolveTowerWar(); err != nil || r.Guilds[1].Fame != 110 {
		t.Fatal("unchanged realm received fame", err)
	}
}

func TestTowerRealmSurvivesGuildRemovalAndZeroRecaptureDivisor(t *testing.T) {
	r := GuildRegistry{Guilds: []Guild{{ID: 1, Kingdom: 7, Fame: 1100}, {ID: 2, Kingdom: 8}}}
	r.Wars.Tower = TowerWarState{Kingdom: 8, InitialKingdom: 7, Owner: 2}
	r.Wars.Cities.Territories[0].Owner = 1
	if err := r.ResolveTowerWar(); err != nil || r.Guilds[0].Fame != 100 || r.Guilds[1].Fame != 100 {
		t.Fatal("zero-recapture divisor must be one", err)
	}
	r.Guilds = r.Guilds[:1]
	r.Wars.Tower.Owner = 0
	if r.TowerKingdom() != 8 {
		t.Fatal("kingdom ownership depended on surviving guild")
	}
}

func TestTowerRealmPenaltyIncludesCastleAsFifthTerritory(t *testing.T) {
	r := GuildRegistry{Guilds: []Guild{{ID: 1, Kingdom: 7, Fame: 3000}, {ID: 2, Kingdom: 8}}}
	r.Wars.Tower = TowerWarState{Kingdom: 8, InitialKingdom: 7, Owner: 2, DefenderCaptures: 2}
	r.Wars.Cities.Territories[0].Owner, r.Wars.Castle.Owner = 1, 1
	if err := r.ResolveTowerWar(); err != nil || r.Guilds[0].Fame != 2000 || r.Guilds[1].Fame != 100 {
		t.Fatal("castle and city must each incur the defending-realm penalty", err)
	}
}

func TestWarMedalClaimsRoundTripValidationAndClone(t *testing.T) {
	r := &GuildRegistry{}
	claim := WarMedalClaim{Event: "tower/2026-10-19/capture/1", Account: "winner", CharacterUID: "00112233445566778899aabbccddeeff", Item: WarMedalGreen}
	r.Wars.Medals = []WarMedalClaim{claim}
	encoded, err := json.Marshal(r.Wars)
	if err != nil {
		t.Fatal(err)
	}
	var loaded GuildWarState
	if err := json.Unmarshal(encoded, &loaded); err != nil || loaded.Validate(r) != nil {
		t.Fatal("valid pending reward did not survive reload", err)
	}
	clone := loaded.Clone()
	clone.Medals[0].Item = WarMedalGold
	if loaded.Medals[0].Item != WarMedalGreen {
		t.Fatal("reward snapshots alias live claims")
	}
	for _, item := range []uint16{WarMedalGreen, WarMedalGold, WarMedalRed} {
		clone.Medals[0].Item = item
		if err := clone.Validate(r); err != nil {
			t.Fatal(err)
		}
	}
	clone.Medals = append(clone.Medals, claim)
	if clone.Validate(r) == nil {
		t.Fatal("duplicate event recipient accepted")
	}
	clone = loaded.Clone()
	clone.Medals[0].Item = 4906
	if clone.Validate(r) == nil {
		t.Fatal("superseded reward accepted")
	}
	// Fields absent in pre-upgrade JSON remain loadable.
	var old GuildWarState
	if err := json.Unmarshal([]byte(`{}`), &old); err != nil || old.Validate(r) != nil {
		t.Fatal("legacy state rejected", err)
	}
}
