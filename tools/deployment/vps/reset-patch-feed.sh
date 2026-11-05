#!/usr/bin/env bash
# Reset the launcher feed to an owner-selected full-client baseline.
# Run only after the new full client is available for download. The retired
# feed and its patch ZIPs move to a timestamped archive outside the served
# tree; nothing is deleted. Usage: sudo bash reset-patch-feed.sh <update.htm> [baseline]
set -euo pipefail
umask 022
test "$EUID" -eq 0
base=$(realpath -- "${1:?baseline update.htm is required}")
version=${2:-48001}
[[ "$version" =~ ^[0-9]{5}$ ]] && (( version >= 48001 && version <= 49999 )) || { echo 'Invalid baseline version' >&2; exit 1; }
printf '[WYD]\n%s notice.txt\n' "$version" | cmp -s - "$base" || { echo 'Unexpected baseline feed' >&2; exit 1; }
root=/srv/wyd-go/patch
install -d -m 755 "$root"
exec 9>"$root/.publish.lock"
flock -x 9
archive=/srv/wyd-go/patch-archive/$(date -u +%Y%m%dT%H%M%SZ)
install -d -m 700 "$archive"
shopt -s nullglob
for file in "$root"/update.htm "$root"/.update.htm.previous "$root"/patch-*.zip; do
    mv -- "$file" "$archive/"
done
install -m 644 "$base" "$root/.update.htm.new"
mv -f -- "$root/.update.htm.new" "$root/update.htm"
echo "Feed reset to $version; previous feed archived in $archive"
