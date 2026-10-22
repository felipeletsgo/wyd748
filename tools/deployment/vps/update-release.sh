#!/usr/bin/env bash
# Installs a release built by Build-VpsRelease.ps1 over an existing deployment.
# Account data and /var/lib/wyd-go state are never reset. Schema migration
# belongs to the authoritative writer; additions include security counters
# and narrow, read-only character progression and public guild views.
# PIN migration requires an explicit --migrate-admin-pin flag and a backup. A
# service restarts only when its files or configuration changed. A failed
# Caddy validation or readiness check restores the previous release.
set -euo pipefail
umask 022
test "$EUID" -eq 0
migrate_pin=false
case "${2:-}" in
    '') ;;
    --migrate-admin-pin) migrate_pin=true ;;
    *) echo 'Unknown release option' >&2; exit 1 ;;
esac
stage=$(realpath -- "${1:?staging directory is required}")
[[ "$stage" =~ ^/tmp/wyd-deploy-[A-Za-z0-9-]+$ ]] || { echo 'Invalid staging directory' >&2; exit 1; }
test -L /srv/wyd-go/current
test -f /etc/wyd-go/game.env
test -f /etc/wyd-go/player.env
previous_schema=$(runuser -u postgres -- psql -X -p 5433 -d wydgo -Atc 'SELECT max(version) FROM schema_migrations')
cd "$stage"
sha256sum --check --strict release.sha256
grep -q '^wyd.forjago.com {' config/wyd-site.caddy
systemctl start wyd-backup.service

previous=$(readlink -f /srv/wyd-go/current)
stamp=$(date -u +%Y%m%dT%H%M%SZ)
release="/srv/wyd-go/releases/$stamp"
test ! -e "$release"
install -d -m 755 "$release"
tar -xzf release.tar.gz -C "$release" --no-same-owner
# Archives built on Windows carry 0777 directory modes; release trees stay root-owned and read-only.
chmod -R u=rwX,go=rX "$release"
chmod 755 "$release/game/wyd-server" "$release/game/wyd-player-api" "$release/game/wyd-log-reader"

restart_game=false
restart_player=false
game_touched=false
player_touched=false
diff -rq --exclude=wyd-player-api --exclude=wyd-log-reader "$previous/game" "$release/game" >/dev/null || restart_game=true
restart_log_reader=false
cmp -s "$previous/game/wyd-log-reader" "$release/game/wyd-log-reader" || restart_log_reader=true
cmp -s "$previous/game/wyd-player-api" "$release/game/wyd-player-api" || restart_player=true
cmp -s /etc/wyd-go/server.txt config/server.txt || restart_game=true
for unit in wyd-game.service wyd-player.service wyd-backup.service wyd-backup.timer; do
    if ! cmp -s "/etc/systemd/system/$unit" "config/$unit"; then
        restart_game=true
        restart_player=true
    fi
done

cp -p /etc/wyd-go/server.txt "/etc/wyd-go/server.txt.pre-$stamp"
cp -p /etc/wyd-go/game.env "/etc/wyd-go/game.env.pre-$stamp"
cp -p /etc/wyd-go/player.env "/etc/wyd-go/player.env.pre-$stamp"
cp -p /etc/wyd-go/staff.json "/etc/wyd-go/staff.json.pre-$stamp"
cp -p /etc/caddy/wyd-site.caddy "/etc/caddy/wyd-site.caddy.pre-$stamp"
mkdir -p "/etc/systemd/system/.wyd-pre-$stamp"
cp -p /etc/systemd/system/wyd-*.service /etc/systemd/system/wyd-*.timer "/etc/systemd/system/.wyd-pre-$stamp/"

switch_to() {
    ln -sfn "$1" /srv/wyd-go/current.new
    mv -Tf /srv/wyd-go/current.new /srv/wyd-go/current
}
rollback() {
    trap - ERR
    echo "Restoring release $previous" >&2
    if [ "$game_touched" = true ]; then systemctl stop wyd-game || true; fi
    if [ "$player_touched" = true ]; then systemctl stop wyd-player || true; fi
    # Versions 6-11 add counter/staff tables and read-only views, not account
    # transformations. Keep these additions, but restore the expected marker
    # before starting an older binary. Never rewind an unknown migration.
    for version in 11 10 9; do
        if [ "$game_touched" = true ] && [ -n "$previous_schema" ] && [ "$previous_schema" -ge 8 ] && [ "$previous_schema" -lt "$version" ]; then
            runuser -u postgres -- psql -X -p 5433 -d wydgo -v ON_ERROR_STOP=1 -c                 "DELETE FROM schema_migrations WHERE version=$version AND (SELECT max(version) FROM schema_migrations)=$version" || true
        fi
    done
    if [ "$game_touched" = true ] && { [ "$previous_schema" = 5 ] || [ "$previous_schema" = 6 ] || [ "$previous_schema" = 7 ]; }; then
        runuser -u postgres -- psql -X -p 5433 -d wydgo -v ON_ERROR_STOP=1 -c \
            'DELETE FROM schema_migrations WHERE version=8 AND (SELECT max(version) FROM schema_migrations)=8' || true
    fi
    if [ "$game_touched" = true ] && { [ "$previous_schema" = 5 ] || [ "$previous_schema" = 6 ]; }; then
        runuser -u postgres -- psql -X -p 5433 -d wydgo -v ON_ERROR_STOP=1 -c \
            'DELETE FROM schema_migrations WHERE version=7 AND (SELECT max(version) FROM schema_migrations)=7' || true
        if [ "$previous_schema" = 5 ]; then
            runuser -u postgres -- psql -X -p 5433 -d wydgo -v ON_ERROR_STOP=1 -c \
                'DELETE FROM schema_migrations WHERE version=6 AND (SELECT max(version) FROM schema_migrations)=6' || true
        fi
    fi
    switch_to "$previous"
    cp -p "/etc/wyd-go/server.txt.pre-$stamp" /etc/wyd-go/server.txt
    cp -p "/etc/wyd-go/game.env.pre-$stamp" /etc/wyd-go/game.env
    cp -p "/etc/wyd-go/player.env.pre-$stamp" /etc/wyd-go/player.env
    cp -p "/etc/wyd-go/staff.json.pre-$stamp" /etc/wyd-go/staff.json
    cp -p "/etc/caddy/wyd-site.caddy.pre-$stamp" /etc/caddy/wyd-site.caddy
    cp -p "/etc/systemd/system/.wyd-pre-$stamp/"* /etc/systemd/system/
    systemctl daemon-reload
    if [ "$game_touched" = true ]; then systemctl restart wyd-game || true; fi
    if [ "$player_touched" = true ]; then systemctl restart wyd-player || true; fi
    systemctl reload caddy || true
}
trap 'rollback' ERR

# Public artifacts: the Guilds.txt export and the launcher patch tree. The
# patch list grows with each release, so an existing update.htm is kept.
install -d -o wyd-game -g wyd-game -m 755 /var/lib/wyd-go-public
install -d -o wyd-player -g wyd-player -m 755 /var/lib/wyd-go-guildmark
if [ ! -e /var/lib/wyd-go-public/Guilds.txt ]; then
    install -o wyd-game -g wyd-game -m 644 /dev/null /var/lib/wyd-go-public/Guilds.txt
fi
install -d -m 755 /srv/wyd-go/patch
if [ ! -e /srv/wyd-go/patch/update.htm ]; then
    install -m 644 config/update.htm /srv/wyd-go/patch/update.htm
fi
# Staff panel: capabilities are versioned; the PIN is generated once here and
# never leaves this host (read it with: sudo grep WYD_ADMIN_PIN /etc/wyd-go/game.env).
install -m 640 -g wyd-game config/staff.json /etc/wyd-go/staff.json
pin_options=()
if [ "$migrate_pin" = true ]; then pin_options+=(--migrate); fi
python3 config/admin-pin.py /etc/wyd-go/game.env "${pin_options[@]}"
python3 config/set-turnstile-secret.py --from-player-env
install -m 700 config/set-turnstile-secret.py /usr/local/sbin/wyd-set-turnstile-secret
cmp -s "/etc/wyd-go/game.env.pre-$stamp" /etc/wyd-go/game.env || restart_game=true
install -m 644 config/server.txt /etc/wyd-go/server.txt
install -m 644 config/wyd-game.service config/wyd-player.service config/wyd-backup.service config/wyd-backup.timer /etc/systemd/system/
install -m 700 config/backup.sh /usr/local/sbin/wyd-backup
install -m 644 config/wyd-site.caddy /etc/caddy/wyd-site.caddy
# Staff panel log reader: its own user and socket. Each step only warns on
# failure (|| keeps the ERR rollback trap out of it): the helper never
# decides whether the game release goes live.
log_reader_warning() { echo "Warning: log reader $1 failed; the game release is unaffected" >&2; }
if ! id wyd-log-reader >/dev/null 2>&1; then
    useradd --system --no-create-home --shell /usr/sbin/nologin wyd-log-reader || log_reader_warning 'user creation'
fi
install -d -m 755 /var/lib/wyd-go-backup || log_reader_warning 'receipt directory'
install -m 644 config/wyd-log-reader.service config/wyd-log-reader.socket /etc/systemd/system/ || log_reader_warning 'unit installation'
systemctl daemon-reload
if ! caddy validate --config /etc/caddy/Caddyfile --adapter caddyfile; then
    rollback
    echo 'Caddy validation failed; previous release restored' >&2
    exit 1
fi
# server.txt binds the game listener publicly; admit only its TCP port.
ufw allow 8281/tcp comment 'WYD game' >/dev/null

switch_to "$release"
# The operator must not disconnect active players to publish a web update.
if [ "$restart_game" = true ]; then
    if [ -n "$(ss -Htn state established '( sport = :8281 )')" ]; then
        echo 'Active game connections prevent a safe restart' >&2
        # No service was restarted; restore only configuration and the symlink.
        restart_game=false
        restart_player=false
        rollback
        exit 1
    fi
    player_touched=true
    systemctl stop wyd-player
    game_touched=true
    systemctl restart wyd-game
fi
# The player role may write only security counters, never game account state.
schema_ready=false
for attempt in {1..15}; do
    if runuser -u postgres -- psql -X -p 5433 -d wydgo -Atc "SELECT to_regclass('public.web_auth_limits') IS NOT NULL AND to_regclass('public.player_character_progression') IS NOT NULL AND to_regclass('public.player_public_guilds') IS NOT NULL" | grep -qx t; then schema_ready=true; break; fi
    sleep 1
done
test "$schema_ready" = true
runuser -u postgres -- psql -X -p 5433 -d wydgo -v ON_ERROR_STOP=1 -c \
    'GRANT SELECT, INSERT, UPDATE, DELETE ON public.web_auth_limits TO wyd_player; GRANT SELECT ON public.player_character_progression, public.player_public_guilds TO wyd_player'
if [ "$restart_game" = true ]; then restart_player=true; fi
if [ "$restart_player" = true ]; then player_touched=true; systemctl restart wyd-player; fi
systemctl reload caddy
systemctl enable --now wyd-log-reader.socket || log_reader_warning 'socket start'
if [ "$restart_log_reader" = true ]; then systemctl try-restart wyd-log-reader.service || log_reader_warning 'restart'; fi

ready=false
for attempt in {1..15}; do
    if curl --fail --silent -o /dev/null -H 'Host: wyd.forjago.com' http://127.0.0.1:8083/readyz &&
        curl --fail --silent -o /dev/null -H 'Host: wyd.forjago.com' http://127.0.0.1:8082/readyz &&
        systemctl is-active --quiet wyd-game &&
        ss -lnt '( sport = :8281 )' | grep -q LISTEN; then
        ready=true
        break
    fi
    sleep 1
done
if [ "$ready" != true ]; then
    rollback
    echo 'Readiness check failed; previous release restored' >&2
    exit 1
fi
rm -rf -- "$stage"
printf 'Release %s is live (game restarted: %s, player API restarted: %s).\n' "$release" "$restart_game" "$restart_player"
