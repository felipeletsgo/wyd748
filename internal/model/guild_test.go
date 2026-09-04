package model

import "testing"

func leaderMember(character string) GuildMember {
	return GuildMember{Character: character, Account: "conta", Rank: GuildRankLeader}
}

func TestValidateGuildName(t *testing.T) {
	// O teto de 12 vem do GUILDNAME_LENGTH do W2PP; "Os Guardas" cabe, espaco
	// interno e permitido.
	valid := []string{"Alfa", "Guild123", "Os Guardas", "A", "123456789012"}
	for _, name := range valid {
		if err := ValidateGuildName(name); err != nil {
			t.Fatalf("nome %q deveria ser valido: %v", name, err)
		}
	}
	invalid := map[string]string{
		"":               "vazio",
		"   ":            "so espacos",
		" Alfa":          "espaco a esquerda",
		"Alfa ":          "espaco a direita",
		"NomeMuitoLongo": "acima de 12 caracteres",
		"Acentuação":     "nao-ASCII",
		"Nome#Hash":      "caractere invalido",
	}
	for name, reason := range invalid {
		if err := ValidateGuildName(name); err == nil {
			t.Fatalf("nome %q deveria ser recusado (%s)", name, reason)
		}
	}
}

func TestNormalizeGuildNameIgnoraCaixaEEspacos(t *testing.T) {
	if NormalizeGuildName("  AlFa  ") != "alfa" {
		t.Fatalf("normalizacao incorreta: %q", NormalizeGuildName("  AlFa  "))
	}
}

func TestGuildRegistryRecusaNomeDuplicadoIgnorandoCaixa(t *testing.T) {
	registry := &GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{
		{ID: 1, Name: "Alfa", Members: []GuildMember{leaderMember("A")}},
		{ID: 2, Name: "ALFA", Members: []GuildMember{leaderMember("B")}},
	}}
	if err := registry.Validate(); err == nil {
		t.Fatal("nomes iguais variando a caixa deveriam ser recusados")
	}
}

func TestGuildRegistryRecusaIDDuplicadoEForaDaFaixa(t *testing.T) {
	duplicated := &GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{
		{ID: 1, Name: "Alfa", Members: []GuildMember{leaderMember("A")}},
		{ID: 1, Name: "Beta", Members: []GuildMember{leaderMember("B")}},
	}}
	if err := duplicated.Validate(); err == nil {
		t.Fatal("id duplicado deveria ser recusado")
	}
	for _, id := range []uint16{0, MaxGuilds} {
		out := &GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{
			{ID: id, Name: "Alfa", Members: []GuildMember{leaderMember("A")}},
		}}
		if err := out.Validate(); err == nil {
			t.Fatalf("id %d fora de 1..%d deveria ser recusado", id, MaxGuildID)
		}
	}
}

func TestGuildExigeExatamenteUmLider(t *testing.T) {
	semLider := &Guild{ID: 1, Name: "Alfa", Members: []GuildMember{
		{Character: "A", Account: "c1", Rank: GuildRankMember},
	}}
	if err := semLider.Validate(); err == nil {
		t.Fatal("guild sem lider deveria ser recusada")
	}
	doisLideres := &Guild{ID: 1, Name: "Alfa", Members: []GuildMember{
		leaderMember("A"), leaderMember("B"),
	}}
	if err := doisLideres.Validate(); err == nil {
		t.Fatal("guild com dois lideres deveria ser recusada")
	}
}

func TestGuildRecusaMembroDuplicadoESemConta(t *testing.T) {
	duplicado := &Guild{ID: 1, Name: "Alfa", Members: []GuildMember{
		leaderMember("A"),
		{Character: "a", Account: "c2", Rank: GuildRankMember},
	}}
	if err := duplicado.Validate(); err == nil {
		t.Fatal("membro repetido (ignorando caixa) deveria ser recusado")
	}
	semConta := &Guild{ID: 1, Name: "Alfa", Members: []GuildMember{
		{Character: "A", Account: "", Rank: GuildRankLeader},
	}}
	if err := semConta.Validate(); err == nil {
		t.Fatal("membro sem conta deveria ser recusado: a guild precisa saber que arquivo persistir")
	}
}

func TestRegistryRecusaPersonagemEmDuasGuilds(t *testing.T) {
	registry := &GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{
		{ID: 1, Name: "Alfa", Members: []GuildMember{leaderMember("Felipe")}},
		{ID: 2, Name: "Beta", Members: []GuildMember{leaderMember("felipe")}},
	}}
	if err := registry.Validate(); err == nil {
		t.Fatal("personagem em duas guilds tornaria o reparo do login ambiguo")
	}
}

func TestRegistryRecusaVersaoDiferente(t *testing.T) {
	registry := &GuildRegistry{Version: GuildRegistryVersion + 1}
	if err := registry.Validate(); err == nil {
		t.Fatal("versao diferente deveria ser recusada, como no Score")
	}
}

func TestGuildRecusaReinoInvalido(t *testing.T) {
	guild := Guild{ID: 1, Name: "Alfa", Kingdom: 6, Members: []GuildMember{leaderMember("Lider")}}
	if err := guild.Validate(); err == nil {
		t.Fatal("guild com reino fora de 0/7/8 foi aceita")
	}
}

// CanDeprivate porta a regra do DoDeprivate nativo: rank ESTRITAMENTE maior.
func TestCanDeprivateSegueAHierarquiaNativa(t *testing.T) {
	cases := []struct {
		actor, target byte
		allowed       bool
		why           string
	}{
		{GuildRankLeader, GuildRankMember, true, "mestre expulsa membro"},
		{GuildRankLeader, GuildRankSubFirst, true, "mestre expulsa sub-lider"},
		{GuildRankSubFirst, GuildRankMember, true, "sub-lider expulsa membro"},
		{GuildRankSubFirst, GuildRankSubFirst, false, "sub-lider nao expulsa um par"},
		{GuildRankSubFirst, GuildRankLeader, false, "sub-lider nao expulsa o mestre"},
		{GuildRankMember, GuildRankMember, false, "membro nao expulsa ninguem"},
		{GuildRankLeader, GuildRankLeader, false, "mestre nao expulsa outro mestre"},
	}
	for _, tc := range cases {
		if got := CanDeprivate(tc.actor, tc.target); got != tc.allowed {
			t.Fatalf("%s: CanDeprivate(%d,%d)=%v, quer %v",
				tc.why, tc.actor, tc.target, got, tc.allowed)
		}
	}
}

func TestFreeSubLeaderRankOcupaOsTresSlots(t *testing.T) {
	guild := &Guild{ID: 1, Name: "Alfa", Members: []GuildMember{leaderMember("Lider")}}
	for want := byte(GuildRankSubFirst); want <= GuildRankSubLast; want++ {
		rank, ok := guild.FreeSubLeaderRank()
		if !ok || rank != want {
			t.Fatalf("slot esperado %d, veio %d (ok=%v)", want, rank, ok)
		}
		guild.Members = append(guild.Members, GuildMember{
			Character: string(rune('A' + rank)), Account: "c", Rank: rank,
		})
	}
	if _, ok := guild.FreeSubLeaderRank(); ok {
		t.Fatal("com os tres slots ocupados nao pode sobrar vaga")
	}
	if err := guild.Validate(); err != nil {
		t.Fatalf("guild com tres sub-lideres deveria ser valida: %v", err)
	}
}

func TestGuildRecusaDoisMembrosNoMesmoSlotDeSubLider(t *testing.T) {
	guild := &Guild{ID: 1, Name: "Alfa", Members: []GuildMember{
		leaderMember("Lider"),
		{Character: "A", Account: "c1", Rank: GuildRankSubFirst},
		{Character: "B", Account: "c2", Rank: GuildRankSubFirst},
	}}
	if err := guild.Validate(); err == nil {
		t.Fatal("dois membros no mesmo slot de sub-lider deveriam ser recusados")
	}
}

func TestGuildRecusaRankInvalido(t *testing.T) {
	guild := &Guild{ID: 1, Name: "Alfa", Members: []GuildMember{
		leaderMember("Lider"),
		{Character: "A", Account: "c1", Rank: 3}, // nao e membro, sub nem mestre
	}}
	if err := guild.Validate(); err == nil {
		t.Fatal("rank fora da escala nativa deveria ser recusado")
	}
}

func TestAliancaPrecisaApontarParaGuildExistente(t *testing.T) {
	semAlvo := &GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{
		{ID: 1, Name: "Alfa", Ally: 99, Members: []GuildMember{leaderMember("A")}},
	}}
	if err := semAlvo.Validate(); err == nil {
		t.Fatal("alianca para guild inexistente deveria ser recusada")
	}
	aSiMesma := &GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{
		{ID: 1, Name: "Alfa", Ally: 1, Members: []GuildMember{leaderMember("A")}},
	}}
	if err := aSiMesma.Validate(); err == nil {
		t.Fatal("guild aliada de si mesma deveria ser recusada")
	}
	valida := &GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{
		{ID: 1, Name: "Alfa", Ally: 2, Members: []GuildMember{leaderMember("A")}},
		{ID: 2, Name: "Beta", Members: []GuildMember{leaderMember("B")}},
	}}
	if err := valida.Validate(); err != nil {
		t.Fatalf("alianca valida deveria passar: %v", err)
	}
}

func TestGuerraPodeSerUnilateralMasNaoApontarParaGuildInvalida(t *testing.T) {
	base := func(target uint16) *GuildRegistry {
		return &GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{
			{ID: 1, Name: "Alfa", WarTarget: target, Members: []GuildMember{leaderMember("Lider1")}},
			{ID: 2, Name: "Beta", Members: []GuildMember{leaderMember("Lider2")}},
		}}
	}
	if err := base(2).Validate(); err != nil {
		t.Fatalf("declaracao unilateral valida foi recusada: %v", err)
	}
	if err := base(1).Validate(); err == nil {
		t.Fatal("guerra contra a propria guild deveria ser recusada")
	}
	if err := base(99).Validate(); err == nil {
		t.Fatal("guerra contra guild inexistente deveria ser recusada")
	}
}

func TestNextGuildIDReusaOMenorLivre(t *testing.T) {
	registry := &GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{
		{ID: 1, Name: "Alfa", Members: []GuildMember{leaderMember("A")}},
		{ID: 3, Name: "Beta", Members: []GuildMember{leaderMember("B")}},
	}}
	id, err := registry.NextGuildID()
	if err != nil || id != 2 {
		t.Fatalf("deveria reusar o menor id livre: id=%d err=%v", id, err)
	}
}

func TestFindByCharacterIgnoraCaixa(t *testing.T) {
	registry := &GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{
		{ID: 4, Name: "Alfa", Members: []GuildMember{leaderMember("Felipe")}},
	}}
	guild, member := registry.FindByCharacter("FELIPE")
	if guild == nil || member == nil || guild.ID != 4 {
		t.Fatal("busca por personagem deveria ignorar a caixa")
	}
}
