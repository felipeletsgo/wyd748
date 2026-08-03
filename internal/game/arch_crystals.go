package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// Os quatro cristais elementais de Kefra, portados do volatile 187 nativo
// (_MSG_UseItem.cpp:7184). Sao feitos como ARCH e cada um acrescenta 100
// pontos ao futuro orcamento do Celestial. Eles NAO sao requisito para criar
// o Celestial; o progresso fica em QuestInfo.Arch.Cristal.
//
// NOTA DE DIVERGENCIA: o `if` do nativo testa ClassMaster CELESTIAL. Aqui o
// portao e ARCH porque o mesmo trecho exige nivel 355, inalcançavel pela
// progressao Celestial 7.54 (cap interno 199). O campo no bloco Arch e o uso
// posterior no calculo de pontos confirmam que se trata de progresso do Arch,
// nao de uma trava da ascensao.
const (
	firstArchCrystal = uint16(4106) // Elime, Sylphid, Salion, Nohas
	archCrystalCount = 4
	// archCrystalExpCost e o preco de cada cristal. O nivel e RECALCULADO a
	// partir da EXP reduzida, entao o personagem desce de nivel -- e assim no
	// nativo, nao e efeito colateral.
	archCrystalExpCost = uint32(100_000_000)
	// archCrystalMinLevel e o nivel INTERNO minimo, na mesma convencao do
	// archMinLevel do rei. O nativo exige BaseScore.Level >= 355
	// (_MSG_UseItem.cpp:6362) e o texto do client repete "the ascension
	// requirement is level 355".
	//
	// O piso e conferido ANTES de descontar, e nao depois: como cada cristal
	// rebaixa o nivel, verificar no fim deixaria o segundo em diante
	// impossivel -- o proprio desconto derrubaria o personagem abaixo do piso.
	archCrystalMinLevel = uint32(355)
)

// archCrystalStep devolve a posicao do cristal na sequencia (1..4).
func archCrystalStep(index uint16) (byte, bool) {
	if index < firstArchCrystal || index >= firstArchCrystal+archCrystalCount {
		return 0, false
	}
	return byte(index-firstArchCrystal) + 1, true
}

// levelForExp acha o nivel correspondente a uma EXP. Usado para REBAIXAR o
// personagem depois do desconto: grantExp so sobe, nunca desce.
func levelForExp(exp uint32) uint32 {
	level := uint32(0)
	for i := 1; i < len(mortalNextLevel); i++ {
		if exp < mortalNextLevel[i] {
			break
		}
		level = uint32(i)
	}
	if level > maxMortalLevel {
		level = maxMortalLevel
	}
	return level
}

// useArchCrystal consome um cristal elemental. Ordem obrigatoria: o cristal N
// so vale se os N-1 anteriores ja foram feitos.
func (w *World) useArchCrystal(s *net.Session, p *Player, item *model.Item, slot byte, rule model.VolatileRule, code int) {
	ch := p.Char
	passo, ok := archCrystalStep(item.Index)
	if !ok {
		log.Printf("[#%d] cristal %d fora da faixa %d..%d", s.ID, item.Index,
			firstArchCrystal, firstArchCrystal+archCrystalCount-1)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if !isArch(ch) {
		s.Send(wire.MessagePanel("Only a God can use this crystal."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.Extended.Level < archCrystalMinLevel {
		s.Send(wire.MessagePanel("You must reach level 355 to take this oath."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.ArchCrystals >= passo {
		s.Send(wire.MessagePanel("You have already done it."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.ArchCrystals != passo-1 {
		s.Send(wire.MessagePanel("You must take the oaths in order."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	// GUARDA DE UNDERFLOW. Na pratica o piso de nivel 355 ja exige mais de 2
	// bilhoes de EXP, muito acima do custo -- mas Exp e uint32, e sem esta
	// checagem um estado inconsistente (edicao manual, tabela de nivel
	// alterada, piso reduzido) faria `Exp -= custo` dar a volta para ~4
	// bilhoes, presenteando o jogador em vez de cobra-lo.
	if ch.Exp < archCrystalExpCost {
		s.Send(wire.MessagePanel("You need 100 million experience for this oath."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	anteriorItem, anteriorExp := *item, ch.Exp
	anteriorNivel, anteriorCristais := ch.Extended.Level, ch.ArchCrystals

	ch.ArchCrystals = passo
	ch.Exp -= archCrystalExpCost
	ch.Extended.Level = levelForExp(ch.Exp)
	syncProgression(ch)
	if rule.Consume {
		consumeOne(item)
	}
	w.recalcPlayer(ch)

	// Persist-before-confirm: 100 milhoes de EXP e caro demais para sair sem
	// estar em disco. Uma falha aqui devolve tudo.
	if err := w.saveAccount(p.Account); err != nil {
		*item, ch.Exp = anteriorItem, anteriorExp
		ch.Extended.Level, ch.ArchCrystals = anteriorNivel, anteriorCristais
		syncProgression(ch)
		w.recalcPlayer(ch)
		s.Send(wire.MessagePanel("Save failed. The oath was not taken."))
		log.Printf("[#%d] ERRO ao salvar cristal %d: %v", s.ID, anteriorItem.Index, err)
		return
	}

	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	s.Send(wire.UpdateScore(p.ID, *ch))
	s.Send(wire.UpdateEtc(p.ID, *ch))
	w.syncPlayerVitalsToObservers(p)
	if ch.ArchCrystals == archCrystalCount {
		s.Send(wire.MessagePanel("The four oaths are complete."))
	}
	log.Printf("[#%d] CRISTAL %d (%d/%d) item=%d volatile=%d exp=%d->%d nivel=%d->%d",
		s.ID, passo, ch.ArchCrystals, archCrystalCount, anteriorItem.Index, code,
		anteriorExp, ch.Exp, anteriorNivel, ch.Extended.Level)
}
