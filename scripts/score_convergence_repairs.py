from pathlib import Path

root = Path(__file__).resolve().parents[1]
path = root / "internal/model/model.go"
text = path.read_text(encoding="utf-8")
old = "\tdirection := n.Direction & 0xF0\n\tscore.Merchant = direction | (score.Merchant & 0x0F)"
new = "\tdirection := uint32(n.Direction & 0xF0)\n\tscore.Merchant = direction | (score.Merchant & 0x0F)"
if old not in text:
    raise RuntimeError("merchant direction boundary not found")
path.write_text(text.replace(old, new, 1), encoding="utf-8", newline="\n")
print("score convergence compile repairs applied")
