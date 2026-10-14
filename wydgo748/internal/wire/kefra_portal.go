package wire

// Kefra portal visibility, EXTENSAO_COORDENADA v1.
// See DOCS/protocol/kefra-portal.md. This is a server snapshot, never a request.
const (
	OpKefraPortalState   = 0x7F30
	KefraPortalStateSize = 16
	KefraPortalVersion   = 1
)

func KefraPortalState(defeated bool) []byte {
	b := Build(OpKefraPortalState, SceneField, KefraPortalStateSize)
	putU16(b, 12, KefraPortalVersion)
	if defeated {
		putU16(b, 14, 1)
	}
	return b
}
