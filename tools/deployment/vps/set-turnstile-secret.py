#!/usr/bin/env python3
"""Install a Turnstile secret from stdin without printing or logging its value."""
import os
from pathlib import Path
import re
import sys
import tempfile


def configure(path: Path, secret: str) -> None:
    if not re.fullmatch(r"[A-Za-z0-9_-]{20,256}", secret):
        raise ValueError("Invalid Turnstile secret format")
    if secret in {f"{prefix}x0000000000000000000000000000000AA" for prefix in "123"}:
        raise ValueError("Production Turnstile cannot use a test secret")
    if path.is_symlink() or not path.is_file():
        raise ValueError("Environment must be an existing regular file")
    previous = path.read_text(encoding="utf-8")
    lines = [line for line in previous.splitlines() if not line.startswith("WYD_TURNSTILE_SECRET=")]
    value = "\n".join(lines) + "\nWYD_TURNSTILE_SECRET=" + secret + "\n"
    descriptor, temporary = tempfile.mkstemp(prefix=".player-env-", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as target:
            os.fchmod(target.fileno(), 0o600)
            target.write(value)
            target.flush()
            os.fsync(target.fileno())
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def main() -> None:
    if os.geteuid() != 0:
        raise ValueError("Root is required to configure authentication environments")
    # Both services validate tokens; preflight both paths before replacing either.
    game = Path("/etc/wyd-go/game.env")
    player = Path("/etc/wyd-go/player.env")
    if any(path.is_symlink() or not path.is_file() for path in (game, player)):
        raise ValueError("Authentication environments must already exist")
    if sys.argv[1:] == ["--from-player-env"]:
        values = [line.split('=', 1)[1] for line in player.read_text().splitlines()
                  if line.startswith("WYD_TURNSTILE_SECRET=")]
        if len(values) != 1:
            raise ValueError("Exactly one configured Turnstile secret is required")
        secret = values[0]
    elif not sys.argv[1:]:
        secret = sys.stdin.readline(258).rstrip("\r\n")
    else:
        raise ValueError("Unknown Turnstile installer option")
    configure(player, secret)
    configure(game, secret)
    print("Turnstile secret installed in root-only authentication environments")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError):
        print("Turnstile configuration failed; no secret value was logged", file=sys.stderr)
        sys.exit(1)
