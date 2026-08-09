from pathlib import Path


def replace_once(path: str, old: str, new: str) -> None:
    p = Path(path)
    text = p.read_text(encoding="utf-8")
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{path}: expected exactly one match, found {count}: {old[:80]!r}")
    p.write_text(text.replace(old, new, 1), encoding="utf-8")

# 1) WYD KR tier boundaries + SubCelestial 100 Fame.
replace_once(
    "internal/game/celestial.go",
    "\tsubCelestialFaceType = byte(4)\n)",
    "\tsubCelestialFaceType = byte(4)\n\tsubCelestialFameCost = uint32(100)\n)",
)
replace_once(
    "internal/game/celestial.go",
    "func celestialArchTier(level uint32) byte {\n\tswitch {\n\tcase level >= 399:\n\t\treturn 5\n\tcase level == 398:\n\t\treturn 4\n\tcase level >= 379:\n\t\treturn 3\n\tcase level >= 369:\n\t\treturn 2\n\tcase level >= 355:\n\t\treturn 1\n\tdefault:\n\t\treturn 0\n\t}\n}",
    "func celestialArchTier(level uint32) byte {\n\t// Faixas oficiais WYD KR, convertidas do nivel exibido para o nivel\n\t// interno do protocolo: 356-370, 371-380, 381-398, 399 e 400.\n\tswitch {\n\tcase level >= 399:\n\t\treturn 5\n\tcase level == 398:\n\t\treturn 4\n\tcase level >= 380:\n\t\treturn 3\n\tcase level >= 370:\n\t\treturn 2\n\tcase level >= 355:\n\t\treturn 1\n\tdefault:\n\t\treturn 0\n\t}\n}",
)
replace_once(
    "internal/game/celestial.go",
    "\tif ch.Extended == nil || ch.Extended.Level < 120 {\n\t\ts.Send(wire.MessagePanel(\"The Celestial must be level 121 or higher.\"))\n\t\ts.Send(wire.SendItem(p.ID, placeInv, slot, *item))\n\t\treturn\n\t}\n\tsubClass, ok := archClassFromSefirot(ch.Equip[sefirotSlot])",
    "\tif ch.Extended == nil || ch.Extended.Level < 120 {\n\t\ts.Send(wire.MessagePanel(\"The Celestial must be level 121 or higher.\"))\n\t\ts.Send(wire.SendItem(p.ID, placeInv, slot, *item))\n\t\treturn\n\t}\n\tif counterBalance(p, fameCounter) < subCelestialFameCost {\n\t\ts.Send(wire.MessagePanel(\"100 Fame is required to create a SubCelestial.\"))\n\t\ts.Send(wire.SendItem(p.ID, placeInv, slot, *item))\n\t\treturn\n\t}\n\tsubClass, ok := archClassFromSefirot(ch.Equip[sefirotSlot])",
)
replace_once(
    "internal/game/celestial.go",
    "\tsnapshot := cloneCharacterState(ch)\n\tsubFace := ch.Equip[0]",
    "\tsnapshot := cloneCharacterState(ch)\n\toldCounters := copyCounters(p)\n\tsubFace := ch.Equip[0]",
)
replace_once(
    "internal/game/celestial.go",
    "\ttarget := addToInv(ch, mystery)\n\tif target < 0 {\n\t\t*ch = snapshot\n\t\ts.Send(wire.MessagePanel(\"Inventory is full.\"))\n\t\treturn\n\t}\n\tsyncProgression(ch) // passa a incluir 6 x nivel da forma alterna\n\tw.recalcPlayer(ch)\n\tif err := w.saveAccount(p.Account); err != nil {\n\t\t*ch = snapshot\n\t\tlog.Printf(\"[#%d] salvar criacao SubCelestial: %v\", s.ID, err)\n\t\ts.Send(wire.MessagePanel(\"Save failed. Nothing was consumed.\"))\n\t\treturn\n\t}",
    "\ttarget := addToInv(ch, mystery)\n\tif target < 0 {\n\t\t*ch = snapshot\n\t\tp.SpecialCoins = oldCounters\n\t\ts.Send(wire.MessagePanel(\"Inventory is full.\"))\n\t\treturn\n\t}\n\tif !spendCounters(p, map[string]uint32{fameCounter: subCelestialFameCost}) {\n\t\t*ch = snapshot\n\t\tp.SpecialCoins = oldCounters\n\t\ts.Send(wire.MessagePanel(\"100 Fame is required to create a SubCelestial.\"))\n\t\treturn\n\t}\n\tsyncProgression(ch) // passa a incluir 6 x nivel da forma alterna\n\tw.recalcPlayer(ch)\n\tif err := w.saveAccountAndCharStateResult(p); err != nil {\n\t\t*ch = snapshot\n\t\tp.SpecialCoins = oldCounters\n\t\tlog.Printf(\"[#%d] salvar criacao SubCelestial: %v\", s.ID, err)\n\t\ts.Send(wire.MessagePanel(\"Save failed. Nothing was consumed.\"))\n\t\treturn\n\t}",
)

# 2) Odin: exact secret-stone recipes 95%, arbitrary seven-rune mix 5%, 2M cost.
replace_once(
    "internal/game/crafting_advanced.go",
    "var secretStoneRecipes = [4][8]uint16{\n\t{5126, 5122, 5121, 5114, 5125, 5111, 5118, 5334},\n\t{5131, 5113, 5115, 5116, 5125, 5112, 5114, 5335},\n\t{5110, 5124, 5117, 5129, 5114, 5125, 5128, 5336},\n\t{5122, 5119, 5132, 5120, 5130, 5133, 5123, 5337},\n}\n",
    "var secretStoneRecipes = [4][8]uint16{\n\t{5126, 5122, 5121, 5114, 5125, 5111, 5118, 5334},\n\t{5131, 5113, 5115, 5116, 5125, 5112, 5114, 5335},\n\t{5110, 5124, 5117, 5129, 5114, 5125, 5128, 5336},\n\t{5122, 5119, 5132, 5120, 5130, 5133, 5123, 5337},\n}\n\nconst (\n\todinSecretStoneGoldCost      = uint32(2_000_000)\n\todinSecretStoneCorrectChance = 95\n\todinSecretStoneRandomChance  = 5\n)\n",
)
replace_once(
    "internal/game/crafting_advanced.go",
    "func (w *World) deterministicOdin(p *Player, req combineRequest, output uint16) {\n\toldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold\n\tchanged := make(map[int]struct{}, 8)\n\tpreferred := int(req.Pos[0])\n\tconsumeCombineItems(p.Char, req, 0, 6, changed)\n\tif !w.putCraftResult(p, preferred, model.Item{Index: output}, changed) {\n\t\tp.Char.Inv, p.Char.Equip, p.Char.Gold = oldInv, oldEquip, oldGold\n\t\tw.sendCombineResult(p, 0)\n\t\treturn\n\t}\n\tw.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, 1)\n}\n",
    "func (w *World) deterministicOdin(p *Player, req combineRequest, output uint16) {\n\toldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold\n\tchanged := make(map[int]struct{}, 8)\n\tpreferred := int(req.Pos[0])\n\tconsumeCombineItems(p.Char, req, 0, 6, changed)\n\tif !w.putCraftResult(p, preferred, model.Item{Index: output}, changed) {\n\t\tp.Char.Inv, p.Char.Equip, p.Char.Gold = oldInv, oldEquip, oldGold\n\t\tw.sendCombineResult(p, 0)\n\t\treturn\n\t}\n\tw.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, 1)\n}\n\nfunc odinRune(index uint16) bool {\n\treturn index >= 5110 && index <= 5133\n}\n\nfunc sevenOdinRunes(req combineRequest) bool {\n\tif req.Items[7].Index != 0 {\n\t\treturn false\n\t}\n\tfor i := 0; i < 7; i++ {\n\t\tif !odinRune(req.Items[i].Index) {\n\t\t\treturn false\n\t\t}\n\t}\n\treturn true\n}\n\n// combineSecretStoneOdin aplica a regra retail: qualquer tentativa valida com\n// sete runas custa 2M e consome as sete runas. Uma sequencia exata usa 95%;\n// uma mistura sem receita de Pedra Secreta usa 5% e, no sucesso, sorteia um\n// dos quatro elementos porque a combinacao nao identifica um elemento unico.\nfunc (w *World) combineSecretStoneOdin(p *Player, req combineRequest, output uint16, chance int) {\n\tif p == nil || p.Char == nil || p.Char.Gold < odinSecretStoneGoldCost {\n\t\tw.sendCombineResult(p, 0)\n\t\treturn\n\t}\n\tsuccess := w.intn(100) < chance\n\tif success && output == 0 {\n\t\toutput = uint16(5334 + w.intn(4))\n\t}\n\n\toldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold\n\tchanged := make(map[int]struct{}, 8)\n\tpreferred := int(req.Pos[0])\n\tconsumeCombineItems(p.Char, req, 0, 6, changed)\n\tp.Char.Gold -= odinSecretStoneGoldCost\n\tif success {\n\t\tif !w.putCraftResult(p, preferred, model.Item{Index: output}, changed) {\n\t\t\tp.Char.Inv, p.Char.Equip, p.Char.Gold = oldInv, oldEquip, oldGold\n\t\t\tw.sendCombineResult(p, 0)\n\t\t\treturn\n\t\t}\n\t}\n\tcode := uint32(2)\n\tif success {\n\t\tcode = 1\n\t}\n\tw.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, code)\n}\n",
)
replace_once(
    "internal/game/crafting_advanced.go",
    "\tfor _, recipe := range secretStoneRecipes {\n\t\tif exactRecipe(req, recipe[:7]) {\n\t\t\tw.deterministicOdin(p, req, recipe[7])\n\t\t\treturn\n\t\t}\n\t}\n\tfury := []uint16{5125, 5115, 5111, 5112, 5120, 5128, 5119}\n\tif exactRecipe(req, fury) {\n\t\tw.deterministicOdin(p, req, 3020)\n\t\treturn\n\t}\n",
    "\tfor _, recipe := range secretStoneRecipes {\n\t\tif exactRecipe(req, recipe[:7]) {\n\t\t\tw.combineSecretStoneOdin(p, req, recipe[7], odinSecretStoneCorrectChance)\n\t\t\treturn\n\t\t}\n\t}\n\tfury := []uint16{5125, 5115, 5111, 5112, 5120, 5128, 5119}\n\tif exactRecipe(req, fury) {\n\t\tw.deterministicOdin(p, req, 3020)\n\t\treturn\n\t}\n\t// A sequencia da Fury e uma receita propria e precisa ser testada antes do\n\t// fallback de sete runas. Qualquer outra mistura de runas tenta uma Pedra\n\t// Secreta aleatoria com 5% de chance.\n\tif sevenOdinRunes(req) {\n\t\tw.combineSecretStoneOdin(p, req, 0, odinSecretStoneRandomChance)\n\t\treturn\n\t}\n",
)

# 3) Update existing Celestial tests for the selected KR boundaries and fame cost.
replace_once(
    "internal/game/celestial_test.go",
    "\t\t{355, 1, 3500, 100},\n\t\t{368, 1, 3500, 100},\n\t\t{369, 2, 3500, 300},\n\t\t{378, 2, 3500, 300},\n\t\t{379, 3, 3501, 600},\n\t\t{397, 3, 3501, 600},",
    "\t\t{355, 1, 3500, 100},\n\t\t{369, 1, 3500, 100},\n\t\t{370, 2, 3500, 300},\n\t\t{379, 2, 3500, 300},\n\t\t{380, 3, 3501, 600},\n\t\t{397, 3, 3501, 600},",
)
replace_once(
    "internal/game/celestial_test.go",
    "\tp.Char.SoulInfo = 7\n\tp.Char.Gold = 123456",
    "\tp.Char.SoulInfo = 7\n\tp.Char.Gold = 123456\n\tp.SpecialCoins = map[string]uint32{fameCounter: subCelestialFameCost}",
)
replace_once(
    "internal/game/celestial_test.go",
    "\tif mysterySlot < 0 || itemStackAmount(p.Char.Inv[mysterySlot]) != 10 {\n\t\tt.Fatal(\"criacao da Sub nao entregou 10 Pedras Misteriosas\")\n\t}\n",
    "\tif mysterySlot < 0 || itemStackAmount(p.Char.Inv[mysterySlot]) != 10 {\n\t\tt.Fatal(\"criacao da Sub nao entregou 10 Pedras Misteriosas\")\n\t}\n\tif got := counterBalance(p, fameCounter); got != 0 {\n\t\tt.Fatalf(\"criacao da Sub nao consumiu 100 Fame: %d\", got)\n\t}\n",
)

# 4) Focused regression tests using the real combine handler and deterministic World RNG.
test_path = Path("internal/game/celestial_native_progression_test.go")
if test_path.exists():
    raise SystemExit(f"{test_path} already exists")
test_path.write_text(r'''package game

import (
    "errors"
    "testing"

    "wydgo/internal/model"
)

func odinSecretStoneAttempt(t *testing.T, runes [7]uint16, gold uint32, rng RNG) (*World, *Player, *craftStore) {
    t.Helper()
    defs := map[uint16]model.ItemDef{
        5334: {Index: 5334}, 5335: {Index: 5335},
        5336: {Index: 5336}, 5337: {Index: 5337},
        3020: {Index: 3020},
    }
    w, p, session, st := newCraftWorld(t, "Odin", defs, gold)
    w.rng = rng
    var items [combineSlots]model.Item
    var pos [combineSlots]int8
    for i, index := range runes {
        items[i] = model.Item{Index: index}
        pos[i] = int8(i)
    }
    placeItems(p.Char, items, pos)
    w.onCombineOdin(session, buildCombinePacket(items, pos))
    return w, p, st
}

func TestOdinSecretStoneCorrectSequenceUses95PercentAndCharges2M(t *testing.T) {
    recipe := secretStoneRecipes[0]
    var runes [7]uint16
    copy(runes[:], recipe[:7])
    _, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost+123, fixedRNG{value: 94})
    if st.saves != 1 {
        t.Fatalf("saves=%d, want 1", st.saves)
    }
    if p.Char.Gold != 123 {
        t.Fatalf("gold=%d, want 123", p.Char.Gold)
    }
    if p.Char.Inv[0].Index != recipe[7] {
        t.Fatalf("result=%d, want %d", p.Char.Inv[0].Index, recipe[7])
    }
    for i := 1; i < 7; i++ {
        if p.Char.Inv[i].Index != 0 {
            t.Fatalf("rune slot %d was not consumed: %+v", i, p.Char.Inv[i])
        }
    }
}

func TestOdinSecretStoneCorrectSequenceRoll95FailsAndConsumes(t *testing.T) {
    recipe := secretStoneRecipes[0]
    var runes [7]uint16
    copy(runes[:], recipe[:7])
    _, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost, fixedRNG{value: 95})
    if st.saves != 1 || p.Char.Gold != 0 {
        t.Fatalf("failed craft must persist cost: saves=%d gold=%d", st.saves, p.Char.Gold)
    }
    for i := 0; i < 7; i++ {
        if p.Char.Inv[i].Index != 0 {
            t.Fatalf("failed craft did not consume rune slot %d: %+v", i, p.Char.Inv[i])
        }
    }
}

func TestOdinRandomSevenRunesUses5PercentAndRandomSecretStone(t *testing.T) {
    // Valid runes, but neither one of the four Secret Stone sequences nor Fury.
    runes := [7]uint16{5110, 5111, 5112, 5113, 5114, 5115, 5116}
    _, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost, fixedRNG{value: 0})
    if st.saves != 1 || p.Char.Gold != 0 {
        t.Fatalf("random craft did not persist cost: saves=%d gold=%d", st.saves, p.Char.Gold)
    }
    if p.Char.Inv[0].Index < 5334 || p.Char.Inv[0].Index > 5337 {
        t.Fatalf("random successful craft produced %d, want a Secret Stone", p.Char.Inv[0].Index)
    }
}

func TestOdinRandomSevenRunesRoll5FailsAndConsumes(t *testing.T) {
    runes := [7]uint16{5110, 5111, 5112, 5113, 5114, 5115, 5116}
    _, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost, fixedRNG{value: 5})
    if st.saves != 1 || p.Char.Gold != 0 {
        t.Fatalf("random failure did not persist cost: saves=%d gold=%d", st.saves, p.Char.Gold)
    }
    for i := 0; i < 7; i++ {
        if p.Char.Inv[i].Index != 0 {
            t.Fatalf("random failure did not consume rune slot %d: %+v", i, p.Char.Inv[i])
        }
    }
}

func TestOdinSecretStoneInsufficientGoldDoesNotConsume(t *testing.T) {
    recipe := secretStoneRecipes[0]
    var runes [7]uint16
    copy(runes[:], recipe[:7])
    _, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost-1, fixedRNG{value: 0})
    if st.saves != 0 || p.Char.Gold != odinSecretStoneGoldCost-1 {
        t.Fatalf("insufficient gold mutated state: saves=%d gold=%d", st.saves, p.Char.Gold)
    }
    for i, index := range runes {
        if p.Char.Inv[i].Index != index {
            t.Fatalf("insufficient gold consumed rune slot %d", i)
        }
    }
}

func TestOdinFurySequenceIsNotCapturedByRandomSecretStoneFallback(t *testing.T) {
    runes := [7]uint16{5125, 5115, 5111, 5112, 5120, 5128, 5119}
    _, p, st := odinSecretStoneAttempt(t, runes, 0, fixedRNG{value: 99})
    if st.saves != 1 || p.Char.Inv[0].Index != furyStoneItem {
        t.Fatalf("Fury recipe was shadowed by random-rune fallback: saves=%d result=%d", st.saves, p.Char.Inv[0].Index)
    }
}

func TestSubCelestialRequires100Fame(t *testing.T) {
    w, p, st := newCelestialWorld(t, 399)
    w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
    p.Char.Extended.Level = 120
    p.Char.Equip[sefirotSlot] = model.Item{Index: 1763}
    p.Char.Inv[0] = model.Item{Index: idealStoneItem}
    p.SpecialCoins = map[string]uint32{fameCounter: subCelestialFameCost - 1}
    before := cloneCharacterState(p.Char)

    w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)

    if st.saves != 1 { // only the initial Celestial creation
        t.Fatalf("Sub with insufficient Fame persisted: saves=%d", st.saves)
    }
    if p.Char.AlternateCelestial != nil || p.Char.Inv[0].Index != before.Inv[0].Index ||
        counterBalance(p, fameCounter) != subCelestialFameCost-1 {
        t.Fatalf("Sub with insufficient Fame mutated state")
    }
}

func TestSubCelestialSaveFailureRestoresFameAndItems(t *testing.T) {
    w, p, st := newCelestialWorld(t, 399)
    w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
    p.Char.Extended.Level = 120
    p.Char.Equip[sefirotSlot] = model.Item{Index: 1763}
    p.Char.Inv[0] = model.Item{Index: idealStoneItem}
    p.SpecialCoins = map[string]uint32{fameCounter: subCelestialFameCost}
    before := cloneCharacterState(p.Char)
    st.err = errors.New("postgres unavailable")

    w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)

    if st.saves != 2 {
        t.Fatalf("expected failed Sub persist attempt, saves=%d", st.saves)
    }
    if p.Char.AlternateCelestial != before.AlternateCelestial || p.Char.Inv != before.Inv ||
        p.Char.Equip != before.Equip || counterBalance(p, fameCounter) != subCelestialFameCost {
        t.Fatalf("Sub rollback did not restore character and Fame")
    }
}
''', encoding="utf-8")

# 5) Documentation: selected native rules, not the superseded Secrets boundaries.
replace_once(
    "DOCS/ASCENSION.md",
    "| 355–368 (356–369) | 100 | 3500 Silver |\n| 369–378 (370–379) | 300 | 3500 Silver |\n| 379–397 (380–398) | 600 | 3501 Gold |",
    "| 355–369 (356–370) | 100 | 3500 Silver |\n| 370–379 (371–380) | 300 | 3500 Silver |\n| 380–397 (381–398) | 600 | 3501 Gold |",
)
replace_once(
    "DOCS/ASCENSION.md",
    "Guias públicos de outras épocas/servidores também citam 100 de fama para criar\na Sub. Esse custo não existe no fluxo da Secrets 7.54 e, por isso, não foi\ninventado aqui. Eles confirmam, porém, as partes estruturais: nível 121,\nSephirot + Pedra Ideal, ambas as formas no mesmo slot, troca em cidade por\nPedra Misteriosa e bônus de pontos cruzado entre os níveis.",
    "A criação da Sub exige **100 de fama**, além do nível 121, Sephirot e Pedra\nIdeal. A regra segue o WYD KR/guias retail escolhidos para esta progressão. As\nduas formas continuam no mesmo slot e a troca em cidade usa Pedra Misteriosa.",
)
replace_once(
    "DOCS/CRAFTING.md",
    "| Alquimista Odin | `0x2D2` | runas, pedras secretas e refinacao avancada |",
    "| Alquimista Odin | `0x2D2` | runas, pedras secretas e refinacao avancada |",
)
# Append a concise Odin section if it is not already present.
craft = Path("DOCS/CRAFTING.md")
text = craft.read_text(encoding="utf-8")
marker = "## Odin: Pedras Secretas retail\n"
if marker not in text:
    text += "\n\n## Odin: Pedras Secretas retail\n\n- sete runas custam **2.000.000 gold** por tentativa;\n- uma das quatro sequencias exatas de Pedra Secreta usa **95%** de sucesso;\n- qualquer outra mistura valida de sete runas usa **5%** e, no sucesso, gera\n  aleatoriamente uma das quatro Pedras Secretas;\n- sucesso ou falha consome as sete runas e o gold;\n- a sequencia propria da Fury Stone continua sendo resolvida antes do fallback\n  aleatorio e nao e tratada como tentativa de Pedra Secreta.\n"
    craft.write_text(text, encoding="utf-8")

print("celestial native progression patch applied")
