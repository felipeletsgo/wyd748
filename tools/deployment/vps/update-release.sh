#!/usr/bin/env bash
# Installs a release built by Build-VpsRelease.ps1 over an existing deployment.
# The database and /var/lib/wyd-go state are never touched. PIN migration
# requires an explicit --migrate-admin-pin flag and has a root-only backup. A
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
cd "$stage"
sha256sum --check --strict release.sha256
grep -q '^wyd.forjago.com {' config/wyd-site.caddy

previous=$(readlink -f /srv/wyd-go/current)
stamp=$(date -u +%Y%m%dT%H%M%SZ)
release="/srv/wyd-go/releases/$stamp"
test ! -e "$release"
install -d -m 755 "$release"
tar -xzf release.tar.gz -C "$release" --no-same-owner
# Archives built on Windows carry 0777 directory modes; release trees stay root-owned and read-only.
chmod -R u=rwX,go=rX "$release"
chmod 755 "$release/game/wyd-server" "$release/game/wyd-player-api"

restart_game=false
restart_player=false
diff -rq --exclude=wyd-player-api "$previous/game" "$release/game" >/dev/null || restart_game=true
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
    switch_to "$previous"
    cp -p "/etc/wyd-go/server.txt.pre-$stamp" /etc/wyd-go/server.txt
    cp -p "/etc/wyd-go/game.env.pre-$stamp" /etc/wyd-go/game.env
    cp -p "/etc/wyd-go/staff.json.pre-$stamp" /etc/wyd-go/staff.json
    cp -p "/etc/caddy/wyd-site.caddy.pre-$stamp" /etc/caddy/wyd-site.caddy
    cp -p "/etc/systemd/system/.wyd-pre-$stamp/"* /etc/systemd/system/
    systemctl daemon-reload
    if [ "$restart_game" = true ]; then systemctl restart wyd-game || true; fi
    if [ "$restart_player" = true ]; then systemctl restart wyd-player || true; fi
    systemctl reload caddy || true
}
trap 'rollback' ERR

# Public artifacts: the Guilds.txt export and the launcher patch tree. The
# patch list grows with each release, so an existing update.htm is kept.
install -d -o wyd-game -g wyd-game -m 755 /var/lib/wyd-go-public
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
cmp -s "/etc/wyd-go/game.env.pre-$stamp" /etc/wyd-go/game.env || restart_game=true
install -m 644 config/server.txt /etc/wyd-go/server.txt
install -m 644 config/wyd-game.service config/wyd-player.service config/wyd-backup.service config/wyd-backup.timer /etc/systemd/system/
install -m 700 config/backup.sh /usr/local/sbin/wyd-backup
install -m 644 config/wyd-site.caddy /etc/caddy/wyd-site.caddy
systemctl daemon-reload
if ! caddy validate --config /etc/caddy/Caddyfile --adapter caddyfile; then
    rollback
    echo 'Caddy validation failed; previous release restored' >&2
    exit 1
fi
# server.txt binds the game listener publicly; admit only its TCP port.
ufw allow 8281/tcp comment 'WYD game' >/dev/null

switch_to "$release"
if [ "$restart_game" = true ]; then systemctl restart wyd-game; fi
if [ "$restart_player" = true ]; then systemctl restart wyd-player; fi
systemctl reload caddy

ready=false
for attempt in {1..15}; do
    if curl --fail --silent -o /dev/null -H 'Host: wyd.forjago.com' http://127.0.0.1:8083/readyz &&
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
