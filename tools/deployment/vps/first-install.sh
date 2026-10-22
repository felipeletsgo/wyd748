#!/usr/bin/env bash
# One-time native deployment. Refuse existing installations rather than reset data.
set -euo pipefail
umask 077
test "$EUID" -eq 0
stage=$(realpath -- "${1:?staging directory is required}")
case "$stage" in /tmp/wyd-deploy-*) ;; *) echo 'Invalid staging directory' >&2; exit 1 ;; esac
test -f "$stage/release.tar.gz"
test ! -e /etc/wyd-go/game.env
test ! -e /srv/wyd-go/current
test ! -d /etc/postgresql/18/main
test -f /etc/caddy/Caddyfile
test -f /etc/caddy/forjago-site.caddy
grep -q '^wyd.forjago.com {' "$stage/config/wyd-site.caddy"
cd "$stage"
sha256sum --check "$stage/release.sha256"

export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -y --no-install-recommends curl ca-certificates postgresql-common python3
install -d -m 755 /usr/share/postgresql-common/pgdg
curl --fail --silent --show-error https://www.postgresql.org/media/keys/ACCC4CF8.asc -o /usr/share/postgresql-common/pgdg/apt.postgresql.org.asc
printf '%s\n' 'Types: deb' 'URIs: https://apt.postgresql.org/pub/repos/apt' 'Suites: noble-pgdg' 'Architectures: amd64' 'Components: main' 'Signed-By: /usr/share/postgresql-common/pgdg/apt.postgresql.org.asc' > /etc/apt/sources.list.d/wyd-pgdg.sources
apt-get update -qq
apt-get install -y --no-install-recommends postgresql-18
pg_ctlcluster 18 main stop
install -m 644 "$stage/config/postgresql.conf" /etc/postgresql/18/main/conf.d/wyd.conf
install -d -m 755 /etc/systemd/system/postgresql@18-main.service.d
printf '%s\n' '[Service]' 'MemoryMax=600M' 'CPUQuota=100%' > /etc/systemd/system/postgresql@18-main.service.d/wyd-limits.conf
systemctl daemon-reload
systemctl restart postgresql@18-main

for principal in wyd-game wyd-player wyd-log-reader; do
    if ! id "$principal" >/dev/null 2>&1; then
        useradd --system --no-create-home --shell /usr/sbin/nologin "$principal"
    fi
done
install -d -m 755 /etc/wyd-go
install -d -m 700 /var/backups/wyd-go
install -d -o wyd-game -g wyd-game -m 700 /var/lib/wyd-go /var/lib/wyd-go/export
export WYD_GAME_SECRET WYD_PLAYER_SECRET
WYD_GAME_SECRET=$(openssl rand -hex 32)
WYD_PLAYER_SECRET=$(openssl rand -hex 32)
runuser --preserve-environment -u postgres -- psql -X -p 5433 -v ON_ERROR_STOP=1 <<'SQL'
\getenv game_secret WYD_GAME_SECRET
\getenv player_secret WYD_PLAYER_SECRET
SELECT format('CREATE ROLE wyd_game LOGIN PASSWORD %L NOSUPERUSER NOCREATEDB NOCREATEROLE', :'game_secret') \gexec
SELECT format('CREATE ROLE wyd_player LOGIN PASSWORD %L NOSUPERUSER NOCREATEDB NOCREATEROLE', :'player_secret') \gexec
CREATE DATABASE wydgo OWNER wyd_game;
REVOKE ALL ON DATABASE wydgo FROM PUBLIC;
GRANT CONNECT ON DATABASE wydgo TO wyd_player;
ALTER ROLE wyd_player SET statement_timeout='3s';
ALTER ROLE wyd_player SET idle_in_transaction_session_timeout='5s';
SQL
printf 'WYD_DATABASE_URL=postgres://wyd_game:%s@127.0.0.1:5433/wydgo?sslmode=disable\n' "$WYD_GAME_SECRET" > /etc/wyd-go/game.env
printf 'WYD_PLAYER_DATABASE_URL=postgres://wyd_player:%s@127.0.0.1:5433/wydgo?sslmode=disable\n' "$WYD_PLAYER_SECRET" > /etc/wyd-go/player.env
unset WYD_GAME_SECRET WYD_PLAYER_SECRET
chmod 600 /etc/wyd-go/*.env
# Registration and published login fail closed until a real secret is installed.
# Supply it through non-echoed stdin; never put it in arguments or staging files.
python3 "$stage/config/set-turnstile-secret.py"
install -m 700 "$stage/config/set-turnstile-secret.py" /usr/local/sbin/wyd-set-turnstile-secret
# Public artifacts: the Guilds.txt export and the launcher patch tree. The
# patch list grows with each release, so an existing update.htm is kept.
install -d -o wyd-game -g wyd-game -m 755 /var/lib/wyd-go-public
install -d -o wyd-player -g wyd-player -m 755 /var/lib/wyd-go-guildmark
if [ ! -e /var/lib/wyd-go-public/Guilds.txt ]; then
    install -o wyd-game -g wyd-game -m 644 /dev/null /var/lib/wyd-go-public/Guilds.txt
fi
install -d -m 755 /srv/wyd-go/patch
if [ ! -e /srv/wyd-go/patch/update.htm ]; then
    install -m 644 "$stage/config/update.htm" /srv/wyd-go/patch/update.htm
fi
# Staff panel: capabilities are versioned; the PIN is generated once here and
# never leaves this host (read it with: sudo grep WYD_ADMIN_PIN /etc/wyd-go/game.env).
install -m 640 -g wyd-game "$stage/config/staff.json" /etc/wyd-go/staff.json
python3 "$stage/config/admin-pin.py" /etc/wyd-go/game.env
install -m 644 "$stage/config/server.txt" /etc/wyd-go/server.txt
release="/srv/wyd-go/releases/$(date -u +%Y%m%dT%H%M%SZ)"
install -d -m 755 "$release"
tar -xzf "$stage/release.tar.gz" -C "$release" --no-same-owner
# Archives built on Windows carry 0777 directory modes; release trees stay root-owned and read-only.
chmod -R u=rwX,go=rX "$release"
chmod 755 "$release/game/wyd-server" "$release/game/wyd-player-api" "$release/game/wyd-log-reader"
ln -s "$release" /srv/wyd-go/current
install -m 644 "$stage/config/wyd-game.service" "$stage/config/wyd-player.service" "$stage/config/wyd-backup.service" "$stage/config/wyd-backup.timer"     "$stage/config/wyd-log-reader.service" "$stage/config/wyd-log-reader.socket" /etc/systemd/system/
install -d -m 755 /var/lib/wyd-go-backup
install -m 700 "$stage/config/backup.sh" /usr/local/sbin/wyd-backup
systemctl daemon-reload
systemctl enable --now wyd-game
# The authoritative writer creates the versioned schema; do not invent a second migration path.
for attempt in {1..20}; do
    if runuser -u postgres -- psql -X -p 5433 -d wydgo -Atc "SELECT to_regclass('public.accounts') IS NOT NULL" | grep -qx t; then break; fi
    sleep 1
done
runuser -u postgres -- psql -X -p 5433 -d wydgo -v ON_ERROR_STOP=1 < "$stage/config/player-privileges.sql"
systemctl enable --now wyd-player wyd-backup.timer wyd-log-reader.socket
ready=false
for attempt in {1..10}; do
    if curl --fail --silent -H 'Host: wyd.forjago.com' http://127.0.0.1:8083/readyz; then ready=true; break; fi
    sleep 1
done
test "$ready" = true
systemctl start wyd-backup.service

backup="/etc/caddy/Caddyfile.pre-wyd-$(date -u +%Y%m%dT%H%M%SZ)"
cp -p /etc/caddy/Caddyfile "$backup"
install -m 644 "$stage/config/wyd-site.caddy" /etc/caddy/wyd-site.caddy
printf '\nimport /etc/caddy/wyd-site.caddy\n' >> /etc/caddy/Caddyfile
if ! caddy validate --config /etc/caddy/Caddyfile --adapter caddyfile; then
    cp -p "$backup" /etc/caddy/Caddyfile
    echo 'Caddy validation failed; original configuration restored without reload' >&2
    exit 1
fi
systemctl reload caddy
# server.txt binds the game listener publicly; admit only its TCP port.
ufw allow 8281/tcp comment 'WYD game'
printf '\nWYD initial native deployment completed; game port 8281 is public.\n'
