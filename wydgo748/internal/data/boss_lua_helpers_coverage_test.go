package data

import (
	"errors"
	"math"
	"reflect"
	"strings"
	"testing"

	lua "github.com/yuin/gopher-lua"
)

func TestEachFieldValidatesKeysNamesAndValues(t *testing.T) {
	for _, tc := range []struct {
		name string
		key  lua.LValue
		val  lua.LValue
		want string
	}{
		{name: "non-string key", key: lua.LNumber(1), val: lua.LString("ok"), want: "settings.key 1 is not a name"},
		{name: "unknown name", key: lua.LString("typo"), val: lua.LString("ok"), want: `settings.unknown field "typo"`},
		{name: "decoder failure", key: lua.LString("count"), val: lua.LString("bad"), want: "settings.count: expected number"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			table := &lua.LTable{}
			table.RawSet(tc.key, tc.val)
			err := eachField(table, "settings.", map[string]func(lua.LValue) error{
				"count": func(value lua.LValue) error { _, err := luaWholeNumber(value); return err },
			})
			if err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("eachField error = %v, want substring %q", err, tc.want)
			}
		})
	}

	var decoded string
	table := &lua.LTable{}
	table.RawSetString("name", lua.LString("boss"))
	if err := eachField(table, "", map[string]func(lua.LValue) error{
		"name": func(value lua.LValue) error { return luaString(value, &decoded) },
	}); err != nil || decoded != "boss" {
		t.Fatalf("valid field decoded as %q, err=%v", decoded, err)
	}
}

func TestEachEntryValidatesAndOrdersListEntries(t *testing.T) {
	first, second := &lua.LTable{}, &lua.LTable{}
	list := &lua.LTable{}
	list.RawSetInt(2, second)
	list.RawSetInt(1, first)
	var indexes []int
	if err := eachEntry(list, "phases", func(index int, entry *lua.LTable) error {
		indexes = append(indexes, index)
		if (index == 1 && entry != first) || (index == 2 && entry != second) {
			t.Fatalf("entry %d was not visited in list order", index)
		}
		return nil
	}); err != nil || !reflect.DeepEqual(indexes, []int{1, 2}) {
		t.Fatalf("ordered traversal = %v, err=%v", indexes, err)
	}

	visitErr := eachEntry(list, "phases", func(int, *lua.LTable) error { return errCoverageVisit })
	if visitErr != errCoverageVisit {
		t.Fatalf("visitor error = %v", visitErr)
	}

	for _, tc := range []struct {
		name  string
		value lua.LValue
		want  string
	}{
		{name: "not a table", value: lua.LString("no"), want: "phases must be a table"},
		{name: "string key", value: listWith(lua.LString("first"), &lua.LTable{}), want: "phases must be a list"},
		{name: "fractional key", value: listWith(lua.LNumber(1.5), &lua.LTable{}), want: "positive integer keys"},
		{name: "zero key", value: listWith(lua.LNumber(0), &lua.LTable{}), want: "positive integer keys"},
		{name: "non-table entry", value: listWith(lua.LNumber(1), lua.LString("no")), want: "phases[1] must be a table"},
		{name: "gap", value: listWith(lua.LNumber(2), &lua.LTable{}), want: "contiguous list starting at 1"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			err := eachEntry(tc.value, "phases", func(int, *lua.LTable) error { return nil })
			if err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("eachEntry error = %v, want substring %q", err, tc.want)
			}
		})
	}
}

var errCoverageVisit = errors.New("visitor stopped")

func listWith(key, value lua.LValue) *lua.LTable {
	list := &lua.LTable{}
	list.RawSet(key, value)
	return list
}

func TestLuaScalarDecodersRejectInvalidValuesAndBounds(t *testing.T) {
	if _, err := luaTable(lua.LString("x"), "skill"); err == nil || !strings.Contains(err.Error(), "skill must be a table") {
		t.Fatalf("luaTable error = %v", err)
	}
	var text string
	if err := luaString(lua.LNumber(1), &text); err == nil || !strings.Contains(err.Error(), "expected text") {
		t.Fatalf("luaString error = %v", err)
	}
	var boolean bool
	if err := luaBool(lua.LTrue, &boolean); err != nil || !boolean {
		t.Fatalf("luaBool(true) = %v, %v", boolean, err)
	}
	if err := luaBool(lua.LFalse, &boolean); err != nil || boolean {
		t.Fatalf("luaBool(false) = %v, %v", boolean, err)
	}
	if err := luaBool(lua.LNumber(1), &boolean); err == nil || !strings.Contains(err.Error(), "expected true or false") {
		t.Fatalf("luaBool error = %v", err)
	}

	for _, tc := range []struct {
		name  string
		value lua.LValue
		want  string
	}{
		{name: "wrong type", value: lua.LString("1"), want: "expected number"},
		{name: "fraction", value: lua.LNumber(1.25), want: "expected integer"},
		{name: "nan", value: lua.LNumber(math.NaN()), want: "invalid number"},
		{name: "positive infinity", value: lua.LNumber(math.Inf(1)), want: "invalid number"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if _, err := luaWholeNumber(tc.value); err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("luaWholeNumber error = %v, want substring %q", err, tc.want)
			}
		})
	}
	if got, err := luaWholeNumber(lua.LNumber(-12)); err != nil || got != -12 {
		t.Fatalf("luaWholeNumber(-12) = %v, %v", got, err)
	}

	for _, tc := range []struct {
		name  string
		value lua.LValue
		check func(lua.LValue) error
	}{
		{name: "int below range", value: lua.LNumber(math.MinInt32 - 1), check: func(value lua.LValue) error { var v int; return luaInt(value, &v) }},
		{name: "int above range", value: lua.LNumber(math.MaxInt32 + 1), check: func(value lua.LValue) error { var v int; return luaInt(value, &v) }},
		{name: "uint16 below range", value: lua.LNumber(-1), check: func(value lua.LValue) error { var v uint16; return luaUint16(value, &v) }},
		{name: "uint16 above range", value: lua.LNumber(math.MaxUint16 + 1), check: func(value lua.LValue) error { var v uint16; return luaUint16(value, &v) }},
		{name: "uint32 below range", value: lua.LNumber(-1), check: func(value lua.LValue) error { var v *uint32; return luaUint32Ptr(value, &v) }},
		{name: "uint32 above range", value: lua.LNumber(math.MaxUint32 + 1), check: func(value lua.LValue) error { var v *uint32; return luaUint32Ptr(value, &v) }},
	} {
		t.Run(tc.name, func(t *testing.T) {
			err := tc.check(tc.value)
			if err == nil || !strings.Contains(err.Error(), "outside") {
				t.Fatalf("range error = %v", err)
			}
		})
	}

	var intValue int
	if err := luaInt(lua.LNumber(math.MinInt32), &intValue); err != nil || int64(intValue) != math.MinInt32 {
		t.Fatalf("luaInt lower boundary = %d, %v", intValue, err)
	}
	var short uint16
	if err := luaUint16(lua.LNumber(math.MaxUint16), &short); err != nil || short != math.MaxUint16 {
		t.Fatalf("luaUint16 upper boundary = %d, %v", short, err)
	}
	var wide *uint32
	if err := luaUint32Ptr(lua.LNumber(math.MaxUint32), &wide); err != nil || wide == nil || *wide != math.MaxUint32 {
		t.Fatalf("luaUint32Ptr upper boundary = %v, %v", wide, err)
	}
	wide = nil
	if err := luaUint32Ptr(lua.LNumber(0), &wide); err != nil || wide == nil || *wide != 0 {
		t.Fatalf("luaUint32Ptr explicit zero = %v, %v", wide, err)
	}
}
