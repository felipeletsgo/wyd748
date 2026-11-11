package store

import "testing"

func TestNilIfAllDistinguishesWholeAndPartialGuildReads(t *testing.T) {
	ids := []int32{2, 7}
	if got := nilIfAll(true, ids); got != nil {
		t.Fatalf("whole-registry read IDs=%v, want nil", got)
	}
	if got := nilIfAll(false, ids); len(got) != len(ids) || got[0] != ids[0] || got[1] != ids[1] {
		t.Fatalf("partial guild read IDs=%v, want %v", got, ids)
	}
}
