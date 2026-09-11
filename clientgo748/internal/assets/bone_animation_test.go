package assets

import (
	"encoding/binary"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"testing"
)

func TestBoneAnimationParsersOfficialAssets(t *testing.T) {
	meshDir := filepath.Join("..", "..", "assets", "current", "mesh")
	catalogData, err := os.ReadFile(filepath.Join(meshDir, "BoneAni4.txt"))
	if err != nil {
		t.Fatal(err)
	}
	entries, err := ParseBoneAnimationCatalog(catalogData)
	if err != nil {
		t.Fatal(err)
	}
	if len(entries) == 0 || entries[0].Index != 0 || entries[0].AnimationTypeCount != 186 || entries[0].Parts != 8 || entries[0].BaseName != `mesh\ch01` {
		t.Fatalf("unexpected first catalog entry: %+v", entries)
	}

	validData, err := os.ReadFile(filepath.Join(meshDir, "ValidIndex.bin"))
	if err != nil {
		t.Fatal(err)
	}
	valid, err := ParseValidAnimationIndices(validData)
	if err != nil {
		t.Fatal(err)
	}
	if len(validData) != MaxBoneAnimationList*MaxValidAnimationList*4 {
		t.Fatalf("unexpected ValidIndex size: %d", len(validData))
	}

	boneData, err := os.ReadFile(filepath.Join(meshDir, "ch01.bon"))
	if err != nil {
		t.Fatal(err)
	}
	bone := ParseBON(boneData)
	if len(bone.Raw) != 376 || len(bone.Links) != 47 {
		t.Fatalf("unexpected ch01 BON: bytes=%d links=%d", len(bone.Raw), len(bone.Links))
	}

	aniName := filepath.Join(meshDir, "ch01"+formatANIIndex(valid[0][0]))
	aniData, err := os.ReadFile(aniName)
	if err != nil {
		t.Fatal(err)
	}
	ani, err := ParseANI(aniData)
	if err != nil {
		t.Fatal(err)
	}
	if ani.TickCount == 0 || ani.FrameCount != uint32(len(bone.Links)) || len(ani.Matrices) != int(ani.TickCount*ani.FrameCount) {
		t.Fatalf("unexpected first ch01 ANI: ticks=%d frames=%d matrices=%d", ani.TickCount, ani.FrameCount, len(ani.Matrices))
	}
}

func TestLoadBoneAnimationEntryOfficialAssets(t *testing.T) {
	root := filepath.Join("..", "..", "assets", "current")
	validData, err := os.ReadFile(filepath.Join(root, "mesh", "ValidIndex.bin"))
	if err != nil {
		t.Fatal(err)
	}
	valid, err := ParseValidAnimationIndices(validData)
	if err != nil {
		t.Fatal(err)
	}
	entry := BoneAnimationCatalogEntry{Index: 0, AnimationTypeCount: 186, Parts: 8, BaseName: `mesh\ch01`}
	animation, err := loadBoneAnimationEntry(root, entry, &valid[0])
	if err != nil {
		t.Fatal(err)
	}
	if animation.FrameCount != 47 || animation.FullTickCount == 0 || len(animation.Clips) == 0 {
		t.Fatalf("unexpected ch01 animation: frames=%d ticks=%d clips=%d", animation.FrameCount, animation.FullTickCount, len(animation.Clips))
	}
	if len(animation.Matrices) != int(animation.FrameCount*animation.FullTickCount) {
		t.Fatalf("matrix accumulation mismatch: got=%d want=%d", len(animation.Matrices), animation.FrameCount*animation.FullTickCount)
	}
}

func TestLoadBoneAnimationEntryResolvesCatalogCase(t *testing.T) {
	root := t.TempDir()
	meshDir := filepath.Join(root, "mesh")
	if err := os.MkdirAll(meshDir, 0o755); err != nil {
		t.Fatal(err)
	}
	bonePath := filepath.Join(meshDir, "BL01.bon")
	if err := os.WriteFile(bonePath, make([]byte, 8), 0o644); err != nil {
		t.Fatal(err)
	}

	entry := BoneAnimationCatalogEntry{Index: 44, Parts: 4, BaseName: `mesh\bl01`}
	var valid [MaxValidAnimationList]int32
	animation, err := loadBoneAnimationEntry(root, entry, &valid)
	if err != nil {
		t.Fatal(err)
	}
	if len(animation.Bone.Raw) != 8 {
		t.Fatalf("unexpected BON size: got=%d want=8", len(animation.Bone.Raw))
	}
}

func TestLoadBoneAnimationSetOfficialAssets(t *testing.T) {
	root := filepath.Join("..", "..", "assets", "current")
	set, err := LoadBoneAnimationSet(root)
	if err != nil {
		t.Fatal(err)
	}
	if set.Entries[0] == nil || set.Entries[57] == nil || set.Entries[87] == nil {
		t.Fatal("expected catalog entries 0, 57, and 87 to be loaded")
	}
	if set.Entries[99] != nil {
		t.Fatal("unexpected animation entry 99")
	}
	if set.Entries[0].FrameCount != 47 || len(set.Entries[0].Bone.Raw) != 376 {
		t.Fatalf("unexpected class 0 payload: frames=%d boneBytes=%d", set.Entries[0].FrameCount, len(set.Entries[0].Bone.Raw))
	}
}

func TestParseBONPreservesTrailingBytes(t *testing.T) {
	data := make([]byte, 11)
	binary.LittleEndian.PutUint32(data[0:4], 0xffffffff)
	binary.LittleEndian.PutUint32(data[4:8], 7)
	data[8], data[9], data[10] = 1, 2, 3
	bone := ParseBON(data)
	if len(bone.Raw) != len(data) || len(bone.Links) != 1 || bone.Links[0].ParentID != 0xffffffff || bone.Links[0].BoneID != 7 {
		t.Fatalf("unexpected BON parse: %+v", bone)
	}
	for i := range data {
		if bone.Raw[i] != data[i] {
			t.Fatalf("raw BON changed at byte %d", i)
		}
	}
}

func TestParseANIRejectsMalformedInput(t *testing.T) {
	if _, err := ParseANI([]byte{1, 2, 3}); !errors.Is(err, ErrTruncatedAnimation) {
		t.Fatalf("unexpected short ANI error: %v", err)
	}
	truncated := make([]byte, 8)
	binary.LittleEndian.PutUint32(truncated[0:4], 1)
	binary.LittleEndian.PutUint32(truncated[4:8], 1)
	if _, err := ParseANI(truncated); !errors.Is(err, ErrTruncatedAnimation) {
		t.Fatalf("unexpected truncated ANI error: %v", err)
	}
	trailing := make([]byte, 8+64+1)
	binary.LittleEndian.PutUint32(trailing[0:4], 1)
	binary.LittleEndian.PutUint32(trailing[4:8], 1)
	if _, err := ParseANI(trailing); !errors.Is(err, ErrInvalidBoneAnimation) {
		t.Fatalf("unexpected trailing ANI error: %v", err)
	}
	if _, err := ParseValidAnimationIndices(make([]byte, 4)); !errors.Is(err, ErrInvalidBoneAnimation) {
		t.Fatalf("unexpected ValidIndex error: %v", err)
	}
}

func formatANIIndex(validIndex int32) string {
	return fmt.Sprintf("%04d.ani", int64(validIndex)+1)
}
