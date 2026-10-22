#!/usr/bin/env bash
# Tests and a backup restore use dedicated disposable databases, never live accounts.
set -euo pipefail
test "$EUID" -eq 0
stage=$(realpath -- "${1:?staging directory is required}")
case "$stage" in /tmp/wyd-deploy-*) ;; *) exit 1 ;; esac
test -f "$stage/playerportal.test"
chmod 700 "$stage/playerportal.test"
runuser -u postgres -- psql -X -p 5433 -Atc "SELECT datname FROM pg_database WHERE datname IN ('wyd_portal_validation','wyd_restore_validation')" | grep -q . && { echo 'Validation database already exists; refusing overwrite' >&2; exit 1; }
runuser -u postgres -- createdb -p 5433 -O wyd_game wyd_portal_validation
cleanup() {
    runuser -u postgres -- dropdb -p 5433 --if-exists wyd_portal_validation
    runuser -u postgres -- dropdb -p 5433 --if-exists wyd_restore_validation
}
trap cleanup EXIT
runuser -u postgres -- psql -X -p 5433 -d wyd_portal_validation -v ON_ERROR_STOP=1 -c 'REVOKE ALL ON DATABASE wyd_portal_validation FROM PUBLIC; GRANT CONNECT ON DATABASE wyd_portal_validation TO wyd_player;'
set -a
source /etc/wyd-go/game.env
source /etc/wyd-go/player.env
set +a
export WYD_TEST_PLAYER_OWNER_URL="${WYD_DATABASE_URL/\/wydgo?/\/wyd_portal_validation?}"
export WYD_TEST_PLAYER_DATABASE_URL="${WYD_PLAYER_DATABASE_URL/\/wydgo?/\/wyd_portal_validation?}"
export WYD_TEST_PLAYER_PRIVILEGES="$stage/config/player-privileges.sql"
"$stage/playerportal.test" -test.v -test.run '^TestPlayerDatabaseIntegration$'
unset WYD_DATABASE_URL WYD_PLAYER_DATABASE_URL WYD_TEST_PLAYER_OWNER_URL WYD_TEST_PLAYER_DATABASE_URL
systemctl start wyd-backup.service
archive=$(find /var/backups/wyd-go -maxdepth 1 -type f -name 'wydgo-*.dump' | sort | tail -n 1)
test -n "$archive"
runuser -u postgres -- createdb -p 5433 -O wyd_game wyd_restore_validation
runuser -u postgres -- /usr/lib/postgresql/18/bin/pg_restore -p 5433 --no-owner --exit-on-error -d wyd_restore_validation < "$archive"
restored=$(runuser -u postgres -- psql -X -p 5433 -d wyd_restore_validation -v ON_ERROR_STOP=1 -Atc 'SELECT count(*) FROM accounts')
printf 'Restored accounts: %s
' "$restored"
# Restore-test receipt for the staff panel (read by wyd-log-reader).
install -d -m 755 /var/lib/wyd-go-backup
printf '{"at":"%s","archive":"%s","restoredAccounts":%s}
'     "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$(basename -- "$archive")" "$restored" > /var/lib/wyd-go-backup/restore.json.partial
chmod 644 /var/lib/wyd-go-backup/restore.json.partial
mv -- /var/lib/wyd-go-backup/restore.json.partial /var/lib/wyd-go-backup/restore.json
