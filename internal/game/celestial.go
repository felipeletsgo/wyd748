package game

import (
	"fmt"
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	idealStoneItem       = uint16(5338)
	mysteriousStoneItem  = uint16(4148)
	furyStoneItem        = uint16(3020)
	arcanaCytheraItem    = uint16(3507)
	celestialSoulBit     = uint32(1 << 30)
	celestialFaceType    = byte(3)
	subCelestialFaceType = byte(4)
	subCelestialFameCost = uint32(100)
)

var cityBounds = [...]struct{ minX, minY, maxX, maxY uint16 }{
	{2052, 2052, 2171, 2163}, // Armia
	{2432, 1672, 2675, 1767}, // Azran
	{2448, 1966, 2476, 2024}, // Erion
	{3605, 3090, 3690, 3260}, // Niflheim
	{1036, 1700, 1072, 1760}, // Noatum
}

func inNativeCity(x, y uint16) bool {
	for _, city := range cityBounds {
		if x >= city.minX && x <= city.maxX && y >= city.minY && y <= city.maxY {
			return true
		}
	}
	return false
}

func celestialArchTier(level uint32) byte {
	// Faixas oficiais WYD KR, convertidas do nivel exibido para o nivel
	// interno do protocolo: 356-370, 371-380, 381-398, 399 e 400.
	switch {
	case level >= 399:
		return 5
	case level == 398:
		return 4
	case level >= 380:
		return 3
	case level >= 370:
		return 2
	case level >= 355:
		return 1
	default:
		return 0
	}
}

func celestialCytheraForTier(tier byte) uint16 {
	switch tier {
	case 1, 2:
		return 3500
	case 3, 4:
		return 3501
	case 5:
		return 3502
	default:
		return 0
	}
}

func newCelestialScore(class byte, previous *model.Score) *model.Score {
	if class > 3 {
		class = 0
	}
	natural := baseClassStats[class]
	score := &model.Score{
		Version: model.ScoreVersion,
		// Ramo normal (isHardCore=0) da Pedra Ideal no W2PP.
		Attack: 488, Defense: 954,
		MaxHP: uint32(baseClassHPMP[class][0]),
		MaxMP: uint32(baseClassHPMP[class][1]),
		Str:   uint32(natural[0]), Int: uint32(natural[1]),
		Dex: uint32(natural[2]), Con: uint32(natural[3]),
		MasterPts: 855,
	}
	if previous != nil {
		score.AttackRun = previous.AttackRun
		score.Merchant = previous.Merchant
		score.RegenHP = previous.RegenHP
		score.RegenMP = previous.RegenMP
		score.Range = previous.Range
	}
	score.CurHP, score.CurMP = score.MaxHP, score.MaxMP
	return score
}

// markCelestialFace projeta no item de rosto os dois bytes que o client 7.48
// usa para distinguir Mortal/Arch/Celestial/Sub. Evolution continua sendo a
// fonte server-side, mas omitir EFV2 faria a UI interpretar um Celestial como
// a evolução anterior mesmo com os cálculos corretos no servidor.
func markCelestialFace(face *model.Item, evolution string) {
	if face == nil {
		return
	}
	switch strings.ToLower(strings.TrimSpace(evolution)) {
	case "celestial":
		face.Eff[3] = celestialFaceType
	case "subcelestial":
		face.Eff[3] = subCelestialFaceType
	default:
		return
	}
	face.Eff[5] = byte(face.Index)
}

func celestialTransformationActive(ch *model.Char, now time.Time) bool {
	if ch == nil {
		return false
	}
	for i := range ch.Affects {
		affect := ch.Affects[i]
		if (affect.Type == 16 || affect.Type == affectFaceTransform) &&
			affect.ExpiresAt.After(now) {
			return true
		}
	}
	return false
}

func celestialInventoryHasItem(ch *model.Char, index uint16) bool {
	if ch == nil || index == 0 {
		return false
	}
	for i := range ch.Inv {
		if ch.Inv[i].Index == index {
			return true
		}
	}
	return false
}

func activeCelestialForm(ch *model.Char) model.CelestialForm {
	return model.CelestialForm{
		Evolution: ch.Evolution, Class: ch.Class, Face: ch.Equip[0],
		Score: ch.Score, Exp: ch.Exp, LearnedSkill: ch.LearnedSkill,
		SecondaryLearnedSkill: ch.SecondaryLearnedSkill,
		ShortSkill:            ch.ShortSkill, Affects: ch.Affects,
	}
}

func loadCelestialForm(ch *model.Char, form model.CelestialForm) {
	ch.Evolution, ch.Class, ch.Equip[0] = form.Evolution, form.Class, form.Face
	ch.Score, ch.Exp, ch.LearnedSkill = form.Score, form.Exp, form.LearnedSkill
	ch.SecondaryLearnedSkill = form.SecondaryLearnedSkill
	ch.ShortSkill, ch.Affects = form.ShortSkill, form.Affects
	ch.RuntimeScore = nil
}

func (w *World) syncCelestialPlayer(p *Player) {
	if p == nil || p.Char == nil || p.Session == nil {
		return
	}
	markCelestialFace(&p.Char.Equip[0], p.Char.Evolution)
	w.recalcPlayer(p.Char)
	filterShortSkills(p.Char)
	p.Session.Send(playerScorePacket(p))
	p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	p.Session.Send(playerAffectsPacket(p))
	p.Session.Send(wire.SetShortSkill(p.ID, p.Char.ShortSkill))
	p.Session.Send(wire.SelfEquip(p.ID, p.Char.Equip[:]))
	w.syncPlayerVitals(p)
	w.refreshAppearance(p)
	w.updatePartyMember(p)
}

// craftIdealStoneAtKing porta o primeiro ramo de KingDom do Secrets 7.54.
// A Pedra Ideal nao nasce no Odin: o rei consome as quatro Pedras Secretas do
// inventario, a Imortalidade/Eternal equipada e o Sephirot equipado. Arch usa
// a primeira Ideal para virar Celestial; Celestial usa outra para criar a Sub.
func (w *World) craftIdealStoneAtKing(s *net.Session, p *Player) bool {
	if p == nil || p.Char == nil ||
		!advancedEvolution(p.Char, archEvolution, "celestial", "subcelestial") ||
		p.Char.Equip[10].Index != 1742 {
		return false
	}
	if _, ok := archClassFromSefirot(p.Char.Equip[sefirotSlot]); !ok {
		return false
	}
	for stone := uint16(5334); stone <= 5337; stone++ {
		if !celestialInventoryHasItem(p.Char, stone) {
			return false
		}
	}

	snapshot := cloneCharacterState(p.Char)
	for stone := uint16(5334); stone <= 5337; stone++ {
		if !consumeInventoryItem(p.Char, stone, 1) {
			*p.Char = snapshot
			return true
		}
	}
	consumeOne(&p.Char.Equip[10])
	consumeOne(&p.Char.Equip[sefirotSlot])

	ideal, err := materializeItem(model.Item{Index: idealStoneItem})
	if err != nil {
		*p.Char = snapshot
		s.Send(wire.MessagePanel("The Ideal Stone could not be created."))
		return true
	}
	target := addToInv(p.Char, ideal)
	if target < 0 {
		*p.Char = snapshot
		s.Send(wire.MessagePanel("Inventory is full. Nothing was consumed."))
		return true
	}
	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		log.Printf("[#%d] salvar composicao da Pedra Ideal: %v", s.ID, err)
		s.Send(wire.MessagePanel("Save failed. Nothing was consumed."))
		return true
	}
	w.resendInventory(p)
	s.Send(wire.SendItem(p.ID, placeEquip, 10, p.Char.Equip[10]))
	s.Send(wire.SendItem(p.ID, placeEquip, sefirotSlot, p.Char.Equip[sefirotSlot]))
	s.Send(wire.MessagePanel("The Ideal Stone was created successfully."))
	log.Printf("[#%d] PEDRA IDEAL criada inv[%d]", s.ID, target)
	return true
}

func (w *World) useCelestialIdeal(s *net.Session, p *Player, item *model.Item, slot byte) {
	if p == nil || p.Char == nil || item == nil || item.Index != idealStoneItem {
		return
	}
	if celestialTransformationActive(p.Char, w.now()) {
		s.Send(wire.MessagePanel("The Ideal Stone cannot be used while transformed."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	ch := p.Char
	switch strings.ToLower(strings.TrimSpace(ch.Evolution)) {
	case archEvolution:
		w.createCelestial(s, p, item, slot)
	case "celestial":
		w.createSubCelestial(s, p, item, slot)
	default:
		s.Send(wire.MessagePanel("There is no Celestial quest available for this character."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	}
}

func (w *World) createCelestial(s *net.Session, p *Player, item *model.Item, slot byte) {
	ch := p.Char
	if ch.Score == nil || ch.Score.Level < 355 {
		s.Send(wire.MessagePanel("The Arch must be level 356 or higher."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.ArchMortalLevel < maxMortalLevel {
		s.Send(wire.MessagePanel("The respective Mortal must be level 400."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	tier := celestialArchTier(ch.Score.Level)
	cytheraIndex := celestialCytheraForTier(tier)
	if tier == 0 || cytheraIndex == 0 {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	cythera, err := materializeItem(model.Item{Index: cytheraIndex})
	if err != nil {
		s.Send(wire.MessagePanel("The Cythera could not be created."))
		return
	}
	snapshot := cloneCharacterState(ch)
	ch.CelestialArchTier = tier
	ch.CelestialLevel40Unlocked = false
	ch.CelestialLevel90Unlocked = false
	ch.CelestialCytheraTier = 0
	ch.CelestialArcana = false
	ch.AlternateCelestial = nil
	ch.Evolution = "celestial"
	markCelestialFace(&ch.Equip[0], ch.Evolution)
	ch.Exp = 0
	ch.Score = newCelestialScore(ch.Class, snapshot.Score)
	ch.Score.Defense = celestialBaseDefense(snapshot.ArchCrystals)
	ch.RuntimeScore = nil
	ch.LearnedSkill = celestialSoulBit
	ch.SecondaryLearnedSkill = 0
	ch.ShortSkill = [20]byte{}
	ch.Affects = [16]model.Affect{}
	ch.Equip[1] = cythera
	kingdom := model.KingdomFromCape(snapshot.Equip[model.CapeSlot].Index)
	cape, _ := model.KingdomCapeAtTier(model.CapeTierMaster, kingdom)
	if snapshot.Equip[model.CapeSlot].UID != "" {
		// O nativo zera a capa anterior antes de entregar a Master. A identidade
		// econômica server-side permanece, mas refino/tintura/effects da capa
		// Arch não atravessam a ascensão.
		ch.Equip[model.CapeSlot] = model.Item{
			Index: cape,
			UID:   snapshot.Equip[model.CapeSlot].UID,
		}
	} else {
		ch.Equip[model.CapeSlot], err = materializeItem(model.Item{Index: cape})
		if err != nil {
			*ch = snapshot
			return
		}
	}
	consumeOne(item)
	syncProgression(ch)
	w.recalcPlayer(ch)
	setPlayerCurHP(ch, playerMaxHP(ch))
	setPlayerCurMP(ch, playerMaxMP(ch))
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		*ch = snapshot
		log.Printf("[#%d] salvar criacao Celestial: %v", s.ID, err)
		s.Send(wire.MessagePanel("Save failed. The Ideal Stone was not consumed."))
		return
	}
	s.Send(wire.MessagePanel("Your character is now Celestial. Re-enter it from character selection."))
	announcement := fmt.Sprintf("The character %s has become Celestial!", ch.Name)
	w.broadcast(func() []byte { return wire.MessageWhisper(0, "[SERVER]", announcement, 7) })
	log.Printf("[#%d] CELESTIAL criado tierArch=%d cythera=%d", s.ID, tier, cytheraIndex)
	w.returnToCharacterSelectionAfterCommittedChange(p, "criacao de Celestial")
}

func (w *World) createSubCelestial(s *net.Session, p *Player, item *model.Item, slot byte) {
	ch := p.Char
	if ch.AlternateCelestial != nil {
		s.Send(wire.MessagePanel("The SubCelestial has already been created."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.Score == nil || ch.Score.Level < 120 {
		s.Send(wire.MessagePanel("The Celestial must be level 121 or higher."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if counterBalance(p, fameCounter) < subCelestialFameCost {
		s.Send(wire.MessagePanel("100 Fame is required to create a SubCelestial."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	subClass, ok := archClassFromSefirot(ch.Equip[sefirotSlot])
	if !ok {
		s.Send(wire.MessagePanel("Equip the SubCelestial class Sefirot."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	mystery := model.Item{Index: mysteriousStoneItem}
	setItemAmount(&mystery, 10)
	mystery, err := materializeItem(mystery)
	if err != nil {
		return
	}
	snapshot := cloneCharacterState(ch)
	oldCounters := copyCounters(p)
	subFace := ch.Equip[0]
	bodyBase := int(subFace.Index) - int(ch.Class)
	if bodyBase < 0 {
		s.Send(wire.MessagePanel("The Celestial face is invalid."))
		return
	}
	subFace.Index = uint16(bodyBase + subClass)
	markCelestialFace(&subFace, "subcelestial")
	ch.AlternateCelestial = &model.CelestialForm{
		Evolution: "subcelestial", Class: byte(subClass), Face: subFace,
		Score:        newCelestialScore(byte(subClass), ch.Score),
		LearnedSkill: celestialSoulBit,
	}
	ch.AlternateCelestial.Score.Defense = celestialBaseDefense(ch.ArchCrystals)
	consumeOne(item)
	ch.Equip[sefirotSlot] = model.Item{}
	target := addToInv(ch, mystery)
	if target < 0 {
		*ch = snapshot
		p.SpecialCoins = oldCounters
		s.Send(wire.MessagePanel("Inventory is full."))
		return
	}
	if !spendCounters(p, map[string]uint32{fameCounter: subCelestialFameCost}) {
		*ch = snapshot
		p.SpecialCoins = oldCounters
		s.Send(wire.MessagePanel("100 Fame is required to create a SubCelestial."))
		return
	}
	syncProgression(ch) // passa a incluir 6 x nivel da forma alterna
	w.recalcPlayer(ch)
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		*ch = snapshot
		p.SpecialCoins = oldCounters
		log.Printf("[#%d] salvar criacao SubCelestial: %v", s.ID, err)
		s.Send(wire.MessagePanel("Save failed. Nothing was consumed."))
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	s.Send(wire.SendItem(p.ID, placeInv, byte(target), ch.Inv[target]))
	s.Send(wire.SendItem(p.ID, placeEquip, sefirotSlot, ch.Equip[sefirotSlot]))
	w.syncCelestialPlayer(p)
	s.Send(wire.MessagePanel("Your SubCelestial has been created. Use the Mysterious Stone to switch."))
	log.Printf("[#%d] SUBCELESTIAL criado classe=%d", s.ID, subClass)
}

func (w *World) useMysteriousStone(s *net.Session, p *Player, item *model.Item, slot byte) {
	if p == nil || p.Char == nil || item == nil || item.Index != mysteriousStoneItem {
		return
	}
	ch := p.Char
	if ch.AlternateCelestial == nil || !isCelestialEvolution(ch) {
		s.Send(wire.MessagePanel("This character has no alternate Celestial form."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if !inNativeCity(p.X, p.Y) {
		s.Send(wire.MessagePanel("The Mysterious Stone can only be used inside a city."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	snapshot := cloneCharacterState(ch)
	current := activeCelestialForm(ch)
	next := *ch.AlternateCelestial
	loadCelestialForm(ch, next)
	ch.AlternateCelestial = &current
	consumeOne(item)
	syncProgression(ch)
	w.recalcPlayer(ch)
	filterShortSkills(ch)
	if playerCurHP(ch) == 0 {
		setPlayerCurHP(ch, playerMaxHP(ch))
	}
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		*ch = snapshot
		log.Printf("[#%d] salvar troca Celestial: %v", s.ID, err)
		s.Send(wire.MessagePanel("Save failed. The form was not changed."))
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	s.Send(wire.SendItem(p.ID, placeEquip, 0, ch.Equip[0]))
	w.syncCelestialPlayer(p)
	s.Send(wire.MessagePanel("Celestial form changed."))
	log.Printf("[#%d] forma ativa=%s classe=%d level=%d", s.ID, ch.Evolution, ch.Class, ch.Score.Level)
}

func (w *World) useFuryStone(s *net.Session, p *Player, item *model.Item, slot byte) {
	if p == nil || p.Char == nil || item == nil {
		return
	}
	if item.Index != furyStoneItem || !isCelestialEvolution(p.Char) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	ch := p.Char
	if counterBalance(p, fameCounter) < 500 {
		s.Send(wire.MessagePanel("500 Fame is required."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.Score.Level < maxCelestialLevel {
		if ch.Score.Level != 89 || ch.CelestialLevel90Unlocked ||
			!advancedEvolution(ch, "celestial") {
			s.Send(wire.MessagePanel("This unlock is only available at Celestial level 90."))
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		snapshot, oldCounters := cloneCharacterState(ch), copyCounters(p)
		ch.CelestialLevel90Unlocked = true
		spendCounters(p, map[string]uint32{fameCounter: 500})
		consumeOne(item)
		if err := w.saveAccountAndCharStateResult(p); err != nil {
			*ch, p.SpecialCoins = snapshot, oldCounters
			s.Send(wire.MessagePanel("Save failed. Nothing was consumed."))
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("Celestial level 90 has been unlocked."))
		return
	}
	if ch.CelestialArcana || ch.Equip[1].Index != 3502 || itemSanc(ch.Equip[1]) < 11 {
		s.Send(wire.MessagePanel("Equip a Mystic Cythera +11 or higher."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	for stone := uint16(5334); stone <= 5337; stone++ {
		if !celestialInventoryHasItem(ch, stone) {
			s.Send(wire.MessagePanel("The four Secret Stones are required."))
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
	}
	snapshot, oldCounters := cloneCharacterState(ch), copyCounters(p)
	for stone := uint16(5334); stone <= 5337; stone++ {
		consumeInventoryItem(ch, stone, 1)
	}
	spendCounters(p, map[string]uint32{fameCounter: 500})
	consumeOne(item)
	success := w.intn(2) != 0
	if success {
		ch.Equip[1].Index = arcanaCytheraItem
		ch.CelestialArcana = true
	}
	w.recalcPlayer(ch)
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		*ch, p.SpecialCoins = snapshot, oldCounters
		s.Send(wire.MessagePanel("Save failed. Nothing was consumed."))
		return
	}
	w.resendInventory(p)
	s.Send(wire.SendItem(p.ID, placeEquip, 1, ch.Equip[1]))
	w.syncCelestialPlayer(p)
	if success {
		s.Send(wire.MessagePanel("The Arcana Cythera was created."))
	} else {
		s.Send(wire.MessagePanel("The Arcana composition failed."))
	}
}

func celestialCytheraMilestone(level uint32) (tier byte, ac, hp byte) {
	switch {
	case level >= 198:
		return 4, 50, 70
	case level >= 180:
		return 3, 45, 60
	case level >= 150:
		return 2, 35, 50
	case level >= 120:
		return 1, 30, 40
	default:
		return 0, 0, 0
	}
}

func updateCelestialCythera(ch *model.Char) bool {
	if !isCelestialEvolution(ch) || ch.Score == nil ||
		ch.Equip[1].Index < 3500 || ch.Equip[1].Index > 3507 {
		return false
	}
	tier, ac, hp := celestialCytheraMilestone(ch.Score.Level)
	if tier == 0 || tier <= ch.CelestialCytheraTier {
		return false
	}
	item := &ch.Equip[1]
	var sancValue byte
	for i := 0; i < 3; i++ {
		effect := item.Eff[i*2]
		if effect == 43 || effect >= 116 && effect <= 125 {
			sancValue = item.Eff[i*2+1]
			break
		}
	}
	// PutAddOnItem da 7.54 apaga todos os pares antigos. Se havia refino,
	// preserva somente seu valor bruto e o normaliza para EF_SANC=43; tintura
	// e demais efeitos não sobrevivem. Uma Cythera sem refino não ganha +3
	// artificialmente.
	if sancValue == 0 {
		item.Eff = [6]byte{3, ac, 4, hp}
	} else {
		item.Eff = [6]byte{43, sancValue, 3, ac, 4, hp}
	}
	ch.CelestialCytheraTier = tier
	ch.RuntimeScore = nil
	return true
}
