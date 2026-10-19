package wire

// EnvironmentEffect preserves the native 7.48 0x3A2 envelope. Coordinates
// must fit positive signed shorts; effect 32 is the Rune Track floor pulse.
func EnvironmentEffect(rect [4]uint16, effect uint16) []byte {
	b := Build(0x3A2, SceneField, 24)
	for i, value := range rect {
		putU16(b, 12+2*i, value)
	}
	putU16(b, 20, effect)
	return b
}
