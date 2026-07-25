package model

import (
	"fmt"
	"strings"
	"time"
)

// boss.go -- configuracao declarativa de um boss, carregada de data/boss/*.lua.
//
// SEPARACAO DE PAPEIS: este arquivo descreve O QUE o boss e. O COMPORTAMENTO
// (como um "caster" escolhe alvo, como um "phased" troca de fase) e hardcoded em
// Go, em internal/game/boss_types.go. O Lua apenas seleciona o tipo pelo nome e
// fornece os numeros. Assim nenhum arquivo de conteudo consegue inventar regra
// nova nem alterar estado do servidor.
//
// Bosses NAO vem do NPCGener: eles tem posicao e respawn proprios. Do NPC do
// catalogo eles herdam apenas os ASSETS (rosto, equipamento, atributos base).

// BossType e o comportamento hardcoded. Um valor fora desta lista derruba o
// boot -- e melhor falhar do que subir um boss inerte.
type BossType string

const (
	// BossTypeChaser persegue e ataca corpo a corpo. E a base dos demais.
	BossTypeChaser BossType = "chaser"
	// BossTypeCaster mantem distancia e lanca as skills configuradas.
	BossTypeCaster BossType = "caster"
	// BossTypeSummoner invoca grupos de adds enquanto luta.
	BossTypeSummoner BossType = "summoner"
	// BossTypePhased troca de comportamento em limiares de HP.
	BossTypePhased BossType = "phased"
)

func (t BossType) Valid() bool {
	switch t {
	case BossTypeChaser, BossTypeCaster, BossTypeSummoner, BossTypePhased:
		return true
	}
	return false
}

// BossSpawn e onde e de quanto em quanto tempo o boss nasce.
type BossSpawn struct {
	X, Y uint16
	// RespawnSeconds e o intervalo apos a morte. Zero = nao renasce (boss de
	// evento, spawnado sob demanda).
	RespawnSeconds int
}

// BossStats sobrescreve os atributos herdados do NPC base. Campo zerado
// significa "mantem o valor do NPC" -- por isso sao ponteiros: um boss pode
// legitimamente querer defesa 0.
type BossStats struct {
	Level     *uint32
	MaxHP     *uint32
	Attack    *uint32
	Defense   *uint32
	AttackRun *uint32
	ExpReward *uint32
	Gold      *uint32
}

// BossSkill e uma magia do repertorio do boss.
type BossSkill struct {
	// ID indexa data/SkillData.csv.
	ID int
	// CooldownSeconds e o intervalo minimo entre usos DESTA skill.
	CooldownSeconds int
	// Range e o alcance maximo para lancar.
	Range int
	// MaxHPPercent limita a skill a partir de certo dano recebido: a skill so
	// entra quando o HP do boss estiver EM OU ABAIXO deste percentual. 0 ou 100
	// significa "sempre disponivel".
	MaxHPPercent int
	// Message anuncia a conjuracao aos jogadores proximos.
	Message string
}

// BossSummon e um grupo de adds invocavel.
type BossSummon struct {
	// NPC e o nome no catalogo (data/npcs/*.json).
	NPC string
	// Count e quantos nascem por invocacao.
	Count int
	// MaxAlive limita o total simultaneo deste grupo. Zero = sem limite alem do
	// Count.
	MaxAlive int
	// CooldownSeconds e o intervalo entre invocacoes.
	CooldownSeconds int
	Message         string
}

// BossPhase e uma virada de comportamento por HP.
type BossPhase struct {
	// HPPercent e o limiar que ATIVA a fase, atravessado de cima para baixo.
	HPPercent int
	// Type e o comportamento que passa a valer. Vazio mantem o tipo do boss.
	Type BossType
	// ShieldPercent aplica um escudo que absorve esta porcentagem do dano.
	// Combinado com ShieldUntilAddsDead, e a mecanica classica de "mate os
	// guardas para ferir o boss".
	ShieldPercent int
	// ShieldUntilAddsDead derruba o escudo quando os adds do encontro morrem.
	ShieldUntilAddsDead bool
	Message             string
}

// BossDrop e uma recompensa especial, independente do carry/droprate nativo.
type BossDrop struct {
	// Item e o indice em data/itemlist.csv.
	Item uint16
	// ChancePercent e a probabilidade (1-100). 100 = garantido.
	ChancePercent int
	// Amount usa o EF_AMOUNT nativo para itens empilhaveis. Zero = uma unidade.
	Amount int
}

// BossConfig e um boss completo.
type BossConfig struct {
	// ID identifica o encontro nos logs e deve ser unico.
	ID string
	// Name e o nome exibido no client. Vazio usa o nome do NPC base.
	Name string
	// NPC e o nome do NPC do catalogo de onde vem rosto, equipamento e
	// atributos base.
	NPC   string
	Type  BossType
	Spawn BossSpawn
	Stats BossStats

	Skills  []BossSkill
	Summons []BossSummon
	Phases  []BossPhase
	Drops   []BossDrop

	// SpawnMessage e anunciada no mundo quando o boss nasce.
	SpawnMessage string
	// DeathMessage e anunciada quando ele morre.
	DeathMessage string
	// SourceFile e preenchido pelo loader, para o erro citar o arquivo.
	SourceFile string
}

// Validate recusa configuracao incoerente. Roda no boot: um boss torto derruba
// o servidor em vez de virar um monstro que nunca age.
func (c *BossConfig) Validate() error {
	where := c.SourceFile
	if where == "" {
		where = c.ID
	}
	if strings.TrimSpace(c.ID) == "" {
		return fmt.Errorf("%s: boss sem id", where)
	}
	if strings.TrimSpace(c.NPC) == "" {
		return fmt.Errorf("%s: boss %q sem npc base", where, c.ID)
	}
	if !c.Type.Valid() {
		return fmt.Errorf("%s: boss %q com tipo %q invalido (use chaser, caster, summoner ou phased)",
			where, c.ID, c.Type)
	}
	if c.Spawn.X == 0 || c.Spawn.Y == 0 {
		return fmt.Errorf("%s: boss %q sem posicao de spawn", where, c.ID)
	}
	if c.Spawn.RespawnSeconds < 0 {
		return fmt.Errorf("%s: boss %q com respawn negativo", where, c.ID)
	}

	// Um caster sem skill nunca lancaria nada: a configuracao esta incompleta.
	if c.Type == BossTypeCaster && len(c.Skills) == 0 {
		return fmt.Errorf("%s: boss %q e caster mas nao tem skills", where, c.ID)
	}
	if c.Type == BossTypeSummoner && len(c.Summons) == 0 {
		return fmt.Errorf("%s: boss %q e summoner mas nao tem summons", where, c.ID)
	}
	if c.Type == BossTypePhased && len(c.Phases) == 0 {
		return fmt.Errorf("%s: boss %q e phased mas nao tem phases", where, c.ID)
	}

	for i, skill := range c.Skills {
		if skill.ID < 0 {
			return fmt.Errorf("%s: boss %q skill[%d] com id negativo", where, c.ID, i)
		}
		if skill.CooldownSeconds < 0 || skill.Range < 0 {
			return fmt.Errorf("%s: boss %q skill[%d] com valor negativo", where, c.ID, i)
		}
		if skill.MaxHPPercent < 0 || skill.MaxHPPercent > 100 {
			return fmt.Errorf("%s: boss %q skill[%d] com max_hp_percent %d fora de [0,100]",
				where, c.ID, i, skill.MaxHPPercent)
		}
	}
	for i, summon := range c.Summons {
		if strings.TrimSpace(summon.NPC) == "" {
			return fmt.Errorf("%s: boss %q summon[%d] sem npc", where, c.ID, i)
		}
		if summon.Count <= 0 {
			return fmt.Errorf("%s: boss %q summon[%d] com count %d", where, c.ID, i, summon.Count)
		}
		if summon.MaxAlive < 0 {
			return fmt.Errorf("%s: boss %q summon[%d] com max_alive negativo", where, c.ID, i)
		}
		if summon.MaxAlive > 0 && summon.MaxAlive < summon.Count {
			return fmt.Errorf("%s: boss %q summon[%d]: max_alive (%d) menor que count (%d)",
				where, c.ID, i, summon.MaxAlive, summon.Count)
		}
	}

	seenPhase := make(map[int]struct{}, len(c.Phases))
	for i, phase := range c.Phases {
		if phase.HPPercent <= 0 || phase.HPPercent >= 100 {
			return fmt.Errorf("%s: boss %q phase[%d] com hp_percent %d fora de (0,100)",
				where, c.ID, i, phase.HPPercent)
		}
		if _, repeated := seenPhase[phase.HPPercent]; repeated {
			return fmt.Errorf("%s: boss %q tem duas fases no mesmo hp_percent %d",
				where, c.ID, phase.HPPercent)
		}
		seenPhase[phase.HPPercent] = struct{}{}
		if phase.Type != "" && !phase.Type.Valid() {
			return fmt.Errorf("%s: boss %q phase[%d] com tipo %q invalido", where, c.ID, i, phase.Type)
		}
		if phase.ShieldPercent < 0 || phase.ShieldPercent > 100 {
			return fmt.Errorf("%s: boss %q phase[%d] com shield_percent %d fora de [0,100]",
				where, c.ID, i, phase.ShieldPercent)
		}
		// Um escudo que so cai com os adds exige que existam adds para matar.
		if phase.ShieldUntilAddsDead && len(c.Summons) == 0 {
			return fmt.Errorf("%s: boss %q phase[%d] usa shield_until_adds_dead mas o boss nao invoca adds: o escudo nunca cairia",
				where, c.ID, i)
		}
	}

	for i, drop := range c.Drops {
		if drop.Item == 0 {
			return fmt.Errorf("%s: boss %q drop[%d] sem item", where, c.ID, i)
		}
		if drop.ChancePercent <= 0 || drop.ChancePercent > 100 {
			return fmt.Errorf("%s: boss %q drop[%d] com chance %d fora de (0,100]",
				where, c.ID, i, drop.ChancePercent)
		}
		if drop.Amount < 0 {
			return fmt.Errorf("%s: boss %q drop[%d] com amount negativo", where, c.ID, i)
		}
	}
	return nil
}

// RespawnDelay devolve o intervalo de renascimento ja como duracao. Zero
// significa "nao renasce".
func (c *BossConfig) RespawnDelay() time.Duration {
	if c.Spawn.RespawnSeconds <= 0 {
		return 0
	}
	return time.Duration(c.Spawn.RespawnSeconds) * time.Second
}

// BossCatalog e o conjunto carregado de data/boss/.
type BossCatalog struct {
	Bosses []BossConfig
}

// Validate recusa IDs duplicados entre arquivos diferentes.
func (c *BossCatalog) Validate() error {
	seen := make(map[string]string, len(c.Bosses))
	for i := range c.Bosses {
		boss := &c.Bosses[i]
		if err := boss.Validate(); err != nil {
			return err
		}
		if other, repeated := seen[boss.ID]; repeated {
			return fmt.Errorf("boss %q definido em %s e em %s", boss.ID, other, boss.SourceFile)
		}
		seen[boss.ID] = boss.SourceFile
	}
	return nil
}
