"""Add local language controls to the pinned, operational native launcher.

The original patching and game-start machine code is retained byte for byte.
A small PE entry shim calls a source-built DLL before the original entry point.
The DLL subclasses only the marked main dialog and persists lang.txt. The only
native data change is the status format, which shows [VERSION] 48001 as 7.48.001.
"""
import argparse
import hashlib
import json
import struct
import tempfile
from pathlib import Path

from build_wyd_launcher import build_dialog, parse_dialog, resource, update_resources

ROOT = Path(__file__).resolve().parents[2]
BASE_SHA256 = "a60dca35b68dbe36ece57541917a433f48282518561e2f7e89282c50f3e2e901"
DLL_NAME = b"WYDLauncherLanguage.dll\0"
EXPORT_NAME = b"InitializeClientLanguage\0"
LANGUAGE_CONTROL = 47100
LANGUAGE_LABEL = 47101
# The launcher formats [VERSION] as (value / 1000, value % 1000). The 7.48
# release line uses [VERSION] 48001+, so a literal "7." yields 7.48.001.
VERSION_FORMAT_RVA = 0x39ED8
VERSION_FORMAT_SLOT = 0x30
VERSION_FORMAT_BEFORE = b"Ready to play.  (Version %d.%03d)\0"
VERSION_FORMAT_AFTER = b"Ready to play.  (Version 7.%d.%03d)\0"


def digest(data):
    return hashlib.sha256(data).hexdigest()


def align(value, alignment):
    return (value + alignment - 1) // alignment * alignment


class Image:
    def __init__(self, data):
        self.data = data
        self.pe = struct.unpack_from("<I", data, 0x3C)[0]
        if data[self.pe:self.pe + 4] != b"PE\0\0":
            raise ValueError("not a PE image")
        machine, self.count = struct.unpack_from("<HH", data, self.pe + 4)
        size = struct.unpack_from("<H", data, self.pe + 20)[0]
        self.optional = self.pe + 24
        if machine != 0x14C or struct.unpack_from("<H", data, self.optional)[0] != 0x10B:
            raise ValueError("expected an x86 PE32 image")
        self.table = self.optional + size
        self.base = struct.unpack_from("<I", data, self.optional + 28)[0]
        self.entry = struct.unpack_from("<I", data, self.optional + 16)[0]
        self.section_alignment, self.file_alignment = struct.unpack_from("<II", data, self.optional + 32)
        self.sections = []
        for i in range(self.count):
            name, virtual_size, rva, raw_size, raw = struct.unpack_from("<8sIIII", data, self.table + i * 40)
            self.sections.append((name.rstrip(b"\0"), virtual_size, rva, raw_size, raw))

    def offset(self, rva):
        for _, virtual_size, start, raw_size, raw in self.sections:
            if start <= rva < start + max(virtual_size, raw_size):
                return raw + rva - start
        raise ValueError(f"RVA outside image: {rva:#x}")

    def directory(self, index):
        return struct.unpack_from("<II", self.data, self.optional + 96 + index * 8)

    def imports(self):
        rva, _ = self.directory(1)
        at = self.offset(rva)
        result = []
        while any(self.data[at:at + 20]):
            result.append(self.data[at:at + 20])
            at += 20
        return result

    def import_address(self, symbol):
        for descriptor in self.imports():
            lookup, _, _, _, iat = struct.unpack("<IIIII", descriptor)
            at = self.offset(lookup or iat)
            for i in range(10000):
                name_rva = struct.unpack_from("<I", self.data, at + i * 4)[0]
                if not name_rva:
                    break
                if name_rva & 0x80000000:
                    continue
                name = self.offset(name_rva) + 2
                if self.data[name:self.data.index(0, name)] == symbol:
                    return self.base + iat + i * 4
        raise ValueError(f"required native import is missing: {symbol!r}")


def language_dialog(original):
    dialog = parse_dialog(original)
    ids = [item["id"] for item in dialog["items"]]
    if ids != [1000, 1001, 1002, 1003, 1004, 2, 1005, 1007] or dialog["cy"] != 267:
        raise ValueError("native main dialog differs from the recorded layout")
    dialog["cy"] = 290
    common = dict(help=0, exstyle=0, extra=b"")
    dialog["items"].extend([
        dict(common, style=0x50000000, x=8, y=276, cx=55, cy=9,
             id=LANGUAGE_LABEL, cls=0x82, text="Game language"),
        dict(common, style=0x50210003, x=70, y=272, cx=150, cy=70,
             id=LANGUAGE_CONTROL, cls=0x85, text=""),
    ])
    return build_dialog(dialog)


def patch_version_format(data):
    image = Image(data)
    at = image.offset(VERSION_FORMAT_RVA)
    slot = data[at:at + VERSION_FORMAT_SLOT]
    if slot != VERSION_FORMAT_BEFORE.ljust(VERSION_FORMAT_SLOT, b"\0"):
        raise ValueError("native version format slot changed")
    result = bytearray(data)
    result[at:at + VERSION_FORMAT_SLOT] = VERSION_FORMAT_AFTER.ljust(VERSION_FORMAT_SLOT, b"\0")
    return bytes(result)


def add_bootstrap(data):
    image = Image(data)
    if image.base != 0x400000 or image.entry != 0x12C59:
        raise ValueError("native image base or entry point changed")
    # The baseline is a fixed-base image. Do not create absolute addresses in
    # an ASLR image or quietly disable an existing relocation contract.
    dll_flags = struct.unpack_from("<H", data, image.optional + 70)[0]
    if dll_flags & 0x40 or image.directory(5) != (0, 0):
        raise ValueError("native relocation contract requires separate review")
    first_raw = min(section[4] for section in image.sections)
    if image.table + (image.count + 2) * 40 > first_raw:
        raise ValueError("not enough section-header space")
    next_rva = align(max(rva + max(vsize, rawsize) for _, vsize, rva, rawsize, _ in image.sections), image.section_alignment)
    code_rva = next_rva
    config_rva = code_rva + image.section_alignment
    original_imports = b"".join(image.imports())
    descriptor_size = len(original_imports) + 40
    lookup_offset = align(descriptor_size, 4)
    iat_offset = lookup_offset + 8
    dll_offset = iat_offset + 8
    name_offset = align(dll_offset + len(DLL_NAME), 2)
    config = bytearray(original_imports)
    config += struct.pack("<IIIII", config_rva + lookup_offset, 0, 0,
                          config_rva + dll_offset, config_rva + iat_offset)
    config += bytes(20)
    config.extend(bytes(lookup_offset - len(config)))
    config += struct.pack("<IIII", config_rva + name_offset, 0, config_rva + name_offset, 0)
    config += DLL_NAME
    config.extend(bytes(name_offset - len(config)))
    config += bytes(2) + EXPORT_NAME
    # PUSHFD/PUSHAD preserve the exact register/stack entry contract. Success
    # restores them and jumps to the original CRT entry; failure exits before
    # any patch download or game launch can occur.
    code = bytearray(b"\x9c\x60\xff\x15" + struct.pack("<I", image.base + config_rva + iat_offset))
    code += b"\x85\xc0\x0f\x84" + struct.pack("<i", 7)
    code += b"\x61\x9d\xe9" + struct.pack("<i", image.entry - (code_rva + len(code) + 7))
    code += b"\x6a\x01\xff\x15" + struct.pack("<I", image.import_address(b"ExitProcess"))
    result = bytearray(data)
    for index, (name, rva, payload, flags) in enumerate([
        (b".wlang", code_rva, code, 0x60000020),
        (b".wlcfg", config_rva, config, 0xC0000040),
    ]):
        raw = align(len(result), image.file_alignment)
        raw_size = align(len(payload), image.file_alignment)
        result.extend(bytes(raw - len(result)))
        result += payload + bytes(raw_size - len(payload))
        struct.pack_into("<8sIIIIIIHHI", result, image.table + (image.count + index) * 40,
                         name.ljust(8, b"\0"), len(payload), rva, raw_size, raw, 0, 0, 0, 0, flags)
    struct.pack_into("<H", result, image.pe + 6, image.count + 2)
    struct.pack_into("<I", result, image.optional + 16, code_rva)
    struct.pack_into("<I", result, image.optional + 56, align(config_rva + len(config), image.section_alignment))
    struct.pack_into("<II", result, image.optional + 104, config_rva, descriptor_size)
    struct.pack_into("<II", result, image.optional + 96 + 11 * 8, 0, 0)  # no stale bound imports
    struct.pack_into("<I", result, image.optional + 64, 0)  # optional PE checksum
    return bytes(result)


def build(source, output):
    baseline = source.read_bytes()
    if digest(baseline) != BASE_SHA256:
        raise ValueError("source is not the pinned operational native launcher")
    dialog = language_dialog(resource(baseline, 5, 102, 1042))
    expected = patch_version_format(baseline)
    with tempfile.TemporaryDirectory() as temporary:
        staged = Path(temporary) / "WYDLauncher.exe"
        staged.write_bytes(expected)
        update_resources(staged, [(5, 102, 1042, dialog)])
        result = add_bootstrap(staged.read_bytes())
    before, after = Image(baseline), Image(result)
    for name, _, _, size, at in before.sections:
        if name == b".rsrc":
            continue
        section = next(entry for entry in after.sections if entry[0] == name)
        if expected[at:at + size] != result[section[4]:section[4] + size]:
            raise ValueError(f"native code/data changed: {name!r}")
    if resource(result, 5, 102, 1042) != dialog:
        raise ValueError("language dialog failed post-build verification")
    for kind, name, lang in [(5, 130, 1033), (5, 133, 1033), (16, 1, 1042)]:
        if resource(baseline, kind, name, lang) != resource(result, kind, name, lang):
            raise ValueError("an unrelated native resource changed")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(result)
    receipt = dict(source=str(source.resolve()), source_sha256=digest(baseline),
                   output=str(output.resolve()), output_sha256=digest(result), size=len(result),
                   preserved_native_sections=[".text", ".rdata", ".data except version format"],
                   version_format=VERSION_FORMAT_AFTER.rstrip(b"\0").decode(),
                   original_entry_rva=hex(before.entry), entry_rva=hex(after.entry),
                   dependency=DLL_NAME.rstrip(b"\0").decode(), gameplay="not observed")
    output.with_suffix(".json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(receipt, indent=2))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--output", type=Path, default=ROOT / "tmproject/build/native-launcher/Release/WYDLauncher.exe")
    arguments = parser.parse_args()
    build(arguments.source, arguments.output)
