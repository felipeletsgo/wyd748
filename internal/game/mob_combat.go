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
		if len(w.nearbyPlayers(m.X, m.Y, mobActivationRange)) == 0 {
			m.Awake = false
			m.TargetID = 0
			delete(w.activeMobs, m.ID)
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
		target := w.playerByID(m.TargetID)
		if !validMobTarget(target) || !m.insideLeash(target) ||
			chebyshev(m.X, m.Y, target.X, target.Y) > mobRetentionRange {
			m.TargetID = 0
			target = w.nearestLivingPlayer(m.X, m.Y, mobAggroRange)
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
		if !validMobTarget(target) || chebyshev(m.X, m.Y, target.X, target.Y) > mobAttackRange {
			continue
		}
		m.NextAttack = now.Add(mobAttackInterval)
		w.mobAttackPlayer(m, target, now)
	}
}

func (w *World) mobHasActivePlayer(m *Mob) bool {
	if m == nil {
		return false
	}
	_, ok := w.activeMobs[m.ID]
	return ok
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

func validMobTarget(p *Player) bool {
	return p != nil && p.InWorld && p.Char != nil && playerCurHP(p.Char) > 0 && !hasActiveAffect(p.Char, 28)
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
	var best *Player
	bestDistance := maxDistance + 1
	for _, p := range w.nearbyPlayers(x, y, maxDistance) {
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
	target.LastAttackerID = m.ID
	damage := mobHitsPlayer(m, target.Char)
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

	// FlagLocal=0 faz o 7.48 aplicar o dano e exibir o numero flutuante. Dano
	// zero conserva a animacao de ataque e representa MISS no cliente.
	w.sendToMobView(m, func() []byte {
		return wire.AttackHitExtended(m.ID, target.ID, m.X, m.Y, target.X, target.Y,
			damage, 0, m.Def.Extended.MaxMP)
	})
	// AttackHit anima/aplica o dano; SetHpMp logo depois confirma o HP
	// autoritativo para o alvo e para todos os observadores.
	w.syncPlayerVitals(target)
	w.updatePartyMember(target)
	if lethal {
		// SetHpMp apenas atualiza a barra. O handler 7.48 de CNFMobKill zera o HP
		// do humano e chama TMHuman::Die(), produzindo a pose/tela de morte sem
		// remover o personagem da cena. Esta e a ordem nativa de MobKilled.
		w.sendToPlayerView(target, func() []byte { return wire.CNFMobKill(target.ID, m.ID, target.Char.Exp) })
		log.Printf("[#%d] morreu para mob id=%d %q (dmg=%d)", target.Session.ID, m.ID, m.Def.Name, damage)
	}
}
