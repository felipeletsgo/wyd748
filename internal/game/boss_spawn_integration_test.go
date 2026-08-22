package game

import (
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func uint32Ptr(v uint32) *uint32 { return &v }

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
			Level:   10, MaxHP: 1000, CurHP: 1000,
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
			Level: uint32Ptr(200), MaxHP: uint32Ptr(50000),
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
		t.Fatalf("estado de spawn inesperado: states=%d mobs=%d", len(w.bossSpawns), len(w.mobs))
	}

	state := w.bossSpawns[0]
	first := w.mobsByID[state.mobID]
	if first == nil || w.bossFor(first.ID) == nil {
		t.Fatal("boss nao foi registrado no mundo e no runtime")
	}
	if first.Def.Name != "Configured Boss" || first.Def.Tipo != model.TipoMonstro {
		t.Fatalf("identidade do boss incorreta: nome=%q tipo=%q", first.Def.Name, first.Def.Tipo)
	}
	if first.HP != 50000 || first.Def.Score.Level != 200 ||
		first.Def.Score.Attack != 900 || first.Def.Score.Defense != 700 ||
		first.Def.Score.AttackRun != 6 || first.Def.ExpReward != 12345 || first.Def.Gold != 678 {
		t.Fatalf("overrides nao aplicados: mob=%+v extended=%+v", first, first.Def.Score)
	}
	if len(first.Def.Carry) != 0 || len(first.Def.Vende) != 0 {
		t.Fatal("boss herdou carry/vende do NPC base")
	}
	if w.npcs[0].Tipo != model.TipoNPC || w.npcs[0].Score.MaxHP != 1000 ||
		len(w.npcs[0].Carry) != 1 || len(w.npcs[0].Vende) != 1 {
		t.Fatal("configuracao do boss alterou o NPC base")
	}

	killed := w.onBossMobKilled(first)
	if killed != state || state.mobID != 0 {
		t.Fatal("morte nao desligou a instancia do boss")
	}
	if len(w.groundItems) != 3 {
		t.Fatalf("premiacao de area=%d, quer 3", len(w.groundItems))
	}
	wantRespawn := clock.Now().Add(5 * time.Second)
	if !state.respawnAt.Equal(wantRespawn) {
		t.Fatalf("deadline=%v, quer %v", state.respawnAt, wantRespawn)
	}
	w.removeMobInstance(first)

	w.tickBossRespawns(wantRespawn.Add(-time.Nanosecond))
	if state.mobID != 0 {
		t.Fatal("boss renasceu antes do prazo")
	}
	clock.Advance(5 * time.Second)
	w.tickBossRespawns(clock.Now())
	if state.mobID == 0 || state.mobID == first.ID {
		t.Fatalf("boss nao recebeu nova instancia/ID: antigo=%d novo=%d", first.ID, state.mobID)
	}
	if w.mobsByID[state.mobID] == nil || w.bossFor(state.mobID) == nil {
		t.Fatal("boss renascido nao foi indexado")
	}
}

func TestConfiguredBossRejectsMissingOrLegacyBase(t *testing.T) {
	w, _ := bossSpawnTestWorld()
	w.bossCatalog.Bosses[0].NPC = "Missing"
	if err := w.spawnConfiguredBosses(); err == nil || !strings.Contains(err.Error(), "nao existe") {
		t.Fatalf("NPC ausente deveria falhar claramente: %v", err)
	}

	w, _ = bossSpawnTestWorld()
	w.npcs[0].Score = nil
	if err := w.spawnConfiguredBosses(); err == nil || !strings.Contains(err.Error(), "sem score") {
		t.Fatalf("NPC sem extended deveria falhar claramente: %v", err)
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
		t.Fatalf("drop garantido/pilha incorretos: %+v", p.Char.Inv[0])
	}
	if p.Char.Inv[1].Index != 999 {
		t.Fatal("RNG fixo deveria aceitar o drop de 50%")
	}

	for i := range p.Char.Inv {
		p.Char.Inv[i] = model.Item{Index: uint16(1000 + i)}
	}
	w.rollBossDrops(p, mob, state)
	if len(w.groundItems) == 0 {
		t.Fatal("inventario cheio deveria derrubar o premio no chao")
	}
}

func TestSetItemAmountPreservesEffectsAndClamps(t *testing.T) {
	item := model.Item{Index: 1, Eff: [6]byte{10, 2}}
	setItemAmount(&item, 999)
	if item.Eff[0] != 10 || item.Eff[2] != effectAmount || item.Eff[3] != 255 {
		t.Fatalf("quantidade nao ocupou par livre/clampou: %v", item.Eff)
	}

	full := model.Item{Index: 2, Eff: [6]byte{1, 1, 2, 2, 3, 3}}
	before := full
	setItemAmount(&full, 10)
	if full != before {
		t.Fatal("item sem par livre perdeu outro efeito")
	}
	setItemAmount(nil, 10)
	setItemAmount(&full, 1)
}
