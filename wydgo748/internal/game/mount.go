package game

import (
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// Mount state (HP, level, longevity and food) lives in item effect slots; see
// model/mount.go. This file owns bonuses, stage evolution and consumable
// matching. Packet dispatch lives in consumables.go. Historical/native evidence
// for individual behaviors is tracked separately from these implementation notes.

// mountSlot is the native mount slot (Equip[14]). Equip[15] holds the cape,
// including medals that select Akelonia/Hekalotia.
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

// mountStageThreshold returns the hatchling's evolution level (sIndex += 30).
// Types 2330-2335 evolve at 25/35/45/55/65/75; other types evolve at 100.
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

// mountBonus returns the equipped adult mount's stat bonuses and speed floor.
// BASE_GetItemAbility(EF_MOUNTLIFE) uses (level+20)*attack/100 and
// (level+15)*magic/100 as EF_MAGIC amplification, flat evasion/resistance and
// a speed floor. HP must be > 0; magic is not a flat damage addition.
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
	evasion      int
	resist       int
	runSpeed     int
}

func (w *World) premiumMountBonus(ch *model.Char, now time.Time) premiumMountStats {
	if w == nil || ch == nil {
		return premiumMountStats{}
	}
	item := ch.Equip[mountSlot]
	def, ok := w.items[item.Index]
	if !ok || w.timedItemDuration(item, mountSlot) <= 0 ||
		item.ExpiresUnix != 0 && item.ExpiresUnix <= now.Unix() {
		return premiumMountStats{}
	}
	// Timed horses 3980..3989 carry no stats in the item list; the catalog
	// mirrors the client g_pMountBonus2 table so score and tooltip agree.
	if stats, ok := w.mounts.PremiumStats(item.Index); ok {
		return premiumMountStats{attack: stats.Attack, magicPercent: stats.MagicPercent,
			evasion: stats.Evasion, resist: stats.Resist, runSpeed: stats.Speed}
	}
	return premiumMountStats{
		attack:       staticAbility(def, "EF_DAMAGE"),
		magicPercent: staticAbility(def, "EF_MAGIC"),
		runSpeed:     staticAbility(def, "EF_RUNSPEED"),
	}
}

// mountMaxHP returns the data-driven HP cap, falling back to the global cap.
func (w *World) mountMaxHP(sIndex uint16) int {
	if stats, ok := w.mounts.Stats(model.MountType(sIndex)); ok && stats.MaxHP > 0 {
		return stats.MaxHP
	}
	return model.MountMaxHP
}

// advanceMountStage adds 30 to the item index, adds random longevity and resets
// level. Essence uses bonus 14; growth potions use bonus 20.
func advanceMountStage(mount *model.Item, longevityBonus int, intn func(int) int) {
	mount.Index += model.MountTypeCount
	mount.SetMountLongev(mount.MountLongev() + intn(longevityBonus) + 1)
	mount.SetMountLevel(0)
	mount.SetMountFood(mount.MountFood()) // Preserve food; clear stEffect[2].cValue below.
	mount.Eff[5] = 0
}

// essenceMatchesMount requires (essence-2390)%30 to match the mount type.
func essenceMatchesMount(essenceIndex, mountIndex uint16) bool {
	if essenceIndex < 2390 {
		return false
	}
	amg := int(essenceIndex-2390) % model.MountTypeCount
	return amg == model.MountType(mountIndex)
}

// feedMatchesMount applies the Vol 15 mapping: several mount types share feed.
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

// mountFoodInterval is one complete eligible online hour with an equipped,
// living, unprotected adult mount. Each such hour consumes one food point.
const mountFoodInterval = time.Hour

// tickPlayerMounts drains adult mount food. Zero food removes HP and longevity;
// recalculation removes the bonus. Affect 51 suspends food consumption.
func (w *World) tickPlayerMounts(now time.Time) {
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || w.criticalPlayerReserved(p) {
			continue
		}

		// Incubation and pet reconciliation are independent of the food clock.
		w.tickEquippedEggIncubation(p, now)
		// Incubation can acquire a fence during this iteration. Do not reset
		// clocks or reconcile followers against its still-uncommitted proposal.
		if w.criticalPlayerReserved(p) {
			continue
		}
		w.syncCriaPet(p)

		mount, mslot := equippedMount(p.Char)
		if mount == nil || !model.IsMountAdult(mount.Index) || mount.MountHP() <= 0 ||
			mountInvulnActiveAt(p.Char, now) {
			// Unequipped, dead or protected time does not count toward the next hour.
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
			// Starvation removes HP, bonuses and 0..3 longevity points.
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

// Egg incubation.
//
// Eggs (2300-2329) use native STRUCT_ITEM effects:
//
//	EF_SANC(43)      = successful incubation refinements
//	EF_INCUDELAY(84) = online hours until the next attempt
//
// EF_INCUBATE comes from itemlist.csv and sets the threshold. A threshold-3 egg
// advances to +1, +2 and +3 with waits; the next successful refinement hatches
// it. Only online time with the same egg equipped in slot 14 counts.
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
	// The 7.48 flow decrements stEffect[2].cValue directly. Keep EF_INCUDELAY
	// in the third pair even if the second pair is empty.
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
	// Materialized items have UIDs. Legacy accounts/tests use this fallback;
	// there is only one incubation slot.
	return fmt.Sprintf("egg:%d", egg.Index)
}

func (w *World) resetEggIncubationClock(p *Player) {
	if p == nil || w.criticalPlayerReserved(p) {
		return
	}
	p.EggIncubationUID = ""
	p.NextEggIncubationTick = time.Time{}
}

func (w *World) startEggIncubationClock(p *Player, egg *model.Item, now time.Time) {
	if w.criticalPlayerReserved(p) {
		return
	}
	if p == nil || egg == nil || eggDelay(*egg) <= 0 {
		w.resetEggIncubationClock(p)
		return
	}
	p.EggIncubationUID = eggIncubationClockKey(egg)
	p.NextEggIncubationTick = now.Add(eggIncubationHour)
}

// tickEquippedEggIncubation charges complete online hours with the same egg in
// slot 14. Whole hours are persisted in the item; the fractional Player clock
// is discarded on unequip or logout.
func (w *World) tickEquippedEggIncubation(p *Player, now time.Time) {
	if p == nil || p.Char == nil || w.criticalPlayerReserved(p) {
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
	if w.criticalWrites != nil {
		w.submitEggIncubationHours(p, hours)
		return
	}
	oldEgg := *egg
	if !setEggDelay(egg, remaining-hours) {
		return
	}
	if p.Account != nil {
		if err := w.saveAccount(p.Account); err != nil {
			*egg = oldEgg
			log.Printf("[#%d] save egg %d incubation hour: %v", p.ID, egg.Index, err)
			return
		}
	}
	p.NextEggIncubationTick = p.NextEggIncubationTick.Add(time.Duration(hours) * eggIncubationHour)
	if eggDelay(*egg) == 0 {
		w.resetEggIncubationClock(p)
	}
	w.publishEggIncubationHours(p)
}

// Keep item state and the fractional clock unchanged until World acknowledges
// the persisted whole hours. Rejections never fall back to an inline save.
func (w *World) submitEggIncubationHours(p *Player, hours int) {
	if p.Account == nil || p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) ||
		p.Char != &p.Account.Chars[p.CharSlot] {
		log.Printf("Egg incubation rejected: %v", errCriticalPersistenceIdentity)
		return
	}
	proposal := accountStateSnapshot(p.Account)
	newEgg := proposal.Chars[p.CharSlot].Equip[mountSlot]
	if hours <= 0 || hours > eggDelay(newEgg) || !setEggDelay(&newEgg, eggDelay(newEgg)-hours) {
		return
	}
	proposal.Chars[p.CharSlot].Equip[mountSlot] = newEgg
	next := p.NextEggIncubationTick.Add(time.Duration(hours) * eggIncubationHour)
	_, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{proposal}}, []*Player{p},
		func(criticalPersistenceResult) error {
			p.Char.Equip[mountSlot] = newEgg
			p.NextEggIncubationTick = next
			if eggDelay(newEgg) == 0 {
				// The ordinary clock helper deliberately rejects reserved players.
				// This identity-checked continuation owns the committed transition.
				p.EggIncubationUID = ""
				p.NextEggIncubationTick = time.Time{}
			}
			w.publishEggIncubationHours(p)
			return nil
		})
	if err != nil {
		log.Printf("Egg incubation admission rejected: %v", err)
	}
}

func (w *World) publishEggIncubationHours(p *Player) {
	egg := &p.Char.Equip[mountSlot]
	if p.Session != nil && !p.Session.IsClosed() {
		p.Session.Send(wire.SendItem(p.ID, placeEquip, mountSlot, *egg))
		if eggDelay(*egg) == 0 {
			p.Session.Send(wire.MessagePanel("Incubation is complete. Apply Ori/Lac again to continue hatching."))
		} else {
			p.Session.Send(wire.MessagePanel(fmt.Sprintf("Egg incubation: %d hour(s) remaining.", eggDelay(*egg))))
		}
	}
}

// destEggTarget requires an equipped egg. Inventory/cargo pause incubation and
// do not accept Ori/Lac.
func (w *World) destEggTarget(p *Player, req useItemRequest) (*model.Item, uint32, uint32) {
	if p != nil && p.Char != nil && req.dstType == placeEquip && req.dstPos == mountSlot {
		if it := &p.Char.Equip[mountSlot]; model.IsMountEgg(it.Index) {
			return it, placeEquip, mountSlot
		}
	}
	return nil, 0, 0
}

// incubateEgg applies Ori/Lac (Vol 4/5), consuming powder on success or failure
// and starting a 6..8 online-hour wait. Success at the threshold hatches the egg.
func (w *World) incubateEgg(p *Player, s *net.Session, powder *model.Item, powderSlot byte,
	egg *model.Item, eggType, eggSlot uint32, vol int) {
	if p == nil || p.Char == nil || w.criticalPlayerReserved(p) {
		return
	}
	if w.criticalWrites != nil {
		w.admitMountConsumable(p, powderSlot, model.VolatileRule{},
			useItemRequest{dstType: eggType, dstPos: eggSlot}, true)
		return
	}
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
		egg.Index += model.MountTypeCount // 2300 -> 2330 (hatchling).
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
	consumeOne(powder) // Refinement consumes Ori/Lac on both success and failure.
	if err := w.saveAccount(p.Account); err != nil {
		*egg, *powder = oldEgg, oldPowder
		log.Printf("[#%d] save incubation: %v", s.ID, err)
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
	log.Printf("[#%d] incubation vol=%d egg->%d progress=+%d threshold=%d wait=%dh hatched=%v roll=%d/%d",
		s.ID, vol, egg.Index, eggProgress(*egg), threshold, eggDelay(*egg), hatched, roll.Roll, roll.Chance)
}

// accelerateHatch (Vol 196, item 3438) immediately hatches an equipped egg.
// Consumption and transformation are confirmed in the same save.
func (w *World) accelerateHatch(p *Player, s *net.Session, item *model.Item, invSlot byte,
	req useItemRequest) {
	if p == nil || p.Char == nil || w.criticalPlayerReserved(p) {
		return
	}
	if w.criticalWrites != nil {
		w.admitMountConsumable(p, invSlot, model.VolatileRule{MountAction: "hatch"}, req, false)
		return
	}
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
		log.Printf("[#%d] save egg %d hatch accelerator: %v", s.ID, oldEgg.Index, err)
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
	log.Printf("[#%d] accelerator hatched egg %d -> hatchling %d", s.ID, oldEgg.Index, egg.Index)
}

// Hatchling follower pets.
//
// Hatchlings (2330-2358) follow their owner as passive mobs; adult mounts are
// ridden. Face 315+type (315-344) and SummonerID reuse summon infrastructure.
// See historical Server.cpp MountProcess/LinkMountHp.

const (
	criaPetFaceBase = 315
	criaPetFaceEnd  = criaPetFaceBase + model.MountTypeCount // 345
)

func isCriaPetFace(face uint16) bool {
	return face >= criaPetFaceBase && face < criaPetFaceEnd
}

// findCriaPet finds an owner's living hatchling follower.
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

// spawnCriaPet creates the equipped hatchling's follower mob.
func (w *World) spawnCriaPet(p *Player, cria *model.Item) {
	if p == nil || p.Char == nil || cria == nil || w.criticalPlayerReserved(p) {
		return
	}
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
		Name: "Hatchling^", Tipo: model.TipoMonstro,
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
		log.Printf("[#%d] hatchling pet spawn rejected: mob ID range exhausted", p.ID)
		return
	}
	m := &Mob{ID: mobID, Def: def, X: x, Y: y, HP: uint32(hp),
		InstanceID: w.playerRuntimeInstanceID(p.ID),
		GenerIndex: -1, SummonerID: p.ID, SummonKind: summonKindMount, SummonRange: mobAttackRange}
	w.appendMobInstance(m)
	w.publishMobSpawn(m)
	log.Printf("[#%d] hatchling pet spawned face=%d", p.ID, face)
}

// removeCriaPet removes the pet from the scene. Lifecycle callers own ordering.
func (w *World) removeCriaPet(m *Mob) {
	for _, viewer := range w.players {
		w.hideMob(viewer, m, 0)
	}
	m.Dead = true
	w.removeMobInstance(m)
}

// syncCriaPet reconciles the follower with the equipped living hatchling.
// Reserved owners cannot spawn, remove or rebind pets. Future asynchronous
// mount continuations must explicitly order reconciliation with acknowledgement.
func (w *World) syncCriaPet(p *Player) {
	if p == nil || p.Char == nil || !p.InWorld || w.criticalPlayerReserved(p) {
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

// Hatchling hunting experience.

// criaKillsPerLevel counts qualifying mob kills per hatchling level. The
// current value is 2; historical descriptions refer to growth value + 100 EXP.
const criaKillsPerLevel = 2

// Hunting increments Eff[5] when mob level >= hatchling level. At the threshold
// it gains a level and may evolve. Adult mounts gain levels from essence instead.
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
	if p == nil || p.Char == nil || m == nil || m.Def == nil || w.criticalPlayerReserved(p) {
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
		return result // Lower-level mobs do not grant hatchling experience.
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
		w.syncCriaPet(p) // Adult evolution removes the follower.
	}
}

// mountInvulnActiveAt tests active mount protection (affect 51).
func mountInvulnActiveAt(ch *model.Char, now time.Time) bool {
	for i := range ch.Affects {
		if ch.Affects[i].Type == 51 && ch.Affects[i].ExpiresAt.After(now) {
			return true
		}
	}
	return false
}

// loseMountLongevity removes 0..3 longevity points on mount death, starvation
// or rider death. Zero longevity prevents ordinary revival.
func loseMountLongevity(mount *model.Item, intn func(int) int) {
	mount.SetMountLongev(mount.MountLongev() - intn(4))
}

// absorbMountDamage uses the adult mount's type-specific rider damage share
// (65..75 in g_pMountBonus[30][6]). Zero mount HP removes stat bonuses and
// 0..3 longevity. The return value is rider damage. The KR level-120 bonus
// remains unassigned at this boundary without flow evidence.
func (w *World) absorbMountDamage(target *Player, incoming int) int {
	if incoming <= 0 || target == nil || target.Char == nil {
		return incoming
	}
	if w.criticalPlayerReserved(target) {
		return 0
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
		return rider // Affect 51 prevents mount HP loss.
	}
	absorbed := incoming - rider
	hp := mount.MountHP() - absorbed
	if hp < 0 {
		hp = 0
	}
	mount.SetMountHP(hp)
	if hp == 0 {
		// Injury removes longevity and bonuses, then publishes the new state.
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

// mountRiderDied charges 0..3 mount longevity points when the rider dies.
func (w *World) mountRiderDied(target *Player) {
	if target == nil || target.Char == nil || w.criticalPlayerReserved(target) {
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

// mountGrowthRate uses g_pSancRate[2], indexed by level/10: 100% at 0-9,
// decreasing to 20% at 110-119.
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

// applyMountItem validates consumable matching and persists before confirmation.
func (w *World) applyMountItem(p *Player, s *net.Session, item *model.Item, invSlot byte,
	rule model.VolatileRule, code int, req useItemRequest) {
	if p == nil || p.Char == nil || w.criticalPlayerReserved(p) {
		return
	}
	if w.criticalWrites != nil {
		w.admitMountConsumable(p, invSlot, rule, req, false)
		return
	}
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
	usedIndex := item.Index // Consumption may clear Index; log the original value.
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
		log.Printf("[#%d] unknown mountAction %q item=%d", s.ID, rule.MountAction, item.Index)
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
		log.Printf("[#%d] save mount %s: %v", s.ID, rule.MountAction, err)
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
	w.syncCriaPet(p) // Remove an evolved hatchling or retain its follower.
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
	log.Printf("[#%d] mount %s item=%d mount=%d lvl=%d hp=%d food=%d longevity=%d",
		s.ID, rule.MountAction, usedIndex, mount.Index, mount.MountLevel(), mount.MountHP(), mount.MountFood(), mount.MountLongev())
}

// mountEssenceOutcome retains percent rolls until persistence succeeds.
// Rolls[0] is adult growth; optional Rolls[1] is the even-level failure penalty.
type mountEssenceOutcome struct {
	OK      bool
	Message string
	Rolls   []percentRoll
}

// mountEssence (Vol 16) restores 20000 HP and attempts level growth. Adults
// use g_pSancRate[2]; failed even-level growth may lose a level (20% below 50,
// 40% at 50-99, 70% at 100+). Rolls use 1..100, succeeding at roll <= chance.
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

// initShopMounts initializes NPC stock after mount options load. Without HP,
// food and longevity initialization, zero-effect mounts would be sold dead.
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

// initFreshMount initializes zero-effect mounts with HP, food and 10..30
// longevity. Returns whether it initialized the item, avoiding zero-HP grants.
func (w *World) initFreshMount(mount *model.Item) bool {
	if mount == nil || !model.IsMount(mount.Index) {
		return false
	}
	if mount.MountHP() != 0 || mount.MountLevel() != 0 || mount.MountLongev() != 0 || mount.MountFood() != 0 {
		return false // Already initialized.
	}
	mount.SetMountHP(w.mountMaxHP(mount.Index))
	mount.SetMountLongev(w.intn(21) + 10) // 10..30
	if model.IsMountAdult(mount.Index) {
		mount.SetMountFood(model.MountMaxFood) // Adult cap: 100.
	} else {
		mount.SetMountFood(30) // Hatchling cap: 30.
	}
	return true
}

// mountFeed (Vol 15) adds 5000 HP (cap 30000) and 2 food (cap 100). A dead
// mount with longevity revives at 100 HP and 5 food, using mount-master values.
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
		return true, "Mount revived (100 HP, 5 food)."
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
	return true, "Mount fed."
}

// mountLongevityRecoverWithRNG (Vol 93) adds 1..3 longevity (cap 60).
// Zero or maximum longevity is rejected.
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

// mountLongevityRestore (item 3315) restores LP, not HP or invulnerability.
// itemhelp.dat and the legacy handler agree; zero LP is rejected, cap 60.
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

// mountSetLevel handles items 3316/3317: adults below 100 reach 100, or adults
// at 100..119 reach 120. Both require at least 3 LP.
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

// mountGrowthWithRNG (Vol 94) evolves the stage immediately (sIndex += 30).
// Potion category (3344+) must match the mount family; level is not required.
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

// growthCategory maps mount sIndex to Vol 94 growth potion category (3344+).
func growthCategory(mountIndex uint16) int {
	m := int(mountIndex)
	switch {
	case m >= 2333 && m <= 2335:
		return 0 // Kapel: Saber Tooth, Lesser Dragon, Bear.
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
