# 7.48 packet corpus

The shared canonical corpus is in `../../testdata/protocol/`. It contains only
plaintext frames whose origin and layout have been confirmed for the tested
boundary, or explicitly versioned coordinated extensions. Consumers strip
comments and whitespace before decoding the hex.

- `send_item_0x182_24.hex`: a 24-byte `MSG_SendItem` frame validated against
  the native 7.48 contract and consumed by Go and C++ tests to prevent drift.
- `kefra_portal_0x7f30_alive.hex` and `kefra_portal_0x7f30_defeated.hex`:
  16-byte [Kefra portal v1](kefra-portal.md) snapshots, consumed by both
  implementations. These are coordinated-extension fixtures, not native
  captures. They normalize the transport keyword, checksum and tick to zero.

The movement-capture corpus, which still has a server-only consumer, remains
in `wydgo748/internal/game/testdata/packets/` until that consumer migrates.
Do not duplicate a fixture in both source trees.

Do not add dumps taken after a `PacketProtocolV754` bridge as though they
were raw client wire. In particular, native `MSG_AttackOne` and `p39D_754`
use different offsets in some reference trees. Add a new combat capture only
after comparing it with the constructor in the currently distributed
`WYD.exe`.
