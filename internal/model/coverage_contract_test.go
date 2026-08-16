package model

import (
	"testing"
	"time"
)

func loadedTestTerrain() TerrainMap {
	return TerrainMap{
		Height:    make([]byte, TerrainCells),
		Attribute: make([]byte, AttributeCells),
	}
}

func blockTestTerrainCell(m *TerrainMap, x, y uint16) {
	m.Attribute[(int(y)/4)*AttributeWidth+int(x)/4] |= 0x02
}

func TestMountModelCoversAllStagesAndStateEncoding(t *testing.T) {
	catalog := MountCatalog{Types: map[int]MountStats{3: {Attack: 10, Name: "test"}}}
	if stats, ok := catalog.Stats(3); !ok || stats.Attack != 10 || stats.Name != "test" {
		t.Fatalf("catalogo de montaria nao resolveu tipo: %+v ok=%v", stats, ok)
	}
	if _, ok := (MountCatalog{}).Stats(3); ok {
		t.Fatal("catalogo vazio resolveu tipo inexistente")
	}

	for stage, index := range map[string]uint16{
		"egg": MountEggBase + 3, "baby": MountBabyBase + 3, "adult": MountAdultBase + 3,
	} {
		if got := MountType(index); got != 3 {
			t.Fatalf("%s: MountType=%d, esperado 3", stage, got)
		}
	}
	if MountType(MountEggBase-1) != -1 || MountType(MountAdultBase+MountTypeCount) != -1 {
		t.Fatal("indice fora das faixas foi classificado como montaria")
	}
	if !IsMountEgg(MountEggBase) || IsMount(MountEggBase) {
		t.Fatal("ovo deve ser reconhecido sem contar como montaria equipada")
	}
	if !IsMountBaby(MountBabyBase) || !IsMount(MountBabyBase) {
		t.Fatal("cria nao foi reconhecida como montaria")
	}
	if !IsMountAdult(MountAdultBase) || !IsMount(MountAdultBase) {
		t.Fatal("adulta nao foi reconhecida como montaria")
	}

	var item Item
	item.SetMountHP(30000)
	item.SetMountLevel(120)
	item.SetMountLongev(60)
	item.SetMountFood(100)
	if item.MountHP() != 30000 || item.MountLevel() != 120 || item.MountLongev() != 60 || item.MountFood() != 100 {
		t.Fatalf("round-trip do estado da montaria falhou: eff=%v", item.Eff)
	}
	item.SetMountHP(-1)
	item.SetMountLevel(-1)
	item.SetMountLongev(999)
	item.SetMountFood(999)
	if item.MountHP() != 0 || item.MountLevel() != 0 || item.MountLongev() != 255 || item.MountFood() != 255 {
		t.Fatalf("clamp dos campos de montaria falhou: eff=%v", item.Eff)
	}
	item.SetMountHP(70000)
	if item.MountHP() != 65535 {
		t.Fatalf("HP de montaria nao saturou em uint16: %d", item.MountHP())
	}
}

func TestTerrainLineOfSightRejectsWallsCornersAndHeightJumps(t *testing.T) {
	if !(TerrainMap{}).LineOfSight(10, 10, 30, 30) {
		t.Fatal("mapa nao carregado deveria conservar LOS permissiva")
	}

	m := loadedTestTerrain()
	if !m.LineOfSight(100, 100, 108, 100) {
		t.Fatal("linha reta limpa foi bloqueada")
	}

	blocked := loadedTestTerrain()
	blocked.Height[100*TerrainWidth+104] = TerrainBlockedByte
	if blocked.LineOfSight(100, 100, 108, 100) {
		t.Fatal("LOS atravessou HeightMap bloqueado")
	}

	height := loadedTestTerrain()
	height.Height[100*TerrainWidth+104] = RouteHeightRange
	if height.LineOfSight(100, 100, 108, 100) {
		t.Fatal("LOS atravessou salto de altura >= MH=8")
	}

	// Para testar canto fechado sem o coarse AttributeMap 4x4 bloquear a origem,
	// use HeightMap nos dois vizinhos ortogonais da diagonal.
	corner := loadedTestTerrain()
	corner.Height[100*TerrainWidth+101] = TerrainBlockedByte
	corner.Height[101*TerrainWidth+100] = TerrainBlockedByte
	if corner.LineOfSight(100, 100, 101, 101) {
		t.Fatal("LOS atravessou diagonal fechada por duas paredes")
	}

	attribute := loadedTestTerrain()
	blockTestTerrainCell(&attribute, 108, 100)
	if attribute.LineOfSight(100, 100, 108, 100) {
		t.Fatal("LOS terminou em bloco CantGo")
	}
}

func TestVolatileCatalogRulePrecedenceAndFallback(t *testing.T) {
	defaultRule := VolatileRule{Action: "default"}
	codeRule := VolatileRule{Action: "code"}
	itemRule := VolatileRule{Action: "item"}
	catalog := VolatileCatalog{
		Default: defaultRule,
		Rules:   map[int]VolatileRule{7: codeRule},
		Items:   map[uint16]VolatileRule{100: itemRule},
		ItemCodes: map[uint16]int{
			100: 7,
			101: 7,
			102: 8,
		},
	}

	if rule, code, ok := catalog.Rule(100); !ok || code != 7 || rule.Action != "item" {
		t.Fatalf("override por item perdeu precedencia: %+v code=%d ok=%v", rule, code, ok)
	}
	if rule, code, ok := catalog.Rule(101); !ok || code != 7 || rule.Action != "code" {
		t.Fatalf("regra por codigo nao resolveu: %+v code=%d ok=%v", rule, code, ok)
	}
	if rule, code, ok := catalog.Rule(102); !ok || code != 8 || rule.Action != "default" {
		t.Fatalf("fallback default nao resolveu: %+v code=%d ok=%v", rule, code, ok)
	}
	if _, _, ok := catalog.Rule(999); ok {
		t.Fatal("item sem EF_VOLATILE ganhou regra")
	}
}

func TestBossCatalogValidatesEntriesDuplicatesAndRespawn(t *testing.T) {
	valid := BossConfig{
		ID: "audit-boss", NPC: "Scorpion", Type: BossTypeChaser,
		Spawn: BossSpawn{X: 100, Y: 100, RespawnSeconds: 12},
	}
	if err := valid.Validate(); err != nil {
		t.Fatalf("boss minimo valido recusado: %v", err)
	}
	if got := valid.RespawnDelay(); got != 12*time.Second {
		t.Fatalf("respawn=%v, esperado 12s", got)
	}
	valid.Spawn.RespawnSeconds = 0
	if valid.RespawnDelay() != 0 {
		t.Fatal("respawn zero deveria significar sem renascimento")
	}

	catalog := BossCatalog{Bosses: []BossConfig{
		{ID: "same", NPC: "Scorpion", Type: BossTypeChaser, Spawn: BossSpawn{X: 1, Y: 1}, SourceFile: "a.lua"},
		{ID: "same", NPC: "Scorpion", Type: BossTypeChaser, Spawn: BossSpawn{X: 2, Y: 2}, SourceFile: "b.lua"},
	}}
	if err := catalog.Validate(); err == nil {
		t.Fatal("catalogo aceitou ID de boss duplicado")
	}

	invalid := BossCatalog{Bosses: []BossConfig{{
		ID: "caster", NPC: "Scorpion", Type: BossTypeCaster, Spawn: BossSpawn{X: 1, Y: 1},
	}}}
	if err := invalid.Validate(); err == nil {
		t.Fatal("catalogo aceitou caster sem skill")
	}
}
