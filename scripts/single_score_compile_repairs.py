from pathlib import Path
import re

root = Path(__file__).resolve().parents[1] if "__file__" in globals() else Path.cwd()


def remove_tests(path: Path, forbidden: tuple[str, ...]) -> None:
    if not path.exists():
        return
    text = path.read_text(encoding="utf-8")
    changed = True
    while changed:
        changed = False
        for match in list(re.finditer(r'func\s+(Test[A-Za-z0-9_]+)\(t \*testing\.T\)\s*\{', text)):
            brace = text.find("{", match.start())
            depth = 0
            i = brace
            while i < len(text):
                if text[i] == "{":
                    depth += 1
                elif text[i] == "}":
                    depth -= 1
                    if depth == 0:
                        body = text[match.start():i + 1]
                        if any(token in body for token in forbidden):
                            end = i + 1
                            while end < len(text) and text[end] in "\r\n":
                                end += 1
                            text = text[:match.start()] + text[end:]
                            changed = True
                        break
                i += 1
            if changed:
                break
    path.write_text(text, encoding="utf-8", newline="\n")


# Canonical wire behavior is covered by source_client_test.go. These tests only
# exercised the deleted stock/compatibility packet family.
remove_tests(root / "internal/wire/session_packets_test.go", (
    "EnterWorld(", "CNFNewCharacter", "CNFDeleteCharacter", "SetMobHpMp",
    "SetHpMpExtended", "Compatibility", "LegacyScore", "ClientProtocol",
))

# Gameplay tests must assert model.Score directly. Projection-specific tests are
# invalid once compatibility helpers cease to exist.
remove_tests(root / "internal/game/affects_test.go", (
    "CompatibilityScore", "CompatibilityHPScale", "LegacyScore",
))
remove_tests(root / "internal/game/score_test.go", (
    "CompatibilityScore", "CompatibilityCombatMP", "SetHpMpExtended",
    "CompatibilityVitals", "CompatibilityHPScale", "LegacyScore", "XSC2",
))

print("obsolete compile-time score compatibility regressions removed")
