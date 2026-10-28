package game

import (
	"log"
	"strconv"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	crystalRequirementsMessage  = 445
	crystalWeaponMessage        = 446
	crystalSuccessMessage       = 447
	mapFragmentsMessage         = 448
	nativeServicePendingMessage = 449
	zakumEmptyMessage           = 450
	zakumPopulationMessage      = 451
)

// Limit recovered services to verified identities. Merchant
// values on dormant/custom templates do not prove that they own these services.
func officialNPCServiceKind(def *model.NPCDef) (string, bool) {
	if def == nil || def.Score == nil {
		return "", false
	}
	if _, ok := perzenExchangeFor(def); ok {
		return "Perzen sphere exchange", true
	}
	if def.Score.Merchant == 4 {
		switch def.InteractionName() {
		case "Gold_Dragon", "July", "Trainer4", "Trainer5":
			return "crystal and treasure service", true
		}
	}
	if def.InteractionName() == "Leaky_Zakum" && def.Score.Merchant == 30 {
		return "Zakum population service", true
	}
	if def.Score.Merchant == 62 {
		switch def.InteractionName() {
		case "Dragao_Vermelho", "SilverDragon":
			return "Grewpain creation service", true
		}
	}
	return "", false
}

// MODERNIZACAO_COMPATIVEL: recovered 7.59 server rules, existing 7.48
// interaction/item/score/message contracts; not a claim of 7.48 native parity.
func (w *World) handleOfficialNPCService(s *net.Session, p *Player, m *Mob) bool {
	if m == nil {
		return false
	}
	if _, ok := officialNPCServiceKind(m.Def); !ok {
		return false
	}
	if s == nil || s.IsClosed() || p == nil || p.Session != s || !p.InWorld ||
		p.Account == nil || p.Char == nil || p.Char.Score == nil ||
		p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) ||
		p.Char != &p.Account.Chars[p.CharSlot] || p.PersistencePoisoned ||
		w.shuttingDown || w.criticalTradeReserved(p) {
		return true
	}
	if m.Def.Score.Merchant == 30 {
		w.reportZakumPopulation(p, m)
	} else if playerCurHP(p.Char) != 0 {
		if exchange, ok := perzenExchangeFor(m.Def); ok {
			w.usePerzen(p, exchange)
		} else if m.Def.Score.Merchant == 62 {
			w.useSilverDragon(p)
		} else {
			w.useOfficialCrystalService(p)
		}
	}
	return true
}

func (w *World) useSilverDragon(p *Player) {
	const price = 2_000_000
	if p.Char.Gold < price {
		circleServiceMessage(p, 155)
		return
	}
	// Recovered NPC dispatcher, case 62: eighteen occupied slots, not stack
	// units, in the first sixty carry slots. Additional materials are retained.
	remaining := [5]int{10, 2, 4, 1, 1} // Snowball, Twig, Straw, Hat, Carrot.
	inventory := p.Char.Inv
	for slot, item := range inventory[:circleCarrySlots] {
		if item.Index < 1721 || item.Index > 1725 {
			continue
		}
		typeIndex := item.Index - 1721
		if remaining[typeIndex] > 0 {
			remaining[typeIndex]--
			inventory[slot] = model.Item{}
		}
	}
	for _, missing := range remaining {
		if missing != 0 {
			circleServiceMessage(p, 274)
			return
		}
	}
	if _, known := w.items[1726]; !known {
		circleServiceMessage(p, 190)
		return
	}
	// The native result explicitly overrides the catalog's default +3 with
	// rand()%4. A consumed slot is available even when the inventory was full.
	reward, err := materializeItem(model.Item{Index: 1726, Eff: [6]byte{43, byte(w.intn(4))}})
	if err != nil {
		log.Printf("[#%d] Grewpain creation identity: %v", p.ID, err)
		circleServiceMessage(p, 190)
		return
	}
	for slot, item := range inventory[:circleCarrySlots] {
		if item.Index == 0 {
			inventory[slot] = reward
			break
		}
	}
	images := criticalPreimages(p)
	p.Char.Inv = inventory
	p.Char.Gold -= price
	// Publish the committed inventory and balance. Native dialogue 292 belongs
	// to a different text table; it is not a valid 7.48 success-message index.
	w.commitOfficialNPCService(p, images, "Silver Dragon Grewpain creation", false, 0)
}

func (w *World) reportZakumPopulation(p *Player, m *Mob) {
	count := 0
	var last *Player
	for _, candidate := range w.players {
		if candidate == nil || !candidate.InWorld || candidate.Char == nil ||
			candidate.Session == nil || candidate.Session.IsClosed() ||
			w.gameplaySpaceForPlayer(candidate) != m.InstanceID ||
			candidate.X < 2180 || candidate.X > 2296 || candidate.Y < 1160 || candidate.Y > 1270 {
			continue
		}
		count++
		// The native helper traverses player slots in ascending order and
		// overwrites the output name, retaining the highest matching ID.
		if last == nil || candidate.ID > last.ID {
			last = candidate
		}
	}
	if count == 0 {
		circleServiceMessage(p, zakumEmptyMessage)
		return
	}
	p.Session.Send(wire.MessageParameterized(zakumPopulationMessage-1000, last.Char.Name, strconv.Itoa(count-1)))
}

// A completed map invokes an external database event in the recovered server.
// Until that contract is implemented, reject it before touching any fragment.
func completeNativeTreasureMap(inv *[64]model.Item) bool {
	for origin := 0; origin < circleCarrySlots; origin++ {
		if inv[origin].Index != 788 || origin+19 >= circleCarrySlots {
			continue
		}
		complete := true
		for i, offset := range [...]int{1, 9, 10, 18, 19} {
			complete = complete && inv[origin+offset].Index == uint16(789+i)
		}
		if complete {
			return true
		}
	}
	return false
}

func (w *World) useOfficialCrystalService(p *Player) {
	if p.Char.Equip[circleSlot].Index == 4060 || completeNativeTreasureMap(&p.Char.Inv) {
		// Jewel 4060 and its temporary-soul state are absent from the active
		// catalog/contract. Neither branch may consume items without its result.
		circleServiceMessage(p, nativeServicePendingMessage)
		return
	}
	fragments := false
	for _, item := range p.Char.Inv[:circleCarrySlots] {
		fragments = fragments || item.Index >= 788 && item.Index <= 793
	}
	if fragments {
		if _, ok := w.items[485]; !ok {
			circleServiceMessage(p, nativeServicePendingMessage)
			return
		}
		images := criticalPreimages(p)
		for slot, item := range p.Char.Inv[:circleCarrySlots] {
			if item.Index >= 788 && item.Index <= 793 {
				p.Char.Inv[slot].Index = 485 // Preserve effects and economic identity.
			}
		}
		w.commitOfficialNPCService(p, images, "Native treasure fragment conversion", false, mapFragmentsMessage)
		return
	}
	// All sixteen Treasure Settings source IDs in the official gameconfig.txt
	// are zero. There is no enabled table exchange to import before crystals.
	var crystals [7]bool
	for _, item := range p.Char.Inv[:circleCarrySlots] {
		if item.Index >= 421 && item.Index <= 427 {
			crystals[item.Index-421] = true
		}
	}
	for _, present := range crystals {
		if !present {
			circleServiceMessage(p, crystalRequirementsMessage)
			return
		}
	}
	weapon := p.Char.Equip[6]
	def, known := w.items[weapon.Index]
	mobType := itemAbility(weapon, def, "EF_MOBTYPE")
	if !known || weapon.Index == 0 || weapon.Index >= 3000 && weapon.Index < 3200 ||
		(def.Pos != 64 && def.Pos != 192) || mobType == 1 || mobType == 3 {
		circleServiceMessage(p, crystalWeaponMessage)
		return
	}
	skill, ok := w.skills[44]
	if !ok || skill.AffectType != 9 || skill.TickType != 0 || skill.AffectTime <= 0 {
		circleServiceMessage(p, nativeServicePendingMessage)
		return
	}
	images := criticalPreimages(p)
	p.Char.Equip[6] = w.rollOfficialCrystalWeapon(weapon, def)
	for slot, item := range p.Char.Inv[:circleCarrySlots] {
		if item.Index >= 421 && item.Index <= 427 {
			p.Char.Inv[slot] = model.Item{}
		}
	}
	removePlayerAffectTypes(p.Char, 23)
	// SetAffect(44, 200, 200): double the catalog counter, level 200. Use the
	// existing eight-second affect contract and stronger/longer recast guard.
	setAffectAt(p.Char, byte(skill.AffectType), skill.AffectValue, 200, skill.AffectTime*2, w.now())
	w.recalcPlayer(p.Char)
	w.commitOfficialNPCService(p, images, "Native crystal weapon conversion", true, crystalSuccessMessage)
}

// Specialize GenerateItem(item, ReqLevel+25*(slots/10), 1, 1104, 0).
// The explicit 104 override sets high quality before the a3 cap reduces it to
// tier 2. Consequently additional crystal slots do not raise these bonuses.
func (w *World) rollOfficialCrystalWeapon(item model.Item, def model.ItemDef) model.Item {
	type addition struct{ effect, step, adjustment int }
	first := [3]addition{{26, 3, 1}, {2, 9, -1}, {74, 3, 0}}
	second := [4]addition{{26, 3, 1}, {2, 9, -1}, {74, 3, 1}, {59, 0, 0}}
	if def.Unique == 44 || def.Unique == 47 {
		first = [3]addition{{60, 4, -1}, {74, 3, 1}, {59, 0, 0}}
		second = [4]addition{{60, 4, -1}, {74, 3, 1}, {59, 0, 0}, {59, 0, 0}}
	}
	a := first[w.intn(101)%3]
	b := second[(2*w.intn(100)/3)%4]
	w.intn(100) // First tier draw is raised to 4 by the high-quality flag.
	strength := 3
	if w.intn(100) < 16 {
		strength = 4
	}
	sanc := byte(0)
	if roll := w.intn(100) / 2; roll < 6 {
		sanc = 2
	} else if roll < 35 {
		sanc = 1
	}
	item.Eff = [6]byte{43, sanc, byte(a.effect), byte((4 + a.adjustment) * a.step),
		byte(b.effect), byte((strength + b.adjustment) * b.step)}
	for _, effect := range def.StaticEffects {
		switch effect.Name {
		case "EF_SANC":
			item.Eff[0], item.Eff[1] = 43, byte(effect.Value)
		case "EF_AMOUNT":
			item.Eff[0], item.Eff[1] = 61, byte(effect.Value)
		case "EF_INCUBATE":
			item.Eff[0], item.Eff[1] = 78, byte(min(9, effect.Value+w.intn(4)))
		case "EF_WDAY":
			item.Eff[0], item.Eff[1] = 106, byte(effect.Value)
		case "EF_HOUR":
			item.Eff[2], item.Eff[3] = 107, byte(effect.Value)
		case "EF_MIN":
			item.Eff[4], item.Eff[5] = 108, byte(effect.Value)
		}
	}
	return item
}

func (w *World) commitOfficialNPCService(p *Player, images []criticalPreimage, operation string, weapon bool, message int16) {
	rejected := func() {
		circleServiceMessage(p, 188)
		w.resendInventory(p)
	}
	publish := func() {
		if p.Session != nil && !p.Session.IsClosed() {
			w.resendInventory(p)
			if weapon {
				p.Session.Send(wire.SendItem(p.ID, placeEquip, 6, p.Char.Equip[6]))
				p.Session.Send(playerScorePacket(p))
			}
			if message > 0 {
				circleServiceMessage(p, message)
			}
		}
		if weapon {
			w.refreshAppearance(p)
			w.syncPlayerVitalsToObservers(p)
			w.updatePartyMember(p)
		}
	}
	if w.criticalWrites != nil {
		if err := w.queueCriticalLiveMutation(operation, images, publish,
			criticalLiveMutationOptions{reject: rejected}); err != nil {
			log.Printf("[#%d] %s admission: %v", p.ID, operation, err)
			rejected()
		}
		return
	}
	if err := w.saveAccount(p.Account); err != nil {
		w.restoreCriticalPreimages(images)
		log.Printf("[#%d] %s save: %v", p.ID, operation, err)
		rejected()
		return
	}
	publish()
}
