package wire

import (
	"bytes"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestTimedSendItemProjectsOnlyWireCopy(t *testing.T) {
	before := time.Now()
	item := model.Item{Index: 3900, UID: "authoritative-only", ActivatedUnix: before.Unix(),
		ExpiresUnix: before.Add(72 * time.Hour).Unix()}
	packet := SendItem(1, 1, 3, item)
	after := time.Now()
	if len(packet) != 24 || !bytes.Equal(packet[16:18], []byte{0x3c, 0x0f}) {
		t.Fatalf("timed item changed the legacy frame: %x", packet)
	}
	first := model.ExpiryDisplayEffects(item, before)
	last := model.ExpiryDisplayEffects(item, after)
	if !bytes.Equal(packet[18:24], first[:]) && !bytes.Equal(packet[18:24], last[:]) {
		t.Fatalf("wire tooltip is not a remaining-time snapshot: %x", packet[18:24])
	}
	if item.Eff != [6]byte{} || item.UID != "authoritative-only" {
		t.Fatal("wire projection mutated authoritative item metadata")
	}
	// A client echo carries only the eight legacy bytes; expiry, UID and the
	// first activation timestamp remain server owned after matching the echo.
	echo := model.Item{Index: item.Index}
	copy(echo.Eff[:], packet[18:24])
	if !item.WireEqual(echo) {
		t.Fatal("a projected legacy item echo no longer matches its authoritative copy")
	}
}
