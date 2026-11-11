package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

func TestVersionedGuildKeysSortDeduplicateAndResolveVersions(t *testing.T) {
	w := &World{guildVersions: map[uint16]int64{0: 4, 3: 9, 4: 2}}
	selected := w.versionedGuildKeys(criticalWrite{guildKeys: []store.GuildParticipant{
		{GuildID: 3}, {GuildID: 2}, {GuildID: 3},
	}})
	wantSelected := []store.GuildParticipant{{GuildID: 2, Version: -1}, {GuildID: 3, Version: 9}}
	if len(selected) != len(wantSelected) {
		t.Fatalf("selected guild keys = %+v", selected)
	}
	for i := range wantSelected {
		if selected[i] != wantSelected[i] {
			t.Fatalf("selected guild keys = %+v, want %+v", selected, wantSelected)
		}
	}

	whole := w.versionedGuildKeys(criticalWrite{guilds: &model.GuildRegistry{Guilds: []model.Guild{{ID: 3}, {ID: 5}}}})
	wantWhole := []store.GuildParticipant{
		{GuildID: 0, Version: 4}, {GuildID: 3, Version: 9}, {GuildID: 4, Version: 2}, {GuildID: 5, Version: -1},
	}
	if len(whole) != len(wantWhole) {
		t.Fatalf("whole-registry guild keys = %+v", whole)
	}
	for i := range wantWhole {
		if whole[i] != wantWhole[i] {
			t.Fatalf("whole-registry guild keys = %+v, want %+v", whole, wantWhole)
		}
	}
}

func TestCityWarLocalPreservesInstantAndUsesConfiguredZone(t *testing.T) {
	now := time.Date(2026, time.June, 8, 12, 30, 0, 0, time.UTC)
	if got := (*World)(nil).cityWarLocal(now); !got.Equal(now) {
		t.Fatalf("nil world changed city-war time to %v", got)
	}
	if got := (&World{}).cityWarLocal(now); !got.Equal(now) || got.Location() != time.UTC {
		t.Fatalf("default city-war time = %v", got)
	}
	zone := time.FixedZone("game-local", -3*60*60)
	got := (&World{warLocation: zone}).cityWarLocal(now)
	if !got.Equal(now) || got.Location() != zone {
		t.Fatalf("localized city-war time = %v, want same instant in %v", got, zone)
	}
}

func TestRecoveryWorldsReturnsSingleWorldOrClusterMembers(t *testing.T) {
	w := &World{}
	if got := w.recoveryWorlds(); len(got) != 1 || got[0] != w {
		t.Fatalf("single-world recovery scope = %+v", got)
	}
	one, two := &World{channel: 1}, &World{channel: 2}
	cluster := &ChannelCluster{worlds: []*World{one, two}}
	if got := (&World{cluster: cluster}).recoveryWorlds(); len(got) != 2 || got[0] != one || got[1] != two {
		t.Fatalf("cluster recovery scope = %+v", got)
	}
}

func TestGhostShopPacketHelpersHandleNilAndBuildOwnerPackets(t *testing.T) {
	if ghostShopOwnerResetPackets(nil) != nil || ghostShopOwnerResetPackets(&Player{}) != nil {
		t.Fatal("owner reset packets were built without a character")
	}
	ownerPackets := ghostShopOwnerResetPackets(&Player{ID: 7, Char: &model.Char{Score: &model.Score{Version: model.ScoreVersion}}})
	if len(ownerPackets) != 3 {
		t.Fatalf("owner reset packet count = %d, want 3", len(ownerPackets))
	}
	for i, packet := range ownerPackets {
		if len(packet) == 0 {
			t.Errorf("owner reset packet %d is empty", i)
		}
	}
	if ghostShopTradeListPacket(nil) != nil {
		t.Fatal("nil shop produced a trade-list packet")
	}
	if packet := ghostShopTradeListPacket(&GhostShop{ID: 88, Title: "Test Shop"}); len(packet) == 0 {
		t.Fatal("valid shop produced an empty trade-list packet")
	}
}

func TestArchCrystalStepCoversSequenceAndBounds(t *testing.T) {
	for _, tt := range []struct {
		index uint16
		step  byte
		ok    bool
	}{
		{index: firstArchCrystal - 1},
		{index: firstArchCrystal, step: 1, ok: true},
		{index: firstArchCrystal + archCrystalCount/2, step: byte(archCrystalCount/2 + 1), ok: true},
		{index: firstArchCrystal + archCrystalCount - 1, step: byte(archCrystalCount), ok: true},
		{index: firstArchCrystal + archCrystalCount},
	} {
		step, ok := archCrystalStep(tt.index)
		if step != tt.step || ok != tt.ok {
			t.Errorf("archCrystalStep(%d) = %d,%t, want %d,%t", tt.index, step, ok, tt.step, tt.ok)
		}
	}
}

func TestPlayerElementalResistsUsesEffectiveRuntimeScore(t *testing.T) {
	if got := playerElementalResists(nil); got != (model.ElementalResists{}) {
		t.Fatalf("nil character resists = %+v", got)
	}
	character := &model.Char{
		Score:        &model.Score{Version: model.ScoreVersion},
		RuntimeScore: &model.Score{Version: model.ScoreVersion, ResistFire: 11, ResistIce: 22, ResistHoly: 33, ResistThunder: 44},
	}
	want := model.ElementalResists{Fire: 11, Ice: 22, Sacred: 33, Thunder: 44}
	if got := playerElementalResists(character); got != want {
		t.Fatalf("elemental resists = %+v, want %+v", got, want)
	}
}

func TestSetOwnedAffectForPlayerAtRequiresOwnerIdentity(t *testing.T) {
	now := time.Date(2026, time.June, 8, 12, 30, 0, 0, time.UTC)
	target := &model.Char{}
	if setOwnedAffectForPlayerAt(target, nil, 1, 10, 1, 1, now) ||
		setOwnedAffectForPlayerAt(target, &Player{}, 1, 10, 1, 1, now) ||
		setOwnedAffectForPlayerAt(target, &Player{Char: &model.Char{UID: "  "}}, 1, 10, 1, 1, now) {
		t.Fatal("affect accepted an owner without persistent character identity")
	}
	owner := &Player{ID: 42, Char: &model.Char{UID: " owner-uid "}}
	if setOwnedAffectForPlayerAt(target, owner, 0, 10, 1, 1, now) {
		t.Fatal("affect with an invalid type was accepted")
	}
	if !setOwnedAffectForPlayerAt(target, owner, 12, 10, 1, 0, now) {
		t.Fatal("valid owned affect was rejected")
	}
	if affect := target.Affects[0]; affect.Type != 12 || affect.OwnerID != owner.ID || affect.OwnerCharacterUID != "owner-uid" ||
		!affect.ExpiresAt.Equal(now.Add(8*time.Second)) {
		t.Fatalf("owned affect = %+v", affect)
	}
}

func TestOwnedMobAffectsKeepPlayerOrMobSource(t *testing.T) {
	now := time.Date(2026, time.June, 8, 12, 30, 0, 0, time.UTC)
	playerOwner := &Player{ID: 17, Char: &model.Char{UID: " stable-character "}}
	mob := &Mob{ID: 28}
	if setPlayerOwnedMobAffectAt(mob, nil, 13, 10, 1, 2, now) ||
		setPlayerOwnedMobAffectAt(mob, &Player{Char: &model.Char{}}, 13, 10, 1, 2, now) ||
		setPlayerOwnedMobAffectAt(mob, playerOwner, 0, 10, 1, 2, now) {
		t.Fatal("mob affect accepted an invalid player source")
	}
	if !setPlayerOwnedMobAffectAt(mob, playerOwner, 13, 10, 1, 2, now) {
		t.Fatal("mob affect rejected a valid player source")
	}
	if affect := mob.Affects[0]; affect.OwnerID != playerOwner.ID || affect.OwnerCharacterUID != "stable-character" {
		t.Fatalf("player-owned mob affect = %+v", affect)
	}

	mobOwner := &Mob{ID: 29}
	if setMobOwnedMobAffectAt(mob, nil, 14, 20, 1, 1, now) ||
		setMobOwnedMobAffectAt(mob, &Mob{}, 14, 20, 1, 1, now) {
		t.Fatal("mob affect accepted a mob source without an ID")
	}
	if !setMobOwnedMobAffectAt(mob, mobOwner, 14, 20, 1, 1, now) {
		t.Fatal("mob affect rejected a valid mob source")
	}
	if affect := mob.Affects[1]; affect.OwnerID != mobOwner.ID || affect.OwnerCharacterUID != "" {
		t.Fatalf("mob-owned mob affect = %+v", affect)
	}
}

func TestBossEffectActionsApplyAndRemoveAffects(t *testing.T) {
	w := &World{}
	boss := &BossRuntime{Profile: &BossProfile{ID: "coverage-boss"}}
	mob := &Mob{ID: 31}
	action := BossActionDef{AffectType: 19, AffectValue: 25, AffectLevel: 1, AffectDuration: 2, Message: "The boss is protected."}

	w.bossApplyEffect(boss, &Mob{}, action)
	if mob.Affects[0].Type != 0 {
		t.Fatal("boss affect applied without an owner mob ID")
	}
	w.bossApplyEffect(boss, mob, action)
	if affect := mob.Affects[0]; affect.Type != action.AffectType || affect.OwnerID != mob.ID || affect.Value != action.AffectValue {
		t.Fatalf("boss affect = %+v", affect)
	}

	w.bossRemoveEffect(boss, mob, BossActionDef{AffectType: 18})
	if mob.Affects[0].Type != action.AffectType {
		t.Fatal("removing a missing boss affect changed the mob")
	}
	w.bossRemoveEffect(boss, mob, action)
	if mob.Affects[0].Type != 0 {
		t.Fatalf("boss affect remained after removal: %+v", mob.Affects[0])
	}
}

func TestPlayerAppearancePacketRequiresCharacter(t *testing.T) {
	if playerAppearancePacket(nil) != nil || playerAppearancePacket(&Player{}) != nil {
		t.Fatal("appearance packet was built without a character")
	}
	player, _ := networkedTestPlayer(7, "Appearance", 2100, 2100)
	if packet := playerAppearancePacket(player); len(packet) == 0 {
		t.Fatal("valid player produced an empty appearance packet")
	}
}

func TestFindMobPositionOccupiedUsesTheRequestedGameplaySpace(t *testing.T) {
	player, _ := networkedTestPlayer(1, "Global", 2200, 2200)
	w := worldWithNetworkedPlayers(player)
	if !w.findMobPositionOccupied("", player.X, player.Y) {
		t.Fatal("global position ignored its player occupant")
	}
	if w.findMobPositionOccupied("private-room", player.X, player.Y) {
		t.Fatal("private runtime treated a global player as its occupant")
	}
	if w.findMobPositionOccupied("", player.X+1, player.Y) {
		t.Fatal("empty position reported an occupant")
	}
}
