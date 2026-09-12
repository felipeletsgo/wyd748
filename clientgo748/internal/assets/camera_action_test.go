package assets

import (
	"encoding/binary"
	"errors"
	"math"
	"os"
	"path/filepath"
	"runtime"
	"testing"
	"time"
)

func TestParseOfficialDemoCamAction4(t *testing.T) {
	_, file, _, ok := runtime.Caller(0)
	if !ok {
		t.Fatal("runtime.Caller failed")
	}
	path := filepath.Join(filepath.Dir(file), "..", "..", "assets", "current", "UI", "DemoCamAction4.bin")
	data, err := os.ReadFile(path)
	if err != nil {
		t.Fatalf("ReadFile(DemoCamAction4.bin): %v", err)
	}
	action, err := ParseCameraAction(data)
	if err != nil {
		t.Fatalf("ParseCameraAction(DemoCamAction4.bin): %v", err)
	}
	if len(action.Ticks) != 8 {
		t.Fatalf("ticks = %d, want 8", len(action.Ticks))
	}
	first := action.Ticks[0]
	if first.Tick != 0 || first.Local != 0 || first.X != 1102 || first.Y != 1 || first.Z != 1690.5 || !float32Near(first.HorizonAngle, math.Pi) || first.VerticalAngle != 0 {
		t.Fatalf("first tick = %#v", first)
	}
	last := action.Ticks[len(action.Ticks)-1]
	if last.Tick != 29000 || last.Local != 0 || last.X != 1072 || last.Y != 3.5 || last.Z != 1690 || !float32Near(last.HorizonAngle, -math.Pi) || last.VerticalAngle != 0 {
		t.Fatalf("last tick = %#v", last)
	}
}

func TestParseCameraActionRejectsMalformedInput(t *testing.T) {
	valid := make([]byte, 4+cameraActionRecordSize)
	binary.LittleEndian.PutUint32(valid[:4], 1)
	tests := []struct {
		name string
		data []byte
	}{
		{"short header", []byte{1, 0, 0}},
		{"negative count", []byte{0xff, 0xff, 0xff, 0xff}},
		{"too many ticks", []byte{0xe9, 0x03, 0x00, 0x00}},
		{"truncated tick", valid[:len(valid)-1]},
		{"trailing bytes", append(append([]byte(nil), valid...), 0)},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			_, err := ParseCameraAction(test.data)
			if !errors.Is(err, ErrInvalidCameraAction) {
				t.Fatalf("error = %v, want %v", err, ErrInvalidCameraAction)
			}
		})
	}
}

func TestParseCameraActionOwnsTicks(t *testing.T) {
	data := make([]byte, 4+cameraActionRecordSize)
	binary.LittleEndian.PutUint32(data[:4], 1)
	binary.LittleEndian.PutUint32(data[4:8], 1234)
	action, err := ParseCameraAction(data)
	if err != nil {
		t.Fatal(err)
	}
	binary.LittleEndian.PutUint32(data[4:8], 5678)
	if action.Ticks[0].Tick != 1234 {
		t.Fatalf("tick aliases input: %d", action.Ticks[0].Tick)
	}
}

func TestCameraActionPoseUsesNativeBiasInterpolationAndLocalOffset(t *testing.T) {
	action := CameraAction{Ticks: []CameraActionTick{
		{Tick: 0, Local: 1, X: 10, Y: 20, Z: 30, HorizonAngle: 1, VerticalAngle: 2},
		{Tick: 1000, Local: 1, X: 20, Y: 40, Z: 60, HorizonAngle: 3, VerticalAngle: 4},
	}}
	pose, ok := action.Pose(400*time.Millisecond, [3]float32{100, 200, 300})
	if !ok {
		t.Fatal("Pose returned unavailable")
	}
	// Native evaluation samples elapsed+100ms, so 400ms resolves at t=0.5.
	wantPosition := [3]float32{115, 230, 345}
	if pose.Position != wantPosition || pose.HorizonAngle != 2 || pose.VerticalAngle != 3 {
		t.Fatalf("pose = %#v, want position %#v angles (2,3)", pose, wantPosition)
	}
	final, ok := action.Pose(2*time.Second, [3]float32{100, 200, 300})
	if !ok || final.Position != [3]float32{120, 240, 360} || final.HorizonAngle != 3 || final.VerticalAngle != 4 {
		t.Fatalf("final pose = %#v", final)
	}
}

func TestParseCameraActionRejectsInvalidTickFields(t *testing.T) {
	makeTick := func() []byte {
		data := make([]byte, 4+cameraActionRecordSize)
		binary.LittleEndian.PutUint32(data[:4], 1)
		return data
	}

	invalidLocal := makeTick()
	binary.LittleEndian.PutUint16(invalidLocal[8:10], 2)
	if _, err := ParseCameraAction(invalidLocal); !errors.Is(err, ErrInvalidCameraAction) {
		t.Fatalf("invalid local error = %v", err)
	}

	nonFinite := makeTick()
	binary.LittleEndian.PutUint32(nonFinite[12:16], math.Float32bits(float32(math.Inf(1))))
	if _, err := ParseCameraAction(nonFinite); !errors.Is(err, ErrInvalidCameraAction) {
		t.Fatalf("non-finite error = %v", err)
	}

	unsorted := make([]byte, 4+2*cameraActionRecordSize)
	binary.LittleEndian.PutUint32(unsorted[:4], 2)
	binary.LittleEndian.PutUint32(unsorted[4:8], 1000)
	binary.LittleEndian.PutUint32(unsorted[4+cameraActionRecordSize:8+cameraActionRecordSize], 999)
	if _, err := ParseCameraAction(unsorted); !errors.Is(err, ErrInvalidCameraAction) {
		t.Fatalf("unsorted error = %v", err)
	}
}

func float32Near(got float32, want float64) bool {
	return math.Abs(float64(got)-want) < 1e-5
}
