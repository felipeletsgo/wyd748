package model

const CapeEmblemCount = 10

// AppearanceCapeEmblem selects the existing 7.48 texture variant. Invalid or
// absent cosmetic data falls back to valid legacy citizenship, never to an
// out-of-range texture. This projection does not change domain citizenship.
func AppearanceCapeEmblem(ch Char) byte {
	if ch.CapeEmblem >= 1 && ch.CapeEmblem <= CapeEmblemCount {
		return ch.CapeEmblem
	}
	if ch.Citizenship <= CapeEmblemCount {
		return ch.Citizenship
	}
	return 0
}
