package model

import (
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"testing"
)

func TestCapeEmblemBackfillMatchesAuthoritativeCapeFamilies(t *testing.T) {
	sql, err := os.ReadFile(filepath.Join("..", "..", "..", "tools", "deployment", "vps", "backfill-cape-emblems.sql"))
	if err != nil {
		t.Fatal(err)
	}
	list := regexp.MustCompile(`(?s)capa'->>'index' IN \((.*?)\)`).FindSubmatch(sql)
	if len(list) != 2 {
		t.Fatal("maintenance cape family list is missing")
	}
	capes := map[int]bool{}
	for _, match := range regexp.MustCompile(`'([0-9]+)'`).FindAllSubmatch(list[1], -1) {
		id, err := strconv.Atoi(string(match[1]))
		if err != nil || capes[id] {
			t.Fatal("invalid or repeated maintenance cape ID")
		}
		capes[id] = true
	}
	for id := 0; id <= 65535; id++ {
		if capes[id] != (KingdomFromCape(uint16(id)) != KingdomNeutral) {
			t.Fatalf("maintenance disagrees with the authoritative cape family at %d", id)
		}
	}
}
