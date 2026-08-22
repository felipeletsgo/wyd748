package game

import (
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

func minimalWorldInputs() ([]model.NPCDef, []model.NPCGener, model.Catalog, model.CharacterTemplateFile) {
	npc := model.NPCDef{
		Name: "Test_Mob", Tipo: model.TipoMonstro,
		Score: &model.Score{
			Version: model.ScoreVersion,
			Level:   1, MaxHP: 100, CurHP: 100, Attack: 10, Defense: 2, AttackRun: 4,
		},
	}
	gener := model.NPCGener{
		Index: 7, Enabled: true, MinuteGenerate: 2, MaxNumMob: 2,
		MinGroup: 1, MaxGroup: 1, Leader: "Test Mob", Follower: "Test_Mob",
		Segments: [model.MaxGenerSegments]model.GenerSegment{{
			X: 2100, Y: 2100, Range: 0, Wait: 1,
		}},
	}
	catalog := model.Catalog{
		Items:  map[uint16]model.ItemDef{746: {Index: 746, Name: "Cannon"}},
		Skills: map[int]model.SkillDef{},
	}
	templates := model.CharacterTemplateFile{
		Spawn: model.CharacterSpawn{X: 2100, Y: 2100},
		Classes: []model.CharacterTemplate{{
			Name: "TransKnight", Class: 0,
			Score: model.Score{
				Version: model.ScoreVersion, MaxHP: 100, CurHP: 100,
			},
		}},
	}
	return []model.NPCDef{npc}, []model.NPCGener{gener}, catalog, templates
}

func TestNewWorldWiresOptionsAndSpawnsConfiguredContent(t *testing.T) {
	npcs, geners, catalog, templates := minimalWorldInputs()
	clock := newFakeClock(time.Date(2026, 7, 26, 15, 0, 0, 0, time.UTC))
	gameplay := model.DefaultGameplayConfig()
	gameplay.EXPMinimum = 25_000
	zones := model.QuestZoneFile{Zones: []model.QuestZone{{
		Name: "Arena", X1: 2000, Y1: 2000, X2: 2200, Y2: 2200,
	}}}
	teleports := []model.Teleport{{
		SourceX: 2100, SourceY: 2100, DestX: 2200, DestY: 2200, Name: "Portal",
	}}
	mounts := model.MountCatalog{Types: map[int]model.MountStats{0: {Attack: 10}}}
	initItems := []model.InitItem{{Index: 746, X: 2105, Y: 2105, Rotate: 3}}

	w, err := NewWorld(
		&craftStore{}, npcs, geners, catalog, [model.MaxCarry]int{},
		model.VolatileCatalog{}, templates, model.TerrainMap{},
		WithClock(clock),
		WithRNG(fixedRNG{value: 0}),
		WithQuests(model.QuestFile{}),
		WithQuestZones(zones),
		WithInitItems(initItems),
		WithNPCGenerLog("quiet"),
		WithTeleports(teleports),
		WithGameplayConfig(gameplay),
		WithMounts(mounts),
		WithBossCatalog(model.BossCatalog{}),
	)
	if err != nil {
		t.Fatalf("NewWorld: %v", err)
	}
	if len(w.generators) != 1 || len(w.mobs) != 2 {
		t.Fatalf("NPCGener nao materializado: generators=%d mobs=%d", len(w.generators), len(w.mobs))
	}
	if w.generators[0].current != 2 ||
		!w.generators[0].nextGenerate.Equal(clock.Now().Add(2*npcGenerMinute)) {
		t.Fatalf("estado/deadline do gerador incorreto: %+v", w.generators[0])
	}
	if len(w.groundItems) != 1 || w.groundItems[15001] == nil ||
		!w.groundItems[15001].Permanent || w.groundItems[15001].Rotate != 3 {
		t.Fatalf("objeto permanente/canhao nao inicializado: %+v", w.groundItems)
	}
	if len(w.questZones) != 1 || len(w.teleports) != 1 ||
		w.gameplay.EXPMinimum != 25_000 || w.mounts.Types[0].Attack != 10 {
		t.Fatal("options do mundo nao foram preservadas")
	}
	if w.npcGenerLogMode != npcGenerLogQuiet ||
		!w.nextAutoSave.Equal(clock.Now().Add(accountAutoSaveSliceInterval)) ||
		!w.nextQuestZoneReset.Equal(clock.Now().Add(questZoneResetInterval)) {
		t.Fatal("modo de log ou deadlines nao usam a configuracao/relogio injetados")
	}
	if w.charTemplates[0].Name != "TransKnight" || w.charSpawn.X != 2100 {
		t.Fatal("layout de personagem nao foi conectado ao mundo")
	}
}

func TestNewWorldRejectsInvalidGlobalConfigAndMissingGeneratorTemplate(t *testing.T) {
	npcs, geners, catalog, templates := minimalWorldInputs()
	bad := model.DefaultGameplayConfig()
	bad.PartyEXPBonusPercent = model.MaxPartyEXPBonusPercent + 1
	if _, err := NewWorld(
		&craftStore{}, npcs, nil, catalog, [model.MaxCarry]int{},
		model.VolatileCatalog{}, templates, model.TerrainMap{},
		WithGameplayConfig(bad),
	); err == nil || !strings.Contains(err.Error(), "configuracao global") {
		t.Fatalf("configuracao invalida deveria derrubar boot: %v", err)
	}

	geners[0].Follower = "Does_Not_Exist"
	if _, err := NewWorld(
		&craftStore{}, npcs, geners, catalog, [model.MaxCarry]int{},
		model.VolatileCatalog{}, templates, model.TerrainMap{},
	); err == nil || !strings.Contains(err.Error(), "template ausente") {
		t.Fatalf("template ausente deveria derrubar boot: %v", err)
	}
}

func TestWorldIDAllocationWrapsAndReusesOnlyFreeSlots(t *testing.T) {
	w := newZoneTestWorld()
	w.nextMobID = ^uint16(0)
	occupied := &Mob{ID: ^uint16(0), Def: bossTestNPC("Occupied", 1)}
	w.registerMobSpatial(occupied)
	if got := w.allocMobID(); got != firstMobID {
		t.Fatalf("wrap de mob ID=%d, quer %d", got, firstMobID)
	}

	for id := uint16(1); id < 4; id++ {
		p := addZonePlayer(w, id, 2100+id, 2100, 1)
		p.ID = id
	}
	if got, ok := w.allocPlayerID(); !ok || got != 4 {
		t.Fatalf("menor player ID livre=%d, quer 4", got)
	}
}

func TestWorldIDAllocationRejectsFullPlayerWorld(t *testing.T) {
	w := newZoneTestWorld()
	for id := uint16(1); id < 1000; id++ {
		p := &Player{ID: id, Session: net.NewTestSession(int64(id), 1)}
		w.playersByID[id] = p
		w.players[p.Session] = p
	}
	if got, ok := w.allocPlayerID(); ok || got != 0 {
		t.Fatalf("mundo cheio deveria recusar alocacao: id=%d ok=%v", got, ok)
	}
}

func TestWorldIDAllocationHonorsPlayerIndexWhenSessionMapIsStale(t *testing.T) {
	w := newZoneTestWorld()
	w.playersByID[1] = &Player{ID: 1}
	if got, ok := w.allocPlayerID(); !ok || got != 2 {
		t.Fatalf("indice playersByID deveria reservar 1: id=%d ok=%v", got, ok)
	}
}

func TestWorldGeneratorSchedulingAndWalkableFallbacks(t *testing.T) {
	w := newZoneTestWorld()
	now := time.Date(2026, 7, 26, 16, 0, 0, 0, time.UTC)
	g := &generState{def: model.NPCGener{MinuteGenerate: 3}}
	w.scheduleGenerator(g, now)
	if !g.nextGenerate.Equal(now.Add(3 * npcGenerMinute)) {
		t.Fatalf("schedule=%v", g.nextGenerate)
	}
	g.def.MinuteGenerate = 0
	g.nextGenerate = time.Time{}
	w.scheduleGenerator(g, now)
	if !g.nextGenerate.IsZero() {
		t.Fatal("gerador sem intervalo recebeu deadline")
	}

	if got := generName("Mestre_Archi"); got != "Mestre Archi" {
		t.Fatalf("normalizacao de nome=%q", got)
	}
	for i := 0; i < 20; i++ {
		x, y := (&World{}).scatter(100, 100, 0)
		if x != 100 || y != 100 {
			t.Fatalf("scatter sem raio moveu para (%d,%d)", x, y)
		}
	}
	x, y := w.findWalkablePosition(100, 100, 1)
	if x == 0 || y == 0 {
		t.Fatal("fallback de terreno devolveu coordenada invalida")
	}
}
