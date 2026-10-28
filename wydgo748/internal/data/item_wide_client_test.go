package data

import (
	"bufio"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"testing"
)

// The 7.48 ItemList.bin stores catalog effect values in 16 bits. Every server
// catalog value outside that range must reach the client through
// ItemListWide.csv with the same effect id and value, and the sidecar must not
// carry rows the server does not define.
func TestItemListWideMatchesServerCatalog(t *testing.T) {
	dataRoot := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(dataRoot, "catalog", "itemlist.csv"),
		filepath.Join(dataRoot, "catalog", "Itemname.csv"), filepath.Join(dataRoot, "catalog", "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	effectID := map[string]int{}
	for id, name := range catalog.ItemEffects {
		effectID[name] = int(id)
	}
	want := map[string]bool{}
	for index, def := range catalog.Items {
		for _, effect := range def.StaticEffects {
			if effect.Value >= -32768 && effect.Value <= 32767 {
				continue
			}
			id, ok := effectID[effect.Name]
			if !ok {
				if id, err = strconv.Atoi(effect.Name); err != nil {
					t.Fatalf("item %d effect %q has no numeric id", index, effect.Name)
				}
			}
			want[fmt.Sprintf("%d,%d,%d", index, id, effect.Value)] = true
		}
	}
	file, err := os.Open(filepath.Join("..", "..", "..", "tmproject", "client748", "ItemListWide.csv"))
	if err != nil {
		t.Fatal(err)
	}
	defer file.Close()
	got := map[string]bool{}
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" || strings.HasPrefix(line, "#") {
			continue
		}
		got[strings.ReplaceAll(line, " ", "")] = true
	}
	if err := scanner.Err(); err != nil {
		t.Fatal(err)
	}
	for row := range want {
		if !got[row] {
			t.Errorf("ItemListWide.csv lacks %s", row)
		}
	}
	for row := range got {
		if !want[row] {
			t.Errorf("ItemListWide.csv row %s is not a wide server catalog value", row)
		}
	}
}
