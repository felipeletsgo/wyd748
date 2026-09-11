package assets

import (
	"bufio"
	"encoding/binary"
	"errors"
	"fmt"
	"math"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

const (
	MaxBoneAnimationList  = 100
	MaxValidAnimationList = 186
)

var (
	ErrInvalidBoneAnimation = errors.New("clientgo748: invalid bone animation data")
	ErrTruncatedAnimation   = errors.New("clientgo748: truncated ANI animation")
)

// BoneLink representa o par parent/id consumido pelo TMSkinMesh ao montar a
// hierarquia de frames. ParentID == 0xffffffff representa a raiz no TMProject.
type BoneLink struct {
	ParentID uint32
	BoneID   uint32
}

// BoneFile preserva o payload BON completo porque arquivos 7.48 válidos podem
// conter bytes depois do último par parent/id completo. Links expõe somente os
// registros completos de oito bytes, equivalente a numBone = fileSize / 8 no
// TMProject, sem perder o tamanho bruto necessário por outros contratos.
type BoneFile struct {
	Raw   []byte
	Links []BoneLink
}

// AnimationFile é a representação Go do framing ANI validado no TMProject:
// dois DWORDs seguidos por TickCount*FrameCount matrizes 4x4 de float32.
type AnimationFile struct {
	TickCount  uint32
	FrameCount uint32
	Matrices   []MeshMatrix
}

type BoneAnimationCatalogEntry struct {
	Index              int
	AnimationTypeCount int
	Parts              int
	BaseName           string
}

type BoneAnimationClip struct {
	ValidIndex   int32
	TickCount    uint32
	MatrixOffset int
}

// BoneAnimation preserva a semântica útil de stBoneAni sem transportar
// ponteiros nativos, ABI C++ ou ownership do Direct3D para o client Go.
type BoneAnimation struct {
	Name               string
	Parts              int
	AnimationTypeCount int
	Bone               BoneFile
	FrameCount         uint32
	FullTickCount      uint32
	Clips              []BoneAnimationClip
	Matrices           []MeshMatrix
}

// BoneAnimationSet é o estado de assets correspondente à carga feita por
// MeshManager::InitBoneAnimation. A política de lookup/fallback de animação
// permanece fora deste tipo para manter parsing e seleção runtime separados.
type BoneAnimationSet struct {
	Entries      [MaxBoneAnimationList]*BoneAnimation
	ValidIndices [MaxBoneAnimationList][MaxValidAnimationList]int32
}

func ParseBON(data []byte) BoneFile {
	raw := append([]byte(nil), data...)
	links := make([]BoneLink, len(data)/8)
	for i := range links {
		off := i * 8
		links[i] = BoneLink{
			ParentID: binary.LittleEndian.Uint32(data[off : off+4]),
			BoneID:   binary.LittleEndian.Uint32(data[off+4 : off+8]),
		}
	}
	return BoneFile{Raw: raw, Links: links}
}

func ParseANI(data []byte) (AnimationFile, error) {
	if len(data) < 8 {
		return AnimationFile{}, ErrTruncatedAnimation
	}
	ticks := binary.LittleEndian.Uint32(data[0:4])
	frames := binary.LittleEndian.Uint32(data[4:8])
	matrixCount := uint64(ticks) * uint64(frames)
	bodySize := matrixCount * 64
	if matrixCount > uint64(^uint(0)>>1) || bodySize > uint64(^uint(0)>>1) {
		return AnimationFile{}, fmt.Errorf("%w: ANI dimensions overflow", ErrInvalidBoneAnimation)
	}
	expected := uint64(8) + bodySize
	if expected > uint64(len(data)) {
		return AnimationFile{}, ErrTruncatedAnimation
	}
	if expected != uint64(len(data)) {
		return AnimationFile{}, fmt.Errorf("%w: ANI has %d trailing bytes", ErrInvalidBoneAnimation, uint64(len(data))-expected)
	}

	matrices := make([]MeshMatrix, int(matrixCount))
	off := 8
	for i := range matrices {
		for j := range matrices[i] {
			matrices[i][j] = math.Float32frombits(binary.LittleEndian.Uint32(data[off : off+4]))
			off += 4
		}
	}
	return AnimationFile{TickCount: ticks, FrameCount: frames, Matrices: matrices}, nil
}

func ParseValidAnimationIndices(data []byte) ([MaxBoneAnimationList][MaxValidAnimationList]int32, error) {
	var indices [MaxBoneAnimationList][MaxValidAnimationList]int32
	const expected = MaxBoneAnimationList * MaxValidAnimationList * 4
	if len(data) != expected {
		return indices, fmt.Errorf("%w: ValidIndex.bin size %d, want %d", ErrInvalidBoneAnimation, len(data), expected)
	}
	off := 0
	for class := range indices {
		for slot := range indices[class] {
			indices[class][slot] = int32(binary.LittleEndian.Uint32(data[off : off+4]))
			off += 4
		}
	}
	return indices, nil
}

func ParseBoneAnimationCatalog(data []byte) ([]BoneAnimationCatalogEntry, error) {
	scanner := bufio.NewScanner(strings.NewReader(string(data)))
	entries := make([]BoneAnimationCatalogEntry, 0, MaxBoneAnimationList)
	seen := make(map[int]struct{}, MaxBoneAnimationList)
	lineNumber := 0
	for scanner.Scan() {
		lineNumber++
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		fields := strings.Fields(line)
		if len(fields) != 4 {
			return nil, fmt.Errorf("%w: BoneAni4 line %d has %d fields", ErrInvalidBoneAnimation, lineNumber, len(fields))
		}
		index, err := strconv.Atoi(fields[0])
		if err != nil || index < 0 || index >= MaxBoneAnimationList {
			return nil, fmt.Errorf("%w: BoneAni4 line %d invalid index", ErrInvalidBoneAnimation, lineNumber)
		}
		animationTypes, err := strconv.Atoi(fields[1])
		if err != nil || animationTypes < 0 || animationTypes > MaxValidAnimationList {
			return nil, fmt.Errorf("%w: BoneAni4 line %d invalid animation count", ErrInvalidBoneAnimation, lineNumber)
		}
		parts, err := strconv.Atoi(fields[2])
		if err != nil || parts < 0 {
			return nil, fmt.Errorf("%w: BoneAni4 line %d invalid parts", ErrInvalidBoneAnimation, lineNumber)
		}
		if len(fields[3]) >= 128 {
			return nil, fmt.Errorf("%w: BoneAni4 line %d name too long", ErrInvalidBoneAnimation, lineNumber)
		}
		if _, ok := seen[index]; ok {
			return nil, fmt.Errorf("%w: BoneAni4 duplicate index %d", ErrInvalidBoneAnimation, index)
		}
		seen[index] = struct{}{}
		entries = append(entries, BoneAnimationCatalogEntry{
			Index:              index,
			AnimationTypeCount: animationTypes,
			Parts:              parts,
			BaseName:           fields[3],
		})
	}
	if err := scanner.Err(); err != nil {
		return nil, fmt.Errorf("clientgo748: scan BoneAni4: %w", err)
	}
	return entries, nil
}

// LoadBoneAnimationSet porta a semântica de arquivos de
// MeshManager::InitBoneAnimation, autorizada como referência para parsers de
// assets 7.48. ANI ausente é ignorado como no TMProject; BON/catálogo ausente é
// fatal. ValidIndex.bin é opcional e, quando ausente, permanece zerado.
func LoadBoneAnimationSet(assetRoot string) (BoneAnimationSet, error) {
	var set BoneAnimationSet
	meshDir := filepath.Join(assetRoot, "mesh")
	catalogData, err := os.ReadFile(filepath.Join(meshDir, "BoneAni4.txt"))
	if err != nil {
		return set, fmt.Errorf("clientgo748: read BoneAni4.txt: %w", err)
	}
	entries, err := ParseBoneAnimationCatalog(catalogData)
	if err != nil {
		return set, err
	}

	validPath := filepath.Join(meshDir, "ValidIndex.bin")
	validData, err := os.ReadFile(validPath)
	if err == nil {
		set.ValidIndices, err = ParseValidAnimationIndices(validData)
		if err != nil {
			return set, err
		}
	} else if !errors.Is(err, os.ErrNotExist) {
		return set, fmt.Errorf("clientgo748: read ValidIndex.bin: %w", err)
	}

	for _, entry := range entries {
		animation, err := loadBoneAnimationEntry(assetRoot, entry, &set.ValidIndices[entry.Index])
		if err != nil {
			return set, fmt.Errorf("clientgo748: load bone animation %d %q: %w", entry.Index, entry.BaseName, err)
		}
		set.Entries[entry.Index] = animation
	}
	return set, nil
}

func resolveAssetPathCaseInsensitive(assetRoot, relativePath string) (string, error) {
	normalized := filepath.FromSlash(strings.ReplaceAll(relativePath, "\\", "/"))
	candidate := filepath.Join(assetRoot, normalized)
	if _, err := os.Stat(candidate); err == nil {
		return candidate, nil
	} else if !errors.Is(err, os.ErrNotExist) {
		return "", err
	}

	current := assetRoot
	for _, component := range strings.Split(filepath.Clean(normalized), string(filepath.Separator)) {
		if component == "" || component == "." {
			continue
		}

		exact := filepath.Join(current, component)
		if _, err := os.Stat(exact); err == nil {
			current = exact
			continue
		} else if !errors.Is(err, os.ErrNotExist) {
			return "", err
		}

		entries, err := os.ReadDir(current)
		if err != nil {
			return "", err
		}

		matched := ""
		for _, entry := range entries {
			if strings.EqualFold(entry.Name(), component) {
				matched = entry.Name()
				break
			}
		}
		if matched == "" {
			return "", &os.PathError{Op: "resolve", Path: candidate, Err: os.ErrNotExist}
		}
		current = filepath.Join(current, matched)
	}

	return current, nil
}

func loadBoneAnimationEntry(assetRoot string, entry BoneAnimationCatalogEntry, valid *[MaxValidAnimationList]int32) (*BoneAnimation, error) {
	bonePath, err := resolveAssetPathCaseInsensitive(assetRoot, entry.BaseName+".bon")
	if err != nil {
		return nil, fmt.Errorf("read BON: %w", err)
	}
	boneData, err := os.ReadFile(bonePath)
	if err != nil {
		return nil, fmt.Errorf("read BON: %w", err)
	}
	base := strings.TrimSuffix(bonePath, filepath.Ext(bonePath))
	result := &BoneAnimation{
		Name:               entry.BaseName,
		Parts:              entry.Parts,
		AnimationTypeCount: entry.AnimationTypeCount,
		Bone:               ParseBON(boneData),
		Clips:              make([]BoneAnimationClip, 0, entry.AnimationTypeCount),
	}

	for slot := 0; slot < entry.AnimationTypeCount; slot++ {
		validIndex := valid[slot]
		if validIndex < 0 {
			return nil, fmt.Errorf("%w: negative valid animation index %d", ErrInvalidBoneAnimation, validIndex)
		}
		aniPath := fmt.Sprintf("%s%04d.ani", base, int64(validIndex)+1)
		aniData, err := os.ReadFile(aniPath)
		if errors.Is(err, os.ErrNotExist) {
			continue
		}
		if err != nil {
			return nil, fmt.Errorf("read ANI %q: %w", aniPath, err)
		}
		ani, err := ParseANI(aniData)
		if err != nil {
			return nil, fmt.Errorf("parse ANI %q: %w", aniPath, err)
		}
		if len(result.Clips) == 0 {
			result.FrameCount = ani.FrameCount
		} else if ani.FrameCount != result.FrameCount {
			return nil, fmt.Errorf("%w: ANI frame count changed from %d to %d", ErrInvalidBoneAnimation, result.FrameCount, ani.FrameCount)
		}
		if uint64(result.FullTickCount)+uint64(ani.TickCount) > math.MaxUint32 {
			return nil, fmt.Errorf("%w: animation tick count overflow", ErrInvalidBoneAnimation)
		}
		result.Clips = append(result.Clips, BoneAnimationClip{
			ValidIndex:   validIndex,
			TickCount:    ani.TickCount,
			MatrixOffset: len(result.Matrices),
		})
		result.FullTickCount += ani.TickCount
		result.Matrices = append(result.Matrices, ani.Matrices...)
	}

	return result, nil
}
