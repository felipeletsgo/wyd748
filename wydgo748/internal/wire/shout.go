package wire

// OpMessageShout is the native green chat notice, not the top message panel.
const OpMessageShout uint16 = 0xD1D

func PlayerShout(name, message string) []byte {
	// The native receiver terminates the final two body bytes. Identity is
	// supplied by the authenticated character, never by the chat request.
	b := Build(OpMessageShout, 0, 108)
	copy(b[12:106], "["+name+"]> "+message)
	return b
}
