#!/usr/bin/env bash
set -euo pipefail
umask 077
archive="/var/backups/wyd-go/wydgo-$(date -u +%Y%m%dT%H%M%SZ).dump"
test -d /var/backups/wyd-go
runuser -u postgres -- /usr/lib/postgresql/18/bin/pg_dump -p 5433 -Fc wydgo > "${archive}.partial"
/usr/lib/postgresql/18/bin/pg_restore --list "${archive}.partial" >/dev/null
mv -- "${archive}.partial" "$archive"
# Receipt for the staff panel (read by wyd-log-reader). No credentials or
# dump content: only time, file name, size and checksum.
install -d -m 755 /var/lib/wyd-go-backup
bytes=$(stat -c %s -- "$archive")
checksum=$(sha256sum -- "$archive" | cut -d' ' -f1)
printf '{"at":"%s","archive":"%s","bytes":%s,"sha256":"%s"}
'     "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$(basename -- "$archive")" "$bytes" "$checksum" > /var/lib/wyd-go-backup/status.json.partial
chmod 644 /var/lib/wyd-go-backup/status.json.partial
mv -- /var/lib/wyd-go-backup/status.json.partial /var/lib/wyd-go-backup/status.json
printf 'WYD database backup completed.\n'
