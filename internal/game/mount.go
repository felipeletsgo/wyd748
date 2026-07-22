package game

import (
	"fmt"
	"log"
	"math/rand"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// Subsistema de montaria fiel ao W2PP. O estado (HP/level/longevidade/comida)
// vive nos slots de efeito do item (ver model/mount.go). Este arquivo concentra
// a logica de dominio: bonus de atributo, evolucao de estagio e casamento
// item->montaria dos consumiveis. O wiring de pacote fica em consumables.go.

// mountSlot e o slot de montaria preferido no nosso layout (Equip[15]=Montaria),
// usado como default. A busca real e por conteudo (equippedMount), porque o
// client 7.48 pode equipar a montaria em 14 ou 15 conforme o Pos do itemlist.
const mountSlot = 15

// equippedMount devolve o ponteiro para a montaria equipada (cria ou adulta) em
// qualquer slot de equip, e o indice do slot. nil se nao houver montaria. Ser
// agnostico ao slot evita depender do 14-vs-15 do layout.
func equippedMount(ch *model.Char) (*model.Item, int) {
	for i := range ch.Equip {
		if model.IsMount(ch.Equip[i].Index) {
			return &ch.Equip[i], i
		}
	}
	return nil, -1
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
func advanceMountStage(mount *model.Item, longevityBonus int) {
	mount.Index += model.MountTypeCount
	mount.SetMountLongev(mount.MountLongev() + rand.Intn(longevityBonus) + 1)
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
// equipada. Ajustavel: 100 de comida = ~100 min de cavalgada.
const mountFoodInterval = 60 * time.Second

// tickPlayerMounts drena a comida da montaria adulta equipada e viva. Ao zerar a
// comida, a montaria fica sem HP (fiel ao ProcessAdultMount: comida 0 -> HP 0) e
// perde longevidade; o bonus some no recalc. O affect 51 (invuln) suspende o
// dreno.
func (w *World) tickPlayerMounts(now time.Time) {
	for _, p := range w.players {
		if !p.InWorld || p.Char == nil || now.Before(p.NextMountTick) {
			continue
		}
		p.NextMountTick = now.Add(mountFoodInterval)
		// Incubacao: decrementa o delay dos ovos no inventario (1 min por tick).
		for i := range p.Char.Inv {
			egg := &p.Char.Inv[i]
			if model.IsMountEgg(egg.Index) && eggDelay(*egg) > 0 {
				setEggDelay(egg, eggDelay(*egg)-1)
				if eggDelay(*egg) == 0 && p.Session != nil {
					p.Session.Send(wire.SendItem(p.ID, placeInv, byte(i), *egg))
					p.Session.Send(wire.MessagePanel("O ovo esta pronto para o proximo Ori/Lac."))
				}
			}
		}
		// Backstop: garante o pet-cria coerente (cobre login com cria equipada).
		w.syncCriaPet(p)
		mount, mslot := equippedMount(p.Char)
		if mount == nil || !model.IsMountAdult(mount.Index) || mount.MountHP() <= 0 {
			continue
		}
		if mountInvulnActive(p.Char) {
			continue
		}
		food := mount.MountFood() - 1
		if food < 0 {
			food = 0
		}
		mount.SetMountFood(food)
		if food == 0 {
			// Faminta: perde o HP (e o bonus) e -0..3 longevidade.
			mount.SetMountHP(0)
			loseMountLongevity(mount)
			w.recalcPlayer(p.Char)
			if p.Session != nil {
				p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(mslot), *mount))
				p.Session.Send(wire.SelfEquip(p.ID, p.Char.Equip[:]))
				p.Session.Send(wire.UpdateScore(p.ID, *p.Char))
				p.Session.Send(wire.MessagePanel("Sua montaria esta faminta e parou de te ajudar."))
			}
			w.refreshAppearance(p)
		} else if p.Session != nil {
			p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(mslot), *mount))
		}
	}
}

// --- Ovo -> choco (incubacao) ---
//
// O ovo (2300-2329) NAO e montaria: usa os slots de efeito como estado de
// incubacao. Eff[0]=progresso (sucessos de Ori/Lac), Eff[1]=delay em minutos
// ate o proximo uso. Choca quando progresso > EF_INCUBATE (limiar do itemlist:
// Pig=0, Boar=1 ... Steed=6). Fiel ao handler de refino do W2PP (_MSG_UseItem
// :1116-1189), que trata o ovo como refino com limiar EF_INCUBATE + delay
// EF_INCUDELAY.

// eggHatchDelayMin/Rand: delay base + aleatorio entre usos de Ori/Lac no ovo.
// TESTE: 1-4 min (cabe num byte, testavel). FIEL ao original: ~240-540 (4-9h),
// mas ai o delay nao cabe em 1 byte -- suba estes valores + troque o byte por
// 2 bytes se quiser o tempo real.
const (
	eggHatchDelayMin  = 1
	eggHatchDelayRand = 3
)

func eggProgress(egg model.Item) int { return int(egg.Eff[0]) }
func eggDelay(egg model.Item) int    { return int(egg.Eff[1]) }
func setEggProgress(egg *model.Item, v int) {
	egg.Eff[0] = byte(clampMountByte(v))
}
func setEggDelay(egg *model.Item, v int) {
	egg.Eff[1] = byte(clampMountByte(v))
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

// destEggTarget resolve um ovo-alvo no inventario (o Ori/Lac e arrastado sobre o
// ovo guardado). Cargo/equip nao contam.
func (w *World) destEggTarget(p *Player, req useItemRequest) (*model.Item, uint32) {
	if req.dstType == placeInv && int(req.dstPos) < len(p.Char.Inv) {
		if it := &p.Char.Inv[req.dstPos]; model.IsMountEgg(it.Index) {
			return it, req.dstPos
		}
	}
	return nil, 0
}

// incubateEgg processa Ori/Lac (Vol 4/5) sobre um ovo. Se ainda ha delay, recusa
// ("aguarde"); senao rola sucesso pelo progresso atual (mesma curva do refino).
// Sucesso avanca o progresso; progresso > EF_INCUBATE -> CHOCA (ovo+30 = cria,
// estado inicializado). Consome a poeira em sucesso E falha; persiste antes de
// confirmar.
func (w *World) incubateEgg(p *Player, s *net.Session, powder *model.Item, powderSlot byte, egg *model.Item, eggSlot uint32, vol int) {
	resend := func() { s.Send(wire.SendItem(p.ID, placeInv, powderSlot, *powder)) }
	if eggDelay(*egg) > 0 {
		resend()
		s.Send(wire.MessagePanel(fmt.Sprintf("O ovo ainda esta incubando (%d min). Use o acelerador ou aguarde.", eggDelay(*egg))))
		return
	}
	def, ok := w.items[egg.Index]
	if !ok {
		resend()
		return
	}
	threshold := itemAbility(*egg, def, "EF_INCUBATE")
	success := refineRoll(eggProgress(*egg))
	oldEgg, oldPowder := *egg, *powder
	if success {
		setEggProgress(egg, eggProgress(*egg)+1)
	}
	setEggDelay(egg, eggHatchDelayMin+rand.Intn(eggHatchDelayRand+1))
	hatched := success && eggProgress(*egg) > threshold
	if hatched {
		egg.Index += model.MountTypeCount // 2300 -> 2330 (cria)
		egg.Eff = [6]byte{}
		w.initFreshMount(egg)
	}
	consumeOne(powder) // Ori/Lac sempre consomem (sucesso e falha), como no refino
	if err := w.saveAccount(p.Account); err != nil {
		*egg, *powder = oldEgg, oldPowder
		log.Printf("[#%d] ERRO ao salvar incubacao: %v", s.ID, err)
		return
	}
	resend()
	s.Send(wire.SendItem(p.ID, placeInv, byte(eggSlot), *egg))
	switch {
	case hatched:
		s.Send(wire.MessagePanel("O ovo chocou! Nasceu uma cria -- equipe-a."))
	case success:
		s.Send(wire.MessagePanel(fmt.Sprintf("Incubacao avancou (%d/%d).", eggProgress(*egg), threshold+1)))
	default:
		s.Send(wire.MessagePanel("A incubacao falhou desta vez."))
	}
	log.Printf("[#%d] incubacao vol=%d ovo->%d progresso=%d/%d chocou=%v",
		s.ID, vol, egg.Index, eggProgress(*egg), threshold+1, hatched)
}

// accelerateHatch (Vol 196, Hatch_accelerator): zera o delay de incubacao de um
// ovo no inventario, permitindo o proximo Ori/Lac imediatamente.
func (w *World) accelerateHatch(p *Player, s *net.Session, item *model.Item, invSlot byte) {
	for i := range p.Char.Inv {
		egg := &p.Char.Inv[i]
		if model.IsMountEgg(egg.Index) && eggDelay(*egg) > 0 {
			setEggDelay(egg, 0)
			consumeOne(item)
			w.saveAccountAsync(p.Account)
			s.Send(wire.SendItem(p.ID, placeInv, invSlot, *item))
			s.Send(wire.SendItem(p.ID, placeInv, byte(i), *egg))
			s.Send(wire.MessagePanel("Incubacao acelerada! Use Ori/Lac no ovo novamente."))
			log.Printf("[#%d] acelerou incubacao do ovo %d", s.ID, egg.Index)
			return
		}
	}
	s.Send(wire.SendItem(p.ID, placeInv, invSlot, *item))
	s.Send(wire.MessagePanel("Nenhum ovo aguardando incubacao."))
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
	for _, m := range w.mobs {
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
	x, y := w.findFreePosition(p.X, p.Y, 3)
	hp := cria.MountHP()
	if hp < 1 {
		hp = 1
	}
	def := &model.NPCDef{
		Name: "Cria^", Tipo: model.TipoMonstro,
		Equip: model.Equip{Rosto: model.Item{Index: face}},
		Extended: &model.ExtendedScore{
			Version:   model.ExtendedScoreVersion,
			Level:     playerLevel(p.Char),
			MaxHP:     uint32(hp),
			CurHP:     uint32(hp),
			AttackRun: 0x64,
		},
	}
	m := &Mob{ID: w.allocMobID(), Def: def, X: x, Y: y, HP: uint32(hp),
		GenerIndex: -1, SummonerID: p.ID, SummonKind: summonKindMount, SummonRange: mobAttackRange}
	w.mobs = append(w.mobs, m)
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
func (w *World) grantMountHuntExp(p *Player, m *Mob) {
	if p == nil || p.Char == nil || m == nil || m.Def == nil {
		return
	}
	mount, mslot := equippedMount(p.Char)
	if mount == nil || !model.IsMountBaby(mount.Index) {
		return
	}
	mobLevel := 0
	if m.Def.Extended != nil {
		mobLevel = int(m.Def.Extended.Level)
	}
	if mobLevel < mount.MountLevel() {
		return // mob fraco demais nao ensina a cria
	}
	if prog := int(mount.Eff[5]) + 1; prog < criaKillsPerLevel {
		mount.Eff[5] = byte(prog)
		return
	}
	mount.Eff[5] = 0
	level := mount.MountLevel() + 1
	if level > model.MountMaxLevel {
		level = model.MountMaxLevel
	}
	mount.SetMountLevel(level)
	evolved := false
	if th := mountStageThreshold(mount.Index); th > 0 && level >= th {
		advanceMountStage(mount, 14)
		evolved = true
	}
	w.recalcPlayer(p.Char)
	if p.Session != nil {
		p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(mslot), *mount))
		p.Session.Send(wire.SelfEquip(p.ID, p.Char.Equip[:]))
		p.Session.Send(wire.UpdateScore(p.ID, *p.Char))
		if evolved {
			p.Session.Send(wire.MessagePanel("Sua cria cresceu de estagio!"))
		} else {
			p.Session.Send(wire.MessagePanel(fmt.Sprintf("Sua cria subiu para o level %d.", level)))
		}
	}
	w.refreshAppearance(p)
	if evolved {
		w.syncCriaPet(p) // virou adulta -> remove o pet
	}
}

// mountInvulnActive diz se o affect 51 (protecao da montaria) esta ativo.
func mountInvulnActive(ch *model.Char) bool {
	now := time.Now()
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
func loseMountLongevity(mount *model.Item) {
	mount.SetMountLongev(mount.MountLongev() - rand.Intn(4))
}

// absorbMountDamage aplica a absorcao da montaria adulta viva: o cavaleiro toma
// 75% do dano ((dam*3)>>2) e a montaria tanka 25% no proprio HP. Ao zerar o HP,
// a montaria fica ferida (perde o bonus de stat e -0..3 longevidade). Devolve o
// dano que o cavaleiro efetivamente recebe. Fiel a _MSG_Attack.cpp:2419.
func (w *World) absorbMountDamage(target *Player, incoming int) int {
	if incoming <= 0 || target == nil || target.Char == nil {
		return incoming
	}
	mount, mslot := equippedMount(target.Char)
	if mount == nil || !model.IsMountAdult(mount.Index) || mount.MountHP() <= 0 {
		return incoming
	}
	rider := incoming * 3 / 4
	if rider < 1 {
		rider = 1
	}
	if mountInvulnActive(target.Char) {
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
		loseMountLongevity(mount)
		w.recalcPlayer(target.Char)
		if target.Session != nil {
			target.Session.Send(wire.SendItem(target.ID, placeEquip, byte(mslot), *mount))
			target.Session.Send(wire.SelfEquip(target.ID, target.Char.Equip[:]))
			target.Session.Send(wire.UpdateScore(target.ID, *target.Char))
			w.refreshAppearance(target) // o 0x336 apaga a tintura; reasseverar (evento raro)
			target.Session.Send(wire.MessagePanel("Sua montaria foi ferida!"))
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
	loseMountLongevity(mount)
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
// persiste antes de confirmar. Invuln nao muta o item -- aplica um affect.
func (w *World) applyMountItem(p *Player, s *net.Session, item *model.Item, invSlot byte, rule model.VolatileRule, code int) {
	resend := func() { s.Send(wire.SendItem(p.ID, placeInv, invSlot, *item)) }

	if rule.MountAction == "hatch" {
		// Acelerador (Vol 196): atua num OVO do inventario, nao na montaria.
		w.accelerateHatch(p, s, item, invSlot)
		return
	}

	if rule.MountAction == "invuln" {
		// Pocao de recuperacao/invulnerabilidade (Vol 90-92): affect 51 por
		// rule.DurationUnits. Enquanto ativo, o tick nao drena HP/comida (fiel ao
		// ProcessAdultMount, que trata comida como 100 sob o affect 51).
		if !setAffect(p.Char, 51, 0, 0, rule.DurationUnits) {
			resend()
			s.Send(wire.MessagePanel("Protecao da montaria ja esta ativa."))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		w.recalcPlayer(p.Char)
		w.publishPlayerAffects(p)
		w.syncPlayerVitals(p)
		resend()
		log.Printf("[#%d] mount invuln (affect 51) item=%d dur=%d", s.ID, item.Index, rule.DurationUnits)
		return
	}

	mount, mslot := equippedMount(p.Char)
	if mount == nil {
		resend()
		s.Send(wire.MessagePanel("Equipe a montaria para usar isto nela."))
		return
	}

	old, oldItem := *mount, *item
	usedIndex := item.Index // o consumo pode zerar o Index; loga o real
	var ok bool
	var msg string
	switch rule.MountAction {
	case "essence":
		ok, msg = mountEssence(mount, item.Index)
	case "feed":
		ok, msg = mountFeed(mount, item.Index)
	case "longevity":
		ok, msg = mountLongevityRecover(mount)
	case "growth":
		ok, msg = mountGrowth(mount, item.Index)
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
		return
	}
	w.recalcPlayer(p.Char)
	resend()
	s.Send(wire.SendItem(p.ID, placeEquip, byte(mslot), *mount))
	s.Send(wire.SelfEquip(p.ID, p.Char.Equip[:]))
	s.Send(wire.UpdateScore(p.ID, *p.Char))
	w.syncPlayerVitals(p)
	w.refreshAppearance(p)
	w.syncCriaPet(p) // cria pode ter virado adulta (some) ou continuar cria
	if msg != "" {
		s.Send(wire.MessagePanel(msg))
	}
	log.Printf("[#%d] mount %s item=%d montaria=%d lvl=%d hp=%d comida=%d long=%d",
		s.ID, rule.MountAction, usedIndex, mount.Index, mount.MountLevel(), mount.MountHP(), mount.MountFood(), mount.MountLongev())
}

// mountEssence porta o handler do amago (Vol 16): restaura HP a 20000, sobe 1
// level. Em ADULTA rola a chance de sucesso (g_pSancRate[2]); na falha ha chance
// de reduzir 1 level (so level par: 20% <50, 40% 50-99, 70% >=100). Ao cruzar o
// limite de estagio, evolui (sIndex+=30).
func mountEssence(mount *model.Item, essenceIndex uint16) (bool, string) {
	if !essenceMatchesMount(essenceIndex, mount.Index) {
		return false, "Este amago nao corresponde a montaria."
	}
	mount.SetMountHP(model.MountEssenceHP)
	level := mount.MountLevel()
	if model.IsMountAdult(mount.Index) {
		if level >= model.MountMaxLevel {
			return false, "A montaria ja esta no level maximo."
		}
		if rand.Intn(100) > mountSuccessRate(level) {
			if level%2 == 0 {
				r2 := rand.Intn(100)
				if (level < 50 && r2 < 20) || (level >= 50 && level < 100 && r2 < 40) || (level >= 100 && r2 < 70) {
					mount.SetMountLevel(level - 1)
					return true, "Falha: o level da montaria caiu."
				}
			}
			return true, "Falha na evolucao da montaria."
		}
	}
	level++
	mount.SetMountLevel(level)
	if th := mountStageThreshold(mount.Index); th > 0 && level >= th {
		advanceMountStage(mount, 14)
		return true, "Sua montaria cresceu de estagio!"
	}
	return true, "Sua montaria subiu de level."
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
	mount.SetMountLongev(rand.Intn(21) + 10) // 10..30
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
		return false, "Esta racao nao corresponde a montaria."
	}
	if mount.MountHP() <= 0 {
		if mount.MountLongev() <= 0 {
			return false, "A montaria perdeu a longevidade e nao pode reviver."
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
func mountLongevityRecover(mount *model.Item) (bool, string) {
	long := mount.MountLongev()
	if long < 1 || long >= model.MountMaxLongevity {
		return false, "Longevidade da montaria no maximo."
	}
	long += rand.Intn(3) + 1
	if long > model.MountMaxLongevity {
		long = model.MountMaxLongevity
	}
	mount.SetMountLongev(long)
	return true, "Longevidade da montaria restaurada."
}

// mountGrowth porta a pocao de crescimento (Vol 94): evolui o estagio na hora
// (sIndex+=30), sem depender de level. A categoria da pocao (3344+) deve casar
// com a familia da montaria.
func mountGrowth(mount *model.Item, potionIndex uint16) (bool, string) {
	cat := int(potionIndex) - 3344
	if cat < 0 || growthCategory(mount.Index) != cat {
		return false, "Esta pocao nao corresponde a montaria."
	}
	advanceMountStage(mount, 20)
	return true, "Sua montaria cresceu de estagio!"
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
