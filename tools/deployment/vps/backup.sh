#!/usr/bin/env bash
set -euo pipefail
umask 077
archive="/var/backups/wyd-go/wydgo-$(date -u +%Y%m%dT%H%M%SZ).dump"
test -d /var/backups/wyd-go
runuser -u postgres -- /usr/lib/postgresql/18/bin/pg_dump -p 5433 -Fc wydgo > "${archive}.partial"
/usr/lib/postgresql/18/bin/pg_restore --list "${archive}.partial" >/dev/null
mv -- "${archive}.partial" "$archive"
printf 'WYD database backup completed.\n'
