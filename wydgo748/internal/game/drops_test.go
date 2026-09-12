package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

func TestDropRateForSlotUsesNativeLevelBands(t *testing.T) {
	w := &World{}
	w.dropRates[0] = 900
	w.dropRates[8] = 999
	w.dropRates[11] = 999
	w.dropRates[24] = 2000
	w.dropRates[60] = 5000

	tests := []struct {
		slot, level int
		wantRate    int
		guaranteed  bool
	}{
		{0, 4, 36, false},   // 4% de 900
		{0, 15, 45, false},  // 5% de 900
		{0, 65, 891, false}, // 99% de 900
		{8, 4, 4, false},    // sobrescrita especial
		{11, 4, 0, true},    // garantido
		{24, 4, 2000, false},
		{60, 4, 4500, false}, // 90% de 5000
	}
	for _, tt := range tests {
		rate, guaranteed := w.dropRateForSlot(tt.slot, tt.level)
		if rate != tt.wantRate || guaranteed != tt.guaranteed {
			t.Fatalf("slot=%d level=%d: rate=%d guaranteed=%v; quer %d/%v",
				tt.slot, tt.level, rate, guaranteed, tt.wantRate, tt.guaranteed)
		}
	}
}

func TestDropRateIsClampedToNativeMaximum(t *testing.T) {
	w := &World{}
	w.dropRates[24] = 50000
	rate, _ := w.dropRateForSlot(24, 100)
	if rate != 32000 {
		t.Fatalf("rate=%d, quer clamp 32000", rate)
	}
}

func TestParseGroundItemRequestsRejectsTruncatedAndWrappedFields(t *testing.T) {
	drop := make([]byte, 32)
	binary.LittleEndian.PutUint32(drop[12:16], placeInv)
	binary.LittleEndian.PutUint32(drop[16:20], 62)
	binary.LittleEndian.PutUint32(drop[20:24], 7)
	if req, ok := parseDropItemRequest(drop); !ok || req.srcPos != 62 || req.rotate != 7 {
		t.Fatalf("drop valido rejeitado: %+v ok=%v", req, ok)
	}
	if _, ok := parseDropItemRequest(drop[:31]); ok {
		t.Fatal("drop truncado foi aceito")
	}
	// The native handler accepts Cargo type 2 through its full persisted range.
	binary.LittleEndian.PutUint32(drop[12:16], placeStorage)
	binary.LittleEndian.PutUint32(drop[16:20], model.PlayerCargoSlots-1)
	if req, ok := parseDropItemRequest(drop); !ok || req.srcType != placeStorage ||
		req.srcPos != model.PlayerCargoSlots-1 {
		t.Fatalf("drop de Cargo valido rejeitado: %+v ok=%v", req, ok)
	}
	if _, ok := parseDropItemRequest(append(drop, 0)); ok {
		t.Fatal("drop com cauda fora do contrato exato de 32 bytes foi aceito")
	}
	binary.LittleEndian.PutUint32(drop[12:16], placeInv)
	binary.LittleEndian.PutUint32(drop[16:20], 256) // antes truncava para slot 0
	if _, ok := parseDropItemRequest(drop); ok {
		t.Fatal("SourPos 256 foi truncado/aceito")
	}
	binary.LittleEndian.PutUint32(drop[16:20], 0)
	binary.LittleEndian.PutUint32(drop[12:16], placeEquip)
	if _, ok := parseDropItemRequest(drop); ok {
		t.Fatal("drop direto de equipamento foi aceito")
	}
	binary.LittleEndian.PutUint32(drop[12:16], placeInv)
	binary.LittleEndian.PutUint32(drop[16:20], model.PlayerCarrySlots)
	if _, ok := parseDropItemRequest(drop); ok {
		t.Fatal("slot invisivel 63 foi aceito no drop")
	}

	get := make([]byte, 28)
	binary.LittleEndian.PutUint32(get[12:16], placeInv)
	binary.LittleEndian.PutUint32(get[16:20], 62)
	binary.LittleEndian.PutUint16(get[20:22], 10000)
	if req, ok := parseGetItemRequest(get); !ok || req.destPos != 62 || req.itemID != 10000 {
		t.Fatalf("get valido rejeitado: %+v ok=%v", req, ok)
	}
	if _, ok := parseGetItemRequest(get[:27]); ok {
		t.Fatal("get truncado foi aceito")
	}
	binary.LittleEndian.PutUint32(get[16:20], model.PlayerCarrySlots)
	if _, ok := parseGetItemRequest(get); ok {
		t.Fatal("DestPos fora do carry foi aceito")
	}
}
