package data

import (
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestLoadVolatilesDiscoversCatalogItems(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "volatiles.json")
	if err := os.WriteFile(path, []byte(`{
		"default":{"action":"generic"},
		"rules":{"1":{"action":"restore","consume":true,"valueSource":"item_effects"}},
		"items":{"401":{"action":"restore","consume":true,"hp":999}}
	}`), 0600); err != nil {
		t.Fatal(err)
	}
	items := map[uint16]model.ItemDef{
		400: {Index: 400, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 1}}},
		401: {Index: 401, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 1}}},
		500: {Index: 500},
	}
	catalog, err := LoadVolatiles(path, items)
	if err != nil {
		t.Fatal(err)
	}
	if len(catalog.ItemCodes) != 2 || catalog.Codes[1] != 2 {
		t.Fatalf("descoberta incorreta: items=%v codes=%v", catalog.ItemCodes, catalog.Codes)
	}
	rule, code, ok := catalog.Rule(401)
	if !ok || code != 1 || rule.HP != 999 {
		t.Fatalf("override nao aplicado: rule=%+v code=%d ok=%v", rule, code, ok)
	}
}

func TestLoadVolatilesValidaQuestReward(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "volatiles.json")
	items := map[uint16]model.ItemDef{
		4117: {Index: 4117, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 191}}},
	}
	for _, body := range []string{
		`{"default":{"action":"generic"},"items":{"4117":{"action":"quest_reward","consume":true,"minLevel":39,"maxLevelExclusive":115}}}`,
		`{"default":{"action":"generic"},"items":{"4117":{"action":"quest_reward","consume":true,"exp":5000,"minLevel":39,"maxLevelExclusive":39}}}`,
	} {
		if err := os.WriteFile(path, []byte(body), 0600); err != nil {
			t.Fatal(err)
		}
		if _, err := LoadVolatiles(path, items); err == nil {
			t.Fatalf("quest_reward invalida foi aceita: %s", body)
		}
	}
}

// TestVolatilesRealConfigDifferentiatesByItemIndex carrega os arquivos REAIS e
// prova a regra do felipe: itens que compartilham o mesmo EF_VOLATILE podem ter
// parametros diferentes, resolvidos por Index. Tambem garante que o volatiles.json
// de producao carrega sem erro contra o catalogo real.
func TestVolatilesRealConfigDifferentiatesByItemIndex(t *testing.T) {
	catalog, err := LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	vc, err := LoadVolatiles("../../data/volatiles.json", catalog.Items)
	if err != nil {
		t.Fatalf("volatiles.json real nao carrega contra o catalogo: %v", err)
	}

	// Sefirah 7 dias vs 30 dias: MESMO codigo (67, o portador aceito pelo client),
	// duracoes diferentes por override de Index -> diferenciacao por Index funciona.
	r7, c7, ok7 := vc.Rule(3361)
	r30, c30, ok30 := vc.Rule(3363)
	if !ok7 || !ok30 || c7 != 67 || c30 != 67 {
		t.Fatalf("codigos Sefirah inesperados: c7=%d c30=%d ok=%v/%v", c7, c30, ok7, ok30)
	}
	if r7.DurationUnits == r30.DurationUnits {
		t.Fatalf("Sefirah 7d e 30d resolveram a MESMA duracao (%d): diferenciacao por index falhou",
			r7.DurationUnits)
	}
	if r30.DurationUnits <= r7.DurationUnits {
		t.Fatalf("Sefirah 30d (%d) deveria durar mais que 7d (%d)", r30.DurationUnits, r7.DurationUnits)
	}

	// Buff de dano keyed por codigo (Kappa 787 -> Type 4 escalando por %).
	if r, _, _ := vc.Rule(787); r.Action != "buff" || r.AffectType != 4 || r.AffectValue == 0 {
		t.Fatalf("Kappa (787) nao virou buff Type 4 escalado: %+v", r)
	}
	// Fairy Dust usa o proximo marco da tabela nativa, nunca EXP fixa.
	if r, _, _ := vc.Rule(414); r.Action != "grant_next_level" {
		t.Fatalf("Fairy_Dust (414) nao virou grant_next_level: %+v", r)
	}
	// Gema Estelar / Warp (12/13) e bau de EXP (198) resolvem as acoes certas.
	if vc.Rules[12].Action != "save_position" {
		t.Fatalf("code 12 deveria ser save_position: %q", vc.Rules[12].Action)
	}
	if vc.Rules[13].Action != "warp_saved" {
		t.Fatalf("code 13 deveria ser warp_saved: %q", vc.Rules[13].Action)
	}
	if r := vc.Rules[198]; r.Action != "buff" || r.AffectType != 39 || !r.Accumulate {
		t.Fatalf("code 198 deveria ser buff acumulativo affect 39: %+v", r)
	}
	// Rodada final do _MSG_UseItem: os multiplexados por Index nao podem
	// regredir silenciosamente para a regra no_direct_use do codigo.
	for itemID, action := range map[uint16]string{
		3453: "mount_revive", 4003: "loot_box",
		1731: "instance_ticket", 3171: "instance_ticket", 3172: "instance_ticket",
		3324: "instance_ticket", 3390: "instance_ticket",
		3328: "dungeon_teleport", 3329: "dungeon_teleport",
		3909: "timed_access", 3439: "timed_access",
		3393: "grant_counter", 4114: "grant_counter",
	} {
		r, _, ok := vc.Rule(itemID)
		if !ok || r.Action != action {
			t.Fatalf("item %d deveria ser %s: %+v ok=%v", itemID, action, r, ok)
		}
	}
	for _, itemID := range []uint16{3443, 3455, 5338} {
		r, _, ok := vc.Rule(itemID)
		if !ok || r.Action != "celestial_pending" || r.Consume {
			t.Fatalf("Celestial %d deve ficar bloqueado sem consumo: %+v ok=%v",
				itemID, r, ok)
		}
	}
	// Refino: Ori(4)=teto+6, Lac(5)=teto+9, Molar(194)=set +6.
	if r := vc.Rules[4]; r.Action != "refine" || r.RefineMax != 6 {
		t.Fatalf("code 4 deveria ser refine teto 6: %+v", r)
	}
	if r := vc.Rules[5]; r.Action != "refine" || r.RefineMax != 9 {
		t.Fatalf("code 5 deveria ser refine teto 9: %+v", r)
	}
	if r := vc.Rules[194]; r.Action != "refine_set" || r.RefineMax != 6 {
		t.Fatalf("code 194 deveria ser refine_set teto 6: %+v", r)
	}
	// As 24 salas regulares e os tres Nessus sao instancias distintas. Cada
	// sala comum entrega o pergaminho seguinte; o boss encerra a cadeia.
	waterItems := []uint16{
		777, 778, 779, 780, 781, 782, 783, 784, 785,
		3173, 3174, 3175, 3176, 3177, 3178, 3179, 3180, 3181,
		3182, 3183, 3184, 3185, 3186, 3187, 3188, 3189, 3190,
	}
	instanceIDs := make(map[string]struct{}, len(waterItems))
	for _, itemID := range waterItems {
		r, _, ok := vc.Rule(itemID)
		if !ok || r.Action != "instance_ticket" || r.Instance == nil {
			t.Fatalf("pergaminho Water %d sem instancia: %+v ok=%v", itemID, r, ok)
		}
		instanceIDs[r.Instance.ID] = struct{}{}
	}
	if len(instanceIDs) != len(waterItems) {
		t.Fatalf("salas Water repetidas: ids=%d itens=%d", len(instanceIDs), len(waterItems))
	}
	chains := [][]uint16{
		{777, 778, 779, 780, 781, 782, 783, 784, 785},
		{3173, 3174, 3175, 3176, 3177, 3178, 3179, 3180, 3181},
		{3182, 3183, 3184, 3185, 3186, 3187, 3188, 3189, 3190},
	}
	for _, chain := range chains {
		for index, itemID := range chain {
			r, _, _ := vc.Rule(itemID)
			want := uint16(0)
			if index+1 < len(chain) {
				want = chain[index+1]
			}
			if r.Instance.RewardItem != want {
				t.Fatalf("Water %d recompensa %d, quer %d",
					itemID, r.Instance.RewardItem, want)
			}
		}
	}
}

func TestLoadVolatilesAppliesGenericToEveryUnconfiguredCode(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "volatiles.json")
	if err := os.WriteFile(path, []byte(`{"default":{"action":"generic","consume":false}}`), 0600); err != nil {
		t.Fatal(err)
	}
	items := map[uint16]model.ItemDef{
		700: {Index: 700, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 77}}},
	}
	catalog, err := LoadVolatiles(path, items)
	if err != nil {
		t.Fatal(err)
	}
	rule, code, ok := catalog.Rule(700)
	if !ok || code != 77 || rule.Action != "generic" || rule.Consume {
		t.Fatalf("fallback generico incorreto: rule=%+v code=%d ok=%v", rule, code, ok)
	}
}
