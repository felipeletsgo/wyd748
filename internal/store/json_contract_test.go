package store

import (
	"testing"

	"wydgo/internal/model"
)

func TestJSONStoreAsyncAccountNamesAndInstanceStateRoundTrip(t *testing.T) {
	dir := t.TempDir()
	st := NewJSONStore(dir)
	acc := postgresTestAccount("jsonacc", "JsonHero", model.Item{Index: 4011})
	acc.Chars[0].Gold = 123
	if err := st.SaveAccountAsync(acc); err != nil {
		t.Fatalf("SaveAccountAsync: %v", err)
	}
	st.Flush()
	loaded, err := st.LoadAccount(acc.Name)
	if err != nil {
		t.Fatal(err)
	}
	if loaded.Chars[0].Gold != 123 {
		t.Fatalf("autosave JSON nao ficou duravel: gold=%d", loaded.Chars[0].Gold)
	}
	names, err := st.CharacterNames()
	if err != nil {
		t.Fatal(err)
	}
	if _, ok := names["jsonhero"]; !ok {
		t.Fatalf("CharacterNames nao indexou JsonHero: %v", names)
	}

	state := &model.InstanceStateSnapshot{
		Version: model.InstanceStateVersion,
		NightmarePartyRuns: map[string]int{"normal:json": 2},
	}
	if err := st.SaveInstanceState(state); err != nil {
		t.Fatal(err)
	}
	got, err := st.LoadInstanceState()
	if err != nil {
		t.Fatal(err)
	}
	if got.Version != model.InstanceStateVersion || got.NightmarePartyRuns["normal:json"] != 2 {
		t.Fatalf("instance state JSON=%+v", got)
	}

	if err := st.SaveInstanceState(nil); err != nil {
		t.Fatal(err)
	}
	got, err = st.LoadInstanceState()
	if err != nil || got.Version != model.InstanceStateVersion || len(got.NightmarePartyRuns) != 0 {
		t.Fatalf("SaveInstanceState(nil)=%+v err=%v", got, err)
	}
	if err := st.SaveInstanceState(&model.InstanceStateSnapshot{Version: model.InstanceStateVersion + 1}); err == nil {
		t.Fatal("JSON store aceitou versao futura de instance_state")
	}
}

func TestJSONStoreCharacterNamesOnMissingDirectoryIsEmpty(t *testing.T) {
	st := &JSONStore{dir: t.TempDir() + "/missing"}
	names, err := st.CharacterNames()
	if err != nil {
		t.Fatal(err)
	}
	if len(names) != 0 {
		t.Fatalf("diretorio ausente retornou nomes: %v", names)
	}
}
