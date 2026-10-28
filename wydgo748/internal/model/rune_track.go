package model

// RuneTrackGeneratorRoom maps historical generator labels to ticket refinements.
// Room +6 includes the four trial guardians after the ordinary room ranges.
func RuneTrackGeneratorRoom(label int) int {
	switch {
	case label >= 5662 && label <= 5714:
		return 0
	case label >= 5715 && label <= 5774:
		return 1
	case label >= 5798 && label <= 5857:
		return 2
	case label >= 5957 && label <= 5984:
		return 3
	case label >= 5858 && label <= 5907:
		return 4
	case label >= 5908 && label <= 5956:
		return 5
	case label >= 5775 && label <= 5797 || label >= 5985 && label <= 5988:
		return 6
	}
	return -1
}
