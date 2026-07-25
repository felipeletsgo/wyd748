package game

import (
	"fmt"
	"log"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// cargoMerchantType e o nibble baixo de Merchant dos NPCs de armazem.
const cargoMerchantType = 2

// NOTA DE DESENHO -- por que nao existe um "PendingQuest" server-side aqui.
//
// O plano original exigia um estado pendente entre oferta e confirmacao, para
// que um ClickOk=1 solto nao concedesse recompensa. A captura do client 7.48
// mostrou que esse desenho e impossivel: quando o NPC tem o gatilho de
// confirmacao (tipo 11 + rosto 58, ou tipos 10/13/14/15), o primeiro clique
// NAO gera pacote -- o client apenas abre a janela local. So o "Sim" envia, ja
// com ClickOk=1. Nao existe pacote de oferta para criar o estado pendente.
//
// Portanto o servidor recebe exatamente UM pacote por interacao, e a defesa
// real e a validacao server-side em executeQuest: requisitos, flag de
// concluida, alcance e visibilidade. O pacote do client so carrega o id do NPC;
// custo, item e recompensa vem sempre do quests.json.

// Tipos de NPC que ja possuem handler proprio. Uma quest configurada em cima de
// um deles nunca dispararia, porque o roteamento testa os tipos conhecidos
// antes -- por isso o boot recusa essa configuracao em vez de ignora-la.
const (
	npcKindShop  = "loja/mestre"
	npcKindCargo = "cargo"
	npcKindCraft = "compositor"
)

// reservedNPCKind devolve o handler dedicado do NPC, se houver. Centraliza o
// vocabulario de tipos para que o roteamento e a validacao de boot nao possam
// divergir.
func reservedNPCKind(def *model.NPCDef) (string, bool) {
	if def == nil || def.Extended == nil {
		return "", false
	}
	if _, isShop := shopTypeForMerchant(def.Extended.Merchant); isShop {
		return npcKindShop, true
	}
	if def.Extended.Merchant&0x0F == craftingMerchant {
		return npcKindCraft, true
	}
	if def.Extended.Merchant&0xF == cargoMerchantType {
		return npcKindCargo, true
	}
	return "", false
}

// indexQuests casa cada quest com o NPC correspondente e recusa configuracoes
// que nunca funcionariam. Roda no boot: erro aqui derruba o servidor.
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
			return nil, fmt.Errorf("quest %d referencia o npc %q, que nao existe em data/npcs",
				quest.ID, quest.NPC)
		}
		if kind, reserved := reservedNPCKind(def); reserved {
			return nil, fmt.Errorf(
				"quest %d configurada no npc %q, que ja e %s: o clique nunca chegaria na quest",
				quest.ID, quest.NPC, kind)
		}
		out[key] = quest
	}
	return out, nil
}

// questForNPC devolve a quest configurada para o NPC, se houver. A allowlist e
// o proprio quests.json: NPC ausente dali nunca vira quest.
func (w *World) questForNPC(def *model.NPCDef) *model.QuestDef {
	if def == nil || len(w.questsByNPC) == 0 {
		return nil
	}
	if quest, ok := w.questsByNPC[model.NormalizeQuestNPC(def.Name)]; ok {
		return quest
	}
	return w.questsByNPC[model.NormalizeQuestNPC(generName(def.Name))]
}

// onQuestInteraction trata o clique num NPC de quest.
//
// A CONFIRMACAO E CLIENT-SIDE, nao server-side. O client 7.48 decide sozinho se
// abre a janela de "tem certeza?", olhando o tipo do NPC e o mesh de rosto
// (TMFieldScene: tipo 11 + rosto 58, alem dos tipos 10/13/14/15). Consequencia
// medida em captura:
//
//   - NPC sem esse gatilho (ex.: Kibita, rosto 16): o clique manda ClickOk=0.
//   - NPC com o gatilho (ex.: Coveiro, rosto 58): o primeiro clique NAO manda
//     pacote nenhum -- so abre a janela. O "Sim" manda ClickOk=1.
//
// Nos dois casos o servidor recebe UM unico pacote por interacao. Por isso nao
// da para exigir um pacote de "oferta" antes de conceder: as quests com
// confirmacao nativa nunca mandariam esse pacote e jamais poderiam concluir.
// A defesa real e a validacao server-side (requisitos, alcance, visibilidade e
// flag de concluida), que roda integralmente em executeQuest.
func (w *World) onQuestInteraction(s *net.Session, p *Player, m *Mob, quest *model.QuestDef, clickOk int32) {
	if clickOk == 1 {
		log.Printf("[#%d] QUEST %d confirmada na janela nativa do client", s.ID, quest.ID)
	}
	w.executeQuest(s, p, m, quest)
}

// questBlockedMessage devolve o motivo da recusa, preferindo o texto do autor
// da quest quando ele definiu um.
func questBlockedMessage(quest *model.QuestDef, reason string) string {
	if quest.Messages.Denied != "" {
		return quest.Messages.Denied
	}
	return reason
}

// questRequirementsMet valida level, gold, itens e quest anterior. Devolve o
// motivo da recusa para o painel.
func (w *World) questRequirementsMet(p *Player, quest *model.QuestDef) (string, bool) {
	ch := p.Char
	level := uint32(0)
	if ch.Extended != nil {
		level = ch.Extended.Level
	}
	req := quest.Requires
	if req.MortalOnly && strings.TrimSpace(ch.Evolution) != "" {
		return "Available only to Mortal characters.", false
	}
	if req.MinLevel != 0 && level < req.MinLevel {
		return fmt.Sprintf("You must be level %d.", req.MinLevel), false
	}
	if req.MaxLevel != 0 && level > req.MaxLevel {
		return fmt.Sprintf("Disponivel apenas ate o nivel %d.", req.MaxLevel), false
	}
	if req.AfterQuest != 0 && !questCompleted(ch, req.AfterQuest) {
		return "You cannot take this quest yet.", false
	}
	if req.Gold != 0 && ch.Gold < req.Gold {
		return "Not enough gold.", false
	}
	// O Kibita nativo exige Citizen == 0: nao da para comprar cidadania duas
	// vezes, nem trocar a de outro canal pela deste.
	if quest.Rewards.Citizenship && ch.Citizenship != 0 {
		return "You already have citizenship.", false
	}
	for _, item := range req.Items {
		if countInventoryItem(ch, item.Index) < item.Quantity() {
			return "You do not have the required items.", false
		}
	}
	// O consumo tambem precisa estar disponivel: exigir na oferta evita
	// prometer uma recompensa que a confirmacao nao conseguiria pagar.
	for _, item := range quest.Consumes {
		if countInventoryItem(ch, item.Index) < item.Quantity() {
			return "You do not have the required items.", false
		}
	}
	if free := freeInventorySlots(ch); free < len(quest.Rewards.Items) {
		return "Your inventory is full.", false
	}
	return "", true
}

// countInventoryItem soma as unidades de um item nos slots visiveis, honrando
// as pilhas de EF_AMOUNT.
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

// itemStackAmount devolve o tamanho da pilha (EF_AMOUNT) ou 1.
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

// executeQuest executa a quest de ponta a ponta. Revalida tudo (o estado pode
// ter mudado desde que o jogador abriu a janela), consome, concede e persiste;
// se a gravacao falhar, reverte por completo.

func (w *World) executeQuest(s *net.Session, p *Player, m *Mob, quest *model.QuestDef) {
	// Toda recusa e logada com o motivo. Sem isso o log mostra a confirmacao
	// chegando e depois silencio, e nao da para distinguir "ja concluida" de
	// uma falha de verdade -- exatamente o que confundiu no primeiro teste.
	if !quest.Repeatable && questCompleted(p.Char, quest.ID) {
		s.Send(wire.MessagePanel("You have already completed this quest."))
		log.Printf("[#%d] QUEST %d recusada: ja concluida", s.ID, quest.ID)
		return
	}
	// Toda a defesa esta aqui: o pacote do client nunca carrega custo, item ou
	// recompensa -- so o id do NPC. Alcance e visibilidade ja foram validados
	// em resolveNPCInteraction antes de chegar neste ponto.
	if reason, ok := w.questRequirementsMet(p, quest); !ok {
		s.Send(wire.MessagePanel(questBlockedMessage(quest, reason)))
		log.Printf("[#%d] QUEST %d recusada: %s", s.ID, quest.ID, reason)
		return
	}

	// Snapshot para rollback: a persistencia vem antes da confirmacao ao client.
	previousInv := p.Char.Inv
	previousGold := p.Char.Gold
	previousExp := p.Char.Exp
	previousDone := append([]int32(nil), p.Char.QuestsDone...)
	previousX, previousY := p.X, p.Y
	previousCitizenship := p.Char.Citizenship

	for _, item := range quest.Consumes {
		if !consumeInventoryItem(p.Char, item.Index, item.Quantity()) {
			p.Char.Inv = previousInv
			s.Send(wire.MessagePanel("You do not have the required items."))
			log.Printf("[#%d] QUEST %d recusada: consumo do item %d x%d falhou",
				s.ID, quest.ID, item.Index, item.Quantity())
			return
		}
	}
	if quest.Requires.Gold != 0 {
		p.Char.Gold -= quest.Requires.Gold
	}
	for _, item := range quest.Rewards.Items {
		if !grantInventoryItem(p.Char, item) {
			p.Char.Inv, p.Char.Gold = previousInv, previousGold
			s.Send(wire.MessagePanel("Your inventory is full."))
			log.Printf("[#%d] QUEST %d recusada: sem espaco para o item %d",
				s.ID, quest.ID, item.Index)
			return
		}
	}
	if quest.Rewards.Gold != 0 {
		// Mesmo teto das barras de gold: recompensa nao pode estourar o limite.
		if p.Char.Gold > maxCharacterGold-quest.Rewards.Gold {
			p.Char.Gold = maxCharacterGold
		} else {
			p.Char.Gold += quest.Rewards.Gold
		}
	}
	if quest.Rewards.Exp != 0 {
		// O bau de experiencia (affect 39) tambem dobra a EXP de quest.
		grantExp(p.Char, expWithDoubleBuff(p.Char, quest.Rewards.Exp))
	}
	if quest.Rewards.Citizenship {
		// A cidadania guarda o NUMERO DO CANAL, como no nativo.
		p.Char.Citizenship = w.channel
		// O lider carrega a guild junto: no Kibita nativo, se quem compra e o
		// mestre, a guild inteira vira cidada.
		if guild, member := w.guildOf(p.Char); guild != nil &&
			member.Rank == model.GuildRankLeader {
			log.Printf("[#%d] CIDADANIA guild %d (%q) tornou-se cidada do canal %d",
				s.ID, guild.ID, guild.Name, w.channel)
		}
	}
	if !quest.Repeatable {
		markQuestCompleted(p.Char, quest.ID)
	}

	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv = previousInv
		p.Char.Gold, p.Char.Exp = previousGold, previousExp
		p.Char.QuestsDone = previousDone
		p.X, p.Y = previousX, previousY
		p.Char.Citizenship = previousCitizenship
		s.Send(wire.MessagePanel("Save failed. The quest was not completed."))
		log.Printf("[#%d] ERRO quest %d: %v", s.ID, quest.ID, err)
		return
	}

	w.recalcPlayer(p.Char)
	s.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	s.Send(wire.UpdateScore(p.ID, *p.Char))
	if t := quest.Rewards.Teleport; t != nil {
		// O teleporte ja re-materializa o avatar (com a tintura) no destino.
		w.teleportPlayer(p, t.X, t.Y)
	}
	s.Send(wire.MessagePanel(quest.Messages.Success))
	log.Printf("[#%d] QUEST %d concluida por %q", s.ID, quest.ID, p.Char.Name)
}

// consumeInventoryItem retira `amount` unidades, respeitando pilhas.
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

// grantInventoryItem poe o item no primeiro slot livre visivel.
func grantInventoryItem(ch *model.Char, reward model.QuestItem) bool {
	item := model.Item{Index: reward.Index}
	if amount := reward.Quantity(); amount > 1 {
		item.Eff[0] = effectAmount
		if amount > 255 {
			amount = 255
		}
		item.Eff[1] = byte(amount)
	}
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if ch.Inv[i].Index == 0 {
			ch.Inv[i] = item
			return true
		}
	}
	return false
}

// questCompleted consulta a lista de quests concluidas do personagem.
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
