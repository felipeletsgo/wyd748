from pathlib import Path

root = Path(__file__).resolve().parents[1]
path = root / "internal/wire/codec.go"
text = path.read_text(encoding="utf-8")

text = text.replace(
    "func partyDisplayHP(hp, maxHP uint32) (uint16, uint16) {",
    "func partyDisplayHP(hp, maxHP uint32) (uint32, uint32) {",
)
text = text.replace(
    "\treturn packetU16(hp), packetU16(maxHP)\n}\n\nfunc putU16",
    "\treturn hp, maxHP\n}\n\nfunc putU16",
    1,
)
# PartyRequest and PartyMember write WORD fields explicitly; their inputs stay
# canonical uint32 values from Score.
text = text.replace("putU16(b, 16, maxHP)", "putU16(b, 16, packetU16(maxHP))")
text = text.replace("putU16(b, 18, hp)", "putU16(b, 18, packetU16(hp))")

path.write_text(text, encoding="utf-8", newline="\n")
print("party wire narrowing repaired")
