package game

import (
	"encoding/json"
	"os"
	"strconv"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// The client reads affect word 40 as a stun (no movement, attack or skills).
// The transformation must reach the client in the Transformation slot 16 while
// the server keeps its own semantic type 40 for the face mesh.
func TestFaceTransformUsesNonBlockingClientAffectSlot(t *testing.T) {
	ch := &model.Char{Score: testScore(model.Score{})}
	now := time.Now()
	if !setFaceAffectAt(ch, 216, 75, now) {
		t.Fatal("face transformation was not applied")
	}
	affect := ch.Affects[0]
	if affect.Type != affectFaceTransform || affect.Value != 216 {
		t.Fatalf("server affect=%+v want type %d value 216", affect, affectFaceTransform)
	}
	if affect.ClientType != affectFaceTransformClientType || affectFaceTransformClientType == 40 {
		t.Fatalf("client slot=%d must not be the stun slot 40", affect.ClientType)
	}
	if got := affect.ExpiresAt.Sub(now); got != 10*time.Minute {
		t.Fatalf("75 units last %v want 10m", got)
	}
	if mesh := bodyMeshAt(ch, now); mesh[0] != 216 {
		t.Fatalf("body face=%d want 216", mesh[0])
	}
	// MSG_UpdateScore affect words: low byte = 8 s units, high byte = slot.
	pkt := wire.UpdateScore(1, *ch)
	if pkt[153] != affectFaceTransformClientType {
		t.Fatalf("published affect slot=%d want %d", pkt[153], affectFaceTransformClientType)
	}
}

func TestTransformationLettersLastTenMinutes(t *testing.T) {
	raw, err := os.ReadFile("../../data/content/volatiles.json")
	if err != nil {
		t.Fatalf("read volatiles: %v", err)
	}
	var file struct {
		Rules map[string]struct {
			Action        string `json:"action"`
			DurationUnits int    `json:"durationUnits"`
		} `json:"rules"`
	}
	if err := json.Unmarshal(raw, &file); err != nil {
		t.Fatalf("parse volatiles: %v", err)
	}
	for code := 70; code <= 77; code++ {
		rule, ok := file.Rules[strconv.Itoa(code)]
		if !ok || rule.Action != "face_transform" {
			t.Fatalf("volatile %d is not a face transformation", code)
		}
		if got := time.Duration(rule.DurationUnits*8) * time.Second; got != 10*time.Minute {
			t.Fatalf("volatile %d lasts %v want 10m", code, got)
		}
	}
}
