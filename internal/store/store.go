// Package store abstrai a persistencia de contas/chars. O game so conhece a
// interface Store -- trocar JSON por MySQL depois nao toca na logica do jogo.
package store

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"sync"

	"wydgo/internal/model"
)

func decodeAccountJSON(data []byte, account *model.Account) error {
	decoder := json.NewDecoder(bytes.NewReader(data))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(account); err != nil {
		return err
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		if err == nil {
			return fmt.Errorf("conteudo JSON adicional")
		}
		return err
	}
	return nil
}

var ErrAccountExists = errors.New("store: conta ja existe")

// Store = persistencia de conta/char (impl inicial em JSON).
type Store interface {
	LoadAccount(name string) (*model.Account, error)
	SaveAccount(acc *model.Account) error
	CharacterNameExists(name string) (bool, error)
}

// CharacterNameExists garante a unicidade global do nome, sem confiar apenas
// nos personagens da conta que esta conectada.
func (s *JSONStore) CharacterNameExists(name string) (bool, error) {
	entries, err := os.ReadDir(s.dir)
	if err != nil {
		return false, err
	}
	for _, entry := range entries {
		if entry.IsDir() || !strings.EqualFold(filepath.Ext(entry.Name()), ".json") {
			continue
		}
		b, err := os.ReadFile(filepath.Join(s.dir, entry.Name()))
		if err != nil {
			return false, err
		}
		var acc model.Account
		if err := decodeAccountJSON(b, &acc); err != nil {
			return false, fmt.Errorf("store: verificar nomes em %q: %w", entry.Name(), err)
		}
		if err := acc.Validate(); err != nil {
			return false, fmt.Errorf("store: verificar nomes em %q: %w", entry.Name(), err)
		}
		for _, ch := range acc.Chars {
			if ch.Name != "" && strings.EqualFold(ch.Name, name) {
				return true, nil
			}
		}
	}
	return false, nil
}

// CharacterNames devolve todos os nomes de personagem (minusculos) presentes no
// disco. O World usa isto UMA vez no boot para manter um indice em memoria e nao
// varrer o diretorio inteiro de contas a cada criacao de personagem -- o vetor de
// DoS do 0x20F, que um cliente podia repetir sem gastar slot.
func (s *JSONStore) CharacterNames() (map[string]struct{}, error) {
	names := make(map[string]struct{})
	entries, err := os.ReadDir(s.dir)
	if err != nil {
		if os.IsNotExist(err) {
			return names, nil
		}
		return nil, err
	}
	for _, entry := range entries {
		if entry.IsDir() || !strings.EqualFold(filepath.Ext(entry.Name()), ".json") {
			continue
		}
		b, err := os.ReadFile(filepath.Join(s.dir, entry.Name()))
		if err != nil {
			return nil, err
		}
		var acc model.Account
		if err := decodeAccountJSON(b, &acc); err != nil {
			return nil, fmt.Errorf("store: indexar nomes em %q: %w", entry.Name(), err)
		}
		for _, ch := range acc.Chars {
			if ch.Name != "" {
				names[strings.ToLower(ch.Name)] = struct{}{}
			}
		}
	}
	return names, nil
}

func (s *JSONStore) AccountNameExists(name string) (bool, error) {
	entries, err := os.ReadDir(s.dir)
	if err != nil {
		if os.IsNotExist(err) {
			return false, nil
		}
		return false, err
	}
	for _, entry := range entries {
		if entry.IsDir() || !strings.EqualFold(filepath.Ext(entry.Name()), ".json") {
			continue
		}
		if strings.EqualFold(strings.TrimSuffix(entry.Name(), filepath.Ext(entry.Name())), name) {
			return true, nil
		}
	}
	return false, nil
}

// CreateAccount cria uma conta sem jamais substituir um arquivo existente.
// A checagem case-insensitive e feita pela camada account; O_EXCL fecha a
// janela de corrida para duas requisicoes simultaneas com a mesma grafia.
func (s *JSONStore) CreateAccount(acc *model.Account) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.initErr != nil {
		return s.initErr
	}
	if err := acc.Validate(); err != nil {
		return fmt.Errorf("store: criar conta: %w", err)
	}
	exists, err := s.AccountNameExists(acc.Name)
	if err != nil {
		return err
	}
	if exists {
		return ErrAccountExists
	}
	b, err := json.MarshalIndent(acc, "", "  ")
	if err != nil {
		return err
	}
	if err := os.MkdirAll(s.dir, 0o755); err != nil {
		return err
	}
	f, err := os.OpenFile(s.path(acc.Name), os.O_WRONLY|os.O_CREATE|os.O_EXCL, 0o644)
	if err != nil {
		if os.IsExist(err) {
			return ErrAccountExists
		}
		return err
	}
	path := f.Name()
	ok := false
	defer func() {
		f.Close()
		if !ok {
			os.Remove(path)
		}
	}()
	if _, err := f.Write(b); err != nil {
		return err
	}
	if err := f.Sync(); err != nil {
		return err
	}
	if err := f.Close(); err != nil {
		return err
	}
	ok = true
	return nil
}

// jsonStore guarda 1 arquivo JSON por conta em dir/<name>.json.
type JSONStore struct {
	dir string
	// guildsPath e o estado AUTORITATIVO (JSON, dentro da transacao).
	guildsPath string
	// guildsTxtPath e um artefato DERIVADO para o client 7.48, fora da
	// transacao. Vazio = exportacao desligada.
	guildsTxtPath string
	// charStateDir guarda o estado de SESSAO por personagem (buffs, moedas) num
	// arquivo separado, fora do dir de contas -- senao os varredores de conta
	// tentariam parsea-lo como conta. Default: pasta charstate/ irma do dir.
	charStateDir string
	// writeQueue leva o fsync do AUTOSAVE para uma goroutine dedicada, tirando o
	// bloqueio de disco do game-loop. Escritas SINCRONAS (anti-dupe) continuam
	// diretas, mas dao flush na fila antes para nao serem sobrescritas por um
	// autosave pendente mais VELHO -- a ordenacao e garantida porque o World e
	// single-goroutine (nunca enfileira autosave enquanto executa um save sincrono).
	writeQueue chan writeJob
	mu         sync.Mutex
	initErr    error
}

// writeJob e uma tarefa da goroutine de persistencia. run faz a escrita
// (autosave); done, quando presente, e fechado depois de run -- usado como
// barreira de flush pelos saves sincronos.
type writeJob struct {
	run  func()
	done chan struct{}
}

// persistLoop drena a fila de escritas de autosave em ordem FIFO. Uma unica
// goroutine consumidora garante que escritas para a mesma conta nunca se
// sobreponham fora de ordem.
func (s *JSONStore) persistLoop() {
	for job := range s.writeQueue {
		if job.run != nil {
			job.run()
		}
		if job.done != nil {
			close(job.done)
		}
	}
}

// flushWrites bloqueia ate a fila drenar tudo que ja foi enfileirado. Um save
// sincrono chama isto antes de escrever direto, garantindo que um autosave
// pendente (estado mais velho) nao sobrescreva o que ele acabou de gravar.
func (s *JSONStore) flushWrites() {
	if s.writeQueue == nil {
		return
	}
	done := make(chan struct{})
	s.writeQueue <- writeJob{done: done}
	<-done
}

// enqueueAsyncWrite agenda uma escrita fora do game-loop. Se a fila nao existir
// (impl legada), escreve na hora.
func (s *JSONStore) enqueueAsyncWrite(run func()) {
	if s.writeQueue == nil {
		run()
		return
	}
	s.writeQueue <- writeJob{run: run}
}

// Option configura o JSONStore sem quebrar os chamadores que so precisam de
// contas (account-api e account-create).
type Option func(*JSONStore)

// WithGuildsPath define onde vive o registro de guilds. O default fica ao LADO
// do diretorio de contas, nunca dentro dele: CharacterNameExists e
// AccountNameExists varrem todo *.json de s.dir e tentariam parsear o
// guilds.json como se fosse uma conta.
func WithGuildsPath(path string) Option {
	return func(s *JSONStore) { s.guildsPath = path }
}

// WithGuildsTxtPath liga a exportacao do Guilds.txt que o CLIENT 7.48 le
// localmente (o WYD.exe abre "./Guilds.txt"). Vazio desliga a exportacao.
//
// O arquivo e gravado junto da transacao, mas NAO faz parte dela: ele e um
// artefato derivado, para distribuir aos clients. Se a gravacao dele falhar, o
// estado autoritativo (guilds.json + contas) continua valido -- por isso a
// falha e reportada sem desfazer a transacao.
func WithGuildsTxtPath(path string) Option {
	return func(s *JSONStore) { s.guildsTxtPath = path }
}

func defaultGuildsPath(dir string) string {
	return filepath.Join(filepath.Dir(filepath.Clean(dir)), "guilds.json")
}

func defaultCharStateDir(dir string) string {
	return filepath.Join(filepath.Dir(filepath.Clean(dir)), "charstate")
}

// WithCharStatePath define a pasta do estado de sessao (buffs/moedas). Fica FORA
// do dir de contas pelo mesmo motivo do guilds.json.
func WithCharStatePath(path string) Option {
	return func(s *JSONStore) { s.charStateDir = path }
}

// NewJSONStore cria um Store baseado em arquivos JSON no diretorio dir.
func NewJSONStore(dir string, opts ...Option) *JSONStore {
	s := &JSONStore{dir: dir, guildsPath: defaultGuildsPath(dir),
		charStateDir: defaultCharStateDir(dir)}
	for _, opt := range opts {
		opt(s)
	}
	// Recuperacao de transacao roda ANTES da goroutine de escrita: no boot nao ha
	// concorrencia e os writes de recuperacao sao diretos.
	s.initErr = s.recoverAccountTransactions()
	s.writeQueue = make(chan writeJob, 256)
	go s.persistLoop()
	return s
}

func (s *JSONStore) path(name string) string {
	return filepath.Join(s.dir, name+".json")
}

func (s *JSONStore) accountPath(name string) (string, error) {
	exact := s.path(name)
	if _, err := os.Stat(exact); err == nil {
		return exact, nil
	} else if !os.IsNotExist(err) {
		return "", err
	}
	entries, err := os.ReadDir(s.dir)
	if err != nil {
		return "", err
	}
	wanted := name + ".json"
	for _, entry := range entries {
		if !entry.IsDir() && strings.EqualFold(entry.Name(), wanted) {
			return filepath.Join(s.dir, entry.Name()), nil
		}
	}
	return "", os.ErrNotExist
}

func (s *JSONStore) LoadAccount(name string) (*model.Account, error) {
	if s.initErr != nil {
		return nil, s.initErr
	}
	path, err := s.accountPath(name)
	if err != nil {
		return nil, err
	}
	b, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var acc model.Account
	if err := decodeAccountJSON(b, &acc); err != nil {
		return nil, fmt.Errorf("store: parse conta %q: %w", name, err)
	}
	if err := acc.Validate(); err != nil {
		return nil, fmt.Errorf("store: validar conta %q: %w", name, err)
	}
	return &acc, nil
}

// SaveAccount e o save SINCRONO (anti-dupe): confirma ao chamador so depois do
// fsync. Da flush na fila antes para nao ser sobrescrito por um autosave async
// pendente com estado mais velho.
func (s *JSONStore) SaveAccount(acc *model.Account) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.initErr != nil {
		return s.initErr
	}
	if err := acc.Validate(); err != nil {
		return fmt.Errorf("store: salvar conta: %w", err)
	}
	b, err := json.MarshalIndent(acc, "", "  ")
	if err != nil {
		return err
	}
	if err := os.MkdirAll(s.dir, 0o755); err != nil {
		return err
	}
	s.flushWrites()
	return s.writeAccountFile(s.path(acc.Name), b)
}

// SaveAccountAsync agenda a escrita FORA do game-loop (autosave periodico). O
// marshal/validacao acontecem AGORA (snapshot do estado atual, no game-loop); so
// o fsync sai para a goroutine de persistencia. Nao espera nem confirma: um
// autosave nao gate confirmacao de client (isso e papel do SaveAccount sincrono).
func (s *JSONStore) SaveAccountAsync(acc *model.Account) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.initErr != nil {
		return s.initErr
	}
	if err := acc.Validate(); err != nil {
		return fmt.Errorf("store: autosave conta: %w", err)
	}
	b, err := json.MarshalIndent(acc, "", "  ")
	if err != nil {
		return err
	}
	if err := os.MkdirAll(s.dir, 0o755); err != nil {
		return err
	}
	name, path := acc.Name, s.path(acc.Name)
	s.enqueueAsyncWrite(func() {
		if err := s.writeAccountFile(path, b); err != nil {
			log.Printf("store: autosave conta %q: %v", name, err)
		}
	})
	return nil
}

// Tipos de destino do journal. O apply resolve o caminho A PARTIR DO TIPO --
// nunca do nome do arquivo -- para que uma entrada nao consiga se disfarcar de
// outra (um guilds.json nao pode virar arquivo de conta, e vice-versa).
const (
	txnKindAccount = "account"
	txnKindGuilds  = "guilds"

	txnManifestName = "MANIFEST"
	txnCommitName   = "COMMIT"
	txnManifestVer  = 1
)

type txnEntry struct {
	File string `json:"file"`           // nome do arquivo DENTRO do journal
	Kind string `json:"kind"`           // account | guilds
	Name string `json:"name,omitempty"` // conta: nome; guilds: vazio
}

type txnManifest struct {
	Version int        `json:"version"`
	Entries []txnEntry `json:"entries"`
}

// SaveAccounts persiste varias contas como uma unica transacao recuperavel.
func (s *JSONStore) SaveAccounts(accounts ...*model.Account) error {
	return s.SaveGameState(nil, accounts...)
}

// SaveGameState persiste o registro de guilds e as contas afetadas como UMA
// unica transacao recuperavel. Guild e contas precisam cair juntas: salvar em
// arquivos independentes deixaria membro sem guild (ou guild com membro
// inexistente) se o processo morresse no meio.
//
// O diretorio de journal so recebe COMMIT depois que todos os JSONs estao
// completos e sincronizados. Se o processo cair entre os replaces, o proximo
// NewJSONStore reaplica todos os estados finais antes de aceitar login.
func (s *JSONStore) SaveGameState(guilds *model.GuildRegistry, accounts ...*model.Account) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.initErr != nil {
		return s.initErr
	}
	if guilds == nil && len(accounts) == 0 {
		return nil
	}
	if guilds != nil {
		if s.guildsPath == "" {
			return errors.New("store: caminho do guilds.json nao configurado")
		}
		if err := guilds.Validate(); err != nil {
			return fmt.Errorf("store: salvar guilds: %w", err)
		}
	}
	if err := os.MkdirAll(s.dir, 0o755); err != nil {
		return err
	}
	txnDir, err := os.MkdirTemp(s.dir, ".account-txn-")
	if err != nil {
		return err
	}
	committed := false
	defer func() {
		if !committed {
			_ = os.RemoveAll(txnDir)
		}
	}()

	manifest := txnManifest{Version: txnManifestVer}
	seen := make(map[string]struct{}, len(accounts))
	for _, acc := range accounts {
		if acc == nil || acc.Name == "" || filepath.Base(acc.Name) != acc.Name {
			return errors.New("store: conta invalida na transacao")
		}
		key := strings.ToLower(acc.Name)
		if _, duplicate := seen[key]; duplicate {
			return errors.New("store: conta duplicada na transacao")
		}
		if err := acc.Validate(); err != nil {
			return fmt.Errorf("store: salvar transacao: %w", err)
		}
		seen[key] = struct{}{}
		b, err := json.MarshalIndent(acc, "", "  ")
		if err != nil {
			return err
		}
		file := acc.Name + ".json"
		if err := writeSyncedFile(filepath.Join(txnDir, file), b, 0o644); err != nil {
			return err
		}
		manifest.Entries = append(manifest.Entries,
			txnEntry{File: file, Kind: txnKindAccount, Name: acc.Name})
	}
	if guilds != nil {
		b, err := json.MarshalIndent(guilds, "", "  ")
		if err != nil {
			return err
		}
		// Nome reservado dentro do journal; o destino real vem do TIPO.
		const file = "guilds.registry.json"
		if err := writeSyncedFile(filepath.Join(txnDir, file), b, 0o644); err != nil {
			return err
		}
		manifest.Entries = append(manifest.Entries, txnEntry{File: file, Kind: txnKindGuilds})
	}

	manifestBytes, err := json.MarshalIndent(manifest, "", "  ")
	if err != nil {
		return err
	}
	if err := writeSyncedFile(filepath.Join(txnDir, txnManifestName), manifestBytes, 0o644); err != nil {
		return err
	}
	if err := syncDirectory(txnDir); err != nil {
		return err
	}
	if err := writeSyncedFile(filepath.Join(txnDir, txnCommitName), []byte("commit\n"), 0o644); err != nil {
		return err
	}
	if err := syncDirectory(txnDir); err != nil {
		return err
	}
	committed = true
	// Flush antes de aplicar: um autosave async pendente (estado mais velho) nao
	// pode sobrescrever os arquivos que a transacao vai gravar.
	s.flushWrites()
	if err := s.applyTransaction(txnDir); err != nil {
		// O journal comprometido fica no disco para recuperacao no proximo boot.
		return err
	}
	// Artefato derivado, DEPOIS do commit: o Guilds.txt so existe para o client
	// 7.48 ler localmente. Falhar aqui nao invalida a transacao ja aplicada, e
	// o arquivo e regravado inteiro na proxima mudanca de guild.
	if guilds != nil {
		if err := s.exportGuildsTxt(guilds); err != nil {
			return fmt.Errorf("store: exportar Guilds.txt: %w", err)
		}
	}
	return nil
}

// exportGuildsTxt regrava o arquivo inteiro no formato nativo. E sempre uma
// reescrita completa (nao um append como no W2PP): assim dissolver ou renomear
// guild se reflete no arquivo, em vez de deixar linha orfa acumulando.
func (s *JSONStore) exportGuildsTxt(guilds *model.GuildRegistry) error {
	if s.guildsTxtPath == "" {
		return nil
	}
	// Instancia unica: grupo e canal 0, que casa com o WORD (canal<<12)|id
	// enviado no wire.
	return writeFileAtomic(s.guildsTxtPath, guilds.GuildsTxt(0, 0))
}

// LoadGuilds le o registro canonico. Ausencia de arquivo e estado valido
// (servidor novo): devolve um registro vazio na versao corrente.
func (s *JSONStore) LoadGuilds() (*model.GuildRegistry, error) {
	if s.initErr != nil {
		return nil, s.initErr
	}
	if s.guildsPath == "" {
		return nil, errors.New("store: caminho do guilds.json nao configurado")
	}
	b, err := os.ReadFile(s.guildsPath)
	if err != nil {
		if os.IsNotExist(err) {
			return &model.GuildRegistry{Version: model.GuildRegistryVersion}, nil
		}
		return nil, err
	}
	decoder := json.NewDecoder(bytes.NewReader(b))
	decoder.DisallowUnknownFields()
	var registry model.GuildRegistry
	if err := decoder.Decode(&registry); err != nil {
		return nil, fmt.Errorf("store: parse guilds: %w", err)
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		if err == nil {
			return nil, fmt.Errorf("store: parse guilds: conteudo JSON adicional")
		}
		return nil, err
	}
	if err := registry.Validate(); err != nil {
		return nil, fmt.Errorf("store: validar guilds: %w", err)
	}
	return &registry, nil
}

// writeFileAtomic grava em temporario no MESMO diretorio do destino e faz
// rename. Ficar no diretorio do destino mantem o rename dentro do mesmo
// filesystem, o que importa agora que o journal escreve fora do dir de contas
// (o guilds.json vive ao lado dele).
func writeFileAtomic(path string, b []byte) error {
	dir := filepath.Dir(path)
	if err := os.MkdirAll(dir, 0o755); err != nil {
		return err
	}
	tmp, err := os.CreateTemp(dir, "."+filepath.Base(path)+".tmp-")
	if err != nil {
		return err
	}
	tmpPath := tmp.Name()
	defer os.Remove(tmpPath)
	if err := tmp.Chmod(0o644); err != nil {
		tmp.Close()
		return err
	}
	if _, err := tmp.Write(b); err != nil {
		tmp.Close()
		return err
	}
	if err := tmp.Sync(); err != nil {
		tmp.Close()
		return err
	}
	if err := tmp.Close(); err != nil {
		return err
	}
	return os.Rename(tmpPath, path)
}

func (s *JSONStore) writeAccountFile(path string, b []byte) error {
	tmp, err := os.CreateTemp(s.dir, "."+filepath.Base(path)+".tmp-")
	if err != nil {
		return err
	}
	tmpPath := tmp.Name()
	defer os.Remove(tmpPath)
	if err := tmp.Chmod(0o644); err != nil {
		tmp.Close()
		return err
	}
	if _, err := tmp.Write(b); err != nil {
		tmp.Close()
		return err
	}
	if err := tmp.Sync(); err != nil {
		tmp.Close()
		return err
	}
	if err := tmp.Close(); err != nil {
		return err
	}
	return os.Rename(tmpPath, path)
}

func writeSyncedFile(path string, data []byte, mode os.FileMode) error {
	f, err := os.OpenFile(path, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, mode)
	if err != nil {
		return err
	}
	if _, err := f.Write(data); err != nil {
		f.Close()
		return err
	}
	if err := f.Sync(); err != nil {
		f.Close()
		return err
	}
	return f.Close()
}

func syncDirectory(path string) error {
	dir, err := os.Open(path)
	if err != nil {
		return err
	}
	defer dir.Close()
	err = dir.Sync()
	// Windows nao oferece fsync de diretorio pela mesma API. No Ubuntu esse
	// Sync garante a durabilidade das entradas antes/depois do marcador COMMIT.
	if runtime.GOOS == "windows" {
		return nil
	}
	return err
}

func (s *JSONStore) recoverAccountTransactions() error {
	entries, err := os.ReadDir(s.dir)
	if err != nil {
		if os.IsNotExist(err) {
			return nil
		}
		return err
	}
	for _, entry := range entries {
		if !entry.IsDir() || !strings.HasPrefix(entry.Name(), ".account-txn-") {
			continue
		}
		txnDir := filepath.Join(s.dir, entry.Name())
		if _, err := os.Stat(filepath.Join(txnDir, "COMMIT")); err != nil {
			if os.IsNotExist(err) {
				if err := os.RemoveAll(txnDir); err != nil {
					return err
				}
				continue
			}
			return err
		}
		if err := s.applyTransaction(txnDir); err != nil {
			return fmt.Errorf("store: recuperar transacao %q: %w", entry.Name(), err)
		}
	}
	return nil
}

// resolveTxnTarget devolve o caminho final de uma entrada A PARTIR DO SEU TIPO,
// validando que o resultado cai dentro da raiz esperada daquele tipo. Sem isso,
// um manifesto adulterado poderia escrever em qualquer lugar do disco.
func (s *JSONStore) resolveTxnTarget(entry txnEntry) (string, error) {
	// O nome do arquivo dentro do journal nunca pode escapar do journal.
	if !safePathElement(entry.File) {
		return "", fmt.Errorf("entrada de journal com arquivo invalido %q", entry.File)
	}
	switch entry.Kind {
	case txnKindAccount:
		if !safePathElement(entry.Name) {
			return "", fmt.Errorf("entrada de journal com conta invalida %q", entry.Name)
		}
		target := filepath.Join(s.dir, entry.Name+".json")
		if err := ensureWithin(s.dir, target); err != nil {
			return "", err
		}
		return target, nil
	case txnKindGuilds:
		if s.guildsPath == "" {
			return "", errors.New("caminho do guilds.json nao configurado")
		}
		return s.guildsPath, nil
	default:
		return "", fmt.Errorf("entrada de journal com tipo desconhecido %q", entry.Kind)
	}
}

// safePathElement aceita apenas um unico elemento de caminho comum. Recusa
// vazio, separadores e os elementos especiais "." e ".." -- este ultimo passa
// pelo filepath.Base (Base("..") == "..") e viraria o arquivo "...json" dentro
// do diretorio; nao escapa da raiz, mas nao e um nome de conta valido.
func safePathElement(name string) bool {
	if name == "" || name == "." || name == ".." {
		return false
	}
	return filepath.Base(name) == name && !strings.ContainsRune(name, '/') &&
		!strings.ContainsRune(name, '\\')
}

// ensureWithin recusa alvos que escapem da raiz (separadores, "..", symlink de
// nome). Mesma severidade do filepath.Base(acc.Name) != acc.Name ja aplicado na
// escrita da transacao.
func ensureWithin(root, target string) error {
	cleanRoot := filepath.Clean(root)
	cleanTarget := filepath.Clean(target)
	rel, err := filepath.Rel(cleanRoot, cleanTarget)
	if err != nil {
		return fmt.Errorf("alvo %q fora da raiz %q", target, root)
	}
	if rel == ".." || strings.HasPrefix(rel, ".."+string(filepath.Separator)) {
		return fmt.Errorf("alvo %q fora da raiz %q", target, root)
	}
	return nil
}

// readTxnManifest devolve (manifesto, true) quando o journal e tipado. Journal
// legado (sem MANIFEST) devolve false e e tratado como transacao so de contas,
// preservando a recuperacao de qualquer transacao em voo de versoes anteriores.
func readTxnManifest(txnDir string) (txnManifest, bool, error) {
	b, err := os.ReadFile(filepath.Join(txnDir, txnManifestName))
	if err != nil {
		if os.IsNotExist(err) {
			return txnManifest{}, false, nil
		}
		return txnManifest{}, false, err
	}
	var manifest txnManifest
	if err := json.Unmarshal(b, &manifest); err != nil {
		return txnManifest{}, false, fmt.Errorf("manifesto invalido: %w", err)
	}
	if manifest.Version != txnManifestVer {
		return txnManifest{}, false, fmt.Errorf("manifesto versao %d; esperado %d",
			manifest.Version, txnManifestVer)
	}
	return manifest, true, nil
}

func (s *JSONStore) applyTransaction(txnDir string) error {
	manifest, typed, err := readTxnManifest(txnDir)
	if err != nil {
		return err
	}
	if !typed {
		return s.applyLegacyAccountTransaction(txnDir)
	}

	// Resolver e validar TUDO antes de escrever qualquer coisa: um manifesto com
	// uma entrada invalida nao pode deixar o journal aplicado pela metade.
	type pendingWrite struct {
		target string
		data   []byte
	}
	writes := make([]pendingWrite, 0, len(manifest.Entries))
	for _, entry := range manifest.Entries {
		target, err := s.resolveTxnTarget(entry)
		if err != nil {
			return fmt.Errorf("store: %w", err)
		}
		data, err := os.ReadFile(filepath.Join(txnDir, entry.File))
		if err != nil {
			return err
		}
		writes = append(writes, pendingWrite{target: target, data: data})
	}
	for _, write := range writes {
		if err := writeFileAtomic(write.target, write.data); err != nil {
			return err
		}
	}
	if err := syncDirectory(s.dir); err != nil {
		return err
	}
	if err := os.RemoveAll(txnDir); err != nil {
		return err
	}
	return syncDirectory(s.dir)
}

// applyLegacyAccountTransaction e o comportamento anterior, mantido apenas para
// journals gravados antes do manifesto existir.
func (s *JSONStore) applyLegacyAccountTransaction(txnDir string) error {
	entries, err := os.ReadDir(txnDir)
	if err != nil {
		return err
	}
	for _, entry := range entries {
		if entry.IsDir() || !strings.EqualFold(filepath.Ext(entry.Name()), ".json") {
			continue
		}
		b, err := os.ReadFile(filepath.Join(txnDir, entry.Name()))
		if err != nil {
			return err
		}
		target := filepath.Join(s.dir, entry.Name())
		if err := ensureWithin(s.dir, target); err != nil {
			return fmt.Errorf("store: %w", err)
		}
		if err := s.writeAccountFile(target, b); err != nil {
			return err
		}
	}
	if err := syncDirectory(s.dir); err != nil {
		return err
	}
	if err := os.RemoveAll(txnDir); err != nil {
		return err
	}
	return syncDirectory(s.dir)
}
