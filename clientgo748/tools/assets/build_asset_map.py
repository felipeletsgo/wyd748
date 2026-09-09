#!/usr/bin/env python3
"""Build the product-oriented asset census used by the Go client plan.

The low-level Ghidra catalog remains the source for native functions. This
tool deliberately emits one deterministic row per asset family so the next
loader can be chosen from real inputs instead of guessed dependencies. It does
not parse proprietary formats and therefore never promotes an unknown family
to a contract merely because files exist.
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


FAMILIES = {
    ".ani": ("ANIM_ANI", "animation"),
    ".bin": ("TABLE_BIN", "auxiliary table"),
    ".bon": ("SKEL_BON", "skeleton"),
    ".csv": ("TABLE_CSV", "auxiliary table"),
    ".cur": ("CURSOR_CUR", "cursor"),
    ".dat": ("TABLE_DAT", "auxiliary table"),
    ".msa": ("EFFECT_MSA", "effect definition"),
    ".msh": ("MESH_MSH", "geometry"),
    ".trn": ("TERRAIN_TRN", "terrain"),
    ".txt": ("TABLE_TXT", "auxiliary table"),
    ".wys": ("EFFECT_WYS", "effect resource"),
    ".wyt": ("TEX_WYT", "texture"),
}

TEXT_SUFFIXES = {".csv", ".json", ".txt"}


def canonical_size(path: Path) -> int:
    """Return a checkout-independent size for text assets.

    Git may materialize text with CRLF on Windows and LF on Linux. The asset
    census describes package content, so it uses the same canonical LF view as
    the package manifest while leaving binary formats byte-exact.
    """

    data = path.read_bytes()
    if path.suffix.lower() in TEXT_SUFFIXES or path.name.lower() == "itemhelp.dat":
        return len(data.replace(b"\r\n", b"\n"))
    return len(data)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    # Defaults are resolved from the client package, not from the caller's
    # current directory. This keeps the documented root-level invocation
    # reproducible while still allowing explicit paths for focused audits.
    parser.add_argument("--root", type=Path)
    parser.add_argument(
        "--map-out", type=Path
    )
    parser.add_argument("--deps-out", type=Path)
    return parser.parse_args()


def write_map(root: Path, output: Path) -> int:
    totals: dict[str, tuple[int, int]] = {}
    for path in sorted(p for p in root.rglob("*") if p.is_file()):
        suffix = path.suffix.lower()
        count, size = totals.get(suffix, (0, 0))
        totals[suffix] = (count + 1, size + canonical_size(path))

    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
        writer.writerow(
            [
                "id",
                "family",
                "role",
                "file_count",
                "bytes",
                "depends_on",
                "used_by",
                "status",
                "go_target",
                "evidence",
            ]
        )
        for suffix in sorted(totals):
            count, size = totals[suffix]
            family_id, role = FAMILIES.get(
                suffix, (f"UNKNOWN_{suffix[1:].upper() or 'NOEXT'}", "unknown")
            )
            if suffix == ".wyt":
                status = "TESTED"
                go_target = "internal/assets"
                evidence = "references/research/flows/render-assets/initial-server-logo-texture.md"
            else:
                status = "UNKNOWN"
                go_target = "—"
                evidence = "pending native loader trace"
            writer.writerow(
                [
                    family_id,
                    suffix[1:].upper() or "(none)",
                    role,
                    count,
                    size,
                    "—",
                    "—",
                    status,
                    go_target,
                    evidence,
                ]
            )
    return sum(count for count, _ in totals.values())


def write_dependencies(output: Path) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
        writer.writerow(["source", "target", "relation", "evidence", "status"])
        # Keep this file intentionally sparse. A missing edge is safer than a
        # guessed loader dependency; new rows require a native/resource proof.
        writer.writerow(
            [
                "UI/logo1.wyt",
                "TEX_WYT",
                "materializes texture",
                "references/research/flows/render-assets/initial-server-logo-texture.md",
                "CONFIRMED",
            ]
        )


def main() -> int:
    args = parse_args()
    client_root = Path(__file__).resolve().parents[2]
    root = args.root if args.root is not None else client_root / "assets/current"
    map_out = args.map_out if args.map_out is not None else client_root / "references/assets/asset-map.tsv"
    deps_out = args.deps_out if args.deps_out is not None else client_root / "references/assets/asset-dependencies.tsv"
    if not root.is_dir():
        raise SystemExit(f"asset root does not exist: {root}")
    count = write_map(root, map_out)
    write_dependencies(deps_out)
    print(f"censused {count} files from {root}")
    print(f"wrote {map_out}")
    print(f"wrote {deps_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
