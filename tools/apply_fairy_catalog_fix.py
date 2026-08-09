from pathlib import Path

DURATIONS = {
    3900: 3,
    3901: 3,
    3902: 3,
    3903: 5,
    3904: 5,
    3905: 5,
    3906: 7,
    3907: 7,
    3908: 7,
    3911: 7,
    3912: 15,
    3913: 30,
    3914: 7,
    3915: 7,
}


def replace_once(text: str, old: str, new: str, guard: str) -> str:
    if old not in text:
        raise SystemExit(f"guard failed: {guard}")
    return text.replace(old, new, 1)


def update_itemlist() -> None:
    path = Path("data/itemlist.csv")
    raw = path.read_bytes()
    newline = b"\r\n" if b"\r\n" in raw else b"\n"
    lines = raw.decode("utf-8").splitlines()
    found = set()
    for n, line in enumerate(lines):
        if not line or line.startswith("#"):
            continue
        fields = line.split(",")
        try:
            index = int(fields[0].strip())
        except ValueError:
            continue
        if index not in DURATIONS:
            continue
        found.add(index)
        wanted = {
            "EF_WDAY": DURATIONS[index],
            "EF_HOUR": 0,
            "EF_MIN": 0,
        }
        for effect, value in wanted.items():
            pos = None
            for i in range(9, len(fields) - 1, 2):
                if fields[i].strip() == effect:
                    pos = i
                    break
            if pos is None:
                fields.extend([effect, str(value)])
            else:
                fields[pos + 1] = str(value)
        lines[n] = ",".join(fields)
    missing = sorted(set(DURATIONS) - found)
    if missing:
        raise SystemExit(f"fairy rows missing from itemlist: {missing}")
    path.write_bytes(newline.join(line.encode("utf-8") for line in lines) + newline)


def update_fairies_go() -> None:
    path = Path("internal/game/fairies.go")
    text = path.read_text(encoding="utf-8")
    text = replace_once(
        text,
        "// initializeFairyTimer reproduz BASE_SetDateFairy sem hardcode de duracao. A\n"
        "// primeira equipacao materializa no proprio item o saldo definido no itemlist.\n"
        "// Silver/Gold normalmente chegam por transformacao e preservam os tres efeitos;\n"
        "// se forem criadas diretamente sem timer, nao ganham uma duracao artificial.\n",
        "// initializeFairyTimer reproduz BASE_SetDateFairy sem hardcode de duracao. A\n"
        "// primeira equipacao materializa no proprio item o saldo definido no itemlist.\n"
        "// Silver/Gold transformadas preservam o saldo dinamico herdado; quando um item\n"
        "// nasce sem timer, usa a duracao estatica do catalogo como qualquer outra fada.\n",
        "fairies.go timer comment",
    )
    text = replace_once(
        text,
        "// Antes do primeiro tick depois de equipar, as fadas-base ainda carregam a\n"
        "// duracao apenas no itemlist. Elas ja devem conceder o bonus imediatamente.\n",
        "// Antes do primeiro tick depois de equipar, uma fada com duracao estatica\n"
        "// ainda carrega o saldo apenas no itemlist. O bonus ja deve valer imediatamente.\n",
        "fairies.go active bonus comment",
    )
    path.write_text(text, encoding="utf-8")


def update_fairies_test() -> None:
    path = Path("internal/game/fairies_test.go")
    text = path.read_text(encoding="utf-8")
    text = replace_once(
        text,
        "\tfor _, index := range []uint16{3911, 3912, 3913} {\n"
        "\t\titems[index] = fairyTestDef(index, 7)\n"
        "\t}\n"
        "\titems[3914] = fairyTestDef(3914, 0)\n"
        "\titems[3915] = fairyTestDef(3915, 0)\n",
        "\titems[3911] = fairyTestDef(3911, 7)\n"
        "\titems[3912] = fairyTestDef(3912, 15)\n"
        "\titems[3913] = fairyTestDef(3913, 30)\n"
        "\titems[3914] = fairyTestDef(3914, 7)\n"
        "\titems[3915] = fairyTestDef(3915, 7)\n",
        "fairyTestWorld",
    )
    text = replace_once(
        text,
        '{name: "silver", index: 3914, exp: 16, drop: 32, inherited: true}',
        '{name: "silver", index: 3914, exp: 16, drop: 32}',
        "silver bonus case",
    )
    text = replace_once(
        text,
        '{name: "gold", index: 3915, exp: 24, drop: 48, inherited: true}',
        '{name: "gold", index: 3915, exp: 24, drop: 48}',
        "gold bonus case",
    )
    text = replace_once(
        text,
        "func TestDirectSilverWithoutInheritedTimerIsInactive(t *testing.T) {\n"
        "\tw := fairyTestWorld()\n"
        "\tch := &model.Char{}\n"
        "\tch.Equip[fairySlot] = model.Item{Index: 3914}\n"
        "\tif got := w.activeFairyBonus(ch); got != (fairyBonus{}) {\n"
        "\t\tt.Fatalf(\"direct silver without timer must be inactive, got %+v\", got)\n"
        "\t}\n"
        "\tif w.hasActiveSilverFairy(ch) {\n"
        "\t\tt.Fatal(\"silver without inherited timer must not enable Water automation\")\n"
        "\t}\n"
        "}\n",
        "func TestDirectSilverUsesStaticCatalogDuration(t *testing.T) {\n"
        "\tw := fairyTestWorld()\n"
        "\tch := &model.Char{}\n"
        "\tch.Equip[fairySlot] = model.Item{Index: 3914}\n"
        "\tif got := w.activeFairyBonus(ch); got != (fairyBonus{expPercent: 16, dropPercent: 32}) {\n"
        "\t\tt.Fatalf(\"direct silver bonus=%+v\", got)\n"
        "\t}\n"
        "\tif !w.hasActiveSilverFairy(ch) {\n"
        "\t\tt.Fatal(\"direct silver with catalog duration must enable Water automation\")\n"
        "\t}\n"
        "\tp := &Player{Char: ch, InWorld: true}\n"
        "\tw.tickEquippedFairy(p, time.Unix(1_700_000_000, 0))\n"
        "\tremaining, ok := fairyTimerMinutes(ch.Equip[fairySlot], w.items[3914])\n"
        "\tif !ok || remaining != 7*24*60 {\n"
        "\t\tt.Fatalf(\"direct silver timer=%d ok=%v, want %d\", remaining, ok, 7*24*60)\n"
        "\t}\n"
        "}\n",
        "direct Silver regression test",
    )
    path.write_text(text, encoding="utf-8")


def update_water_test() -> None:
    path = Path("internal/game/water_auto_test.go")
    text = path.read_text(encoding="utf-8")
    text = replace_once(
        text,
        "func equipActiveSilverForWaterTest(t *testing.T, w *World, p *Player) {\n"
        "\tt.Helper()\n"
        "\tw.items[3914] = fairyTestDef(3914, 0)\n"
        "\tp.Char.Equip[fairySlot] = fairyWithMinutes(t, w, 3914, 60)\n"
        "}\n",
        "func equipActiveSilverForWaterTest(t *testing.T, w *World, p *Player) {\n"
        "\tt.Helper()\n"
        "\tw.items[3914] = fairyTestDef(3914, 7)\n"
        "\tp.Char.Equip[fairySlot] = model.Item{Index: 3914}\n"
        "}\n",
        "Water Silver helper",
    )
    text = replace_once(
        text,
        "\t\t{name: \"silver only in inventory\", setup: func(t *testing.T, w *World, leader, _ *Player) {\n"
        "\t\t\tw.items[3914] = fairyTestDef(3914, 0)\n"
        "\t\t\tleader.Char.Inv[6] = fairyWithMinutes(t, w, 3914, 60)\n"
        "\t\t}},\n",
        "\t\t{name: \"silver only in inventory\", setup: func(t *testing.T, w *World, leader, _ *Player) {\n"
        "\t\t\tw.items[3914] = fairyTestDef(3914, 7)\n"
        "\t\t\tleader.Char.Inv[6] = model.Item{Index: 3914}\n"
        "\t\t}},\n",
        "Water inventory Silver",
    )
    path.write_text(text, encoding="utf-8")


def update_catalog_test() -> None:
    path = Path("internal/data/catalog_test.go")
    text = path.read_text(encoding="utf-8")
    marker = (
        "\tif !foundAC {\n"
        "\t\tt.Fatalf(\"EF_AC do item 1103 nao carregado: %+v\", helmet.StaticEffects)\n"
        "\t}\n"
    )
    addition = marker + (
        "\tfairyDays := map[uint16]int{\n"
        "\t\t3900: 3, 3901: 3, 3902: 3,\n"
        "\t\t3903: 5, 3904: 5, 3905: 5,\n"
        "\t\t3906: 7, 3907: 7, 3908: 7,\n"
        "\t\t3911: 7, 3912: 15, 3913: 30,\n"
        "\t\t3914: 7, 3915: 7,\n"
        "\t}\n"
        "\tfor index, wantDays := range fairyDays {\n"
        "\t\tdef, ok := catalog.Items[index]\n"
        "\t\tif !ok {\n"
        "\t\t\tt.Fatalf(\"fada %d ausente do catalogo\", index)\n"
        "\t\t}\n"
        "\t\teffects := make(map[string]int, len(def.StaticEffects))\n"
        "\t\tfor _, effect := range def.StaticEffects {\n"
        "\t\t\teffects[effect.Name] = effect.Value\n"
        "\t\t}\n"
        "\t\tday, dayOK := effects[\"EF_WDAY\"]\n"
        "\t\thour, hourOK := effects[\"EF_HOUR\"]\n"
        "\t\tminute, minuteOK := effects[\"EF_MIN\"]\n"
        "\t\tif !dayOK || !hourOK || !minuteOK || day != wantDays || hour != 0 || minute != 0 {\n"
        "\t\t\tt.Fatalf(\"timer da fada %d invalido: effects=%v want=%dd 0h 0m\", index, effects, wantDays)\n"
        "\t\t}\n"
        "\t}\n"
    )
    text = replace_once(text, marker, addition, "catalog fairy timer test")
    path.write_text(text, encoding="utf-8")


update_itemlist()
update_fairies_go()
update_fairies_test()
update_water_test()
update_catalog_test()
