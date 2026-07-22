package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	netpkg "wydgo/internal/net"
)

func guildTestWorld(guilds ...model.Guild) *World {
	return &World{guilds: &model.GuildRegistry{
		Version: model.GuildRegistryVersion,
		Guilds:  guilds,
	}}
}

func guildWithMembers(id uint16, name string, members ...model.GuildMember) model.Guild {
	return model.Guild{ID: id, Name: name, Members: members}
}

func member(character, account string, rank byte) model.GuildMember {
	return model.GuildMember{Character: character, Account: account, Rank: rank}
}

// --- autorizacao -----------------------------------------------------------

func TestRequireGuildLeader(t *testing.T) {
	w := guildTestWorld(guildWithMembers(1, "Alfa",
		member("Lider", "c1", model.GuildRankLeader),
		member("Membro", "c2", model.GuildRankMember),
	))

	if _, err := w.requireGuildLeader(&model.Char{Name: "Lider"}); err != nil {
		t.Fatalf("lider deveria passar: %v", err)
	}
	_, err := w.requireGuildLeader(&model.Char{Name: "Membro"})
	if !errors.Is(err, errGuildNotLeader) {
		t.Fatalf("membro comum nao pode gerenciar a guild: %v", err)
	}
	_, err = w.requireGuildLeader(&model.Char{Name: "Estranho"})
	if !errors.Is(err, errGuildMissing) {
		t.Fatalf("quem nao tem guild deveria receber errGuildMissing: %v", err)
	}
}

func TestGuildAuthMessageDistingueOsDoisCasos(t *testing.T) {
	if got := guildAuthMessage(errGuildNotLeader, "expulsar"); got != "Somente o lider pode expulsar." {
		t.Fatalf("mensagem de nao-lider incorreta: %q", got)
	}
	if got := guildAuthMessage(errGuildMissing, "expulsar"); got != "Voce nao pertence a uma guild." {
		t.Fatalf("mensagem de sem-guild incorreta: %q", got)
	}
}

// --- sucessao e dissolucao -------------------------------------------------

func TestRemoveGuildMemberPromoveOMaisAntigoQuandoOLiderSai(t *testing.T) {
	w := guildTestWorld(guildWithMembers(1, "Alfa",
		member("Lider", "c1", model.GuildRankLeader),
		member("Antigo", "c2", model.GuildRankMember),
		member("Novato", "c3", model.GuildRankMember),
	))
	guild := w.guilds.FindByID(1)

	dissolved, promoted := w.removeGuildMember(guild, "Lider")
	if dissolved {
		t.Fatal("guild com membros restantes nao pode dissolver")
	}
	if promoted != "Antigo" {
		t.Fatalf("sucessao deveria ir para o mais antigo: %q", promoted)
	}
	if guild.Members[0].Rank != model.GuildRankLeader {
		t.Fatal("o promovido precisa virar lider na lista canonica")
	}
	if err := guild.Validate(); err != nil {
		t.Fatalf("guild deveria continuar valida apos a sucessao: %v", err)
	}
}

func TestRemoveGuildMemberDissolveQuandoSaiOUltimo(t *testing.T) {
	w := guildTestWorld(guildWithMembers(1, "Alfa",
		member("Unico", "c1", model.GuildRankLeader),
	))
	guild := w.guilds.FindByID(1)

	dissolved, promoted := w.removeGuildMember(guild, "Unico")
	if !dissolved {
		t.Fatal("ultimo membro saindo deveria dissolver a guild")
	}
	if promoted != "" {
		t.Fatalf("nao ha quem promover: %q", promoted)
	}
	if w.guilds.FindByID(1) != nil {
		t.Fatal("guild dissolvida deveria sair do registro")
	}
}

func TestRemoveGuildMemberComumNaoAlteraLideranca(t *testing.T) {
	w := guildTestWorld(guildWithMembers(1, "Alfa",
		member("Lider", "c1", model.GuildRankLeader),
		member("Membro", "c2", model.GuildRankMember),
	))
	guild := w.guilds.FindByID(1)

	dissolved, promoted := w.removeGuildMember(guild, "Membro")
	if dissolved || promoted != "" {
		t.Fatalf("saida de membro comum nao mexe na lideranca: dissolved=%v promoted=%q",
			dissolved, promoted)
	}
	if guild.Member("Lider").Rank != model.GuildRankLeader {
		t.Fatal("o lider deveria permanecer lider")
	}
}

// --- reparo do estado desnormalizado ---------------------------------------

func TestRepairGuildStateLimpaGuildInexistente(t *testing.T) {
	w := guildTestWorld()
	ch := &model.Char{Name: "Orfao", GuildID: 7, GuildRank: model.GuildRankLeader}

	if !w.repairGuildState(ch) {
		t.Fatal("estado orfao deveria ser reparado")
	}
	if ch.GuildID != 0 || ch.GuildRank != 0 {
		t.Fatalf("guild inexistente deveria zerar o estado: id=%d rank=%d", ch.GuildID, ch.GuildRank)
	}
}

func TestRepairGuildStateCorrigeRankDivergente(t *testing.T) {
	w := guildTestWorld(guildWithMembers(3, "Alfa",
		member("Felipe", "c1", model.GuildRankLeader),
	))
	// O personagem ficou offline como membro comum e virou lider por sucessao.
	ch := &model.Char{Name: "Felipe", GuildID: 3, GuildRank: model.GuildRankMember}

	if !w.repairGuildState(ch) {
		t.Fatal("rank divergente deveria ser reparado")
	}
	if ch.GuildID != 3 || ch.GuildRank != model.GuildRankLeader {
		t.Fatalf("reparo incorreto: id=%d rank=%d", ch.GuildID, ch.GuildRank)
	}
}

func TestRepairGuildStateNaoMexeQuandoEstaCoerente(t *testing.T) {
	w := guildTestWorld(guildWithMembers(3, "Alfa",
		member("Felipe", "c1", model.GuildRankLeader),
	))
	ch := &model.Char{Name: "Felipe", GuildID: 3, GuildRank: model.GuildRankLeader}
	if w.repairGuildState(ch) {
		t.Fatal("estado coerente nao deveria ser marcado como reparado")
	}
}

// --- parsing de comando ----------------------------------------------------

func TestParseSlashCommandAceitaComandoSemArgumento(t *testing.T) {
	cases := []struct {
		in       string
		name     string
		arg      string
		expectOK bool
	}{
		{"/aceitar", "aceitar", "", true},
		{"/sair", "sair", "", true},
		{"/criar Alfa", "criar", "Alfa", true},
		{"/convidar   Felipe  ", "convidar", "Felipe", true},
		{"  /criar   Nome Composto ", "criar", "Nome Composto", true},
		{"sem barra", "", "", false},
		{"/", "", "", false},
		{"", "", "", false},
	}
	for _, tc := range cases {
		name, arg, ok := parseSlashCommand(tc.in)
		if ok != tc.expectOK || name != tc.name || arg != tc.arg {
			t.Fatalf("parseSlashCommand(%q) = (%q,%q,%v), quer (%q,%q,%v)",
				tc.in, name, arg, ok, tc.name, tc.arg, tc.expectOK)
		}
	}
}

// O parseSlashWhisperCommand exige corpo; e exatamente por isso que o
// dispatcher usa parseSlashCommand -- senao /aceitar nunca chegaria.
func TestParseSlashWhisperCommandNaoCobreComandoSemCorpo(t *testing.T) {
	if _, _, ok := parseSlashWhisperCommand("/aceitar"); ok {
		t.Fatal("parseSlashWhisperCommand nao deveria aceitar comando sem corpo")
	}
}

// --- classificacao de canal ------------------------------------------------

// Regressao da armadilha registrada no plano: '--' e o chat GLOBAL e precisa
// ser reconhecido antes de '-' (guild). Se a ordem dos testes de prefixo
// inverter, todo global passa a vazar na rota de guild.
func TestChatChannelOfTestaGlobalAntesDeGuild(t *testing.T) {
	cases := map[string]string{
		"--anuncio":   chatChannelGlobal,
		"--":          chatChannelGlobal,
		"-conversa":   chatChannelGuild,
		"-":           chatChannelGuild,
		"=grupo":      chatChannelParty,
		"texto solto": chatChannelWhisper,
		"":            chatChannelWhisper,
	}
	for message, want := range cases {
		if got := chatChannelOf(message); got != want {
			t.Fatalf("chatChannelOf(%q) = %q, quer %q", message, got, want)
		}
	}
}

// --- membros online --------------------------------------------------------

func TestOnlineGuildMembersFiltraPorGuildEPresenca(t *testing.T) {
	w := guildTestWorld(guildWithMembers(5, "Alfa",
		member("A", "c1", model.GuildRankLeader),
	))
	inGuild := &Player{ID: 1, InWorld: true, Char: &model.Char{Name: "A", GuildID: 5}}
	otherGuild := &Player{ID: 2, InWorld: true, Char: &model.Char{Name: "B", GuildID: 6}}
	offline := &Player{ID: 3, InWorld: false, Char: &model.Char{Name: "C", GuildID: 5}}
	noGuild := &Player{ID: 4, InWorld: true, Char: &model.Char{Name: "D"}}
	w.players = map[*netpkg.Session]*Player{
		{ID: 1}: inGuild,
		{ID: 2}: otherGuild,
		{ID: 3}: offline,
		{ID: 4}: noGuild,
	}

	got := w.onlineGuildMembers(5)
	if len(got) != 1 || got[0] != inGuild {
		t.Fatalf("apenas o membro conectado da guild 5 deveria entrar: %d", len(got))
	}
	if len(w.onlineGuildMembers(0)) != 0 {
		t.Fatal("guild 0 significa sem guild e nao pode devolver ninguem")
	}
}
