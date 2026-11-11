package lang

import (
	"os"
	"os/exec"
	"testing"
)

func TestLanguageCode(t *testing.T) {
	for language, want := range map[Language]string{
		English:       "en",
		Portuguese:    "pt",
		languageCount: "en",
	} {
		if got := language.Code(); got != want {
			t.Errorf("Language(%d).Code() = %q, want %q", language, got, want)
		}
	}
}

func TestCatalogCoversAndHas(t *testing.T) {
	catalog, err := Load(writeCatalog(t,
		"Fixed\tA fixed message.\nFormatted\tYou need %d gold.\n",
		"Fixed\tMensagem fixa.\nFormatted\tVocê precisa de %d de gold.\n"))
	if err != nil {
		t.Fatal(err)
	}
	for _, text := range []string{"A fixed message.", "You need 12 gold."} {
		if !catalog.Covers(text) {
			t.Errorf("Covers(%q) = false", text)
		}
	}
	if catalog.Covers("You need many gold.") || catalog.Covers("unrelated text") {
		t.Fatal("Covers accepted text that is not a catalog message")
	}
	if !catalog.Has("Fixed") || catalog.Has("Missing") {
		t.Fatal("Has did not reflect the English catalog keys")
	}

	var nilCatalog *Catalog
	if nilCatalog.Covers("A fixed message.") || nilCatalog.Has("Fixed") {
		t.Fatal("nil catalog reported message coverage")
	}
	if nilCatalog.Keys(English) != nil {
		t.Fatal("nil catalog returned keys")
	}
	if text, ok := nilCatalog.Raw(English, "Fixed"); ok || text != "" {
		t.Fatalf("nil catalog raw text=%q, exists=%t", text, ok)
	}
	if catalog.Keys(Language(99)) != nil {
		t.Fatal("invalid language returned catalog keys")
	}
	if text, ok := catalog.Raw(Language(99), "Fixed"); ok || text != "" {
		t.Fatalf("invalid language raw text=%q, exists=%t", text, ok)
	}
}

func TestTranslateKeepsEnglishFallbackAndOtherFormatVerbs(t *testing.T) {
	catalog, err := Load(writeCatalog(t,
		"Fixed\tOnly English.\nFormatted\tYou found %v items.\n",
		""))
	if err != nil {
		t.Fatal(err)
	}
	if got := catalog.Translate(Portuguese, "Only English."); got != "Only English." {
		t.Fatalf("English fallback = %q", got)
	}
	if got := catalog.Translate(Portuguese, "You found three items."); got != "You found three items." {
		t.Fatalf("formatted translation fallback = %q", got)
	}
	var nilCatalog *Catalog
	if got := nilCatalog.Translate(Portuguese, "untranslated"); got != "untranslated" {
		t.Fatalf("nil catalog translated text to %q", got)
	}
}

func TestDefaultLoadsAndCachesProductionCatalog(t *testing.T) {
	first := Default()
	if first == nil {
		t.Fatal("Default returned nil with the production catalog available")
	}
	if !first.Has("_NN_Not_Enough_Money") {
		t.Fatal("Default catalog is missing the native insufficient-gold message")
	}
	if second := Default(); second != first {
		t.Fatal("Default did not return the cached catalog")
	}
}

func TestDefaultReturnsNilWhenNoCatalogExists(t *testing.T) {
	cmd, err := os.Executable()
	if err != nil {
		t.Fatal(err)
	}
	workingDir := t.TempDir()
	proc := exec.Command(cmd, "-test.run=^TestDefaultHelperProcess$")
	proc.Dir = workingDir
	proc.Env = append(os.Environ(), "WYD_LANG_DEFAULT_HELPER=missing")
	if output, err := proc.CombinedOutput(); err != nil {
		t.Fatalf("Default helper failed: %v output=%s", err, output)
	}
}

func TestDefaultHelperProcess(t *testing.T) {
	if os.Getenv("WYD_LANG_DEFAULT_HELPER") != "missing" {
		return
	}
	if catalog := Default(); catalog != nil {
		t.Fatal("Default found a catalog outside the working-directory ancestors")
	}
}
