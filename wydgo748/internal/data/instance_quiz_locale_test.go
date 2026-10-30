package data

import (
	"bytes"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// Every Cube question names a strdef row whose EN text is the server fallback
// and whose PT row is translated (EXTENSAO_COORDENADA, question packet v2).
func TestCubeQuestionsHaveLocalizedRows(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "catalog", "itemlist.csv"),
		filepath.Join(root, "catalog", "Itemname.csv"), filepath.Join(root, "catalog", "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := LoadVolatilesWithInstances(filepath.Join(root, "content", "volatiles.json"),
		filepath.Join(root, "content", "instances.json"), filepath.Join(root, "spawns", "instances"), catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	rows := func(lang string) [][]byte {
		raw, err := os.ReadFile(filepath.Join("..", "..", "..", "tmproject", "client748", lang, "UI", "strdef.bin"))
		if err != nil {
			t.Fatal(err)
		}
		payload := raw[:len(raw)-4]
		out := make([][]byte, len(payload)/128)
		for i := range out {
			row := make([]byte, 128)
			for j := range row {
				row[j] = payload[i*128+j] ^ 0x5a
			}
			out[i] = bytes.TrimRight(row, "\x00")
		}
		return out
	}
	en, pt := rows("EN"), rows("PT")
	count := 0
	for name, inst := range volatiles.Instances {
		for i, stage := range inst.Stages {
			q := stage.Quiz
			if q == nil {
				continue
			}
			count++
			if q.TextIndex == 0 || int(q.TextIndex) >= len(en) || int(q.TextIndex) >= len(pt) {
				t.Fatalf("%s stage %d: question %q has no strdef row (%d)", name, i, q.Question, q.TextIndex)
			}
			if string(en[q.TextIndex]) != strings.ReplaceAll(q.Question, "_", " ") {
				t.Fatalf("%s stage %d: EN row %q differs from %q", name, i, en[q.TextIndex], q.Question)
			}
			if len(pt[q.TextIndex]) == 0 || bytes.Equal(pt[q.TextIndex], en[q.TextIndex]) {
				t.Fatalf("%s stage %d: PT row %d is not translated", name, i, q.TextIndex)
			}
		}
	}
	if count == 0 {
		t.Fatal("no Cube questions loaded")
	}
}
