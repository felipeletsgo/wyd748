package model

import (
	"fmt"
	"strings"
	"time"
	"unicode"
)

const (
	// MaxGuilds espelha o MAX_GUILD=4096 do W2PP. O indice 0 significa "sem
	// guild", entao os IDs validos vao de 1 a 4095 e cabem no WORD que o
	// protocolo reserva para guild.
	MaxGuilds  = 4096
	MinGuildID = 1
	MaxGuildID = MaxGuilds - 1

	// GuildNameMaxLen espelha o GUILDNAME_LENGTH=12 do W2PP.
	GuildNameMaxLen = 12

	// Ranks na escala nativa (GuildLevel do W2PP). O client e o DoDeprivate
	// nativo comparam ranks numericamente, entao a escala precisa ser a mesma:
	// 9 = mestre, 6/7/8 = os tres slots de sub-lider, 0 = membro comum.
	GuildRankMember   = 0
	GuildRankSubFirst = 6
	GuildRankSubLast  = 8
	GuildRankLeader   = 9

	// MaxGuildSubLeaders espelha os tres slots Sub1/Sub2/Sub3 do nativo.
	MaxGuildSubLeaders = GuildRankSubLast - GuildRankSubFirst + 1
)

// IsSubLeader diz se o rank e um dos tres slots de sub-lider.
func IsSubLeader(rank byte) bool {
	return rank >= GuildRankSubFirst && rank <= GuildRankSubLast
}

// CanDeprivate porta a regra do DoDeprivate nativo: so da para expulsar quem
// tem rank ESTRITAMENTE menor. Assim sub-lider expulsa membro comum, o mestre
// expulsa qualquer um, e ninguem expulsa um par nem o proprio mestre.
func CanDeprivate(actorRank, targetRank byte) bool {
	return actorRank > targetRank
}

// GuildMember e uma entrada da lista canonica de membros. Guarda a conta junto
// do personagem porque a guild precisa saber qual arquivo persistir quando o
// membro estiver offline.
type GuildMember struct {
	Character string    `json:"character"`
	Account   string    `json:"account"`
	Rank      byte      `json:"rank"`
	JoinedAt  time.Time `json:"joinedAt"`
}

// Guild e o registro canonico. Char.GuildID/GuildRank sao copias
// desnormalizadas dessa verdade, reparadas no login.
type Guild struct {
	ID      uint16        `json:"id"`
	Name    string        `json:"name"`
	Members []GuildMember `json:"members"`
	// Kingdom espelha GuildInfo.Clan: 0 neutra, 7 Hekalotia, 8 Akelonia.
	Kingdom byte `json:"kingdom,omitempty"`
	// Ally e a guild aliada (0 = nenhuma). O nativo guarda UMA aliada por
	// guild, e o client reflete isso num unico m_usAllyGuild.
	Ally      uint16    `json:"ally,omitempty"`
	CreatedAt time.Time `json:"createdAt"`
}

// GuildRegistry e o conteudo do guilds.json.
type GuildRegistry struct {
	Version int     `json:"version"`
	Guilds  []Guild `json:"guilds"`
}

// GuildRegistryVersion e a unica versao aceita pelo loader, seguindo o mesmo
// contrato estrito do Score v2.
const GuildRegistryVersion = 1

// NormalizeGuildName devolve a forma usada para comparar unicidade. A exibicao
// preserva o nome original; a unicidade ignora caixa e espacos nas pontas.
func NormalizeGuildName(name string) string {
	return strings.ToLower(strings.TrimSpace(name))
}

// ValidateGuildName aplica as regras de nome no servidor. Charset conservador
// (ASCII alfanumerico mais espaco interno) porque o nome ainda vai atravessar
// um protocolo cp1252 quando a exibicao no client for confirmada.
func ValidateGuildName(name string) error {
	trimmed := strings.TrimSpace(name)
	if trimmed == "" {
		return fmt.Errorf("nome de guild vazio")
	}
	if trimmed != name {
		return fmt.Errorf("nome de guild %q possui espaco nas pontas", name)
	}
	if len(trimmed) > GuildNameMaxLen {
		return fmt.Errorf("nome de guild %q excede %d caracteres", name, GuildNameMaxLen)
	}
	for _, r := range trimmed {
		if r > unicode.MaxASCII {
			return fmt.Errorf("nome de guild %q possui caractere nao-ASCII", name)
		}
		if r == ' ' {
			continue
		}
		if !unicode.IsLetter(r) && !unicode.IsDigit(r) {
			return fmt.Errorf("nome de guild %q possui caractere invalido %q", name, r)
		}
	}
	return nil
}

// Leader devolve o membro lider. A validacao garante que existe exatamente um.
func (g *Guild) Leader() *GuildMember {
	for i := range g.Members {
		if g.Members[i].Rank == GuildRankLeader {
			return &g.Members[i]
		}
	}
	return nil
}

// Member acha um membro pelo nome do personagem, sem distincao de caixa.
func (g *Guild) Member(character string) *GuildMember {
	for i := range g.Members {
		if strings.EqualFold(g.Members[i].Character, character) {
			return &g.Members[i]
		}
	}
	return nil
}

func (g *Guild) Validate() error {
	if g == nil {
		return fmt.Errorf("guild ausente")
	}
	if g.ID < MinGuildID || g.ID > MaxGuildID {
		return fmt.Errorf("guild %q com id %d fora da faixa %d..%d",
			g.Name, g.ID, MinGuildID, MaxGuildID)
	}
	if err := ValidateGuildName(g.Name); err != nil {
		return fmt.Errorf("guild %d: %w", g.ID, err)
	}
	if g.Kingdom != KingdomNeutral && g.Kingdom != KingdomHekalotia &&
		g.Kingdom != KingdomAkelonia {
		return fmt.Errorf("guild %q com reino invalido %d", g.Name, g.Kingdom)
	}
	if len(g.Members) == 0 {
		return fmt.Errorf("guild %q sem membros", g.Name)
	}
	leaders := 0
	subRanks := make(map[byte]struct{}, MaxGuildSubLeaders)
	seen := make(map[string]struct{}, len(g.Members))
	for i := range g.Members {
		member := &g.Members[i]
		if member.Character == "" {
			return fmt.Errorf("guild %q possui membro sem personagem", g.Name)
		}
		if member.Account == "" {
			return fmt.Errorf("guild %q membro %q sem conta", g.Name, member.Character)
		}
		key := strings.ToLower(member.Character)
		if _, duplicate := seen[key]; duplicate {
			return fmt.Errorf("guild %q possui membro duplicado %q", g.Name, member.Character)
		}
		seen[key] = struct{}{}
		switch {
		case member.Rank == GuildRankLeader:
			leaders++
		case IsSubLeader(member.Rank):
			// Cada slot de sub-lider (6, 7 e 8) e unico, como Sub1/Sub2/Sub3.
			if _, taken := subRanks[member.Rank]; taken {
				return fmt.Errorf("guild %q possui dois membros no slot de sub-lider %d",
					g.Name, member.Rank)
			}
			subRanks[member.Rank] = struct{}{}
		case member.Rank != GuildRankMember:
			return fmt.Errorf("guild %q membro %q com rank invalido %d",
				g.Name, member.Character, member.Rank)
		}
	}
	if leaders != 1 {
		return fmt.Errorf("guild %q possui %d lideres; esperado exatamente 1", g.Name, leaders)
	}
	return nil
}

// FreeSubLeaderRank devolve o menor slot de sub-lider livre (6, 7 ou 8).
func (g *Guild) FreeSubLeaderRank() (byte, bool) {
	used := make(map[byte]struct{}, MaxGuildSubLeaders)
	for i := range g.Members {
		if IsSubLeader(g.Members[i].Rank) {
			used[g.Members[i].Rank] = struct{}{}
		}
	}
	for rank := byte(GuildRankSubFirst); rank <= GuildRankSubLast; rank++ {
		if _, taken := used[rank]; !taken {
			return rank, true
		}
	}
	return 0, false
}

func (r *GuildRegistry) Validate() error {
	if r == nil {
		return fmt.Errorf("registro de guild ausente")
	}
	if r.Version != GuildRegistryVersion {
		return fmt.Errorf("registro de guild versao %d; esperado %d",
			r.Version, GuildRegistryVersion)
	}
	ids := make(map[uint16]struct{}, len(r.Guilds))
	names := make(map[string]struct{}, len(r.Guilds))
	characters := make(map[string]struct{})
	for i := range r.Guilds {
		guild := &r.Guilds[i]
		if err := guild.Validate(); err != nil {
			return err
		}
		if _, duplicate := ids[guild.ID]; duplicate {
			return fmt.Errorf("guild id %d duplicado", guild.ID)
		}
		ids[guild.ID] = struct{}{}
		name := NormalizeGuildName(guild.Name)
		if _, duplicate := names[name]; duplicate {
			return fmt.Errorf("nome de guild %q duplicado", guild.Name)
		}
		names[name] = struct{}{}
		// Um personagem so pode pertencer a uma guild: sem isso o reparo do
		// estado desnormalizado no login ficaria ambiguo.
		for j := range guild.Members {
			key := strings.ToLower(guild.Members[j].Character)
			if _, duplicate := characters[key]; duplicate {
				return fmt.Errorf("personagem %q aparece em mais de uma guild",
					guild.Members[j].Character)
			}
			characters[key] = struct{}{}
		}
	}
	// Alianca so pode apontar para guild existente, nunca para si mesma.
	for i := range r.Guilds {
		guild := &r.Guilds[i]
		if guild.Ally == 0 {
			continue
		}
		if guild.Ally == guild.ID {
			return fmt.Errorf("guild %q aliada de si mesma", guild.Name)
		}
		if _, exists := ids[guild.Ally]; !exists {
			return fmt.Errorf("guild %q aliada da guild %d, que nao existe",
				guild.Name, guild.Ally)
		}
	}
	return nil
}

// FindByID e FindByName servem tanto ao runtime quanto a validacao de comandos.
func (r *GuildRegistry) FindByID(id uint16) *Guild {
	if id == 0 {
		return nil
	}
	for i := range r.Guilds {
		if r.Guilds[i].ID == id {
			return &r.Guilds[i]
		}
	}
	return nil
}

func (r *GuildRegistry) FindByName(name string) *Guild {
	wanted := NormalizeGuildName(name)
	for i := range r.Guilds {
		if NormalizeGuildName(r.Guilds[i].Name) == wanted {
			return &r.Guilds[i]
		}
	}
	return nil
}

// FindByCharacter localiza a guild de um personagem pela lista canonica.
func (r *GuildRegistry) FindByCharacter(character string) (*Guild, *GuildMember) {
	for i := range r.Guilds {
		if member := r.Guilds[i].Member(character); member != nil {
			return &r.Guilds[i], member
		}
	}
	return nil, nil
}

// GuildsTxt serializa o registro no formato que o CLIENT 7.48 le localmente de
// "./Guilds.txt". O WYD.exe abre esse arquivo em "rb" e parseia com
// `%d %d %d %s...` -- grupo, servidor, indice e nome.
//
// O "servidor" e o CANAL: o client remonta o WORD de guild como
// (canal << 12) | indice, que e exatamente o valor que enviamos em
// CreateMob@98 e 0x336@74. Rodando em instancia unica, grupo e canal sao 0 e o
// indice e o proprio GuildID (por isso ele cabe em 12 bits).
//
// Sem esse arquivo no client, nenhum nome de guild aparece, independente do que
// o servidor mande no wire -- nao existe pacote de nome de guild no 7.48.
func (r *GuildRegistry) GuildsTxt(group, channel int) []byte {
	var sb strings.Builder
	for i := range r.Guilds {
		guild := &r.Guilds[i]
		// O parser do client quebra o nome em tokens por espaco; nomes com
		// espaco continuam legiveis porque ele aceita varios %s seguidos.
		fmt.Fprintf(&sb, "%d %d %d %s\n", group, channel, guild.ID, guild.Name)
	}
	return []byte(sb.String())
}

// NextGuildID devolve o menor id livre em 1..4095, espelhando a alocacao por
// slot que o resto do servidor ja usa para ClientId.
func (r *GuildRegistry) NextGuildID() (uint16, error) {
	used := make(map[uint16]struct{}, len(r.Guilds))
	for i := range r.Guilds {
		used[r.Guilds[i].ID] = struct{}{}
	}
	for id := uint16(MinGuildID); id <= MaxGuildID; id++ {
		if _, taken := used[id]; !taken {
			return id, nil
		}
	}
	return 0, fmt.Errorf("limite de %d guilds atingido", MaxGuildID)
}
