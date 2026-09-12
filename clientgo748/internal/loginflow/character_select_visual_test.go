package loginflow

import (
	"math"
	"testing"

	"wydclient748/internal/assets"
	"wydclient748/internal/ui"
)

func TestCharacterStand02ClipIndexUsesValidIndexMapping(t *testing.T) {
	const skin = 7
	set := &assets.BoneAnimationSet{}
	set.ValidIndices[skin][characterStand02Slot] = 42
	set.Entries[skin] = &assets.BoneAnimation{Clips: []assets.BoneAnimationClip{
		{ValidIndex: 10},
		{ValidIndex: 42},
		{ValidIndex: 99},
	}}

	got, err := characterStand02ClipIndex(set, skin)
	if err != nil {
		t.Fatal(err)
	}
	if got != 1 {
		t.Fatalf("STAND02 clip=%d want=1", got)
	}
}

func TestCharacterSelectTransformMatches748Slots(t *testing.T) {
	const epsilon = 0.0005
	for slot := 0; slot < 4; slot++ {
		got := characterSelectTransform(slot)
		time := float32(slot) * 0.333
		wantX := float32(2053.0) + (float32(2048.2)-float32(2053.0))*time
		wantZ := float32(2048.2) + (float32(2053.0)-float32(2048.2))*time
		if math.Abs(float64(got.Position.X-wantX)) > epsilon || math.Abs(float64(got.Position.Z-wantZ)) > epsilon {
			t.Fatalf("slot %d position=%+v want X=%f Z=%f", slot, got.Position, wantX, wantZ)
		}
		if got.Position.Y != 0 || got.Yaw != -45 || got.Scale != 1 {
			t.Fatalf("slot %d transform=%+v", slot, got)
		}
	}
}

func TestCharacterSelectRootForResponsiveViewports(t *testing.T) {
	tests := []struct {
		width  int32
		height int32
		want   ui.Rect
	}{
		{800, 600, ui.Rect{X: 476, Y: 115, Width: 249, Height: 370}},
		{1024, 768, ui.Rect{X: 644, Y: 199, Width: 249, Height: 370}},
		{1280, 960, ui.Rect{X: 836, Y: 295, Width: 249, Height: 370}},
	}
	for _, tt := range tests {
		got := characterSelectRootFor(viewportProbe{width: tt.width, height: tt.height})
		if got != tt.want {
			t.Fatalf("viewport %dx%d root=%+v want=%+v", tt.width, tt.height, got, tt.want)
		}
	}
}

func TestCopyCharacterSelectControlsOwnsWords(t *testing.T) {
	controls := []assets.SceneControl{
		{Kind: 1, Words: []int32{characterSelectRootID, 0, 0, 218, 547, characterSelectRootWidth, characterSelectRootHeight, 0, 0, 1}},
		{Kind: 2, Words: []int32{characterSelectEnterID, characterSelectRootID, 0, 10, 343, 86, 21, 0, 0, 15}},
	}

	owned, err := copyCharacterSelectControls(controls)
	if err != nil {
		t.Fatal(err)
	}
	controls[0].Words[3] = 999
	controls[1].Words[0] = 0
	if owned[0].Words[3] != 218 || owned[1].Words[0] != characterSelectEnterID {
		t.Fatalf("copied controls alias input: %+v", owned)
	}

	root := ui.Rect{X: 476, Y: 115, Width: 249, Height: 370}
	rect, ok := characterSelectControlRect(owned, characterSelectEnterID, root)
	if !ok {
		t.Fatal("official select button was not resolved")
	}
	want := ui.Rect{X: 486, Y: 458, Width: 86, Height: 21}
	if rect != want {
		t.Fatalf("select button=%+v want=%+v", rect, want)
	}
}

func TestCopyCharacterSelectStringsOwnsTable(t *testing.T) {
	table := map[int32]string{15: "Connect"}
	owned := copyCharacterSelectStrings(table)
	table[15] = "changed"
	if owned[15] != "Connect" {
		t.Fatalf("copied string table aliases input: %q", owned[15])
	}
}
