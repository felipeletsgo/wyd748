package game

import (
	"errors"
	"fmt"
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	kingdomJoinMinLevel = uint32(219) // nivel 220 exibido no client
	kingdomKnightLevel  = uint32(255) // nivel 256 exibido no client/W2PP
	kingdomLeaveCost    = uint32(16)
	kingdomWarpCooldown = 5 * time.Second
)

var (
	errKingdomAlready       = errors.New("personagem ja possui a capa adequada")
	errKingdomWrongRealm    = errors.New("personagem pertence ao reino oposto")
	errKingdomNeedLevel220  = errors.New("personagem abaixo do nivel 220")
	errKingdomNeedLevel256  = errors.New("personagem abaixo do nivel 256")
	errKingdomInvalidCape   = errors.New("capa nao participa da progressao de reino")
	errKingdomNeedCelestial = errors.New("capa Master exige personagem Celestial")
)

type kingdomCapeChange struct {
	Index    uint16
	Tier     model.KingdomCapeTier
	Preserve bool
}

func characterKingdom(ch *model.Char) byte {
	if ch == nil {
		return model.KingdomNeutral
	}
	return model.KingdomFromCape(ch.Equip[model.CapeSlot].Index)
}

func kingdomForNPC(def *model.NPCDef) byte {
	if def == nil {
		return model.KingdomNeutral
	}
	switch strings.ToLower(generName(def.Name)) {
	case "king harabard":
		return model.KingdomHekalotia
	case "king glantuar":
		return model.KingdomAkelonia
	default:
		return model.KingdomNeutral
	}
}

func isKingdomBroker(def *model.NPCDef) bool {
	return def != nil && strings.EqualFold(generName(def.Name), "Kingdom Brocker")
}

func kingdomJoinCost(kingdom byte) uint32 {
	if kingdom == model.KingdomHekalotia {
		return 4
	}
	return 15
}

func isCelestial(ch *model.Char) bool {
	if ch == nil {
		return false
	}
	return strings.EqualFold(ch.Evolution, "celestial") || strings.EqualFold(ch.Evolution, "subcelestial")
}

func kingdomUsesEmblem(ch *model.Char) bool {
	return ch != nil && !isCelestial(ch) && ch.Equip[13].Index == model.KingdomEmblem
}

// kingdomCapeForJoin porta a progressao do case KING da W2PP sem duplicar o
// reino no personagem. O primeiro clique cria/converte a capa 220; a capa
// basica 545/546 pode voltar ao mesmo rei no nivel 256 para virar 543/544.
// Elite/Hero preservam seu tier neutro e Celestial sempre recebe Master.
func kingdomCapeForJoin(ch *model.Char, kingdom byte) (kingdomCapeChange, error) {
	if kingdom != model.KingdomHekalotia && kingdom != model.KingdomAkelonia {
		return kingdomCapeChange{}, errKingdomInvalidCape
	}
	if ch == nil || ch.Extended == nil {
		return kingdomCapeChange{}, errKingdomNeedLevel220
	}
	currentIndex := ch.Equip[model.CapeSlot].Index
	currentKingdom := model.KingdomFromCape(currentIndex)
	if currentKingdom != model.KingdomNeutral && currentKingdom != kingdom {
		return kingdomCapeChange{}, errKingdomWrongRealm
	}

	if isCelestial(ch) {
		cape, _ := model.KingdomCapeAtTier(model.CapeTierMaster, kingdom)
		if currentIndex == cape {
			return kingdomCapeChange{}, errKingdomAlready
		}
		// A criacao do Celestial converte qualquer capa do seu reino para Master;
		// o rei tambem repara personagens antigos que ainda carreguem tier menor.
		return kingdomCapeChange{Index: cape, Tier: model.CapeTierMaster, Preserve: currentIndex == 3199}, nil
	}
	if ch.Extended.Level < kingdomJoinMinLevel {
		return kingdomCapeChange{}, errKingdomNeedLevel220
	}

	tier, knownTier := model.KingdomCapeTierOf(currentIndex)
	if currentKingdom == kingdom {
		if knownTier && tier == model.CapeTierBasic && ch.Extended.Level >= kingdomKnightLevel {
			cape, _ := model.KingdomCapeAtTier(model.CapeTierKnight, kingdom)
			return kingdomCapeChange{Index: cape, Tier: model.CapeTierKnight}, nil
		}
		return kingdomCapeChange{}, errKingdomAlready
	}

	if currentIndex == 0 || currentIndex == 548 {
		cape, _ := model.KingdomCapeAtTier(model.CapeTierBasic, kingdom)
		return kingdomCapeChange{Index: cape, Tier: model.CapeTierBasic, Preserve: currentIndex == 548}, nil
	}
	if !knownTier {
		return kingdomCapeChange{}, errKingdomInvalidCape
	}
	if tier == model.CapeTierMaster {
		return kingdomCapeChange{}, errKingdomNeedCelestial
	}
	if ch.Extended.Level < kingdomKnightLevel {
		return kingdomCapeChange{}, errKingdomNeedLevel256
	}
	cape, ok := model.KingdomCape(currentIndex, kingdom)
	if !ok {
		return kingdomCapeChange{}, errKingdomInvalidCape
	}
	return kingdomCapeChange{Index: cape, Tier: tier, Preserve: true}, nil
}

func sapphireCount(ch *model.Char) uint32 {
	if ch == nil {
		return 0
	}
	total := countInventoryItem(ch, model.SapphireItem)
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if ch.Inv[i].Index == model.SapphirePack {
			total += 10 // o 4131 vale dez por SLOT, nao por EF_AMOUNT interno
		}
	}
	return total
}

// consumeSapphires usa primeiro safiras avulsas e depois pacotes de dez. Como
// o pacote nativo e indivisivel, a ultima unidade pode pagar o restante menor
// que dez, exatamente como o loop do case KING/KINGDOM do W2PP.
func consumeSapphires(ch *model.Char, amount uint32) bool {
	if sapphireCount(ch) < amount {
		return false
	}
	loose := countInventoryItem(ch, model.SapphireItem)
	if loose > amount {
		loose = amount
	}
	if loose != 0 && !consumeInventoryItem(ch, model.SapphireItem, loose) {
		return false
	}
	remaining := amount - loose
	if remaining == 0 {
		return true
	}
	packs := (remaining + 9) / 10
	for i := 0; i < model.PlayerCarrySlots && packs > 0; i++ {
		if ch.Inv[i].Index != model.SapphirePack {
			continue
		}
		ch.Inv[i] = model.Item{}
		packs--
	}
	return packs == 0
}

// handleKingdomNPC trata os reis e os dois brokers. O booleano indica que o
// NPC pertence a este subsistema, mesmo quando a operacao foi recusada.
func (w *World) handleKingdomNPC(s *net.Session, p *Player, m *Mob) bool {
	if p == nil || p.Char == nil || m == nil || m.Def == nil {
		return false
	}
	if kingdom := kingdomForNPC(m.Def); kingdom != model.KingdomNeutral {
		w.joinKingdom(s, p, kingdom)
		return true
	}
	if isKingdomBroker(m.Def) {
		w.leaveKingdom(s, p, time.Now())
		return true
	}
	return false
}

func (w *World) joinKingdom(s *net.Session, p *Player, kingdom byte) {
	if playerCurHP(p.Char) == 0 {
		s.Send(wire.MessagePanel("Dead characters cannot choose a kingdom."))
		return
	}
	current := characterKingdom(p.Char)
	if guild, _ := w.guildOf(p.Char); guild != nil && guild.Kingdom != model.KingdomNeutral &&
		guild.Kingdom != kingdom {
		s.Send(wire.MessagePanel("Your guild belongs to another kingdom."))
		return
	}
	change, err := kingdomCapeForJoin(p.Char, kingdom)
	if err != nil {
		switch err {
		case errKingdomAlready:
			s.Send(wire.MessagePanel("You already have the proper cape of " + model.KingdomName(kingdom) + "."))
		case errKingdomWrongRealm:
			s.Send(wire.MessagePanel("Leave your current kingdom before choosing another."))
		case errKingdomNeedLevel220:
			s.Send(wire.MessagePanel("You must be level 220 to choose a kingdom."))
		case errKingdomNeedLevel256:
			s.Send(wire.MessagePanel("You must be level 256 to receive this cape."))
		case errKingdomNeedCelestial:
			s.Send(wire.MessagePanel("The Master cape requires a Celestial character."))
		default:
			s.Send(wire.MessagePanel("Your current cape cannot receive the kingdom medal."))
		}
		return
	}
	cost := kingdomJoinCost(kingdom)
	useEmblem := kingdomUsesEmblem(p.Char)
	if !useEmblem && sapphireCount(p.Char) < cost {
		s.Send(wire.MessagePanel(fmt.Sprintf("You need %d Sapphires.", cost)))
		return
	}

	snapshot := cloneCharacterState(p.Char)
	if useEmblem {
		p.Char.Equip[13] = model.Item{}
	} else if !consumeSapphires(p.Char, cost) {
		*p.Char = snapshot
		s.Send(wire.MessagePanel("The Sapphires could not be consumed."))
		return
	}
	if change.Preserve {
		p.Char.Equip[model.CapeSlot].Index = change.Index
	} else {
		// Trocar/promover a capa e uma transformacao do mesmo item. Efeitos
		// podem ser reiniciados pela regra, mas a identidade nao pode mudar.
		p.Char.Equip[model.CapeSlot] = model.Item{
			Index: change.Index,
			UID:   p.Char.Equip[model.CapeSlot].UID,
		}
	}
	w.recalcPlayer(p.Char)
	if err := w.commitKingdomChange(p, snapshot, kingdom); err != nil {
		s.Send(wire.MessagePanel("Save failed. The kingdom was not changed."))
		log.Printf("[#%d] REINO join save: %v", s.ID, err)
		return
	}
	w.syncKingdomChange(p)
	if current == kingdom {
		s.Send(wire.MessagePanel("Your " + model.KingdomName(kingdom) + " cape was promoted."))
	} else {
		s.Send(wire.MessagePanel("You now belong to " + model.KingdomName(kingdom) + "."))
	}
	payment := fmt.Sprintf("%d safiras", cost)
	if useEmblem {
		payment = "emblema 4081"
	}
	log.Printf("[#%d] REINO %q capa=%d tier=%d reino=%s pagamento=%s", s.ID, p.Char.Name,
		change.Index, change.Tier, model.KingdomName(kingdom), payment)
}

func (w *World) leaveKingdom(s *net.Session, p *Player, now time.Time) {
	if playerCurHP(p.Char) == 0 {
		s.Send(wire.MessagePanel("Dead characters cannot leave a kingdom."))
		return
	}
	kingdom := characterKingdom(p.Char)
	if kingdom == model.KingdomNeutral {
		s.Send(wire.MessagePanel("You do not belong to a kingdom."))
		return
	}
	// O broker nativo nao permite retirar a capa no domingo.
	if now.Weekday() == time.Sunday {
		s.Send(wire.MessagePanel("You cannot leave the kingdom on Sunday."))
		return
	}
	neutral, ok := model.NeutralCape(p.Char.Equip[model.CapeSlot].Index)
	if !ok {
		s.Send(wire.MessagePanel("The broker cannot convert your kingdom cape."))
		return
	}
	if sapphireCount(p.Char) < kingdomLeaveCost {
		s.Send(wire.MessagePanel(fmt.Sprintf("You need %d Sapphires.", kingdomLeaveCost)))
		return
	}

	snapshot := cloneCharacterState(p.Char)
	if !consumeSapphires(p.Char, kingdomLeaveCost) {
		*p.Char = snapshot
		s.Send(wire.MessagePanel("The Sapphires could not be consumed."))
		return
	}
	p.Char.Equip[model.CapeSlot].Index = neutral
	w.recalcPlayer(p.Char)
	if err := w.commitKingdomChange(p, snapshot, model.KingdomNeutral); err != nil {
		s.Send(wire.MessagePanel("Save failed. You are still in the kingdom."))
		log.Printf("[#%d] REINO leave save: %v", s.ID, err)
		return
	}
	w.syncKingdomChange(p)
	s.Send(wire.MessagePanel("You left " + model.KingdomName(kingdom) + "."))
	log.Printf("[#%d] REINO %q deixou %s por %d Safiras", s.ID, p.Char.Name,
		model.KingdomName(kingdom), kingdomLeaveCost)
}

// setLeaderGuildKingdom replica a atualizacao de GuildInfo.Clan feita pelo
// case KING nativo. Membros comuns alteram somente sua capa; o lider carrega a
// guild para o novo reino na mesma transacao de persistencia.
func (w *World) setLeaderGuildKingdom(ch *model.Char, kingdom byte) bool {
	guild, member := w.guildOf(ch)
	if guild == nil || member == nil || member.Rank != model.GuildRankLeader {
		return false
	}
	guild.Kingdom = kingdom
	return true
}

// commitKingdomChange persiste a troca de capa/reino ja aplicada em p.Char. Se o
// jogador for lider, a guild acompanha o novo reino na MESMA transacao
// (saveGuildState); senao grava apenas a conta. Em falha, restaura char e guilds
// ao snapshot e devolve o erro para o chamador mensagear no seu contexto (join
// mantem o reino anterior; leave mantem o jogador no reino).
func (w *World) commitKingdomChange(p *Player, snapshot model.Char, kingdom byte) error {
	var guildSnapshot []model.Guild
	if w.guilds != nil {
		guildSnapshot = w.snapshotGuilds()
	}
	var err error
	if w.setLeaderGuildKingdom(p.Char, kingdom) {
		err = w.saveGuildState(p.Account)
	} else {
		err = w.saveAccount(p.Account)
	}
	if err != nil {
		*p.Char = snapshot
		if w.guilds != nil {
			w.restoreGuilds(guildSnapshot)
		}
	}
	return err
}

func (w *World) syncKingdomChange(p *Player) {
	p.Session.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
	p.Session.Send(wire.SendItem(p.ID, placeEquip, 13, p.Char.Equip[13]))
	p.Session.Send(wire.SendItem(p.ID, placeEquip, model.CapeSlot, p.Char.Equip[model.CapeSlot]))
	p.Session.Send(wire.UpdateScore(p.ID, *p.Char))
	p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	w.syncPlayerVitals(p)
	w.refreshAppearance(p)
}

func (w *World) kingdomCommandTeleport(s *net.Session, p *Player, king bool) {
	if playerCurHP(p.Char) == 0 {
		s.Send(wire.MessagePanel("Dead characters cannot use this teleport."))
		return
	}
	if time.Now().Before(p.NextKingdomTeleport) {
		s.Send(wire.MessagePanel("Wait before using the kingdom teleport again."))
		return
	}
	kingdom := characterKingdom(p.Char)
	var x, y uint16
	if king {
		switch kingdom {
		case model.KingdomHekalotia:
			x, y = 1748, 1574
		case model.KingdomAkelonia:
			x, y = 1748, 1880
		default:
			s.Send(wire.MessagePanel("Choose a kingdom before visiting the king."))
			return
		}
	} else {
		switch kingdom {
		case model.KingdomHekalotia:
			x, y = 1690, 1618
		case model.KingdomAkelonia:
			x, y = 1690, 1842
		default:
			x, y = 1702, 1726
		}
	}
	w.cancelTrade(p, "teleporte de reino")
	if !w.teleportPlayer(p, x, y) {
		s.Send(wire.MessagePanel("The kingdom teleport could not be completed."))
		return
	}
	p.NextKingdomTeleport = time.Now().Add(kingdomWarpCooldown)
	log.Printf("[#%d] REINO comando %q -> (%d,%d)", s.ID, p.Char.Name, p.X, p.Y)
}
