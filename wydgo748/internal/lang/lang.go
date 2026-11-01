// Package lang loads the server message catalogs and resolves player-facing
// text in the language of the receiving channel.
//
// The format follows the native server Language.txt table: one message per
// line, a key, one TAB and the text. The native table keyed rows by number
// (_NN_ names in the W2PP Language.h); this catalog keys rows by name and
// reuses the native _NN_ names where a message has a native equivalent.
// Text uses Go fmt verbs. Files are UTF-8; Text returns Windows-1252 bytes,
// the code page the 7.48 client renders.
package lang

import (
	"bufio"
	"bytes"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strings"
	"sync"

	"golang.org/x/text/encoding/charmap"
)

// Language selects a message catalog.
type Language uint8

const (
	English Language = iota
	Portuguese
	languageCount
)

// DefaultDir is the catalog directory relative to the server working directory.
const DefaultDir = "data/content/language"

// MaxTextBytes is the native message-row capacity (128 bytes with NUL).
const MaxTextBytes = 127

var languageFiles = [languageCount]string{English: "en.txt", Portuguese: "pt.txt"}

// Code returns the configuration code of the language.
func (l Language) Code() string {
	if l == Portuguese {
		return "pt"
	}
	return "en"
}

// Parse converts a configuration code to a language.
func Parse(code string) (Language, error) {
	switch strings.ToLower(strings.TrimSpace(code)) {
	case "en":
		return English, nil
	case "pt":
		return Portuguese, nil
	}
	return English, fmt.Errorf("unknown language %q (use en or pt)", code)
}

// ForChannel returns the source default for a channel: channel 1 serves
// Brazilian Portuguese players and every other channel serves English.
func ForChannel(channel byte) Language {
	if channel == 1 {
		return Portuguese
	}
	return English
}

// Catalog holds the Windows-1252 text of each language by key.
type Catalog struct {
	text [languageCount]map[string]string
	// byEnglish maps an English text without format verbs to its key.
	byEnglish map[string]string
	// templates match formatted English text, longest literal text first.
	templates []template
}

type template struct {
	key     string
	pattern *regexp.Regexp
}

var (
	keyPattern  = regexp.MustCompile(`^[A-Za-z0-9_]+$`)
	verbPattern = regexp.MustCompile(`%[-+# 0]*[0-9]*(?:\.[0-9]+)?[a-zA-Z%]`)
)

// Load reads and validates en.txt and pt.txt from dir. English is required
// and complete; Portuguese may omit keys, which then fall back to English.
func Load(dir string) (*Catalog, error) {
	c := &Catalog{}
	for l := Language(0); l < languageCount; l++ {
		path := filepath.Join(dir, languageFiles[l])
		rows, err := parseFile(path)
		if err != nil {
			return nil, err
		}
		c.text[l] = rows
	}
	if len(c.text[English]) == 0 {
		return nil, fmt.Errorf("%s: no messages", filepath.Join(dir, languageFiles[English]))
	}
	c.byEnglish = make(map[string]string, len(c.text[English]))
	for key, text := range c.text[English] {
		if len(Verbs(text)) > 0 || strings.Contains(text, "%%") {
			if pattern := templatePattern(text); pattern != nil {
				c.templates = append(c.templates, template{key: key, pattern: pattern})
			}
			continue
		}
		if previous, dup := c.byEnglish[text]; !dup || key < previous {
			c.byEnglish[text] = key
		}
	}
	sort.Slice(c.templates, func(i, j int) bool {
		li, lj := literalLength(c.text[English][c.templates[i].key]), literalLength(c.text[English][c.templates[j].key])
		if li != lj {
			return li > lj
		}
		return c.templates[i].key < c.templates[j].key
	})
	for key, text := range c.text[Portuguese] {
		english, ok := c.text[English][key]
		if !ok {
			return nil, fmt.Errorf("%s: key %s is not in %s", languageFiles[Portuguese], key, languageFiles[English])
		}
		if !sameVerbs(english, text) {
			return nil, fmt.Errorf("%s: key %s has format verbs %q, English has %q",
				languageFiles[Portuguese], key, Verbs(text), Verbs(english))
		}
	}
	return c, nil
}

func parseFile(path string) (map[string]string, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	data = bytes.TrimPrefix(data, []byte("\xef\xbb\xbf"))
	rows := map[string]string{}
	encoder := charmap.Windows1252.NewEncoder()
	scanner := bufio.NewScanner(bytes.NewReader(data))
	for line := 1; scanner.Scan(); line++ {
		raw := strings.TrimRight(scanner.Text(), "\r")
		if strings.TrimSpace(raw) == "" || strings.HasPrefix(raw, "#") {
			continue
		}
		key, text, ok := strings.Cut(raw, "\t")
		if !ok || !keyPattern.MatchString(key) {
			return nil, fmt.Errorf("%s:%d: expected <key><TAB><text>", path, line)
		}
		if strings.TrimSpace(text) == "" || strings.ContainsAny(text, "\t") {
			return nil, fmt.Errorf("%s:%d: key %s has empty text or another TAB", path, line, key)
		}
		if _, dup := rows[key]; dup {
			return nil, fmt.Errorf("%s:%d: duplicate key %s", path, line, key)
		}
		encoded, err := encoder.String(text)
		if err != nil {
			return nil, fmt.Errorf("%s:%d: key %s is not Windows-1252 text: %w", path, line, key, err)
		}
		if len(encoded) > MaxTextBytes {
			return nil, fmt.Errorf("%s:%d: key %s has %d bytes, limit %d", path, line, key, len(encoded), MaxTextBytes)
		}
		rows[key] = encoded
	}
	if err := scanner.Err(); err != nil {
		return nil, fmt.Errorf("%s: %w", path, err)
	}
	return rows, nil
}

// Text formats the message key in language l. A key absent from l uses the
// English text; a key absent from both returns the key itself.
func (c *Catalog) Text(l Language, key string, args ...any) string {
	format, ok := c.lookup(l, key)
	if !ok {
		return key
	}
	if len(args) == 0 {
		return format
	}
	return fmt.Sprintf(format, args...)
}

func (c *Catalog) lookup(l Language, key string) (string, bool) {
	if c == nil {
		return "", false
	}
	if l < languageCount {
		if text, ok := c.text[l][key]; ok {
			return text, true
		}
	}
	text, ok := c.text[English][key]
	return text, ok
}

// Translate returns text in language l when text is the English text of a
// catalog message: an exact fixed message, or a formatted message whose
// values replace the verbs of the language l text in order. Other text is
// unchanged. It localizes messages that reach a send point as built text.
func (c *Catalog) Translate(l Language, text string) string {
	if c == nil || l == English {
		return text
	}
	if key, ok := c.byEnglish[text]; ok {
		return c.Text(l, key)
	}
	for _, t := range c.templates {
		match := t.pattern.FindStringSubmatch(text)
		if match == nil {
			continue
		}
		target, ok := c.lookup(l, t.key)
		if !ok {
			return text
		}
		return fill(target, match[1:])
	}
	return text
}

// templatePattern compiles an English format into an anchored pattern with
// one group per verb. Numeric verbs match digits; other verbs match any text.
func templatePattern(format string) *regexp.Regexp {
	var b strings.Builder
	b.WriteString(`^`)
	last := 0
	for _, loc := range verbPattern.FindAllStringIndex(format, -1) {
		b.WriteString(regexp.QuoteMeta(format[last:loc[0]]))
		verb := format[loc[0]:loc[1]]
		switch verb[len(verb)-1] {
		case '%':
			b.WriteString(`%`)
		case 'd':
			b.WriteString(`([-+]?[0-9]+)`)
		case 'c':
			b.WriteString(`(.)`)
		default:
			b.WriteString(`(.+?)`)
		}
		last = loc[1]
	}
	b.WriteString(regexp.QuoteMeta(format[last:]))
	b.WriteString(`$`)
	pattern, err := regexp.Compile(b.String())
	if err != nil {
		return nil
	}
	return pattern
}

// fill replaces the verbs of format, in order, with already formatted values.
func fill(format string, values []string) string {
	var b strings.Builder
	last, next := 0, 0
	for _, loc := range verbPattern.FindAllStringIndex(format, -1) {
		b.WriteString(format[last:loc[0]])
		if format[loc[0]:loc[1]] == "%%" {
			b.WriteByte('%')
		} else if next < len(values) {
			b.WriteString(values[next])
			next++
		}
		last = loc[1]
	}
	b.WriteString(format[last:])
	return b.String()
}

func literalLength(format string) int {
	return len(verbPattern.ReplaceAllString(format, ""))
}

// Covers reports whether Translate recognizes text as a catalog message.
func (c *Catalog) Covers(text string) bool {
	if c == nil {
		return false
	}
	if _, ok := c.byEnglish[text]; ok {
		return true
	}
	for _, t := range c.templates {
		if t.pattern.MatchString(text) {
			return true
		}
	}
	return false
}

// Has reports whether the English catalog defines key.
func (c *Catalog) Has(key string) bool {
	_, ok := c.lookup(English, key)
	return ok
}

// Keys returns the keys defined for language l.
func (c *Catalog) Keys(l Language) []string {
	if c == nil || l >= languageCount {
		return nil
	}
	keys := make([]string, 0, len(c.text[l]))
	for key := range c.text[l] {
		keys = append(keys, key)
	}
	return keys
}

// Raw returns the unformatted Windows-1252 text of key in language l only.
func (c *Catalog) Raw(l Language, key string) (string, bool) {
	if c == nil || l >= languageCount {
		return "", false
	}
	text, ok := c.text[l][key]
	return text, ok
}

// Verbs returns the fmt verbs of text in order, excluding the %% escape.
func Verbs(text string) []string {
	var verbs []string
	for _, verb := range verbPattern.FindAllString(text, -1) {
		if verb != "%%" {
			verbs = append(verbs, verb)
		}
	}
	return verbs
}

func sameVerbs(a, b string) bool {
	va, vb := Verbs(a), Verbs(b)
	if len(va) != len(vb) {
		return false
	}
	for i := range va {
		if va[i] != vb[i] {
			return false
		}
	}
	return true
}

// Encode converts UTF-8 text to Windows-1252, replacing unsupported runes
// with '?'. Use it for text that does not come from the catalog.
func Encode(text string) string {
	var out strings.Builder
	encoder := charmap.Windows1252.NewEncoder()
	for _, r := range text {
		encoded, err := encoder.String(string(r))
		if err != nil {
			out.WriteByte('?')
			continue
		}
		out.WriteString(encoded)
	}
	return out.String()
}

var (
	defaultOnce    sync.Once
	defaultCatalog *Catalog
)

// Default returns the catalog in DefaultDir, found from the working directory
// or one of its parents. Servers load their configured catalog explicitly;
// this fallback serves worlds built without one, such as package tests.
// It returns nil when no valid catalog is found.
func Default() *Catalog {
	defaultOnce.Do(func() {
		dir, err := os.Getwd()
		if err != nil {
			return
		}
		for {
			candidate := filepath.Join(dir, filepath.FromSlash(DefaultDir))
			if _, statErr := os.Stat(filepath.Join(candidate, languageFiles[English])); statErr == nil {
				if catalog, loadErr := Load(candidate); loadErr == nil {
					defaultCatalog = catalog
				}
				return
			}
			parent := filepath.Dir(dir)
			if parent == dir {
				return
			}
			dir = parent
		}
	})
	return defaultCatalog
}
