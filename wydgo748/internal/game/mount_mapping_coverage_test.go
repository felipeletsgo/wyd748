package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestMountFeedMappingCoversNativeTypeGroups(t *testing.T) {
	groups := []struct {
		feedType   int
		mountTypes []int
	}{
		{feedType: 0, mountTypes: []int{0}},
		{feedType: 5, mountTypes: []int{5}},
		{feedType: 6, mountTypes: []int{6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 27}},
		{feedType: 7, mountTypes: []int{19}},
		{feedType: 8, mountTypes: []int{20}},
		{feedType: 9, mountTypes: []int{21, 22, 23, 28}},
		{feedType: 10, mountTypes: []int{24, 25, 26}},
		{feedType: 19, mountTypes: []int{29}},
	}
	for _, group := range groups {
		feed := uint16(2420 + group.feedType)
		for _, mountType := range group.mountTypes {
			for _, base := range []uint16{model.MountBabyBase, model.MountAdultBase} {
				mount := base + uint16(mountType)
				if !feedMatchesMount(feed, mount) {
					t.Errorf("feed %d should match mount %d", feed, mount)
				}
				adultFeed := uint16(3367 + group.feedType)
				if !feedMatchesMount(adultFeed, mount) {
					t.Errorf("adult feed %d should match mount %d", adultFeed, mount)
				}
			}
		}
	}
	if feedMatchesMount(2420+6, model.MountBabyBase+5) || feedMatchesMount(2419, model.MountBabyBase) {
		t.Fatal("unrelated or out-of-range feed matched a mount")
	}
}

func TestMountConsumableByteAndEssenceBoundaries(t *testing.T) {
	for _, tc := range []struct{ input, want int }{
		{input: -1, want: 0}, {input: 0, want: 0}, {input: 255, want: 255}, {input: 256, want: 255},
	} {
		if got := clampMountByte(tc.input); got != tc.want {
			t.Errorf("clampMountByte(%d) = %d, want %d", tc.input, got, tc.want)
		}
	}
	if essenceMatchesMount(2389, model.MountBabyBase) {
		t.Fatal("essence below the table matched a mount")
	}
	if !essenceMatchesMount(2390+4, model.MountBabyBase+4) || essenceMatchesMount(2390+4, model.MountBabyBase+5) {
		t.Fatal("essence type mapping is incorrect")
	}
	if !essenceMatchesMount(2390+model.MountTypeCount+4, model.MountAdultBase+4) {
		t.Fatal("essence type mapping did not repeat at the native type count")
	}
}
