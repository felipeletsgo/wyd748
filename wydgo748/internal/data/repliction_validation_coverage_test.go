package data

import (
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

type replictionCoverageDocument struct {
	Items map[string]model.ReplictionItem `json:"items"`
	Pools map[string][][]int              `json:"pools"`
}

func TestReplictionLoaderRejectsInvalidRulesAndPools(t *testing.T) {
	items := map[uint16]model.ItemDef{
		1: {Index: 1, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 190}}, DynamicEffectNames: map[byte]string{1: "EF_ONE"}},
	}
	for _, tc := range []struct {
		name   string
		mutate func(*replictionCoverageDocument)
		want   string
	}{
		{name: "empty document", mutate: func(doc *replictionCoverageDocument) { doc.Items = nil; doc.Pools = nil }, want: "exige items e pools"},
		{name: "unknown item", mutate: func(doc *replictionCoverageDocument) {
			doc.Items = map[string]model.ReplictionItem{"2": {ItemLevel: 1, MaxSanc: 1}}
		}, want: "ausente do itemlist"},
		{name: "invalid item level", mutate: func(doc *replictionCoverageDocument) {
			rule := doc.Items["1"]
			rule.ItemLevel = 0
			doc.Items["1"] = rule
		}, want: "regra invalida"},
		{name: "invalid position", mutate: func(doc *replictionCoverageDocument) { doc.Pools["3"] = doc.Pools["2"]; delete(doc.Pools, "2") }, want: "posicao invalida"},
		{name: "empty pool", mutate: func(doc *replictionCoverageDocument) { doc.Pools["2"] = nil }, want: "pool repliction 2 vazio"},
		{name: "short entry", mutate: func(doc *replictionCoverageDocument) { doc.Pools["2"] = [][]int{{1, 0}} }, want: "requer quatro valores"},
		{name: "zero effect id", mutate: func(doc *replictionCoverageDocument) { doc.Pools["2"] = [][]int{{0, 0, 1, 0}} }, want: "efeito invalido"},
		{name: "unknown effect id", mutate: func(doc *replictionCoverageDocument) { doc.Pools["2"] = [][]int{{2, 0, 2, 0}} }, want: "ausente do ItemEffect.h"},
		{name: "negative bonus", mutate: func(doc *replictionCoverageDocument) { doc.Pools["2"] = [][]int{{1, -1, 1, 0}} }, want: "valor invalido"},
		{name: "bonus overflow", mutate: func(doc *replictionCoverageDocument) { doc.Pools["2"] = [][]int{{1, 0, 1, 256}} }, want: "valor invalido"},
		{name: "missing required position", mutate: func(doc *replictionCoverageDocument) { delete(doc.Pools, "32") }, want: "nao possui pool da posicao 32"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			doc := validReplictionCoverageDocument()
			tc.mutate(&doc)
			path := writeReplictionCoverageDocument(t, doc)
			if _, err := LoadRepliction(path, items); err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("LoadRepliction error = %v, want substring %q", err, tc.want)
			}
		})
	}

	noEffects := map[uint16]model.ItemDef{1: {Index: 1, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 190}}}}
	path := writeReplictionCoverageDocument(t, validReplictionCoverageDocument())
	noVolatile := map[uint16]model.ItemDef{1: {Index: 1, StaticEffects: []model.StaticEffect{{Name: "EF_AC", Value: 1}}, DynamicEffectNames: map[byte]string{1: "EF_ONE"}}}
	if _, err := LoadRepliction(path, noVolatile); err == nil || !strings.Contains(err.Error(), "nao possui EF_VOLATILE") {
		t.Fatalf("missing volatile effect error = %v", err)
	}
	if _, err := LoadRepliction(path, noEffects); err == nil || !strings.Contains(err.Error(), "ausente do ItemEffect.h") {
		t.Fatalf("missing dynamic effect table error = %v", err)
	}
	if _, err := LoadRepliction(filepath.Join(t.TempDir(), "missing.json"), items); err == nil {
		t.Fatal("LoadRepliction accepted a missing file")
	}
}

func TestReplictionLoaderAcceptsCompleteDocumentAndRejectsMalformedJSON(t *testing.T) {
	items := map[uint16]model.ItemDef{
		1: {Index: 1, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 190}}, DynamicEffectNames: map[byte]string{1: "EF_ONE"}},
	}
	path := writeReplictionCoverageDocument(t, validReplictionCoverageDocument())
	loaded, err := LoadRepliction(path, items)
	if err != nil || len(loaded.Items) != 1 || len(loaded.Pools) != 5 {
		t.Fatalf("complete repliction document = %+v, err=%v", loaded, err)
	}
	malformed := filepath.Join(t.TempDir(), "malformed.json")
	if err := os.WriteFile(malformed, []byte("{"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadRepliction(malformed, items); err == nil || !strings.Contains(err.Error(), "parse") {
		t.Fatalf("malformed repliction JSON error = %v", err)
	}
}

func validReplictionCoverageDocument() replictionCoverageDocument {
	return replictionCoverageDocument{
		Items: map[string]model.ReplictionItem{"1": {ItemLevel: 1, MaxSanc: 1}},
		Pools: map[string][][]int{
			"2": {{1, 0, 1, 0}}, "4": {{1, 0, 1, 0}}, "8": {{1, 0, 1, 0}},
			"16": {{1, 0, 1, 0}}, "32": {{1, 0, 1, 0}},
		},
	}
}

func writeReplictionCoverageDocument(t *testing.T, doc replictionCoverageDocument) string {
	t.Helper()
	body, err := json.Marshal(doc)
	if err != nil {
		t.Fatal(err)
	}
	path := filepath.Join(t.TempDir(), "repliction.json")
	if err := os.WriteFile(path, body, 0o600); err != nil {
		t.Fatal(err)
	}
	return path
}
