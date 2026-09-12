package game

import (
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// Subsistema de montaria fiel ao W2PP. O estado (HP/level/longevidade/comida)
// vive nos slots de efeito do item (ver model/mount.go). Este arquivo concentra
// a logica de dominio: bonus de atributo, evolucao de estagio e casamento
// item->montaria dos consumiveis. O wiring de pacote fica em consumables.go.

// mountSlot e o slot nativo de montaria (Equip[14]). Equip[15] e reservado a
// capa, inclusive as medalhas que definem Akelonia/Hekalotia.
const mountSlot = 14

// equippedMount accepts only the canonical 7.48 mount slot. Invalid persisted
// layouts must be rejected/migrated at the boundary instead of granting a
// hidden mount bonus from another equipment slot.
func equippedMount(ch *model.Char) (*model.Item, int) {
	if ch == nil || !model.IsMount(ch.Equip[mountSlot].Index) {
		return nil, -1
	}
	return &ch.Equip[mountSlot], mountSlot
}

// mountStageThreshold devolve o level em que uma CRIA evolui para o proximo
// estagio (sIndex+=30). Porta os limites do handler de amago do W2PP: crias
// 2330-2335 evoluem em 25/35/45/55/65/75; as demais (2336+) em 100.
func mountStageThreshold(sIndex uint16) int {
	if !model.IsMountBaby(sIndex) {
		return 0
	}
	off := int(sIndex - model.MountBabyBase)
	if off <= 5 {
		return 25 + off*10
	}
	return 100
}

// mountBonus devolve o incremento que a montaria adulta equipada concede ao
// cavaleiro e o piso de velocidade. Fiel a BASE_GetItemAbility(EF_MOUNTLIFE):
// dano += (level+20)*attack/100; magia += (level+15)*magic/100; evasao/resist
// flat; velocidade vira piso. So conta com HP > 0 (montaria viva).
func (w *World) mountBonus(ch *model.Char) (attack, magic, evasion, resist, runSpeedFloor int) {
	mount, _ := equippedMount(ch)
	if mount == nil || !model.IsMountAdult(mount.Index) || mount.MountHP() <= 0 {
		return 0, 0, 0, 0, 0
	}
	stats, ok := w.mounts.Stats(model.MountType(mount.Index))
	if !ok {
		return 0, 0, 0, 0, 0
	}
	level := mount.MountLevel()
	attack = (level + 20) * stats.Attack / 100
	magic = (level + 15) * stats.Magic / 100
	evasion = stats.Evasion
	resist = stats.Resist
	runSpeedFloor = stats.Speed
	return
}

type premiumMountStats struct {
	attack       int
	magicPercent int
	runSpeed     int
}

func (w *World) premiumMountBonus(ch *model.Char, now time.Time) premiumMountStats {
	if w == nil || ch == nil {
		return premiumMountStats{}
	}
	item := ch.Equip[mountSlot]
	def, ok := w.items[item.Index]
	if !ok || timedItemDuration(def, item.Index, mountSlot) <= 0 ||
		item.ExpiresUnix != 0 && item.ExpiresUnix <= now.Unix() {
		return premiumMountStats{}
	}
	return premiumMountStats{
		attack:       staticAbility(def, "EF_DAMAGE"),
		magicPercent: staticAbility(def, "EF_MAGIC"),
		runSpeed:     staticAbility(def, "EF_RUNSPEED"),
	}
}

// mountMaxHP devolve o teto de HP da montaria pelo tipo (data-driven), com
// fallback no teto global.
func (w *World) mountMaxHP(sIndex uint16) int {
	if stats, ok := w.mounts.Stats(model.MountType(sIndex)); ok && stats.MaxHP > 0 {
		return stats.MaxHP
	}
	return model.MountMaxHP
}

// advanceMountStage evolui a cria para o proximo estagio: sIndex += 30 (novo
// mesh cria->adulta), longevidade += rand(1..bonus), level zera. Fiel ao amago
// (bonus 14) e a pocao de crescimento (bonus 20).
func advanceMountStage(mount *model.Item, longevityBonus int, intn func(int) int) {
	mount.Index += model.MountTypeCount
	mount.SetMountLongev(mount.MountLongev() + intn(longevityBonus) + 1)
	mount.SetMountLevel(0)
	mount.SetMountFood(mount.MountFood()) // preserva comida; W2PP zera o flag stEffect[2].cValue
	mount.Eff[5] = 0
}

// essenceMatchesMount confirma que o amago (2390+) casa com o tipo da montaria.
// amgIndex = (essence-2390)%30 deve igualar o tipo da montaria.
func essenceMatchesMount(essenceIndex, mountIndex uint16) bool {
	if essenceIndex < 2390 {
		return false
	}
	amg := int(essenceIndex-2390) % model.MountTypeCount
	return amg == model.MountType(mountIndex)
}

// feedMatchesMount confirma que a racao casa com a montaria. Porta o remapeamento
// do handler Vol 15: varios tipos compartilham a mesma familia de racao.
func feedMatchesMount(feedIndex, mountIndex uint16) bool {
	mount := int(mountIndex-model.MountBabyBase) % model.MountTypeCount
	if mountIndex >= model.MountAdultBase {
		mount = int(mountIndex-model.MountAdultBase) % model.MountTypeCount
	}
	switch {
	case (mount >= 6 && mount <= 15) || mount == 27:
		mount = 6
	case mount == 19:
		mount = 7
	case mount == 20:
		mount = 8
	case mount == 21 || mount == 22 || mount == 23 || mount == 28:
		mount = 9
	case mount == 24 || mount == 25 || mount == 26:
		mount = 10
	case mount == 29:
		mount = 19
	}
	var feed int
	if feedIndex >= 3367 {
		feed = int(feedIndex-3367) % model.MountTypeCount
	} else {
		feed = int(feedIndex-2420) % model.MountTypeCount
	}
	return mount == feed
}

// mountFoodInterval e o intervalo de dreno de comida da montaria adulta enquanto
// o personagem permanece online com ela equipada, viva e sem invulnerabilidade.
// Cada hora completa elegivel consome exatamente 1 de Food.
const mountFoodInterval = time.Hour

// tickPlayerMounts drena a comida da montaria adulta equipada e viva. Ao zerar a
// comida, a montaria fica sem HP (fiel ao ProcessAdultMount: comida 0 -> HP 0) e
// perde longevidade; o bonus some no recalc. O affect 51 (invuln) suspende o
// dreno.
func (w *World) tickPlayerMounts(now time.Time) {
	for _, p := range w.players {
		if !p.InWorld || p.Char == nil {
			continue
		}

		// Incubacao possui deadline proprio e a cria precisa ser sincronizada
		// independentemente do relogio horario de Food.
		w.tickEquippedEggIncubation(p, now)
		w.syncCriaPet(p)

		mount, mslot := equippedMount(p.Char)
		if mount == nil || !model.IsMountAdult(mount.Index) || mount.MountHP() <= 0 ||
			mountInvulnActiveAt(p.Char, now) {
			// Tempo desequipado, morto ou protegido nao conta como uma fracao da
			// proxima hora elegivel.
			p.NextMountTick = time.Time{}
			continue
		}
		if p.NextMountTick.IsZero() {
			p.NextMountTick = now.Add(mountFoodInterval)
			continue
		}
		if now.Before(p.NextMountTick) {
			continue
		}
		p.NextMountTick = now.Add(mountFoodInterval)
		food := mount.MountFood() - 1
		if food < 0 {
			food = 0
		}
		mount.SetMountFood(food)
		if food == 0 {
			// Faminta: perde o HP (e o bonus) e -0..3 longevidade.
			mount.SetMountHP(0)
			loseMountLongevity(mount, w.intn)
			w.recalcPlayer(p.Char)
			if p.Session != nil {
				p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(mslot), *mount))
				p.Session.Send(playerScorePacket(p))
				p.Session.Send(wire.MessagePanel("Your mount is starving and stopped helping you."))
			}
			w.refreshAppearance(p)
		} else if p.Session != nil {
			p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(mslot), *mount))
		}
	}
}

// --- Ovo -> choco (incubacao) ---
//
// O ovo (2300-2329) usa o formato nativo do STRUCT_ITEM:
//
//	EF_SANC(43)      = refinacoes bem-sucedidas da incubacao
//	EF_INCUDELAY(84) = horas ONLINE restantes ate a proxima tentativa
//
// EF_INCUBATE vem exclusivamente do itemlist.csv e e o valor critico do ovo.
// Um ovo de valor 3 recebe +1, espera, +2, espera, +3, espera; a proxima
// refinacao bem-sucedida o transforma em cria. A hora so corre com o mesmo ovo
// equipado no slot 14.
const (
	eggIncubationDelayEffect = 84
	eggHatchDelayMinHours    = 6
	eggHatchDelayMaxHours    = 8
	eggIncubationHour        = time.Hour
)

func eggProgress(egg model.Item) int { return itemSanc(egg) }

func eggDelay(egg model.Item) int {
	for i := 0; i < 3; i++ {
		if egg.Eff[i*2] == eggIncubationDelayEffect {
			return int(egg.Eff[i*2+1])
		}
	}
	return 0
}

func setEggProgress(egg *model.Item, v int) bool {
	return setItemSanc(egg, clampMountByte(v))
}

func setEggDelay(egg *model.Item, v int) bool {
	if egg == nil {
		return false
	}
	v = clampMountByte(v)
	for i := 0; i < 3; i++ {
		if egg.Eff[i*2] == eggIncubationDelayEffect {
			egg.Eff[i*2+1] = byte(v)
			return true
		}
	}
	// Secrets/RegenMob e W2PP decrementam diretamente stEffect[2].cValue;
	// mantenha EF_INCUDELAY no terceiro par mesmo havendo lacuna no segundo.
	if egg.Eff[4] == 0 {
		egg.Eff[4] = eggIncubationDelayEffect
		egg.Eff[5] = byte(v)
		return true
	}
	return false
}

func clampMountByte(v int) int {
	if v < 0 {
		return 0
	}
	if v > 255 {
		return 255
	}
	return v
}

func eggIncubationClockKey(egg *model.Item) string {
	if egg == nil {
		return ""
	}
	if egg.UID != "" {
		return egg.UID
	}
	// Itens materializados possuem UID. O fallback existe somente para contas
	// legadas/testes e continua seguro porque ha um unico slot de incubacao.
	return fmt.Sprintf("egg:%d", egg.Index)
}

func (w *World) resetEggIncubationClock(p *Player) {
	if p == nil {
		return
	}
	p.EggIncubationUID = ""
	p.NextEggIncubationTick = time.Time{}
}

func (w *World) startEggIncubationClock(p *Player, egg *model.Item, now time.Time) {
	if p == nil || egg == nil || eggDelay(*egg) <= 0 {
		w.resetEggIncubationClock(p)
		return
	}
	p.EggIncubationUID = eggIncubationClockKey(egg)
	p.NextEggIncubationTick = now.Add(eggIncubationHour)
}

// tickEquippedEggIncubation debita somente horas completas, online e com o
// mesmo ovo no slot 14. O valor inteiro e persistido no item; a fracao da hora
// fica em Player e se perde ao desequipar/deslogar, exatamente como nos guias
// e no contador de sessao do W2PP.
func (w *World) tickEquippedEggIncubation(p *Player, now time.Time) {
	if p == nil || p.Char == nil {
		return
	}
	if !p.InWorld {
		w.resetEggIncubationClock(p)
		return
	}
	egg := &p.Char.Equip[mountSlot]
	remaining := eggDelay(*egg)
	if !model.IsMountEgg(egg.Index) || remaining <= 0 {
		w.resetEggIncubationClock(p)
		return
	}
	key := eggIncubationClockKey(egg)
	if p.EggIncubationUID != key || p.NextEggIncubationTick.IsZero() {
		w.startEggIncubationClock(p, egg, now)
		return
	}
	if now.Before(p.NextEggIncubationTick) {
		return
	}
	hours := int(now.Sub(p.NextEggIncubationTick)/eggIncubationHour) + 1
	if hours > remaining {
		hours = remaining
	}
	oldEgg := *egg
	if !setEggDelay(egg, remaining-hours) {
		return
	}
	if p.Account != nil {
		if err := w.saveAccount(p.Account); err != nil {
			*egg = oldEgg
			log.Printf("[#%d] ERRO ao salvar hora de incubacao do ovo %d: %v", p.ID, egg.Index, err)
			return
		}
	}
	p.NextEggIncubationTick = p.NextEggIncubationTick.Add(time.Duration(hours) * eggIncubationHour)
	if eggDelay(*egg) == 0 {
		w.resetEggIncubationClock(p)
	}
	if p.Session != nil {
		p.Session.Send(wire.SendItem(p.ID, placeEquip, mountSlot, *egg))
		if eggDelay(*egg) == 0 {
			p.Session.Send(wire.MessagePanel("Incubation is complete. Apply Ori/Lac again to continue hatching."))
		} else {
			p.Session.Send(wire.MessagePanel(fmt.Sprintf("Egg incubation: %d hour(s) remaining.", eggDelay(*egg))))
		}
	}
}

// destEggTarget exige o ovo equipado no slot nativo de montaria. Inventario e
// Cargo pausam a incubacao e nao aceitam Ori/Lac.
func (w *World) destEggTarget(p *Player, req useItemRequest) (*model.Item, uint32, uint32) {
	if p != nil && p.Char != nil && req.dstType == placeEquip && req.dstPos == mountSlot {
		if it := &p.Char.Equip[mountSlot]; model.IsMountEgg(it.Index) {
			return it, placeEquip, mountSlot
		}
	}
	return nil, 0, 0
}

// incubateEgg processa Ori/Lac (Vol 4/5) sobre o ovo equipado. A tentativa usa
// a curva nativa de refino, consome a poeira em sucesso e falha e inicia uma
// espera aleatoria de 6..8 horas online. Ao ja estar no valor critico, a proxima
// tentativa bem-sucedida transforma o ovo em cria.
func (w *World) incubateEgg(p *Player, s *net.Session, powder *model.Item, powderSlot byte,
	egg *model.Item, eggType, eggSlot uint32, vol int) {
	resend := func() { s.Send(wire.SendItem(p.ID, placeInv, powderSlot, *powder)) }
	if eggDelay(*egg) > 0 {
		resend()
		s.Send(wire.MessagePanel(fmt.Sprintf("The egg is still incubating (%d hour(s) remaining).", eggDelay(*egg))))
		return
	}
	def, ok := w.items[egg.Index]
	if !ok {
		resend()
		return
	}
	threshold := staticAbility(def, "EF_INCUBATE")
	if threshold < 0 || threshold > 15 {
		resend()
		return
	}
	progressBefore := eggProgress(*egg)
	roll := w.rollPercent(refineChance(progressBefore))
	success := roll.Success
	oldEgg, oldPowder := *egg, *powder
	hatched := success && progressBefore >= threshold
	if hatched {
		egg.Index += model.MountTypeCount // 2300 -> 2330 (cria)
		egg.Eff = [6]byte{}
		w.initFreshMount(egg)
	} else {
		if success && !setEggProgress(egg, progressBefore+1) {
			*egg = oldEgg
			resend()
			s.Send(wire.MessagePanel("The egg has no free effect slot for incubation."))
			return
		}
		delay := eggHatchDelayMinHours + w.intn(eggHatchDelayMaxHours-eggHatchDelayMinHours+1)
		if !setEggDelay(egg, delay) {
			*egg = oldEgg
			resend()
			s.Send(wire.MessagePanel("The egg has no free effect slot for incubation."))
			return
		}
	}
	consumeOne(powder) // Ori/Lac sempre consomem (sucesso e falha), como no refino
	if err := w.saveAccount(p.Account); err != nil {
		*egg, *powder = oldEgg, oldPowder
		log.Printf("[#%d] ERRO ao salvar incubacao: %v", s.ID, err)
		resend()
		s.Send(wire.SendItem(p.ID, byte(eggType), byte(eggSlot), *egg))
		s.Send(wire.MessagePanel("Save failed. Reconnect to reload the authoritative state."))
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "egg incubation", err)
		return
	}
	resend()
	s.Send(wire.SendItem(p.ID, byte(eggType), byte(eggSlot), *egg))
	s.Send(wire.MessagePanel(roll.message()))
	switch {
	case hatched:
		w.resetEggIncubationClock(p)
		w.recalcPlayer(p.Char)
		w.syncCriaPet(p)
		w.refreshAppearance(p)
		s.Send(wire.MessagePanel("The egg hatched! A hatchling was born."))
	case success:
		w.startEggIncubationClock(p, egg, w.now())
		s.Send(wire.MessagePanel(fmt.Sprintf("Egg incubation advanced (+%d). Wait %d hour(s).",
			eggProgress(*egg), eggDelay(*egg))))
	default:
		w.startEggIncubationClock(p, egg, w.now())
		s.Send(wire.MessagePanel(fmt.Sprintf("Refinement failed. Wait %d hour(s) before trying again.", eggDelay(*egg))))
	}
	log.Printf("[#%d] incubacao vol=%d ovo->%d progresso=+%d critico=%d espera=%dh chocou=%v roll=%d/%d",
		s.ID, vol, egg.Index, eggProgress(*egg), threshold, eggDelay(*egg), hatched, roll.Roll, roll.Chance)
}

// accelerateHatch (Vol 196) porta o item 3438 do W2PP: usado sobre o ovo
// equipado, transforma-o imediatamente em cria. O consumo e a transformacao
// sao confirmados no mesmo save.
func (w *World) accelerateHatch(p *Player, s *net.Session, item *model.Item, invSlot byte,
	req useItemRequest) {
	resend := func() { s.Send(wire.SendItem(p.ID, placeInv, invSlot, *item)) }
	egg, eggType, eggSlot := w.destEggTarget(p, req)
	if egg == nil {
		resend()
		s.Send(wire.MessagePanel("Use the hatch accelerator on an equipped egg."))
		return
	}
	oldItem, oldEgg := *item, *egg
	egg.Index += model.MountTypeCount
	egg.Eff = [6]byte{}
	w.initFreshMount(egg)
	consumeOne(item)
	if err := w.saveAccount(p.Account); err != nil {
		*item, *egg = oldItem, oldEgg
		log.Printf("[#%d] ERRO ao salvar acelerador do ovo %d: %v", s.ID, oldEgg.Index, err)
		resend()
		s.Send(wire.SendItem(p.ID, byte(eggType), byte(eggSlot), *egg))
		s.Send(wire.MessagePanel("Save failed. The accelerator was not consumed."))
		return
	}
	w.resetEggIncubationClock(p)
	w.recalcPlayer(p.Char)
	resend()
	s.Send(wire.SendItem(p.ID, byte(eggType), byte(eggSlot), *egg))
	s.Send(playerScorePacket(p))
	w.syncCriaPet(p)
	w.refreshAppearance(p)
	s.Send(wire.MessagePanel("The hatch accelerator transformed the egg into a hatchling."))
	log.Printf("[#%d] acelerador chocou ovo %d -> cria %d", s.ID, oldEgg.Index, egg.Index)
}

// --- Cria como pet que segue o dono ---
//
// No W2PP a CRIA (2330-2358) e evocada como um mob que acompanha o dono (a
// adulta voce monta). O pet tem o rosto 315+tipo (face 315-344), reusa a infra
// de evocacao (SummonerID) e o tick de evocacao ja o faz seguir; aqui ele e
// PASSIVO (nunca ataca). Ver Server.cpp MountProcess/LinkMountHp.

const (
	criaPetFaceBase = 315
	criaPetFaceEnd  = criaPetFaceBase + model.MountTypeCount // 345
)

func isCriaPetFace(face uint16) bool {
	return face >= criaPetFaceBase && face < criaPetFaceEnd
}

// findCriaPet devolve o pet-cria vivo de um dono, se houver.
func (w *World) findCriaPet(ownerID uint16) *Mob {
	for _, m := range w.summons {
		if m == nil || m.Dead || m.SummonerID != ownerID || m.Def == nil {
			continue
		}
		if isCriaPetFace(m.Def.Equip.Rosto.Index) {
			return m
		}
	}
	return nil
}

// spawnCriaPet cria o mob seguidor da cria equipada.
func (w *World) spawnCriaPet(p *Player, cria *model.Item) {
	mt := model.MountType(cria.Index)
	if mt < 0 {
		return
	}
	face := uint16(criaPetFaceBase + mt)
	x, y := w.findFreeGameplayPosition(p, nil, p.X, p.Y, 3)
	hp := cria.MountHP()
	if hp < 1 {
		hp = 1
	}
	def := &model.NPCDef{
		Name: "Cria^", Tipo: model.TipoMonstro,
		Equip: model.Equip{Rosto: model.Item{Index: face}},
		Score: &model.Score{
			Version:   model.ScoreVersion,
			Level:     playerLevel(p.Char),
			MaxHP:     uint32(hp),
			CurHP:     uint32(hp),
			AttackRun: 0x64,
		},
	}
	mobID := w.allocMobID()
	if mobID == 0 {
		log.Printf("[#%d] cria-pet nao nasceu: faixa de IDs de mob esgotada", p.ID)
		return
	}
	m := &Mob{ID: mobID, Def: def, X: x, Y: y, HP: uint32(hp),
		InstanceID: w.playerRuntimeInstanceID(p.ID),
		GenerIndex: -1, SummonerID: p.ID, SummonKind: summonKindMount, SummonRange: mobAttackRange}
	w.appendMobInstance(m)
	w.publishMobSpawn(m)
	log.Printf("[#%d] cria-pet nasceu face=%d", p.ID, face)
}

// removeCriaPet retira o pet da cena.
func (w *World) removeCriaPet(m *Mob) {
	for _, viewer := range w.players {
		w.hideMob(viewer, m, 0)
	}
	m.Dead = true
	w.removeMobInstance(m)
}

// syncCriaPet reconcilia o pet com o estado atual: nasce se ha cria viva
// equipada e nenhum pet; some se nao ha cria (desequipou, morreu ou virou
// adulta). Chamado apos qualquer mudanca de montaria.
func (w *World) syncCriaPet(p *Player) {
	if p == nil || p.Char == nil || !p.InWorld {
		return
	}
	mount, _ := equippedMount(p.Char)
	hasCria := mount != nil && model.IsMountBaby(mount.Index) && mount.MountHP() > 0
	existing := w.findCriaPet(p.ID)
	switch {
	case hasCria && existing == nil:
		w.spawnCriaPet(p, mount)
	case hasCria && existing != nil:
		w.rebindSummonGameplaySpace(existing, w.playerRuntimeInstanceID(p.ID))
	case !hasCria && existing != nil:
		w.removeCriaPet(existing)
	}
}

// --- XP de caca da cria ---

// criaKillsPerLevel: mortes de mob (level >= level da cria) para subir 1 level.
// TESTE: 2. Ajuste ao gosto (o blog fala em "growth value + 100" de exp).
const criaKillsPerLevel = 2

// grantMountHuntExp da XP de caca a cria equipada quando o dono mata um mob de
// level >= o level da cria (fiel ao blog: a cria sobe 1..99 cacando). O
// progresso acumula em Eff[5]; ao atingir criaKillsPerLevel sobe 1 level e evolui
// de estagio no limite. Montaria ADULTA nao ganha XP de caca (sobe por amago).
type mountHuntUpdate struct {
	player  *Player
	slot    int
	level   int
	changed bool
	evolved bool
}

func (w *World) grantMountHuntExp(p *Player, m *Mob) {
	w.publishMountHuntUpdate(w.applyMountHuntExp(p, m))
}

func (w *World) applyMountHuntExp(p *Player, m *Mob) mountHuntUpdate {
	result := mountHuntUpdate{player: p, slot: -1}
	if p == nil || p.Char == nil || m == nil || m.Def == nil {
		return result
	}
	mount, mslot := equippedMount(p.Char)
	if mount == nil || !model.IsMountBaby(mount.Index) {
		return result
	}
	mobLevel := 0
	if m.Def.Score != nil {
		mobLevel = int(m.Def.Score.Level)
	}
	if mobLevel < mount.MountLevel() {
		return result // mob fraco demais nao ensina a cria
	}
	result.slot, result.changed = mslot, true
	if prog := int(mount.Eff[5]) + 1; prog < criaKillsPerLevel {
		mount.Eff[5] = byte(prog)
		return result
	}
	mount.Eff[5] = 0
	level := mount.MountLevel() + 1
	if level > model.MountMaxLevel {
		level = model.MountMaxLevel
	}
	mount.SetMountLevel(level)
	evolved := false
	if th := mountStageThreshold(mount.Index); th > 0 && level >= th {
		advanceMountStage(mount, 14, w.intn)
		evolved = true
	}
	w.recalcPlayer(p.Char)
	result.level, result.evolved = level, evolved
	return result
}

func (w *World) publishMountHuntUpdate(result mountHuntUpdate) {
	p := result.player
	if !result.changed || p == nil || p.Char == nil || p.Session == nil {
		return
	}
	mount, slot := equippedMount(p.Char)
	if mount == nil || slot != result.slot {
		return
	}
	if p.Session != nil {
		p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(result.slot), *mount))
		p.Session.Send(playerScorePacket(p))
		if result.evolved {
			p.Session.Send(wire.MessagePanel("Your hatchling grew to the next stage!"))
		} else if result.level > 0 {
			p.Session.Send(wire.MessagePanel(fmt.Sprintf("Your hatchling reached level %d.", result.level)))
		}
	}
	w.refreshAppearance(p)
	if result.evolved {
		w.syncCriaPet(p) // virou adulta -> remove o pet
	}
}

// mountInvulnActive diz se o affect 51 (protecao da montaria) esta ativo.
func mountInvulnActiveAt(ch *model.Char, now time.Time) bool {
	for i := range ch.Affects {
		if ch.Affects[i].Type == 51 && ch.Affects[i].ExpiresAt.After(now) {
			return true
		}
	}
	return false
}

// loseMountLongevity tira 0..3 pontos de longevidade (na morte da montaria,
// comida zerada ou morte do cavaleiro). Fiel ao W2PP (rand%4). A 0 a montaria
// morre de vez e precisa reviver no mestre de montaria.
func loseMountLongevity(mount *model.Item, intn func(int) int) {
	mount.SetMountLongev(mount.MountLongev() - intn(4))
}

// absorbMountDamage aplica a absorcao da montaria adulta viva. A tabela nativa
// g_pMountBonus[30][6] usa uma parcela de dano do cavaleiro por tipo (65..75),
// em vez de uma absorcao fixa para todas as montarias. Ao zerar o HP, a
// montaria fica ferida (perde o bonus de stat e -0..3 longevidade). Devolve o
// dano que o cavaleiro efetivamente recebe. O bonus de nivel 120 documentado
// no guia KR ainda nao foi atribuido a esta fronteira sem evidencia do fluxo.
func (w *World) absorbMountDamage(target *Player, incoming int) int {
	if incoming <= 0 || target == nil || target.Char == nil {
		return incoming
	}
	mount, mslot := equippedMount(target.Char)
	if mount == nil || !model.IsMountAdult(mount.Index) || mount.MountHP() <= 0 {
		return incoming
	}
	riderPct := 75
	if stats, ok := w.mounts.Stats(model.MountType(mount.Index)); ok &&
		stats.RiderDamagePct > 0 {
		riderPct = stats.RiderDamagePct
	}
	rider := incoming * riderPct / 100
	if rider < 1 {
		rider = 1
	}
	if mountInvulnActiveAt(target.Char, w.now()) {
		return rider // affect 51: montaria nao perde HP
	}
	absorbed := incoming - rider
	hp := mount.MountHP() - absorbed
	if hp < 0 {
		hp = 0
	}
	mount.SetMountHP(hp)
	if hp == 0 {
		// Ferida: perde longevidade, o bonus some (recalc) e o dono ve o estado.
		loseMountLongevity(mount, w.intn)
		w.recalcPlayer(target.Char)
		if target.Session != nil {
			target.Session.Send(wire.SendItem(target.ID, placeEquip, byte(mslot), *mount))
			target.Session.Send(playerScorePacket(target))
			w.refreshAppearance(target)
			target.Session.Send(wire.MessagePanel("Your mount was wounded!"))
		}
	}
	return rider
}

// mountRiderDied tira longevidade da montaria quando o cavaleiro morre (fiel ao
// W2PP: morte do dono custa 0..3 de longevidade a montaria).
func (w *World) mountRiderDied(target *Player) {
	if target == nil || target.Char == nil {
		return
	}
	mount, mslot := equippedMount(target.Char)
	if mount == nil || !model.IsMountAdult(mount.Index) {
		return
	}
	loseMountLongevity(mount, w.intn)
	if target.Session != nil {
		target.Session.Send(wire.SendItem(target.ID, placeEquip, byte(mslot), *mount))
	}
}

// mountGrowthRate porta g_pSancRate[2] (amagos): chance de sucesso do amago por
// level/10 (0..11). level 0-9 = 100%, cai ate 20% em 110-119.
var mountGrowthRate = [12]int{100, 90, 85, 75, 65, 55, 50, 40, 35, 30, 25, 20}

func mountSuccessRate(level int) int {
	idx := level / 10
	if idx < 0 {
		idx = 0
	}
	if idx >= len(mountGrowthRate) {
		idx = len(mountGrowthRate) - 1
	}
	return mountGrowthRate[idx]
}

// applyMountItem trata os consumiveis de montaria (rule.MountAction). Fiel ao
// W2PP: cada um valida o casamento item->montaria, muta o estado no item e
// persiste antes de confirmar ao client.
func (w *World) applyMountItem(p *Player, s *net.Session, item *model.Item, invSlot byte,
	rule model.VolatileRule, code int, req useItemRequest) {
	resend := func() { s.Send(wire.SendItem(p.ID, placeInv, invSlot, *item)) }

	if rule.MountAction == "hatch" {
		w.accelerateHatch(p, s, item, invSlot, req)
		return
	}

	mount, mslot := equippedMount(p.Char)
	if mount == nil {
		resend()
		s.Send(wire.MessagePanel("Equip the mount to use this on it."))
		return
	}

	old, oldItem := *mount, *item
	usedIndex := item.Index // o consumo pode zerar o Index; loga o real
	var ok bool
	var msg string
	var rolls []percentRoll
	switch rule.MountAction {
	case "essence":
		var outcome mountEssenceOutcome
		outcome = w.mountEssence(mount, item.Index)
		ok, msg, rolls = outcome.OK, outcome.Message, outcome.Rolls
	case "feed":
		ok, msg = mountFeed(mount, item.Index)
	case "longevity":
		ok, msg = mountLongevityRecoverWithRNG(mount, w.intn)
	case "longevity_restore":
		ok, msg = mountLongevityRestore(mount, rule.Amount)
	case "level_set":
		ok, msg = mountSetLevel(mount, rule.MountMinLevel, rule.Amount)
	case "growth":
		ok, msg = mountGrowthWithRNG(mount, item.Index, w.intn)
	default:
		resend()
		log.Printf("[#%d] mountAction desconhecida %q item=%d", s.ID, rule.MountAction, item.Index)
		return
	}
	if !ok {
		resend()
		if msg != "" {
			s.Send(wire.MessagePanel(msg))
		}
		return
	}
	if rule.Consume {
		consumeOne(item)
	}
	if err := w.saveAccount(p.Account); err != nil {
		*mount, *item = old, oldItem
		log.Printf("[#%d] ERRO ao salvar mount %s: %v", s.ID, rule.MountAction, err)
		resend()
		s.Send(wire.SendItem(p.ID, placeEquip, byte(mslot), *mount))
		s.Send(wire.MessagePanel("Save failed. Reconnect to reload the authoritative state."))
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "mount item", err)
		return
	}
	w.recalcPlayer(p.Char)
	resend()
	s.Send(wire.SendItem(p.ID, placeEquip, byte(mslot), *mount))
	s.Send(playerScorePacket(p))
	w.syncPlayerVitals(p)
	w.refreshAppearance(p)
	w.syncCriaPet(p) // cria pode ter virado adulta (some) ou continuar cria
	for i, roll := range rolls {
		message := roll.message()
		if i > 0 {
			message = roll.namedMessage("Penalty")
		}
		s.Send(wire.MessagePanel(message))
	}
	if msg != "" {
		s.Send(wire.MessagePanel(msg))
	}
	log.Printf("[#%d] mount %s item=%d montaria=%d lvl=%d hp=%d comida=%d long=%d",
		s.ID, rule.MountAction, usedIndex, mount.Index, mount.MountLevel(), mount.MountHP(), mount.MountFood(), mount.MountLongev())
}

// mountEssenceOutcome carrega as decisoes percentuais que precisam ser
// publicadas somente depois da persistencia. Rolls[0] e o crescimento adulto;
// Rolls[1], quando existe, e a penalidade por falha em level par.
type mountEssenceOutcome struct {
	OK      bool
	Message string
	Rolls   []percentRoll
}

// mountEssence porta o handler do amago (Vol 16): restaura HP a 20000, sobe 1
// level. Em ADULTA rola a chance de sucesso (g_pSancRate[2]); na falha ha chance
// de reduzir 1 level (so level par: 20% <50, 40% 50-99, 70% >=100). Toda chance
// usa 1..100 e sucesso em roll <= chance.
func (w *World) mountEssence(mount *model.Item, essenceIndex uint16) mountEssenceOutcome {
	if !essenceMatchesMount(essenceIndex, mount.Index) {
		return mountEssenceOutcome{Message: "This essence does not match the mount."}
	}
	mount.SetMountHP(model.MountEssenceHP)
	level := mount.MountLevel()
	var rolls []percentRoll
	if model.IsMountAdult(mount.Index) {
		if level >= model.MountMaxLevel {
			return mountEssenceOutcome{Message: "The mount is already at maximum level."}
		}
		growth := w.rollPercent(mountSuccessRate(level))
		rolls = append(rolls, growth)
		if !growth.Success {
			if level%2 == 0 {
				penaltyChance := 70
				if level < 50 {
					penaltyChance = 20
				} else if level < 100 {
					penaltyChance = 40
				}
				penalty := w.rollPercent(penaltyChance)
				rolls = append(rolls, penalty)
				if penalty.Success {
					mount.SetMountLevel(level - 1)
					return mountEssenceOutcome{
						OK: true, Message: "Failure: the mount lost a level.", Rolls: rolls,
					}
				}
			}
			return mountEssenceOutcome{OK: true, Message: "The mount evolution failed.", Rolls: rolls}
		}
	}
	level++
	mount.SetMountLevel(level)
	if th := mountStageThreshold(mount.Index); th > 0 && level >= th {
		advanceMountStage(mount, 14, w.intn)
		return mountEssenceOutcome{OK: true, Message: "Your mount grew to the next stage!", Rolls: rolls}
	}
	return mountEssenceOutcome{OK: true, Message: "Your mount gained a level.", Rolls: rolls}
}

// initShopMounts inicializa o estado das montarias que os NPCs vendem (HP,
// comida, longevidade), no carregamento. Sem isso elas apareceriam e seriam
// compradas MORTAS (Eff zerado = HP 0). Roda apos as options (w.mounts pronto).
func (w *World) initShopMounts() {
	for i := range w.npcs {
		vende := w.npcs[i].Vende
		for j := range vende {
			if model.IsMount(vende[j].Index) {
				w.initFreshMount(&vende[j])
			}
		}
	}
}

// initFreshMount inicializa uma montaria recem-obtida (Eff todo zerado): HP
// cheio, comida cheia e longevidade inicial 10..30 (fiel ao nascimento da cria).
// Sem isso, uma montaria concedida com estado zerado nasceria "morta" (HP 0).
// Devolve true se inicializou.
func (w *World) initFreshMount(mount *model.Item) bool {
	if mount == nil || !model.IsMount(mount.Index) {
		return false
	}
	if mount.MountHP() != 0 || mount.MountLevel() != 0 || mount.MountLongev() != 0 || mount.MountFood() != 0 {
		return false // ja inicializada
	}
	mount.SetMountHP(w.mountMaxHP(mount.Index))
	mount.SetMountLongev(w.intn(21) + 10) // 10..30
	if model.IsMountAdult(mount.Index) {
		mount.SetMountFood(model.MountMaxFood) // adulta: teto 100
	} else {
		mount.SetMountFood(30) // cria: teto 30
	}
	return true
}

// mountFeed porta o handler da racao (Vol 15): HP += 5000 (teto 30000) e comida
// += 2 (teto 100). Em montaria MORTA (HP 0) com longevidade, a racao REVIVE com
// 100 HP + 5 de comida (valores do mestre de montaria do W2PP) -- atalho ao NPC
// de revive, que ainda nao existe.
func mountFeed(mount *model.Item, feedIndex uint16) (bool, string) {
	if !feedMatchesMount(feedIndex, mount.Index) {
		return false, "This feed does not match the mount."
	}
	if mount.MountHP() <= 0 {
		if mount.MountLongev() <= 0 {
			return false, "The mount lost its longevity and cannot be revived."
		}
		mount.SetMountHP(100)
		mount.SetMountFood(5)
		return true, "Montaria revivida (100 HP, 5 de comida)."
	}
	hp := mount.MountHP() + 5000
	if hp > model.MountMaxHP {
		hp = model.MountMaxHP
	}
	mount.SetMountHP(hp)
	food := mount.MountFood() + 2
	if food > model.MountMaxFood {
		food = model.MountMaxFood
	}
	mount.SetMountFood(food)
	return true, "Montaria alimentada."
}

// mountLongevityRecover porta o Vol 93: longevidade += 1..3 (teto 60). Recusa se
// ja estiver no maximo ou zerada (montaria morta -> precisa reviver primeiro).
func mountLongevityRecoverWithRNG(mount *model.Item, intn func(int) int) (bool, string) {
	long := mount.MountLongev()
	if long < 1 || long >= model.MountMaxLongevity {
		return false, "Mount longevity is already at maximum."
	}
	long += intn(3) + 1
	if long > model.MountMaxLongevity {
		long = model.MountMaxLongevity
	}
	mount.SetMountLongev(long)
	return true, "Mount longevity restored."
}

func mountLongevityRecover(mount *model.Item) (bool, string) {
	return mountLongevityRecoverWithRNG(mount, realRNG{}.Intn)
}

// mountLongevityRestore porta o item 3315 do client 7.48. O itemhelp.dat e o
// handler do Secrets concordam que ele restaura LP (longevidade), nao HP nem
// invulnerabilidade. Montaria com LP zero permanece irrecuperavel e o teto e 60.
func mountLongevityRestore(mount *model.Item, amount int) (bool, string) {
	if mount == nil || !model.IsMountAdult(mount.Index) {
		return false, "This potion can only be used on an adult mount."
	}
	long := mount.MountLongev()
	if long <= 0 {
		return false, "This mount has no remaining life points."
	}
	if long >= model.MountMaxLongevity {
		return false, "Mount life points are already at maximum."
	}
	long += amount
	if long > model.MountMaxLongevity {
		long = model.MountMaxLongevity
	}
	mount.SetMountLongev(long)
	return true, "The mount recovered 1 life point."
}

// mountSetLevel porta os itens 3316/3317 confirmados no Secrets: o primeiro
// leva uma montaria adulta abaixo de 100 ao level 100; o segundo aceita apenas
// level 100..119 e a leva ao 120. Ambos exigem pelo menos 3 LP.
func mountSetLevel(mount *model.Item, minLevel, targetLevel int) (bool, string) {
	if mount == nil || !model.IsMountAdult(mount.Index) {
		return false, "This catalyst can only be used on an adult mount."
	}
	level := mount.MountLevel()
	if mount.MountLongev() < 3 || level < minLevel || level >= targetLevel {
		return false, "This mount does not meet the catalyst requirements."
	}
	mount.SetMountLevel(targetLevel)
	return true, fmt.Sprintf("The mount reached level %d.", targetLevel)
}

// mountGrowth porta a pocao de crescimento (Vol 94): evolui o estagio na hora
// (sIndex+=30), sem depender de level. A categoria da pocao (3344+) deve casar
// com a familia da montaria.
func mountGrowthWithRNG(mount *model.Item, potionIndex uint16, intn func(int) int) (bool, string) {
	cat := int(potionIndex) - 3344
	if cat < 0 || growthCategory(mount.Index) != cat {
		return false, "This potion does not match the mount."
	}
	advanceMountStage(mount, 20, intn)
	return true, "Your mount grew to the next stage!"
}

func mountGrowth(mount *model.Item, potionIndex uint16) (bool, string) {
	return mountGrowthWithRNG(mount, potionIndex, realRNG{}.Intn)
}

// growthCategory mapeia o sIndex da montaria para a categoria da pocao de
// crescimento (item 3344+ -> cat = sIndex-3344). Porta o handler Vol 94.
func growthCategory(mountIndex uint16) int {
	m := int(mountIndex)
	switch {
	case m >= 2333 && m <= 2335:
		return 0 // Kapel: Dente de Sabre, Dragao Menor, Urso
	case (m >= 2336 && m <= 2338) || (m >= 2341 && m <= 2343) || (m >= 2357 && m <= 2358):
		return 1 // Acuban
	case m == 2339 || m == 2340 || (m >= 2344 && m <= 2345):
		return 2 // Menkar
	case m >= 2346 && m <= 2348:
		return 3 // Birago
	case m >= 2351 && m <= 2353:
		return 4 // Yus
	case m >= 2354 && m <= 2356:
		return 5 // Makav
	case m >= 2349 && m <= 2350:
		return 6 // Alperath
	}
	return -1
}
