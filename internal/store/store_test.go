package store

import (
	"bytes"
	"encoding/json"
	"errors"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func validStoredChar(name string, gold uint32) model.Char {
	return model.Char{
		Name: name, Gold: gold,
		Score: &model.Score{Version: model.ScoreVersion},
	}
}

func TestJSONInstanceStateRoundTripSharesAccountTransaction(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	st := NewJSONStore(accounts)
	acc := postgresTestAccount("runtime", "Runtime", model.Item{Index: 4011})
	state := &model.InstanceStateSnapshot{
		Version:            model.InstanceStateVersion,
		NightmarePartyRuns: map[string]int{"nightmare-normal:window": 2},
		Instances: []model.InstanceRuntimeState{{
			RuntimeID: "shared:nightmare-normal", ConfigID: "nightmare-normal-party",
			State: "entry", CurrentStage: 0,
		}},
	}
	if err := st.SaveGameStateWithInstanceState(nil, state, acc); err != nil {
		t.Fatal(err)
	}
	loaded, err := st.LoadInstanceState()
	if err != nil {
		t.Fatal(err)
	}
	if loaded.NightmarePartyRuns["nightmare-normal:window"] != 2 ||
		len(loaded.Instances) != 1 || loaded.Instances[0].RuntimeID != "shared:nightmare-normal" {
		t.Fatalf("estado de instancia nao persistiu junto da conta: %+v", loaded)
	}
	if _, err := st.LoadAccount("runtime"); err != nil {
		t.Fatalf("conta nao persistiu junto do estado: %v", err)
	}
}

func TestLoadInstanceStateRejectsTrailingJSON(t *testing.T) {
	root := t.TempDir()
	path := filepath.Join(root, "instance_state.json")
	st := NewJSONStore(filepath.Join(root, "accounts"))
	if err := os.WriteFile(path, []byte(`{"version":1}{"version":1}`), 0o644); err != nil {
		t.Fatal(err)
	}
	if _, err := st.LoadInstanceState(); err == nil {
		t.Fatal("instance_state com JSON adicional foi aceito")
	}
}

func TestSaveAccountsPersistsBothSidesOfTrade(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(dir)
	a := &model.Account{Name: "alice", PasswordHash: "hash", Chars: []model.Char{validStoredChar("Alice", 100)}}
	b := &model.Account{Name: "bob", PasswordHash: "hash", Chars: []model.Char{validStoredChar("Bob", 200)}}
	if err := s.SaveAccounts(a, b); err != nil {
		t.Fatal(err)
	}
	gotA, err := s.LoadAccount("alice")
	if err != nil {
		t.Fatal(err)
	}
	gotB, err := s.LoadAccount("bob")
	if err != nil {
		t.Fatal(err)
	}
	if gotA.Chars[0].Gold != 100 || gotB.Chars[0].Gold != 200 {
		t.Fatalf("transacao incompleta: alice=%d bob=%d", gotA.Chars[0].Gold, gotB.Chars[0].Gold)
	}
	entries, err := os.ReadDir(dir)
	if err != nil {
		t.Fatal(err)
	}
	for _, entry := range entries {
		if entry.IsDir() {
			t.Fatalf("journal nao removido: %s", entry.Name())
		}
	}
}

func TestSaveAccountsTransfersFilledCelestialSealAndSnapshot(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(dir)
	itemUID, err := model.NewItemUID()
	if err != nil {
		t.Fatal(err)
	}
	sourceUID, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	seal := model.NewCelestialSeal(itemUID, 7)
	a := &model.Account{Name: "alice", PasswordHash: "hash", Chars: []model.Char{validStoredChar("Alice", 100)}}
	b := &model.Account{Name: "bob", PasswordHash: "hash", Chars: []model.Char{validStoredChar("Bob", 200)}}
	a.Chars[0].Inv[0] = seal
	a.CelestialCapsules = []model.CelestialCapsule{{
		ID: 7, ItemUID: itemUID, SourceUID: sourceUID,
		Character: model.Char{
			UID: sourceUID, Name: "ReadyCelestial", Class: 1, Evolution: "celestial",
			Score: &model.Score{Version: model.ScoreVersion},
		},
	}}
	if err := s.SaveAccounts(a, b); err != nil {
		t.Fatal(err)
	}

	a.Chars[0].Inv[0] = model.Item{}
	b.Chars[0].Inv[0] = seal
	b.CelestialCapsules = a.CelestialCapsules
	a.CelestialCapsules = nil
	if err := s.SaveAccounts(a, b); err != nil {
		t.Fatalf("transferencia atomica recusada: %v", err)
	}
	gotA, err := s.LoadAccount("alice")
	if err != nil {
		t.Fatal(err)
	}
	gotB, err := s.LoadAccount("bob")
	if err != nil {
		t.Fatal(err)
	}
	if gotA.Chars[0].Inv[0].Index != 0 || len(gotA.CelestialCapsules) != 0 ||
		gotB.Chars[0].Inv[0].UID != itemUID || len(gotB.CelestialCapsules) != 1 ||
		gotB.CelestialCapsules[0].ItemUID != itemUID {
		t.Fatalf("selo/snapshot nao mudaram juntos: alice=%+v bob=%+v", gotA, gotB)
	}
}

func TestNewJSONStoreRecoversCommittedAccountTransaction(t *testing.T) {
	dir := t.TempDir()
	txn := filepath.Join(dir, ".account-txn-crash")
	if err := os.Mkdir(txn, 0o755); err != nil {
		t.Fatal(err)
	}
	want := &model.Account{Name: "alice", PasswordHash: "hash", Chars: []model.Char{validStoredChar("Alice", 999)}}
	b, err := json.MarshalIndent(want, "", "  ")
	if err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(txn, "alice.json"), b, 0o644); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(txn, "COMMIT"), []byte("commit\n"), 0o644); err != nil {
		t.Fatal(err)
	}

	s := NewJSONStore(dir)
	got, err := s.LoadAccount("alice")
	if err != nil {
		t.Fatal(err)
	}
	if got.Chars[0].Gold != 999 {
		t.Fatalf("gold recuperado=%d", got.Chars[0].Gold)
	}
	if _, err := os.Stat(txn); !os.IsNotExist(err) {
		t.Fatalf("journal recuperado nao foi removido: %v", err)
	}
}

func TestJSONStoreSaveIsAtomicAndReplaceable(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(dir)
	acc := &model.Account{Name: "felipe", PasswordHash: "hash", Chars: []model.Char{validStoredChar("felipe", 10)}}
	if err := s.SaveAccount(acc); err != nil {
		t.Fatal(err)
	}
	acc.Chars[0].Gold = 20
	if err := s.SaveAccount(acc); err != nil {
		t.Fatal(err)
	}
	loaded, err := s.LoadAccount("felipe")
	if err != nil {
		t.Fatal(err)
	}
	if loaded.Chars[0].Gold != 20 {
		t.Fatalf("gold=%d, quer 20", loaded.Chars[0].Gold)
	}
	entries, err := os.ReadDir(dir)
	if err != nil {
		t.Fatal(err)
	}
	if len(entries) != 1 || entries[0].Name() != "felipe.json" {
		t.Fatalf("arquivos residuais do save: %v", entries)
	}
}

func TestJSONStorePersistsAccountCargo(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(dir)
	acc := &model.Account{Name: "felipe", PasswordHash: "hash", CargoGold: 123456}
	acc.Cargo[0] = model.Item{Index: 4011, Eff: [6]byte{43, 9}}
	acc.Cargo[model.PlayerCargoSlots-1] = model.Item{Index: 4012}
	if err := s.SaveAccount(acc); err != nil {
		t.Fatal(err)
	}
	loaded, err := s.LoadAccount("felipe")
	if err != nil {
		t.Fatal(err)
	}
	if loaded.CargoGold != acc.CargoGold || loaded.Cargo[0] != acc.Cargo[0] ||
		loaded.Cargo[model.PlayerCargoSlots-1] != acc.Cargo[model.PlayerCargoSlots-1] {
		t.Fatalf("Cargo nao persistiu: gold=%d item0=%+v item119=%+v",
			loaded.CargoGold, loaded.Cargo[0], loaded.Cargo[model.PlayerCargoSlots-1])
	}
}

func TestJSONStoreRejectsReservedCargoSlots(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(dir)
	acc := &model.Account{Name: "felipe", PasswordHash: "hash"}
	acc.Cargo[model.PlayerCargoSlots] = model.Item{Index: 4011}
	if err := s.SaveAccount(acc); err == nil {
		t.Fatal("slot de cargo sem UI foi aceito")
	}
}

func TestJSONStoreCharacterNameExistsIsCaseInsensitive(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(dir)
	if err := s.SaveAccount(&model.Account{Name: "conta", PasswordHash: "hash",
		Chars: []model.Char{validStoredChar("MeuTK", 0)}}); err != nil {
		t.Fatal(err)
	}
	exists, err := s.CharacterNameExists("meutk")
	if err != nil {
		t.Fatal(err)
	}
	if !exists {
		t.Fatal("nome existente com caixa diferente nao foi encontrado")
	}
	exists, err = s.CharacterNameExists("Outro")
	if err != nil || exists {
		t.Fatalf("nome inexistente: exists=%v err=%v", exists, err)
	}
}

func TestJSONStoreCreateAccountNeverOverwritesAndLoadsCaseInsensitive(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(dir)
	first := &model.Account{Name: "MinhaConta", PasswordHash: "hash", Chars: []model.Char{}}
	if err := s.CreateAccount(first); err != nil {
		t.Fatal(err)
	}
	if err := s.CreateAccount(&model.Account{Name: "minhaconta", PasswordHash: "hash"}); !errors.Is(err, ErrAccountExists) {
		t.Fatalf("duplicata nao retornou ErrAccountExists: %v", err)
	}
	loaded, err := s.LoadAccount("MINHACONTA")
	if err != nil || loaded.Name != "MinhaConta" || loaded.PasswordHash != "hash" {
		t.Fatalf("load case-insensitive falhou: acc=%+v err=%v", loaded, err)
	}
}

func TestJSONStorePreservesEmptyCharacterSlotsAsNull(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(dir)
	acc := &model.Account{
		Name: "slots", PasswordHash: "hash",
		Chars: []model.Char{{}, validStoredChar("Terceiro", 10)},
	}
	if err := s.SaveAccount(acc); err != nil {
		t.Fatal(err)
	}
	raw, err := os.ReadFile(filepath.Join(dir, "slots.json"))
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Contains(raw, []byte(`"chars": [`+"\n"+`    null,`)) {
		t.Fatalf("slot vazio nao foi gravado como null: %s", raw)
	}
	loaded, err := s.LoadAccount("slots")
	if err != nil {
		t.Fatal(err)
	}
	if len(loaded.Chars) != 2 || !loaded.Chars[0].IsEmpty() ||
		loaded.Chars[1].Name != "Terceiro" {
		t.Fatalf("slots nao preservados: %+v", loaded.Chars)
	}
}

func TestJSONStoreRejectsLegacyPlaintextPassword(t *testing.T) {
	dir := t.TempDir()
	body := `{"name":"old","pass":"plaintext","passwordHash":"hash","chars":[]}`
	if err := os.WriteFile(filepath.Join(dir, "old.json"), []byte(body), 0o644); err != nil {
		t.Fatal(err)
	}
	if _, err := NewJSONStore(dir).LoadAccount("old"); err == nil {
		t.Fatal("conta com campo pass obsoleto foi aceita")
	}
}

func TestJSONStoreRejectsLegacyCharacterScore(t *testing.T) {
	dir := t.TempDir()
	body := `{
		"name":"old",
		"passwordHash":"hash",
		"chars":[{
			"name":"OldChar",
			"score":{"level":1},
			"equip":{},
			"inv":[]
		}]
	}`
	if err := os.WriteFile(filepath.Join(dir, "old.json"), []byte(body), 0o644); err != nil {
		t.Fatal(err)
	}
	if _, err := NewJSONStore(dir).LoadAccount("old"); err == nil {
		t.Fatal("personagem com campo score obsoleto foi aceito")
	}
}

func TestProjectAccountsUseCurrentSchema(t *testing.T) {
	dir := filepath.Join("..", "..", "data", "accounts")
	entries, err := os.ReadDir(dir)
	if os.IsNotExist(err) {
		t.Skip("project JSON account fixtures are not present in a clean checkout")
	}
	if err != nil {
		t.Fatal(err)
	}
	s := NewJSONStore(dir)
	loaded := 0
	for _, entry := range entries {
		if entry.IsDir() || filepath.Ext(entry.Name()) != ".json" {
			continue
		}
		name := entry.Name()[:len(entry.Name())-len(filepath.Ext(entry.Name()))]
		if _, err := s.LoadAccount(name); err != nil {
			t.Fatalf("%s fora do schema atual: %v", entry.Name(), err)
		}
		loaded++
	}
	if loaded == 0 {
		// data/accounts e um diretorio operacional opcional e pode existir vazio
		// antes da primeira conta; isso equivale ao clean checkout sem fixtures.
		t.Skip("project JSON account fixtures are not present")
	}
}
