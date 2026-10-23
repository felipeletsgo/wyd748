package wire

import "encoding/binary"

// Coordinated admission contract v1; see DOCS/login-dispatch.md.
// Keep these values aligned with ClientBuildContract.h, not config.txt.
const (
	AccountLoginSize                  = 116
	ClientProtocolVersion      uint32 = 748
	ClientBuildPrefix          uint32 = 0x57470000
	MinimumClientBuildRevision uint32 = 1
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
