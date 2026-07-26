package data

import (
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"time"

	lua "github.com/yuin/gopher-lua"

	"wydgo/internal/model"
)

// boss.go -- carrega data/boss/*.lua.
//
// O Lua aqui e um formato de CONFIGURACAO, nao um motor de comportamento: cada
// arquivo devolve uma tabela e o Go le os valores. O comportamento vive em
// internal/game/boss_types.go, hardcoded.
//
// SANDBOX: o interpretador sobe sem as bibliotecas de sistema (os, io, package,
// debug) e com limite de tempo. Um arquivo de conteudo nao pode abrir arquivo,
// executar processo, acessar rede nem travar o boot. As bibliotecas seguras
// (base, string, table, math) ficam, porque tornam o config expressivo -- dar
// para escrever `hp = 500 * 1000` ou montar uma lista com um loop e justamente
// a vantagem de usar Lua em vez de JSON.

// bossScriptTimeout limita a execucao de UM arquivo. Config e declarativo: se
// levar mais que isso, ha laco infinito.
const bossScriptTimeout = 5 * time.Second

// LoadBossCatalog le todos os .lua do diretorio. Diretorio ausente devolve
// catalogo vazio: um servidor sem boss e uma configuracao legitima.
func LoadBossCatalog(dir string) (model.BossCatalog, error) {
	entries, err := os.ReadDir(dir)
	if os.IsNotExist(err) {
		return model.BossCatalog{}, nil
	}
	if err != nil {
		return model.BossCatalog{}, fmt.Errorf("abrir diretorio de boss %q: %w", dir, err)
	}

	names := make([]string, 0, len(entries))
	for _, entry := range entries {
		if entry.IsDir() || !strings.EqualFold(filepath.Ext(entry.Name()), ".lua") {
			continue
		}
		names = append(names, entry.Name())
	}
	// Ordem alfabetica: o catalogo (e qualquer erro) fica reproduzivel.
	sort.Strings(names)

	var catalog model.BossCatalog
	for _, name := range names {
		path := filepath.Join(dir, name)
		boss, err := loadBossFile(path)
		if err != nil {
			return model.BossCatalog{}, err
		}
		catalog.Bosses = append(catalog.Bosses, boss)
	}
	if err := catalog.Validate(); err != nil {
		return model.BossCatalog{}, err
	}
	return catalog, nil
}

// newBossSandbox cria um estado Lua sem acesso ao sistema operacional.
func newBossSandbox() *lua.LState {
	state := lua.NewState(lua.Options{SkipOpenLibs: true})
	// Somente bibliotecas puras: nenhuma toca em arquivo, processo ou rede.
	for _, lib := range []struct {
		name string
		open lua.LGFunction
	}{
		{lua.BaseLibName, lua.OpenBase},
		{lua.StringLibName, lua.OpenString},
		{lua.TabLibName, lua.OpenTable},
		{lua.MathLibName, lua.OpenMath},
	} {
		state.Push(state.NewFunction(lib.open))
		state.Push(lua.LString(lib.name))
		state.Call(1, 0)
	}
	// OpenBase traz dofile/loadfile, que leem disco. Removidos explicitamente:
	// um arquivo de boss nao pode carregar outro nem sair do diretorio.
	for _, unsafe := range []string{"dofile", "loadfile", "load", "loadstring", "collectgarbage", "print"} {
		state.SetGlobal(unsafe, lua.LNil)
	}
	return state
}

// loadBossFile executa UM arquivo e converte a tabela devolvida.
func loadBossFile(path string) (model.BossConfig, error) {
	state := newBossSandbox()
	defer state.Close()

	// O timeout corta laco infinito sem derrubar o processo.
	ctx, cancel := contextWithTimeout(bossScriptTimeout)
	defer cancel()
	state.SetContext(ctx)

	if err := state.DoFile(path); err != nil {
		return model.BossConfig{}, fmt.Errorf("%s: %w", path, err)
	}
	if state.GetTop() == 0 {
		return model.BossConfig{}, fmt.Errorf("%s: o arquivo precisa terminar com `return { ... }`", path)
	}
	table, ok := state.Get(-1).(*lua.LTable)
	if !ok {
		return model.BossConfig{}, fmt.Errorf("%s: o retorno precisa ser uma tabela", path)
	}

	boss := model.BossConfig{SourceFile: filepath.Base(path)}
	if err := decodeBossTable(table, &boss); err != nil {
		return model.BossConfig{}, fmt.Errorf("%s: %w", path, err)
	}
	return boss, nil
}

// decodeBossTable copia a tabela Lua para a struct. Chave desconhecida e ERRO,
// nao aviso: um `atack = 500` digitado errado passaria despercebido e o boss
// nasceria fraco sem ninguem entender por que.
func decodeBossTable(table *lua.LTable, boss *model.BossConfig) error {
	known := map[string]func(lua.LValue) error{
		"id":            func(v lua.LValue) error { return luaString(v, &boss.ID) },
		"name":          func(v lua.LValue) error { return luaString(v, &boss.Name) },
		"npc":           func(v lua.LValue) error { return luaString(v, &boss.NPC) },
		"spawn_message": func(v lua.LValue) error { return luaString(v, &boss.SpawnMessage) },
		"death_message": func(v lua.LValue) error { return luaString(v, &boss.DeathMessage) },
		"area_reward":   func(v lua.LValue) error { return decodeBossAreaReward(v, &boss.AreaReward) },
		"type": func(v lua.LValue) error {
			var text string
			if err := luaString(v, &text); err != nil {
				return err
			}
			boss.Type = model.BossType(strings.ToLower(strings.TrimSpace(text)))
			return nil
		},
		"spawn":   func(v lua.LValue) error { return decodeBossSpawn(v, &boss.Spawn) },
		"stats":   func(v lua.LValue) error { return decodeBossStats(v, &boss.Stats) },
		"skills":  func(v lua.LValue) error { return decodeBossSkills(v, &boss.Skills) },
		"summons": func(v lua.LValue) error { return decodeBossSummons(v, &boss.Summons) },
		"phases":  func(v lua.LValue) error { return decodeBossPhases(v, &boss.Phases) },
		"drops":   func(v lua.LValue) error { return decodeBossDrops(v, &boss.Drops) },
	}
	return eachField(table, "", known)
}

func decodeBossSpawn(value lua.LValue, spawn *model.BossSpawn) error {
	table, err := luaTable(value, "spawn")
	if err != nil {
		return err
	}
	return eachField(table, "spawn.", map[string]func(lua.LValue) error{
		"x":               func(v lua.LValue) error { return luaUint16(v, &spawn.X) },
		"y":               func(v lua.LValue) error { return luaUint16(v, &spawn.Y) },
		"respawn_seconds": func(v lua.LValue) error { return luaInt(v, &spawn.RespawnSeconds) },
	})
}

// decodeBossAreaReward le a premiacao coletiva: item e quantas unidades caem
// no chao ao redor do boss.
func decodeBossAreaReward(value lua.LValue, reward *model.BossAreaReward) error {
	table, err := luaTable(value, "area_reward")
	if err != nil {
		return err
	}
	return eachField(table, "area_reward.", map[string]func(lua.LValue) error{
		"item":   func(v lua.LValue) error { return luaUint16(v, &reward.Item) },
		"amount": func(v lua.LValue) error { return luaInt(v, &reward.Amount) },
	})
}

func decodeBossStats(value lua.LValue, stats *model.BossStats) error {
	table, err := luaTable(value, "stats")
	if err != nil {
		return err
	}
	return eachField(table, "stats.", map[string]func(lua.LValue) error{
		"level":      func(v lua.LValue) error { return luaUint32Ptr(v, &stats.Level) },
		"max_hp":     func(v lua.LValue) error { return luaUint32Ptr(v, &stats.MaxHP) },
		"attack":     func(v lua.LValue) error { return luaUint32Ptr(v, &stats.Attack) },
		"defense":    func(v lua.LValue) error { return luaUint32Ptr(v, &stats.Defense) },
		"attack_run": func(v lua.LValue) error { return luaUint32Ptr(v, &stats.AttackRun) },
		"exp_reward": func(v lua.LValue) error { return luaUint32Ptr(v, &stats.ExpReward) },
		"gold":       func(v lua.LValue) error { return luaUint32Ptr(v, &stats.Gold) },
	})
}

func decodeBossSkills(value lua.LValue, skills *[]model.BossSkill) error {
	return eachEntry(value, "skills", func(index int, entry *lua.LTable) error {
		var skill model.BossSkill
		prefix := fmt.Sprintf("skills[%d].", index)
		if err := eachField(entry, prefix, map[string]func(lua.LValue) error{
			"id":               func(v lua.LValue) error { return luaInt(v, &skill.ID) },
			"cooldown_seconds": func(v lua.LValue) error { return luaInt(v, &skill.CooldownSeconds) },
			"range":            func(v lua.LValue) error { return luaInt(v, &skill.Range) },
			"max_hp_percent":   func(v lua.LValue) error { return luaInt(v, &skill.MaxHPPercent) },
			"message":          func(v lua.LValue) error { return luaString(v, &skill.Message) },
		}); err != nil {
			return err
		}
		*skills = append(*skills, skill)
		return nil
	})
}

func decodeBossSummons(value lua.LValue, summons *[]model.BossSummon) error {
	return eachEntry(value, "summons", func(index int, entry *lua.LTable) error {
		var summon model.BossSummon
		prefix := fmt.Sprintf("summons[%d].", index)
		if err := eachField(entry, prefix, map[string]func(lua.LValue) error{
			"npc":              func(v lua.LValue) error { return luaString(v, &summon.NPC) },
			"count":            func(v lua.LValue) error { return luaInt(v, &summon.Count) },
			"max_alive":        func(v lua.LValue) error { return luaInt(v, &summon.MaxAlive) },
			"cooldown_seconds": func(v lua.LValue) error { return luaInt(v, &summon.CooldownSeconds) },
			"message":          func(v lua.LValue) error { return luaString(v, &summon.Message) },
		}); err != nil {
			return err
		}
		*summons = append(*summons, summon)
		return nil
	})
}

func decodeBossPhases(value lua.LValue, phases *[]model.BossPhase) error {
	return eachEntry(value, "phases", func(index int, entry *lua.LTable) error {
		var phase model.BossPhase
		prefix := fmt.Sprintf("phases[%d].", index)
		if err := eachField(entry, prefix, map[string]func(lua.LValue) error{
			"hp_percent":     func(v lua.LValue) error { return luaInt(v, &phase.HPPercent) },
			"shield_percent": func(v lua.LValue) error { return luaInt(v, &phase.ShieldPercent) },
			"message":        func(v lua.LValue) error { return luaString(v, &phase.Message) },
			"type": func(v lua.LValue) error {
				var text string
				if err := luaString(v, &text); err != nil {
					return err
				}
				phase.Type = model.BossType(strings.ToLower(strings.TrimSpace(text)))
				return nil
			},
			"shield_until_adds_dead": func(v lua.LValue) error {
				return luaBool(v, &phase.ShieldUntilAddsDead)
			},
		}); err != nil {
			return err
		}
		*phases = append(*phases, phase)
		return nil
	})
}

func decodeBossDrops(value lua.LValue, drops *[]model.BossDrop) error {
	return eachEntry(value, "drops", func(index int, entry *lua.LTable) error {
		var drop model.BossDrop
		prefix := fmt.Sprintf("drops[%d].", index)
		if err := eachField(entry, prefix, map[string]func(lua.LValue) error{
			"item":           func(v lua.LValue) error { return luaUint16(v, &drop.Item) },
			"chance_percent": func(v lua.LValue) error { return luaInt(v, &drop.ChancePercent) },
			"amount":         func(v lua.LValue) error { return luaInt(v, &drop.Amount) },
		}); err != nil {
			return err
		}
		*drops = append(*drops, drop)
		return nil
	})
}
