// Package diagnostics fornece observabilidade local do client Go sem alterar
// contratos de protocolo, estado ou render. Falhas de I/O desabilitam o
// gravador, mas nunca devem impedir o client de iniciar ou continuar rodando.
package diagnostics

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"time"
)

type Field struct {
	Key   string
	Value any
}

func F(key string, value any) Field {
	return Field{Key: key, Value: value}
}

type Recorder struct {
	mu      sync.Mutex
	root    string
	dumpDir string
	log     *os.File
	initErr error
	seq     uint64
	closed  bool
}

// Open cria client.log e dumps/ dentro de root. O retorno nunca é nil: quando
// o diretório não pode ser criado, o Recorder fica desabilitado e InitError
// preserva somente a causa para diagnóstico opcional do chamador.
func Open(root string) *Recorder {
	if root == "" {
		root = "."
	}
	r := &Recorder{root: root, dumpDir: filepath.Join(root, "dumps")}
	if err := os.MkdirAll(r.dumpDir, 0o755); err != nil {
		r.initErr = err
		return r
	}
	logFile, err := os.OpenFile(filepath.Join(root, "client.log"), os.O_CREATE|os.O_APPEND|os.O_WRONLY, 0o644)
	if err != nil {
		r.initErr = err
		return r
	}
	r.log = logFile
	r.Event("diagnostics_started", F("root", root))
	return r
}

func (r *Recorder) InitError() error {
	if r == nil {
		return nil
	}
	r.mu.Lock()
	defer r.mu.Unlock()
	return r.initErr
}

func (r *Recorder) Enabled() bool {
	if r == nil {
		return false
	}
	r.mu.Lock()
	defer r.mu.Unlock()
	return !r.closed && r.log != nil
}

// Event registra um evento em uma única linha. Campos cuja chave sugira
// credencial são sempre substituídos por <redacted>; senhas e SecretCode não
// devem aparecer no client.log mesmo se um chamador os passar por engano.
func (r *Recorder) Event(name string, fields ...Field) {
	if r == nil {
		return
	}
	r.mu.Lock()
	defer r.mu.Unlock()
	if r.closed || r.log == nil {
		return
	}
	r.writeEventLocked(name, fields...)
}

// DumpFrame preserva exatamente os bytes fornecidos e retorna somente o nome
// do arquivo criado. Erros de escrita são registrados, quando possível, e não
// são propagados ao fluxo do client.
func (r *Recorder) DumpFrame(direction string, opcode uint16, frame []byte) string {
	if r == nil {
		return ""
	}
	r.mu.Lock()
	defer r.mu.Unlock()
	if r.closed || r.log == nil {
		return ""
	}
	r.seq++
	direction = strings.ToLower(strings.TrimSpace(direction))
	if direction != "tx" && direction != "rx" {
		direction = "data"
	}
	name := fmt.Sprintf("%s-%06d-op%04X-size%d.bin", direction, r.seq, opcode, len(frame))
	path := filepath.Join(r.dumpDir, name)
	if err := os.WriteFile(path, frame, 0o644); err != nil {
		r.writeEventLocked("dump_error", F("file", name), F("error", err))
		return ""
	}
	return name
}

func (r *Recorder) Close() error {
	if r == nil {
		return nil
	}
	r.mu.Lock()
	defer r.mu.Unlock()
	if r.closed {
		return nil
	}
	r.closed = true
	if r.log == nil {
		return nil
	}
	err := r.log.Close()
	r.log = nil
	return err
}

func (r *Recorder) writeEventLocked(name string, fields ...Field) {
	stamp := time.Now().Format(time.RFC3339Nano)
	_, _ = fmt.Fprintf(r.log, "%s event=%s", stamp, sanitizeToken(name))
	for _, field := range fields {
		key := sanitizeToken(field.Key)
		value := field.Value
		if isSensitiveKey(key) {
			value = "<redacted>"
		}
		_, _ = fmt.Fprintf(r.log, " %s=%q", key, fmt.Sprint(value))
	}
	_, _ = fmt.Fprintln(r.log)
}

func isSensitiveKey(key string) bool {
	key = strings.ToLower(key)
	for _, fragment := range []string{"password", "passwd", "secret", "token", "credential"} {
		if strings.Contains(key, fragment) {
			return true
		}
	}
	return false
}

func sanitizeToken(value string) string {
	value = strings.TrimSpace(value)
	if value == "" {
		return "unknown"
	}
	return strings.Map(func(r rune) rune {
		if r >= 'a' && r <= 'z' || r >= 'A' && r <= 'Z' || r >= '0' && r <= '9' || r == '_' || r == '-' || r == '.' {
			return r
		}
		return '_'
	}, value)
}
