from pathlib import Path

p = Path("internal/game/celestial_test.go")
text = p.read_text(encoding="utf-8")
old = '''\tp.Char.Extended.Level = 120
\tp.Char.Equip[sefirotSlot] = model.Item{Index: 1760}
\tp.Char.Inv[0] = model.Item{Index: idealStoneItem}
\tw.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
\tp.X, p.Y = 100, 100
'''
new = '''\tp.Char.Extended.Level = 120
\tp.Char.Equip[sefirotSlot] = model.Item{Index: 1760}
\tp.Char.Inv[0] = model.Item{Index: idealStoneItem}
\tp.SpecialCoins = map[string]uint32{fameCounter: subCelestialFameCost}
\tw.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
\tp.X, p.Y = 100, 100
'''
if text.count(old) != 1:
    raise SystemExit(f"expected one Mysterious Stone fixture, found {text.count(old)}")
p.write_text(text.replace(old, new, 1), encoding="utf-8")
