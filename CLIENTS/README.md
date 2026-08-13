# WYD KR client snapshot

`WYD/` contains the static client snapshot used as the KR reference for the
7.48 asset import. The directory is stored with Git LFS because it contains
the complete mesh, texture, map, UI, sound and executable payload.

Install Git LFS before cloning or downloading the client:

```text
git lfs install
git clone https://github.com/felipeletsgo/wyd748.git
```

Reference executable SHA-256:

```text
CB725630B77B57B0774FFA83B8918CC2C00AA064C2DCB4B4011F0B5A734E963E
```

`WYD.log` and the empty `ScreenShot/` directory are local runtime artifacts
and are intentionally not versioned.
