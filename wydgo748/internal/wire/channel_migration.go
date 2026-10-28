package wire

// ChannelMigration uses the retained 7.48 0x52A image. The token is opaque
// except for its *<channel> prefix; credentials never travel in this response.
func ChannelMigration(characterID uint16, account, ticket string) []byte {
	b := Build(0x52A, characterID, 80)
	copy(b[12:27], account)
	copy(b[28:79], ticket)
	return b
}
