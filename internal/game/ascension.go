package game

import (
	"fmt"
	"log"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// ascension.go -- a cadeia Mortal -> Arch, portada de
// SERVER W2PP/SOURCE SERVER/Code/TMSrv/_MSG_Quest.cpp (cases COMP_SEPHI,
// BLACKORACLE e KING) e DBSrv/CFileDB.cpp (_MSG_DBCreateArchCharacter).
//
// Sao tres etapas encadeadas, cada uma num NPC diferente:
//
//  1. Sefirot   (mestre de skill da classe): 8 pedras Sephira + 30M de gold.
//  2. Pedra Eterna (Black_Oracle): Unicon + Phoenix adjacentes + 10 safiras.
//  3. Arch      (rei): Mortal 370+, com a Pedra no slot 10 e o Sefirot no 11.
//
// O Arch nasce como personagem NOVO no primeiro slot livre; o Mortal continua
// existindo e jogavel, exatamente como no nativo.

const (
	// Itens da cadeia, todos conferidos em data/itemlist.csv.
	unicornSpiritItem = 1740 // Unicon's_Spirit
	phoenixSpiritItem = 1741 // Phoenix's_Spirit
	eternalStoneItem  = 1742 // Eternal_Stone (a "Pedra da Imortalidade")
	// sefirotFirstItem+classe: 1760 TK, 1761 FM, 1762 BM, 1763 HT.
	sefirotFirstItem = 1760
	// sephiraFirstStone..+7: Kocuna, Veean, Keced, Gebra, Tipalet, Nechark,
	// Yesord, Marcute.
	sephiraFirstStone = 1744
	sephiraStoneCount = 8

	sefirotGoldCost       = 30_000_000
	eternalStoneSapphires = 10

	// archMinLevel e o nivel INTERNO exigido pelo rei. A fonte W2PP usa 380,
	// mas os tutoriais da comunidade divergem (355, 370, 300) porque cada
	// servidor calibra o proprio portao; 370 e a regra escolhida para o nosso
	// -- e casa com "when a mortal reaches level 370-400 it can be reborn".
	archMinLevel = 370

	// Slots de equipamento usados pelo rei na ascensao.
	eternalStoneSlot = 10
	sefirotSlot      = 11

	// archFaceOffset porta `Equip[0] = MortalFace + 5 + cls`. Com os rostos do
	// nosso catalogo (TK 1, FM 11, BM 21, HT 31) resulta em 6, 17, 28 e 39 --
	// todos existentes e da classe certa.
	archFaceOffset = 5

	// archEvolution e o valor gravado em Char.Evolution. Qualquer valor nao
	// vazio ja exclui o personagem das regras MortalOnly (quest e volatile).
	archEvolution = "arch"
)

// sefirotClassForNPC resolve a classe pelo NOME do MESTRE DE SKILL.
//
// O compositor do Sefirot e o mestre de skill da classe -- nao um NPC dedicado.
// Confirmado in-game (o client abre a janela de confirmacao no Foema_Ancian) e
// na fonte goldvivi ("Skills Teacher (Armia town) - creates Sephiroth").
//
// A classe de cada mestre foi identificada pela FAIXA DE SKILLS que ele vende
// (indice 5000 + skill), que e criterio objetivo:
//
//	Knight_Leader  5000..5023 -> skills 0..23   -> TK
//	Foema_Ancian   5024..5047 -> skills 24..47  -> FM
//	Archi_Master   5048..5071 -> skills 48..71  -> BM
//	ForeLearner    5072..5095 -> skills 72..95  -> HT
//
// O nativo distingue pelo campo Exp do mob (994..997); os nossos JSONs nao
// preservam esse discriminador, entao o nome e a fonte confiavel -- mesmo
// criterio ja usado pelos reis em kingdom.go.
func sefirotClassForNPC(def *model.NPCDef) (int, bool) {
	if def == nil {
		return 0, false
	}
	switch strings.ToLower(generName(def.Name)) {
	case "knight leader":
		return 0, true
	case "foema ancian":
		return 1, true
	case "archi master":
		return 2, true
	case "forelearner", "fore learner":
		return 3, true
	}
	return 0, false
}

// hasSephiraStones informa se as oito pedras estao no inventario. Serve de
// GATE do clique: sem elas, o clique no mestre e um pedido de loja de skill
// comum e nao pode virar tentativa de composicao.
func hasSephiraStones(ch *model.Char) bool {
	return findSephiraStones(ch) != nil
}

func isBlackOracle(def *model.NPCDef) bool {
	return def != nil && strings.EqualFold(generName(def.Name), "Black Oracle")
}

// handleAscensionNPC trata o Black_Oracle (Pedra Eterna). O Sefirot NAO passa
// por aqui: o compositor dele e um mestre de skill, cujo clique precisa ser
// interceptado ANTES da abertura da loja -- ver onNPCClick.
func (w *World) handleAscensionNPC(s *net.Session, p *Player, m *Mob) bool {
	if p == nil || p.Char == nil || m == nil || m.Def == nil {
		return false
	}
	if isBlackOracle(m.Def) {
		w.craftEternalStone(s, p)
		return true
	}
	return false
}

// findSephiraStones devolve os slots das 8 pedras (uma de cada indice), ou nil
// se faltar alguma. Uma pedra por tipo, como no laco nativo.
func findSephiraStones(ch *model.Char) []int {
	slots := make([]int, 0, sephiraStoneCount)
	for offset := 0; offset < sephiraStoneCount; offset++ {
		wanted := uint16(sephiraFirstStone + offset)
		found := -1
		for slot := 0; slot < model.PlayerCarrySlots; slot++ {
			if ch.Inv[slot].Index == wanted {
				found = slot
				break
			}
		}
		if found < 0 {
			return nil
		}
		slots = append(slots, found)
	}
	return slots
}

// craftSefirot porta o COMP_SEPHI: oito pedras Sephira e 30 milhoes de gold
// viram o Sefirot da classe do NPC.
func (w *World) craftSefirot(s *net.Session, p *Player, class int) {
	stones := findSephiraStones(p.Char)
	if stones == nil {
		s.Send(wire.MessagePanel("Bring the eight Sephira stones."))
		return
	}
	if p.Char.Gold < sefirotGoldCost {
		s.Send(wire.MessagePanel("Eight Sephira stones and 30,000,000 gold are required."))
		return
	}
	sefirot := model.Item{Index: uint16(sefirotFirstItem + class)}
	// Confere espaco ANTES de consumir: as pedras liberam slots, entao a sobra
	// e garantida, mas a checagem mantem a ordem "valida tudo, so entao muta".
	if freeInventorySlots(p.Char) == 0 && len(stones) == 0 {
		s.Send(wire.MessagePanel("Inventory is full."))
		return
	}

	snapshot := cloneCharacterState(p.Char)
	for _, slot := range stones {
		p.Char.Inv[slot] = model.Item{}
	}
	p.Char.Gold -= sefirotGoldCost
	target := addToInv(p.Char, sefirot)
	if target < 0 {
		*p.Char = snapshot
		s.Send(wire.MessagePanel("Inventory is full."))
		return
	}
	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		log.Printf("[#%d] ERRO ao salvar Sefirot: %v", s.ID, err)
		s.Send(wire.MessagePanel("Save failed. Nothing was consumed."))
		w.resendInventory(p)
		return
	}
	for _, slot := range stones {
		s.Send(wire.SendItem(p.ID, placeInv, byte(slot), p.Char.Inv[slot]))
	}
	s.Send(wire.SendItem(p.ID, placeInv, byte(target), p.Char.Inv[target]))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	s.Send(wire.MessagePanel("The Sefirot has been created."))
	log.Printf("[#%d] SEFIROT criado classe=%d item=%d", s.ID, class, sefirot.Index)
}

// findAdjacentSpirits acha Unicon (1740) seguido de Phoenix (1741) no slot
// IMEDIATAMENTE posterior. A adjacencia e exigencia do nativo, nao detalhe de
// implementacao: e assim que o jogador "monta" o par no inventario.
func findAdjacentSpirits(ch *model.Char) (int, bool) {
	for slot := 0; slot+1 < model.PlayerCarrySlots; slot++ {
		if ch.Inv[slot].Index == unicornSpiritItem && ch.Inv[slot+1].Index == phoenixSpiritItem {
			return slot, true
		}
	}
	return 0, false
}

// craftEternalStone porta o BLACKORACLE: os dois espiritos adjacentes mais dez
// safiras viram a Pedra Eterna.
func (w *World) craftEternalStone(s *net.Session, p *Player) {
	slot, ok := findAdjacentSpirits(p.Char)
	if !ok {
		s.Send(wire.MessagePanel("Place Unicon's Spirit and Phoenix's Spirit side by side, in that order."))
		return
	}
	if sapphireCount(p.Char) < eternalStoneSapphires {
		s.Send(wire.MessagePanel("10 Sapphires are required."))
		return
	}

	snapshot := cloneCharacterState(p.Char)
	if !consumeSapphires(p.Char, eternalStoneSapphires) {
		*p.Char = snapshot
		s.Send(wire.MessagePanel("The Sapphires could not be consumed."))
		return
	}
	p.Char.Inv[slot] = model.Item{}
	p.Char.Inv[slot+1] = model.Item{}
	target := addToInv(p.Char, model.Item{Index: eternalStoneItem})
	if target < 0 {
		*p.Char = snapshot
		s.Send(wire.MessagePanel("Inventory is full."))
		return
	}
	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		log.Printf("[#%d] ERRO ao salvar Pedra Eterna: %v", s.ID, err)
		s.Send(wire.MessagePanel("Save failed. Nothing was consumed."))
		w.resendInventory(p)
		return
	}
	w.resendInventory(p)
	s.Send(wire.MessagePanel("The Eternal Stone has been forged."))
	log.Printf("[#%d] PEDRA ETERNA criada slot=%d", s.ID, target)
}

// resendInventory reenvia o inventario inteiro. Usado pelas composicoes que
// mexem em varios slots de uma vez (safiras avulsas + pacotes), onde enumerar
// os slots alterados sairia mais caro e mais fragil que um refresh.
func (w *World) resendInventory(p *Player) {
	if p == nil || p.Char == nil || p.Session == nil {
		return
	}
	p.Session.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
	p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
}

// archClassFromSefirot devolve a classe do Arch a partir do Sefirot equipado.
func archClassFromSefirot(item model.Item) (int, bool) {
	class := int(item.Index) - sefirotFirstItem
	if class < 0 || class > 3 {
		return 0, false
	}
	return class, true
}

// freeCharacterSlot devolve o primeiro slot vazio da conta, como o laco do
// DBSrv nativo. -1 quando nao ha espaco.
func freeCharacterSlot(account *model.Account) int {
	for slot := 0; slot < maxCharactersPerAccount; slot++ {
		if slot >= len(account.Chars) || account.Chars[slot].Name == "" {
			return slot
		}
	}
	return -1
}

// growCharacterSlots cresce Account.Chars ate conter o slot pedido e RE-APONTA
// p.Char.
//
// O append pode REALOCAR o array. Como p.Char e `&p.Account.Chars[CharSlot]`
// (handlers.go), o ponteiro ficaria orfao no array antigo: toda escrita
// posterior -- consumo de item, rollback, e dali em diante a sessao inteira --
// iria para memoria que ninguem persiste, enquanto saveAccount grava o array
// novo. Na ascensao isso deixava a Pedra e o Sefirot INTACTOS no disco, ou
// seja, ascensao repetivel.
func (w *World) growCharacterSlots(p *Player, slot int) {
	if p == nil || p.Account == nil {
		return
	}
	for len(p.Account.Chars) <= slot {
		p.Account.Chars = append(p.Account.Chars, model.Char{})
	}
	// Re-aponta mesmo quando nao houve realocacao: e barato e nao depende de
	// adivinhar se o append cresceu o array.
	if p.CharSlot >= 0 && p.CharSlot < len(p.Account.Chars) {
		p.Char = &p.Account.Chars[p.CharSlot]
	}
}

// createArch porta a criacao do Arch no rei. Devolve true quando o NPC foi
// tratado por este caminho -- inclusive nas recusas, para o rei nao seguir e
// tentar entregar uma capa de reino no mesmo clique.
//
// O Mortal PERMANECE na conta: o Arch e um personagem NOVO, com o mesmo nome,
// no primeiro slot livre. E o comportamento nativo.
func (w *World) createArch(s *net.Session, p *Player) bool {
	stone := p.Char.Equip[eternalStoneSlot]
	sefirot := p.Char.Equip[sefirotSlot]
	// Sem os dois itens equipados o jogador nao esta pedindo ascensao: deixa o
	// clique seguir para a logica de reino.
	if stone.Index != eternalStoneItem {
		return false
	}
	class, ok := archClassFromSefirot(sefirot)
	if !ok {
		return false
	}

	if strings.TrimSpace(p.Char.Evolution) != "" {
		s.Send(wire.MessagePanel("Only a Mortal can ascend."))
		return true
	}
	if p.Char.Extended == nil || p.Char.Extended.Level < archMinLevel {
		s.Send(wire.MessagePanel("You must be level 371 to ascend."))
		return true
	}
	slot := freeCharacterSlot(p.Account)
	if slot < 0 {
		s.Send(wire.MessagePanel("There is no room for a new character on this account."))
		return true
	}

	// O Arch herda o NOME do Mortal (fiel ao nativo), entao NAO passa pela
	// checagem de unicidade: o nome ja pertence a esta conta.
	arch, built := w.newCharacterFromTemplate(p.Char.Name, class)
	if !built {
		log.Printf("[#%d] layout ausente para classe %d na ascensao", s.ID, class)
		s.Send(wire.MessagePanel("The Arch could not be created."))
		return true
	}
	arch.Evolution = archEvolution
	// Guarda a origem: o slot alimenta o recalculo a cada login e o nivel fica
	// em cache para o calculo de pontos, que so recebe o Char.
	arch.ArchMortalSlot = p.CharSlot
	arch.ArchMortalLevel = p.Char.Extended.Level
	// Rosto do Arch = rosto do Mortal + 5 + classe (CFileDB.cpp:1993).
	arch.Equip[0].Index = p.Char.Equip[0].Index + archFaceOffset + uint16(class)

	previousChars := append([]model.Char(nil), p.Account.Chars...)
	previousEquip := p.Char.Equip
	// Cresce a fatia ANTES de qualquer escrita por p.Char: growCharacterSlots
	// re-aponta o ponteiro, que o append poderia deixar orfao.
	w.growCharacterSlots(p, slot)
	p.Account.Chars[slot] = arch
	// A Pedra e o Sefirot sao consumidos pela ascensao.
	p.Char.Equip[eternalStoneSlot] = model.Item{}
	p.Char.Equip[sefirotSlot] = model.Item{}

	if err := w.saveAccount(p.Account); err != nil {
		// Trocar a fatia tambem invalida p.Char: re-aponta ANTES de restaurar o
		// equipamento, senao o rollback grava no array descartado.
		p.Account.Chars = previousChars
		if p.CharSlot >= 0 && p.CharSlot < len(p.Account.Chars) {
			p.Char = &p.Account.Chars[p.CharSlot]
		}
		p.Char.Equip = previousEquip
		log.Printf("[#%d] ERRO ao salvar ascensao de %q: %v", s.ID, p.Char.Name, err)
		s.Send(wire.MessagePanel("Save failed. The ascension was not completed."))
		return true
	}

	s.Send(wire.SendItem(p.ID, placeEquip, eternalStoneSlot, p.Char.Equip[eternalStoneSlot]))
	s.Send(wire.SendItem(p.ID, placeEquip, sefirotSlot, p.Char.Equip[sefirotSlot]))
	w.refreshAppearance(p)
	s.Send(wire.MessagePanel("Your Arch has been created. It awaits at character selection."))
	// Anuncio ao mundo, DEPOIS do save: ascensao que falhou nao vira noticia.
	// Mesmo canal e cor do /spk ([SERVER], cor 7). "God" e o termo em ingles
	// para o Arch -- e como o client global e as fontes o chamam.
	announcement := fmt.Sprintf("The character %s has become a God!", arch.Name)
	w.broadcast(func() []byte { return wire.MessageWhisper(0, "[SERVER]", announcement, 7) })
	log.Printf("[#%d] ARCH criado nome=%q classe=%d slot=%d rosto=%d (mortal nivel=%d)",
		s.ID, arch.Name, class, slot, arch.Equip[0].Index, p.Char.Extended.Level)
	return true
}

// isArch informa se o personagem ja ascendeu.
func isArch(ch *model.Char) bool {
	return ch != nil && strings.EqualFold(ch.Evolution, archEvolution)
}

// refreshArchMortalLevel atualiza, no login, o nivel do Mortal de origem de cada
// Arch da conta.
//
// O nativo recalcula esse valor a cada entrada (ProcessDBMessage.cpp:1002), e e
// isso que faz o Arch continuar ganhando pontos conforme o Mortal sobe de nivel
// depois da ascensao. Mortal apagado mantem o ultimo valor conhecido, em vez de
// zerar pontos ja distribuidos.
func refreshArchMortalLevel(account *model.Account) bool {
	if account == nil {
		return false
	}
	changed := false
	for i := range account.Chars {
		arch := &account.Chars[i]
		if arch.Name == "" || !isArch(arch) {
			continue
		}
		slot := arch.ArchMortalSlot
		if slot < 0 || slot >= len(account.Chars) || slot == i {
			continue
		}
		mortal := &account.Chars[slot]
		if mortal.Name == "" || mortal.Extended == nil {
			continue
		}
		if arch.ArchMortalLevel != mortal.Extended.Level {
			arch.ArchMortalLevel = mortal.Extended.Level
			changed = true
		}
	}
	return changed
}
