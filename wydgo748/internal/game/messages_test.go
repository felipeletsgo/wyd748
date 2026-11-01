package game

import (
	"encoding/json"
	"go/ast"
	"go/parser"
	"go/token"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"testing"

	"golang.org/x/text/encoding/charmap"

	"wydgo/internal/lang"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func portugueseText(t *testing.T, text string) string {
	t.Helper()
	encoded, err := charmap.Windows1252.NewEncoder().String(text)
	if err != nil {
		t.Fatal(err)
	}
	return encoded
}

// Every message key written in game source must exist in the catalog files,
// and every catalog key must still have a sender or a matched English text.
func TestSourceMessageKeysExistInTheCatalog(t *testing.T) {
	catalog, err := lang.Load(filepath.Join("..", "..", lang.DefaultDir))
	if err != nil {
		t.Fatal(err)
	}
	files, err := filepath.Glob("*.go")
	if err != nil {
		t.Fatal(err)
	}
	keyLiteral := regexp.MustCompile(`"(_[A-Z]{2}_[A-Za-z0-9_]+)"`)
	used := map[string]bool{}
	for _, file := range files {
		if strings.HasSuffix(file, "_test.go") {
			continue
		}
		data, err := os.ReadFile(file)
		if err != nil {
			t.Fatal(err)
		}
		for _, match := range keyLiteral.FindAllStringSubmatch(string(data), -1) {
			used[match[1]] = true
			if !catalog.Has(match[1]) {
				t.Errorf("%s uses message key %s, absent from en.txt", file, match[1])
			}
		}
	}
	if len(used) < 400 {
		t.Fatalf("found only %d message keys in source", len(used))
	}
}

func TestPortugueseWorldLocalizesKeyedAndBuiltMessages(t *testing.T) {
	pt := &World{language: lang.Portuguese}
	en := &World{}
	if got := pt.msg("_NN_Not_Enough_Money"); got != portugueseText(t, "Você não tem gold suficiente.") {
		t.Fatalf("Portuguese keyed message = %q", got)
	}
	if got := en.msg("_NN_Not_Enough_Money"); got != "You do not have enough gold." {
		t.Fatalf("English keyed message = %q", got)
	}
	if got := pt.msg("_DN_Need_D_Sapphire", 30); got != portugueseText(t, "Você precisa de 30 Safiras.") {
		t.Fatalf("Portuguese formatted message = %q", got)
	}
	// Rejections and helpers still build English text; the send point localizes it.
	built := map[string]string{
		"This instance is full.":                      "Esta instância está cheia.",
		"Guild Moon declared war on your guild.":      "A guilda Moon declarou guerra à sua guilda.",
		"The mount reached level 12.":                 "A montaria atingiu o nível 12.",
		"This Ancient item requires four Coral gems.": "Este item Ancião exige quatro gemas Coral.",
	}
	for english, want := range built {
		if got := pt.tr(english); got != portugueseText(t, want) {
			t.Errorf("tr(%q) = %q, want %q", english, got, want)
		}
		if got := en.tr(english); got != english {
			t.Errorf("English tr(%q) = %q", english, got)
		}
	}
	if got := pt.tr("hello from a player"); got != "hello from a player" {
		t.Fatalf("player text changed: %q", got)
	}
	if got := pt.kingdomName(model.KingdomAkelonia); got != "Akelonia" {
		t.Fatalf("kingdom name = %q", got)
	}
	if got := pt.kingdomName(0); got != "Neutro" || en.kingdomName(0) != "Neutral" {
		t.Fatalf("neutral kingdom = %q / %q", got, en.kingdomName(0))
	}
}

func TestShutdownNoticeUsesTheChannelLanguage(t *testing.T) {
	pt := &World{language: lang.Portuguese}
	en := &World{}
	if got := en.shutdownNotice("shutdown", 5*60e9); got != "Server maintenance in 5 minutes. Please log out safely." {
		t.Fatalf("English notice = %q", got)
	}
	want := portugueseText(t, "Manutenção do servidor em 5 minutos. Saia do jogo com segurança.")
	if got := pt.shutdownNotice("shutdown", 5*60e9); got != want {
		t.Fatalf("Portuguese notice = %q", got)
	}
}

// Content text reaches players through w.tr, so each text needs a catalog row.
func TestContentTextHasCatalogTranslations(t *testing.T) {
	catalog, err := lang.Load(filepath.Join("..", "..", lang.DefaultDir))
	if err != nil {
		t.Fatal(err)
	}
	var texts []string
	var quests struct {
		Quests []struct {
			Messages map[string]string `json:"messages"`
			Dialogue []string          `json:"dialogue"`
		} `json:"quests"`
	}
	readJSON(t, "quests.json", &quests)
	for _, quest := range quests.Quests {
		for _, message := range quest.Messages {
			texts = append(texts, message)
		}
		texts = append(texts, quest.Dialogue...)
	}
	type stage struct {
		Name string `json:"name"`
		Quiz *struct {
			Question string `json:"question"`
		} `json:"quiz"`
	}
	type instance struct {
		Name   string  `json:"name"`
		Stages []stage `json:"stages"`
	}
	var instances struct {
		Instances map[string]instance `json:"instances"`
		Items     map[string]struct {
			Instance *instance `json:"instance"`
		} `json:"items"`
	}
	readJSON(t, "instances.json", &instances)
	addInstance := func(i instance) {
		texts = append(texts, i.Name)
		for _, s := range i.Stages {
			texts = append(texts, s.Name)
			if s.Quiz != nil {
				texts = append(texts, strings.ReplaceAll(s.Quiz.Question, "_", " "))
			}
		}
	}
	for _, i := range instances.Instances {
		addInstance(i)
	}
	for _, item := range instances.Items {
		if item.Instance != nil {
			addInstance(*item.Instance)
		}
	}
	bossFiles, err := filepath.Glob(filepath.Join("..", "..", "data", "content", "boss", "*.lua"))
	if err != nil || len(bossFiles) == 0 {
		t.Fatalf("boss files: %v", err)
	}
	luaMessage := regexp.MustCompile(`(?:message|spawn_message|death_message)\s*=\s*"([^"]*)"`)
	for _, file := range bossFiles {
		data, err := os.ReadFile(file)
		if err != nil {
			t.Fatal(err)
		}
		for _, match := range luaMessage.FindAllStringSubmatch(string(data), -1) {
			texts = append(texts, match[1])
		}
	}
	if len(texts) < 200 {
		t.Fatalf("collected only %d content texts", len(texts))
	}
	for _, text := range texts {
		if text != "" && !catalog.Covers(text) {
			t.Errorf("content text has no catalog row: %q", text)
		}
	}
}

func readJSON(t *testing.T, name string, target any) {
	t.Helper()
	data, err := os.ReadFile(filepath.Join("..", "..", "data", "content", name))
	if err != nil {
		t.Fatal(err)
	}
	if err := json.Unmarshal(data, target); err != nil {
		t.Fatal(err)
	}
}

// A guild promotion from a channel 1 leader reaches a channel 2 member in
// English, and the leader still reads the confirmation in Portuguese.
func TestCrossChannelGuildPromotionUsesTheRecipientLanguage(t *testing.T) {
	f := criticalGuildJoinFixture(t, false)
	w := f.w
	target, _ := networkedTestPlayer(f.actor.ID, "Remote", 2100, 2100)
	other := worldWithNetworkedPlayers(target)
	other.store, other.channel = w.store, 2
	w.channel, w.language, other.language = 1, lang.Portuguese, lang.English
	c, err := NewChannelCluster(w.store)
	if err != nil {
		t.Fatal(err)
	}
	c.owner = w.criticalWrites
	if err := c.attach(w); err != nil {
		t.Fatal(err)
	}
	if err := c.attach(other); err != nil {
		t.Fatal(err)
	}
	other.criticalWrites = c.owner
	c.enter(w)
	if !w.claimAccountSession(target.Session, target.Account.Name) {
		t.Fatal("remote account claim")
	}
	target.Char.GuildID = 1
	w.guilds.Guilds[0].Members = append(w.guilds.Guilds[0].Members, model.GuildMember{
		Character: target.Char.Name, Account: target.Account.Name, Rank: model.GuildRankMember,
	})
	if err := w.queueGuildPromotion(f.actor, target, w.guilds.FindByID(1), model.GuildRankSubFirst); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, f.base.entered)
	f.release()
	awaitReadyCriticalResults(t, f.q, 1)
	c.enter(other)
	other.processCriticalCompletions(1)
	if target.Char.GuildRank != model.GuildRankSubFirst {
		t.Fatal("promotion did not settle")
	}
	panels := func(p *Player) []string {
		var out []string
		for packet, ok := p.Session.DequeuePacketForTest(); ok; packet, ok = p.Session.DequeuePacketForTest() {
			if !wire.Decrypt(packet) {
				t.Fatal("invalid packet")
			}
			if wire.ParseHeader(packet).Type == wire.OpMessagePanel {
				out = append(out, strings.TrimRight(string(packet[12:107]), "\x00"))
			}
		}
		return out
	}
	if got := panels(target); len(got) != 1 || got[0] != "You were promoted to sub-leader." {
		t.Fatalf("channel 2 member notices = %q", got)
	}
	want := portugueseText(t, "Remote agora é sub-líder.")
	if got := panels(f.actor); len(got) != 1 || got[0] != want {
		t.Fatalf("channel 1 leader notices = %q, want %q", got, want)
	}
}

// Each keyed call passes exactly one value per format verb of its message.
func TestKeyedMessageCallsPassEveryFormatValue(t *testing.T) {
	catalog, err := lang.Load(filepath.Join("..", "..", lang.DefaultDir))
	if err != nil {
		t.Fatal(err)
	}
	files, err := filepath.Glob("*.go")
	if err != nil {
		t.Fatal(err)
	}
	checked := 0
	for _, file := range files {
		if strings.HasSuffix(file, "_test.go") {
			continue
		}
		positions := token.NewFileSet()
		parsed, err := parser.ParseFile(positions, file, nil, 0)
		if err != nil {
			t.Fatal(err)
		}
		ast.Inspect(parsed, func(node ast.Node) bool {
			call, ok := node.(*ast.CallExpr)
			if !ok {
				return true
			}
			selector, ok := call.Fun.(*ast.SelectorExpr)
			if !ok {
				return true
			}
			keyIndex := 0
			switch selector.Sel.Name {
			case "msg":
			case "msgFor", "panel":
				keyIndex = 1
			default:
				return true
			}
			if len(call.Args) <= keyIndex || call.Ellipsis.IsValid() {
				return true
			}
			literal, ok := call.Args[keyIndex].(*ast.BasicLit)
			if !ok || literal.Kind != token.STRING {
				return true
			}
			key, err := strconv.Unquote(literal.Value)
			if err != nil {
				t.Fatal(err)
			}
			format, ok := catalog.Raw(lang.English, key)
			if !ok {
				return true
			}
			checked++
			if got, want := len(call.Args)-keyIndex-1, len(lang.Verbs(format)); got != want {
				t.Errorf("%s: %s receives %d values, its text has %d verbs", positions.Position(call.Pos()), key, got, want)
			}
			return true
		})
	}
	if checked < 500 {
		t.Fatalf("checked only %d keyed calls", checked)
	}
}

// Error strings follow Go style: lowercase and unpunctuated. A capitalized
// sentence ending in . ! or ? is player text, wherever it is built or returned,
// so the catalog must translate it at the send point.
func TestEverySourceSentenceHasACatalogRow(t *testing.T) {
	catalog, err := lang.Load(filepath.Join("..", "..", lang.DefaultDir))
	if err != nil {
		t.Fatal(err)
	}
	files, err := filepath.Glob("*.go")
	if err != nil {
		t.Fatal(err)
	}
	english := map[string]bool{}
	for _, key := range catalog.Keys(lang.English) {
		text, _ := catalog.Raw(lang.English, key)
		english[text] = true
	}
	literal := regexp.MustCompile(`"((?:[^"\\\n]|\\.)*)"`)
	sentence := regexp.MustCompile(`^[A-Z!].* .*[.!?]$`)
	for _, file := range files {
		if strings.HasSuffix(file, "_test.go") {
			continue
		}
		data, err := os.ReadFile(file)
		if err != nil {
			t.Fatal(err)
		}
		for n, line := range strings.Split(string(data), "\n") {
			trimmed := strings.TrimSpace(line)
			if strings.HasPrefix(trimmed, "//") || strings.Contains(line, "log.") || strings.Contains(line, "logf(") {
				continue
			}
			for _, match := range literal.FindAllStringSubmatch(line, -1) {
				text, err := strconv.Unquote(`"` + match[1] + `"`)
				if err != nil || len(text) < 8 || !sentence.MatchString(text) {
					continue
				}
				// A format literal is a catalog row itself; a built text matches one.
				if !english[text] && !catalog.Covers(text) {
					t.Errorf("%s:%d: %q has no catalog row", file, n+1, text)
				}
			}
		}
	}
}

func TestBuiltInformationAndRollTextUseTheChannelLanguage(t *testing.T) {
	pt := &World{language: lang.Portuguese}
	for english, want := range map[string]string{
		"1 Armia: owner=3 challenger=0 bid=10 victories=2": "1 Armia: dono=3 desafiante=0 lance=10 vitórias=2",
		"Success: 45/100":                       "Sucesso: 45/100",
		"Penalty Fail: 3/10":                    "Penalidade - Falha: 3/10",
		"The Nightmare boss is not configured.": "O chefe do Pesadelo não está configurado.",
	} {
		if got := pt.tr(english); got != portugueseText(t, want) {
			t.Errorf("tr(%q) = %q, want %q", english, got, want)
		}
	}
	if got := pt.characterCitizenship(&model.Char{Citizenship: 2}); got != "Canal 2" {
		t.Fatalf("citizenship = %q", got)
	}
}
