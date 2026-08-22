from pathlib import Path

root = Path(__file__).resolve().parents[1]

path = root / "internal/model/model.go"
text = path.read_text(encoding="utf-8")
old = "\tscore.CurHP = minScoreValue(currentHP, score.MaxHP)"
new = "\tscore.CurHP = currentHP\n\tif score.CurHP > score.MaxHP {\n\t\tscore.CurHP = score.MaxHP\n\t}"
if old in text:
    text = text.replace(old, new, 1)
path.write_text(text, encoding="utf-8", newline="\n")

print("single-score repairs applied")
