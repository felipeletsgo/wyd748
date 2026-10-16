package wire

import (
	"encoding/binary"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestDistinctBuffVisualsAcrossOwnerAndObserverSnapshots(t *testing.T) {
	expires := time.Now().Add(80 * time.Second)
	ch := model.Char{}
	ch.Affects[0] = model.Affect{Type: 24, Value: 7, Level: 40, ExpiresAt: expires}
	ch.Affects[1] = model.Affect{Type: 31, ClientType: 24, Value: 150, Level: 55, ExpiresAt: expires}
	ch.Affects[2] = model.Affect{Type: 31, Value: 10, ExpiresAt: expires}
	before := ch.Affects
	want := []byte{24, 36, 31}
	for _, packet := range []struct {
		name           string
		data           []byte
		offset, stride int
	}{
		{"owner affects", UpdateAffects(9, ch), 12, 8},
		{"score refresh", UpdateScore(9, ch), 153, 2},
		{"spawn or reentry", CreateMobVisual(9, "Test", 2100, 2100, nil, nil, nil, ch.Affects[:], 0), 71, 2},
	} {
		t.Run(packet.name, func(t *testing.T) {
			for i, visual := range want {
				if got := packet.data[packet.offset+i*packet.stride]; got != visual {
					t.Fatalf("slot %d visual=%d, want %d", i, got, visual)
				}
			}
		})
	}
	owner := UpdateAffects(9, ch)
	if owner[13] != 40 || owner[21] != 55 || binary.LittleEndian.Uint16(owner[22:24]) != 150 {
		t.Fatalf("icon correction changed affect level or value: % X", owner[12:28])
	}
	if !reflect.DeepEqual(ch.Affects, before) {
		t.Fatal("wire projection mutated authoritative affects")
	}
}

func TestLegacyCriticalArmorVisualNormalization(t *testing.T) {
	for _, affect := range []model.Affect{
		{Type: 50}, {Type: 50, ClientType: 50}, {Type: 50, ClientType: 24},
		{Type: 31, ClientType: 24}, {Type: 31, ClientType: 36},
	} {
		if got := clientAffectType(affect); got != 36 {
			t.Fatalf("legacy affect %+v projected as %d, want 36", affect, got)
		}
	}
	if got := clientAffectType(model.Affect{Type: 24}); got != 24 {
		t.Fatalf("Samaritan projected as %d, want 24", got)
	}
}
