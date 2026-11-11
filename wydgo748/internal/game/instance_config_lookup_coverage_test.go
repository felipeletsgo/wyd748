package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestInstanceConfigLookupCoversCatalogSourcesAndRuntimeFallbacks(t *testing.T) {
	instance := model.VolatileInstance{ID: "nightmare", SharedGroup: "shared-nightmare", Name: "Nightmare"}
	other := model.VolatileInstance{ID: "other", SharedGroup: "shared-nightmare", Name: "Other"}
	tests := []struct {
		name      string
		catalog   model.VolatileCatalog
		configID  string
		runtimeID string
		wantID    string
		wantFound bool
	}{
		{
			name: "named data template",
			catalog: model.VolatileCatalog{Instances: map[string]model.VolatileInstance{
				"nightmare": instance, "other": other,
			}},
			configID: "nightmare", runtimeID: "shared:shared-nightmare", wantID: "nightmare", wantFound: true,
		},
		{
			name: "item rule exact ID",
			catalog: model.VolatileCatalog{Items: map[uint16]model.VolatileRule{
				1: {}, 2: {Instance: &instance},
			}},
			configID: "nightmare", runtimeID: "", wantID: "nightmare", wantFound: true,
		},
		{
			name: "code rule exact ID",
			catalog: model.VolatileCatalog{Rules: map[int]model.VolatileRule{
				1: {}, 2: {Instance: &instance},
			}},
			configID: "nightmare", runtimeID: "", wantID: "nightmare", wantFound: true,
		},
		{
			name:      "named data runtime fallback",
			catalog:   model.VolatileCatalog{Instances: map[string]model.VolatileInstance{"nightmare": instance}},
			runtimeID: "shared:shared-nightmare", wantID: "nightmare", wantFound: true,
		},
		{
			name:      "item rule runtime fallback",
			catalog:   model.VolatileCatalog{Items: map[uint16]model.VolatileRule{1: {Instance: &instance}}},
			runtimeID: "shared:shared-nightmare", wantID: "nightmare", wantFound: true,
		},
		{
			name:      "code rule runtime fallback",
			catalog:   model.VolatileCatalog{Rules: map[int]model.VolatileRule{1: {Instance: &instance}}},
			runtimeID: "shared:shared-nightmare", wantID: "nightmare", wantFound: true,
		},
		{name: "no runtime fallback", catalog: model.VolatileCatalog{Instances: map[string]model.VolatileInstance{"nightmare": instance}}, configID: "missing"},
		{name: "unknown runtime fallback", catalog: model.VolatileCatalog{Instances: map[string]model.VolatileInstance{"nightmare": instance}}, configID: "missing", runtimeID: "unknown"},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w := &World{volatiles: tc.catalog}
			got, found := w.instanceConfigByID(tc.configID, tc.runtimeID)
			if found != tc.wantFound {
				t.Fatalf("found=%t, want %t; config=%+v", found, tc.wantFound, got)
			}
			if found && got.ID != tc.wantID {
				t.Fatalf("selected instance %q, want %q", got.ID, tc.wantID)
			}
		})
	}
}
