package game

import (
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// boss_spawn.go -- nascimento e renascimento dos bosses.
//
// Bosses NAO passam pelo NPCGener. Cada um tem posicao e respawn proprios,
// declarados no seu .lua. Do NPC do catalogo vem apenas os ASSETS (rosto,
// equipamento, atributos base), que o .lua pode sobrescrever.
//
// Isso mantem a promessa central: nenhum mob existente do mundo vira boss.

// bossSpawnState acompanha um boss configurado ao longo do tempo.
type bossSpawnState struct {
	config  model.BossConfig
	profile *BossProfile
	// def e o NPCDef proprio deste boss: uma COPIA do NPC base com os
	// atributos sobrescritos. Copia, e nao referencia, senao alterar o boss
	// alteraria todos os mobs comuns daquele NPC.
	def *model.NPCDef
	// mobID e a instancia viva; zero quando o boss esta morto.
	mobID uint16
	// respawnAt e quando renasce. Zero = sem respawn agendado.
	respawnAt time.Time
}

// WithBossCatalog injeta os bosses carregados de data/boss/.
func WithBossCatalog(catalog model.BossCatalog) WorldOption {
	return func(w *World) { w.bossCatalog = catalog }
}

// spawnConfiguredBosses monta o NPCDef de cada boss e o materializa. Roda no
// fim do boot, depois do NPCGener.
//
// Erro aqui DERRUBA O BOOT de proposito: um boss que referencia NPC inexistente
// e erro de conteudo, e falhar cedo e melhor que descobrir em producao que o
// chefe do servidor nunca nasceu.
func (w *World) spawnConfiguredBosses() error {
	if len(w.bossCatalog.Bosses) == 0 {
		return nil
	}
	w.bossSpawns = make([]*bossSpawnState, 0, len(w.bossCatalog.Bosses))
	for _, config := range w.bossCatalog.Bosses {
		base := w.npcDefByName(config.NPC)
		if base == nil {
			return fmt.Errorf("boss %q (%s): NPC base %q nao existe em data/npcs",
				config.ID, config.SourceFile, config.NPC)
		}
		// O loader de NPC ja recusa extendedScore ausente; a guarda existe para
		// que um catalogo montado a mao (teste, ferramenta) devolva erro claro
		// em vez de panicar no deref logo abaixo.
		if base.Extended == nil {
			return fmt.Errorf("boss %q (%s): NPC base %q sem extendedScore",
				config.ID, config.SourceFile, config.NPC)
		}
		profile, err := compileBossProfile(config)
		if err != nil {
			return err
		}
		state := &bossSpawnState{
			config:  config,
			profile: profile,
			def:     bossDefFrom(base, config),
		}
		w.bossSpawns = append(w.bossSpawns, state)
		if err := w.spawnBoss(state); err != nil {
			return err
		}
	}
	log.Printf("bosses: %d encontros carregados de data/boss", len(w.bossSpawns))
	return nil
}

// bossDefFrom cria o NPCDef do boss: copia do NPC base (assets) com os
// atributos do .lua por cima.
func bossDefFrom(base *model.NPCDef, config model.BossConfig) *model.NPCDef {
	def := *base // copia rasa: Equip e array, ExpReward/Gold sao escalares
	if config.Name != "" {
		def.Name = config.Name
	}
	// Um boss e sempre hostil, mesmo que o NPC base nao fosse.
	def.Tipo = model.TipoMonstro

	// Extended e ponteiro no base: copiar o valor evita que o boss altere os
	// atributos de todos os mobs daquele NPC.
	extended := *base.Extended
	def.Extended = &extended

	stats := config.Stats
	if stats.Level != nil {
		def.Extended.Level = *stats.Level
	}
	if stats.MaxHP != nil {
		def.Extended.MaxHP = *stats.MaxHP
		def.Extended.CurHP = *stats.MaxHP
	}
	if stats.Attack != nil {
		def.Extended.Attack = *stats.Attack
	}
	if stats.Defense != nil {
		def.Extended.Defense = *stats.Defense
	}
	if stats.AttackRun != nil {
		def.Extended.AttackRun = byte(*stats.AttackRun)
	}
	if stats.ExpReward != nil {
		def.ExpReward = *stats.ExpReward
	}
	if stats.Gold != nil {
		def.Gold = *stats.Gold
	}
	// O carry do NPC base nao vale para o boss: os drops especiais dele saem da
	// tabela propria (config.Drops), rolada em rollBossDrops. Zerar aqui tambem
	// evita drop duplo, porque killMobState continua chamando rollMobDrops.
	def.Carry = nil
	// Vende so faz sentido em mercador, e boss e sempre hostil. Zerar corta o
	// compartilhamento do slice com o NPC base que a copia rasa deixaria.
	def.Vende = nil
	return &def
}

// spawnBoss materializa a instancia e registra o runtime.
func (w *World) spawnBoss(state *bossSpawnState) error {
	x, y := w.findFreePosition(state.config.Spawn.X, state.config.Spawn.Y, 3)
	mob := &Mob{
		ID: w.allocMobID(), Def: state.def, X: x, Y: y,
		HP: state.def.Extended.MaxHP, GenerIndex: -1,
	}
	// Segments[0] e a "casa" usada pelo leash da IA comum.
	mob.Segments[0].X, mob.Segments[0].Y = state.config.Spawn.X, state.config.Spawn.Y
	w.mobs = append(w.mobs, mob)
	w.publishMobSpawn(mob)

	if err := w.RegisterBoss(mob.ID, state.profile); err != nil {
		return fmt.Errorf("boss %q: %w", state.config.ID, err)
	}
	state.mobID = mob.ID
	state.respawnAt = time.Time{}

	if state.config.SpawnMessage != "" {
		w.broadcast(func() []byte { return wire.MessagePanel(state.config.SpawnMessage) })
	}
	log.Printf("BOSS %q nasceu em (%d,%d) mob=%d hp=%d",
		state.config.ID, x, y, mob.ID, mob.HP)
	return nil
}

// onBossMobKilled reage a morte de um boss configurado: anuncia, agenda o
// renascimento e devolve o estado. Devolve nil se o mob nao era um boss.
func (w *World) onBossMobKilled(mobID uint16) *bossSpawnState {
	for _, state := range w.bossSpawns {
		if state.mobID != mobID {
			continue
		}
		state.mobID = 0
		if state.config.DeathMessage != "" {
			w.broadcast(func() []byte { return wire.MessagePanel(state.config.DeathMessage) })
		}
		if state.config.RespawnDelay() > 0 {
			state.respawnAt = w.now().Add(state.config.RespawnDelay())
			log.Printf("BOSS %q morreu; renasce em %s", state.config.ID, state.config.RespawnDelay())
		} else {
			log.Printf("BOSS %q morreu; sem respawn configurado", state.config.ID)
		}
		return state
	}
	return nil
}

// tickBossRespawns renasce os bosses vencidos. Chamado pelo tick do World.
func (w *World) tickBossRespawns(now time.Time) {
	for _, state := range w.bossSpawns {
		if state.mobID != 0 || state.respawnAt.IsZero() || now.Before(state.respawnAt) {
			continue
		}
		state.respawnAt = time.Time{}
		if err := w.spawnBoss(state); err != nil {
			// Respawn nao derruba o servidor: registra e tenta de novo no
			// proximo ciclo, em vez de matar o mundo por um boss.
			log.Printf("BOSS %q: falha ao renascer: %v", state.config.ID, err)
			state.respawnAt = now.Add(time.Minute)
		}
	}
}

// setItemAmount grava o EF_AMOUNT nativo (a pilha), espelhando a leitura de
// itemStackAmount. Reusa um par de efeito ja marcado como quantidade ou ocupa o
// primeiro livre; sem espaco, o item fica com uma unidade em vez de perder o
// efeito de outra coisa.
func setItemAmount(item *model.Item, amount int) {
	if item == nil || amount <= 1 {
		return
	}
	if amount > 255 {
		amount = 255 // EF_AMOUNT e um byte no wire nativo
	}
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] == effectAmount || item.Eff[i*2] == 0 {
			item.Eff[i*2] = effectAmount
			item.Eff[i*2+1] = byte(amount)
			return
		}
	}
}

// rollBossDrops entrega os drops especiais ao matador. E INDEPENDENTE do carry
// nativo (que foi zerado em bossDefFrom): a tabela vem do .lua e cada linha tem
// sua propria chance percentual, em vez do rand()%rate por slot do WYD.
//
// Reusa addToInv/spawnDrop, o mesmo caminho do drop comum: inventario cheio faz
// o item cair no chao em vez de sumir.
func (w *World) rollBossDrops(p *Player, mob *Mob, state *bossSpawnState) {
	if p == nil || p.Char == nil {
		return
	}
	for _, drop := range state.config.Drops {
		if drop.ChancePercent < 100 && w.intn(100) >= drop.ChancePercent {
			continue
		}
		item := model.Item{Index: drop.Item}
		if drop.Amount > 1 {
			setItemAmount(&item, drop.Amount)
		}
		if slot := addToInv(p.Char, item); slot >= 0 {
			p.Session.Send(wire.SendItem(p.ID, placeInv, byte(slot), p.Char.Inv[slot]))
			log.Printf("[#%d] BOSS %q dropou item=%d -> inv[%d]",
				p.Session.ID, state.config.ID, drop.Item, slot)
			continue
		}
		w.spawnDrop(mob.X, mob.Y, item)
		log.Printf("[#%d] BOSS %q dropou item=%d -> CHAO (inventario cheio)",
			p.Session.ID, state.config.ID, drop.Item)
	}
}
