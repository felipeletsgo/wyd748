package graphics

import (
	"errors"
	"testing"
	"time"

	"wydclient748/internal/assets"
)

func TestSampleAnimationClipUsesTMProjectTickLayout(t *testing.T) {
	animation := testAnimationClip()

	tests := []struct {
		elapsed time.Duration
		wantX   float32
		wantY   float32
	}{
		{elapsed: 0, wantX: 10, wantY: 20},
		{elapsed: 119 * time.Millisecond, wantX: 10, wantY: 20},
		{elapsed: 120 * time.Millisecond, wantX: 30, wantY: 40},
		{elapsed: 240 * time.Millisecond, wantX: 10, wantY: 20},
	}
	for _, tc := range tests {
		matrices, err := SampleAnimationClip(animation, 0, tc.elapsed, false)
		if err != nil {
			t.Fatal(err)
		}
		if len(matrices) != 2 || matrices[0][12] != tc.wantX || matrices[1][13] != tc.wantY {
			t.Fatalf("elapsed %s: unexpected pose %+v", tc.elapsed, matrices)
		}
	}
}

func TestSampleAnimationClipSmoothsFourSubstepsAndWraps(t *testing.T) {
	animation := testAnimationClip()

	quarter, err := SampleAnimationClip(animation, 0, 30*time.Millisecond, true)
	if err != nil {
		t.Fatal(err)
	}
	if quarter[0][12] != 15 || quarter[1][13] != 25 {
		t.Fatalf("unexpected quarter-step pose: %+v", quarter)
	}

	wrap, err := SampleAnimationClip(animation, 0, 210*time.Millisecond, true)
	if err != nil {
		t.Fatal(err)
	}
	if wrap[0][12] != 15 || wrap[1][13] != 25 {
		t.Fatalf("unexpected wrapped pose: %+v", wrap)
	}

	if animation.Matrices[0][12] != 10 || animation.Matrices[1][13] != 20 {
		t.Fatal("SampleAnimationClip mutated the ANI asset")
	}
}

func TestApplyAnimationClipFeedsSkeletonHierarchy(t *testing.T) {
	skeleton, err := BuildSkeleton(assets.BoneFile{Links: []assets.BoneLink{
		{ParentID: ^uint32(0), BoneID: 0},
		{ParentID: 0, BoneID: 1},
	}})
	if err != nil {
		t.Fatal(err)
	}
	animation := testAnimationClip()
	if err := skeleton.ApplyAnimationClip(animation, 0, 120*time.Millisecond, false); err != nil {
		t.Fatal(err)
	}

	palette, err := skeleton.BuildSkinPalette(assets.Mesh{
		Palette:   []assets.MeshMatrix{identityMeshMatrix()},
		BoneNames: []uint32{1},
	})
	if err != nil {
		t.Fatal(err)
	}
	// Bone 1 receives Y=40 and inherits X=30 from BON bone 0. The lookup is
	// intentionally for bone 1 because BoneName 0 resolves to the synthetic root.
	if palette[0][12] != 30 || palette[0][13] != 40 {
		t.Fatalf("unexpected animated hierarchy palette: %+v", palette[0])
	}
}

func TestSampleAnimationClipRejectsInvalidState(t *testing.T) {
	animation := testAnimationClip()
	zeroFrames := *animation
	zeroFrames.FrameCount = 0
	zeroTicks := *animation
	zeroTicks.Clips = append([]assets.BoneAnimationClip(nil), animation.Clips...)
	zeroTicks.Clips[0].TickCount = 0
	negativeOffset := *animation
	negativeOffset.Clips = append([]assets.BoneAnimationClip(nil), animation.Clips...)
	negativeOffset.Clips[0].MatrixOffset = -1
	badRange := *animation
	badRange.Clips = append([]assets.BoneAnimationClip(nil), animation.Clips...)
	badRange.Clips[0].MatrixOffset = len(animation.Matrices)

	tests := []struct {
		animation *assets.BoneAnimation
		clip      int
		elapsed   time.Duration
	}{
		{animation: nil, clip: 0},
		{animation: animation, clip: 1},
		{animation: animation, clip: 0, elapsed: -time.Millisecond},
		{animation: &zeroFrames, clip: 0},
		{animation: &zeroTicks, clip: 0},
		{animation: &negativeOffset, clip: 0},
		{animation: &badRange, clip: 0},
	}
	for i, tc := range tests {
		_, err := SampleAnimationClip(tc.animation, tc.clip, tc.elapsed, false)
		if !errors.Is(err, ErrInvalidAnimationPose) {
			t.Fatalf("case %d: expected ErrInvalidAnimationPose, got %v", i, err)
		}
	}
}

func testAnimationClip() *assets.BoneAnimation {
	tick0Bone0 := identityMeshMatrix()
	tick0Bone0[12] = 10
	tick0Bone1 := identityMeshMatrix()
	tick0Bone1[13] = 20
	tick1Bone0 := identityMeshMatrix()
	tick1Bone0[12] = 30
	tick1Bone1 := identityMeshMatrix()
	tick1Bone1[13] = 40
	return &assets.BoneAnimation{
		FrameCount:    2,
		FullTickCount: 2,
		Clips: []assets.BoneAnimationClip{{
			ValidIndex:   100,
			TickCount:    2,
			MatrixOffset: 0,
		}},
		Matrices: []assets.MeshMatrix{tick0Bone0, tick0Bone1, tick1Bone0, tick1Bone1},
	}
}
