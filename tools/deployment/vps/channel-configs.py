#!/usr/bin/env python3
"""Derive the channel-process configurations from server.txt (W9 phase 5).

server.txt stays the single source. Channel 1 keeps its listener, language and
staff panel; channel 2 takes channel2_address and channel2_language, has no
staff panel, and exports Guilds.txt to a private path. Each file names the
other channel in channel_peers. Usage: channel-configs.py SERVER_TXT OUT_DIR
"""
import os
import sys
import tempfile

PEER_KEYS = ('channel2_enabled', 'channel2_address', 'channel2_language', 'channel_peers', 'channel_id')


def parse(path):
    lines, values = [], {}
    with open(path, encoding='utf-8') as source:
        for raw in source.read().splitlines():
            key, sep, value = raw.partition('=')
            if sep and not raw.lstrip().startswith('#'):
                values[key.strip()] = value.strip()
            lines.append(raw)
    return lines, values


def derive(lines, values, channel):
    if values.get('channel2_enabled', 'false').lower() != 'true':
        raise SystemExit('server.txt must enable channel 2 before the channel processes')
    overrides = {'channel_id': str(channel), 'channel_peers': '2' if channel == 1 else '1'}
    if channel == 2:
        overrides['listen_address'] = values['channel2_address']
        overrides['channel_language'] = values.get('channel2_language', 'en')
        overrides['web_admin_enabled'] = 'false'
        overrides['guilds_txt'] = '/var/lib/wyd-go/Guilds-ch2.txt'
    out, written = [], set()
    for raw in lines:
        key, sep, _ = raw.partition('=')
        key = key.strip()
        if sep and not raw.lstrip().startswith('#'):
            if key in PEER_KEYS and key not in overrides:
                continue
            if key in overrides:
                out.append(f'{key}={overrides[key]}')
                written.add(key)
                continue
        out.append(raw)
    for key, value in overrides.items():
        if key not in written:
            out.append(f'{key}={value}')
    return '\n'.join(out) + '\n'


def main():
    if len(sys.argv) != 3:
        raise SystemExit(__doc__)
    lines, values = parse(sys.argv[1])
    for channel in (1, 2):
        target = os.path.join(sys.argv[2], f'server-ch{channel}.txt')
        text = derive(lines, values, channel)
        handle, temporary = tempfile.mkstemp(dir=sys.argv[2], prefix=f'.server-ch{channel}.')
        with os.fdopen(handle, 'w', encoding='utf-8', newline='\n') as output:
            output.write(text)
        os.chmod(temporary, 0o644)
        os.replace(temporary, target)


if __name__ == '__main__':
    main()
