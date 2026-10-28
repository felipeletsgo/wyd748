package wire

import "encoding/binary"

// Coordinated admission contract v2 (release line 7.48); see
// DOCS/login-dispatch.md. Keep these values aligned with ClientBuildContract.h.
// The prefix is the release line (748 << 16) and the revision its patch
// number, so 0x02EC0001 is 7.48.001. The retired v1 line used 0x5747xxxx and
// can never match this prefix.
const (
	AccountLoginSize                  = 116
	ClientProtocolVersion      uint32 = 748
	ClientBuildPrefix          uint32 = 0x02EC0000
	MinimumClientBuildRevision uint32 = 10
	CurrentClientBuild                = ClientBuildPrefix | MinimumClientBuildRevision
)

// CompatibleClientLogin checks only compiled identity, never credentials.
// The caller must already have validated the opcode and framing envelope.
func CompatibleClientLogin(pkt []byte) bool {
	if len(pkt) != AccountLoginSize {
		return false
	}
	protocol := binary.LittleEndian.Uint32(pkt[40:44])
	build := binary.LittleEndian.Uint32(pkt[44:48])
	return protocol == ClientProtocolVersion && build&0xFFFF0000 == ClientBuildPrefix &&
		build&0xFFFF >= MinimumClientBuildRevision
}
