package game

import (
	"encoding/binary"
	"log"
	"strings"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// removePlayerFromWorld desfaz apenas o estado efemero do personagem. A conta
// e a sessao continuam autenticadas para que 0x215 possa voltar a tela de
// selecao sem uma nova conexao TCP.
func (w *World) removePlayerFromWorld(p *Player, reason string) {
	if p == nil {
		return
	}
	w.detachPlayerFromItemInstances(p.ID, w.now())
	w.unregisterPlayerSpatial(p)
	delete(w.playersByID, p.ID)
	w.closeGhostShop(p, reason)
	w.cancelTrade(p, reason)
	w.removePartyPlayer(p)
	// Evocacoes pertencem ao dono: despawnam quando ele sai do mundo, senao
	// ficariam orfas e seguiriam o proximo player a reusar este ID.
	w.removePlayerSummons(p.ID)
	if p.InWorld {
		for _, other := range w.players {
			if other != p && other.InWorld && other.hasVisible(p.ID) {
				other.Session.Send(wire.RemoveMob(p.ID, 0))
				other.hide(p.ID)
			}
		}
	}
	p.InWorld = false
	resetCharacterRuntime(p)
}

// resetCharacterRuntime zera TODO estado do Player que pertence ao PERSONAGEM,
// preservando apenas o que e da sessao (Session, Account).
//
// O Player e REUSADO no vaivem da tela de selecao: a conta continua
// autenticada e o mesmo objeto recebe o proximo personagem. Qualquer campo
// esquecido aqui vaza para ele -- foi assim que as moedas especiais passavam de
// um personagem para outro (e o autosave as gravava, duplicando), que os
// cooldowns de skill valiam para o personagem errado e que um convite de guild
// feito a A podia ser aceito por B.
//
// Ao acrescentar campo de personagem ao Player, zere-o aqui.
// TestCharacterRuntimeIsFullyReset quebra se isso for esquecido.
func resetCharacterRuntime(p *Player) {
	if p == nil {
		return
	}
	p.Char = nil
	p.CharSlot = -1
	p.ID = 0
	p.X, p.Y = 0, 0
	p.Visible = nil

	// Contexto de NPC/janela aberta.
	p.ShopNPC = 0
	p.CraftNPC = 0
	p.BrowsingGhostShopID = 0
	p.GhostShop = nil
	p.Trade = nil

	// Grupo e guild: convite e do PERSONAGEM, nao da conta.
	p.Party = nil
	p.InviteFrom = 0
	p.InviteUntil = time.Time{}
	p.GuildInviteFrom = 0
	p.GuildInviteUntil = time.Time{}
	p.NextGuildInvite = time.Time{}
	p.Rebuy = [maxRebuyEntries]RebuyEntry{}

	// Combate.
	p.CombatTargetID = 0
	p.LastAttackerID = 0
	p.LastAttackAt = time.Time{}
	p.LastAttackTick = 0
	p.AttackProgress = 0
	p.DeadAt = time.Time{}
	p.PKMode = false

	// Cooldowns e temporizadores.
	p.SkillReady = nil
	p.LastPotion = time.Time{}
	p.LastCraft = time.Time{}
	p.NextRegen = time.Time{}
	p.NextCPRecovery = time.Time{}
	p.NextMountTick = time.Time{}
	p.NextKingdomTeleport = time.Time{}

	// Movimento publicado aos observadores.
	p.MovePublished = false
	p.MovePublishedStartX = 0
	p.MovePublishedStartY = 0
	p.MovePublishedTargetX = 0
	p.MovePublishedTargetY = 0
	p.MovePublishedRoute = [maxMovementRouteBytes]byte{}
	p.MoveBudget = 0
	p.MoveBudgetAt = time.Time{}

	// Moedas especiais vivem no charstate do PERSONAGEM.
	p.SpecialCoins = nil
}

// onCharacterLogout trata 0x215. A resposta 0x116 e comprovadamente a
// transicao que o client 7.48 espera para voltar a TM_SELECTCHAR_STATE.
func (w *World) onCharacterLogout(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) != 12 {
		return
	}
	charID, name := p.ID, p.Char.Name
	// Persiste buffs/moedas ANTES de sair do mundo, para sobreviverem ao retorno
	// a selecao e a reentrada.
	w.saveCharState(p)
	w.removePlayerFromWorld(p, "retorno a selecao")
	// Character logout also detaches a private Water member. Persist that UID
	// association before the session returns to character select.
	w.flushInstanceStateIfDirty()
	if p.Account != nil {
		if err := w.saveAccount(p.Account); err != nil {
			log.Printf("[#%d] ERRO ao salvar conta %q no character-logout: %v", s.ID, p.Account.Name, err)
		}
	}
	s.Send(wire.CNFCharacterLogout(charID))
	log.Printf("[#%d] CHARACTER-LOGOUT %q -> selecao", s.ID, name)
}

// onDeleteCharacter valida a senha de novo porque 0x211 e uma operacao
// destrutiva. O cliente envia Slot@12, MobName@16 e Password@32 (44 bytes).
func (w *World) onDeleteCharacter(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Account == nil || p.InWorld || len(pkt) != 44 {
		return
	}
	slot := int(int32(binary.LittleEndian.Uint32(pkt[12:16])))
	name, password := cstr(pkt[16:32]), cstr(pkt[32:44])
	if slot < 0 || slot >= len(p.Account.Chars) || p.Account.Chars[slot].Name == "" ||
		!strings.EqualFold(name, p.Account.Chars[slot].Name) || p.Account.PasswordHash == "" {
		s.Send(wire.MessagePanel("The character could not be deleted."))
		return
	}
	ok, err := account.VerifyPassword(p.Account.PasswordHash, password)
	if err != nil || !ok {
		log.Printf("[#%d] exclusao recusada para %q: senha invalida", s.ID, name)
		s.Send(wire.MessagePanel("Wrong password."))
		return
	}
	previous := p.Account.Chars[slot]
	p.Account.Chars[slot] = model.Char{}
	if err := w.saveAccount(p.Account); err != nil {
		p.Account.Chars[slot] = previous
		log.Printf("[#%d] ERRO ao excluir personagem %q: %v", s.ID, name, err)
		s.Send(wire.MessagePanel("The deletion could not be saved."))
		return
	}
	// PostgreSQL ja remove por ON DELETE CASCADE; o adaptador JSON recebe a
	// limpeza explicita. Falha aqui nao desfaz a exclusao ja confirmada: o
	// sidecar e derivado e o UID jamais sera reutilizado por outro personagem.
	if stateStore, ok := w.store.(charStateStore); ok && previous.UID != "" {
		if err := stateStore.SaveCharState(previous.UID, nil); err != nil {
			log.Printf("[#%d] limpar charstate do personagem excluido %q: %v",
				s.ID, name, err)
		}
	}
	// O nome so volta a ficar livre quando NENHUM personagem o usa mais. O slot
	// ja foi zerado acima, entao accountUsesName so enxerga os que sobraram.
	if w.charNames != nil && !accountUsesName(p.Account, previous.Name) {
		delete(w.charNames, strings.ToLower(previous.Name))
	}
	s.Send(wire.CNFDeleteCharacter(uint16(s.ID), p.Account.Chars))
	log.Printf("[#%d] personagem excluido: %q slot=%d", s.ID, name, slot)
}

// accountUsesName diz se a conta ainda tem algum personagem com esse nome.
//
// Existe por causa do Arch: ele herda o nome do Mortal (fiel ao nativo), entao
// UM nome pode pertencer a DOIS personagens. E sao os unicos homonimos
// possiveis no servidor -- a criacao normal (0x20F) exige nome globalmente
// unico, e a ascensao e o unico caminho que contorna isso.
//
// Sem esta checagem, apagar um dos gemeos removia o nome de charNames enquanto
// o outro continuava existindo, e o nome ficava livre para OUTRA conta criar um
// personagem homonimo -- furando a unicidade global.
func accountUsesName(acc *model.Account, name string) bool {
	if acc == nil || name == "" {
		return false
	}
	for i := range acc.Chars {
		if strings.EqualFold(acc.Chars[i].Name, name) {
			return true
		}
	}
	return false
}

// onREQMobByID recupera uma entidade que o client ainda nao materializou mas
// que foi referenciada por Action. So respondemos para entidades no raio de
// visibilidade; assim o pacote nao vira uma consulta global do mapa.
func (w *World) onREQMobByID(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || len(pkt) != 16 {
		return
	}
	id := binary.LittleEndian.Uint16(pkt[12:14])
	if id == 0 || id == p.ID {
		return
	}
	if m := w.mobByID(id); m != nil && w.mobVisibleToPlayer(p, m) &&
		inView(p.X, p.Y, m.X, m.Y) {
		wasVisible := p.hasVisible(id)
		w.showMob(p, m)
		if wasVisible { // pode ser uma recuperacao apos perda local do client.
			ancient := m.Def.Equip.AncientCodes()
			p.Session.Send(wire.CreateMobVisualExtended(m.ID, m.Def.Name, m.X, m.Y,
				m.Def.Mesh(), ancient[:], mobPublicExtended(m), m.Affects[:], 0))
		}
		return
	}
	if target := w.playerByID(id); target != nil && target.InWorld && target.Char != nil &&
		w.playersShareGameplaySpace(p, target) && inView(p.X, p.Y, target.X, target.Y) {
		sendPlayerEnterView(p, target)
		p.show(id)
		return
	}
	if shop := w.ghostShops[id]; shop != nil && inView(p.X, p.Y, shop.X, shop.Y) {
		p.Session.Send(wire.CreateMobTradeExtended(shop.ID, shop.Name, shop.X, shop.Y,
			shop.Mesh[:], &shop.Extended, shop.Title))
		p.show(id)
	}
}

// onMotion e 0x2BC sao emitidos pelo client 7.48 para efeitos/telemetria que
// nao alteram estado autoritativo do emulador. Reconhece-los evita falso
// "sem handler" sem inventar uma semantica que o server ainda nao usa.
func (w *World) onMotion(s *net.Session, pkt []byte) {
	if len(pkt) != 20 {
		w.noticeProtocol(s, wire.OpMotion, "tamanho inesperado")
	}
}

func (w *World) onClientUnknown2BC(s *net.Session, pkt []byte) {
	if len(pkt) != 108 {
		w.noticeProtocol(s, wire.OpClientUnknown2BC, "tamanho inesperado")
	}
}

func (w *World) noticeProtocol(s *net.Session, opcode uint16, detail string) {
	if s == nil {
		return
	}
	key := uint16(s.ID)<<4 ^ opcode
	now := time.Now()
	if last := w.lastProtocolNotice[key]; !last.IsZero() && now.Sub(last) < time.Minute {
		return
	}
	w.lastProtocolNotice[key] = now
	log.Printf("[#%d] protocolo 0x%X ignorado: %s", s.ID, opcode, detail)
}
