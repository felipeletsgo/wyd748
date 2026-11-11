package data

import (
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestValidateNPCsRejectsInvalidDefinitions(t *testing.T) {
	merchant := byte(16)
	for _, tc := range []struct {
		name   string
		mutate func(*model.NPCDef)
		want   string
	}{
		{name: "missing name", mutate: func(npc *model.NPCDef) { npc.Name = "" }, want: "has no name"},
		{name: "long display name", mutate: func(npc *model.NPCDef) { npc.DisplayName = "1234567890123456" }, want: "display name exceeds"},
		{name: "unsupported display byte", mutate: func(npc *model.NPCDef) { npc.DisplayName = "bad\x01name" }, want: "unsupported byte"},
		{name: "invalid type", mutate: func(npc *model.NPCDef) { npc.Tipo = "creature" }, want: "invalid type"},
		{name: "too many drops", mutate: func(npc *model.NPCDef) {
			npc.Tipo = model.TipoMonstro
			npc.Carry = make([]model.Item, model.MaxCarry+1)
		}, want: "carry slots"},
		{name: "too much shop stock", mutate: func(npc *model.NPCDef) { npc.Vende = make([]model.Item, model.MaxCarry+1) }, want: "shop slots"},
		{name: "monster shop stock", mutate: func(npc *model.NPCDef) { npc.Tipo = model.TipoMonstro; npc.Vende = []model.Item{{Index: 1}} }, want: "monster with shop stock"},
		{name: "NPC drops", mutate: func(npc *model.NPCDef) { npc.Carry = []model.Item{{Index: 1}} }, want: "NPC with monster drops"},
		{name: "equipment UID", mutate: func(npc *model.NPCDef) { npc.Equip.Rosto.UID = "stored" }, want: "equip[0] has an instance UID"},
		{name: "equipment index", mutate: func(npc *model.NPCDef) { npc.Equip.Rosto.Index = model.ItemListSize }, want: "exceeds ItemList"},
		{name: "carry UID", mutate: func(npc *model.NPCDef) { npc.Tipo = model.TipoMonstro; npc.Carry = []model.Item{{UID: "stored"}} }, want: "carry[0] has an instance UID"},
		{name: "shop UID", mutate: func(npc *model.NPCDef) { npc.Vende = []model.Item{{UID: "stored"}} }, want: "vende[0] has an instance UID"},
		{name: "missing score", mutate: func(npc *model.NPCDef) { npc.Score = nil }, want: "has no score"},
		{name: "invalid score", mutate: func(npc *model.NPCDef) { npc.Score.Version = 0 }, want: "score v0"},
		{name: "invalid direction", mutate: func(npc *model.NPCDef) { npc.Direction = 1 }, want: "invalid direction"},
		{name: "merchant override on monster", mutate: func(npc *model.NPCDef) { npc.Tipo = model.TipoMonstro; value := byte(1); npc.ClientMerchant = &value }, want: "invalid client merchant override"},
		{name: "merchant override overflow", mutate: func(npc *model.NPCDef) { npc.ClientMerchant = &merchant }, want: "invalid client merchant override"},
		{name: "resistance overflow", mutate: func(npc *model.NPCDef) { npc.Score.ResistFire = 256 }, want: "resistance outside the byte range"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			npc := validNPCForCoverage()
			tc.mutate(&npc)
			if err := validateNPCs("test", []model.NPCDef{npc}); err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("validateNPCs error = %v, want substring %q", err, tc.want)
			}
		})
	}
}

func TestValidateNPCsDefaultsTypeAndChecksCatalogItems(t *testing.T) {
	npc := validNPCForCoverage()
	npc.Tipo = ""
	npcs := []model.NPCDef{npc}
	if err := validateNPCs("test", npcs); err != nil || npcs[0].Tipo != model.TipoNPC {
		t.Fatalf("empty type defaulted to %q, err=%v", npcs[0].Tipo, err)
	}

	npc = validNPCForCoverage()
	npc.Equip.Rosto.Index = 1
	if _, err := ValidateNPCDefinition(npc, nil); err == nil || !strings.Contains(err.Error(), "unsupported item 1") {
		t.Fatalf("missing catalog item error = %v", err)
	}
	validated, err := ValidateNPCDefinition(npc, map[uint16]model.ItemDef{1: {Index: 1}})
	if err != nil || validated.Equip.Rosto.Index != 1 {
		t.Fatalf("valid NPC definition = %+v, err=%v", validated, err)
	}
}

func validNPCForCoverage() model.NPCDef {
	return model.NPCDef{Name: "CoverageNPC", Tipo: model.TipoNPC, Score: &model.Score{Version: model.ScoreVersion}}
}
