"""Builds tmproject/client748/WYDLauncher.exe from the OnGame 7.47 launcher.

The original WYDLauncher.exe (local-only references/OnGame/WYD) is a working
7.4x patcher whose update protocol matches this client: it downloads
update.htm, compares each "<version> <file>..." line with [VERSION] in
config.txt, downloads newer ZIPs plus guilds.txt, extracts them and starts
WYD.EXE. This script adapts a verified copy without touching the reference:

* update host: the OnGame URLs become http://wyd.forjago.com/patch/;
* the FTP fallback to a third-party IP (80.77.12.45) becomes 127.0.0.1;
* every Portuguese UI string and dialog label is translated to English;
* the resolution radios follow this client's 1-based [RES] table;
* RES 6 no longer forces the classic UI (it is 1280x960 here, not 640x480).

Run on Windows from the repository root:  python -I tools/client-assets/build_wyd_launcher.py
"""
import ctypes
import hashlib
import shutil
import struct
import sys
import tempfile
from ctypes import wintypes
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE_DIR = ROOT / "references" / "OnGame" / "WYD"
CLIENT_DIR = ROOT / "tmproject" / "client748"

SOURCE_SHA256 = "6cd8b16f15794e87cd9ffaec32d3e33ab90c98eb96303ba2ddf799dd8144da75"
ASSETS = {  # copied unchanged: neutral "With Your Destiny" artwork and its HTML frame
    "Launcher.bmp": "34f490adf7f96e0fcd0b7f4f22bf03cb4aaca1619663af603f138ef97a789ca8",
    "Launcher.hat": "120d00100a9e49eba21b90d656c7a9d5b025621fae1700d64c0b3530fa76e3f7",
}

PATCH_BASE = "http://wyd.forjago.com/patch/"
TITLE = "WYD 7.48"

# .data strings: virtual address -> (original cp1252 text, replacement). The
# image maps .data at file offset == VA - 0x400000. Replacements must fit the
# original slot and keep every printf conversion in the same order.
STRINGS = {
    0x4390D0: ("WYD - www.wyd.com.br", TITLE),
    0x4391D0: ("Não foi possível armazenar o conteúdo.", "Could not save the content."),
    0x4391F8: ("Escolha a música que deseja modificar.", "Choose the music to change."),
    0x43922C: ("Selecione o arquivo MP3 para música de fundo", "Select an MP3 file for background music"),
    0x439280: ("Cancelar", "Cancel"),  # button labels set at run time (SetDlgItemText)
    0x43928C: ("Padrão", "Normal"),
    0x439294: ("Salvar", "Save"),
    0x43929C: ("Escolha o efeito sonoro que deseja modificar", "Choose the sound effect to change."),
    0x4392D4: ("Selecione o arquivo WAV a ser utilizado como efeito sonoro",
               "Select a WAV file to use as a sound effect"),
    0x43931C: ("Nova Interface não poderá ser executada no modo 640x480.",
               "The new interface cannot run at 640x480."),
    0x4394D4: ("Não foi possível localizar a informação da versão em config.txt",
               "Could not find the version information in config.txt"),
    0x439514: ("config.txt não é um arquivo de configuração correto",
               "config.txt is not a valid configuration file"),
    0x439570: ("A versão não está atualizada. Faça o download da versão mais recente no site.",
               "This version is outdated. Download the latest client from the website."),
    0x43964C: ("http://ongame.nefficient.com/ongame/wyd/patch/update_regular/update.htm",
               PATCH_BASE + "update.htm"),
    0x43974C: ("http://ongame.nefficient.com/ongame/wyd/patch/update_regular/", PATCH_BASE),
    0x43984C: ("80.77.12.45", "127.0.0.1"),
    0x43994C: ("Servidor 1", "WYD-Go"),
    0x43995C: ("Servidor 2", "WYD-Go"),
    0x4399D0: ("Falha na finalização. Finalize o WYD e reinicie o programa.",
               "Could not finish. Close WYD and restart the launcher."),
    0x439A0C: ("O launcher está sendo executado.", "The launcher is already running."),
    0x439A30: ("O jogo já está sendo executado. Deseja finalizar o jogo?.",
               "The game is already running. Close it?"),
    0x439A80: ("Examinando a última versão.", "Checking for updates."),
    0x439A9C: ("Inicializando as configurações do jogo.", "Loading game settings."),
    0x439AD4: ("Configurar", "Settings"),
    0x439AE0: ("Iniciar", "Start"),
    0x439AE8: ("Não foi possível encontrar o arquivo de configuração do jogo (Config.txt). "
               "Tente Instalar novamente.",
               "Could not find the game configuration file (config.txt). Reinstall the game."),
    0x439B70: ("Falha na inicialização do DLL. Caso não tenha instalado DirectX 8.1, instale antes.",
               "DLL initialization failed. Install DirectX 8.1 or later first."),
    0x439BC4: ("http://www.wydonline.co.kr/dxpop.htm", "https://wyd.forjago.com/download/"),
    0x439BEC: ("Não foi possível encontrar WYD.EXE. Reinstale o programa.",
               "Could not find WYD.EXE. Reinstall the game."),
    0x439C28: ("O arquivo executável do WYD ou arquivo do Direct X está danificado.",
               "The WYD executable or the DirectX files are damaged."),
    0x439C6C: ("Erro: Não pôde ser executado devido ao erro %d.", "Error: the game could not start (%d)."),
    0x439C9C: ("O arquivo executável ou DirectX está danificado. Verifique o erro nos discos e "
               "reinstale o programa.",
               "The executable or DirectX is damaged. Check your disk and reinstall the game."),
    0x439D04: ("WYD ou DirectX está sendo utilizado por outro programa.",
               "WYD or DirectX is in use by another program."),
    0x439D3C: ("Outro programa está utilizando WYD ou DirectX.", "Another program is using WYD or DirectX."),
    0x439D6C: ("Erro na instalação do dispositivo e do driver. Instale o driver de vídeo e o "
               "DirectX 8.1 novamente.",
               "Device or driver error. Reinstall your video driver and DirectX 8.1."),
    0x439DDC: ("Não foi possível extrair o arquivo: %s(%s)", "Could not extract the file: %s(%s)"),
    0x439E08: ("Não foi possível extrair o arquivo: %s", "Could not extract the file: %s"),
    0x439E30: ("Não foi possível conectar no servidor de download. Tente novamente mais tarde.",
               "Could not connect to the download server. Try again later."),
    0x439E80: ("Não foi possível atualizar o arquivo (%s).\nVerifique se o arquivo está sendo utilizado.",
               "Could not update the file (%s).\nMake sure the game is closed."),
    0x439ED8: ("Pronto para iniciar o jogo.  (Versão %d.%03d)", "Ready to play.  (Version %d.%03d)"),
    0x439F08: ("Baixando o arquivo %s. ", "Downloading %s."),
    0x439F28: ("Faça o download do CLIENT do jogo através do site.",
               "Download the full game client from the website."),
    0x439F5C: ("As informações da atualização não foram localizadas. Tente conectar mais tarde.",
               "Update information was not found. Try again later."),
    0x439FAC: ("Não foi possível ler o conteúdo da informação da atualização.",
               "Could not read the update information."),
    0x439FEC: ("Não foi possível armazenar os conteúdos das informações da atualização.",
               "Could not save the update information."),
    0x43A040: ("Não foi possível conectar no servidor de atualização.",
               "Could not connect to the update server."),
}

# Code: the config loader forces the classic UI for RES 1 or RES 6 (both
# 640x480 in the OnGame table). Here RES 6 is 1280x960, so compare with 1 twice.
CODE = [(0x40263D, bytes.fromhex("83f806"), bytes.fromhex("83f801"))]

DIALOG_TEXT = {
    "Iniciar": "Start", "Configurar": "Settings", "Fechar": "Close",
    "Configuração": "Settings", "Salvar": "Save", "Cancelar": "Cancel",
    "Resolução": "Resolution", "Efeitos Sonoros": "Sound Effects", "Animação": "Animation",
    "Baixa  - 15 FPS": "Low  - 15 FPS", "Alta   - 60 FPS": "High - 60 FPS", "Música": "Music",
    "DES                        MAX": "OFF                        MAX", "Brilho": "Brightness",
    "Padrão": "Default", "Procura": "Browse", "Arquivo": "File", "Cursor": "Cursor",
    "Imagem": "Image", "Sistema": "System", "Som": "Sound", "Modo": "Mode",
    "Tela Cheia": "Fullscreen", "Janela": "Window", "Vídeo de Introdução": "Intro Video",
    "Não executar": "Don't play", "Executar": "Play", "Modo de interface": "Interface",
    "NOVA": "NEW", "CLÁSSICA": "CLASSIC", "Rotação Câmera": "Camera Rotation",
    "Textura": "Texture", "Textura Compactada": "Compressed", "Textura Descompactada": "Uncompressed",
    "Esquerda": "Left", "Direita": "Right", "Modo das teclas": "Key Mode", "Ação": "Action",
    "Chat": "Chat", "Eu aceito": "I agree", "Dialog": "Agreement",
}
KEEP_TEXT = {"", "Progress1", "Slider1", "text2", "{8856F961-340A-11D0-A96B-00C04FD705A2}"}

# Client [RES] table (NewApp.cpp stResList, 1-based). Radio order is the value.
RESOLUTIONS = [(640, 480), (800, 600), (1024, 768), (1280, 720), (1280, 800), (1280, 960),
               (1280, 1024), (1440, 900), (1600, 900), (1600, 1024), (1600, 1080)]
OLD_RES_IDS = [1004, 1005, 1006, 1007, 1008, 1017, 1018, 1019, 1020, 1021]
# 1004 keeps the 640x480 warning handler; 1017 is not reused because the
# message map binds it to the same warning.
NEW_RES_IDS = [1004, 1005, 1006, 1007, 1008, 1018, 1019, 1020, 1021, 1038, 1039]

VERSION_TEXT = {"Programa de Aplicacao WYDUpgrade MFC": "WYD 7.48 Launcher",
                "Programa de Aplicacao WYDUpgrade": "WYD 7.48 Launcher"}

RT_DIALOG, RT_VERSION = 5, 16
DIALOGS = [(102, 1042), (130, 1033), (133, 1033)]


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_sz(b, o):
    w = struct.unpack_from("<H", b, o)[0]
    if w == 0:
        return "", o + 2
    if w == 0xFFFF:
        return struct.unpack_from("<H", b, o + 2)[0], o + 4
    e = o
    while struct.unpack_from("<H", b, e)[0]:
        e += 2
    return b[o:e].decode("utf-16le"), e + 2


def write_sz(v):
    if isinstance(v, int):
        return struct.pack("<HH", 0xFFFF, v)
    return v.encode("utf-16le") + b"\0\0"


def parse_dialog(b):
    ex = struct.unpack_from("<HH", b, 0) == (1, 0xFFFF)
    if ex:
        _, _, helpid, exstyle, style, n, x, y, cx, cy = struct.unpack_from("<HHIIIHhhhh", b, 0)
        o = 26
    else:
        style, exstyle, n, x, y, cx, cy = struct.unpack_from("<IIHhhhh", b, 0)
        helpid, o = 0, 18
    menu, o = read_sz(b, o)
    cls, o = read_sz(b, o)
    title, o = read_sz(b, o)
    font = None
    if style & 0x40:  # DS_SETFONT
        if ex:
            font = struct.unpack_from("<HHBB", b, o)
            o += 6
        else:
            font = (struct.unpack_from("<H", b, o)[0],)
            o += 2
        face, o = read_sz(b, o)
        font = font + (face,)
    items = []
    for _ in range(n):
        o = (o + 3) & ~3
        if ex:
            hid, es, st, ix, iy, icx, icy, iid = struct.unpack_from("<IIIhhhhI", b, o)
            o += 24
        else:
            st, es, ix, iy, icx, icy, iid = struct.unpack_from("<IIhhhhH", b, o)
            o += 18
            hid = 0
        c, o = read_sz(b, o)
        t, o = read_sz(b, o)
        ln = struct.unpack_from("<H", b, o)[0]
        extra = b[o + 2:o + 2 + ln]
        o += 2 + ln
        items.append(dict(help=hid, exstyle=es, style=st, x=ix, y=iy, cx=icx, cy=icy, id=iid,
                          cls=c, text=t, extra=extra))
    return dict(ex=ex, helpid=helpid, exstyle=exstyle, style=style, x=x, y=y, cx=cx, cy=cy,
                menu=menu, cls=cls, title=title, font=font, items=items)


def build_dialog(g):
    out = bytearray()
    if g["ex"]:
        out += struct.pack("<HHIIIHhhhh", 1, 0xFFFF, g["helpid"], g["exstyle"], g["style"],
                           len(g["items"]), g["x"], g["y"], g["cx"], g["cy"])
    else:
        out += struct.pack("<IIHhhhh", g["style"], g["exstyle"], len(g["items"]),
                           g["x"], g["y"], g["cx"], g["cy"])
    out += write_sz(g["menu"]) + write_sz(g["cls"]) + write_sz(g["title"])
    if g["font"] is not None:
        if g["ex"]:
            out += struct.pack("<HHBB", *g["font"][:4])
        else:
            out += struct.pack("<H", g["font"][0])
        out += write_sz(g["font"][-1])
    for it in g["items"]:
        while len(out) % 4:
            out += b"\0"
        if g["ex"]:
            out += struct.pack("<IIIhhhhI", it["help"], it["exstyle"], it["style"], it["x"], it["y"],
                               it["cx"], it["cy"], it["id"])
        else:
            out += struct.pack("<IIhhhhH", it["style"], it["exstyle"], it["x"], it["y"],
                               it["cx"], it["cy"], it["id"])
        out += write_sz(it["cls"]) + write_sz(it["text"]) + struct.pack("<H", len(it["extra"]))
        out += it["extra"]
    return bytes(out)


def translate(text, where):
    if isinstance(text, int) or text in KEEP_TEXT:
        return text
    if text not in DIALOG_TEXT:
        raise SystemExit(f"untranslated dialog text in {where}: {text!r}")
    return DIALOG_TEXT[text]


def adapt_dialog(dialog_id, g):
    g["title"] = translate(g["title"], dialog_id)
    for it in g["items"]:
        if not isinstance(it["text"], int) and "x" in it["text"] and "bit" in it["text"]:
            continue  # resolution radios are rebuilt below
        it["text"] = translate(it["text"], dialog_id)
    if dialog_id != 130:
        return g
    ids = [it["id"] for it in g["items"]]
    start = ids.index(OLD_RES_IDS[0])
    if ids[start:start + len(OLD_RES_IDS)] != OLD_RES_IDS:
        raise SystemExit("resolution radios are not where the 7.47 template places them")
    first = g["items"][start]
    radios = []
    for index, ((w, h), rid) in enumerate(zip(RESOLUTIONS, NEW_RES_IDS)):
        # WS_GROUP only on the first radio keeps DDX_Radio's index == RES - 1.
        style = first["style"] if index == 0 else first["style"] & ~0x00020000
        radios.append(dict(first, id=rid, style=style, y=12 + 8 * index, cy=9, cx=81,
                           text=f"{w:4d} x {h:4d}"))
    g["items"][start:start + len(OLD_RES_IDS)] = radios
    return g


def patch_version(data: bytes) -> bytes:
    data = bytearray(data)
    for old, new in VERSION_TEXT.items():
        raw = old.encode("utf-16le")
        repl = new.encode("utf-16le").ljust(len(raw), b"\0")
        at = data.find(raw)
        if at < 0:
            raise SystemExit(f"version string not found: {old}")
        data[at:at + len(raw)] = repl
    return bytes(data)


def rsrc_section(pe: bytes):
    """Returns (file offset, RVA) of .rsrc from the section table."""
    header = struct.unpack_from("<I", pe, 0x3C)[0]
    count, optional = struct.unpack_from("<H", pe, header + 6)[0], struct.unpack_from("<H", pe, header + 20)[0]
    for i in range(count):
        entry = header + 24 + optional + i * 40
        if pe[entry:entry + 6] == b".rsrc" + bytes(1):
            rva, _, raw = struct.unpack_from("<III", pe, entry + 12)
            return raw, rva
    raise SystemExit("image has no .rsrc section")


def resource(pe: bytes, type_id: int, name_id: int, lang: int) -> bytes:
    """Reads one resource by type, numeric name and language."""
    base_off, base_rva = rsrc_section(pe)

    def find(dir_off, key):
        named, ids = struct.unpack_from("<HH", pe, base_off + dir_off + 12)
        for i in range(named + ids):
            name, off = struct.unpack_from("<II", pe, base_off + dir_off + 16 + i * 8)
            if name == key:
                return off
        raise SystemExit(f"resource {type_id}/{name_id}/{lang} not found")

    off = find(find(find(0, type_id) & 0x7FFFFFFF, name_id) & 0x7FFFFFFF, lang)
    rva, size = struct.unpack_from("<II", pe, base_off + off)
    return pe[rva - base_rva + base_off:rva - base_rva + base_off + size]


def update_resources(path: Path, updates):
    k32 = ctypes.WinDLL("kernel32", use_last_error=True)
    k32.BeginUpdateResourceW.restype = wintypes.HANDLE
    k32.BeginUpdateResourceW.argtypes = [wintypes.LPCWSTR, wintypes.BOOL]
    k32.UpdateResourceW.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p, wintypes.WORD,
                                    ctypes.c_void_p, wintypes.DWORD]
    k32.EndUpdateResourceW.argtypes = [wintypes.HANDLE, wintypes.BOOL]
    handle = k32.BeginUpdateResourceW(str(path), False)
    if not handle:
        raise SystemExit(f"BeginUpdateResource failed: {ctypes.get_last_error()}")
    for type_id, name_id, lang, data in updates:
        buf = ctypes.create_string_buffer(data, len(data))
        if not k32.UpdateResourceW(handle, type_id, name_id, lang, buf, len(data)):
            k32.EndUpdateResourceW(handle, True)
            raise SystemExit(f"UpdateResource {type_id}/{name_id} failed: {ctypes.get_last_error()}")
    if not k32.EndUpdateResourceW(handle, False):
        raise SystemExit(f"EndUpdateResource failed: {ctypes.get_last_error()}")


def main():
    source = (SOURCE_DIR / "WYDLauncher.exe").read_bytes()
    if sha256(source) != SOURCE_SHA256:
        raise SystemExit("references/OnGame/WYD/WYDLauncher.exe is not the verified 7.47 launcher")
    image = bytearray(source)
    for va, (old, new) in STRINGS.items():
        off = va - 0x400000
        old_b, new_b = old.encode("cp1252"), new.encode("cp1252")
        if bytes(image[off:off + len(old_b) + 1]) != old_b + b"\0":
            raise SystemExit(f"string at {va:#x} differs from the 7.47 image")
        if len(new_b) > len(old_b) or old.count("%") != new.count("%"):
            raise SystemExit(f"replacement at {va:#x} does not fit or changes its format")
        image[off:off + len(old_b)] = new_b.ljust(len(old_b), b"\0")
    for va, old, new in CODE:
        off = va - 0x400000
        if bytes(image[off:off + len(old)]) != old:
            raise SystemExit(f"code at {va:#x} differs from the 7.47 image")
        image[off:off + len(new)] = new

    updates = []
    for dialog_id, lang in DIALOGS:
        g = adapt_dialog(dialog_id, parse_dialog(resource(source, RT_DIALOG, dialog_id, lang)))
        updates.append((RT_DIALOG, dialog_id, lang, build_dialog(g)))
    updates.append((RT_VERSION, 1, 1042, patch_version(resource(source, RT_VERSION, 1, 1042))))

    with tempfile.TemporaryDirectory() as work:
        staged = Path(work) / "WYDLauncher.exe"
        staged.write_bytes(bytes(image))
        update_resources(staged, updates)
        result = staged.read_bytes()
    # Resource updates rewrite only .rsrc; the patched code and data must survive.
    code_end = rsrc_section(source)[0]
    if result[0x1000:code_end] != bytes(image[0x1000:code_end]):
        raise SystemExit("resource update changed code or data sections")
    for dialog_id, lang in DIALOGS:
        for it in parse_dialog(resource(result, RT_DIALOG, dialog_id, lang))["items"]:
            if isinstance(it["text"], str) and any(ord(ch) > 0x7E for ch in it["text"]):
                raise SystemExit(f"non-ASCII text left in dialog {dialog_id}: {it['text']!r}")

    CLIENT_DIR.joinpath("WYDLauncher.exe").write_bytes(result)
    for name, digest in ASSETS.items():
        asset = SOURCE_DIR / name
        if sha256(asset.read_bytes()) != digest:
            raise SystemExit(f"{asset} is not the verified 7.47 asset")
        shutil.copyfile(asset, CLIENT_DIR / name)
    print(f"WYDLauncher.exe {sha256(result)} ({len(result)} bytes)")


if __name__ == "__main__":
    if sys.platform != "win32":
        raise SystemExit("UpdateResource requires Windows")
    main()
