package data

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestLoadInitItemsRejectsRotationOutsidePacketByteRange(t *testing.T) {
	items := map[uint16]model.ItemDef{746: {Index: 746}}
	for _, input := range []string{
		"746,217,215,256\n",
		"746,217,215,65535\n",
	} {
		path := filepath.Join(t.TempDir(), "init_items.csv")
		if err := os.WriteFile(path, []byte(input), 0600); err != nil {
			t.Fatal(err)
		}
		if _, err := LoadInitItems(path, items); err == nil || !strings.Contains(err.Error(), "rotation") {
			t.Fatalf("rotation in %q was not rejected: %v", input, err)
		}
	}
}

func TestLoadInitItemsRejectsGroundMaskRotationOutsideClientRange(t *testing.T) {
	items := map[uint16]model.ItemDef{
		458: {Index: 458, StaticEffects: []model.StaticEffect{{Name: "EF_GROUND", Value: 1}}},
		746: {Index: 746},
	}
	for _, test := range []struct {
		name    string
		line    string
		reject  bool
	}{
		{"ground mask rotation 3", "458,217,215,3\n", false},
		{"ground mask rotation 4", "458,217,215,4\n", true},
		{"ordinary item rotation 4", "746,217,215,4\n", false},
	} {
		t.Run(test.name, func(t *testing.T) {
			path := filepath.Join(t.TempDir(), "init_items.csv")
			if err := os.WriteFile(path, []byte(test.line), 0600); err != nil {
				t.Fatal(err)
			}
			_, err := LoadInitItems(path, items)
			if test.reject {
				if err == nil || !strings.Contains(err.Error(), "client mask range") {
					t.Fatalf("invalid ground mask rotation was not rejected: %v", err)
				}
			} else if err != nil {
				t.Fatal(err)
			}
		})
	}
}
