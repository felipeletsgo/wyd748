package store

import (
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

// newTestStore monta um store com dir de contas e guilds.json irmao, que e o
// layout real (guilds.json NAO pode viver dentro do dir de contas, senao
// AccountNameExists/CharacterNameExists tentam parsea-lo como conta).
func newTestStore(t *testing.T) (*JSONStore, string, string) {
	t.Helper()
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	guilds := filepath.Join(root, "guilds.json")
	return NewJSONStore(accounts, WithGuildsPath(guilds)), accounts, guilds
}

func sampleGuild() *model.GuildRegistry {
	return &model.GuildRegistry{
		Version: model.GuildRegistryVersion,
		Guilds: []model.Guild{{
			ID:   1,
			Name: "Alfa",
			Members: []model.GuildMember{
				{Character: "Felipe", Account: "conta1", Rank: model.GuildRankLeader},
			},
		}},
	}
}

func TestDefaultGuildsPathFicaForaDoDirDeContas(t *testing.T) {
	got := defaultGuildsPath(filepath.Join("data", "accounts"))
	want := filepath.Join("data", "guilds.json")
	if got != want {
		t.Fatalf("guilds.json deveria ficar ao lado do dir de contas: got=%q want=%q", got, want)
	}
}

func TestSaveGameStatePersisteGuildEContaJuntas(t *testing.T) {
	s, accounts, guildsPath := newTestStore(t)
	acc := &model.Account{Name: "conta1", PasswordHash: "hash"}

	if err := s.SaveGameState(sampleGuild(), acc); err != nil {
		t.Fatalf("SaveGameState: %v", err)
	}
	if _, err := os.Stat(filepath.Join(accounts, "conta1.json")); err != nil {
		t.Fatalf("conta nao persistida: %v", err)
	}
	if _, err := os.Stat(guildsPath); err != nil {
		t.Fatalf("guilds.json nao persistido: %v", err)
	}
	// O guilds.json nao pode ter caido dentro do dir de contas.
	if _, err := os.Stat(filepath.Join(accounts, "guilds.json")); !os.IsNotExist(err) {
		t.Fatal("guilds.json vazou para dentro do diretorio de contas")
	}
	registry, err := s.LoadGuilds()
	if err != nil {
		t.Fatalf("LoadGuilds: %v", err)
	}
	if len(registry.Guilds) != 1 || registry.Guilds[0].Name != "Alfa" {
		t.Fatalf("registro relido incorreto: %+v", registry)
	}
}

func TestSaveGameStateFalhaNoGuildsTxtNaoInvalidaCommit(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	guildsPath := filepath.Join(root, "guilds.json")
	blocker := filepath.Join(root, "nao-e-diretorio")
	if err := os.WriteFile(blocker, []byte("bloqueio"), 0o644); err != nil {
		t.Fatal(err)
	}
	s := NewJSONStore(
		accounts,
		WithGuildsPath(guildsPath),
		WithGuildsTxtPath(filepath.Join(blocker, "Guilds.txt")),
	)
	acc := &model.Account{Name: "conta1", PasswordHash: "hash"}

	if err := s.SaveGameState(sampleGuild(), acc); err != nil {
		t.Fatalf("falha de artefato derivado nao pode negar commit autoritativo: %v", err)
	}
	if _, err := s.LoadAccount("conta1"); err != nil {
		t.Fatalf("conta comprometida nao foi persistida: %v", err)
	}
	registry, err := s.LoadGuilds()
	if err != nil {
		t.Fatalf("guild comprometida nao foi persistida: %v", err)
	}
	if len(registry.Guilds) != 1 || registry.Guilds[0].Name != "Alfa" {
		t.Fatalf("registro autoritativo incorreto apos falha do Guilds.txt: %+v", registry)
	}
}

func TestLoadGuildsSemArquivoDevolveRegistroVazio(t *testing.T) {
	s, _, _ := newTestStore(t)
	registry, err := s.LoadGuilds()
	if err != nil {
		t.Fatalf("servidor novo nao pode falhar por falta de guilds.json: %v", err)
	}
	if registry.Version != model.GuildRegistryVersion || len(registry.Guilds) != 0 {
		t.Fatalf("esperava registro vazio na versao corrente: %+v", registry)
	}
}

// writeJournal monta um journal a mao para simular queda do processo.
func writeJournal(t *testing.T, accounts string, manifest *txnManifest, files map[string][]byte, commit bool) string {
	t.Helper()
	txnDir, err := os.MkdirTemp(accounts, ".account-txn-")
	if err != nil {
		t.Fatal(err)
	}
	for name, data := range files {
		if err := os.WriteFile(filepath.Join(txnDir, name), data, 0o644); err != nil {
			t.Fatal(err)
		}
	}
	if manifest != nil {
		b, err := json.Marshal(manifest)
		if err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(filepath.Join(txnDir, txnManifestName), b, 0o644); err != nil {
			t.Fatal(err)
		}
	}
	if commit {
		if err := os.WriteFile(filepath.Join(txnDir, txnCommitName), []byte("commit\n"), 0o644); err != nil {
			t.Fatal(err)
		}
	}
	return txnDir
}

func TestRecuperacaoAplicaJournalComCommit(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	guildsPath := filepath.Join(root, "guilds.json")

	accBytes, _ := json.Marshal(&model.Account{Name: "conta1", PasswordHash: "hash"})
	guildBytes, _ := json.Marshal(sampleGuild())
	writeJournal(t, accounts, &txnManifest{
		Version: txnManifestVer,
		Entries: []txnEntry{
			{File: "conta1.json", Kind: txnKindAccount, Name: "conta1"},
			{File: "guilds.registry.json", Kind: txnKindGuilds},
		},
	}, map[string][]byte{
		"conta1.json":          accBytes,
		"guilds.registry.json": guildBytes,
	}, true)

	s := NewJSONStore(accounts, WithGuildsPath(guildsPath))
	if s.initErr != nil {
		t.Fatalf("recuperacao falhou: %v", s.initErr)
	}
	if _, err := os.Stat(filepath.Join(accounts, "conta1.json")); err != nil {
		t.Fatalf("conta nao recuperada: %v", err)
	}
	if _, err := os.Stat(guildsPath); err != nil {
		t.Fatalf("guilds.json nao recuperado: %v", err)
	}
}

func TestRecuperacaoDescartaJournalSemCommit(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	guildsPath := filepath.Join(root, "guilds.json")

	accBytes, _ := json.Marshal(&model.Account{Name: "conta1", PasswordHash: "hash"})
	txnDir := writeJournal(t, accounts, &txnManifest{
		Version: txnManifestVer,
		Entries: []txnEntry{{File: "conta1.json", Kind: txnKindAccount, Name: "conta1"}},
	}, map[string][]byte{"conta1.json": accBytes}, false)

	s := NewJSONStore(accounts, WithGuildsPath(guildsPath))
	if s.initErr != nil {
		t.Fatalf("journal sem COMMIT deve ser descartado sem erro: %v", s.initErr)
	}
	if _, err := os.Stat(filepath.Join(accounts, "conta1.json")); !os.IsNotExist(err) {
		t.Fatal("journal sem COMMIT nao pode ser aplicado")
	}
	if _, err := os.Stat(txnDir); !os.IsNotExist(err) {
		t.Fatal("journal sem COMMIT deveria ter sido removido")
	}
}

// Journal gravado antes do manifesto existir precisa continuar recuperavel.
func TestRecuperacaoDeJournalLegadoSemManifesto(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	accBytes, _ := json.Marshal(&model.Account{Name: "conta1", PasswordHash: "hash"})
	writeJournal(t, accounts, nil, map[string][]byte{"conta1.json": accBytes}, true)

	s := NewJSONStore(accounts, WithGuildsPath(filepath.Join(root, "guilds.json")))
	if s.initErr != nil {
		t.Fatalf("journal legado deve ser recuperado: %v", s.initErr)
	}
	if _, err := os.Stat(filepath.Join(accounts, "conta1.json")); err != nil {
		t.Fatalf("conta do journal legado nao recuperada: %v", err)
	}
}

func TestRecuperacaoRecusaTipoDesconhecido(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	writeJournal(t, accounts, &txnManifest{
		Version: txnManifestVer,
		Entries: []txnEntry{{File: "x.json", Kind: "coisa-nova", Name: "x"}},
	}, map[string][]byte{"x.json": []byte("{}")}, true)

	s := NewJSONStore(accounts, WithGuildsPath(filepath.Join(root, "guilds.json")))
	if s.initErr == nil {
		t.Fatal("tipo de destino desconhecido deveria falhar explicitamente")
	}
}

// O destino sai do TIPO, mas o nome ainda e usado para compor o caminho da
// conta: um nome com travessia precisa ser recusado antes de qualquer escrita.
func TestRecuperacaoRecusaCaminhoForaDaRaiz(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	for _, name := range []string{"../fora", "sub/conta", ".."} {
		writeJournal(t, accounts, &txnManifest{
			Version: txnManifestVer,
			Entries: []txnEntry{{File: "x.json", Kind: txnKindAccount, Name: name}},
		}, map[string][]byte{"x.json": []byte("{}")}, true)

		s := NewJSONStore(accounts, WithGuildsPath(filepath.Join(root, "guilds.json")))
		if s.initErr == nil {
			t.Fatalf("nome de conta %q deveria ser recusado", name)
		}
		if _, err := os.Stat(filepath.Join(root, "fora.json")); !os.IsNotExist(err) {
			t.Fatalf("escreveu fora da raiz com nome %q", name)
		}
		// limpa o journal invalido para o proximo caso
		entries, _ := os.ReadDir(accounts)
		for _, e := range entries {
			if e.IsDir() {
				_ = os.RemoveAll(filepath.Join(accounts, e.Name()))
			}
		}
	}
}

// Uma entrada invalida no meio do manifesto nao pode deixar as anteriores
// aplicadas: a resolucao acontece toda antes da primeira escrita.
func TestManifestoInvalidoNaoAplicaNadaParcialmente(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	accBytes, _ := json.Marshal(&model.Account{Name: "conta1", PasswordHash: "hash"})
	writeJournal(t, accounts, &txnManifest{
		Version: txnManifestVer,
		Entries: []txnEntry{
			{File: "conta1.json", Kind: txnKindAccount, Name: "conta1"},
			{File: "y.json", Kind: "invalido", Name: "y"},
		},
	}, map[string][]byte{"conta1.json": accBytes, "y.json": []byte("{}")}, true)

	s := NewJSONStore(accounts, WithGuildsPath(filepath.Join(root, "guilds.json")))
	if s.initErr == nil {
		t.Fatal("manifesto com entrada invalida deveria falhar")
	}
	if _, err := os.Stat(filepath.Join(accounts, "conta1.json")); !os.IsNotExist(err) {
		t.Fatal("nenhuma entrada podia ter sido aplicada")
	}
}

// O client 7.48 le "./Guilds.txt" localmente (o WYD.exe abre em "rb" e parseia
// com %d %d %d %s). Sem esse arquivo nenhum nome de guild aparece, entao a
// exportacao precisa sair junto de cada mudanca no registro.
func TestSaveGameStateExportaGuildsTxtNoFormatoNativo(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	txtPath := filepath.Join(root, "Guilds.txt")
	s := NewJSONStore(accounts,
		WithGuildsPath(filepath.Join(root, "guilds.json")),
		WithGuildsTxtPath(txtPath))

	if err := s.SaveGameState(sampleGuild()); err != nil {
		t.Fatalf("SaveGameState: %v", err)
	}
	data, err := os.ReadFile(txtPath)
	if err != nil {
		t.Fatalf("Guilds.txt nao exportado: %v", err)
	}
	// grupo=0 canal=0 indice=1 nome=Alfa
	if got := strings.TrimSpace(string(data)); got != "0 0 1 Alfa" {
		t.Fatalf("formato nativo incorreto: %q", got)
	}
}

// A exportacao e reescrita completa, nao append: dissolver guild precisa sumir
// do arquivo, senao o client continua exibindo nome de guild que nao existe.
func TestExportGuildsTxtReescreveOArquivoInteiro(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	txtPath := filepath.Join(root, "Guilds.txt")
	s := NewJSONStore(accounts,
		WithGuildsPath(filepath.Join(root, "guilds.json")),
		WithGuildsTxtPath(txtPath))

	duas := &model.GuildRegistry{Version: model.GuildRegistryVersion, Guilds: []model.Guild{
		{ID: 1, Name: "Alfa", Members: []model.GuildMember{
			{Character: "A", Account: "c1", Rank: model.GuildRankLeader}}},
		{ID: 2, Name: "Beta", Members: []model.GuildMember{
			{Character: "B", Account: "c2", Rank: model.GuildRankLeader}}},
	}}
	if err := s.SaveGameState(duas); err != nil {
		t.Fatal(err)
	}
	// dissolve a Beta
	duas.Guilds = duas.Guilds[:1]
	if err := s.SaveGameState(duas); err != nil {
		t.Fatal(err)
	}
	data, _ := os.ReadFile(txtPath)
	if strings.Contains(string(data), "Beta") {
		t.Fatalf("guild dissolvida continuou no Guilds.txt: %q", string(data))
	}
}

// Exportacao desligada nao pode quebrar a persistencia autoritativa.
func TestSaveGameStateFuncionaSemExportacao(t *testing.T) {
	s, _, _ := newTestStore(t) // sem WithGuildsTxtPath
	if err := s.SaveGameState(sampleGuild()); err != nil {
		t.Fatalf("exportacao desligada nao deveria afetar o save: %v", err)
	}
}

func TestSaveGameStateRecusaRegistroInvalido(t *testing.T) {
	s, _, _ := newTestStore(t)
	// duas guilds com o mesmo nome variando so a caixa
	registry := &model.GuildRegistry{
		Version: model.GuildRegistryVersion,
		Guilds: []model.Guild{
			{ID: 1, Name: "Alfa", Members: []model.GuildMember{
				{Character: "A", Account: "c1", Rank: model.GuildRankLeader}}},
			{ID: 2, Name: "ALFA", Members: []model.GuildMember{
				{Character: "B", Account: "c2", Rank: model.GuildRankLeader}}},
		},
	}
	if err := s.SaveGameState(registry); err == nil {
		t.Fatal("nome duplicado ignorando caixa deveria ser recusado")
	}
}
