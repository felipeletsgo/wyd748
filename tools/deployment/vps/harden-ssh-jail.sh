#!/usr/bin/env bash
# Scope SSH bans without restarting any game, web or database service.
set -euo pipefail
test "$EUID" -eq 0
config="$(dirname -- "$(realpath -- "$0")")/zz-wyd-sshd.local"
target=/etc/fail2ban/jail.d/zz-wyd-sshd.local
test -f "$config"
systemctl is-active --quiet fail2ban
# The UFW action falls back to all ports when the application is missing.
# Fail closed instead of installing such a configuration.
ufw app info OpenSSH | grep -qx '  22/tcp'
stamp=$(date -u +%Y%m%dT%H%M%SZ)
backup="$target.pre-$stamp"
had_previous=false
if [ -e "$target" ]; then
    cp -p -- "$target" "$backup"
    had_previous=true
fi
restore() {
    if [ "$had_previous" = true ]; then
        cp -p -- "$backup" "$target"
    else
        mv -- "$target" "$target.rejected-$stamp"
    fi
    fail2ban-client reload sshd
}
install -o root -g root -m 644 "$config" "$target"
if ! fail2ban-client -t || ! fail2ban-client reload sshd; then
    restore
    echo 'SSH jail validation failed; previous configuration restored' >&2
    exit 1
fi
action=$(fail2ban-client get sshd action ufw actionban)
if ! grep -q 'to any app "OpenSSH"' <<< "$action"; then
    restore
    echo 'SSH-only action was not activated; previous configuration restored' >&2
    exit 1
fi
echo 'SSH bans are scoped to the verified OpenSSH profile (TCP 22).'
