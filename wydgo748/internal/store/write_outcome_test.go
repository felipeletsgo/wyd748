package store

import (
	"errors"
	"testing"
)

func TestUnsentWriteClassification(t *testing.T) {
	if IsUnsentWrite(nil) {
		t.Fatal("nil error classified as an unsent write")
	}
	ordinary := errors.New("adapter failed")
	if IsUnsentWrite(ordinary) || IsUnsentWrite(rejectWrite(ordinary)) {
		t.Fatal("adapter error classified as a write that never started")
	}

	unsent := RejectUnsentWrite("coordinator queue full")
	if !IsUnsentWrite(unsent) || !IsUnsentWrite(errors.Join(errors.New("request failed"), unsent)) {
		t.Fatal("unsent write marker was not found through the error chain")
	}
	if IsUnsentWrite(indeterminateWrite(unsent)) {
		t.Fatal("unknown commit outcome must override an unsent rejection")
	}
	if got, want := unsent.Error(), "store: write rejected before commit: write never sent: coordinator queue full"; got != want {
		t.Fatalf("unsent rejection error=%q want %q", got, want)
	}
	unknown := indeterminateWrite(errors.New("commit outcome unavailable"))
	if got, want := unknown.Error(), "commit outcome unavailable"; got != want {
		t.Fatalf("unknown write error=%q want %q", got, want)
	}
}
