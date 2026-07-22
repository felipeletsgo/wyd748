package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// guildStore expoe a persistencia transacional de guild+contas. Segue o mesmo
// padrao do tradeBatchStore: o game conhece a interface minima e faz assercao,
// sem obrigar todo Store a implementar guild.
type guildStore interface {
	LoadGuilds() (*model.GuildRegistry, error)
	SaveGameState(guilds *model.GuildRegistry, accounts ...*model.Account) error
}

// guildInviteTTL espelha a validade do convite de grupo: um convite esquecido
// nao pode ficar valido para sempre.
const guildInviteTTL = 30 * time.Second

// saveGuildState grava o registro de guild e as contas afetadas numa unica
// transacao. Sem store transacional o comando e recusado, em vez de gravar
// metade do estado: guild sem membro (ou membro sem guild) e pior que falhar.
func (w *World) saveGuildState(accounts ...*model.Account) error {
	gs, ok := w.store.(guildStore)
	if !ok {
		return fmt.Errorf("store atual nao suporta persistencia de guild")
	}
	for _, account := range accounts {
		pinAccountEntryPositions(account)
	}
	return gs.SaveGameState(w.guilds, accounts...)
}

// Erros de autorizacao. Ficam separados da mensagem enviada ao client para que
// a regra possa ser testada sem uma sessao de rede.
var (
	errGuildMissing   = errors.New("personagem nao pertence a uma guild")
	errGuildNotLeader = errors.New("acao permitida somente ao lider")
)

// requireGuildLeader resolve a guild do personagem e exige lideranca. E a regra
// unica usada por /convidar e /expulsar; o comando so traduz o erro em painel.
func (w *World) requireGuildLeader(ch *model.Char) (*model.Guild, error) {
	guild, member := w.guildOf(ch)
	if guild == nil {
		return nil, errGuildMissing
	}
	if member.Rank != model.GuildRankLeader {
		return nil, errGuildNotLeader
	}
	return guild, nil
}

// guildAuthMessage traduz o erro de autorizacao no texto do painel.
func guildAuthMessage(err error, action string) string {
	if errors.Is(err, errGuildNotLeader) {
		return fmt.Sprintf("Somente o lider pode %s.", action)
	}
	return "Voce nao pertence a uma guild."
}

// guildOf devolve a guild do personagem pela lista canonica.
func (w *World) guildOf(ch *model.Char) (*model.Guild, *model.GuildMember) {
	if w.guilds == nil || ch == nil || ch.Name == "" {
		return nil, nil
	}
	return w.guilds.FindByCharacter(ch.Name)
}

// repairGuildState reconcilia os campos desnormalizados do personagem com o
// registro canonico. Roda no login: se a guild sumiu (dissolvida enquanto o
// jogador estava offline) ou o rank mudou, o personagem e corrigido aqui.
// Devolve true quando alterou algo, para o chamador decidir se persiste.
func (w *World) repairGuildState(ch *model.Char) bool {
	if ch == nil || ch.Name == "" {
		return false
	}
	guild, member := w.guildOf(ch)
	if guild == nil {
		if ch.GuildID == 0 && ch.GuildRank == 0 {
			return false
		}
		log.Printf("guild: %q apontava para guild %d inexistente; estado limpo",
			ch.Name, ch.GuildID)
		ch.GuildID, ch.GuildRank = 0, 0
		return true
	}
	if ch.GuildID == guild.ID && ch.GuildRank == member.Rank {
		return false
	}
	log.Printf("guild: %q reparado para guild=%d rank=%d (era guild=%d rank=%d)",
		ch.Name, guild.ID, member.Rank, ch.GuildID, ch.GuildRank)
	ch.GuildID, ch.GuildRank = guild.ID, member.Rank
	return true
}

// onlineGuildMembers devolve os players conectados da guild informada.
func (w *World) onlineGuildMembers(guildID uint16) []*Player {
	if guildID == 0 {
		return nil
	}
	var members []*Player
	for _, p := range w.players {
		if p.InWorld && p.Char != nil && p.Char.GuildID == guildID {
			members = append(members, p)
		}
	}
	return members
}

// sendGuildChat entrega o texto do canal '-' apenas aos membros online. O
// prefixo e preservado: o client usa o primeiro caractere para escolher cor e
// o offset de corte do texto.
func (w *World) sendGuildChat(sender *Player, message string) {
	if sender == nil || sender.Char == nil || message == "" {
		return
	}
	guild, _ := w.guildOf(sender.Char)
	if guild == nil {
		sender.Session.Send(wire.MessagePanel("Voce nao pertence a uma guild."))
		log.Printf("[#%d] chat de guild recusado: %q nao pertence a nenhuma guild",
			sender.Session.ID, sender.Char.Name)
		return
	}
	delivered := 0
	for _, member := range w.onlineGuildMembers(guild.ID) {
		if member == sender {
			continue // o client do emissor ja inseriu a linha localmente
		}
		member.Session.Send(wire.MessageWhisper(0, sender.Char.Name, message, 3))
		delivered++
	}
	// Logar quantos receberam: com 0 entregas o emissor ve a propria linha e
	// pensa que funcionou, quando na verdade estava sozinho online na guild.
	log.Printf("[#%d] GUILD chat %q -> guild %d (%d membro(s) online alem do emissor)",
		sender.Session.ID, sender.Char.Name, guild.ID, delivered)
}

// ---------------------------------------------------------------------------
// Comandos
// ---------------------------------------------------------------------------

func (w *World) guildCommandCreate(s *net.Session, p *Player, arg string) {
	name := strings.TrimSpace(arg)
	if name == "" {
		s.Send(wire.MessagePanel("Uso: /criar <nome da guild>"))
		return
	}
	if err := model.ValidateGuildName(name); err != nil {
		s.Send(wire.MessagePanel("Nome de guild invalido."))
		log.Printf("[#%d] /criar recusado: %v", s.ID, err)
		return
	}
	if guild, _ := w.guildOf(p.Char); guild != nil {
		s.Send(wire.MessagePanel("Voce ja pertence a uma guild."))
		return
	}
	if w.guilds.FindByName(name) != nil {
		s.Send(wire.MessagePanel("Ja existe uma guild com esse nome."))
		return
	}
	id, err := w.guilds.NextGuildID()
	if err != nil {
		s.Send(wire.MessagePanel("Limite de guilds atingido."))
		log.Printf("[#%d] /criar: %v", s.ID, err)
		return
	}

	previousGuilds := w.guilds.Guilds
	oldID, oldRank := p.Char.GuildID, p.Char.GuildRank
	w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{
		ID:        id,
		Name:      name,
		CreatedAt: time.Now().UTC(),
		Members: []model.GuildMember{{
			Character: p.Char.Name,
			Account:   p.Account.Name,
			Rank:      model.GuildRankLeader,
			JoinedAt:  time.Now().UTC(),
		}},
	})
	p.Char.GuildID, p.Char.GuildRank = id, model.GuildRankLeader

	if err := w.saveGuildState(p.Account); err != nil {
		w.guilds.Guilds = previousGuilds
		p.Char.GuildID, p.Char.GuildRank = oldID, oldRank
		s.Send(wire.MessagePanel("Falha ao salvar. A guild nao foi criada."))
		log.Printf("[#%d] ERRO /criar %q: %v", s.ID, name, err)
		return
	}
	s.Send(wire.MessagePanel(fmt.Sprintf("Guild %q criada.", name)))
	log.Printf("[#%d] GUILD criada id=%d %q por %q", s.ID, id, name, p.Char.Name)
}

func (w *World) guildCommandInvite(s *net.Session, p *Player, arg string) {
	target := strings.TrimSpace(arg)
	if target == "" {
		s.Send(wire.MessagePanel("Uso: /convidar <nick>"))
		return
	}
	guild, err := w.requireGuildLeader(p.Char)
	if err != nil {
		s.Send(wire.MessagePanel(guildAuthMessage(err, "convidar")))
		return
	}
	if strings.EqualFold(target, p.Char.Name) {
		s.Send(wire.MessagePanel("Voce ja esta na guild."))
		return
	}
	// Primeiro marco: sem indice personagem->conta, so da para mexer em quem
	// esta conectado. Convidar offline exigiria varrer todas as contas.
	invited := w.playerByCharacterName(target)
	if invited == nil || invited.Char == nil {
		s.Send(wire.MessagePanel("Jogador nao esta conectado."))
		return
	}
	if other, _ := w.guildOf(invited.Char); other != nil {
		s.Send(wire.MessagePanel("Esse jogador ja pertence a uma guild."))
		return
	}
	invited.GuildInviteFrom = guild.ID
	invited.GuildInviteUntil = time.Now().Add(guildInviteTTL)
	invited.Session.Send(wire.MessagePanel(fmt.Sprintf(
		"%s convidou voce para a guild %s. Use /aceitar.", p.Char.Name, guild.Name)))
	s.Send(wire.MessagePanel(fmt.Sprintf("Convite enviado para %s.", invited.Char.Name)))
	log.Printf("[#%d] GUILD convite %q -> %q (guild %d)", s.ID, p.Char.Name, invited.Char.Name, guild.ID)
}

func (w *World) guildCommandAccept(s *net.Session, p *Player, _ string) {
	if p.GuildInviteFrom == 0 || time.Now().After(p.GuildInviteUntil) {
		p.GuildInviteFrom = 0
		s.Send(wire.MessagePanel("Voce nao possui convite de guild valido."))
		return
	}
	if guild, _ := w.guildOf(p.Char); guild != nil {
		p.GuildInviteFrom = 0
		s.Send(wire.MessagePanel("Voce ja pertence a uma guild."))
		return
	}
	guild := w.guilds.FindByID(p.GuildInviteFrom)
	if guild == nil {
		p.GuildInviteFrom = 0
		s.Send(wire.MessagePanel("A guild nao existe mais."))
		return
	}

	previousMembers := append([]model.GuildMember(nil), guild.Members...)
	oldID, oldRank := p.Char.GuildID, p.Char.GuildRank
	guild.Members = append(guild.Members, model.GuildMember{
		Character: p.Char.Name,
		Account:   p.Account.Name,
		Rank:      model.GuildRankMember,
		JoinedAt:  time.Now().UTC(),
	})
	p.Char.GuildID, p.Char.GuildRank = guild.ID, model.GuildRankMember

	if err := w.saveGuildState(p.Account); err != nil {
		guild.Members = previousMembers
		p.Char.GuildID, p.Char.GuildRank = oldID, oldRank
		s.Send(wire.MessagePanel("Falha ao salvar. Voce nao entrou na guild."))
		log.Printf("[#%d] ERRO /aceitar: %v", s.ID, err)
		return
	}
	p.GuildInviteFrom = 0
	s.Send(wire.MessagePanel(fmt.Sprintf("Voce entrou na guild %s.", guild.Name)))
	w.announceToGuild(guild.ID, fmt.Sprintf("%s entrou na guild.", p.Char.Name), p)
	log.Printf("[#%d] GUILD %q entrou na guild %d", s.ID, p.Char.Name, guild.ID)
}

// removeGuildMember tira o personagem da lista canonica e resolve a sucessao.
// Devolve as contas que precisam ser persistidas junto do registro.
func (w *World) removeGuildMember(guild *model.Guild, character string) (dissolved bool, promoted string) {
	kept := guild.Members[:0:0]
	var removedRank byte
	for _, member := range guild.Members {
		if strings.EqualFold(member.Character, character) {
			removedRank = member.Rank
			continue
		}
		kept = append(kept, member)
	}
	guild.Members = kept
	if len(guild.Members) == 0 {
		// Ultimo membro saiu: dissolve em vez de deixar guild orfa.
		for i := range w.guilds.Guilds {
			if w.guilds.Guilds[i].ID == guild.ID {
				w.guilds.Guilds = append(w.guilds.Guilds[:i], w.guilds.Guilds[i+1:]...)
				break
			}
		}
		return true, ""
	}
	if removedRank == model.GuildRankLeader {
		// Sucessao pelo membro mais antigo, que e o primeiro da lista.
		guild.Members[0].Rank = model.GuildRankLeader
		return false, guild.Members[0].Character
	}
	return false, ""
}

// applyGuildMembership sincroniza o estado desnormalizado de um personagem
// conectado depois de uma mudanca no registro.
func (w *World) applyGuildMembership(character string, guildID uint16, rank byte) *Player {
	target := w.playerByCharacterName(character)
	if target == nil || target.Char == nil {
		return nil
	}
	target.Char.GuildID, target.Char.GuildRank = guildID, rank
	return target
}

func (w *World) guildCommandLeave(s *net.Session, p *Player, _ string) {
	guild, _ := w.guildOf(p.Char)
	if guild == nil {
		s.Send(wire.MessagePanel("Voce nao pertence a uma guild."))
		return
	}
	guildID, guildName := guild.ID, guild.Name
	snapshot := w.snapshotGuilds()
	oldID, oldRank := p.Char.GuildID, p.Char.GuildRank

	dissolved, promoted := w.removeGuildMember(guild, p.Char.Name)
	p.Char.GuildID, p.Char.GuildRank = 0, 0

	accounts := []*model.Account{p.Account}
	var promotedPlayer *Player
	if promoted != "" {
		promotedPlayer = w.applyGuildMembership(promoted, guildID, model.GuildRankLeader)
		if promotedPlayer != nil && promotedPlayer.Account != p.Account {
			accounts = append(accounts, promotedPlayer.Account)
		}
	}
	if dissolved {
		// Ninguem mais online para sincronizar: a guild deixou de existir.
		log.Printf("[#%d] GUILD %d %q dissolvida (ultimo membro saiu)", s.ID, guildID, guildName)
	}

	if err := w.saveGuildState(accounts...); err != nil {
		w.restoreGuilds(snapshot)
		p.Char.GuildID, p.Char.GuildRank = oldID, oldRank
		if promotedPlayer != nil {
			w.repairGuildState(promotedPlayer.Char)
		}
		s.Send(wire.MessagePanel("Falha ao salvar. Voce continua na guild."))
		log.Printf("[#%d] ERRO /sair: %v", s.ID, err)
		return
	}
	s.Send(wire.MessagePanel(fmt.Sprintf("Voce saiu da guild %s.", guildName)))
	if promoted != "" {
		w.announceToGuild(guildID, fmt.Sprintf("%s agora lidera a guild.", promoted), nil)
	}
	log.Printf("[#%d] GUILD %q saiu da guild %d", s.ID, p.Char.Name, guildID)
}

func (w *World) guildCommandExpel(s *net.Session, p *Player, arg string) {
	target := strings.TrimSpace(arg)
	if target == "" {
		s.Send(wire.MessagePanel("Uso: /expulsar <nick>"))
		return
	}
	guild, err := w.requireGuildLeader(p.Char)
	if err != nil {
		s.Send(wire.MessagePanel(guildAuthMessage(err, "expulsar")))
		return
	}
	if strings.EqualFold(target, p.Char.Name) {
		s.Send(wire.MessagePanel("Use /sair para deixar a guild."))
		return
	}
	victim := guild.Member(target)
	if victim == nil {
		s.Send(wire.MessagePanel("Esse jogador nao esta na sua guild."))
		return
	}
	// Primeiro marco: so expulsa quem esta conectado. Sem indice
	// personagem->conta nao da para atualizar o JSON de um membro offline.
	victimPlayer := w.playerByCharacterName(victim.Character)
	if victimPlayer == nil || victimPlayer.Char == nil {
		s.Send(wire.MessagePanel("So e possivel expulsar membros conectados."))
		return
	}
	// Mesma hierarquia do pacote nativo: rank estritamente maior.
	if _, actor := w.guildOf(p.Char); actor != nil && !model.CanDeprivate(actor.Rank, victim.Rank) {
		s.Send(wire.MessagePanel("Voce nao pode expulsar esse membro."))
		return
	}
	w.expelGuildMember(s, p, guild, victimPlayer)
}

// onGuildDeprivate trata o 0x28C _MSG_Deprivate: o botao de EXPULSAR da UI.
//
// Diferente do comando /expulsar, que casa por nome, o pacote nativo manda o
// CLIENT ID do alvo (MSG_STANDARDPARM, Parm@12) -- por isso os dois caminhos
// existem. A regra de hierarquia e a do DoDeprivate: quem age precisa ter rank
// ESTRITAMENTE maior, entao sub-lider expulsa membro comum e o mestre expulsa
// qualquer um, mas ninguem expulsa um par nem o proprio mestre.
func (w *World) onGuildDeprivate(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || len(pkt) < 16 {
		return
	}
	targetID := uint16(binary.LittleEndian.Uint32(pkt[12:16]))
	guild, actor := w.guildOf(p.Char)
	if guild == nil {
		s.Send(wire.MessagePanel("Voce nao pertence a uma guild."))
		return
	}
	target := w.playerByID(targetID)
	if target == nil || target.Char == nil {
		s.Send(wire.MessagePanel("Jogador nao esta conectado."))
		log.Printf("[#%d] 0x28C alvo %d nao encontrado", s.ID, targetID)
		return
	}
	if target == p {
		s.Send(wire.MessagePanel("Use /sair para deixar a guild."))
		return
	}
	victim := guild.Member(target.Char.Name)
	if victim == nil {
		s.Send(wire.MessagePanel("Esse jogador nao esta na sua guild."))
		return
	}
	if !model.CanDeprivate(actor.Rank, victim.Rank) {
		s.Send(wire.MessagePanel("Voce nao pode expulsar esse membro."))
		log.Printf("[#%d] 0x28C recusado: rank %d nao expulsa rank %d",
			s.ID, actor.Rank, victim.Rank)
		return
	}
	w.expelGuildMember(s, p, guild, target)
}

// expelGuildMember e o caminho unico de expulsao, compartilhado pelo pacote
// 0x28C e pelo comando /expulsar.
func (w *World) expelGuildMember(s *net.Session, actor *Player, guild *model.Guild, victim *Player) {
	guildID, victimName := guild.ID, victim.Char.Name
	snapshot := w.snapshotGuilds()
	oldID, oldRank := victim.Char.GuildID, victim.Char.GuildRank

	w.removeGuildMember(guild, victimName)
	victim.Char.GuildID, victim.Char.GuildRank = 0, 0

	accounts := []*model.Account{actor.Account}
	if victim.Account != actor.Account {
		accounts = append(accounts, victim.Account)
	}
	if err := w.saveGuildState(accounts...); err != nil {
		w.restoreGuilds(snapshot)
		victim.Char.GuildID, victim.Char.GuildRank = oldID, oldRank
		s.Send(wire.MessagePanel("Falha ao salvar. Ninguem foi expulso."))
		log.Printf("[#%d] ERRO expulsar %q: %v", s.ID, victimName, err)
		return
	}
	s.Send(wire.MessagePanel(fmt.Sprintf("%s foi expulso da guild.", victimName)))
	victim.Session.Send(wire.MessagePanel("Voce foi expulso da guild."))
	log.Printf("[#%d] GUILD %q expulsou %q da guild %d", s.ID, actor.Char.Name, victimName, guildID)
}

// guildCommandSubLeader promove um membro a sub-lider, ocupando o menor dos
// tres slots livres (6, 7 ou 8), como Sub1/Sub2/Sub3 do nativo.
func (w *World) guildCommandSubLeader(s *net.Session, p *Player, arg string) {
	target := strings.TrimSpace(arg)
	if target == "" {
		s.Send(wire.MessagePanel("Uso: /criarsub <nick>"))
		return
	}
	guild, err := w.requireGuildLeader(p.Char)
	if err != nil {
		s.Send(wire.MessagePanel(guildAuthMessage(err, "promover")))
		return
	}
	promoted := w.playerByCharacterName(target)
	if promoted == nil || promoted.Char == nil {
		s.Send(wire.MessagePanel("Jogador nao esta conectado."))
		return
	}
	member := guild.Member(promoted.Char.Name)
	if member == nil {
		s.Send(wire.MessagePanel("Esse jogador nao esta na sua guild."))
		return
	}
	if member.Rank != model.GuildRankMember {
		s.Send(wire.MessagePanel("Esse membro ja possui cargo."))
		return
	}
	rank, ok := guild.FreeSubLeaderRank()
	if !ok {
		s.Send(wire.MessagePanel("A guild ja possui tres sub-lideres."))
		return
	}

	snapshot := w.snapshotGuilds()
	oldRank := promoted.Char.GuildRank
	member.Rank = rank
	promoted.Char.GuildRank = rank

	accounts := []*model.Account{p.Account}
	if promoted.Account != p.Account {
		accounts = append(accounts, promoted.Account)
	}
	if err := w.saveGuildState(accounts...); err != nil {
		w.restoreGuilds(snapshot)
		promoted.Char.GuildRank = oldRank
		s.Send(wire.MessagePanel("Falha ao salvar. A promocao nao foi aplicada."))
		log.Printf("[#%d] ERRO /criarsub: %v", s.ID, err)
		return
	}
	s.Send(wire.MessagePanel(fmt.Sprintf("%s agora e sub-lider.", promoted.Char.Name)))
	promoted.Session.Send(wire.MessagePanel("Voce foi promovido a sub-lider."))
	log.Printf("[#%d] GUILD %q promovido a sub-lider (rank %d) na guild %d",
		s.ID, promoted.Char.Name, rank, guild.ID)
}

// onGuildAlly trata o 0xE12 _MSG_GuildAlly. O nativo exige que o remetente seja
// o MESTRE da guild informada; o alvo e um id de guild, nao um personagem.
func (w *World) onGuildAlly(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || len(pkt) < 20 {
		return
	}
	claimed := uint16(binary.LittleEndian.Uint32(pkt[12:16]))
	allyID := uint16(binary.LittleEndian.Uint32(pkt[16:20]))
	guild, err := w.requireGuildLeader(p.Char)
	if err != nil {
		s.Send(wire.MessagePanel(guildAuthMessage(err, "aliar")))
		return
	}
	// O client manda a propria guild no primeiro campo; nao confiar nele.
	if claimed != 0 && claimed != guild.ID {
		log.Printf("[#%d] 0x E12 com guild divergente: pacote=%d real=%d", s.ID, claimed, guild.ID)
	}
	if allyID == 0 {
		guild.Ally = 0
	} else {
		if allyID == guild.ID {
			s.Send(wire.MessagePanel("Uma guild nao pode se aliar a si mesma."))
			return
		}
		if w.guilds.FindByID(allyID) == nil {
			s.Send(wire.MessagePanel("Guild aliada nao encontrada."))
			return
		}
		guild.Ally = allyID
	}

	snapshot := w.snapshotGuilds()
	if err := w.saveGuildState(p.Account); err != nil {
		w.restoreGuilds(snapshot)
		s.Send(wire.MessagePanel("Falha ao salvar. A alianca nao foi alterada."))
		log.Printf("[#%d] ERRO 0xE12: %v", s.ID, err)
		return
	}
	if guild.Ally == 0 {
		s.Send(wire.MessagePanel("Alianca desfeita."))
	} else {
		s.Send(wire.MessagePanel("Alianca firmada."))
	}
	log.Printf("[#%d] GUILD %d alianca -> %d", s.ID, guild.ID, guild.Ally)
}

// announceToGuild manda um aviso de painel para os membros online, opcionalmente
// pulando um jogador (normalmente o autor da acao, que ja recebeu resposta).
func (w *World) announceToGuild(guildID uint16, message string, skip *Player) {
	for _, member := range w.onlineGuildMembers(guildID) {
		if member == skip {
			continue
		}
		member.Session.Send(wire.MessagePanel(message))
	}
}

// snapshotGuilds/restoreGuilds dao rollback em memoria quando a gravacao falha.
// A copia e profunda na lista de membros porque os comandos a mutam no lugar.
func (w *World) snapshotGuilds() []model.Guild {
	out := make([]model.Guild, len(w.guilds.Guilds))
	for i, guild := range w.guilds.Guilds {
		out[i] = guild
		out[i].Members = append([]model.GuildMember(nil), guild.Members...)
	}
	return out
}

func (w *World) restoreGuilds(snapshot []model.Guild) {
	w.guilds.Guilds = snapshot
}
