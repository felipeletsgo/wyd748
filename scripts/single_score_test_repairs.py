from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]

# Compatibility-only regression files are invalid by construction now.
for rel in [
    "internal/model/hp_buff_scale_test.go",
]:
    path = root / rel
    if path.exists():
        path.unlink()

# Model tests must assert one canonical Score, never a legacy projection.
path = root / "internal/model/model_test.go"
text = path.read_text(encoding="utf-8")
for name in [
    "TestCompatibilityScoreCannotBePersistedAsJSON",
    "TestExtendedCompatibilityScoreIsProportionalAndSafe",
]:
    pattern = re.compile(rf"\nfunc {name}\(t \*testing\.T\) \{{.*?\n\}}\n", re.S)
    text = pattern.sub("\n", text, count=1)

# Fix an accidentally tautological persistence assertion from the migration.
text = text.replace(
    'if bytes.Contains(data, []byte(`"score":`)) || !bytes.Contains(data, []byte(`"score":`)) {',
    'if !bytes.Contains(data, []byte(`"score":`)) || bytes.Contains(data, []byte(`"extendedScore":`)) {',
)

# NPC regression now verifies the actual canonical values only.
pattern = re.compile(r"\nfunc TestNPCUsesExtendedStatsAndOnlyProjectsAtWireBoundary\(t \*testing\.T\) \{.*?\n\}\n", re.S)
replacement = r'''
func TestNPCUsesCanonicalScoreWithoutProjection(t *testing.T) {
	def := NPCDef{
		Score: &Score{
			Version: ScoreVersion,
			Level:   400, Attack: 350_000, MagicAttack: 420_000, Defense: 275_000,
			MaxHP: 1_000_000, MaxMP: 500_000,
			Str: 180_000, Int: 190_000, Dex: 170_000, Con: 200_000,
			Mastery: [4]uint32{300, 301, 302, 303},
		},
	}
	score := def.MakeScore(750_000)
	if score.Attack != 350_000 || score.MagicAttack != 420_000 ||
		score.Defense != 275_000 || score.CurHP != 750_000 || score.MaxHP != 1_000_000 ||
		score.Mastery[0] != 300 {
		t.Fatalf("NPC perdeu valores do Score canonico: %+v", score)
	}
}
'''
text, count = pattern.subn("\n" + replacement + "\n", text, count=1)
if count == 0:
    # A prior repair may already have renamed it; leave the canonical version intact.
    pass
path.write_text(text, encoding="utf-8", newline="\n")

# npcconvert must verify Merchant/Direction merge in the canonical Score itself.
path = root / "cmd/npcconvert/main_test.go"
text = path.read_text(encoding="utf-8")
old = '''\tif score := got.MakeScore(got.Score.MaxHP).CompatibilityScore(); score.Merchant != 0xFB {\n\t\tt.Fatalf("merchant visual=0x%X, quer 0xFB", score.Merchant)\n\t}'''
new = '''\tif score := got.MakeScore(got.Score.MaxHP); score.Merchant != 0xFB {\n\t\tt.Fatalf("merchant canonico=0x%X, quer 0xFB", score.Merchant)\n\t}'''
text = text.replace(old, new)
text = text.replace("extended=%+v", "score=%+v")
path.write_text(text, encoding="utf-8", newline="\n")

print("obsolete score compatibility tests removed")
