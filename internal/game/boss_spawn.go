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

	w.announceBoss(x, y, state.config.SpawnMessage)
	log.Printf("BOSS %q nasceu em (%d,%d) mob=%d hp=%d",
		state.config.ID, x, y, mob.ID, mob.HP)
	return nil
}

// bossAnnounceRadius e o alcance dos avisos de boss, em tiles Chebyshev (a
// area 16x16 pedida). Anuncio de boss e informacao do ENCONTRO: quem esta do
// outro lado do mapa nao tem o que fazer com ela, e um broadcast global vira
// spam para o servidor inteiro a cada respawn.
const bossAnnounceRadius = 16

// announceBoss manda o aviso so para quem esta perto o bastante para lutar.
func (w *World) announceBoss(x, y uint16, message string) {
	if message == "" {
		return
	}
	for _, p := range w.nearbyWorldPlayers(x, y, bossAnnounceRadius) {
		p.Session.Send(wire.MessagePanel(message))
	}
}

// spawnBossAreaReward espalha a premiacao coletiva pelo chao ao redor do boss.
// E diferente dos Drops: aquilo vai para o inventario de quem deu o golpe
// final, isto fica no chao para todos que participaram recolherem.
//
// Uma unidade por celula, em aneis crescentes a partir do corpo -- empilhar
// tudo numa celula so daria a premiacao inteira a quem estivesse em cima.
func (w *World) spawnBossAreaReward(m *Mob, reward model.BossAreaReward) {
	if reward.Item == 0 || reward.Amount <= 0 {
		return
	}
	if _, ok := w.items[reward.Item]; !ok {
		log.Printf("BOSS: premiacao de area ignorada, item %d nao existe", reward.Item)
		return
	}
	ocupada := make(map[uint32]bool, reward.Amount)
	for _, g := range w.groundItems {
		ocupada[uint32(g.X)<<16|uint32(g.Y)] = true
	}
	postos := 0
	for raio := 1; raio <= bossAnnounceRadius && postos < reward.Amount; raio++ {
		for dx := -raio; dx <= raio && postos < reward.Amount; dx++ {
			for dy := -raio; dy <= raio && postos < reward.Amount; dy++ {
				// So a borda do anel: o interior ja foi coberto pelos raios
				// anteriores.
				if abs(dx) != raio && abs(dy) != raio {
					continue
				}
				x, y := int(m.X)+dx, int(m.Y)+dy
				if x < 0 || y < 0 || x > 0xFFFF || y > 0xFFFF {
					continue
				}
				chave := uint32(x)<<16 | uint32(y)
				if ocupada[chave] {
					continue
				}
				if w.spawnGroundReward(uint16(x), uint16(y), reward.Item) {
					ocupada[chave] = true
					postos++
				}
			}
		}
	}
	log.Printf("BOSS %q: premiacao de area, %d de %d unidades do item %d no chao",
		m.Def.Name, postos, reward.Amount, reward.Item)
}

// spawnGroundReward poe UMA unidade no chao na celula exata, sem o jitter do
// spawnDrop -- aqui a posicao ja foi escolhida pelo anel.
func (w *World) spawnGroundReward(x, y uint16, index uint16) bool {
	id := w.allocGroundItemID(index)
	if id == 0 {
		return false
	}
	g := &GroundItem{ID: id, Item: model.Item{Index: index}, X: x, Y: y,
		Expire: w.now().Add(groundRewardLifetime)}
	w.groundItems[id] = g
	w.publishItemSpawn(g)
	return true
}

// groundRewardLifetime da mais folga que o drop comum: a premiacao de area cai
// de uma vez e o grupo precisa de tempo para recolher tudo.
const groundRewardLifetime = 5 * time.Minute

func abs(v int) int {
	if v < 0 {
		return -v
	}
	return v
}

// onBossMobKilled reage a morte de um boss configurado: anuncia, agenda o
// renascimento e devolve o estado. Devolve nil se o mob nao era um boss.
func (w *World) onBossMobKilled(m *Mob) *bossSpawnState {
	if m == nil {
		return nil
	}
	for _, state := range w.bossSpawns {
		if state.mobID != m.ID {
			continue
		}
		state.mobID = 0
		w.announceBoss(m.X, m.Y, state.config.DeathMessage)
		w.spawnBossAreaReward(m, state.config.AreaReward)
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
