package graphics

import (
	"errors"
	"fmt"
	"time"

	"wydclient748/internal/assets"
)

const (
	// DefaultAnimationStep preserva o valor usado por TMSkinMesh::FrameMove
	// quando m_dwFPS ainda não foi configurado. O nome "FPS" no TMProject é
	// enganoso: o campo é usado como intervalo de 30 ms entre substeps.
	DefaultAnimationStep = 30 * time.Millisecond
	animationSubsteps    = 4
)

var ErrInvalidAnimationPose = errors.New("clientgo748: invalid animation pose")

// SampleAnimationClip converte um instante do clip em uma matriz local por
// frame/osso. A disposição ANI é tick-major: FrameCount matrizes consecutivas
// por tick, exatamente como o cálculo numBone*dwTick de TMSkinMesh::FrameMove.
//
// smooth=false reproduz o caminho de amostragem exata. smooth=true reproduz a
// interpolação linear de quatro substeps usada pelo caminho genérico do
// TMProject, inclusive o wrap do último tick para o primeiro tick do clip. A
// transição especial entre clips e o slerp de ch01/ch02 não pertencem a esta
// função e serão aplicados por uma camada de estado de animação separada.
func SampleAnimationClip(animation *assets.BoneAnimation, clipIndex int, elapsed time.Duration, smooth bool) ([]assets.MeshMatrix, error) {
	if animation == nil {
		return nil, fmt.Errorf("%w: animation is nil", ErrInvalidAnimationPose)
	}
	if clipIndex < 0 || clipIndex >= len(animation.Clips) {
		return nil, fmt.Errorf("%w: clip index %d outside %d loaded clips", ErrInvalidAnimationPose, clipIndex, len(animation.Clips))
	}
	if elapsed < 0 {
		return nil, fmt.Errorf("%w: negative elapsed time %s", ErrInvalidAnimationPose, elapsed)
	}
	if animation.FrameCount == 0 || uint64(animation.FrameCount) > uint64(^uint(0)>>1) {
		return nil, fmt.Errorf("%w: invalid frame count %d", ErrInvalidAnimationPose, animation.FrameCount)
	}

	clip := animation.Clips[clipIndex]
	if clip.TickCount == 0 {
		return nil, fmt.Errorf("%w: clip %d has zero ticks", ErrInvalidAnimationPose, clipIndex)
	}
	if clip.MatrixOffset < 0 {
		return nil, fmt.Errorf("%w: clip %d has negative matrix offset %d", ErrInvalidAnimationPose, clipIndex, clip.MatrixOffset)
	}
	frameCount := int(animation.FrameCount)
	clipMatrices := uint64(clip.TickCount) * uint64(animation.FrameCount)
	matrixOffset := uint64(clip.MatrixOffset)
	if clipMatrices > ^uint64(0)-matrixOffset {
		return nil, fmt.Errorf("%w: clip %d matrix range overflows", ErrInvalidAnimationPose, clipIndex)
	}
	clipEnd := matrixOffset + clipMatrices
	if clipEnd > uint64(len(animation.Matrices)) {
		return nil, fmt.Errorf("%w: clip %d matrix range [%d,%d) outside %d matrices", ErrInvalidAnimationPose, clipIndex, clip.MatrixOffset, clipEnd, len(animation.Matrices))
	}

	substep := uint64(elapsed / DefaultAnimationStep)
	cycleSubsteps := uint64(clip.TickCount) * animationSubsteps
	substep %= cycleSubsteps
	currentTick := substep / animationSubsteps
	fraction := substep % animationSubsteps

	currentOffset := clip.MatrixOffset + int(currentTick)*frameCount
	result := append([]assets.MeshMatrix(nil), animation.Matrices[currentOffset:currentOffset+frameCount]...)
	if !smooth || fraction == 0 || clip.TickCount == 1 {
		return result, nil
	}

	nextTick := (currentTick + 1) % uint64(clip.TickCount)
	nextOffset := clip.MatrixOffset + int(nextTick)*frameCount
	alpha := float32(fraction) / animationSubsteps
	for frame := range result {
		for component := range result[frame] {
			current := result[frame][component]
			next := animation.Matrices[nextOffset+frame][component]
			result[frame][component] = current + (next-current)*alpha
		}
	}
	return result, nil
}

// ApplyAnimationClip atualiza a hierarquia com uma amostra ANI e recalcula as
// matrizes combinadas. O Skeleton mantém ownership da hierarquia; o asset ANI
// continua imutável e pode ser compartilhado entre várias instâncias.
func (s *Skeleton) ApplyAnimationClip(animation *assets.BoneAnimation, clipIndex int, elapsed time.Duration, smooth bool) error {
	if s == nil {
		return fmt.Errorf("%w: skeleton is nil", ErrInvalidAnimationPose)
	}
	matrices, err := SampleAnimationClip(animation, clipIndex, elapsed, smooth)
	if err != nil {
		return err
	}
	s.ApplyFrameMatrices(matrices)
	return nil
}
