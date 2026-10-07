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
	kingdomJoinMinLevel = uint32(219) // displayed level 220
	kingdomKnightLevel  = uint32(255) // displayed level 256 in WYD 7.48
	kingdomLeaveCost    = uint32(16)
	kingdomWarpCooldown = 5 * time.Second
)

var (
	errKingdomAlready       = errors.New("character already has the proper cape")
	errKingdomWrongRealm    = errors.New("character belongs to the opposite kingdom")
	errKingdomNeedLevel220  = errors.New("character is below level 220")
	errKingdomNeedLevel256  = errors.New("character is below level 256")
	errKingdomInvalidCape   = errors.New("cape does not support kingdom progression")
	errKingdomNeedCelestial = errors.New("Master cape requires a Celestial character")
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

// kingdomCapeForJoin keeps kingdom authority in the cape. The first click
// creates/converts the level 220 cape; basic 545/546 upgrades to 543/544 at 256.
// Elite/Hero retain their neutral tier, while Celestial always receives Master.
func kingdomCapeForJoin(ch *model.Char, kingdom byte) (kingdomCapeChange, error) {
	if kingdom != model.KingdomHekalotia && kingdom != model.KingdomAkelonia {
		return kingdomCapeChange{}, errKingdomInvalidCape
	}
	if ch == nil || ch.Score == nil {
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
		// Celestial creation converts kingdom capes to Master; the king also
		// repairs older characters that still carry a lower tier.
		return kingdomCapeChange{Index: cape, Tier: model.CapeTierMaster, Preserve: currentIndex == 3199}, nil
	}
	if ch.Score.Level < kingdomJoinMinLevel {
		return kingdomCapeChange{}, errKingdomNeedLevel220
	}

	tier, knownTier := model.KingdomCapeTierOf(currentIndex)
	if currentKingdom == kingdom {
		if knownTier && tier == model.CapeTierBasic && ch.Score.Level >= kingdomKnightLevel {
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
	if ch.Score.Level < kingdomKnightLevel {
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
			total += 10 // 4131 contributes ten per slot, not per internal EF_AMOUNT
		}
	}
	return total
}

// consumeSapphires spends loose units before indivisible packs of ten. The last
// pack may cover a remainder below ten, matching the native KING/KINGDOM loop.
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

// handleKingdomNPC handles kings and brokers. True means this subsystem owns
// the NPC interaction, including rejected operations.
func (w *World) handleKingdomNPC(s *net.Session, p *Player, m *Mob) bool {
	if p == nil || p.Char == nil || m == nil || m.Def == nil {
		return false
	}
	if kingdom := kingdomForNPC(m.Def); kingdom != model.KingdomNeutral {
		if w.craftIdealStoneAtKing(s, p) {
			return true
		}
		w.joinKingdom(s, p, kingdom)
		return true
	}
	if isKingdomBroker(m.Def) {
		w.leaveKingdom(s, p, w.now())
		return true
	}
	return false
}

func (w *World) joinKingdom(s *net.Session, p *Player, kingdom byte) {
	if w.criticalGuildMutationReserved(p) {
		return
	}
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
	if w.criticalWrites != nil {
		if err := w.queueKingdomCape(p, kingdom, change, cost, useEmblem, func() {
			if current == kingdom {
				s.Send(wire.MessagePanel("Your " + model.KingdomName(kingdom) + " cape was promoted."))
			} else {
				s.Send(wire.MessagePanel("You now belong to " + model.KingdomName(kingdom) + "."))
			}
		}); err != nil {
			s.Send(wire.MessagePanel("Save failed. The kingdom was not changed."))
		}
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
		// Cape conversion preserves identity even when rules reset effects.
		p.Char.Equip[model.CapeSlot] = model.Item{
			Index: change.Index,
			UID:   p.Char.Equip[model.CapeSlot].UID,
		}
	}
	w.recalcPlayer(p.Char)
	if err := w.commitKingdomChange(p, snapshot, kingdom); err != nil {
		s.Send(wire.MessagePanel("Save failed. The kingdom was not changed."))
		log.Printf("[#%d] KINGDOM join save: %v", s.ID, err)
		return
	}
	w.syncKingdomChange(p)
	if current == kingdom {
		s.Send(wire.MessagePanel("Your " + model.KingdomName(kingdom) + " cape was promoted."))
	} else {
		s.Send(wire.MessagePanel("You now belong to " + model.KingdomName(kingdom) + "."))
	}
	payment := fmt.Sprintf("%d Sapphires", cost)
	if useEmblem {
		payment = "emblem 4081"
	}
	log.Printf("[#%d] KINGDOM %q cape=%d tier=%d kingdom=%s payment=%s", s.ID, p.Char.Name,
		change.Index, change.Tier, model.KingdomName(kingdom), payment)
}

func (w *World) leaveKingdom(s *net.Session, p *Player, now time.Time) {
	if w.criticalGuildMutationReserved(p) {
		return
	}
	if playerCurHP(p.Char) == 0 {
		s.Send(wire.MessagePanel("Dead characters cannot leave a kingdom."))
		return
	}
	kingdom := characterKingdom(p.Char)
	if kingdom == model.KingdomNeutral {
		s.Send(wire.MessagePanel("You do not belong to a kingdom."))
		return
	}
	// The native broker does not remove kingdom membership on Sunday.
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
	if w.criticalWrites != nil {
		if err := w.queueKingdomCape(p, model.KingdomNeutral, kingdomCapeChange{Index: neutral, Preserve: true},
			kingdomLeaveCost, false, func() { s.Send(wire.MessagePanel("You left " + model.KingdomName(kingdom) + ".")) }); err != nil {
			s.Send(wire.MessagePanel("Save failed. You are still in the kingdom."))
		}
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
		log.Printf("[#%d] KINGDOM leave save: %v", s.ID, err)
		return
	}
	w.syncKingdomChange(p)
	s.Send(wire.MessagePanel("You left " + model.KingdomName(kingdom) + "."))
	log.Printf("[#%d] KINGDOM %q left %s for %d Sapphires", s.ID, p.Char.Name,
		model.KingdomName(kingdom), kingdomLeaveCost)
}

// setLeaderGuildKingdom updates GuildInfo.Clan for a leader in the same
// transaction. Ordinary members change only their cape.
func (w *World) setLeaderGuildKingdom(ch *model.Char, kingdom byte) bool {
	guild, member := w.guildOf(ch)
	if guild == nil || member == nil || member.Rank != model.GuildRankLeader {
		return false
	}
	guild.Kingdom = kingdom
	return true
}

// commitKingdomChange is the legacy synchronous cape transaction. Leaders save
// the guild atomically with the account; rejection restores both snapshots.
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
	p.Session.Send(playerScorePacket(p))
	p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	w.syncPlayerVitals(p)
	w.refreshAppearance(p)
}

func (w *World) kingdomCommandTeleport(s *net.Session, p *Player, king bool) {
	if w.criticalTradeReserved(p) {
		return
	}
	if playerCurHP(p.Char) == 0 {
		s.Send(wire.MessagePanel("Dead characters cannot use this teleport."))
		return
	}
	now := w.now()
	if now.Before(p.NextKingdomTeleport) {
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
	if w.criticalWrites == nil {
		w.cancelTrade(p, "kingdom teleport")
	}
	if !w.requestPlayerTeleport(p, x, y, func() {
		p.NextKingdomTeleport = w.now().Add(kingdomWarpCooldown)
		log.Printf("[#%d] KINGDOM command %q -> (%d,%d)", s.ID, p.Char.Name, p.X, p.Y)
	}, nil) {
		s.Send(wire.MessagePanel("The kingdom teleport could not be completed."))
		return
	}
}
