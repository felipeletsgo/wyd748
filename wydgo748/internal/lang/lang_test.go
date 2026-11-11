package lang

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"golang.org/x/text/encoding/charmap"
)

const productionDir = "../../data/content/language"

func cp1252(t *testing.T, text string) string {
	t.Helper()
	encoded, err := charmap.Windows1252.NewEncoder().String(text)
	if err != nil {
		t.Fatal(err)
	}
	return encoded
}

func writeCatalog(t *testing.T, en, pt string) string {
	t.Helper()
	dir := t.TempDir()
	if err := os.WriteFile(filepath.Join(dir, "en.txt"), []byte(en), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(dir, "pt.txt"), []byte(pt), 0o600); err != nil {
		t.Fatal(err)
	}
	return dir
}

func TestProductionCatalogIsCompleteAndFitsTheNotice(t *testing.T) {
	c, err := Load(productionDir)
	if err != nil {
		t.Fatal(err)
	}
	english, portuguese := c.Keys(English), c.Keys(Portuguese)
	if len(english) < 600 || len(portuguese) != len(english) {
		t.Fatalf("catalog has %d English and %d Portuguese messages", len(english), len(portuguese))
	}
	for _, key := range english {
		text, ok := c.Raw(Portuguese, key)
		if !ok {
			t.Errorf("%s has no Portuguese text", key)
			continue
		}
		// The native top notice and whisper text fields hold 94 bytes plus NUL.
		if len(text) > 94 {
			t.Errorf("%s Portuguese text has %d bytes: %q", key, len(text), text)
		}
	}
}

func TestProductionCatalogKeepsNativeMessageNames(t *testing.T) {
	c, err := Load(productionDir)
	if err != nil {
		t.Fatal(err)
	}
	for key, english := range map[string]string{
		"_NN_Not_Enough_Money":        "You do not have enough gold.",
		"_NN_Version_Not_Match_Rerun": "Client update required. Close the game and run the launcher to update.",
		"_DN_Need_D_Sapphire":         "You need %d Sapphires.",
	} {
		if got, _ := c.Raw(English, key); got != english {
			t.Errorf("%s = %q, want %q", key, got, english)
		}
	}
	if got := c.Text(Portuguese, "_NN_Not_Enough_Money"); got != cp1252(t, "Você não tem gold suficiente.") {
		t.Fatalf("Portuguese text is %q", got)
	}
}

func TestTextFormatsAndFallsBack(t *testing.T) {
	dir := writeCatalog(t,
		"# comment\nA\tYou need %d gold.\nB\tOnly English.\n",
		"A\tVocê precisa de %d de gold.\n")
	c, err := Load(dir)
	if err != nil {
		t.Fatal(err)
	}
	if got := c.Text(Portuguese, "A", 5); got != cp1252(t, "Você precisa de 5 de gold.") {
		t.Fatalf("formatted Portuguese = %q", got)
	}
	if got := c.Text(Portuguese, "B"); got != "Only English." {
		t.Fatalf("missing Portuguese text = %q, want English fallback", got)
	}
	if got := c.Text(English, "Missing"); got != "Missing" {
		t.Fatalf("unknown key = %q", got)
	}
	var none *Catalog
	if got := none.Text(Portuguese, "A"); got != "A" {
		t.Fatalf("nil catalog = %q", got)
	}
}

func TestTranslateMatchesFixedAndFormattedEnglish(t *testing.T) {
	dir := writeCatalog(t,
		"F\tNot enough gold.\nW\tGuild %s declared war on your guild.\nL\tYou must be level %d.\nC\tColosseum %c: wave %d/5 has begun.\nP\t%d%% done\n",
		"F\tGold insuficiente.\nW\tA guilda %s declarou guerra à sua guilda.\nL\tVocê precisa ter nível %d.\nC\tColiseu %c: a onda %d/5 começou.\nP\t%d%% feito\n")
	c, err := Load(dir)
	if err != nil {
		t.Fatal(err)
	}
	cases := map[string]string{
		"Not enough gold.":                           "Gold insuficiente.",
		"Guild Red Moon declared war on your guild.": "A guilda Red Moon declarou guerra à sua guilda.",
		"You must be level 255.":                     "Você precisa ter nível 255.",
		"Colosseum A: wave 3/5 has begun.":           "Coliseu A: a onda 3/5 começou.",
		"40% done":                                   "40% feito",
		"Player text stays unchanged.":               "Player text stays unchanged.",
		"You must be level high.":                    "You must be level high.",
	}
	for english, want := range cases {
		if got := c.Translate(Portuguese, english); got != cp1252(t, want) {
			t.Errorf("Translate(%q) = %q, want %q", english, got, want)
		}
		if got := c.Translate(English, english); got != english {
			t.Errorf("English Translate(%q) = %q", english, got)
		}
	}
}

func TestLoadRejectsInvalidCatalogs(t *testing.T) {
	cases := map[string][2]string{
		"verb mismatch":          {"A\tYou need %d gold.\n", "A\tVocê precisa de %s.\n"},
		"verb count mismatch":    {"A\t%s needs %d gold.\n", "A\t%s precisa de gold.\n"},
		"unknown pt key":         {"A\tText.\n", "B\tTexto.\n"},
		"duplicate key":          {"A\tText.\nA\tOther.\n", ""},
		"missing tab":            {"A Text.\n", ""},
		"empty english":          {"# only a comment\n", ""},
		"not windows1252":        {"A\tSnowman ☃.\n", ""},
		"too long":               {"A\t" + strings.Repeat("x", MaxTextBytes+1) + "\n", ""},
		"scanner token too long": {"A\t" + strings.Repeat("x", 70_000) + "\n", ""},
	}
	for name, files := range cases {
		if _, err := Load(writeCatalog(t, files[0], files[1])); err == nil {
			t.Errorf("%s: Load accepted an invalid catalog", name)
		}
	}
	if _, err := Load(t.TempDir()); err == nil {
		t.Error("Load accepted a directory without en.txt")
	}
}

func TestLanguageSelection(t *testing.T) {
	if ForChannel(1) != Portuguese || ForChannel(2) != English {
		t.Fatal("channel 1 must default to Portuguese and channel 2 to English")
	}
	for code, want := range map[string]Language{"pt": Portuguese, "EN": English, " en ": English} {
		if got, err := Parse(code); err != nil || got != want {
			t.Errorf("Parse(%q) = %v, %v", code, got, err)
		}
	}
	if _, err := Parse("es"); err == nil {
		t.Error("Parse accepted an unsupported language")
	}
}

func TestEncodeReplacesUnsupportedRunes(t *testing.T) {
	if got := Encode("Olá ☃"); got != cp1252(t, "Olá ?") {
		t.Fatalf("Encode = %q", got)
	}
}
