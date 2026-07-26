package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// boss_actions.go -- EXECUCAO das acoes. E o unico lugar do subsistema que muta
// o mundo, e roda sempre dentro da goroutine do World, apos a validacao
// autoritativa em boss_events.go.

// executeBossAction aplica a acao ja validada. depth propaga o limite de
// eventos encadeados: uma acao pode gerar evento que dispara outra regra.
func (w *World) executeBossAction(boss *BossRuntime, mob *Mob, action BossActionDef, event BossEvent, depth int) {
	switch action.Kind {
	case ActionChangePhase:
		w.bossChangePhase(boss, mob, action, depth)
	case ActionCastSkill:
		w.bossCastSkill(boss, mob, action, event)
	case ActionSummon:
		w.bossSummonAdds(boss, mob, action)
	case ActionApplyEffect:
		w.bossApplyEffect(boss, mob, action)
	case ActionRemoveEffect:
		w.bossRemoveEffect(boss, mob, action)
	}
}

// bossChangePhase troca a fase e anuncia. Cancela a acao pendente: uma mecanica
// da fase anterior nao deve completar depois da virada.
func (w *World) bossChangePhase(boss *BossRuntime, mob *Mob, action BossActionDef, depth int) {
	phase, ok := boss.Profile.Phases[action.Phase]
	if !ok || boss.Phase == action.Phase {
		return
	}
	previous := boss.Phase
	if boss.Pending != nil {
		// Incrementar a geracao invalida o callback ja agendado.
		boss.Generation++
		boss.Pending = nil
	}
	boss.Phase = action.Phase
	log.Printf("BOSS %q: fase %d -> %d (%s)", boss.Profile.ID, previous, phase.ID, phase.Name)
	if phase.EnterMessage != "" {
		w.announceToMobView(mob, phase.EnterMessage)
	}
	_ = depth // reservado: entrada de fase ainda nao emite evento proprio na v1
}

// bossCastSkill aplica a skill do boss. A v1 cobre o caso do encontro atual:
// dano direto no alvo do evento, ja validado como jogador vivo e proximo.
func (w *World) bossCastSkill(boss *BossRuntime, mob *Mob, action BossActionDef, event BossEvent) {
	target := w.playerByID(event.SourceID)
	if !validMobTarget(target) {
		return
	}
	// Alcance da skill configurada; sem valor, cai no corpo a corpo.
	reach := action.CastRange
	if reach <= 0 {
		reach = mobAttackRange
	}
	if chebyshev(mob.X, mob.Y, target.X, target.Y) > reach {
		return // saiu de alcance entre a aceitacao e a execucao
	}
	skill, known := w.skills[action.SkillID]
	if !known {
		// Skill inexistente no catalogo: nao inventa dano. O boot ja loga o
		// perfil, entao isto aparece como skill silenciosa e nao como golpe
		// disfarcado de magia.
		log.Printf("BOSS %q: skill %d nao existe em SkillData.csv; ignorada",
			boss.Profile.ID, action.SkillID)
		return
	}
	if action.Message != "" {
		w.announceToMobView(mob, action.Message)
	}

	damage := bossSkillDamage(mob, target, skill)
	now := w.now()
	w.applyMobDamageToPlayer(mob, target, damage, now, func(applied uint32) []byte {
		// SkillHitExtended (0x36C + 0x39D) faz o client desenhar a MAGIA, e nao
		// o golpe corpo a corpo. Motion/mastery em 0: o boss nao tem barra de
		// skill, o efeito visual vem do proprio indice.
		return wire.SkillHitExtended(mob.ID, target.ID, mob.X, mob.Y, target.X, target.Y,
			applied, playerMaxHP(target.Char), 0, mob.Def.Extended.MaxMP, int16(skill.Index), 0, 0)
	})
	log.Printf("BOSS %q: skill %d (%s) em jogador id=%d dano=%d",
		boss.Profile.ID, action.SkillID, skill.Name, target.ID, damage)
}

// bossSkillDamage calcula o dano MAGICO de uma skill de boss. Porta o caminho
// das skills de jogador para o lado do mob: nucleo magico do atacante, defesa e
// resistencia elemental do alvo.
//
// Existe porque o caminho de golpe comum (mobHitsPlayer) e fisico: usar ele
// aqui faria a "magia" ignorar resistencia elemental e escalar com forca --
// seria um golpe corpo a corpo com outro nome.
func bossSkillDamage(m *Mob, target *Player, skill model.SkillDef) uint32 {
	if m == nil || m.Def == nil || target == nil || target.Char == nil {
		return 0
	}
	extended := m.Def.Extended
	// Nucleo magico do mob: MagicAttack proprio mais a contribuicao de INT e do
	// nivel, na mesma forma do calculo de skill do jogador.
	core := int(extended.MagicAttack) + int(extended.Int)/4 + int(extended.Level)
	if core <= 0 {
		// NPC sem atributo magico: cai no ataque fisico para a skill ainda doer,
		// em vez de bater zero e parecer bug.
		core = effectiveMobAttack(m)
	}
	// InstanceValue escala o efeito da skill no catalogo nativo.
	if skill.InstanceValue > 0 {
		core = core * (100 + skill.InstanceValue) / 100
	}
	damage := skillFinalDamage(core, playerDefense(target.Char), 0)
	damage = applySkillResistance(damage, skill.InstanceType, playerElementalResists(target.Char), false)
	return uint32(clampInt(damage, 1, int(maxExtendedStat)))
}

// bossSummonAdds cria os adds do encontro a partir de um NPC do catalogo. Os
// adds sao mobs COMUNS: usam a IA normal e sao mortos normalmente. O encontro
// so guarda referencia para contagem e limpeza.
func (w *World) bossSummonAdds(boss *BossRuntime, mob *Mob, action BossActionDef) {
	template := w.npcDefByName(action.SummonNPC)
	if template == nil {
		log.Printf("BOSS %q: NPC de add %q nao existe no catalogo", boss.Profile.ID, action.SummonNPC)
		return
	}
	// max_alive limita o total simultaneo: sem isso um boss de cooldown curto
	// encheria a tela de adds durante uma luta longa.
	wanted := action.SummonCount
	if action.SummonMax > 0 {
		if free := action.SummonMax - boss.addsAlive(); free < wanted {
			wanted = free
		}
	}
	if wanted <= 0 {
		return
	}
	created := 0
	for i := 0; i < wanted; i++ {
		x, y := w.findFreePosition(mob.X, mob.Y, 3)
		add := &Mob{
			ID: w.allocMobID(), Def: template, X: x, Y: y,
			HP: template.Extended.MaxHP, GenerIndex: -1,
			// LeaderID aponta para o boss: e a ligacao natural do encontro.
			LeaderID: mob.ID,
		}
		w.mobs = append(w.mobs, add)
		w.publishMobSpawn(add) // registra no grid/mobsByID e materializa no client
		boss.Adds[add.ID] = struct{}{}
		created++
	}
	if action.Message != "" {
		w.announceToMobView(mob, action.Message)
	}
	log.Printf("BOSS %q: invocou %d adds %q", boss.Profile.ID, created, action.SummonNPC)
}

// bossApplyEffect aplica um affect nativo no proprio boss (escudo, enrage).
func (w *World) bossApplyEffect(boss *BossRuntime, mob *Mob, action BossActionDef) {
	if !setMobAffect(mob, mob.ID, action.AffectType, action.AffectValue,
		action.AffectLevel, action.AffectDuration) {
		return
	}
	w.publishMobAffects(mob)
	if action.Message != "" {
		w.announceToMobView(mob, action.Message)
	}
	log.Printf("BOSS %q: efeito %d aplicado", boss.Profile.ID, action.AffectType)
}

// bossRemoveEffect tira um affect do boss (queda do escudo quando os adds
// morrem, por exemplo).
func (w *World) bossRemoveEffect(boss *BossRuntime, mob *Mob, action BossActionDef) {
	if !clearMobAffect(mob, action.AffectType) {
		return
	}
	w.publishMobAffects(mob)
	if action.Message != "" {
		w.announceToMobView(mob, action.Message)
	}
	log.Printf("BOSS %q: efeito %d removido", boss.Profile.ID, action.AffectType)
}

// bossShieldAffect e o affect usado como escudo de encontro. O 19 e a
// "Imunidade" nativa: ja tem visual no client e nao e consumido por nenhuma
// regra de mob (as de mob sao 1, 3, 10 e 12).
const bossShieldAffect = 19

// bossMitigateDamage reduz o dano recebido enquanto o escudo estiver ativo.
// Value e a porcentagem absorvida (100 = imune).
//
// Existe porque NENHUM affect de mob concedia mitigacao: os quatro honrados
// hoje sao debuffs. Sem esta funcao o "escudo" seria puramente decorativo e o
// boss morreria durante a fase em que deveria estar protegido -- uma mentira de
// gameplay, nao uma simplificacao.
func (w *World) bossMitigateDamage(m *Mob, damage uint32) uint32 {
	if damage == 0 || w.bossFor(m.ID) == nil {
		return damage
	}
	affect := activeMobAffect(m, bossShieldAffect)
	if affect == nil {
		return damage
	}
	absorbed := clampInt(affect.Value, 0, 100)
	return damage * uint32(100-absorbed) / 100
}

// clearMobAffect remove um affect do mob. Devolve false se ele nao existia --
// assim uma regra nao anuncia a queda de um escudo que nunca subiu.
func clearMobAffect(m *Mob, affectType byte) bool {
	if m == nil || affectType == 0 {
		return false
	}
	removed := false
	for i := range m.Affects {
		if m.Affects[i].Type == affectType {
			m.Affects[i] = model.Affect{}
			m.AffectOwners[i] = 0
			removed = true
		}
	}
	return removed
}

// announceToMobView manda uma mensagem a quem enxerga o mob. Usa o painel
// nativo, o mesmo canal das falas de NPC.
func (w *World) announceToMobView(m *Mob, message string) {
	if message == "" {
		return
	}
	w.sendToMobView(m, func() []byte { return wire.MessagePanel(message) })
}

// npcDefByName resolve um NPC do catalogo pelo nome, aceitando a forma com
// underscore usada pelo NPCGener.
func (w *World) npcDefByName(name string) *model.NPCDef {
	wanted := generName(name)
	for i := range w.npcs {
		if generName(w.npcs[i].Name) == wanted {
			return &w.npcs[i]
		}
	}
	return nil
}
