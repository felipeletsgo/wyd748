package game

import (
	"encoding/binary"
	"strings"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

func uint32Ptr(v uint32) *uint32 { return &v }

func TestConfiguredBossDatasetPreservesFaces(t *testing.T) {
	npcs, err := data.LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	for _, npc := range npcs {
		face := npc.Equip.Rosto.Index
		if face == 0 {
			t.Errorf("NPC %q has no face", npc.Name)
		} else if _, ok := catalog.Items[face]; !ok {
			t.Errorf("NPC %q references unknown face %d", npc.Name, face)
		}
	}
	bosses, err := data.LoadBossCatalog("../../data/content/boss")
	if err != nil {
		t.Fatal(err)
	}
	w, _ := bossSpawnTestWorld()
	w.npcs, w.items, w.skills, w.bossCatalog = npcs, catalog.Items, catalog.Skills, bosses
	if err := w.spawnConfiguredBosses(); err != nil {
		t.Fatal(err)
	}
	want := map[string]uint16{
		"Ancient Necromancer": 73, "Black Archmage": 72,
		"Guardian of the Ruins": 399, "Demon Lord": 71,
	}
	if len(w.bossSpawns) != len(want) {
		t.Fatalf("spawned %d bosses, want %d", len(w.bossSpawns), len(want))
	}
	for _, state := range w.bossSpawns {
		mob := w.mobsByID[state.mobID]
		face, ok := want[state.config.Name]
		if mob == nil || !ok || mob.Def.Equip.Rosto.Index != face {
			t.Errorf("boss %q lost its expected face %d", state.config.Name, face)
			continue
		}
		anct := mob.Def.Equip.AncientCodes()
		packet := wire.CreateMobVisual(mob.ID, mob.Def.VisibleName(), mob.X, mob.Y,
			mob.Def.Mesh(), anct[:], mob.Def.MakeScore(mob.HP), nil, 0)
		if got := binary.LittleEndian.Uint16(packet[34:36]) & 0xFFF; got != face {
			t.Errorf("boss %q sends face %d, want %d", state.config.Name, got, face)
		}
		if state.config.Name == "Ancient Necromancer" &&
			mob.Def.Equip.Rosto.Eff != [6]byte{43, 7} {
			t.Error("Ancient Necromancer lost its original face refinement")
		}
	}
}

func bossSpawnTestWorld() (*World, *fakeClock) {
	clock := newFakeClock(time.Date(2026, 7, 26, 12, 0, 0, 0, time.UTC))
	w := newZoneTestWorld()
	w.clock = clock
	w.rng = fixedRNG{value: 0}
	w.nextMobID = firstMobID
	w.nextItemID = 10000
	w.items = map[uint16]model.ItemDef{
		420: {Index: 420, Name: "Laktorerium_Scrap"},
		421: {Index: 421, Name: "Boss_Trophy"},
	}
	w.npcs = []model.NPCDef{{
		Name: "Base_Boss", Tipo: model.TipoNPC,
		Score: &model.Score{
			Version: model.ScoreVersion,
			Level:   10, MaxHP: 1000, CurHP: 1000, Con: 1,
			Attack: 100, Defense: 50, AttackRun: 4,
		},
		Carry: []model.Item{{Index: 100}},
		Vende: []model.Item{{Index: 200}},
	}}
	w.bossCatalog = model.BossCatalog{Bosses: []model.BossConfig{{
		ID: "spawn_test", Name: "Configured Boss", NPC: "Base Boss",
		Type:  model.BossTypeChaser,
		Spawn: model.BossSpawn{X: 2100, Y: 2100, RespawnSeconds: 5},
		Stats: model.BossStats{
			Level: uint32Ptr(200), MaxHP: uint32Ptr(50000), Con: uint32Ptr(10000),
			Attack: uint32Ptr(900), Defense: uint32Ptr(700),
			AttackRun: uint32Ptr(6), ExpReward: uint32Ptr(12345), Gold: uint32Ptr(678),
		},
		Drops:        []model.BossDrop{{Item: 421, ChancePercent: 100, Amount: 7}},
		AreaReward:   model.BossAreaReward{Item: 420, Amount: 3},
		DeathMessage: "Boss defeated",
	}}}
	return w, clock
}

func TestConfiguredBossSpawnDeathAndRespawnLifecycle(t *testing.T) {
	w, clock := bossSpawnTestWorld()
	if err := w.spawnConfiguredBosses(); err != nil {
		t.Fatalf("spawnConfiguredBosses: %v", err)
	}
	if len(w.bossSpawns) != 1 || len(w.mobs) != 1 {
		t.Fatalf("unexpected spawn state: states=%d mobs=%d", len(w.bossSpawns), len(w.mobs))
	}

	state := w.bossSpawns[0]
	first := w.mobsByID[state.mobID]
	if first == nil || w.bossFor(first.ID) == nil {
		t.Fatal("boss was not registered in the world and the runtime")
	}
	if first.Def.Name != "Configured Boss" || first.Def.Tipo != model.TipoMonstro {
		t.Fatalf("wrong boss identity: name=%q type=%q", first.Def.Name, first.Def.Tipo)
	}
	if first.HP != 50000 || first.Def.Score.Level != 200 || first.Def.Score.Con != 10000 ||
		first.Def.Score.Attack != 900 || first.Def.Score.Defense != 700 ||
		first.Def.Score.AttackRun != 6 || first.Def.ExpReward != 12345 || first.Def.Gold != 678 {
		t.Fatalf("overrides not applied: mob=%+v extended=%+v", first, first.Def.Score)
	}
	if len(first.Def.Carry) != 0 || len(first.Def.Vende) != 0 {
		t.Fatal("boss inherited carry/vende from the base NPC")
	}
	if w.npcs[0].Tipo != model.TipoNPC || w.npcs[0].Score.MaxHP != 1000 ||
		w.npcs[0].Score.Con != 1 ||
		len(w.npcs[0].Carry) != 1 || len(w.npcs[0].Vende) != 1 {
		t.Fatal("boss configuration changed the base NPC")
	}

	killed := w.onBossMobKilled(first)
	if killed != state || state.mobID != 0 {
		t.Fatal("death did not detach the boss instance")
	}
	if len(w.groundItems) != 3 {
		t.Fatalf("area reward=%d, want 3", len(w.groundItems))
	}
	wantRespawn := clock.Now().Add(5 * time.Second)
	if !state.respawnAt.Equal(wantRespawn) {
		t.Fatalf("deadline=%v, want %v", state.respawnAt, wantRespawn)
	}
	w.removeMobInstance(first)

	w.tickBossRespawns(wantRespawn.Add(-time.Nanosecond))
	if state.mobID != 0 {
		t.Fatal("boss respawned before the deadline")
	}
	clock.Advance(5 * time.Second)
	w.tickBossRespawns(clock.Now())
	if state.mobID == 0 || state.mobID == first.ID {
		t.Fatalf("boss did not get a new instance/ID: old=%d new=%d", first.ID, state.mobID)
	}
	if w.mobsByID[state.mobID] == nil || w.bossFor(state.mobID) == nil {
		t.Fatal("respawned boss was not indexed")
	}
	if got := w.mobsByID[state.mobID].Def.Score.Con; got != 10000 {
		t.Fatalf("respawned boss CON=%d, want 10000", got)
	}
}

func TestConfiguredBossRejectsMissingOrLegacyBase(t *testing.T) {
	w, _ := bossSpawnTestWorld()
	w.bossCatalog.Bosses[0].NPC = "Missing"
	if err := w.spawnConfiguredBosses(); err == nil || !strings.Contains(err.Error(), "does not exist") {
		t.Fatalf("missing NPC should fail clearly: %v", err)
	}

	w, _ = bossSpawnTestWorld()
	w.npcs[0].Score = nil
	if err := w.spawnConfiguredBosses(); err == nil || !strings.Contains(err.Error(), "has no score") {
		t.Fatalf("an NPC without extended data should fail clearly: %v", err)
	}
}

func TestConfiguredBossRejectsUnknownRuntimeReferencesBeforeMutation(t *testing.T) {
	tests := []struct {
		name   string
		mutate func(*model.BossConfig)
		want   string
	}{
		{"skill", func(c *model.BossConfig) {
			c.Skills = []model.BossSkill{{ID: 9999, CooldownSeconds: 1, Range: 1}}
		}, "unknown SkillData index"},
		{"summon", func(c *model.BossConfig) {
			c.Summons = []model.BossSummon{{NPC: "Missing Add", Count: 1, CooldownSeconds: 1}}
		}, "unknown NPC"},
		{"drop", func(c *model.BossConfig) {
			c.Drops = []model.BossDrop{{Item: 999, ChancePercent: 100}}
		}, "unknown item"},
		{"area reward", func(c *model.BossConfig) {
			c.AreaReward = model.BossAreaReward{Item: 999, Amount: 1}
		}, "area reward references unknown item"},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w, _ := bossSpawnTestWorld()
			tc.mutate(&w.bossCatalog.Bosses[0])
			if err := w.spawnConfiguredBosses(); err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("invalid %s reference was accepted: %v", tc.name, err)
			}
			if len(w.bossSpawns) != 0 || len(w.mobs) != 0 {
				t.Fatalf("invalid %s reference partially mutated the world: states=%d mobs=%d",
					tc.name, len(w.bossSpawns), len(w.mobs))
			}
		})
	}
}

func TestBossRegisterFailureLeavesNoPublishedOrphan(t *testing.T) {
	w, _ := bossSpawnTestWorld()
	p, session := networkedTestPlayer(1, "Observer", 2100, 2100)
	w.players = map[*gameNet.Session]*Player{session: p}
	w.playersByID = map[uint16]*Player{p.ID: p}
	w.updatePlayerSpatial(p)
	state := &bossSpawnState{config: w.bossCatalog.Bosses[0], def: &w.npcs[0]}
	before := session.QueuedPacketsForTest()
	if err := w.spawnBoss(state); err == nil {
		t.Fatal("nil boss profile should fail registration")
	}
	if len(w.mobs) != 0 || len(w.mobsByID) != 0 || state.mobID != 0 {
		t.Fatalf("failed boss left an orphan: mobs=%d index=%d state=%d",
			len(w.mobs), len(w.mobsByID), state.mobID)
	}
	if session.QueuedPacketsForTest() != before {
		t.Fatal("boss was published before registration succeeded")
	}
}

func TestBossDropsApplyChanceAmountAndInventoryFallback(t *testing.T) {
	w, _ := bossSpawnTestWorld()
	_, p, _ := handlerTestWorld(t)
	w.players[p.Session] = p
	w.playersByID[p.ID] = p
	w.store = &craftStore{}
	state := &bossSpawnState{config: w.bossCatalog.Bosses[0]}
	mob := &Mob{ID: 1200, X: 2100, Y: 2100, Def: bossTestNPC("Boss", 100)}

	state.config.Drops = append(state.config.Drops,
		model.BossDrop{Item: 999, ChancePercent: 50, Amount: 1})
	w.rollBossDrops(p, mob, state)
	if p.Char.Inv[0].Index != 421 || itemStackAmount(p.Char.Inv[0]) != 7 {
		t.Fatalf("guaranteed drop or stack amount is incorrect: %+v", p.Char.Inv[0])
	}
	if p.Char.Inv[1].Index != 0 {
		t.Fatal("an unknown item must not enter the inventory")
	}

	for i := range p.Char.Inv {
		p.Char.Inv[i] = model.Item{Index: uint16(1000 + i)}
	}
	w.rollBossDrops(p, mob, state)
	if len(w.groundItems) == 0 {
		t.Fatal("a full inventory should drop the reward on the ground")
	}
}

func TestSetItemAmountPreservesEffectsAndClamps(t *testing.T) {
	item := model.Item{Index: 1, Eff: [6]byte{10, 2}}
	setItemAmount(&item, 999)
	if item.Eff[0] != 10 || item.Eff[2] != effectAmount || item.Eff[3] != 255 {
		t.Fatalf("amount did not take a free pair or was not clamped: %v", item.Eff)
	}

	full := model.Item{Index: 2, Eff: [6]byte{1, 1, 2, 2, 3, 3}}
	before := full
	setItemAmount(&full, 10)
	if full != before {
		t.Fatal("item without a free pair lost another effect")
	}
	setItemAmount(nil, 10)
	setItemAmount(&full, 1)
}
