package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

// testBossConfig reproduz o encontro classico -- duas fases, escudo que cai com
// os adds e um contra-ataque -- pelo caminho REAL: configuracao .lua traduzida
// por compileBossProfile.
//
// O NPC base e ficticio: o teste monta o proprio NPCDef, entao nao depende (nem
// deve depender) de nenhum mob do catalogo real.
func testBossConfig() model.BossConfig {
	return model.BossConfig{
		ID: "boss_teste", NPC: "Boss_De_Teste", Type: model.BossTypePhased,
		Spawn:   model.BossSpawn{X: 2100, Y: 2100, RespawnSeconds: 60},
		Skills:  []model.BossSkill{{ID: 26, CooldownSeconds: 6, Range: 3}},
		Summons: []model.BossSummon{{NPC: "Skeleton", Count: 4}},
		Phases: []model.BossPhase{{
			HPPercent: 60, Type: model.BossTypeSummoner,
			ShieldPercent: 100, ShieldUntilAddsDead: true,
			Message: "O boss convoca seus guardioes!",
		}},
	}
}

// testBossProfile compila a config de teste, falhando o teste se ela for
// invalida.
func testBossProfile(t *testing.T) *BossProfile {
	t.Helper()
	profile, err := compileBossProfile(testBossConfig())
	if err != nil {
		t.Fatalf("compilar perfil de teste: %v", err)
	}
	return profile
}

// bossActionOfKind acha a acao GERADA de um tipo. Os IDs sao alocados por
// compileBossProfile, entao buscar pelo tipo mantem o teste legivel e imune a
// mudanca na ordem de geracao.
func bossActionOfKind(t *testing.T, profile *BossProfile, kind BossActionKind) BossActionID {
	t.Helper()
	for id, action := range profile.Actions {
		if action.Kind == kind {
			return id
		}
	}
	t.Fatalf("perfil nao tem acao do tipo %v", kind)
	return 0
}

// bossRuleOfKind acha a regra cuja acao e do tipo pedido.
func bossRuleOfKind(t *testing.T, profile *BossProfile, kind BossActionKind) BossRuleID {
	t.Helper()
	for _, rule := range profile.Rules {
		if profile.Actions[rule.ActionID].Kind == kind {
			return rule.ID
		}
	}
	t.Fatalf("perfil nao tem regra para acao do tipo %v", kind)
	return 0
}

func bossTestNPC(name string, maxHP uint32) *model.NPCDef {
	return &model.NPCDef{
		Name: name, Tipo: model.TipoMonstro,
		Score: &model.Score{
			Version: model.ScoreVersion,
			MaxHP:   maxHP, CurHP: maxHP, Level: 100,
			Attack: 50, Defense: 10, AttackRun: 0x64,
		},
	}
}

// newBossTestWorld monta um mundo minimo com um boss ja registrado.
func newBossTestWorld(t *testing.T, profile *BossProfile, maxHP uint32) (*World, *Mob, *BossRuntime, *fakeClock) {
	t.Helper()
	clock := newFakeClock(time.Date(2026, 7, 24, 12, 0, 0, 0, time.UTC))
	w := newZoneTestWorld()
	w.clock = clock
	w.questZones = nil
	w.npcs = []model.NPCDef{*bossTestNPC("Skeleton", 100)}

	if err := profile.Compile(); err != nil {
		t.Fatalf("perfil invalido: %v", err)
	}
	def := bossTestNPC(profile.NPCName, maxHP)
	// O boss usa um ID ja alocado e nextMobID aponta para depois dele, como faz
	// o NewWorld. Sem isso os adds nasceriam com IDs colidindo com o boss.
	mob := &Mob{ID: 1000, Def: def, X: 2100, Y: 2100, HP: maxHP, GenerIndex: -1}
	w.nextMobID = 1001
	w.mobs = append(w.mobs, mob)
	w.registerMobSpatial(mob)

	if err := w.RegisterBoss(mob.ID, profile); err != nil {
		t.Fatalf("registrar boss: %v", err)
	}
	return w, mob, w.bosses[mob.ID], clock
}

// damageBoss simula o caminho de dano do jogador: aplica mitigacao, reduz o HP
// e notifica -- a mesma ordem de handlers.go.
func damageBoss(w *World, mob *Mob, damage uint32, sourceID uint16) {
	damage = w.bossMitigateDamage(mob, damage)
	oldHP := mob.HP
	if damage >= mob.HP {
		mob.HP = 0
	} else {
		mob.HP -= damage
	}
	w.notifyMobDamaged(mob, oldHP, sourceID, damage)
}

func TestBossProfileCompileRejectsBrokenReferences(t *testing.T) {
	for _, tc := range []struct {
		name    string
		mutate  func(*BossProfile)
		wantErr bool
	}{
		{"valido", func(*BossProfile) {}, false},
		{"fase inicial inexistente", func(p *BossProfile) { p.InitialPhase = 99 }, true},
		{"regra aponta acao inexistente", func(p *BossProfile) { p.Rules[0].ActionID = 99 }, true},
		{"limiar fora do intervalo", func(p *BossProfile) { p.HealthThresholds = []int{150} }, true},
		{"regra duplicada", func(p *BossProfile) { p.Rules = append(p.Rules, p.Rules[0]) }, true},
		{"condicao com fase inexistente", func(p *BossProfile) {
			p.Rules[0].Conditions = []BossCondition{{Kind: CondPhaseIs, Phase: 42}}
		}, true},
		{"mudanca para fase inexistente", func(p *BossProfile) {
			id := bossActionOfKind(t, p, ActionChangePhase)
			action := p.Actions[id]
			action.Phase = 77
			p.Actions[id] = action
		}, true},
	} {
		t.Run(tc.name, func(t *testing.T) {
			profile := testBossProfile(t)
			tc.mutate(profile)
			err := profile.Compile()
			if tc.wantErr && err == nil {
				t.Fatal("perfil invalido deveria ser recusado")
			}
			if !tc.wantErr && err != nil {
				t.Fatalf("perfil valido recusado: %v", err)
			}
		})
	}
}

// TestBossRulesSortedByPriorityWithDeterministicTieBreak protege o determinismo
// exigido pela spec: mesma prioridade sempre dispara na mesma ordem.
func TestBossRulesSortedByPriorityWithDeterministicTieBreak(t *testing.T) {
	profile := &BossProfile{
		ID: "t", NPCName: "X", InitialPhase: 1,
		Phases:  map[BossPhaseID]BossPhase{1: {ID: 1}},
		Actions: map[BossActionID]BossActionDef{1: {ID: 1, Kind: ActionApplyEffect, AffectType: 1}},
		Rules: []BossRule{
			{ID: 30, Event: BossEventDamaged, Priority: 10, ActionID: 1},
			{ID: 10, Event: BossEventDamaged, Priority: 90, ActionID: 1},
			{ID: 20, Event: BossEventDamaged, Priority: 10, ActionID: 1},
		},
	}
	if err := profile.Compile(); err != nil {
		t.Fatal(err)
	}
	rules := profile.rulesFor(BossEventDamaged)
	want := []BossRuleID{10, 20, 30} // prioridade desc; empate pelo menor ID
	for i, rule := range rules {
		if rule.ID != want[i] {
			t.Fatalf("ordem[%d]=%d, quer %d (ordem completa: %v)", i, rule.ID, want[i], rules)
		}
	}
}

func TestBossThresholdEmittedOnlyOnCrossing(t *testing.T) {
	w, mob, boss, _ := newBossTestWorld(t, testBossProfile(t), 1000)

	// 100% -> 70%: ainda acima do limiar de 60.
	damageBoss(w, mob, 300, 1)
	if boss.Phase != bossInitialPhase {
		t.Fatalf("mudou de fase antes do limiar: %d", boss.Phase)
	}

	// 70% -> 55%: atravessa 60.
	damageBoss(w, mob, 150, 1)
	if boss.Phase != phaseIDFor(0) {
		t.Fatalf("fase=%d, quer %d apos atravessar 60%%", boss.Phase, phaseIDFor(0))
	}
	if _, crossed := boss.crossedThresholds[60]; !crossed {
		t.Fatal("limiar 60 deveria estar marcado como atravessado")
	}
}

// TestBossMultipleThresholdsCrossedInOneHit cobre o golpe grande que atravessa
// varios limiares: todos devem ser emitidos, do maior para o menor.
func TestBossMultipleThresholdsCrossedInOneHit(t *testing.T) {
	profile := testBossProfile(t)
	profile.HealthThresholds = []int{75, 50, 25}
	var seen []int
	profile.Rules = nil // sem regras: so observamos os eventos
	if err := profile.Compile(); err != nil {
		t.Fatal(err)
	}
	w, _, boss, _ := newBossTestWorld(t, profile, 1000)

	// Emite direto a transicao 80% -> 20%, que cruza os tres limiares de uma vez.
	w.emitCrossedThresholds(boss, 1, 80, 20, w.now())
	for _, threshold := range profile.HealthThresholds {
		if _, crossed := boss.crossedThresholds[threshold]; crossed {
			seen = append(seen, threshold)
		}
	}
	want := []int{75, 50, 25}
	if len(seen) != len(want) {
		t.Fatalf("limiares atravessados=%v, quer %v", seen, want)
	}
	for i := range want {
		if seen[i] != want[i] {
			t.Fatalf("ordem descendente quebrada: %v", seen)
		}
	}
}

func TestBossThresholdNotReemittedWithoutNewCrossing(t *testing.T) {
	w, mob, boss, _ := newBossTestWorld(t, testBossProfile(t), 1000)
	damageBoss(w, mob, 450, 1) // 100% -> 55%: atravessa 60
	spawnedFirst := boss.addsAlive()

	// Mais dano abaixo do limiar nao pode reemitir o evento.
	damageBoss(w, mob, 50, 1)
	if boss.addsAlive() != spawnedFirst {
		t.Fatalf("limiar reemitido: adds %d -> %d", spawnedFirst, boss.addsAlive())
	}
}

func TestBossThresholdRetriesAfterActionConflict(t *testing.T) {
	w, mob, boss, clock := newBossTestWorld(t, testBossProfile(t), 1000)
	changeID := bossActionOfKind(t, boss.Profile, ActionChangePhase)
	// Uma acao nao-interruptivel ja em execucao bloqueia a virada obrigatoria.
	boss.Pending = &BossPendingAction{
		Generation: 1, ActionID: changeID, ExecuteAt: clock.Now().Add(time.Minute),
		Priority: 999, Interruptible: false,
	}
	w.emitCrossedThresholds(boss, 1, 70, 55, clock.Now())
	if _, crossed := boss.crossedThresholds[60]; crossed {
		t.Fatal("limiar bloqueado nao deveria ser consumido")
	}
	if _, pending := boss.pendingThresholds[60]; !pending {
		t.Fatal("limiar bloqueado deveria ficar pendente")
	}

	boss.Pending = nil
	w.retryBossThresholds(boss, clock.Now())
	if _, crossed := boss.crossedThresholds[60]; !crossed {
		t.Fatal("limiar deveria ser consumido depois que o conflito terminou")
	}
	if _, pending := boss.pendingThresholds[60]; pending {
		t.Fatal("limiar aceito nao deveria permanecer pendente")
	}
	if boss.Phase != phaseIDFor(0) {
		t.Fatalf("fase=%d, quer %d", boss.Phase, phaseIDFor(0))
	}
	_ = mob
}

func TestBossShieldAbsorbsDamageAndDropsWithLastAdd(t *testing.T) {
	w, mob, boss, clock := newBossTestWorld(t, testBossProfile(t), 1000)

	damageBoss(w, mob, 450, 1) // atravessa 60% -> fase 2 + escudo
	if boss.Phase != phaseIDFor(0) {
		t.Fatalf("fase=%d, quer fase 2", boss.Phase)
	}
	if activeMobAffectAt(mob, bossShieldAffect, clock.Now()) == nil {
		t.Fatal("escudo deveria estar ativo na fase 2")
	}

	// O summon tem CastTime: vence no tick.
	clock.Advance(time.Second)
	w.tickBossActions(clock.Now())
	if boss.addsAlive() != 4 {
		t.Fatalf("adds vivos=%d, quer 4", boss.addsAlive())
	}

	// Com escudo de 100%, o dano e absorvido por completo.
	hpBefore := mob.HP
	damageBoss(w, mob, 200, 1)
	if mob.HP != hpBefore {
		t.Fatalf("escudo nao absorveu: HP %d -> %d", hpBefore, mob.HP)
	}

	// Matando os adds um a um; o escudo so cai com o ultimo.
	addIDs := make([]uint16, 0, 4)
	for id := range boss.Adds {
		addIDs = append(addIDs, id)
	}
	for i, id := range addIDs {
		w.notifyBossAddDied(id)
		if i < len(addIDs)-1 && activeMobAffectAt(mob, bossShieldAffect, clock.Now()) == nil {
			t.Fatalf("escudo caiu cedo, com %d adds ainda vivos", boss.addsAlive())
		}
	}
	if activeMobAffectAt(mob, bossShieldAffect, clock.Now()) != nil {
		t.Fatal("escudo deveria cair quando o ultimo add morre")
	}

	// Vulneravel de novo.
	hpBefore = mob.HP
	damageBoss(w, mob, 200, 1)
	if mob.HP >= hpBefore {
		t.Fatalf("boss deveria voltar a receber dano: HP %d -> %d", hpBefore, mob.HP)
	}
}

func TestBossOnceRuleConsumedOnlyWhenAccepted(t *testing.T) {
	profile := testBossProfile(t)
	w, mob, boss, _ := newBossTestWorld(t, profile, 1000)

	// A regra de virada de fase e Once. Depois de aceita, some do jogo.
	damageBoss(w, mob, 450, 1)
	if _, consumed := boss.ConsumedRules[bossRuleOfKind(t, boss.Profile, ActionChangePhase)]; !consumed {
		t.Fatal("regra Once aceita deveria ter sido consumida")
	}

	// Forcar a fase de volta e reemitir o limiar nao pode repetir a transicao,
	// pois a regra ja foi consumida.
	boss.Phase = bossInitialPhase
	delete(boss.crossedThresholds, 60)
	damageBoss(w, mob, 10, 1)
	if boss.Phase != bossInitialPhase {
		t.Fatal("regra Once nao deveria disparar duas vezes")
	}
}

func TestBossCooldownBlocksRepeatedReaction(t *testing.T) {
	// Boss SO de skill: com summons no perfil, a regra de summon (prioridade
	// maior) viraria acao pendente e bloquearia a skill antes do cooldown.
	config := testBossConfig()
	config.Type = model.BossTypeCaster
	config.Summons, config.Phases = nil, nil
	profile, err := compileBossProfile(config)
	if err != nil {
		t.Fatal(err)
	}
	w, mob, boss, clock := newBossTestWorld(t, profile, 10000)

	damageBoss(w, mob, 10, 1)
	firstReady := boss.Cooldowns[bossActionOfKind(t, boss.Profile, ActionCastSkill)]
	if firstReady.IsZero() {
		t.Fatal("contra-ataque deveria iniciar cooldown ao ser aceito")
	}

	// Novo dano dentro do cooldown nao pode reagendar.
	clock.Advance(time.Second)
	damageBoss(w, mob, 10, 1)
	if got := boss.Cooldowns[bossActionOfKind(t, boss.Profile, ActionCastSkill)]; !got.Equal(firstReady) {
		t.Fatalf("cooldown reiniciado dentro da janela: %v -> %v", firstReady, got)
	}
}

// TestBossStaleGenerationCallbackIgnored cobre a invalidacao por geracao: uma
// acao agendada antes de um cancelamento nao pode executar depois dele.
func TestBossStaleGenerationCallbackIgnored(t *testing.T) {
	w, mob, boss, clock := newBossTestWorld(t, testBossProfile(t), 1000)

	damageBoss(w, mob, 450, 1) // agenda o summon (CastTime)
	if boss.Pending == nil {
		t.Fatal("summon deveria ficar pendente")
	}
	staleGeneration := boss.Pending.Generation

	// Qualquer cancelamento incrementa a geracao e aposenta o que estava
	// agendado (e o que a troca de fase faz internamente).
	boss.Generation++
	boss.Pending = nil
	// Reinstala a acao pendente obsoleta, como faria um callback atrasado.
	boss.Pending = &BossPendingAction{
		Generation: staleGeneration, ActionID: bossActionOfKind(t, boss.Profile, ActionSummon),
		ExecuteAt: clock.Now(),
	}
	clock.Advance(time.Second)
	w.tickBossActions(clock.Now())

	if boss.addsAlive() != 0 {
		t.Fatalf("callback de geracao antiga executou: adds=%d", boss.addsAlive())
	}
	if boss.Pending != nil {
		t.Fatal("acao obsoleta deveria ter sido descartada")
	}
}

// TestCommonMobIgnoresBossSubsystem garante o requisito central: um mob comum
// nao paga nada nem se comporta diferente por existir o subsistema.
func TestCommonMobIgnoresBossSubsystem(t *testing.T) {
	w, _, _, _ := newBossTestWorld(t, testBossProfile(t), 1000)
	common := &Mob{ID: 2000, Def: bossTestNPC("Skeleton", 100), X: 2100, Y: 2100, HP: 100}
	w.mobs = append(w.mobs, common)
	w.registerMobSpatial(common)

	if w.bossFor(common.ID) != nil {
		t.Fatal("mob comum nao pode ter runtime de boss")
	}
	// Mitigacao nao se aplica e a notificacao e inofensiva.
	if got := w.bossMitigateDamage(common, 100); got != 100 {
		t.Fatalf("dano em mob comum foi mitigado: %d", got)
	}
	damageBoss(w, common, 40, 1)
	if common.HP != 60 {
		t.Fatalf("HP do mob comum=%d, quer 60", common.HP)
	}
}

func TestBossSkillDamageUsesMagicCoreAndGuardsInvalidInputs(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Target", 2100, 2100)
	p.Char.Score.Defense = 100
	applyExtendedScore(p.Char)
	m := &Mob{ID: 1000, Def: testNPCDef(model.Score{
		Level: 100, MaxHP: 1000, CurHP: 1000,
		MagicAttack: 2000, Int: 400, Attack: 10,
	})}
	skill := model.SkillDef{Index: 26, InstanceValue: 50, InstanceType: 1}
	if damage := bossSkillDamage(m, p, skill); damage <= 1 {
		t.Fatalf("nucleo magico do boss produziu dano=%d", damage)
	}
	if bossSkillDamage(nil, p, skill) != 0 ||
		bossSkillDamage(m, nil, skill) != 0 ||
		bossSkillDamage(&Mob{}, p, skill) != 0 {
		t.Fatal("entrada invalida deveria produzir dano zero")
	}

	m.Def.Score.MagicAttack = 0
	m.Def.Score.Int = 0
	m.Def.Score.Level = 0
	m.Def.Score.Attack = 500
	if damage := bossSkillDamage(m, p, model.SkillDef{}); damage <= 1 {
		t.Fatalf("fallback para ataque fisico nao funcionou: %d", damage)
	}
}

func TestUnregisterBossRemovesLivingAddsFromWorldAndView(t *testing.T) {
	viewer, _ := networkedTestPlayer(1, "Viewer", 2100, 2100)
	w := worldWithNetworkedPlayers(viewer)
	add := &Mob{ID: 1200, Def: bossTestNPC("Add", 100), X: 2101, Y: 2100, HP: 100}
	w.mobs = append(w.mobs, add)
	w.registerMobSpatial(add)
	viewer.show(add.ID)
	profile := testBossProfile(t)
	runtime := newBossRuntime(1100, profile)
	runtime.Adds[add.ID] = struct{}{}
	w.bosses = map[uint16]*BossRuntime{1100: runtime}

	w.UnregisterBoss(1100)
	if w.bossFor(1100) != nil || w.mobsByID[add.ID] != nil || viewer.hasVisible(add.ID) {
		t.Fatal("boss/add/visibilidade sobreviveram ao unregister")
	}
	if !add.Dead || len(runtime.Adds) != 0 {
		t.Fatal("add removido nao foi marcado/limpo no runtime")
	}
	w.UnregisterBoss(9999)
}
