#!/usr/bin/env bash
# Switches the game between one process for both channels (wyd-game) and one
# process per channel (wyd-game@1, wyd-game@2) (W9 phase 5). The channel
# configurations are derived from /etc/wyd-go/server.txt. The switch refuses
# while a player is connected and restores the single process when the channel
# processes do not become ready. Usage: wyd-channel-processes enable|disable
set -euo pipefail
test "$EUID" -eq 0
mode=${1:?enable or disable}
test -f /etc/systemd/system/wyd-game@.service
test -x /usr/local/sbin/wyd-channel-configs

ready() {
    local unit
    for unit in "$@"; do systemctl is-active --quiet "$unit" || return 1; done
    ss -lnt '( sport = :8281 )' | grep -q LISTEN &&
        ss -lnt '( sport = :8282 )' | grep -q LISTEN &&
        curl --fail --silent -o /dev/null -H 'Host: wyd.forjago.com' http://127.0.0.1:8082/readyz
}
wait_ready() {
    local attempt
    for attempt in {1..30}; do
        if ready "$@"; then return 0; fi
        sleep 1
    done
    return 1
}

if [ -n "$(ss -Htn state established '( sport = :8281 or sport = :8282 )')" ]; then
    echo 'Active game connections prevent the switch' >&2
    exit 1
fi
case "$mode" in
enable)
    grep -qx 'channel2_enabled=true' /etc/wyd-go/server.txt
    /usr/local/sbin/wyd-channel-configs /etc/wyd-go/server.txt /etc/wyd-go
    systemctl start wyd-backup.service
    systemctl disable --now wyd-game
    systemctl enable --now wyd-game@1 wyd-game@2
    if ! wait_ready wyd-game@1 wyd-game@2; then
        echo 'Channel processes are not ready; restoring the single game process' >&2
        systemctl disable --now wyd-game@1 wyd-game@2 || true
        systemctl enable --now wyd-game
        wait_ready wyd-game || echo 'The single game process is not ready either' >&2
        exit 1
    fi
    echo 'Channel processes wyd-game@1 and wyd-game@2 are live.'
    ;;
disable)
    systemctl disable --now wyd-game@1 wyd-game@2
    systemctl enable --now wyd-game
    wait_ready wyd-game
    echo 'The single game process wyd-game is live.'
    ;;
*)
    echo 'Usage: wyd-channel-processes enable|disable' >&2
    exit 1
    ;;
esac
