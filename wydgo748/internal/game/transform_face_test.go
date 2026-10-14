package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

// W2PP GetCurrentScore Type 16 refines the transformed face from the fourth
// mastery (Nature) and the level; the client draws that refinement as glow.
func TestTransformationGlowGrowsWithNature(t *testing.T) {
	ch := &model.Char{Class: 2, Evolution: "mortal", Score: testScore(model.Score{Level: 100})}
	ch.Affects[0] = model.Affect{Type: 16, Value: 1, Level: 200, ExpiresAt: time.Now().Add(time.Minute)}
	glow := func(nature uint32) (uint16, byte) {
		ch.Score.Mastery[3] = nature
		ch.RuntimeScore = nil
		return bodyMesh(ch)[0], bodyAncient(ch)[0]
	}
	// (0 + 100*2)/3 = 66 -> (66-15)/12 = 4 for the Wolf.
	if mesh, anct := glow(0); mesh != 22+4*0x1000 || anct != 43 {
		t.Fatalf("Wolf at Nature 0: mesh=%#x anct=%d", mesh, anct)
	}
	// (200 + 200)/3 = 133 -> (133-15)/12 = 9, the cap.
	if mesh, _ := glow(200); mesh != 22+9*0x1000 {
		t.Fatalf("Wolf at Nature 200: mesh=%#x, want +9", mesh)
	}
	// Titan's base is 155: low Nature and level leave no glow.
	ch.Affects[0].Value = 4
	ch.Score.Level = 10
	if mesh, anct := glow(0); mesh != 25 || anct != 0 {
		t.Fatalf("Titan without Nature: mesh=%#x anct=%d", mesh, anct)
	}
}
