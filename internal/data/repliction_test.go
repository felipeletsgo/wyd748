package data

import (
	"path/filepath"
	"testing"
)

func TestLoadNativeReplictionCatalog(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(
		filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"),
		filepath.Join(root, "ItemEffect.h"),
		filepath.Join(root, "SkillData.csv"),
	)
	if err != nil {
		t.Fatal(err)
	}
	repliction, err := LoadRepliction(filepath.Join(root, "repliction.json"), catalog.Items)
	if err != nil {
		t.Fatal(err)
	}
	if len(repliction.Items) != 10 {
		t.Fatalf("itens Repliction=%d, esperado 10", len(repliction.Items))
	}
	wantPools := map[int]int{2: 25, 4: 48, 8: 48, 16: 27, 32: 30}
	for position, want := range wantPools {
		if got := len(repliction.Pools[position]); got != want {
			t.Fatalf("pool pos=%d possui %d entradas, esperado %d", position, got, want)
		}
	}
}
