#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def patch(path, old, new, label):
    file = ROOT / path
    text = file.read_text(encoding="utf-8")
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{label}: expected 1 occurrence, found {count}")
    file.write_text(text.replace(old, new, 1), encoding="utf-8")


patch(
    "internal/game/crafting_test.go",
    '''func TestAylinNativeChanceBoundary(t *testing.T) {\n\ttests := []struct {\n\t\troll int\n\t\twant bool\n\t}{\n\t\t{roll: -1, want: false},\n\t\t{roll: 0, want: true},\n\t\t{roll: 39, want: true},\n\t\t{roll: 40, want: true},\n\t\t{roll: 41, want: false},\n\t\t{roll: 99, want: false},\n\t\t{roll: 100, want: false},\n\t}\n\tfor _, test := range tests {\n\t\tif got := aylinRollSucceeds(test.roll); got != test.want {\n\t\t\tt.Errorf("roll=%d: got=%t want=%t", test.roll, got, test.want)\n\t\t}\n\t}\n}\n''',
    '''func TestAylinChanceUsesOneToHundredBoundary(t *testing.T) {\n\tw := &World{rng: fixedRNG{value: 39}} // visible roll 40\n\tif got := w.rollPercent(40); !got.Success || got.Roll != 40 {\n\t\tt.Fatalf("roll 40/40 should succeed: %+v", got)\n\t}\n\tw.rng = fixedRNG{value: 40} // visible roll 41\n\tif got := w.rollPercent(40); got.Success || got.Roll != 41 {\n\t\tt.Fatalf("roll 41/40 should fail: %+v", got)\n\t}\n}\n''',
    "Aylin test migration",
)

patch(
    "internal/game/consumables_test.go",
    '''// TestRefineRollCertainAtZero confirma que +0 -> +1 tem chance 100 (sempre) e que\n// um sanc fora da tabela nunca refina.\nfunc TestRefineRollCertainAtZero(t *testing.T) {\n\tfor i := 0; i < 200; i++ {\n\t\tif !refineRoll(0) {\n\t\t\tt.Fatal("refino de +0 deveria ser sempre bem-sucedido (chance 100)")\n\t\t}\n\t}\n\tif refineRoll(15) || refineRoll(-1) {\n\t\tt.Fatal("sanc fora da tabela nunca deveria refinar")\n\t}\n}\n''',
    '''// TestRefineChanceCertainAtZero confirma que +0 -> +1 tem chance 100 e que\n// sanc fora da tabela recebe chance zero.\nfunc TestRefineChanceCertainAtZero(t *testing.T) {\n\tif got := refineChance(0); got != 100 {\n\t\tt.Fatalf("chance de +0=%d, quer 100", got)\n\t}\n\tif got := refineChance(15); got != 5 {\n\t\tt.Fatalf("chance de +15=%d, quer 5", got)\n\t}\n\tif refineChance(16) != 0 || refineChance(-1) != 0 {\n\t\tt.Fatal("sanc fora da tabela deveria ter chance zero")\n\t}\n\tw := &World{rng: fixedRNG{value: 99}} // visible roll 100\n\tif roll := w.rollPercent(refineChance(0)); !roll.Success || roll.Roll != 100 {\n\t\tt.Fatalf("100/100 deveria ser sucesso: %+v", roll)\n\t}\n}\n''',
    "refinement test migration",
)

patch(
    "internal/game/volatile_actions_test.go",
    '''func TestMountEssenceLevelsAndEvolves(t *testing.T) {\n\t// Cria Pig (2330) no level 24: um amago (2390) casa e sobe pra 25 -> evolui pra\n\t// adulta (2360), level zera, longevidade sobe.\n\tmount := model.Item{Index: 2330}\n\tmount.SetMountLevel(24)\n\tok, _ := mountEssence(&mount, 2390)\n\tif !ok {\n\t\tt.Fatal("amago casado deveria funcionar")\n\t}\n\tif mount.Index != 2360 || mount.MountLevel() != 0 {\n\t\tt.Fatalf("apos evolucao index=%d level=%d, quer 2360/0", mount.Index, mount.MountLevel())\n\t}\n\tif mount.MountLongev() < 1 {\n\t\tt.Fatalf("longevidade deveria subir na evolucao, veio %d", mount.MountLongev())\n\t}\n\t// Amago de tipo errado e recusado.\n\tother := model.Item{Index: 2360}\n\tif ok, _ := mountEssence(&other, 2391); ok {\n\t\tt.Fatal("amago de outro tipo nao deveria casar")\n\t}\n}\n''',
    '''func TestMountEssenceLevelsAndEvolves(t *testing.T) {\n\t// Cria Pig (2330) no level 24: um amago (2390) casa e sobe pra 25 -> evolui pra\n\t// adulta (2360), level zera, longevidade sobe. Cria nao possui gate percentual.\n\tw := &World{rng: fixedRNG{value: 0}}\n\tmount := model.Item{Index: 2330}\n\tmount.SetMountLevel(24)\n\toutcome := w.mountEssence(&mount, 2390)\n\tif !outcome.OK {\n\t\tt.Fatal("amago casado deveria funcionar")\n\t}\n\tif len(outcome.Rolls) != 0 {\n\t\tt.Fatalf("cria nao deveria rolar percentual: %+v", outcome.Rolls)\n\t}\n\tif mount.Index != 2360 || mount.MountLevel() != 0 {\n\t\tt.Fatalf("apos evolucao index=%d level=%d, quer 2360/0", mount.Index, mount.MountLevel())\n\t}\n\tif mount.MountLongev() < 1 {\n\t\tt.Fatalf("longevidade deveria subir na evolucao, veio %d", mount.MountLongev())\n\t}\n\t// Amago de tipo errado e recusado.\n\tother := model.Item{Index: 2360}\n\tif outcome := w.mountEssence(&other, 2391); outcome.OK {\n\t\tt.Fatal("amago de outro tipo nao deveria casar")\n\t}\n}\n''',
    "mount essence test migration",
)

print("percentage tests migrated")
