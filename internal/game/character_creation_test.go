package game

import (
	"path/filepath"
	"testing"

	"wydgo/internal/data"
)

func TestCharacterNameValidation(t *testing.T) {
	valid := []string{"Felipe", "Abcd", "TrezeCaracter"}
	invalid := []string{
		"abc", "TK01", "TK-Teste", "nome_completo", "nome grande",
		"QuatorzeLetras", "admin", "KING", "abc;drop",
	}
	for _, name := range valid {
		if !validCharacterName(name) {
			t.Errorf("nome valido recusado: %q", name)
		}
	}
	for _, name := range invalid {
		if validCharacterName(name) {
			t.Errorf("nome invalido aceito: %q", name)
		}
	}
}

func TestAllFourClassTemplatesCreatePlayableCharacters(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(
		filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"),
		filepath.Join(root, "SkillData.csv"),
	)
	if err != nil {
		t.Fatal(err)
	}
	cfg, err := data.LoadCharacterTemplates(filepath.Join(root, "character_templates.json"), catalog.Items)
	if err != nil {
		t.Fatal(err)
	}
	w := &World{items: catalog.Items, skills: catalog.Skills, charSpawn: cfg.Spawn}
	for _, tpl := range cfg.Classes {
		w.charTemplates[tpl.Class] = tpl
	}
	for class := 0; class < 4; class++ {
		ch, ok := w.newCharacterFromTemplate("Teste", class)
		if !ok {
			t.Fatalf("classe %d nao foi criada", class)
		}
		if int(ch.Class) != class || ch.X != 2100 || ch.Y != 2100 {
			t.Fatalf("classe %d identidade/nascimento incorretos: %+v", class, ch)
		}
		e := effectiveExtended(&ch)
		if e.CurHP == 0 || e.CurHP != e.MaxHP ||
			e.CurMP == 0 || e.CurMP != e.MaxMP {
			t.Fatalf("classe %d nao nasceu cheia: HP=%d/%d MP=%d/%d", class,
				e.CurHP, e.MaxHP, e.CurMP, e.MaxMP)
		}
		if ch.LearnedSkill != 0 || e.StatusPts != 0 || e.MasterPts != 0 || e.SkillPts != 0 {
			t.Fatalf("classe %d nasceu com progresso indevido: learned=%X pts=%d/%d/%d", class,
				ch.LearnedSkill, e.StatusPts, e.MasterPts, e.SkillPts)
		}
	}
}
