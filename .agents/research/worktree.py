"""Working-tree file inventory shared by the research matrix generators.

`git ls-files` alone lists only the index: it misses new files that are not
committed yet and still lists files deleted in the working tree. The matrices
describe the working tree, so they take tracked plus untracked (not ignored)
files and keep only those that exist. A path that exists but cannot be read
is not hidden here; the caller's read raises.
"""
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def files(prefix, suffixes=None, root=ROOT):
    """Existing working-tree files under prefix (tracked and untracked, not ignored)."""
    listed = subprocess.run(['git', 'ls-files', '--cached', '--others', '--exclude-standard', '--', prefix],
                            cwd=root, capture_output=True, text=True, encoding='utf-8', check=True).stdout
    result = []
    seen = set()
    for rel in listed.splitlines():
        if rel in seen or (suffixes and not rel.endswith(tuple(suffixes))):
            continue
        seen.add(rel)
        path = root / rel
        if path.is_file():
            result.append(path)
    return sorted(result)


def relative(prefix, suffixes=None, root=ROOT):
    """Same inventory as files(), as POSIX paths relative to root."""
    return [p.relative_to(root).as_posix() for p in files(prefix, suffixes, root)]
