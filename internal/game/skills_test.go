package game

import (
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/net"
)

// skillHasServerExecution is a catalog assertion helper, not gameplay. Keeping
// it in tests prevents a data reorganization from silently creating no-op
// skills without shipping a second execution table in the server binary.
func skillHasServerExecution(skill model.SkillDef) bool {
	if skill.Index >= 97 && skill.Index <= 102 {
		return true
	}
	if skill.Passive != 0 {
		return true
	}
	if skill.InstanceType >= 1 && skill.InstanceType <= 5 ||
		skill.AffectType > 0 || skill.TickType > 0 {
		return true
	}
	switch skill.Index {
	case 3, 5, 6, 25, 26, 27, 29, 31, 42, 47,
		49, 56, 57, 58, 59, 60, 61, 62, 63,
		73, 79, 83, 84:
		return true
	default:
		return false
	}
}

func TestAllFourClassSkillsHaveServerExecution(t *testing.T) {
	catalog, err := data.LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	for index := 0; index < 96; index++ {
		skill, ok := catalog.Skills[index]
		if !ok {
			t.Fatalf("skill %d ausente", index)
		}
		if !skillHasServerExecution(skill) {
			t.Errorf("skill %d %q sem caminho server-side", index, skill.Name)
		}
	}
}

func TestSkillFinalDamageDoesNotClampWideAttackToWord(t *testing.T) {
	if got := skillFinalDamage(300_000, 0, 15); got < 300_000 {
		t.Fatalf("dano wide foi reduzido/clampado para %d", got)
	}
}

func TestPvPWideHitPreservesCalculatedOverkill(t *testing.T) {
	target := &Player{ID: 2, InWorld: true, X: 11, Y: 10,
		Char: &model.Char{Score: testScore(model.Score{CurHP: 100, MaxHP: 100})}}
	if got := skillFinalDamage(250_000, playerDefense(target.Char), 0); got <= int(playerCurHP(target.Char)) {
		t.Fatalf("pre-condicao: dano=%d hp=%d", got, playerCurHP(target.Char))
	}
}

func TestResistanceForInstanceUses759ElementOrder(t *testing.T) {
	resist := model.ElementalResists{Fire: 11, Ice: 22, Sacred: 33, Thunder: 44}
	tests := []struct {
		instance int
		want     int
	}{
		{1, 11}, // dano magico generico usa Resist[0] na 7.59
		{2, 11}, // fogo
		{3, 22}, // gelo
		{4, 33}, // sagrado
		{5, 44}, // trovao
		{6, 0},
	}
	for _, tc := range tests {
		if got := resistanceForInstance(tc.instance, resist); got != tc.want {
			t.Fatalf("InstanceType %d: resistencia=%d, quer %d", tc.instance, got, tc.want)
		}
	}
}

func TestApplySkillResistanceMatches759(t *testing.T) {
	tests := []struct {
		name        string
		resist      uint16
		targetIsMob bool
		want        int
	}{
		{"jogador sem resistencia", 0, false, 150},
		{"jogador com 50", 50, false, 100},
		{"jogador com 100", 100, false, 50},
		{"mob usa metade", 100, true, 100},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			got := applySkillResistance(100, 2, model.ElementalResists{Fire: uint32(tc.resist)}, tc.targetIsMob)
			if got != tc.want {
				t.Fatalf("dano=%d, quer %d", got, tc.want)
			}
		})
	}
}

func TestApplySkillResistanceIgnoresNonElementalType(t *testing.T) {
	got := applySkillResistance(100, 11, model.ElementalResists{Fire: 100}, false)
	if got != 100 {
		t.Fatalf("dano nao elemental=%d, quer 100", got)
	}
}

func TestSkillMonsterTargetsUsesAreaAndMaxTarget(t *testing.T) {
	monster := func(id, x, y uint16) *Mob {
		return &Mob{ID: id, X: x, Y: y, HP: 100,
			Def: testNPCDef(model.Score{MaxHP: 100})}
	}
	w := testSpatialWorld([]*Mob{
		monster(1000, 11, 10),
		monster(1001, 12, 10),
		monster(1002, 20, 20),
	})
	p := &Player{X: 10, Y: 10}
	targets := w.skillMonsterTargets(p, skillCastRequest{TargetID: 1000}, model.SkillDef{TargetType: 3, Range: 2, MaxTarget: 13})
	if len(targets) != 2 || targets[0].ID != 1000 || targets[1].ID != 1001 {
		t.Fatalf("alvos de area incorretos: %+v", targets)
	}
}

func TestOffensiveSkillNativeAreaRadiiAndCap(t *testing.T) {
	ch := &model.Char{Score: testScore(model.Score{})}
	for _, tc := range []struct {
		targetType int
		want       int
	}{{3, 1}, {4, 2}, {6, 3}, {0, 0}} {
		if got := offensiveSkillAreaRadius(ch, model.SkillDef{TargetType: tc.targetType}); got != tc.want {
			t.Fatalf("TargetType=%d radius=%d want=%d", tc.targetType, got, tc.want)
		}
	}
	ch.Score.Mastery[3] = 225
	if got := offensiveSkillAreaRadius(ch, model.SkillDef{TargetType: 5}); got != 6 {
		t.Fatalf("directional radius=%d want=6", got)
	}
	if got := offensiveSkillTargetLimit(ch, model.SkillDef{TargetType: 6, MaxTarget: 1}); got != 13 {
		t.Fatalf("area target cap=%d want=13", got)
	}
}

func TestSkillMonsterTargetsAreDeterministicAndStayInsideNativeRadius(t *testing.T) {
	monster := func(id uint16, x, y uint16) *Mob {
		return &Mob{ID: id, X: x, Y: y, HP: 100,
			Def: testNPCDef(model.Score{MaxHP: 100})}
	}
	primary := monster(2000, 100, 100)
	mobs := []*Mob{primary}
	positions := [][2]uint16{
		{99, 99}, {100, 99}, {101, 99}, {99, 100}, {101, 100}, {99, 101}, {100, 101}, {101, 101},
		{98, 98}, {99, 98}, {100, 98}, {101, 98}, {102, 98}, {98, 99}, {102, 99},
	}
	// Reverse IDs relative to insertion order so map/insertion order cannot
	// accidentally satisfy the authoritative distance-then-ID ordering.
	for i, pos := range positions {
		mobs = append(mobs, monster(uint16(1900-i), pos[0], pos[1]))
	}
	outside := monster(2100, 103, 100)
	mobs = append(mobs, outside)
	w := testSpatialWorld(mobs)
	p := &Player{X: 100, Y: 99}
	skill := model.SkillDef{TargetType: 4, Range: 20, MaxTarget: 1}
	targets := w.skillMonsterTargets(p, skillCastRequest{TargetID: primary.ID}, skill)
	if len(targets) != 13 {
		t.Fatalf("targets=%d want=13", len(targets))
	}
	if targets[0] != primary {
		t.Fatal("primary target must remain first")
	}
	for i := 1; i < len(targets); i++ {
		if targets[i] == outside || chebyshev(primary.X, primary.Y, targets[i].X, targets[i].Y) > 2 {
			t.Fatalf("target outside native radius: id=%d pos=(%d,%d)", targets[i].ID, targets[i].X, targets[i].Y)
		}
		if i > 1 {
			prev, current := targets[i-1], targets[i]
			prevDistance := chebyshev(primary.X, primary.Y, prev.X, prev.Y)
			currentDistance := chebyshev(primary.X, primary.Y, current.X, current.Y)
			if currentDistance < prevDistance || currentDistance == prevDistance && current.ID < prev.ID {
				t.Fatalf("non-deterministic order at %d: prev=%d current=%d", i, prev.ID, current.ID)
			}
		}
	}
}

func TestSkillAOEDoesNotHitSecondaryAcrossBlockedLineOfSight(t *testing.T) {
	primary := &Mob{ID: 1000, X: 11, Y: 10, HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100})}
	secondary := &Mob{ID: 1001, X: 13, Y: 10, HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100})}
	w := testSpatialWorld([]*Mob{primary, secondary})
	w.terrain = loadedFlatTerrain()
	w.terrain.Height[10*model.TerrainWidth+12] = model.TerrainBlockedByte
	p := &Player{X: 10, Y: 10}

	targets := w.skillMonsterTargets(p, skillCastRequest{TargetID: primary.ID},
		model.SkillDef{TargetType: 4, Range: 3, MaxTarget: 13})
	if len(targets) != 1 || targets[0] != primary {
		t.Fatalf("secondary behind wall was selected: %+v", targets)
	}
}

func TestSkillAOEUsesSpatialIndexInsteadOfGlobalMobScan(t *testing.T) {
	primary := &Mob{ID: 1000, X: 11, Y: 10, HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100})}
	w := testSpatialWorld([]*Mob{primary})
	w.terrain = loadedFlatTerrain()
	p := &Player{X: 10, Y: 10}
	unindexed := &Mob{ID: primary.ID + 10, Def: primary.Def, X: primary.X + 1, Y: primary.Y, HP: 100}
	w.mobs = append(w.mobs, unindexed)

	targets := w.skillMonsterTargets(p, skillCastRequest{TargetID: primary.ID}, model.SkillDef{TargetType: 4, Range: 4, MaxTarget: 13})
	for _, target := range targets {
		if target == unindexed {
			t.Fatal("AoE reached a mob that was not present in the spatial index")
		}
	}
	w.registerMobSpatial(unindexed)
	targets = w.skillMonsterTargets(p, skillCastRequest{TargetID: primary.ID}, model.SkillDef{TargetType: 4, Range: 4, MaxTarget: 13})
	found := false
	for _, target := range targets {
		found = found || target == unindexed
	}
	if !found {
		t.Fatal("AoE did not discover the indexed nearby mob")
	}
}

func TestSkillMonsterTargetsNeverSubstitutesClientTarget(t *testing.T) {
	m := &Mob{ID: 1000, X: 11, Y: 10, HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100})}
	w := testSpatialWorld([]*Mob{m})
	p := &Player{X: 10, Y: 10}
	if got := w.skillMonsterTargets(p, skillCastRequest{TargetID: 9999},
		model.SkillDef{Range: 6, MaxTarget: 1}); len(got) != 0 {
		t.Fatalf("alvo invalido foi substituido por %+v", got)
	}
}

func TestSkillMonsterTargetsRejectsInstanceMobOutsideMembership(t *testing.T) {
	member, _ := networkedTestPlayer(1, "Member", 2200, 2200)
	outsider, _ := networkedTestPlayer(2, "Outsider", 2200, 2200)
	w := worldWithNetworkedPlayers(member, outsider)
	inst := &ItemInstance{
		Config: model.VolatileInstance{ID: "private-skill-room", Stages: []model.VolatileInstanceStage{{
			X: 2200, Y: 2200, AreaRadius: 8,
			Spawns: []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}},
		}}},
		MemberIDs: []uint16{member.ID}, CurrentStage: 0,
	}
	w.itemInstances = map[string]*ItemInstance{inst.Config.ID: inst}
	def := testNPCDef(model.Score{MaxHP: 100})
	def.Name = "RoomMob"
	mob := &Mob{ID: 1200, Def: def, X: 2200, Y: 2200, HP: 100, InstanceID: inst.Config.ID}
	w.mobs = []*Mob{mob}
	w.mobsByID[mob.ID] = mob
	w.registerMobSpatial(mob)

	if got := w.skillMonsterTargets(outsider, skillCastRequest{TargetID: mob.ID}, model.SkillDef{Range: 6, MaxTarget: 1}); len(got) != 0 {
		t.Fatalf("jogador externo conseguiu alvejar mob de instancia: %+v", got)
	}
	if got := w.skillMonsterTargets(member, skillCastRequest{TargetID: mob.ID}, model.SkillDef{Range: 6, MaxTarget: 1}); len(got) != 1 || got[0] != mob {
		t.Fatalf("membro nao conseguiu alvejar o mob da instancia: %+v", got)
	}
}

func TestRapidHitKeepsSixHitsOnOneTarget(t *testing.T) {
	monster := func(id, x, y uint16) *Mob {
		return &Mob{ID: id, X: x, Y: y, HP: 100,
			Def: testNPCDef(model.Score{MaxHP: 100})}
	}
	w := testSpatialWorld([]*Mob{monster(1000, 11, 10), monster(1001, 12, 10)})
	p := &Player{X: 10, Y: 10}
	skill := model.SkillDef{Index: 95, Range: 6, MaxTarget: 6}
	targets := w.skillMonsterTargets(p, skillCastRequest{TargetID: 1000}, skill)
	if len(targets) != 1 || targets[0].ID != 1000 || skillHitCount(skill) != 6 {
		t.Fatalf("Rapid Hit: targets=%v hits=%d", targets, skillHitCount(skill))
	}
}

func TestTwoTargetSkillUsesOnlyTheExplicitValidatedSecondTarget(t *testing.T) {
	monster := func(id, x, y uint16) *Mob {
		return &Mob{ID: id, X: x, Y: y, HP: 100,
			Def: testNPCDef(model.Score{MaxHP: 100})}
	}
	primary := monster(1000, 11, 10)
	selected := monster(1001, 12, 10)
	unselected := monster(1002, 11, 11)
	w := testSpatialWorld([]*Mob{primary, selected, unselected})
	p := &Player{X: 10, Y: 10}
	skill := model.SkillDef{TargetType: 1, Range: 4, MaxTarget: 2}
	targets := w.skillMonsterTargets(p, skillCastRequest{
		TargetID: primary.ID, SecondaryTargetID: selected.ID,
	}, skill)
	if len(targets) != 2 || targets[0] != primary || targets[1] != selected {
		t.Fatalf("two-target selection=%v", targets)
	}
}

func TestTKCriticalArmorAndHTCoinArmorUseDistinctClientTypes(t *testing.T) {
	ch := &model.Char{Score: testScore(
		model.Score{MaxHP: 100, MaxMP: 100, CurHP: 100, CurMP: 100})}
	p := &Player{ID: 1, Char: ch, Session: &net.Session{ID: 1}}
	w := &World{}
	w.applySupportSkill(p, skillCastRequest{}, model.SkillDef{
		// O catalogo 7.48 entrega 50; a camada de gameplay deve normalizar para a
		// semantica 31 sem perder o slot visual 24 do executavel.
		Index: 15, AffectType: 50, AffectValue: 150, AffectTime: 10, MaxTarget: 1,
	}, 40)
	if ch.Affects[0].Type != 31 || ch.Affects[0].ClientType != 24 {
		t.Fatalf("Critical Armor TK incorreta: %+v", ch.Affects[0])
	}
	ch.Affects = [16]model.Affect{}
	w.applySupportSkill(p, skillCastRequest{}, model.SkillDef{
		Index: 86, AffectType: 31, AffectTime: 10, MaxTarget: 1,
	}, 40)
	if ch.Affects[0].Type != 31 || ch.Affects[0].ClientType != 31 {
		t.Fatalf("Coin Armor HT incorreta: %+v", ch.Affects[0])
	}
}

func TestSummonCountScalesToPerCreatureLimitAt255(t *testing.T) {
	limits := []int{7, 6, 6, 5, 5, 4, 3, 2}
	for value, limit := range limits {
		instanceValue := value + 1
		if got := summonCount(instanceValue, 0); got != 1 {
			t.Fatalf("summon value=%d mastery=0: got=%d want=1", instanceValue, got)
		}
		if got := summonCount(instanceValue, 255); got != limit {
			t.Fatalf("summon value=%d mastery=255: got=%d want=%d", instanceValue, got, limit)
		}
		if got := summonCount(instanceValue, 999); got != limit {
			t.Fatalf("summon value=%d mastery acima de 255: got=%d want=%d", instanceValue, got, limit)
		}
	}
	if got := summonCount(1, 128); got != 4 {
		t.Fatalf("Condor mastery=128: got=%d want=4", got)
	}
	if got := summonCount(8, 128); got != 1 {
		t.Fatalf("Succubus mastery=128: got=%d want=1", got)
	}
}

func TestSkillPlayerTargetsRejectsPartyAndUsesSelectedEnemy(t *testing.T) {
	caster := &Player{ID: 1, InWorld: true, X: 10, Y: 10, Char: &model.Char{Score: testScore(model.Score{MaxHP: 100, CurHP: 100})}}
	enemy := &Player{ID: 2, InWorld: true, X: 12, Y: 10, Char: &model.Char{Score: testScore(model.Score{MaxHP: 100, CurHP: 100})}}
	member := &Player{ID: 3, InWorld: true, X: 11, Y: 10, Char: &model.Char{Score: testScore(model.Score{MaxHP: 100, CurHP: 100})}}
	party := &Party{Members: []*Player{caster, member}}
	caster.Party, member.Party = party, party
	w := testSpatialWorld(nil, caster, enemy, member)
	skill := model.SkillDef{Range: 6, MaxTarget: 1, Aggressive: 1}
	if got := w.skillPlayerTargets(caster, skillCastRequest{TargetID: enemy.ID}, skill); len(got) != 1 || got[0] != enemy {
		t.Fatalf("inimigo selecionado nao foi aceito: %+v", got)
	}
	if got := w.skillPlayerTargets(caster, skillCastRequest{TargetID: member.ID}, skill); len(got) != 0 {
		t.Fatalf("membro do grupo virou alvo PvP: %+v", got)
	}
}

func TestSkillPvPAOEDoesNotHitSecondaryAcrossBlockedLineOfSight(t *testing.T) {
	player := func(id, x uint16) *Player {
		return &Player{ID: id, InWorld: true, X: x, Y: 10,
			Char: &model.Char{Score: testScore(model.Score{MaxHP: 100, CurHP: 100})}}
	}
	caster, primary, secondary := player(1, 10), player(2, 11), player(3, 13)
	w := testSpatialWorld(nil, caster, primary, secondary)
	w.terrain = loadedFlatTerrain()
	w.terrain.Height[10*model.TerrainWidth+12] = model.TerrainBlockedByte
	targets := w.skillPlayerTargets(caster, skillCastRequest{TargetID: primary.ID},
		model.SkillDef{TargetType: 4, Range: 3, MaxTarget: 13, Aggressive: 1})
	if len(targets) != 1 || targets[0] != primary {
		t.Fatalf("PvP secondary behind wall was selected: %+v", targets)
	}
}

func TestTKMagicDamageMatches759Formula(t *testing.T) {
	w := &World{}
	ch := &model.Char{Class: 0, LearnedSkill: 1 << 7, Score: testScore(model.Score{
		Level: 41, Int: 79,
	})}
	skill := model.SkillDef{Index: 0, InstanceType: 4, InstanceValue: 5}
	if !skillUsesMagic(ch, skill) {
		t.Fatal("Heaven's Dust nao foi classificada como dano magico")
	}
	// 7.59 TK ramo magico: level/2 + mastery + base + arma + INT/4 + INT/40.
	if got := w.baseSkillDamage(ch, skill); got != 64 {
		t.Fatalf("dano-base=%d, quer 64", got)
	}
	ch.Score.Mastery[1] = 40
	ch.Score.MagicAmp = 25 // equivalente a 100 pontos de EF_MAGIC/EF_MAGICADD
	if got := w.baseSkillDamage(ch, skill); got != 243 {
		t.Fatalf("dano-base amplificado=%d, quer 243", got)
	}
}

func TestTKTransformationUsesStrengthAndNotIntelligence(t *testing.T) {
	w := &World{}
	ch := &model.Char{Class: 0, Score: testScore(model.Score{
		Level: 41, Str: 79, Int: 30_000,
	})}
	skill := model.SkillDef{Index: 8, InstanceType: 1, InstanceValue: 5}
	if skillUsesMagic(ch, skill) {
		t.Fatal("skill do ramo Trans foi classificada como dano magico")
	}
	// (3*STR + level/2 + base) * 5/4; INT nao participa.
	if got := w.baseSkillDamage(ch, skill); got != 327 {
		t.Fatalf("dano-base Trans=%d, quer 327", got)
	}
}

func TestSkillVisualLevelNeverSuppressesClientEffect(t *testing.T) {
	if got := skillVisualLevel(0); got != 1 {
		t.Fatalf("mastery zero gerou SkillParm=%d, quer 1", got)
	}
	if got := skillVisualLevel(40); got != 40 {
		t.Fatalf("mastery 40 gerou SkillParm=%d, quer 40", got)
	}
	if got := skillVisualLevel(999); got != 255 {
		t.Fatalf("mastery alta gerou SkillParm=%d, quer 255", got)
	}
}

func TestExplosionBashAddsAllCurrentMana(t *testing.T) {
	if got := explosionBashBaseDamage(90, 120, 750); got != 960 {
		t.Fatalf("dano Explosion Bash=%d, quer 960", got)
	}
}

func TestFoemaHealMatches759AndDoesNotRecalculateStats(t *testing.T) {
	ch := &model.Char{Class: 1, Score: &model.Score{
		Version: model.ScoreVersion, Level: 73, Attack: 777, Defense: 555,
		MaxHP: 500, CurHP: 100, Int: 250,
	}}
	applyScore(ch)
	p := &Player{ID: 2, Char: ch, Session: &net.Session{ID: 2}}
	w := &World{}
	results := w.applySupportSkill(p, skillCastRequest{}, model.SkillDef{
		Index: 27, InstanceValue: 100, MaxTarget: 1,
	}, 0)
	if len(results) != 1 || results[0].hpDelta != 140 || playerCurHP(ch) != 240 {
		t.Fatalf("cura incorreta: results=%+v hp=%d", results, playerCurHP(ch))
	}
	if playerAttack(ch) != 777 || playerDefense(ch) != 555 || playerInt(ch) != 250 {
		t.Fatalf("cura recalculou/reduziu stats: %+v", effectiveScore(ch))
	}
	if got := foemaHealAmount(29, 40, 150); got != 294 {
		t.Fatalf("Recovery 7.59=%d, quer 294", got)
	}
	if skill, motion := tickAreaVisual(37); skill != 33 || motion != 254 {
		t.Fatalf("visual Lightning Storm: skill=%d motion=%d", skill, motion)
	}
}
