"""Package the operational native launcher and reviewed client assets. Do not publish."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import stat
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[2]
NATIVE_SHA256 = '85c9c648b389be91ca348351590fcbefa6752a4d4e849afbbcb1f054ca12fde2'
DIRECTORIES = ('Effect', 'EN', 'Env', 'mesh', 'music', 'PT', 'Shader', 'sound', 'UI')
ROOT_FILES = ('AniSound4.txt', 'cdata.bin', 'InitItem.bin', 'itemhelp.dat', 'ItemList.bin',
              'ItemListWide.csv', 'Itemname.bin', 'minimap.dat', 'Music.txt', 'object.bin',
              'serverlist.bin', 'SkillData.bin', 'skillhelp.dat', 'sn.bin', 'sound.txt',
              'WYDLauncher.exe', 'WYDLauncherLanguage.dll', 'Launcher.bmp', 'Launcher.hat')


def sha(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def regular(path):
    for part in (path, *path.parents):
        info = part.lstat()
        if stat.S_ISLNK(info.st_mode) or getattr(info, 'st_file_attributes', 0) & 0x400:
            raise ValueError(f'Linked input rejected: {part}')
    if not path.is_file():
        raise ValueError(f'Regular file required: {path}')


def safe(relative):
    for segment in relative.split('/'):
        stem = segment.split('.')[0].lower()
        if (not re.fullmatch(r'[A-Za-z0-9._-]+', segment) or segment in ('.', '..') or
                segment.endswith('.') or stem in ('con', 'prn', 'aux', 'nul') or
                re.fullmatch(r'(com|lpt)[1-9]', stem)):
            raise ValueError(f'Unsafe archive path: {relative}')


def verify(output):
    receipt = json.loads((output / 'release.json').read_text(encoding='utf-8'))
    archive = output / receipt['archive']['path']
    regular(archive)
    if sha(archive) != receipt['archive']['sha256'] or archive.stat().st_size != receipt['archive']['size']:
        raise ValueError('Archive identity mismatch')
    expected = {e['path']: e for e in receipt['files']}
    if expected['WYDLauncher.exe']['sha256'] != NATIVE_SHA256:
        raise ValueError('Operational native launcher required')
    with zipfile.ZipFile(archive) as bundle:
        if len(bundle.namelist()) != len(expected) or set(bundle.namelist()) != set(expected):
            raise ValueError('Unexpected or duplicate archive entries')
        for name, entry in expected.items():
            safe(name)
            with bundle.open(name) as stream:
                digest = hashlib.file_digest(stream, 'sha256').hexdigest()
            if digest != entry['sha256'] or bundle.getinfo(name).file_size != entry['size']:
                raise ValueError(f'Entry identity mismatch: {name}')
    return dict(verified=True, archive=str(archive), sha256=receipt['archive']['sha256'],
                bytes=receipt['archive']['size'], entries=len(expected), launcher='native-adapted')


def build(args):
    if not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9._-]{0,63}', args.release) or not 48001 <= args.version <= 49999:
        raise ValueError('Invalid release or feed version')
    output = Path(os.path.abspath(args.output))
    if not output.is_relative_to(ROOT / 'build') or output == ROOT / 'build' or output.exists():
        raise ValueError('Use a new output directory under root build/')
    for ancestor in output.parents:
        if ancestor.exists() and (ancestor.is_symlink() or getattr(ancestor.lstat(), 'st_file_attributes', 0) & 0x400):
            raise ValueError('Linked output ancestor rejected')
    client = ROOT / 'tmproject/client748'
    sources = {name: client / name for name in ROOT_FILES}
    for folder in DIRECTORIES:
        if not (client / folder).is_dir():
            raise ValueError(f'Missing asset directory: {folder}')
        for path in sorted((client / folder).rglob('*')):
            if path.is_symlink() or getattr(path.lstat(), 'st_file_attributes', 0) & 0x400:
                raise ValueError(f'Linked asset rejected: {path}')
            if path.is_file():
                if path.suffix.lower() in ('.exe', '.dll', '.pdb', '.log', '.dmp', '.zip', '.ini'):
                    raise ValueError(f'Unexpected asset file: {path}')
                sources[path.relative_to(client).as_posix()] = path
    for name in ('WYD.exe', 'WydCrashReporter.dll'):
        sources[name] = ROOT / 'tmproject/build/TMProject748/Release' / name
    regular(Path(args.runtime_receipt))
    runtime = json.loads(Path(args.runtime_receipt).read_text(encoding='utf-8-sig'))
    if runtime['signatureStatus'] != 'Valid' or runtime['architecture'] != 'x86':
        raise ValueError('Verified Microsoft x86 runtime required')
    sources['prerequisites/vc_redist.x86.exe'] = Path(runtime['destination'])
    seen = set()
    for name, path in sources.items():
        safe(name); regular(path)
        if name.lower() in seen:
            raise ValueError(f'Case collision: {name}')
        seen.add(name.lower())
    if sha(sources['WYDLauncher.exe']) != NATIVE_SHA256:
        raise ValueError('Operational native launcher required; replacement launchers are rejected')
    if sha(sources['prerequisites/vc_redist.x86.exe']) != runtime['sha256'].lower():
        raise ValueError('Runtime receipt mismatch')
    generated = {
        'lang.txt': b'EN\r\n',
        'config.txt': (f'[VERSION] {args.version}\r\n[RES] 3\r\n[ANIMATION] 2\r\n[SOUND] 100\r\n[MUSIC] 100\r\n'
                       '[SERVER] 0\r\n[BRIGHT] 51\r\n[CURSOR] 2\r\n[DEMO] 1\r\n[WINDOW] 1\r\n[CLASSIC] 0\r\n'
                       '[CAMERAROTATE] 0\r\n[DXT] 0\r\n[KEYTYPE] 0\r\n').encode('ascii'),
    }
    output.mkdir(parents=True)
    archive = output / f'WYD-client-native-{args.release}.zip'
    entries = []
    with zipfile.ZipFile(archive, 'x', compression=zipfile.ZIP_DEFLATED, compresslevel=1) as bundle:
        for name, path in sorted(sources.items()):
            digest = sha(path); size = path.stat().st_size
            bundle.write(path, name)
            if sha(path) != digest:
                raise ValueError(f'Input changed during packaging: {path}')
            entries.append(dict(path=name, sha256=digest, size=size, source=str(path)))
        for name, data in generated.items():
            bundle.writestr(name, data)
            entries.append(dict(path=name, sha256=hashlib.sha256(data).hexdigest(), size=len(data), source='generated fresh-install default'))
    receipt = dict(schemaVersion=1, release=args.release, feedVersion=args.version, launcher='native-adapted',
                   head=subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip(),
                   provenance='working-tree', files=entries, runtime=runtime,
                   archive=dict(path=archive.name, size=archive.stat().st_size, sha256=sha(archive)))
    (output / 'release.json').write_text(json.dumps(receipt, indent=2)+'\n', encoding='utf-8')
    return verify(output)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command', required=True)
    create = sub.add_parser('build')
    for name in ('release', 'output', 'runtime-receipt'):
        create.add_argument('--'+name, required=True)
    create.add_argument('--version', required=True, type=int)
    check = sub.add_parser('verify'); check.add_argument('--output', required=True)
    args = parser.parse_args()
    try:
        print(json.dumps(build(args) if args.command == 'build' else verify(Path(args.output)), indent=2))
    except (OSError, ValueError, KeyError, zipfile.BadZipFile) as error:
        parser.exit(1, f'Native client release rejected: {error}\n')
