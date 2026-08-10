package game

import (
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type summonTemplate struct {
	name                    string
	face                    uint16
	maxSummons              int
	baseDefense, baseAttack int
	baseHP                  int
	minDamage, maxDamage    int
	minDefense, maxDefense  int
	minHP, maxHP            int
}

var summonTemplates = [...]summonTemplate{
	{"Condor", 206, 7, 15, 70, 60, 40, 150, 50, 75, 100, 400},
	{"Javali", 225, 6, 20, 70, 100, 40, 125, 50, 150, 125, 400},
	{"Wolf", 226, 6, 40, 80, 7000, 40, 200, 50, 125, 125, 400},
	{"Urso", 227, 5, 60, 100, 7000, 40, 200, 50, 200, 150, 400},
	{"Tigre", 244, 5, 30, 150, 100, 40, 250, 50, 175, 150, 400},
	{"Gorila", 245, 4, 45, 100, 200, 40, 225, 50, 250, 175, 400},
	{"Dragao Negro", 307, 3, 80, 100, 350, 50, 250, 50, 250, 174, 400},
	{"Succubus", 396, 2, 30, 250, 200, 70, 250, 60, 200, 180, 250},
}

func summonCount(instanceValue, mastery int) int {
	idx := instanceValue - 1
	if idx < 0 || idx >= len(summonTemplates) {
		return 0
	}
	mastery = clampInt(mastery, 0, 255)
	limit := summonTemplates[idx].maxSummons
	// Toda evocacao comeca com uma criatura. Os pontos 0..255 distribuem
	// linearmente as vagas restantes; mastery 255 atinge exatamente o limite.
	return 1 + mastery*(limit-1)/255
}

// removePlayerSummons despawna TODAS as evocacoes de um dono. Obrigatorio quando
// o dono deixa o mundo (logout/desconexao): sem isto as evocacoes ficavam vivas e
// orfas e, como o ID de player e REUSADO (menor slot livre), passavam a seguir o
// proximo jogador que entrasse com o mesmo ID. Evocacao pertence exclusivamente a
// quem a invocou.
func (w *World) removePlayerSummons(ownerID uint16) {
	if ownerID == 0 {
		return
	}
	var summons []*Mob
	for _, m := range w.summons {
		if m != nil && m.SummonerID == ownerID {
			summons = append(summons, m)
		}
	}
	for _, m := range summons {
		for _, viewer := range w.players {
			w.hideMob(viewer, m, 0)
		}
		m.Dead = true
		w.removeMobInstance(m)
	}
}

func (w *World) removeContractSummons(ownerID uint16) {
	var found []*Mob
	for _, m := range w.summons {
		if m != nil && m.SummonerID == ownerID && m.SummonKind == summonKindContract {
			found = append(found, m)
		}
	}
	for _, m := range found {
		for _, viewer := range w.players {
			w.hideMob(viewer, m, 0)
		}
		m.Dead = true
		w.removeMobInstance(m)
	}
}

// rebindSummonGameplaySpace closes the small transition window in which a
// player enters/leaves a private instance while an already materialized
// summon still carries the previous public/runtime identity.
func (w *World) rebindSummonGameplaySpace(summon *Mob, space string) {
	if summon == nil || summon.InstanceID == space {
		return
	}
	for _, viewer := range w.players {
		w.hideMob(viewer, summon, 0)
	}
	summon.InstanceID = space
	w.publishMobSpawn(summon)
}

type contractSummonPlan struct {
	owner    *Player
	previous []*Mob
	summon   *Mob
}

// planContractSummon valida e materializa a substituicao sem alterar o World.
// A operacao de item pode entao persistir o consumo sabendo que nao existe mais
// nenhum ponto de falha na criacao da entidade.
func (w *World) planContractSummon(owner *Player, t *model.VolatileSummon) (*contractSummonPlan, bool) {
	if owner == nil || t == nil || t.Face == 0 || t.HP == 0 {
		return nil, false
	}
	x, y := w.findFreeGameplayPosition(owner, nil, owner.X, owner.Y, 3)
	if !w.terrain.Walkable(x, y) {
		return nil, false
	}
	attackRun := byte(clampInt(int(t.MoveSpeed), 1, 15))
	def := &model.NPCDef{
		Name: t.Name + "^", Tipo: model.TipoMonstro,
		Equip: model.Equip{Rosto: model.Item{Index: t.Face}, Arma: model.Item{Index: t.Weapon}, Escudo: model.Item{Index: t.Shield}},
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, Level: playerLevel(owner.Char),
			Attack: t.Attack, Defense: t.Defense, MaxHP: t.HP, MaxMP: 100, AttackRun: attackRun},
	}
	def.Extended.CurHP, def.Extended.CurMP = t.HP, 100
	mobID := w.allocMobID()
	if mobID == 0 {
		return nil, false
	}
	m := &Mob{ID: mobID, Def: def, X: x, Y: y, HP: t.HP, GenerIndex: -1,
		InstanceID: w.playerRuntimeInstanceID(owner.ID),
		SummonerID: owner.ID, SummonKind: summonKindContract, SummonRange: t.AttackRange}
	plan := &contractSummonPlan{owner: owner, summon: m}
	for _, existing := range w.summons {
		if existing != nil && existing.SummonerID == owner.ID &&
			existing.SummonKind == summonKindContract {
			plan.previous = append(plan.previous, existing)
		}
	}
	return plan, true
}

func (w *World) commitContractSummon(plan *contractSummonPlan) {
	if plan == nil || plan.owner == nil || plan.summon == nil {
		return
	}
	for _, existing := range plan.previous {
		if existing == nil || w.mobsByID[existing.ID] != existing {
			continue
		}
		for _, viewer := range w.players {
			w.hideMob(viewer, existing, 0)
		}
		existing.Dead = true
		w.removeMobInstance(existing)
	}
	m := plan.summon
	w.appendMobInstance(m)
	w.publishMobSpawn(m)
}

func (w *World) castSummon(owner *Player, skill model.SkillDef, mastery int) bool {
	idx := skill.InstanceValue - 1
	if owner == nil || idx < 0 || idx >= len(summonTemplates) {
		return false
	}
	template := summonTemplates[idx]
	space := w.playerRuntimeInstanceID(owner.ID)
	wanted := summonCount(skill.InstanceValue, mastery)
	current := 0
	obsolete := make([]*Mob, 0)
	for _, m := range w.summons {
		if m.SummonerID != owner.ID || m.Dead || m.SummonKind == summonKindContract || m.SummonKind == summonKindMount {
			continue
		}
		if m.Def.Equip.Rosto.Index != template.face {
			for _, viewer := range w.players {
				w.hideMob(viewer, m, 0)
			}
			m.Dead = true
			obsolete = append(obsolete, m)
			continue
		}
		w.rebindSummonGameplaySpace(m, space)
		oldX, oldY := m.X, m.Y
		m.X, m.Y = w.findFreeGameplayPosition(owner, nil, owner.X, owner.Y, 3)
		w.publishMobMove(m, oldX, oldY, uint32(m.Def.Extended.AttackRun&0x0f))
		current++
	}
	for _, m := range obsolete {
		w.removeMobInstance(m)
	}
	created := 0
	for current < wanted {
		x, y := w.findFreeGameplayPosition(owner, nil, owner.X, owner.Y, 3)
		base := playerInt(owner.Char) + playerCon(owner.Char)
		attack := template.baseAttack + base*template.minDamage/60 + mastery*template.maxDamage/60
		defense := template.baseDefense + base*template.minDefense/100 + mastery*template.maxDefense/100
		hp := template.baseHP + base*template.minHP/70 + mastery*template.maxHP/70
		def := &model.NPCDef{
			Name: template.name + "^", Tipo: model.TipoMonstro,
			Equip: model.Equip{Rosto: model.Item{Index: template.face}},
			Extended: &model.ExtendedScore{
				Version:   model.ExtendedScoreVersion,
				Level:     playerLevel(owner.Char),
				Defense:   uint32(clampInt(defense, 0, int(maxExtendedStat))),
				Attack:    uint32(clampInt(attack, 0, int(maxExtendedStat))),
				MaxHP:     uint32(clampInt(hp, 1, int(maxExtendedStat))),
				MaxMP:     100,
				Int:       uint32(playerInt(owner.Char)),
				Con:       uint32(playerCon(owner.Char)),
				AttackRun: 0x64,
			},
		}
		def.Extended.CurHP, def.Extended.CurMP = def.Extended.MaxHP, def.Extended.MaxMP
		mobID := w.allocMobID()
		if mobID == 0 {
			log.Printf("[#%d] invocacao %q interrompida: faixa de IDs de mob esgotada", owner.Session.ID, template.name)
			break
		}
		m := &Mob{ID: mobID, Def: def, X: x, Y: y, HP: def.Extended.MaxHP,
			InstanceID: space,
			GenerIndex: -1, LeaderID: 0, SummonerID: owner.ID, SummonKind: summonKindBM, SummonRange: mobAttackRange}
		w.appendMobInstance(m)
		w.publishMobSpawn(m)
		current++
		created++
	}
	log.Printf("[#%d] invocou %q quantidade=%d/%d", owner.Session.ID, template.name, current, wanted)
	return created > 0 || current > 0
}

type summonCombatTarget struct {
	id   uint16
	x, y uint16
	mob  *Mob
	user *Player
}

const summonCommandRange = 30

func (w *World) summonTarget(owner *Player, id uint16) summonCombatTarget {
	if owner == nil || id == 0 || id == owner.ID {
		return summonCombatTarget{}
	}
	if id >= 1000 {
		m := w.mobByID(id)
		if m == nil || m.HP == 0 || !w.playerCanInteractWithMob(owner, m) ||
			chebyshev(owner.X, owner.Y, m.X, m.Y) > summonCommandRange {
			return summonCombatTarget{}
		}
		return summonCombatTarget{id: id, x: m.X, y: m.Y, mob: m}
	}
	p := w.playerByID(id)
	if !validMobTargetAt(p, w.now()) || p == owner || !w.playersShareGameplaySpace(owner, p) ||
		p.Party != nil && p.Party == owner.Party ||
		chebyshev(owner.X, owner.Y, p.X, p.Y) > summonCommandRange {
		return summonCombatTarget{}
	}
	return summonCombatTarget{id: id, x: p.X, y: p.Y, user: p}
}

func (w *World) ownerSummonTarget(owner *Player) summonCombatTarget {
	if target := w.summonTarget(owner, owner.CombatTargetID); target.id != 0 {
		return target
	}
	owner.CombatTargetID = 0
	if target := w.summonTarget(owner, owner.LastAttackerID); target.id != 0 {
		return target
	}
	owner.LastAttackerID = 0
	return summonCombatTarget{}
}

func (w *World) moveSummonToward(summon *Mob, x, y uint16, stopDistance int, now time.Time) {
	if now.Before(summon.NextMove) {
		return
	}
	w.moveMobToward(summon, x, y, stopDistance, now)
}

func (w *World) tickSummonCombat(now time.Time) {
	for _, summon := range w.summons {
		if summon == nil || summon.Dead || summon.HP == 0 || summon.SummonerID == 0 {
			continue
		}
		owner := w.playerByID(summon.SummonerID)
		if owner == nil || !owner.InWorld {
			continue
		}
		w.rebindSummonGameplaySpace(summon, w.playerRuntimeInstanceID(owner.ID))
		if summon.SummonKind == summonKindThornWall {
			continue
		}
		// Pet-cria da montaria: passivo, apenas acompanha o dono, nunca ataca.
		if summon.SummonKind == summonKindMount || summon.Def != nil && isCriaPetFace(summon.Def.Equip.Rosto.Index) {
			if chebyshev(summon.X, summon.Y, owner.X, owner.Y) > 3 {
				w.moveSummonToward(summon, owner.X, owner.Y, 3, now)
			}
			continue
		}
		target := w.ownerSummonTarget(owner)
		if target.id == 0 {
			summon.TargetID = 0
			// Sem ordem de combate, a evocacao apenas acompanha o BM e nao
			// inicia aggro por conta propria.
			if chebyshev(summon.X, summon.Y, owner.X, owner.Y) > 3 {
				w.moveSummonToward(summon, owner.X, owner.Y, 3, now)
			}
			continue
		}
		summon.TargetID = target.id
		attackRange := summon.SummonRange
		if attackRange <= 0 {
			attackRange = mobAttackRange
		}
		if chebyshev(summon.X, summon.Y, target.x, target.y) > attackRange {
			w.moveSummonToward(summon, target.x, target.y, attackRange, now)
			continue
		}
		if !w.combatLineOfSight(summon.X, summon.Y, target.x, target.y) {
			w.moveSummonToward(summon, target.x, target.y, 1, now)
			continue
		}
		if now.Before(summon.NextMove) {
			continue
		}
		if now.Before(summon.NextAttack) {
			continue
		}
		summon.NextAttack = now.Add(mobAttackInterval)
		if target.mob != nil {
			damage := uint32(clampInt(int(summon.Def.Extended.Attack)-
				int(target.mob.Def.Extended.Defense)/2, 1, int(maxExtendedStat)))
			oldHP := target.mob.HP
			if damage >= target.mob.HP {
				target.mob.HP = 0
			} else {
				target.mob.HP -= damage
			}
			w.sendToMobView(summon, func() []byte {
				return wire.AttackHitExtended(summon.ID, target.mob.ID, summon.X, summon.Y, target.mob.X, target.mob.Y,
					damage, target.mob.Def.Extended.MaxHP, 0, summon.Def.Extended.MaxMP)
			})
			if target.mob.HP == 0 {
				w.killMobState(owner, target.mob, damage, minU32(damage, oldHP))
			}
			continue
		}
		damage := w.mobHitsPlayer(summon, target.user.Char)
		damage = uint32(w.absorbMountDamage(target.user, int(damage)))
		w.cancelTrade(target.user, "personagem foi atacado")
		currentHP := playerCurHP(target.user.Char)
		if damage >= currentHP {
			w.receiveDeathLetter(target.user, owner.Char.Name, "jogador")
			w.mountRiderDied(target.user)
			setPlayerCurHP(target.user.Char, 0)
			target.user.DeadAt = now
			w.applyPvPKills(owner, target.user)
		} else {
			setPlayerCurHP(target.user.Char, currentHP-damage)
		}
		target.user.LastAttackerID = owner.ID
		w.sendToPlayerView(target.user, func() []byte {
			return wire.AttackHitExtended(summon.ID, target.user.ID, summon.X, summon.Y, target.user.X, target.user.Y,
				damage, playerMaxHP(target.user.Char), 0, summon.Def.Extended.MaxMP)
		})
		w.syncPlayerVitals(target.user)
		w.updatePartyMember(target.user)
		if playerCurHP(target.user.Char) == 0 {
			w.publishPlayerDeath(target.user, summon.ID)
		}
	}
}
