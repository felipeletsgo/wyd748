package model

import "testing"

func TestInitItemValidation(t *testing.T) {
	for _, test := range []struct {
		name string
		item InitItem
	}{
		{"zero item index", InitItem{X: 1, Y: 1}},
		{"zero x", InitItem{Index: 1, Y: 1}},
		{"zero y", InitItem{Index: 1, X: 1}},
	} {
		t.Run(test.name, func(t *testing.T) {
			if err := test.item.Validate(); err == nil {
				t.Fatal("invalid permanent world item was accepted")
			}
		})
	}
	if err := (InitItem{Index: 1, X: 4095, Y: 4095, Rotate: 3}).Validate(); err != nil {
		t.Fatalf("valid permanent world item was rejected: %v", err)
	}
}
