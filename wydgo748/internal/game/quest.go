package game

import (
	"fmt"
	"log"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// cargoMerchantType is the low Merchant nibble of warehouse NPCs.
const cargoMerchantType = 2

// DESIGN NOTE: why there is no server-side "PendingQuest" here.
//
// The original plan required a pending state between offer and confirmation so
// a stray ClickOk=1 could not grant a reward. A 7.48 client capture showed that
// design is impossible: when the NPC has the confirmation trigger (type 11 +
// face 58, or types 10/13/14/15), the first click sends NO packet; the client
// only opens its local window. Only "Yes" sends, already with ClickOk=1. No
// offer packet exists to create a pending state.
//
// The server therefore receives exactly ONE packet per interaction, and the
// real defense is server-side validation in executeQuest: requirements, the
// completed flag, range and visibility. The client packet carries only the NPC
// ID; cost, items and rewards always come from quests.json.

// NPC kinds that already have their own handler. A quest configured on one of
// them would never fire because routing tests known kinds first, so startup
// rejects that configuration instead of ignoring it.
const (
	npcKindShop               = "shop/master"
	npcKindCargo              = "cargo"
	npcKindCraft              = "compositor"
	npcKindAbilityResetMaster = "ability reset master"
	npcKindAirMove            = "air transport"
)

// reservedNPCKind returns the NPC's dedicated handler, if any. It keeps the
// kind vocabulary in one place so routing and startup validation cannot
// diverge.
func reservedNPCKind(def *model.NPCDef) (string, bool) {
	if def == nil || def.Score == nil {
		return "", false
	}
	if _, isShop := shopTypeForMerchant(def.Score.Merchant); isShop {
		return npcKindShop, true
	}
	if def.Score.Merchant&0x0F == craftingMerchant {
		return npcKindCraft, true
	}
	if def.Score.Merchant&0xF == cargoMerchantType {
		return npcKindCargo, true
	}
	if isAirMoveNPC(def) {
		return npcKindAirMove, true
	}
	if isMountMasterNPC(def) {
		return npcKindMountMaster, true
	}
	if isAbilityResetMasterNPC(def) {
		return npcKindAbilityResetMaster, true
	}
	return "", false
}

// indexQuests matches each quest with its NPC and rejects configurations that
// could never work. It runs at startup: an error here stops the server.
func indexQuests(file model.QuestFile, npcs []model.NPCDef) (map[string]*model.QuestDef, error) {
	byName := make(map[string]*model.NPCDef, len(npcs)*2)
	for i := range npcs {
		byName[model.NormalizeQuestNPC(npcs[i].Name)] = &npcs[i]
		byName[model.NormalizeQuestNPC(generName(npcs[i].Name))] = &npcs[i]
	}
	out := make(map[string]*model.QuestDef, len(file.Quests))
	for i := range file.Quests {
		quest := &file.Quests[i]
		key := model.NormalizeQuestNPC(quest.NPC)
		def, exists := byName[key]
		if !exists {
			return nil, fmt.Errorf("quest %d references NPC %q, which does not exist in data/npcs",
				quest.ID, quest.NPC)
		}
		if kind, reserved := reservedNPCKind(def); reserved {
			return nil, fmt.Errorf(
				"quest %d is configured on NPC %q, which is already a %s: the click would never reach the quest",
				quest.ID, quest.NPC, kind)
		}
		out[key] = quest
	}
	return out, nil
}

// questForNPC returns the quest configured for the NPC, if any. The allowlist
// is quests.json itself: an NPC absent from it never becomes a quest.
func (w *World) questForNPC(def *model.NPCDef) *model.QuestDef {
	if def == nil || len(w.questsByNPC) == 0 {
		return nil
	}
	if quest, ok := w.questsByNPC[model.NormalizeQuestNPC(def.Name)]; ok {
		return quest
	}
	return w.questsByNPC[model.NormalizeQuestNPC(generName(def.Name))]
}

// onQuestInteraction handles a click on a quest NPC.
//
// CONFIRMATION IS CLIENT-SIDE, not server-side. The 7.48 client alone decides
// whether to open the "are you sure?" window from the NPC type and face mesh
// (TMFieldScene: type 11 + face 58, plus types 10/13/14/15). Captured result:
//
//   - NPC without that trigger (e.g. Kibita, face 16): the click sends ClickOk=0.
//   - NPC with the trigger (e.g. Gravedigger, face 58): the first click sends
//     no packet and only opens the window. "Yes" sends ClickOk=1.
//
// Either way the server receives ONE packet per interaction, so it cannot
// require an "offer" packet before granting: natively confirmed quests would
// never send it and could never complete. The real defense is server-side
// validation (requirements, range, visibility and completed flag), which runs
// entirely in executeQuest.
func (w *World) onQuestInteraction(s *net.Session, p *Player, m *Mob, quest *model.QuestDef, clickOk int32) {
	if clickOk == 1 {
		log.Printf("[#%d] QUEST %d confirmed in the native client window", s.ID, quest.ID)
	}
	w.executeQuest(s, p, m, quest)
}

// questBlockedMessage returns the rejection reason, preferring the quest
// author's text when one is defined.
func questBlockedMessage(quest *model.QuestDef, reason string) string {
	if quest.Messages.Denied != "" {
		return quest.Messages.Denied
	}
	return reason
}

// questRequirementsMet validates level, gold, items and the prior quest. It
// returns the rejection reason for the message panel.
func (w *World) questRequirementsMet(p *Player, quest *model.QuestDef) (string, bool) {
	ch := p.Char
	level := uint32(0)
	if ch.Score != nil {
		level = ch.Score.Level
	}
	req := quest.Requires
	if req.MortalOnly && strings.TrimSpace(ch.Evolution) != "" {
		return "Available only to Mortal characters.", false
	}
	if req.MinLevel != 0 && level < req.MinLevel {
		return fmt.Sprintf("You must be level %d.", req.MinLevel), false
	}
	if req.MaxLevel != 0 && level > req.MaxLevel {
		return fmt.Sprintf("Available only up to level %d.", req.MaxLevel), false
	}
	if req.AfterQuest != 0 && !questCompleted(ch, req.AfterQuest) {
		return "You cannot take this quest yet.", false
	}
	if req.Gold != 0 && ch.Gold < req.Gold {
		return "Not enough gold.", false
	}
	// Native Kibita requires Citizen == 0: citizenship cannot be bought twice,
	// nor swapped from another channel to this one.
	if quest.Rewards.Citizenship && ch.Citizenship != 0 {
		return "You already have citizenship.", false
	}
	for _, item := range req.Items {
		if countInventoryItem(ch, item.Index) < item.Quantity() {
			return "You do not have the required items.", false
		}
	}
	// Consumption must be available too: requiring it at the offer avoids
	// promising a reward the confirmation could not pay for.
	for _, item := range quest.Consumes {
		if countInventoryItem(ch, item.Index) < item.Quantity() {
			return "You do not have the required items.", false
		}
	}
	if name, ok := hasCounters(p, quest.Requires.Counters); !ok {
		return counterDeniedMessage(name), false
	}
	if name, ok := hasCounters(p, quest.ConsumeCounters); !ok {
		// A zero balance still passes when the NPC recharge can cover it; the
		// Survivor exchanges the seal for the configured entry allowance.
		if !rechargeCovers(p, ch, quest, name) {
			return counterDeniedMessage(name), false
		}
	}
	if free := freeInventorySlots(ch); free < len(quest.Rewards.Items) {
		return "Your inventory is full.", false
	}
	return "", true
}

// countInventoryItem sums an item's units in visible slots, honoring
// EF_AMOUNT stacks.
func countInventoryItem(ch *model.Char, index uint16) uint32 {
	if ch == nil || index == 0 {
		return 0
	}
	var total uint32
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if ch.Inv[i].Index != index {
			continue
		}
		total += itemStackAmount(ch.Inv[i])
	}
	return total
}

// itemStackAmount returns the stack size (EF_AMOUNT) or 1.
func itemStackAmount(item model.Item) uint32 {
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] == effectAmount {
			if amount := uint32(item.Eff[i*2+1]); amount > 0 {
				return amount
			}
			return 1
		}
	}
	return 1
}

func freeInventorySlots(ch *model.Char) int {
	free := 0
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if ch.Inv[i].Index == 0 {
			free++
		}
	}
	return free
}

// executeQuest runs the quest end to end. It revalidates everything (state may
// have changed since the player opened the window), consumes, grants and
// persists; a failed save reverts everything.

// speakQuestNPC makes the NPC say one configured line aloud. It is sent as
// 0x333 with the NPC ID, so the client shows it as the NPC's speech and nearby
// players read it too, matching native SendSay, which also uses GridMulticast.
//
// The choice uses w.intn, the injectable RNG, so tests can fix the line.
func (w *World) speakQuestNPC(p *Player, m *Mob, quest *model.QuestDef) {
	if m == nil || len(quest.Dialogue) == 0 {
		return
	}
	line := quest.Dialogue[w.intn(len(quest.Dialogue))]
	for _, listener := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		listener.Session.Send(wire.MessageChat(m.ID, line))
	}
	_ = p
}

func (w *World) executeQuest(s *net.Session, p *Player, m *Mob, quest *model.QuestDef) {
	// The NPC speaks BEFORE any check: it reacts to the player's presence, not
	// to the outcome. A rejection deserves a line too.
	w.speakQuestNPC(p, m, quest)

	// Every rejection is logged with its reason. Otherwise the log shows the
	// confirmation arriving and then silence, and "already completed" cannot be
	// told apart from a real failure.
	if !quest.Repeatable && questCompleted(p.Char, quest.ID) {
		s.Send(wire.MessagePanel("You have already completed this quest."))
		log.Printf("[#%d] QUEST %d rejected: already completed", s.ID, quest.ID)
		return
	}
	// The whole defense is here: the client packet never carries cost, items or
	// rewards, only the NPC ID. Range and visibility were already validated in
	// resolveNPCInteraction before this point.
	if reason, ok := w.questRequirementsMet(p, quest); !ok {
		s.Send(wire.MessagePanel(questBlockedMessage(quest, reason)))
		log.Printf("[#%d] QUEST %d rejected: %s", s.ID, quest.ID, reason)
		return
	}

	// Rollback snapshot: persistence precedes the client confirmation.
	images := criticalPreimages(p)
	previousChar := cloneCharacterState(p.Char)
	previousX, previousY := p.X, p.Y
	previousCounters := copyCounters(p)
	rollback := func() {
		*p.Char = previousChar
		p.X, p.Y = previousX, previousY
		p.SpecialCoins = previousCounters
	}
	var teleportX, teleportY uint16
	if target := quest.Rewards.Teleport; target != nil {
		var ok bool
		teleportX, teleportY, ok = w.resolvePlayerTeleportDestination(p, target.X, target.Y)
		if !ok {
			s.Send(wire.MessagePanel("The quest destination is unavailable."))
			log.Printf("[#%d] QUEST %d rejected: teleport destination unavailable", s.ID, quest.ID)
			return
		}
	}

	for _, item := range quest.Consumes {
		if !consumeInventoryItem(p.Char, item.Index, item.Quantity()) {
			rollback()
			s.Send(wire.MessagePanel("You do not have the required items."))
			log.Printf("[#%d] QUEST %d rejected: consuming item %d x%d failed",
				s.ID, quest.ID, item.Index, item.Quantity())
			return
		}
	}
	// The recharge runs BEFORE spending: without a balance the NPC
	// exchanges the item for entries and only then charges the visit.
	applyQuestRecharge(p, quest)
	if !spendCounters(p, quest.ConsumeCounters) {
		rollback()
		s.Send(wire.MessagePanel("You do not have enough entries."))
		log.Printf("[#%d] QUEST %d rejected: insufficient counter balance", s.ID, quest.ID)
		return
	}
	grantCounters(p, quest.Rewards.Counters)
	if quest.Requires.Gold != 0 {
		p.Char.Gold -= quest.Requires.Gold
	}
	for _, item := range quest.Rewards.Items {
		if !grantInventoryItem(p.Char, item) {
			rollback()
			s.Send(wire.MessagePanel("Your inventory is full."))
			log.Printf("[#%d] QUEST %d rejected: no room for item %d",
				s.ID, quest.ID, item.Index)
			return
		}
	}
	if r := quest.Rewards.Refine; r != nil {
		if !refineQuestReward(p.Char, r) {
			rollback()
			s.Send(wire.MessagePanel("You have nothing equipped to improve."))
			log.Printf("[#%d] QUEST %d rejected: nothing to refine in slot %d",
				s.ID, quest.ID, r.Slot)
			return
		}
	}
	if quest.Rewards.Gold != 0 {
		// Same cap as gold bars: a reward cannot exceed the limit.
		if p.Char.Gold > maxCharacterGold-quest.Rewards.Gold {
			p.Char.Gold = maxCharacterGold
		} else {
			p.Char.Gold += quest.Rewards.Gold
		}
	}
	cytheraChanged := false
	leveledUp := false
	if quest.Rewards.Exp != 0 {
		// The experience chest (affect 39) also doubles quest EXP.
		levels, _ := grantExp(p.Char, expWithDoubleBuffAt(p.Char, quest.Rewards.Exp, w.now()))
		leveledUp = levels > 0
		cytheraChanged = levels > 0 && updateCelestialCythera(p.Char)
	}
	if quest.Rewards.Citizenship {
		// Citizenship stores the CHANNEL NUMBER, as natively.
		p.Char.Citizenship = w.channel
		// This transaction changes only the buyer, not the guild's members.
	}
	if !quest.Repeatable {
		markQuestCompleted(p.Char, quest.ID)
	}
	teleporting := teleportX != 0 && teleportY != 0
	if teleporting {
		p.Char.X, p.Char.Y = teleportX, teleportY
		// Stage the hometown from the destination without moving the live
		// runtime position: under the coordinator the move waits for commit.
		stagePlayerHomeCity(&Player{Char: p.Char, X: teleportX, Y: teleportY})
	}
	publish := func() {
		w.recalcPlayer(p.Char)
		if teleporting {
			// The position participated in the quest commit. Only the
			// region change remains to publish; no second save can fail.
			x, y := teleportX, teleportY
			if rx, ry, ok := w.resolvePlayerTeleportDestination(p, x, y); ok {
				x, y = rx, ry
			}
			p.X, p.Y = x, y
			p.Char.X, p.Char.Y = x, y
			// The NPC entry is the only legitimate way into a quest zone.
			w.grantQuestZoneEntry(p, x, y)
		}
		if !s.IsClosed() {
			s.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
			s.Send(wire.UpdateEtc(p.ID, *p.Char))
			s.Send(playerScorePacket(p))
			if cytheraChanged {
				s.Send(wire.SendItem(p.ID, placeEquip, 1, p.Char.Equip[1]))
			}
			if quest.Rewards.Citizenship {
				s.Send(wire.SendItem(p.ID, placeEquip, 0, wire.CharacterFaceItem(*p.Char)))
			}
		}
		if quest.Rewards.Citizenship && !s.IsClosed() {
			w.restoreTransformationView(p)
		}
		if cytheraChanged {
			w.refreshAppearance(p)
		}
		if leveledUp || quest.Rewards.Citizenship {
			w.syncPlayerChaos(p)
		}
		if teleporting {
			w.publishPlayerTeleport(p)
		}
		if !s.IsClosed() {
			s.Send(wire.MessagePanel(quest.Messages.Success))
		}
		log.Printf("[#%d] QUEST %d completed by %q", s.ID, quest.ID, p.Char.Name)
	}
	if w.criticalWrites != nil {
		failed := func() {
			if !s.IsClosed() {
				s.Send(wire.MessagePanel("Save failed. The quest was not completed."))
			}
		}
		if err := w.queueCriticalLiveMutation("quest", images, publish,
			criticalLiveMutationOptions{sidecar: true, reject: failed}); err != nil {
			log.Printf("[#%d] quest %d admission rejected: %v", s.ID, quest.ID, err)
			failed()
		}
		return
	}

	// PostgreSQL commits account and charstate in one transaction, including
	// items, gold, EXP, the completed quest and special counters.
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		rollback()
		s.Send(wire.MessagePanel("Save failed. The quest was not completed."))
		log.Printf("[#%d] ERROR quest %d: %v", s.ID, quest.ID, err)
		return
	}
	publish()
}

// consumeInventoryItem removes `amount` units, honoring stacks.
func consumeInventoryItem(ch *model.Char, index uint16, amount uint32) bool {
	if countInventoryItem(ch, index) < amount {
		return false
	}
	remaining := amount
	for i := 0; i < model.PlayerCarrySlots && remaining > 0; i++ {
		if ch.Inv[i].Index != index {
			continue
		}
		stack := itemStackAmount(ch.Inv[i])
		if stack <= remaining {
			remaining -= stack
			ch.Inv[i] = model.Item{}
			continue
		}
		setItemStackAmount(&ch.Inv[i], stack-remaining)
		remaining = 0
	}
	return remaining == 0
}

func setItemStackAmount(item *model.Item, amount uint32) {
	if amount > 255 {
		amount = 255
	}
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] == effectAmount {
			item.Eff[i*2+1] = byte(amount)
			return
		}
	}
}

// grantInventoryItem places the item in the first free visible slot.
// refineQuestReward refines EQUIPPED gear, like the native Training Camp
// guards (_MSG_Quest.cpp: Trainer 2 writes EF_SANC 7 on the weapon, Trainer 3
// writes 6 on every piece).
//
// A negative slot refines everything. It returns false when nothing was
// refined: rewarding someone without equipment would spend the one-time quest
// without delivering anything.
func refineQuestReward(ch *model.Char, r *model.QuestRefine) bool {
	if ch == nil || r == nil {
		return false
	}
	refinar := func(slot int) bool {
		if ch.Equip[slot].Index == 0 {
			return false
		}
		return setItemSanc(&ch.Equip[slot], r.Sanc)
	}
	if r.Slot >= 0 {
		return refinar(r.Slot)
	}
	algum := false
	for slot := 0; slot < model.MaxEquipSlots; slot++ {
		if refinar(slot) {
			algum = true
		}
	}
	return algum
}

func grantInventoryItem(ch *model.Char, reward model.QuestItem) bool {
	item := model.Item{Index: reward.Index}
	if amount := reward.Quantity(); amount > 1 {
		item.Eff[0] = effectAmount
		if amount > 255 {
			amount = 255
		}
		item.Eff[1] = byte(amount)
	}
	var err error
	item, err = materializeItem(item)
	if err != nil {
		return false
	}
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if ch.Inv[i].Index == 0 {
			ch.Inv[i] = item
			return true
		}
	}
	return false
}

// questCompleted checks the character's completed quest list.
// counterDeniedMessage explains a missing counter to the player. Internal
// names (kefra_ticket) never reach the screen.
func counterDeniedMessage(name string) string {
	if texto, ok := counterLabels[name]; ok {
		return texto
	}
	return "You do not meet the requirements yet."
}

// counterLabels holds the text of each known counter. A new counter without
// an entry falls back to the generic message instead of leaking its name.
var counterLabels = map[string]string{
	kefraTicketCounter: "You have no entries left.",
}

func questCompleted(ch *model.Char, questID int) bool {
	if ch == nil {
		return false
	}
	for _, done := range ch.QuestsDone {
		if done == int32(questID) {
			return true
		}
	}
	return false
}

func markQuestCompleted(ch *model.Char, questID int) {
	if questCompleted(ch, questID) {
		return
	}
	ch.QuestsDone = append(ch.QuestsDone, int32(questID))
}
