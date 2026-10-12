#!/usr/bin/env python3
"""Maintain a six-digit staff PIN without printing credentials."""

import argparse
import os
from pathlib import Path
import re
import secrets
import tempfile


def configure(path: Path, migrate: bool = False) -> str:
    if path.is_symlink():
        raise ValueError("Credential file must not be a symlink")
    original = path.read_text(encoding="utf-8")
    lines = original.splitlines(keepends=True)
    indexes = [i for i, line in enumerate(lines) if line.startswith("WYD_ADMIN_PIN=")]
    if len(indexes) > 1:
        raise ValueError("Duplicate administrative PIN entries")
    if indexes:
        value = lines[indexes[0]].partition("=")[2].strip()
        if re.fullmatch(r"[0-9]{6}", value):
            return "unchanged"
        if not migrate:
            raise ValueError("Existing PIN requires explicit --migrate to six digits")
    pin = f"{secrets.randbelow(1_000_000):06d}"
    if indexes:
        lines[indexes[0]] = f"WYD_ADMIN_PIN={pin}\n"
    else:
        if lines and not lines[-1].endswith("\n"):
            lines[-1] += "\n"
        lines.append(f"WYD_ADMIN_PIN={pin}\n")
    stat = path.stat()
    descriptor, temporary = tempfile.mkstemp(prefix=".wyd-pin-", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as output:
            output.write("".join(lines))
            output.flush()
            os.fsync(output.fileno())
        os.chmod(temporary, 0o600)
        if hasattr(os, "chown"):
            os.chown(temporary, stat.st_uid, stat.st_gid)
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)
    return "migrated" if indexes else "created"


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", type=Path)
    parser.add_argument("--migrate", action="store_true")
    arguments = parser.parse_args()
    try:
        print(configure(arguments.path, arguments.migrate))
    except (ValueError, OSError) as error:
        parser.exit(1, f"Administrative PIN configuration failed: {error}\n")
