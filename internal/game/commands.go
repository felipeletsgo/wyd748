package game

import (
	"bytes"
	"fmt"
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const commandClearInventory = "/limparinv"
const magicTrumpet = uint16(3330)

func parseChatText(pkt []byte) (string, bool) {
	if len(pkt) < 13 || len(pkt) > 140 {
		return "", false
	}
	body := pkt[12:]
	if end := bytes.IndexByte(body, 0); end >= 0 {
		body = body[:end]
	}
	return strings.TrimSpace(string(body)), true
}

// O client interpreta "/nome texto" como MSG_MessageWhisper. Assim,
// "/limparinv" chega com MobName="limparinv" e String vazio, nao como chat.
func parseWhisperTarget(pkt []byte) (string, bool) {
	if len(pkt) < 28 {
		return "", false
	}
	target := pkt[12:28]
	if end := bytes.IndexByte(target, 0); end >= 0 {
		target = target[:end]
	}
	return normalizeWhisperTarget(string(target)), true
}

func normalizeWhisperTarget(target string) string {
	target = strings.TrimSpace(strings.TrimPrefix(strings.TrimSpace(target), "/"))
	if len(target) >= 2 && target[0] == '"' && target[len(target)-1] == '"' {
		target = strings.TrimSpace(target[1 : len(target)-1])
	}
	return target
}

func parseWhisperText(pkt []byte) (string, bool) {
	// O 7.48 usa 0x334 com 128 bytes; o texto termina antes do Color@124.
	if len(pkt) < 124 {
		return "", false
	}
	body := pkt[28:124]
	if end := bytes.IndexByte(body, 0); end >= 0 {
		body = body[:end]
	}
	return strings.TrimSpace(string(body)), true
}

// O 7.48 possui dois caminhos para o mesmo comando visual. Dependendo do foco
// da caixa de chat, /nick texto pode chegar como p334 separado ou como String
// de p333. Aceitamos tambem a sintaxe nativa /"nick" texto.
func parseSlashWhisperCommand(message string) (target, body string, ok bool) {
	message = strings.TrimSpace(message)
	if len(message) < 2 || message[0] != '/' {
		return "", "", false
	}
	rest := strings.TrimSpace(message[1:])
	if rest == "" {
		return "", "", false
	}
	if rest[0] == '"' {
		end := strings.IndexByte(rest[1:], '"')
		if end < 0 {
			return "", "", false
		}
		target = strings.TrimSpace(rest[1 : end+1])
		body = strings.TrimSpace(rest[end+2:])
	} else {
		cut := strings.IndexAny(rest, " \t")
		if cut < 0 {
			return "", "", false
		}
		target = strings.TrimSpace(rest[:cut])
		body = strings.TrimSpace(rest[cut:])
	}
	return target, body, target != "" && body != ""
}

// clearInventory zera todo o array estrutural, inclusive o indice 63 que nao
// possui celula na UI 7.48. Retorna quantos itens/pilhas foram removidos.
func clearInventory(ch *model.Char) int {
	if ch == nil {
		return 0
	}
	removed := 0
	for i := range ch.Inv {
		if ch.Inv[i].Index != 0 {
			removed++
		}
		ch.Inv[i] = model.Item{}
	}
	return removed
}

func (w *World) executeClearInventory(s *net.Session, p *Player) {
	if p.GhostShop != nil {
		s.Send(wire.MessagePanel("Close Auto Trade before clearing your inventory."))
		return
	}
	oldInv := p.Char.Inv
	removed := clearInventory(p.Char)
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv = oldInv
		log.Printf("[#%d] ERRO /limparinv conta=%q: %v", s.ID, p.Account.Name, err)
		s.Send(wire.MessagePanel("Save failed. The inventory was not changed."))
		return
	}
	s.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
	s.Send(wire.MessagePanel("Inventory cleared."))
	log.Printf("[#%d] /limparinv removeu %d item(ns) da conta %q",
		s.ID, removed, p.Account.Name)
}

func (w *World) onMessageChat(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld {
		return
	}
	message, ok := parseChatText(pkt)
	if !ok || message == "" {
		return
	}

	// Comando primeiro: dependendo do foco da caixa de chat, o MESMO comando
	// chega como 0x333 (aqui) ou como 0x334. Os dois caminhos usam o dispatcher.
	if name, arg, ok := parseSlashCommand(message); ok {
		if w.dispatchChatCommand(s, p, name, arg) {
			return
		}
	}
	if target, body, ok := parseSlashWhisperCommand(message); ok {
		w.deliverWhisper(s, p, target, body)
		return
	}
	// Chat local: reenviar como 0x333 COM O ID DO EMISSOR.
	//
	// O balao na cabeca vem de TMHuman::OnPacketMessageChat -- um handler da
	// ENTIDADE, nao da cena: o client acha o humano por Header.ID e chama
	// SetChatMessage. Mandar 0x334, ou 0x333 com id=0, faz a linha aparecer no
	// maximo na lista de chat e nunca desenha o balao.
	// (TMFieldScene::OnPacketMessageChat e so o fallback para quem esta fora da
	// visao, e por isso exige lista de grupo.)
	//
	// O emissor desenha o proprio balao localmente antes de enviar, entao
	// reenviar para ele duplicaria a linha.
	for _, observer := range w.players {
		if observer == p || !observer.InWorld || !observer.hasVisible(p.ID) {
			continue
		}
		observer.Session.Send(wire.MessageChat(p.ID, message))
	}
	log.Printf("[#%d] CHAT local %q (%d observador(es))", s.ID, p.Char.Name, w.localChatObservers(p))
}

// localChatObservers conta quem enxerga o emissor. Serve so ao log: com zero
// observadores o emissor ve o proprio balao e acha que funcionou, quando na
// verdade estava sozinho na area de visibilidade.
func (w *World) localChatObservers(sender *Player) int {
	count := 0
	for _, observer := range w.players {
		if observer == sender || !observer.InWorld || !observer.hasVisible(sender.ID) {
			continue
		}
		count++
	}
	return count
}

func (w *World) onMessageWhisper(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld {
		return
	}
	target, okTarget := parseWhisperTarget(pkt)
	message, okText := parseWhisperText(pkt)
	if !okTarget || !okText {
		return
	}
	// O comando vem em MobName ("/criar Alfa" chega como target="criar",
	// message="Alfa"). Precisa rodar ANTES do corte por mensagem vazia: comandos
	// como /aceitar e /sair chegam sem corpo nenhum.
	if w.dispatchChatCommand(s, p, target, message) {
		return
	}
	// "/nick" SEM mensagem: o client manda MobName preenchido e String vazia
	// (TMFieldScene so copia o texto quando ha algo depois do nick). Isso era
	// descartado; agora vira consulta de informacoes do personagem.
	if message == "" {
		w.sendCharacterInfo(s, p, target)
		return
	}
	// Prefixos confirmados no TMFieldScene 7.48. O caractere inicial e parte do
	// protocolo visual e precisa ser preservado no reenvio: o client usa ele
	// para escolher a cor e o offset de corte do texto.
	switch chatChannelOf(message) {
	case chatChannelParty:
		w.sendPartyChat(p, strings.TrimSpace(message[1:]))
	case chatChannelGlobal:
		for _, observer := range w.players {
			if observer == p || !observer.InWorld {
				continue // o client do emissor ja inseriu a propria mensagem.
			}
			observer.Session.Send(wire.MessageWhisper(0, p.Char.Name, message, 3))
		}
	case chatChannelGuild:
		w.sendGuildChat(p, message)
	default:
		w.deliverWhisper(s, p, target, message)
	}
}

// Canais selecionados por prefixo no 0x334.
const (
	chatChannelParty   = "party"
	chatChannelGlobal  = "global"
	chatChannelGuild   = "guild"
	chatChannelWhisper = "whisper"
)

// chatChannelOf classifica a mensagem pelo prefixo digitado.
//
// A ORDEM E LOAD-BEARING: '--' (global, decisao deste projeto) precisa ser
// testado ANTES de '-' (guild), senao todo global cai na rota de guild. O
// client renderiza os dois pelo mesmo ramo -- e o servidor que os separa.
func chatChannelOf(message string) string {
	switch {
	case strings.HasPrefix(message, "="):
		return chatChannelParty
	case strings.HasPrefix(message, "--"):
		return chatChannelGlobal
	case strings.HasPrefix(message, "-"):
		return chatChannelGuild
	default:
		return chatChannelWhisper
	}
}

// parseSlashCommand separa "/nome argumento" aceitando argumento VAZIO. O
// parseSlashWhisperCommand exige corpo (e um whisper), entao nao serve para
// comandos como /aceitar e /sair.
func parseSlashCommand(message string) (name, arg string, ok bool) {
	message = strings.TrimSpace(message)
	if len(message) < 2 || message[0] != '/' {
		return "", "", false
	}
	rest := strings.TrimSpace(message[1:])
	if rest == "" {
		return "", "", false
	}
	if cut := strings.IndexAny(rest, " \t"); cut >= 0 {
		return rest[:cut], strings.TrimSpace(rest[cut:]), true
	}
	return rest, "", true
}

// dispatchChatCommand e o ponto unico de comandos, chamado pelo 0x333 e pelo
// 0x334. Devolve true quando consumiu a mensagem. Um comando sempre tem
// precedencia sobre um nick de mesmo nome, como no TMSrv nativo.
func (w *World) dispatchChatCommand(s *net.Session, p *Player, name, arg string) bool {
	switch strings.ToLower(strings.TrimSpace(name)) {
	case "time", "day":
		// Tecla Insert do client 7.48: TMFieldScene manda um whisper com
		// MobName="time" (algumas builds tambem "day"). Sem interceptar aqui, o
		// whisper cai no lookup de personagem (sendCharacterInfo) e devolve
		// "nao esta conectado" -- exatamente o erro "time/day nao conectado".
		// Espelha o SendClientMessage do W2PP (_MSG_MessagePanel) com a data/hora
		// do host, no mesmo formato "%H:%M:%S | %d-%m-%Y".
		s.Send(wire.MessagePanel(time.Now().Format("15:04:05 | 02-01-2006")))
	case "limparinv", "clearinv":
		w.executeClearInventory(s, p)
	case "hpdebug":
		w.dumpHPProjection(s, p)
	case "spk":
		w.executeShout(s, p, arg)
	case "kingdom", "reino":
		w.kingdomCommandTeleport(s, p, false)
	case "king", "rei":
		w.kingdomCommandTeleport(s, p, true)
	case "criar", "create":
		w.guildCommandCreate(s, p, arg)
	case "convidar", "invite":
		w.guildCommandInvite(s, p, arg)
	case "aceitar", "accept":
		w.guildCommandAccept(s, p, arg)
	case "sair", "leave":
		w.guildCommandLeave(s, p, arg)
	case "expulsar", "expel":
		w.guildCommandExpel(s, p, arg)
	case "criarsub", "subcreate", "createsub":
		w.guildCommandSubLeader(s, p, arg)
	default:
		return false
	}
	return true
}

// sendCharacterInfo responde ao "/nick" sem mensagem com um resumo do
// personagem. As linhas vao pelo canal de whisper para ficarem na lista de
// chat, em vez do painel flutuante, que some.
//
// So funciona para quem esta CONECTADO: nao existe indice personagem->conta,
// entao consultar alguem offline exigiria varrer todos os JSONs de conta.
func (w *World) sendCharacterInfo(s *net.Session, p *Player, target string) {
	if target == "" {
		return
	}
	found := w.playerByCharacterName(target)
	if found == nil || found.Char == nil {
		s.Send(wire.MessagePanel(fmt.Sprintf("%s is not online.", target)))
		return
	}
	ch := found.Char
	// Tudo numa UNICA mensagem de aviso (0x101). O 0x324 nativo do W2PP (janela
	// de inspecionar) foi testado in-game e o client 7.48 nao abre nada -- e
	// adicao tardia do 759. Varias linhas tambem nao servem: o painel de aviso
	// mostra uma de cada vez e as anteriores somem.
	s.Send(wire.MessagePanel(w.characterInfoLine(ch)))
	log.Printf("[#%d] INFO %q consultou %q", s.ID, p.Char.Name, ch.Name)
}

// displayLevel converte o nivel interno (base zero) no numero que o jogador ve.
// O ConsultaInfoPlayer nativo faz o mesmo +1.
func displayLevel(ch *model.Char) uint32 {
	if ch == nil || ch.Extended == nil {
		return 1
	}
	return ch.Extended.Level + 1
}

// characterInfoLine monta o resumo em UMA linha, no formato que cabe no painel
// de aviso. Fica separado do envio para ser testavel sem sessao de rede.
func (w *World) characterInfoLine(ch *model.Char) string {
	parts := []string{fmt.Sprintf("%s  Lv.%d", ch.Name, displayLevel(ch))}

	if guild, member := w.guildOf(ch); guild != nil {
		role := "Member"
		switch {
		case member.Rank == model.GuildRankLeader:
			role = "Leader"
		case model.IsSubLeader(member.Rank):
			role = "Sub-leader"
		}
		guildPart := fmt.Sprintf("Guild: %s (%s)", guild.Name, role)
		if ally := w.guilds.FindByID(guild.Ally); ally != nil {
			guildPart += fmt.Sprintf(" aliada de %s", ally.Name)
		}
		parts = append(parts, guildPart)
	} else {
		parts = append(parts, "sem guild")
	}

	// A cidadania e concedida pelo NPC Kibita; enquanto o sistema nao existir,
	// characterCitizenship devolve vazio e a parte simplesmente nao aparece.
	if citizenship := characterCitizenship(ch); citizenship != "" {
		parts = append(parts, "Cidadania: "+citizenship)
	}
	if kingdom := characterKingdom(ch); kingdom != model.KingdomNeutral {
		parts = append(parts, "Reino: "+model.KingdomName(kingdom))
	}
	return strings.Join(parts, " | ")
}

// characterCitizenship descreve a cidadania. O valor guardado e o NUMERO DO
// CANAL (cidadania 1 = canal 1), como no nativo.
func characterCitizenship(ch *model.Char) string {
	if ch == nil || ch.Citizenship == 0 {
		return ""
	}
	return fmt.Sprintf("Canal %d", ch.Citizenship)
}

func (w *World) deliverWhisper(s *net.Session, p *Player, target, message string) {
	if s == nil || p == nil || p.Char == nil || target == "" || message == "" {
		return
	}
	recipient := w.playerByCharacterName(target)
	if recipient == nil {
		s.Send(wire.MessagePanel("That player is not online."))
		log.Printf("[#%d] WHISPER %q -> %q: desconectado", s.ID, p.Char.Name, target)
		return
	}
	if strings.HasPrefix(message, "!") {
		// O '!' nao e um texto decorativo: o handler 0x334 do client grava a
		// mensagem no painel H. A carta de morte usa exatamente esse mesmo canal.
		recipient.Session.Send(wire.MessageWhisper(0, p.Char.Name, message, 0))
		log.Printf("[#%d] CARTA %q -> %q", s.ID, p.Char.Name, recipient.Char.Name)
		return
	}
	// Replica o TMSrv/W2PP: /nick mensagem chega como 0x334, o servidor troca
	// MobName pelo remetente, preserva String sem prefixos e usa o ID dele.
	recipient.Session.Send(wire.MessageWhisper(p.ID, p.Char.Name, message, 0))
	log.Printf("[#%d] WHISPER %q -> %q", s.ID, p.Char.Name, recipient.Char.Name)
}

// playerByCharacterName acha o jogador ONLINE pelo nome do personagem.
//
// O Arch herda o nome do Mortal, entao existem homonimos -- mas a busca nao
// precisa de desempate, e isso se apoia em dois invariantes:
//
//  1. homonimos so nascem da ascensao, logo estao sempre na MESMA conta (a
//     criacao normal exige nome globalmente unico);
//  2. uma conta nao entra duas vezes ao mesmo tempo (claimAccountSession).
//
// Juntos, garantem no maximo UM homonimo online. Se algum dos dois cair, esta
// busca passa a ser ambigua em silencio --
// TestAccountSessionIsExclusiveCaseInsensitive guarda o segundo.
func (w *World) playerByCharacterName(name string) *Player {
	for _, p := range w.players {
		if p.InWorld && p.Char != nil && strings.EqualFold(p.Char.Name, name) {
			return p
		}
	}
	return nil
}

func (w *World) sendPartyChat(sender *Player, message string) {
	if sender == nil || sender.Char == nil || sender.Party == nil || message == "" {
		return
	}
	for _, member := range sender.Party.Members {
		if member == nil || !member.InWorld || member.Char == nil || member == sender {
			continue // o TMFieldScene ja insere o texto localmente antes do envio.
		}
		member.Session.Send(wire.MessageWhisper(0, sender.Char.Name, "="+message, 1))
	}
}

// executeShout consome uma unidade de Shout (item 3330) do inventario antes
// de anunciar. A persistencia vem antes do broadcast: se o disco falhar, nem
// o item nem a mensagem sao confirmados, evitando anuncio gratuito por erro.
func (w *World) executeShout(s *net.Session, p *Player, message string) {
	if message == "" {
		s.Send(wire.MessagePanel("Usage: /spk message"))
		return
	}
	if len(message) > 120 {
		message = message[:120]
	}
	slot := -1
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if p.Char.Inv[i].Index == magicTrumpet {
			slot = i
			break
		}
	}
	if slot < 0 {
		s.Send(wire.MessagePanel("You need a Shout to announce."))
		return
	}
	previous := p.Char.Inv[slot]
	p.Char.Inv[slot] = model.Item{}
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[slot] = previous
		log.Printf("[#%d] ERRO /spk conta=%q: %v", s.ID, p.Account.Name, err)
		s.Send(wire.MessagePanel("Failed to consume the Shout. No announcement was sent."))
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, byte(slot), p.Char.Inv[slot]))
	w.broadcast(func() []byte { return wire.MessageWhisper(0, "[SERVER]", message, 7) })
	log.Printf("[#%d] /spk %q", s.ID, message)
}
