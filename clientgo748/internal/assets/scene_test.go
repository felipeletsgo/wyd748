package assets

import (
	"os"
	"testing"
)

func TestParseActiveServerAndLoginScene(t *testing.T) {
	data, err := os.ReadFile("../../assets/current/UI/SelServerScene2.bin")
	if err != nil {
		t.Fatal(err)
	}
	controls, err := ParseScene(data)
	if err != nil {
		t.Fatal(err)
	}
	if len(controls) != 22 {
		t.Fatalf("controls = %d", len(controls))
	}
	for i, id := range []int32{4608, 5121, 5122, 4609, 4611, 4610, 5632, 5633, 5634} {
		if controls[13+i].Words[0] != id {
			t.Fatalf("control %d: want %d", 13+i, id)
		}
	}
	for _, length := range []int{0, 1, 3, 43, len(data) - 1} {
		if got, err := ParseScene(data[:length]); err == nil || got != nil {
			t.Fatalf("accepted truncation %d", length)
		}
	}
	data[4] ^= 0xff
	if controls[0].Words[0] != 4623 {
		t.Fatal("parser retained input storage")
	}
}

func TestParseSceneRejectsInactiveInlineVariant(t *testing.T) {
	data, err := os.ReadFile("../../assets/current/UI/LoginScene2.bin")
	if err != nil {
		t.Fatal(err)
	}
	if got, err := ParseScene(data); err == nil || got != nil {
		t.Fatal("accepted incompatible inline resource")
	}
}

func TestFindControlUsesNativeID(t *testing.T) {
	data, err := os.ReadFile("../../assets/current/UI/SelServerScene2.bin")
	if err != nil { t.Fatal(err) }
	controls, err := ParseScene(data)
	if err != nil { t.Fatal(err) }
	control, ok := FindControl(controls, 5121)
	if !ok || control.Kind != 13 { t.Fatalf("account edit not resolved: kind=%d ok=%v", control.Kind, ok) }
	if _, ok := FindControl(controls, 999999); ok { t.Fatal("unexpected control") }
}
