package data

import (
	"context"
	"fmt"
	"math"
	"time"

	lua "github.com/yuin/gopher-lua"
)

// boss_lua.go -- conversao de valores Lua para Go.
//
// Todas as funcoes daqui sao ESTRITAS: tipo errado, numero fracionario onde se
// espera inteiro e valor fora da faixa viram erro de boot. O objetivo e que um
// engano no arquivo de conteudo apareca imediatamente, com o caminho do campo,
// em vez de virar um boss silenciosamente errado.

func contextWithTimeout(d time.Duration) (context.Context, context.CancelFunc) {
	return context.WithTimeout(context.Background(), d)
}

// eachField percorre as chaves da tabela e aplica o decodificador de cada uma.
// Chave desconhecida e erro: protege contra campo digitado errado.
func eachField(table *lua.LTable, prefix string, known map[string]func(lua.LValue) error) error {
	var failure error
	table.ForEach(func(key, value lua.LValue) {
		if failure != nil {
			return
		}
		name, ok := key.(lua.LString)
		if !ok {
			failure = fmt.Errorf("%schave %v nao e um nome", prefix, key)
			return
		}
		decode, exists := known[string(name)]
		if !exists {
			failure = fmt.Errorf("%scampo desconhecido %q", prefix, string(name))
			return
		}
		if err := decode(value); err != nil {
			failure = fmt.Errorf("%s%s: %w", prefix, string(name), err)
		}
	})
	return failure
}

// eachEntry percorre uma lista de tabelas (skills, summons, phases, drops).
func eachEntry(value lua.LValue, field string, visit func(index int, entry *lua.LTable) error) error {
	table, err := luaTable(value, field)
	if err != nil {
		return err
	}
	var failure error
	index := 0
	table.ForEach(func(key, item lua.LValue) {
		if failure != nil {
			return
		}
		if _, ok := key.(lua.LNumber); !ok {
			failure = fmt.Errorf("%s deve ser uma lista; encontrei a chave %v", field, key)
			return
		}
		entry, ok := item.(*lua.LTable)
		if !ok {
			failure = fmt.Errorf("%s[%d] deve ser uma tabela", field, index+1)
			return
		}
		if err := visit(index+1, entry); err != nil {
			failure = err
		}
		index++
	})
	return failure
}

func luaTable(value lua.LValue, field string) (*lua.LTable, error) {
	table, ok := value.(*lua.LTable)
	if !ok {
		return nil, fmt.Errorf("%s deve ser uma tabela, veio %s", field, value.Type())
	}
	return table, nil
}

func luaString(value lua.LValue, target *string) error {
	text, ok := value.(lua.LString)
	if !ok {
		return fmt.Errorf("esperava texto, veio %s", value.Type())
	}
	*target = string(text)
	return nil
}

func luaBool(value lua.LValue, target *bool) error {
	switch value {
	case lua.LTrue:
		*target = true
	case lua.LFalse:
		*target = false
	default:
		return fmt.Errorf("esperava true ou false, veio %s", value.Type())
	}
	return nil
}

// luaWholeNumber exige um numero INTEIRO. Lua so tem float; aceitar 2.5 onde se
// espera contagem esconderia erro de digitacao.
func luaWholeNumber(value lua.LValue) (float64, error) {
	number, ok := value.(lua.LNumber)
	if !ok {
		return 0, fmt.Errorf("esperava numero, veio %s", value.Type())
	}
	raw := float64(number)
	if math.IsNaN(raw) || math.IsInf(raw, 0) {
		return 0, fmt.Errorf("numero invalido")
	}
	if raw != math.Trunc(raw) {
		return 0, fmt.Errorf("esperava inteiro, veio %v", raw)
	}
	return raw, nil
}

func luaInt(value lua.LValue, target *int) error {
	raw, err := luaWholeNumber(value)
	if err != nil {
		return err
	}
	if raw < math.MinInt32 || raw > math.MaxInt32 {
		return fmt.Errorf("valor %v fora da faixa suportada", raw)
	}
	*target = int(raw)
	return nil
}

func luaUint16(value lua.LValue, target *uint16) error {
	raw, err := luaWholeNumber(value)
	if err != nil {
		return err
	}
	if raw < 0 || raw > math.MaxUint16 {
		return fmt.Errorf("valor %v fora de [0,65535]", raw)
	}
	*target = uint16(raw)
	return nil
}

// luaUint32Ptr aloca o destino: e assim que "campo ausente" se distingue de
// "campo definido como zero" nos atributos que sobrescrevem o NPC base.
func luaUint32Ptr(value lua.LValue, target **uint32) error {
	raw, err := luaWholeNumber(value)
	if err != nil {
		return err
	}
	if raw < 0 || raw > math.MaxUint32 {
		return fmt.Errorf("valor %v fora de [0,4294967295]", raw)
	}
	converted := uint32(raw)
	*target = &converted
	return nil
}
