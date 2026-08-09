from pathlib import Path

p = Path('internal/game/crafting_advanced.go')
text = p.read_text(encoding='utf-8')
old = '{5126, 5122, 5121, 5114, 5125, 5111, 5118, 5334},'
new = '{5126, 5127, 5121, 5114, 5125, 5111, 5118, 5334},'
if text.count(old) != 1:
    raise SystemExit(f'expected exactly one old Water recipe, found {text.count(old)}')
p.write_text(text.replace(old, new, 1), encoding='utf-8')

p = Path('internal/game/celestial_native_progression_test.go')
text = p.read_text(encoding='utf-8')
anchor = 'func TestOdinSecretStoneCorrectSequenceUses95PercentAndCharges2M(t *testing.T) {'
test = '''func TestOdinSecretStoneRecipesMatchRetailRunes(t *testing.T) {\n\twant := [4][8]uint16{\n\t\t{5126, 5127, 5121, 5114, 5125, 5111, 5118, 5334},\n\t\t{5131, 5113, 5115, 5116, 5125, 5112, 5114, 5335},\n\t\t{5110, 5124, 5117, 5129, 5114, 5125, 5128, 5336},\n\t\t{5122, 5119, 5132, 5120, 5130, 5133, 5123, 5337},\n\t}\n\tif secretStoneRecipes != want {\n\t\tt.Fatalf("Secret Stone recipes=%v, want %v", secretStoneRecipes, want)\n\t}\n}\n\n'''
if text.count(anchor) != 1:
    raise SystemExit(f'expected one test anchor, found {text.count(anchor)}')
if 'TestOdinSecretStoneRecipesMatchRetailRunes' in text:
    raise SystemExit('regression test already exists')
p.write_text(text.replace(anchor, test + anchor, 1), encoding='utf-8')
print('Water Secret Stone recipe corrected')
