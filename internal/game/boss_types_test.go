package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func u32(v uint32) *uint32 { return &v }

func chaserConfig() model.BossConfig {
	return model.BossConfig{
		ID: "c", NPC: "Aparicao", Type: model.BossTypeChaser,
		Spawn: model.BossSpawn{X: 100, Y: 200, RespawnSeconds: 60},
	}
}

// TestChaserNaoGeraRegras confirma o desenho: o chaser reusa a IA de mob comum,
// entao nao precisa de nenhuma regra de boss.
func TestChaserNaoGeraRegras(t *testing.T) {
	profile, err := compileBossProfile(chaserConfig())
	if err != nil {
		t.Fatal(err)
	}
	if len(profile.Rules) != 0 {
		t.Fatalf("chaser gerou %d regras, quer 0: %+v", len(profile.Rules), profile.Rules)
	}
	if len(profile.Phases) != 1 {
		t.Fatalf("chaser deveria ter so a fase inicial, tem %d", len(profile.Phases))
	}
}

func TestCasterGeraUmaRegraPorSkill(t *testing.T) {
	config := chaserConfig()
	config.Type = model.BossTypeCaster
	config.Skills = []model.BossSkill{
		{ID: 26, CooldownSeconds: 8, Range: 7, MaxHPPercent: 100},
		{ID: 27, CooldownSeconds: 25, Range: 10, MaxHPPercent: 40},
	}
	profile, err := compileBossProfile(config)
	if err != nil {
		t.Fatal(err)
	}
	rules := profile.rulesFor(BossEventDamaged)
	if len(rules) != 2 {
		t.Fatalf("regras de dano=%d, quer 2", len(rules))
	}

	var comPorta, semPorta int
	for _, rule := range rules {
		action := profile.Actions[rule.ActionID]
		if action.Kind != ActionCastSkill {
			t.Fatalf("acao %d nao e skill", rule.ActionID)
		}
		gated := false
		for _, condition := range rule.Conditions {
			if condition.Kind == CondHPBelowOrEqual {
				gated = true
				if condition.Value != 40 {
					t.Errorf("porta de HP=%d, quer 40", condition.Value)
				}
			}
		}
		if gated {
			comPorta++
		} else {
			semPorta++
		}
	}
	// max_hp_percent=100 nao vira porta; 40 vira.
	if comPorta != 1 || semPorta != 1 {
		t.Fatalf("portas de HP: com=%d sem=%d, quer 1 e 1", comPorta, semPorta)
	}
	if got := profile.Actions[rules[0].ActionID].Cooldown; got == 0 {
		t.Error("cooldown da skill nao foi propagado")
	}
}

func TestSummonerPropagaLimites(t *testing.T) {
	config := chaserConfig()
	config.Type = model.BossTypeSummoner
	config.Summons = []model.BossSummon{
		{NPC: "Skeleton", Count: 4, MaxAlive: 8, CooldownSeconds: 30},
	}
	profile, err := compileBossProfile(config)
	if err != nil {
		t.Fatal(err)
	}
	rules := profile.rulesFor(BossEventDamaged)
	if len(rules) != 1 {
		t.Fatalf("regras=%d, quer 1", len(rules))
	}
	action := profile.Actions[rules[0].ActionID]
	if action.SummonNPC != "Skeleton" || action.SummonCount != 4 || action.SummonMax != 8 {
		t.Fatalf("summon mal propagado: %+v", action)
	}
	if action.Cooldown != 30*time.Second {
		t.Fatalf("cooldown=%v, quer 30s", action.Cooldown)
	}
}

// TestPhasedOrdenaLimiaresDecrescente protege a ordem de travessia: a fase de
// maior HP precisa vir primeiro, senao um golpe grande aplicaria a fase errada.
func TestPhasedOrdenaLimiaresDecrescente(t *testing.T) {
	config := chaserConfig()
	config.Type = model.BossTypePhased
	config.Summons = []model.BossSummon{{NPC: "Skeleton", Count: 2}}
	// Declaradas fora de ordem de proposito.
	config.Phases = []model.BossPhase{
		{HPPercent: 25, Type: model.BossTypeCaster},
		{HPPercent: 60, ShieldPercent: 100, ShieldUntilAddsDead: true},
	}
	profile, err := compileBossProfile(config)
	if err != nil {
		t.Fatal(err)
	}
	want := []int{60, 25}
	if len(profile.HealthThresholds) != len(want) {
		t.Fatalf("limiares=%v, quer %v", profile.HealthThresholds, want)
	}
	for i := range want {
		if profile.HealthThresholds[i] != want[i] {
			t.Fatalf("limiares=%v, quer %v (decrescente)", profile.HealthThresholds, want)
		}
	}
	// Fase inicial + duas configuradas.
	if len(profile.Phases) != 3 {
		t.Fatalf("fases=%d, quer 3", len(profile.Phases))
	}
	// A fase de 60% pediu escudo que cai com os adds: precisa existir a regra
	// de remover o efeito em AddDied.
	if len(profile.rulesFor(BossEventAddDied)) != 1 {
		t.Fatalf("faltou a regra de derrubar o escudo em AddDied")
	}
}

func TestPhasedComEscudoGeraOrdemCorreta(t *testing.T) {
	config := chaserConfig()
	config.Type = model.BossTypePhased
	config.Summons = []model.BossSummon{{NPC: "Skeleton", Count: 2}}
	config.Phases = []model.BossPhase{{HPPercent: 60, ShieldPercent: 100, ShieldUntilAddsDead: true}}
	profile, err := compileBossProfile(config)
	if err != nil {
		t.Fatal(err)
	}
	rules := profile.rulesFor(BossEventHealthThresholdCrossed)
	// A ordem importa e e o coracao do encontro:
	//   1. troca de fase   -- habilita as acoes da fase nova;
	//   2. escudo          -- instantaneo, precisa entrar antes de qualquer
	//                         acao pendente bloquea-lo;
	//   3. summon da fase  -- vira acao PENDENTE, entao vem por ultimo.
	want := []BossActionKind{ActionChangePhase, ActionApplyEffect, ActionSummon}
	if len(rules) != len(want) {
		t.Fatalf("regras de limiar=%d, quer %d", len(rules), len(want))
	}
	for i, kind := range want {
		if got := profile.Actions[rules[i].ActionID].Kind; got != kind {
			t.Fatalf("regra[%d] e %v, quer %v", i, got, kind)
		}
	}
	// A cascata depende de TODAS as regras do evento serem avaliadas: nenhuma
	// pode interromper a sequencia (por isso o motor nao tem StopEvaluation).
}

func TestBossTipoAcumulaRegrasDeTodasAsListas(t *testing.T) {
	config := chaserConfig()
	config.Type = model.BossTypePhased
	config.Skills = []model.BossSkill{{ID: 26, CooldownSeconds: 5}}
	config.Summons = []model.BossSummon{{NPC: "Skeleton", Count: 2}}
	config.Phases = []model.BossPhase{{HPPercent: 50}}
	profile, err := compileBossProfile(config)
	if err != nil {
		t.Fatal(err)
	}
	// O tipo define o obrigatorio, nao o permitido: um phased com skills e
	// summons recebe as regras dos tres.
	if len(profile.rulesFor(BossEventDamaged)) != 2 {
		t.Fatalf("esperava skill + summon reagindo a dano, veio %d",
			len(profile.rulesFor(BossEventDamaged)))
	}
	if len(profile.rulesFor(BossEventHealthThresholdCrossed)) != 1 {
		t.Fatal("esperava a regra de troca de fase")
	}
}

func TestBossDefHerdaAssetsESobrescreveStats(t *testing.T) {
	base := &model.NPCDef{
		Name: "Aparicao", Tipo: "npc",
		Equip:     model.Equip{Rosto: model.Item{Index: 218}},
		ExpReward: 100, Gold: 50,
		Carry:    []model.Item{{Index: 999}},
		Extended: &model.ExtendedScore{Version: 2, MaxHP: 4200, Attack: 100, Defense: 50, Level: 10},
	}
	config := chaserConfig()
	config.Name = "Chefe"
	config.Stats = model.BossStats{MaxHP: u32(90000), Attack: u32(700), ExpReward: u32(250000)}

	def := bossDefFrom(base, config)

	if def.Equip.Rosto.Index != 218 {
		t.Error("assets do NPC base deveriam ser herdados")
	}
	if def.Name != "Chefe" {
		t.Errorf("nome=%q, quer Chefe", def.Name)
	}
	if def.Tipo != model.TipoMonstro {
		t.Errorf("boss precisa ser hostil, tipo=%q", def.Tipo)
	}
	if def.Extended.MaxHP != 90000 || def.Extended.Attack != 700 {
		t.Errorf("stats nao sobrescritos: %+v", def.Extended)
	}
	// Defesa nao foi configurada: mantem a do NPC base.
	if def.Extended.Defense != 50 {
		t.Errorf("defesa=%d, quer manter 50 do NPC base", def.Extended.Defense)
	}
	if def.ExpReward != 250000 {
		t.Errorf("expReward=%d", def.ExpReward)
	}
	// O carry do NPC base nao vale: o boss usa a tabela de drops propria.
	if def.Carry != nil {
		t.Errorf("carry do NPC base deveria ser ignorado: %+v", def.Carry)
	}
	// E o mais importante: o NPC base NAO pode ter sido alterado.
	if base.Extended.MaxHP != 4200 || base.Name != "Aparicao" || base.Tipo != "npc" {
		t.Errorf("o NPC base foi mutado: %+v", base.Extended)
	}
}
