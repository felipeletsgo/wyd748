package game

import (
	"log"
	"time"

	"wydgo/internal/wire"
)

// O TMSrv usa GetEnemyFromView enquanto o mob esta em MOB_PEACE. A aquisicao
// nativa normal ocorre numa janela 9x9 (raio Chebyshev 4); a ativacao em raio
// 16 apenas acorda a IA, sem conceder aggro ou alcance adicional.
const (
	mobAggroRange      = 4
	mobAttackRange     = 1
	mobRetentionRange  = 7
	mobLeashRange      = 16
	mobActivationRange = 16
	mobAttackInterval  = 1500 * time.Millisecond
)

// tickMobCombat porta o fluxo StandingByProcessor -> SetBattle ->
// BattleProcessor do W2PP. Como activeMobs ja contem somente monstros acordados
// por proximidade, aquisicao e perseguicao rodam a cada 1 segundo.
// O World roda em uma unica goroutine, portanto alvo, HP e posicao mudam juntos.
func (w *World) tickMobCombat(now time.Time, shard, shardCount int, allowMovement bool) {
	for _, m := range w.activeMobs {
		if shardCount > 1 && int(m.ID)%shardCount != shard {
			continue
		}
		if m.Dead || !m.Def.IsMonster() || m.SummonerID != 0 {
			continue
		}
		// Movimento, morte e desconexao atualizam o grid por evento. Esta checagem
		// tambem recolhe mobs cujo ultimo jogador proximo morreu sem se mover.
		nearbyTarget := false
		if m.InstanceID != "" {
			nearbyTarget = w.instanceMobHasNearbyMember(m, mobActivationRange)
		} else {
			nearbyTarget = len(w.nearbyPlayersInGameplaySpace(m.X, m.Y, mobActivationRange, "")) != 0
		}
		if !nearbyTarget {
			m.Awake = false
			m.TargetID = 0
			delete(w.activeMobs, m.ID)
			continue
		}
		if m.InstanceID != "" && !w.instanceMobInsideArea(m) {
			// Referencias antigas ou uma falha de movimento nao podem liberar o
			// mob para o mapa. Recolha-o para o centro da sala antes de procurar
			// um alvo novamente.
			m.TargetID = 0
			if allowMovement {
				stage, ok := instanceStageForMob(w.instanceForMob(m))
				if ok {
					w.moveMobToward(m, stage.X, stage.Y, 0, now)
				}
			}
			continue
		}
		if !m.insideHomeLeash() {
			m.TargetID = 0
			home := m.Segments[0]
			if allowMovement && home.X != 0 && home.Y != 0 {
				w.moveMobToward(m, home.X, home.Y, 0, now)
			}
			continue
		}
		var target *Player
		if m.InstanceID != "" {
			target = w.playerByID(m.TargetID)
		} else {
			target = w.playerByID(m.TargetID)
		}
		allowedTarget := w.mobCanTargetPlayer(m, target)
		if !allowedTarget || chebyshev(m.X, m.Y, target.X, target.Y) > mobRetentionRange {
			m.TargetID = 0
			if m.InstanceID != "" {
				target = w.nearestInstanceMember(m)
			} else {
				target = w.nearestLivingPlayerInGameplaySpace(m.X, m.Y, mobAggroRange, "")
			}
			if target == nil {
				continue
			}
			m.TargetID = target.ID
		}

		distance := chebyshev(m.X, m.Y, target.X, target.Y)
		if allowMovement && distance > mobAttackRange {
			w.chasePlayer(m, target, now)
		}
	}
}

// tickActiveMobActions nao decide alvos nem calcula rotas. Ele apenas cumpre
// deadlines de ataque dos mobs acordados. A percepcao roda a cada 1 s, mas o
// intervalo configurado de 1,5 s continua sendo a autoridade sobre o DPS.
func (w *World) tickActiveMobActions(now time.Time) {
	for _, m := range w.activeMobs {
		if m == nil || m.Dead || m.Def == nil || !m.Def.IsMonster() || m.SummonerID != 0 {
			if m != nil {
				m.Awake = false
				m.TargetID = 0
				delete(w.activeMobs, m.ID)
			}
			continue
		}
		if now.Before(m.NextMove) || now.Before(m.NextAttack) {
			continue
		}
		target := w.playerByID(m.TargetID)
		allowedTarget := w.mobCanTargetPlayer(m, target)
		if !allowedTarget || chebyshev(m.X, m.Y, target.X, target.Y) > mobAttackRange {
			continue
		}
		m.NextAttack = now.Add(mobAttackInterval)
		w.mobAttackPlayer(m, target, now)
	}
}

func (m *Mob) insideHomeLeash() bool {
	if m == nil {
		return false
	}
	home := m.Segments[0]
	if home.X == 0 || home.Y == 0 {
		return true
	}
	return chebyshev(home.X, home.Y, m.X, m.Y) <= mobLeashRange
}

func validMobTargetAt(p *Player, now time.Time) bool {
	return p != nil && p.InWorld && p.Char != nil && playerCurHP(p.Char) > 0 &&
		!hasActiveAffectAt(p.Char, 28, now)
}

func (m *Mob) insideLeash(p *Player) bool {
	if p == nil {
		return false
	}
	home := m.Segments[0]
	if home.X == 0 || home.Y == 0 {
		return true
	}
	return chebyshev(home.X, home.Y, p.X, p.Y) <= mobLeashRange
}

func (w *World) playerByID(id uint16) *Player {
	if id == 0 {
		return nil
	}
	return w.playersByID[id]
}

func (w *World) nearestLivingPlayer(x, y uint16, maxDistance int) *Player {
	return w.nearestLivingPlayerInGameplaySpace(x, y, maxDistance, "")
}

func (w *World) nearestLivingPlayerInGameplaySpace(x, y uint16, maxDistance int, space string) *Player {
	var best *Player
	bestDistance := maxDistance + 1
	for _, p := range w.nearbyPlayersInGameplaySpace(x, y, maxDistance, space) {
		distance := chebyshev(x, y, p.X, p.Y)
		if distance <= maxDistance && distance < bestDistance {
			best, bestDistance = p, distance
		}
	}
	return best
}

func (w *World) chasePlayer(m *Mob, target *Player, now time.Time) {
	if now.Before(m.NextMove) {
		return
	}
	w.moveMobToward(m, target.X, target.Y, mobAttackRange, now)
}

func (w *World) mobAttackPlayer(m *Mob, target *Player, now time.Time) {
	damage := w.mobHitsPlayer(m, target.Char)
	damage = absorbFlatDamage(damage, w.equipmentGemBonuses(target.Char).absorbDamage)
	// FlagLocal=0 faz o 7.48 aplicar o dano e exibir o numero flutuante. Dano
	// zero conserva a animacao de ataque e representa MISS no cliente.
	w.applyMobDamageToPlayer(m, target, damage, now, func(applied uint32) []byte {
		return wire.AttackHitExtended(m.ID, target.ID, m.X, m.Y, target.X, target.Y,
			applied, playerMaxHP(target.Char), 0, m.Def.Extended.MaxMP)
	})
}

// applyMobDamageToPlayer aplica um dano JA CALCULADO e cuida do resto: absorcao
// da montaria, pacote de animacao, morte e sincronizacao. E compartilhado pelo
// golpe comum e pela magia de boss, que so diferem no calculo e no pacote.
func (w *World) applyMobDamageToPlayer(m *Mob, target *Player, damage uint32,
	now time.Time, build func(applied uint32) []byte) {
	target.LastAttackerID = m.ID
	// Montaria adulta viva absorve 25% do dano no proprio HP.
	damage = uint32(w.absorbMountDamage(target, int(damage)))
	currentHP := playerCurHP(target.Char)
	lethal := damage >= currentHP
	if lethal {
		w.cancelTrade(target, "personagem morreu")
		w.receiveDeathLetter(target, m.Def.Name, "mob")
		w.mountRiderDied(target)
		setPlayerCurHP(target.Char, 0)
		target.DeadAt = now
		m.TargetID = 0
	} else {
		setPlayerCurHP(target.Char, currentHP-damage)
	}

	w.sendToMobView(m, func() []byte { return build(damage) })
	// AttackHit anima/aplica o dano; SetHpMp logo depois confirma o HP
	// autoritativo para o alvo e para todos os observadores.
	w.syncPlayerVitals(target)
	w.updatePartyMember(target)
	if lethal {
		// SetHpMp apenas atualiza a barra. O handler 7.48 de CNFMobKill zera o HP
		// do humano e chama TMHuman::Die(), produzindo a pose/tela de morte sem
		// remover o personagem da cena. Esta e a ordem nativa de MobKilled.
		w.publishPlayerDeath(target, m.ID)
		log.Printf("[#%d] morreu para mob id=%d %q (dmg=%d)", target.Session.ID, m.ID, m.Def.Name, damage)
	}
}
