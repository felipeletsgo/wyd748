"""Exports 7.48 NewUI artwork from the client textures for the public portal.

UI/*.wyt textures are uncompressed TGA images behind a 4-byte "WT10" prefix.
Only the 7.48 NewUI sheets are used (ServerList2.wyt, loginbox2.wyt) plus the
7.48 login logo; the pre-7.48 classic sheets are not. This script writes, into
wydgo748/web/portal/public/ui/:

* panel.png  - the empty stone window of the server list, for a CSS 9-slice;
* button.png - the login-box button, for a CSS 9-slice;
* field.png  - the login-box text field, for a CSS 9-slice;
* title.png  - the ornamented title plate under the login box, for a CSS 9-slice;
* logo.png   - the "WYD II FC" login logo (logo1.wyt + logo2.wyt).

Run from the repository root:  python -I tools/client-assets/export_portal_ui.py
"""
import struct
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
UI = ROOT / "tmproject" / "client748" / "UI"
OUT = ROOT / "wydgo748" / "web" / "portal" / "public" / "ui"


def read_wyt(path: Path):
    data = path.read_bytes()
    if data[:4] != b"WT10":
        raise SystemExit(f"{path.name} is not a WT10 texture")
    tga = data[4:]
    id_length, image_type = tga[0], tga[2]
    width, height, bpp, descriptor = struct.unpack_from("<HHBB", tga, 12)
    if image_type != 2 or bpp not in (24, 32):
        raise SystemExit(f"{path.name}: unsupported TGA type {image_type}/{bpp} bpp")
    pixels = tga[18 + id_length:]
    step = bpp // 8
    image = []
    for y in range(height):
        source = y if descriptor & 0x20 else height - 1 - y
        row = []
        for x in range(width):
            o = (source * width + x) * step
            b, g, r = pixels[o], pixels[o + 1], pixels[o + 2]
            row.append((r, g, b, pixels[o + 3] if step == 4 else 255))
        image.append(row)
    return image


def crop(image, x0, y0, x1, y1):
    return [row[x0:x1] for row in image[y0:y1]]


def write_png(path: Path, image):
    height, width = len(image), len(image[0])
    raw = b"".join(b"\0" + bytes(c for px in row for c in px) for row in image)

    def chunk(kind, payload):
        return (struct.pack(">I", len(payload)) + kind + payload +
                struct.pack(">I", zlib.crc32(kind + payload) & 0xFFFFFFFF))

    path.write_bytes(b"\x89PNG\r\n\x1a\n" +
                     chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)) +
                     chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    for stale in ("panel.png", "button.png", "field.png", "title.png", "logo.png"):
        (OUT / stale).unlink(missing_ok=True)

    # The right-hand channel window of the server list is an empty 112x256
    # stone panel with the ornamented NewUI corners.
    write_png(OUT / "panel.png", crop(read_wyt(UI / "ServerList2.wyt"), 144, 0, 256, 256))

    login = read_wyt(UI / "loginbox2.wyt")
    write_png(OUT / "button.png", crop(login, 22, 86, 93, 106))   # Login/Close button
    write_png(OUT / "field.png", crop(login, 80, 33, 186, 49))    # Account field
    write_png(OUT / "title.png", crop(login, 1, 176, 242, 200))   # wide title plate

    left, right = read_wyt(UI / "logo1.wyt"), read_wyt(UI / "logo2.wyt")
    logo = [l + r for l, r in zip(left, right)]
    write_png(OUT / "logo.png", crop(logo, 2, 42, 256 + 251, 206))
    for name in ("panel.png", "button.png", "field.png", "title.png", "logo.png"):
        print(OUT / name)


if __name__ == "__main__":
    main()
