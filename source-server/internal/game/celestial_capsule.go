package game

import (
	"encoding/binary"
	"log"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	capsuleGridLimit = uint16(4096)
	maxCapsuleName   = 12
)

func celestialEvolution(value string) bool {
	switch strings.ToLower(strings.TrimSpace(value)) {
	case "celestial", "subcelestial":
		return true
	default:
		return false
	}
}

func (w *World) capsuleByID(account *model.Account, id uint16) (*model.CelestialCapsule, int) {
	if account == nil || id == 0 {
		return nil, -1
	}
	for i := range account.CelestialCapsules {
		if account.CelestialCapsules[i].ID == id {
			return &account.CelestialCapsules[i], i
		}
	}
	return nil, -1
}

func accountOwnsCapsuleSeal(account *model.Account, capsule *model.CelestialCapsule) bool {
	if account == nil || capsule == nil {
		return false
	}
	matches := 0
	check := func(item model.Item) {
		if item.UID != capsule.ItemUID {
			return
		}
		if id, ok := model.CelestialSealID(item); ok && id == capsule.ID {
			matches++
		}
	}
	for i := 0; i < model.PlayerCargoSlots; i++ {
		check(account.Cargo[i])
	}
	for charIndex := range account.Chars {
		if account.Chars[charIndex].Name == "" {
			continue
		}
		for slot := 0; slot < model.PlayerCarrySlots; slot++ {
			check(account.Chars[charIndex].Inv[slot])
		}
	}
	return matches == 1
}

func nextCelestialCapsuleID(account *model.Account) (uint16, bool) {
	if account == nil {
		return 0, false
	}
	used := make(map[uint16]struct{}, len(account.CelestialCapsules))
	for _, capsule := range account.CelestialCapsules {
		used[capsule.ID] = struct{}{}
	}
	for n := 1; n <= 65535; n++ {
		id := uint16(n)
		if _, exists := used[id]; !exists {
			return id, true
		}
	}
	return 0, false
}

func firstCargoSlot(account *model.Account) int {
	if account == nil {
		return -1
	}
	for i := 0; i < model.PlayerCargoSlots; i++ {
		if account.Cargo[i].Index == 0 {
			return i
		}
	}
	return -1
}

func validCapsuleCharacterName(name string) bool {
	return len(name) >= 4 && len(name) <= maxCapsuleName && validCharacterName(name)
}

func capsuleInfoFor(capsule *model.CelestialCapsule) wire.CapsuleInfoData {
	var info wire.CapsuleInfoData
	if capsule == nil || capsule.Character.Score == nil {
		return info
	}
	ch := &capsule.Character
	e := ch.Score
	info.Class = capsuleDisplayClass(ch)
	info.Level = clampCapsuleStat(e.Level)
	info.Str = clampCapsuleStat(e.Str)
	info.Int = clampCapsuleStat(e.Int)
	info.Dex = clampCapsuleStat(e.Dex)
	info.Con = clampCapsuleStat(e.Con)
	info.Mastery[0] = clampCapsuleStat(e.Mastery[0])
	info.Mastery[1] = clampCapsuleStat(e.Mastery[1])
	info.Skill = capsuleSkillSummary(ch.Class, ch.LearnedSkill)
	// O client 7.48 interpreta Quest como quatro nibbles cumulativos. A ficha
	// Celestial completa a cadeia Arch ao possuir os quatro cristais.
	if ch.ArchCrystals >= 4 {
		info.Quest = 0x1000
	}
	return info
}

func capsuleDisplayClass(ch *model.Char) uint16 {
	if ch == nil {
		return 0
	}
	celestialClass := ch.Class
	if ch.AlternateCelestial == nil {
		return uint16(celestialClass) * 10
	}
	subClass := ch.AlternateCelestial.Class
	if strings.EqualFold(strings.TrimSpace(ch.Evolution), "subcelestial") {
		celestialClass, subClass = ch.AlternateCelestial.Class, ch.Class
	}
	// A UI decodifica C*10+(S+6) como "SubCelestial [Celestial]".
	return uint16(celestialClass)*10 + uint16(subClass) + 6

}

func capsuleSkillSummary(class byte, learned uint32) [9]uint16 {
	var result [9]uint16
	for branch := 0; branch < 3; branch++ {
		var learnedInBranch []uint16
		for local := branch * 8; local < branch*8+8; local++ {
			if learned&(uint32(1)<<local) != 0 {
				learnedInBranch = append(learnedInBranch, uint16(int(class)*24+local))
			}
		}
		if len(learnedInBranch) > 3 {
			learnedInBranch = learnedInBranch[len(learnedInBranch)-3:]
		}
		copy(result[branch*3:branch*3+3], learnedInBranch)
	}
	return result
}

func clampCapsuleStat(value uint32) uint16 {
	const maxClientShort = uint32(32767)
	if value > maxClientShort {
		return uint16(maxClientShort)
	}
	return uint16(value)
}

// useCelestialCapsule implementa o trecho de _MSG_UseItem.cpp que o W2PP
// reserva ao Spirit's Seal vazio. O personagem sai do mundo apenas depois da
// transacao confirmar: em falha, item, ficha e charstate permanecem intactos.
func (w *World) useCelestialCapsule(s *net.Session, p *Player, item *model.Item, slot byte) {
	resend := func() {
		if p != nil && p.ID != 0 && item != nil {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		}
	}
	if p == nil || p.Account == nil || p.Char == nil || item == nil ||
		item.Index != model.CelestialSealItem || item.Eff[0] != 0 {
		resend()
		return
	}
	if !celestialEvolution(p.Char.Evolution) {
		resend()
		s.Send(wire.MessagePanel("Only a Celestial can use an empty Spirit's Seal."))
		return
	}
	if p.Char.UID == "" || p.Char.Score == nil {
		resend()
		return
	}
	for _, capsule := range p.Account.CelestialCapsules {
		if capsule.SourceUID == p.Char.UID {
			resend()
			s.Send(wire.MessagePanel("This character is already sealed."))
			return
		}
	}
	id, ok := nextCelestialCapsuleID(p.Account)
	if !ok {
		resend()
		s.Send(wire.MessagePanel("No capsule identifiers are available."))
		return
	}
	seal, err := materializeItem(model.NewCelestialSeal("", id))
	if err != nil {
		resend()
		return
	}
	cargoSlot := firstCargoSlot(p.Account)
	if cargoSlot < 0 {
		resend()
		s.Send(wire.MessagePanel("Your Cargo is full."))
		return
	}

	oldChar := cloneCharacterState(p.Char)
	oldChars := append([]model.Char(nil), p.Account.Chars...)
	oldCargo := p.Account.Cargo
	oldCapsules := append([]model.CelestialCapsule(nil), p.Account.CelestialCapsules...)
	sealedCharacter := cloneCharacterState(p.Char)
	// Timed affects are session state, not progression. The native capsule file
	// stores the character snapshot but does not keep active server timers.
	sealedCharacter.Affects = [16]model.Affect{}
	sealedCharacter.RuntimeScore = nil
	sealedCharacter.X, sealedCharacter.Y = w.charSpawn.X, w.charSpawn.Y
	consumeOne(&sealedCharacter.Inv[slot])
	p.Account.CelestialCapsules = append(p.Account.CelestialCapsules, model.CelestialCapsule{
		ID: id, ItemUID: seal.UID, SourceUID: p.Char.UID, Character: sealedCharacter,
		CreatedUnix: w.now().Unix(),
	})
	p.Account.Cargo[cargoSlot] = seal
	charSlot := p.CharSlot
	oldName := p.Char.Name
	if charSlot < 0 || charSlot >= len(p.Account.Chars) {
		p.Account.CelestialCapsules = oldCapsules
		p.Account.Cargo = oldCargo
		resend()
		return
	}
	p.Account.Chars[charSlot] = model.Char{}
	if err := w.saveAccountAndCharStateForUID(p.Account, oldChar.UID, nil); err != nil {
		p.Account.Chars = oldChars
		p.Account.Cargo = oldCargo
		p.Account.CelestialCapsules = oldCapsules
		p.Char = &p.Account.Chars[charSlot]
		log.Printf("[#%d] encapsulamento Celestial cancelado: %v", s.ID, err)
		resend()
		return
	}
	if w.charNames != nil && !accountUsesName(p.Account, oldName) {
		delete(w.charNames, strings.ToLower(oldName))
	}
	name := oldName
	// O slot persistido ja esta vazio, mas o teardown ainda precisa da ficha
	// antiga para fechar loja, trade, party e summons com a identidade correta.
	p.Char = &oldChar
	w.removePlayerFromWorld(p, "encapsulamento Celestial")
	// Capsule transitions rebuild selection using the login-negotiated ABI.
	s.Send(selectionUpdatePacket(s, wire.OpCNFDeleteCharacter, uint16(s.ID), p))
	s.Send(characterListPacket(s, p))
	s.Send(wire.MessagePanel("Celestial character sealed successfully."))
	log.Printf("[#%d] Celestial %q encapsulado id=%d cargo=%d", s.ID, name, id, cargoSlot)
}

type putoutSealRequest struct {
	sourceType, sourcePos uint32
	destType, destPos     uint32
	gridX, gridY          uint16
	warpID                uint16
	name                  string
}

func parsePutoutSealRequest(pkt []byte) (putoutSealRequest, bool) {
	if len(pkt) != 52 {
		return putoutSealRequest{}, false
	}
	req := putoutSealRequest{
		sourceType: binary.LittleEndian.Uint32(pkt[12:16]),
		sourcePos:  binary.LittleEndian.Uint32(pkt[16:20]),
		destType:   binary.LittleEndian.Uint32(pkt[20:24]),
		destPos:    binary.LittleEndian.Uint32(pkt[24:28]),
		gridX:      binary.LittleEndian.Uint16(pkt[28:30]),
		gridY:      binary.LittleEndian.Uint16(pkt[30:32]),
		warpID:     binary.LittleEndian.Uint16(pkt[32:34]),
		name:       cstr(pkt[34:50]),
	}
	return req, true
}

// onCapsuleInfo responde ao hover do item 3443. O ID vem dos dois bytes de
// EF_VALUE; nunca aceitamos um índice alegado pelo cliente sem conferir o
// item e o snapshot persistidos na conta.
func (w *World) onCapsuleInfo(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Account == nil || !p.InWorld || len(pkt) != 16 {
		return
	}
	id := uint16(binary.LittleEndian.Uint32(pkt[12:16]))
	capsule, _ := w.capsuleByID(p.Account, id)
	if capsule == nil || !accountOwnsCapsuleSeal(p.Account, capsule) {
		return
	}
	s.Send(wire.CNFCapsuleInfo(uint16(s.ID), id, capsuleInfoFor(capsule)))
}

// onPutoutSeal porta _MSG_PutoutSeal. A ficha extraida nasce em um slot livre
// da conta, preserva rosto/capa e progressao Celestial, mas perde equipamento,
// carry, gold e guild como no DBSrv nativo.
func (w *World) onPutoutSeal(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Account == nil || p.Char == nil || !p.InWorld ||
		playerCurHP(p.Char) == 0 {
		return
	}
	// O DBSrv encerra qualquer negociação antes de manipular o selo. Além de
	// reproduzir o fluxo nativo, isso impede que o mesmo item fique reservado
	// simultaneamente numa janela de trade e na extração.
	if p.Trade != nil {
		w.cancelTrade(p, "extracao Celestial")
		return
	}
	req, ok := parsePutoutSealRequest(pkt)
	if !ok || req.gridX >= capsuleGridLimit || req.gridY >= capsuleGridLimit ||
		(req.sourceType != placeInv && req.sourceType != placeStorage) {
		return
	}
	var maxSource uint32 = model.PlayerCarrySlots
	if req.sourceType == placeStorage {
		maxSource = model.PlayerCargoSlots
	}
	if req.sourcePos >= maxSource {
		return
	}
	if req.sourceType == placeStorage && !w.nearCargoNPC(p) {
		s.Send(wire.MessagePanel("Open your Cargo near a Cargo Keeper first."))
		return
	}
	source := playerSlotOf(p, byte(req.sourceType), byte(req.sourcePos))
	if source == nil || source.Index != model.CelestialSealItem {
		s.Send(wire.MessagePanel("This is not a Spirit's Seal."))
		return
	}
	id, ok := model.CelestialSealID(*source)
	if !ok || source.UID == "" {
		s.Send(wire.MessagePanel("The Spirit's Seal is empty or invalid."))
		return
	}
	capsule, capsuleIndex := w.capsuleByID(p.Account, id)
	if capsule == nil || capsule.ItemUID != source.UID {
		s.Send(wire.MessagePanel("The Spirit's Seal data is unavailable."))
		return
	}
	if !validCapsuleCharacterName(req.name) {
		s.Send(wire.MessagePanel("Invalid character name."))
		return
	}
	taken, err := w.characterNameTaken(req.name)
	if err != nil || taken {
		s.Send(wire.MessagePanel("That character name is already in use."))
		return
	}
	newSlot := -1
	for i := 0; i < maxCharactersPerAccount; i++ {
		if i >= len(p.Account.Chars) || p.Account.Chars[i].Name == "" {
			newSlot = i
			break
		}
	}
	if newSlot < 0 {
		s.Send(wire.MessagePanel("There is no empty character slot."))
		return
	}
	newUID, err := model.NewCharacterUID()
	if err != nil {
		return
	}
	created := cloneCharacterState(&capsule.Character)
	created.UID, created.Name = newUID, req.name
	created.X, created.Y = w.charSpawn.X, w.charSpawn.Y
	created.RuntimeScore = nil
	created.Affects = [16]model.Affect{}
	// W2PP keeps only the face and cape (Equip[0] and Equip[15]) from the
	// capsule. All carry is cleared before the new character is committed.
	face, cape := created.Equip[0], created.Equip[model.CapeSlot]
	created.Equip = [16]model.Item{}
	created.Equip[0], created.Equip[model.CapeSlot] = face, cape
	created.Inv = [64]model.Item{}
	created.Gold = 0
	created.GuildID, created.GuildRank = 0, 0
	w.recalcPlayer(&created)
	setPlayerCurHP(&created, playerMaxHP(&created))
	setPlayerCurMP(&created, playerMaxMP(&created))

	oldChars := append([]model.Char(nil), p.Account.Chars...)
	oldCargo := p.Account.Cargo
	oldCapsules := append([]model.CelestialCapsule(nil), p.Account.CelestialCapsules...)
	activeUID := p.Char.UID
	activeState := buildCharState(p, w.now())
	if newSlot >= len(p.Account.Chars) {
		for len(p.Account.Chars) <= newSlot {
			p.Account.Chars = append(p.Account.Chars, model.Char{})
		}
	}
	p.Account.Chars[newSlot] = created
	// append pode mover o backing array; Player.Char sempre deve voltar a
	// apontar para a ficha autoritativa dentro de Account.Chars.
	p.Char = &p.Account.Chars[p.CharSlot]
	// Appending a character can reallocate the slice; resolve the source again
	// instead of writing through a stale pointer into the old backing array.
	source = playerSlotOf(p, byte(req.sourceType), byte(req.sourcePos))
	if source == nil || source.Index != model.CelestialSealItem {
		p.Account.Chars = oldChars
		p.Account.Cargo = oldCargo
		p.Account.CelestialCapsules = oldCapsules
		p.Char = &p.Account.Chars[p.CharSlot]
		return
	}
	*source = model.Item{}
	p.Account.CelestialCapsules = append(p.Account.CelestialCapsules[:capsuleIndex],
		p.Account.CelestialCapsules[capsuleIndex+1:]...)
	if err := w.saveAccountAndCharStateForUID(p.Account, activeUID, activeState); err != nil {
		p.Account.Chars = oldChars
		p.Account.Cargo = oldCargo
		p.Account.CelestialCapsules = oldCapsules
		p.Char = &p.Account.Chars[p.CharSlot]
		s.Send(wire.MessagePanel("The character could not be extracted."))
		return
	}
	if w.charNames != nil {
		w.charNames[strings.ToLower(req.name)] = struct{}{}
	}
	w.removePlayerFromWorld(p, "extracao Celestial")
	// Extraction follows the same source/stock selection boundary as creation.
	s.Send(selectionUpdatePacket(s, wire.OpCNFNewCharacter, uint16(s.ID), p))
	s.Send(characterListPacket(s, p))
	s.Send(wire.MessagePanel("Celestial character extracted successfully."))
	log.Printf("[#%d] Celestial capsule id=%d extraida como %q slot=%d", s.ID, id, req.name, newSlot)
}
