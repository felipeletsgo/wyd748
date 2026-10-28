"""Reject incompatible game-only updates before activation changes any state."""
import argparse
from pathlib import Path, PurePosixPath
import tarfile


def check(installed_game, config, archive):
    expected = Path(__file__).with_name('data-layout.version').read_text().strip()
    installed = Path(installed_game) / 'data-layout.version'
    if not installed.is_file() or installed.read_text().strip() != expected:
        raise ValueError('Data layout migration requires a full release, including configuration and player API')
    with tarfile.open(archive, 'r:gz') as release:
        marker = release.extractfile('game/data-layout.version')
        if marker is None or marker.read(32).decode().strip() != expected:
            raise ValueError('Incoming data layout requires a full release')
        members = {member.name.rstrip('/') for member in release.getmembers()}
        for line in Path(config).read_text().splitlines():
            line = line.strip()
            if not line or line.startswith(('#', ';')) or '=' not in line:
                continue
            key, value = (part.strip() for part in line.split('=', 1))
            key = key.lower()
            value = value.strip('"')
            if key in {'npcgener2', 'accounts', 'guilds', 'charstate', 'instance_state'}:
                raise ValueError(f'Obsolete configuration key {key}: use a full release')
            # Collapse dots and repeated separators, but retain traversal for rejection.
            path = PurePosixPath(value)
            current_root = PurePosixPath('/srv/wyd-go/current/game')
            if path.is_relative_to(current_root):
                path = path.relative_to(current_root)
            if not path.parts or path.parts[0] != 'data':
                continue
            if '..' in path.parts or '\\' in value:
                raise ValueError(f'Unsupported data path for {key}: use a full release')
            member = 'game/' + str(path)
            if member not in members and not any(name.startswith(member + '/') for name in members):
                raise ValueError(f'Configured input {key} is absent from incoming data: use a full release')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--installed-game', required=True)
    parser.add_argument('--config', required=True)
    parser.add_argument('--archive', required=True)
    args = parser.parse_args()
    try:
        check(args.installed_game, args.config, args.archive)
    except (OSError, ValueError, KeyError, tarfile.TarError) as error:
        parser.exit(1, f'Game-only preflight rejected: {error}\n')


if __name__ == '__main__':
    main()
