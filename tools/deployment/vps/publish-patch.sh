#!/usr/bin/env bash
# Installs one launcher patch uploaded by Publish-ClientPatch.ps1. The ZIP is
# installed first and update.htm is replaced last, so the launcher never
# reads a listed patch that is missing. The previous list is kept.
set -euo pipefail
umask 022
test "$EUID" -eq 0
stage=$(realpath -- "${1:?staging directory is required}")
zip_name=${2:?patch name is required}
[[ "$stage" =~ ^/tmp/wyd-patch-[0-9]{5}$ ]] || { echo 'Invalid staging directory' >&2; exit 1; }
[[ "$zip_name" =~ ^patch-[0-9]{5}\.zip$ ]]
root=/srv/wyd-go/patch
install -d -m 755 "$root"
# Serialize publishers and require the complete current list as a prefix.
exec 9>"$root/.publish.lock"
flock -x 9
test -f "$stage/$zip_name"
test -f "$stage/update.htm"
head -n 1 "$stage/update.htm" | grep -qx '\[WYD\]'
grep -qx "[0-9]* $zip_name" "$stage/update.htm"
python3 -m zipfile -t "$stage/$zip_name" >/dev/null
test ! -e "$root/$zip_name"
python3 - "$root/update.htm" "$stage/update.htm" "$zip_name" <<'PY'
import pathlib
import sys
old, new, name = sys.argv[1:]
previous = pathlib.Path(old).read_text(encoding='ascii').splitlines()
candidate = pathlib.Path(new).read_text(encoding='ascii').splitlines()
version = int(name[6:11])
if candidate != previous + [f'{version} {name}']:
    raise SystemExit('The patch list must append exactly one entry to the current feed')
if not 48002 <= version <= 49999 or version <= max(int(line.split()[0]) for line in previous[1:]):
    raise SystemExit('The patch version must increase')
PY

install -d -m 755 "$root"
install -m 644 "$stage/$zip_name" "$root/$zip_name"
cp -p "$root/update.htm" "$root/.update.htm.previous"
install -m 644 "$stage/update.htm" "$root/.update.htm.new"
mv -f -- "$root/.update.htm.new" "$root/update.htm"
rm -rf -- "$stage"
echo "Published $zip_name"
