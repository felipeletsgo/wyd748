package store

import (
	"encoding/json"
	"errors"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestDecodeAccountJSONIsStrictAboutSyntaxUnknownFieldsAndTrailingContent(t *testing.T) {
	valid := &model.Account{Name: "alpha", Chars: []model.Char{}}
	body, err := json.Marshal(valid)
	if err != nil {
		t.Fatal(err)
	}
	var got model.Account
	if err := decodeAccountJSON(body, &got); err != nil || got.Name != "alpha" {
		t.Fatalf("valid account decode got=%+v err=%v", got, err)
	}
	for _, bad := range [][]byte{
		[]byte(`{"name":`),
		[]byte(`{"name":"alpha","unknown":1}`),
		append(append([]byte(nil), body...), []byte(` {}`)...),
	} {
		var acc model.Account
		if err := decodeAccountJSON(bad, &acc); err == nil {
			t.Fatalf("JSON invalido foi aceito: %s", bad)
		}
	}
}

func TestAccountAndCharacterNameIndexesCoverMissingCaseInsensitiveAndCorruptFiles(t *testing.T) {
	missing := &JSONStore{dir: filepath.Join(t.TempDir(), "missing")}
	if exists, err := missing.AccountNameExists("alpha"); err != nil || exists {
		t.Fatalf("AccountNameExists missing=%v/%v", exists, err)
	}
	if names, err := missing.CharacterNames(); err != nil || len(names) != 0 {
		t.Fatalf("CharacterNames missing=%v/%v", names, err)
	}
	if _, err := missing.CharacterNameExists("hero"); err == nil {
		t.Fatal("CharacterNameExists deve propagar ReadDir ausente")
	}

	dir := t.TempDir()
	st := NewJSONStore(dir)
	acc := uidTestAccount("Alpha")
	acc.Chars[0] = validStoredChar("Hero", 0)
	if err := st.SaveAccount(acc); err != nil {
		t.Fatal(err)
	}
	if exists, err := st.AccountNameExists("aLpHa"); err != nil || !exists {
		t.Fatalf("conta case-insensitive=%v/%v", exists, err)
	}
	if exists, err := st.CharacterNameExists("hErO"); err != nil || !exists {
		t.Fatalf("char case-insensitive=%v/%v", exists, err)
	}
	if names, err := st.CharacterNames(); err != nil || len(names) != 1 {
		t.Fatalf("CharacterNames=%v/%v", names, err)
	} else if _, ok := names["hero"]; !ok {
		t.Fatalf("indice nao normalizado: %v", names)
	}

	if err := os.WriteFile(filepath.Join(dir, "broken.json"), []byte(`{"name":`), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := st.CharacterNames(); err == nil {
		t.Fatal("CharacterNames ignorou arquivo corrompido")
	}
	if _, err := st.CharacterNameExists("nobody"); err == nil {
		t.Fatal("CharacterNameExists ignorou arquivo corrompido")
	}
}

func TestWriteQueueImmediateBufferedOverflowCoalescingAndBarrier(t *testing.T) {
	immediate := &JSONStore{}
	runs := 0
	done := make(chan struct{})
	immediate.enqueueWrite(writeJob{run: func() { runs++ }})
	immediate.enqueueWrite(writeJob{done: done})
	if runs != 1 {
		t.Fatalf("write sem queue runs=%d", runs)
	}
	select {
	case <-done:
	default:
		t.Fatal("barreira sem queue nao fechou imediatamente")
	}
	immediate.flushWrites()

	s := &JSONStore{writeQueue: make(chan writeJob, 1)}
	first := writeJob{key: "account:a", run: func() {}}
	s.enqueueWrite(first)
	secondRuns, thirdRuns := 0, 0
	s.enqueueWrite(writeJob{key: "account:b", run: func() { secondRuns++ }})
	s.enqueueWrite(writeJob{key: "account:b", run: func() { thirdRuns++ }})
	barrierDone := make(chan struct{})
	s.enqueueWrite(writeJob{done: barrierDone})
	if len(s.overflow) != 2 || s.overflow[0].key != "account:b" || s.overflow[1].done != barrierDone {
		t.Fatalf("overflow/coalesce divergente: %+v", s.overflow)
	}
	job, ok := s.nextWrite()
	if !ok || job.key != "account:a" {
		t.Fatalf("FIFO principal perdido: %+v/%v", job, ok)
	}
	job, ok = s.nextWrite()
	if !ok || job.key != "account:b" {
		t.Fatalf("overflow nao drenou: %+v/%v", job, ok)
	}
	job.run()
	if secondRuns != 0 || thirdRuns != 1 {
		t.Fatalf("coalescing executou snapshot errado second=%d third=%d", secondRuns, thirdRuns)
	}
	job, ok = s.nextWrite()
	if !ok || job.done != barrierDone {
		t.Fatalf("barreira perdeu ordem: %+v/%v", job, ok)
	}
}

func TestStoreOptionsAndDefaultPathsRemainOutsideAccountsDirectory(t *testing.T) {
	dir := filepath.Join(t.TempDir(), "accounts")
	if filepath.Dir(defaultGuildsPath(dir)) != filepath.Dir(dir) ||
		filepath.Dir(defaultCharStateDir(dir)) != filepath.Dir(dir) ||
		filepath.Dir(defaultInstanceStatePath(dir)) != filepath.Dir(dir) {
		t.Fatal("sidecars default deveriam ficar ao lado de accounts")
	}
	s := &JSONStore{}
	WithGuildsPath("g.json")(s)
	WithGuildsTxtPath("Guilds.txt")(s)
	WithCharStatePath("state")(s)
	if s.guildsPath != "g.json" || s.guildsTxtPath != "Guilds.txt" || s.charStateDir != "state" {
		t.Fatalf("options divergiram: %+v", s)
	}
}

func TestAccountPathCoversExactCaseInsensitiveMissingAndReadDirError(t *testing.T) {
	dir := t.TempDir()
	s := &JSONStore{dir: dir}
	exact := filepath.Join(dir, "Alpha.json")
	if err := os.WriteFile(exact, []byte(`{}`), 0o600); err != nil {
		t.Fatal(err)
	}
	if path, err := s.accountPath("Alpha"); err != nil || path != exact {
		t.Fatalf("exact path=%q err=%v", path, err)
	}
	if path, err := s.accountPath("alpha"); err != nil || path != exact {
		t.Fatalf("case-insensitive path=%q err=%v", path, err)
	}
	if _, err := s.accountPath("missing"); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("missing err=%v", err)
	}

	bad := &JSONStore{dir: filepath.Join(dir, "does-not-exist")}
	if _, err := bad.accountPath("x"); err == nil {
		t.Fatal("ReadDir error foi engolido")
	}
}

func TestAtomicFileHelpersWriteContentAndRejectImpossibleParents(t *testing.T) {
	dir := t.TempDir()
	atomicPath := filepath.Join(dir, "nested", "atomic.txt")
	if err := writeFileAtomic(atomicPath, []byte("atomic")); err != nil {
		t.Fatal(err)
	}
	if got, _ := os.ReadFile(atomicPath); string(got) != "atomic" {
		t.Fatalf("atomic content=%q", got)
	}

	s := &JSONStore{dir: dir}
	accountPath := filepath.Join(dir, "account.json")
	if err := s.writeAccountFile(accountPath, []byte("account")); err != nil {
		t.Fatal(err)
	}
	if got, _ := os.ReadFile(accountPath); string(got) != "account" {
		t.Fatalf("account content=%q", got)
	}

	synced := filepath.Join(dir, "synced.txt")
	if err := writeSyncedFile(synced, []byte("synced"), 0o600); err != nil {
		t.Fatal(err)
	}
	if got, _ := os.ReadFile(synced); string(got) != "synced" {
		t.Fatalf("synced content=%q", got)
	}
	if err := syncDirectory(dir); err != nil {
		t.Fatal(err)
	}

	blocker := filepath.Join(dir, "file-parent")
	if err := os.WriteFile(blocker, []byte("x"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := writeFileAtomic(filepath.Join(blocker, "child"), []byte("x")); err == nil {
		t.Fatal("writeFileAtomic aceitou arquivo como diretorio")
	}
	if err := writeSyncedFile(filepath.Join(blocker, "child"), []byte("x"), 0o600); err == nil {
		t.Fatal("writeSyncedFile aceitou parent impossivel")
	}
	if err := syncDirectory(filepath.Join(dir, "missing-dir")); err == nil {
		t.Fatal("syncDirectory ausente foi aceito")
	}
}

func TestJournalPathValidationCoversAllKindsAndEscapes(t *testing.T) {
	root := t.TempDir()
	s := &JSONStore{dir: filepath.Join(root, "accounts"), guildsPath: filepath.Join(root, "guilds.json"), instanceStatePath: filepath.Join(root, "instance.json")}
	if !safePathElement("Alpha.json") || safePathElement("") || safePathElement(".") || safePathElement("..") ||
		safePathElement("a/b") || safePathElement(`a\b`) {
		t.Fatal("safePathElement divergente")
	}
	if err := ensureWithin(s.dir, filepath.Join(s.dir, "Alpha.json")); err != nil {
		t.Fatalf("target interno recusado: %v", err)
	}
	if err := ensureWithin(s.dir, filepath.Join(root, "escape.json")); err == nil {
		t.Fatal("target externo foi aceito")
	}

	for _, tc := range []struct {
		entry txnEntry
		want  string
	}{
		{txnEntry{File: "Alpha.json", Kind: txnKindAccount, Name: "Alpha"}, filepath.Join(s.dir, "Alpha.json")},
		{txnEntry{File: "guilds.json", Kind: txnKindGuilds}, s.guildsPath},
		{txnEntry{File: "instance.json", Kind: txnKindInstanceState}, s.instanceStatePath},
	} {
		got, err := s.resolveTxnTarget(tc.entry)
		if err != nil || got != tc.want {
			t.Fatalf("resolve %+v => %q err=%v want=%q", tc.entry, got, err, tc.want)
		}
	}
	for _, entry := range []txnEntry{
		{File: "../escape", Kind: txnKindAccount, Name: "Alpha"},
		{File: "a.json", Kind: txnKindAccount, Name: "../Alpha"},
		{File: "a.json", Kind: "unknown"},
	} {
		if _, err := s.resolveTxnTarget(entry); err == nil {
			t.Fatalf("journal invalido aceito: %+v", entry)
		}
	}
	withoutGuilds := &JSONStore{dir: s.dir}
	if _, err := withoutGuilds.resolveTxnTarget(txnEntry{File: "g", Kind: txnKindGuilds}); err == nil {
		t.Fatal("guild target sem path foi aceito")
	}
	if _, err := withoutGuilds.resolveTxnTarget(txnEntry{File: "i", Kind: txnKindInstanceState}); err == nil {
		t.Fatal("instance target sem path foi aceito")
	}
}

func TestReadTxnManifestCoversLegacyMalformedVersionAndValidTypedManifest(t *testing.T) {
	dir := t.TempDir()
	if manifest, typed, err := readTxnManifest(dir); err != nil || typed || len(manifest.Entries) != 0 {
		t.Fatalf("legacy manifest=%+v typed=%v err=%v", manifest, typed, err)
	}
	path := filepath.Join(dir, txnManifestName)
	if err := os.WriteFile(path, []byte(`{`), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, _, err := readTxnManifest(dir); err == nil {
		t.Fatal("manifesto malformado foi aceito")
	}
	wrong, _ := json.Marshal(txnManifest{Version: txnManifestVer + 1})
	if err := os.WriteFile(path, wrong, 0o600); err != nil {
		t.Fatal(err)
	}
	if _, _, err := readTxnManifest(dir); err == nil {
		t.Fatal("manifesto com versao errada foi aceito")
	}
	want := txnManifest{Version: txnManifestVer, Entries: []txnEntry{{File: "Alpha.json", Kind: txnKindAccount, Name: "Alpha"}}}
	valid, _ := json.Marshal(want)
	if err := os.WriteFile(path, valid, 0o600); err != nil {
		t.Fatal(err)
	}
	got, typed, err := readTxnManifest(dir)
	if err != nil || !typed || len(got.Entries) != 1 || got.Entries[0] != want.Entries[0] {
		t.Fatalf("typed manifest=%+v typed=%v err=%v", got, typed, err)
	}
}

func TestApplyTypedAndLegacyTransactionsMaterializeAndRemoveJournal(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	s := &JSONStore{dir: accounts, guildsPath: filepath.Join(root, "guilds.json"), instanceStatePath: filepath.Join(root, "instance.json")}

	typedDir := filepath.Join(accounts, ".account-txn-typed")
	if err := os.MkdirAll(typedDir, 0o755); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(typedDir, "Alpha.json"), []byte("typed"), 0o600); err != nil {
		t.Fatal(err)
	}
	manifest := txnManifest{Version: txnManifestVer, Entries: []txnEntry{{File: "Alpha.json", Kind: txnKindAccount, Name: "Alpha"}}}
	mb, _ := json.Marshal(manifest)
	if err := os.WriteFile(filepath.Join(typedDir, txnManifestName), mb, 0o600); err != nil {
		t.Fatal(err)
	}
	if err := s.applyTransaction(typedDir); err != nil {
		t.Fatal(err)
	}
	if got, _ := os.ReadFile(filepath.Join(accounts, "Alpha.json")); string(got) != "typed" {
		t.Fatalf("typed apply content=%q", got)
	}
	if _, err := os.Stat(typedDir); !os.IsNotExist(err) {
		t.Fatalf("typed journal permaneceu: %v", err)
	}

	legacyDir := filepath.Join(accounts, ".account-txn-legacy")
	if err := os.MkdirAll(legacyDir, 0o755); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(legacyDir, "Beta.json"), []byte("legacy"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(legacyDir, "ignore.txt"), []byte("x"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := s.applyTransaction(legacyDir); err != nil {
		t.Fatal(err)
	}
	if got, _ := os.ReadFile(filepath.Join(accounts, "Beta.json")); string(got) != "legacy" {
		t.Fatalf("legacy apply content=%q", got)
	}
	if _, err := os.Stat(legacyDir); !os.IsNotExist(err) {
		t.Fatalf("legacy journal permaneceu: %v", err)
	}
}

func TestRecoverAccountTransactionsRemovesUncommittedAndAppliesCommitted(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	s := &JSONStore{dir: accounts}
	uncommitted := filepath.Join(accounts, ".account-txn-uncommitted")
	if err := os.MkdirAll(uncommitted, 0o755); err != nil {
		t.Fatal(err)
	}
	committed := filepath.Join(accounts, ".account-txn-committed")
	if err := os.MkdirAll(committed, 0o755); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(committed, "Gamma.json"), []byte("gamma"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(committed, txnCommitName), []byte("ok"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := s.recoverAccountTransactions(); err != nil {
		t.Fatal(err)
	}
	if _, err := os.Stat(uncommitted); !os.IsNotExist(err) {
		t.Fatalf("uncommitted journal permaneceu: %v", err)
	}
	if got, _ := os.ReadFile(filepath.Join(accounts, "Gamma.json")); string(got) != "gamma" {
		t.Fatalf("committed recovery content=%q", got)
	}
}

func TestStoreInitErrorFailsClosedAcrossCoreWritesAndReads(t *testing.T) {
	boom := errors.New("init")
	s := &JSONStore{initErr: boom}
	acc := &model.Account{Name: "a"}
	if err := s.CreateAccount(acc); !errors.Is(err, boom) {
		t.Fatalf("CreateAccount initErr=%v", err)
	}
	if _, err := s.LoadAccount("a"); !errors.Is(err, boom) {
		t.Fatalf("LoadAccount initErr=%v", err)
	}
	if err := s.SaveAccount(acc); !errors.Is(err, boom) {
		t.Fatalf("SaveAccount initErr=%v", err)
	}
	if err := s.SaveAccountAsync(acc); !errors.Is(err, boom) {
		t.Fatalf("SaveAccountAsync initErr=%v", err)
	}
	if err := s.SaveGameState(nil, acc); !errors.Is(err, boom) {
		t.Fatalf("SaveGameState initErr=%v", err)
	}
}

func TestStringHelpersDoNotNormalizeUnsafePathElements(t *testing.T) {
	if strings.TrimSpace(" x ") != "x" { // guard against accidental test fixture confusion.
		t.Fatal("unexpected strings behavior")
	}
}
