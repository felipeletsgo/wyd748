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
	// Decorative king variants share names with kingdom services. A configured
	// selector must identify the service before its name can grant progression.
	if def.Score != nil && def.Score.Merchant != 15 {
		return model.KingdomNeutral
	}
	switch strings.ToLower(generName(def.InteractionName())) {
	case "king harabard":
		return model.KingdomHekalotia
	case "king glantuar":
		return model.KingdomAkelonia
	default:
		return model.KingdomNeutral
	}
}

func isKingdomBroker(def *model.NPCDef) bool {
	return def != nil && strings.EqualFold(generName(def.InteractionName()), "Kingdom Brocker")
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
		s.Send(wire.MessagePanel(w.msg("_WG_Dead_Characters_Cannot_Choose_Kingdom")))
		return
	}
	current := characterKingdom(p.Char)
	if !w.guildKingdomChangeAllowed(p.Char, kingdom) {
		s.Send(wire.MessagePanel(w.msg("_WG_Only_Guild_Leader_Can_Change_Guilds")))
		return
	}
	change, err := kingdomCapeForJoin(p.Char, kingdom)
	if err != nil {
		switch err {
		case errKingdomAlready:
			w.panel(s, "_WG_Already_Have_Kingdom_Cape", w.kingdomName(kingdom))
		case errKingdomWrongRealm:
			s.Send(wire.MessagePanel(w.msg("_WG_Leave_Current_Kingdom_Before_Choosing_Another")))
		case errKingdomNeedLevel220:
			s.Send(wire.MessagePanel(w.msg("_WG_Must_Level_Choose_Kingdom")))
		case errKingdomNeedLevel256:
			s.Send(wire.MessagePanel(w.msg("_WG_Must_Level_Receive_Cape")))
		case errKingdomNeedCelestial:
			s.Send(wire.MessagePanel(w.msg("_WG_Master_Cape_Requires_Celestial_Character")))
		default:
			s.Send(wire.MessagePanel(w.msg("_WG_Current_Cape_Cannot_Receive_Kingdom_Medal")))
		}
		return
	}
	cost := kingdomJoinCost(kingdom)
	useEmblem := kingdomUsesEmblem(p.Char)
	if !useEmblem && sapphireCount(p.Char) < cost {
		s.Send(wire.MessagePanel(w.msg("_DN_Need_D_Sapphire", cost)))
		return
	}
	if w.criticalWrites != nil {
		if err := w.queueKingdomCape(p, kingdom, change, cost, useEmblem, func() {
			if current == kingdom {
				w.panel(s, "_WG_Kingdom_Cape_Promoted", w.kingdomName(kingdom))
			} else {
				w.panel(s, "_WG_Now_Belong_To_Kingdom", w.kingdomName(kingdom))
			}
		}); err != nil {
			s.Send(wire.MessagePanel(w.msg("_WG_Save_Failed_Kingdom_Was_Not_Changed")))
		}
		return
	}

	snapshot := cloneCharacterState(p.Char)
	if useEmblem {
		p.Char.Equip[13] = model.Item{}
	} else if !consumeSapphires(p.Char, cost) {
		*p.Char = snapshot
		s.Send(wire.MessagePanel(w.msg("_WG_Sapphires_Could_Not_Consumed")))
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
	w.ensureCapeEmblem(p.Char)
	w.recalcPlayer(p.Char)
	if err := w.commitKingdomChange(p, snapshot, kingdom); err != nil {
		s.Send(wire.MessagePanel(w.msg("_WG_Save_Failed_Kingdom_Was_Not_Changed")))
		log.Printf("[#%d] KINGDOM join save: %v", s.ID, err)
		return
	}
	w.syncKingdomChange(p)
	if current == kingdom {
		w.panel(s, "_WG_Kingdom_Cape_Promoted", w.kingdomName(kingdom))
	} else {
		w.panel(s, "_WG_Now_Belong_To_Kingdom", w.kingdomName(kingdom))
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
		s.Send(wire.MessagePanel(w.msg("_WG_Dead_Characters_Cannot_Leave_Kingdom")))
		return
	}
	kingdom := characterKingdom(p.Char)
	if !w.guildKingdomChangeAllowed(p.Char, model.KingdomNeutral) {
		s.Send(wire.MessagePanel(w.msg("_WG_Only_Guild_Leader_Can_Change_Guilds")))
		return
	}
	if kingdom == model.KingdomNeutral {
		s.Send(wire.MessagePanel(w.msg("_WG_Do_Not_Belong_Kingdom")))
		return
	}
	// The native broker does not remove kingdom membership on Sunday.
	if now.Weekday() == time.Sunday {
		s.Send(wire.MessagePanel(w.msg("_WG_Cannot_Leave_Kingdom_Sunday")))
		return
	}
	neutral, ok := model.NeutralCape(p.Char.Equip[model.CapeSlot].Index)
	if !ok {
		s.Send(wire.MessagePanel(w.msg("_WG_Broker_Cannot_Convert_Kingdom_Cape")))
		return
	}
	if sapphireCount(p.Char) < kingdomLeaveCost {
		s.Send(wire.MessagePanel(w.msg("_DN_Need_D_Sapphire", kingdomLeaveCost)))
		return
	}
	if w.criticalWrites != nil {
		if err := w.queueKingdomCape(p, model.KingdomNeutral, kingdomCapeChange{Index: neutral, Preserve: true},
			kingdomLeaveCost, false, func() { w.panel(s, "_NN_Leaving_kingdom", w.kingdomName(kingdom)) }); err != nil {
			s.Send(wire.MessagePanel(w.msg("_WG_Save_Failed_Still_Kingdom")))
		}
		return
	}

	snapshot := cloneCharacterState(p.Char)
	if !consumeSapphires(p.Char, kingdomLeaveCost) {
		*p.Char = snapshot
		s.Send(wire.MessagePanel(w.msg("_WG_Sapphires_Could_Not_Consumed")))
		return
	}
	p.Char.Equip[model.CapeSlot].Index = neutral
	w.recalcPlayer(p.Char)
	if err := w.commitKingdomChange(p, snapshot, model.KingdomNeutral); err != nil {
		s.Send(wire.MessagePanel(w.msg("_WG_Save_Failed_Still_Kingdom")))
		log.Printf("[#%d] KINGDOM leave save: %v", s.ID, err)
		return
	}
	w.syncKingdomChange(p)
	w.panel(s, "_NN_Leaving_kingdom", w.kingdomName(kingdom))
	log.Printf("[#%d] KINGDOM %q left %s for %d Sapphires", s.ID, p.Char.Name,
		model.KingdomName(kingdom), kingdomLeaveCost)
}

// setLeaderGuildKingdom updates GuildInfo.Clan for a leader in the same
// transaction. Ordinary members cannot change kingdom independently.
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
		guild, _ := w.guildOf(p.Char)
		_, changes := w.guildKingdomAccounts(p, accountStateSnapshot(p.Account), guild.ID, kingdom)
		accounts := make([]*model.Account, 0, len(changes))
		for _, account := range changes {
			accounts = append(accounts, account)
		}
		err = w.saveGuildState(accounts...)
		if err == nil {
			installKingdomAccounts(changes)
			// The caller publishes the initiating character after success.
			delete(changes, p)
			w.publishGuildKingdomAccounts(changes)
		}
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
	p.Session.Send(wire.SendItem(p.ID, placeEquip, 0, wire.CharacterFaceItem(*p.Char)))
	p.Session.Send(wire.SendItem(p.ID, placeEquip, 13, p.Char.Equip[13]))
	p.Session.Send(wire.SendItem(p.ID, placeEquip, model.CapeSlot, p.Char.Equip[model.CapeSlot]))
	p.Session.Send(playerScorePacket(p))
	p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	w.syncPlayerVitals(p)
	w.syncPlayerChaos(p)
	w.restoreTransformationView(p)
}

func (w *World) kingdomCommandTeleport(s *net.Session, p *Player, king bool) {
	if w.criticalTradeReserved(p) {
		return
	}
	if playerCurHP(p.Char) == 0 {
		s.Send(wire.MessagePanel(w.msg("_WG_Dead_Characters_Cannot_Use_Teleport")))
		return
	}
	now := w.now()
	if now.Before(p.NextKingdomTeleport) {
		s.Send(wire.MessagePanel(w.msg("_WG_Wait_Before_Using_Kingdom_Teleport_Again")))
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
			s.Send(wire.MessagePanel(w.msg("_WG_Choose_Kingdom_Before_Visiting_King")))
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
		s.Send(wire.MessagePanel(w.msg("_WG_Kingdom_Teleport_Could_Not_Completed")))
		return
	}
}

// warriorSealItem is Warrior's Seal (volatile 199).
const warriorSealItem = 4146

// warriorSealCape follows W2PP _MSG_UseItem for Warrior's Seal: a Mortal at
// level 355 (internal 354) or above without an Elite or Hero cape receives the
// Elite cape of its kingdom, or the white Elite of Adventurer without one.
// The cape keeps its server identity but not refinement or effects, as the
// reference implementation clears the slot first.
func warriorSealCape(ch *model.Char) bool {
	if ch == nil || ch.Score == nil || !matchesEvolution(ch, "mortal") || ch.Score.Level < 354 {
		return false
	}
	current := ch.Equip[model.CapeSlot]
	if tier, ok := model.KingdomCapeTierOf(current.Index); ok &&
		(tier == model.CapeTierElite || tier == model.CapeTierHero) {
		return false
	}
	cape, ok := model.KingdomCapeAtTier(model.CapeTierElite, model.KingdomFromCape(current.Index))
	if !ok {
		return false
	}
	if current.UID != "" {
		ch.Equip[model.CapeSlot] = model.Item{Index: cape, UID: current.UID}
		return true
	}
	item, err := materializeItem(model.Item{Index: cape})
	if err != nil {
		return false
	}
	ch.Equip[model.CapeSlot] = item
	return true
}
