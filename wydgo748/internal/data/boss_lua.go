package data

import (
	"context"
	"fmt"
	"math"
	"sort"
	"time"

	lua "github.com/yuin/gopher-lua"
)

// boss_lua.go converts Lua values to Go. All decoders are strict: wrong types,
// fractional counts, and out-of-range values fail startup with a field path.

func contextWithTimeout(d time.Duration) (context.Context, context.CancelFunc) {
	return context.WithTimeout(context.Background(), d)
}

// eachField decodes each table key. Unknown keys are errors to catch typos.
func eachField(table *lua.LTable, prefix string, known map[string]func(lua.LValue) error) error {
	var failure error
	table.ForEach(func(key, value lua.LValue) {
		if failure != nil {
			return
		}
		name, ok := key.(lua.LString)
		if !ok {
			failure = fmt.Errorf("%skey %v is not a name", prefix, key)
			return
		}
		decode, exists := known[string(name)]
		if !exists {
			failure = fmt.Errorf("%sunknown field %q", prefix, string(name))
			return
		}
		if err := decode(value); err != nil {
			failure = fmt.Errorf("%s%s: %w", prefix, string(name), err)
		}
	})
	return failure
}

// eachEntry traverses a list of tables (skills, summons, phases, drops).
func eachEntry(value lua.LValue, field string, visit func(index int, entry *lua.LTable) error) error {
	table, err := luaTable(value, field)
	if err != nil {
		return err
	}
	type listEntry struct {
		key   float64
		value *lua.LTable
	}
	entries := make([]listEntry, 0)
	var failure error
	table.ForEach(func(key, item lua.LValue) {
		if failure != nil {
			return
		}
		number, ok := key.(lua.LNumber)
		if !ok {
			failure = fmt.Errorf("%s must be a list; found key %v", field, key)
			return
		}
		rawKey := float64(number)
		if math.IsNaN(rawKey) || math.IsInf(rawKey, 0) || rawKey < 1 || rawKey != math.Trunc(rawKey) {
			failure = fmt.Errorf("%s must use positive integer keys; found key %v", field, key)
			return
		}
		entry, ok := item.(*lua.LTable)
		if !ok {
			failure = fmt.Errorf("%s[%v] must be a table", field, key)
			return
		}
		entries = append(entries, listEntry{key: rawKey, value: entry})
	})
	if failure != nil {
		return failure
	}
	sort.Slice(entries, func(i, j int) bool {
		return entries[i].key < entries[j].key
	})
	for index, entry := range entries {
		expectedKey := float64(index + 1)
		if entry.key != expectedKey {
			return fmt.Errorf("%s must be a contiguous list starting at 1; found key %v at position %d", field, entry.key, index+1)
		}
		if err := visit(index+1, entry.value); err != nil {
			return err
		}
	}
	return nil
}

func luaTable(value lua.LValue, field string) (*lua.LTable, error) {
	table, ok := value.(*lua.LTable)
	if !ok {
		return nil, fmt.Errorf("%s must be a table, got %s", field, value.Type())
	}
	return table, nil
}

func luaString(value lua.LValue, target *string) error {
	text, ok := value.(lua.LString)
	if !ok {
		return fmt.Errorf("expected text, got %s", value.Type())
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
		return fmt.Errorf("expected true or false, got %s", value.Type())
	}
	return nil
}

// luaWholeNumber requires an integer. Accepting fractional counts would hide typos.
func luaWholeNumber(value lua.LValue) (float64, error) {
	number, ok := value.(lua.LNumber)
	if !ok {
		return 0, fmt.Errorf("expected number, got %s", value.Type())
	}
	raw := float64(number)
	if math.IsNaN(raw) || math.IsInf(raw, 0) {
		return 0, fmt.Errorf("invalid number")
	}
	if raw != math.Trunc(raw) {
		return 0, fmt.Errorf("expected integer, got %v", raw)
	}
	return raw, nil
}

func luaInt(value lua.LValue, target *int) error {
	raw, err := luaWholeNumber(value)
	if err != nil {
		return err
	}
	if raw < math.MinInt32 || raw > math.MaxInt32 {
		return fmt.Errorf("value %v outside the supported range", raw)
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
		return fmt.Errorf("value %v outside [0,65535]", raw)
	}
	*target = uint16(raw)
	return nil
}

// luaUint32Ptr distinguishes an absent override from an explicit zero.
func luaUint32Ptr(value lua.LValue, target **uint32) error {
	raw, err := luaWholeNumber(value)
	if err != nil {
		return err
	}
	if raw < 0 || raw > math.MaxUint32 {
		return fmt.Errorf("value %v outside [0,4294967295]", raw)
	}
	converted := uint32(raw)
	*target = &converted
	return nil
}
