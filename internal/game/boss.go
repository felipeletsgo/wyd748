package game

import (
	"log"
	"time"
)

// boss.go -- runtime e registro dos bosses.
//
// ARQUITETURA (DOCS/boss_inteligente.md §2): todo boss E um Mob comum e
// continua participando de grid, visibilidade, combate, morte e pacotes como
// qualquer outro. O comportamento extra vive num BossRuntime PARALELO, indexado
// pelo ID do mob. Consequencias praticas:
//
//   - a IA de mob comum nao muda e nao paga nada por existir boss no mundo;
//   - nao ha `if mob.IsBoss` espalhado pelo servidor: a logica exclusiva fica
//     concentrada aqui e nos boss_*.go;
//   - nenhuma goroutine por boss -- o World continua sendo o unico executor.

// BossPendingAction e uma acao ja aceita, aguardando o instante de execucao.
type BossPendingAction struct {
	// Generation identifica ESTA execucao. Um callback so age se a geracao
	// ainda bate; reset, cancelamento e substituicao incrementam o contador e
	// invalidam o que estava agendado.
	Generation uint64
	ActionID   BossActionID
	TargetID   uint16
	ExecuteAt  time.Time
	Priority   int
	// Interruptible permite que uma acao de prioridade maior a substitua.
	Interruptible bool
}

// BossRuntime e o estado de comportamento de um boss. O estado FISICO (posicao,
// HP, affects) continua no Mob.
type BossRuntime struct {
	MobID   uint16
	Profile *BossProfile

	Phase BossPhaseID

	// InCombat marca que o encontro comecou. Nao volta a false: o HP do boss
	// PERMANECE onde os jogadores o deixaram, de proposito -- um chefe de HP
	// altissimo e feito para cair ao longo de horas, e restaurar vida
	// inviabilizaria o combate em varias sessoes.
	InCombat bool

	Cooldowns map[BossActionID]time.Time
	Pending   *BossPendingAction
	// ConsumedRules guarda as regras Once ja aceitas.
	ConsumedRules map[BossRuleID]struct{}

	// Generation cresce a cada acao iniciada, cancelamento ou reset.
	Generation uint64

	// Adds sao os mobs invocados por ESTE encontro, para contagem e limpeza.
	Adds map[uint16]struct{}

	// crossedThresholds evita reemitir um limiar ja atravessado enquanto o
	// encontro nao reseta.
	crossedThresholds map[int]struct{}
}

// newBossRuntime cria o runtime no estado inicial do perfil.
func newBossRuntime(mobID uint16, profile *BossProfile) *BossRuntime {
	return &BossRuntime{
		MobID:             mobID,
		Profile:           profile,
		Phase:             profile.InitialPhase,
		Cooldowns:         make(map[BossActionID]time.Time, len(profile.Actions)),
		ConsumedRules:     make(map[BossRuleID]struct{}),
		Adds:              make(map[uint16]struct{}),
		crossedThresholds: make(map[int]struct{}),
	}
}

// actionReady informa se a acao esta fora de cooldown.
func (b *BossRuntime) actionReady(actionID BossActionID, now time.Time) bool {
	ready, tracked := b.Cooldowns[actionID]
	return !tracked || !now.Before(ready)
}

// addsAlive conta os adds vivos do encontro.
func (b *BossRuntime) addsAlive() int { return len(b.Adds) }

// RegisterBoss liga um perfil a um mob ja spawnado. Devolve erro em vez de
// panicar: um perfil torto nao pode derrubar o boot do mundo inteiro.
func (w *World) RegisterBoss(mobID uint16, profile *BossProfile) error {
	if profile == nil {
		return errBossProfileNil
	}
	if profile.rulesByEvent == nil {
		if err := profile.Compile(); err != nil {
			return err
		}
	}
	mob := w.mobsByID[mobID]
	if mob == nil {
		return errBossMobMissing
	}
	if w.bosses == nil {
		w.bosses = make(map[uint16]*BossRuntime)
	}
	w.bosses[mobID] = newBossRuntime(mobID, profile)
	log.Printf("BOSS %q registrado no mob id=%d (%d regras)", profile.ID, mobID, len(profile.Rules))
	return nil
}

// UnregisterBoss remove o runtime (morte definitiva, despawn). Os adds do
// encontro sao removidos junto: sem isso ficariam orfaos no mundo.
func (w *World) UnregisterBoss(mobID uint16) {
	boss := w.bosses[mobID]
	if boss == nil {
		return
	}
	w.removeBossAdds(boss)
	delete(w.bosses, mobID)
}

// bossFor devolve o runtime de um mob, ou nil se nao for boss. E o unico ponto
// de entrada usado pelos caminhos de gameplay comuns -- uma consulta de mapa,
// barata o bastante para ficar no caminho de dano.
func (w *World) bossFor(mobID uint16) *BossRuntime {
	if len(w.bosses) == 0 {
		return nil
	}
	return w.bosses[mobID]
}

// removeBossAdds tira do mundo os adds ainda vivos do encontro. Segue o mesmo
// caminho dos summons (removePlayerSummons): esconde de quem via e so entao
// descarta a instancia, para nao deixar entrada fantasma no Visible do client.
func (w *World) removeBossAdds(boss *BossRuntime) {
	for addID := range boss.Adds {
		if mob := w.mobsByID[addID]; mob != nil && !mob.Dead {
			for _, viewer := range w.players {
				w.hideMob(viewer, mob, 0)
			}
			mob.Dead = true
			w.removeMobInstance(mob)
		}
		delete(boss.Adds, addID)
	}
}

