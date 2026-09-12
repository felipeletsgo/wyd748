package assets

import (
	"encoding/binary"
	"errors"
	"fmt"
	"math"
	"os"
	"time"
)

const (
	cameraActionRecordSize = 28
	maxCameraActionTicks   = 1000
)

var ErrInvalidCameraAction = errors.New("assets: invalid camera action")

// CameraActionTick representa exatamente um stCameraTick do client 7.48.
// Local indica que a posição deve ser somada à posição do personagem local;
// DemoCamAction4 usa apenas coordenadas absolutas (Local == 0).
type CameraActionTick struct {
	Tick          uint32
	Local         int16
	X             float32
	Y             float32
	Z             float32
	HorizonAngle  float32
	VerticalAngle float32
}

// CameraAction mantém a sequência de keyframes na ordem do arquivo nativo.
type CameraAction struct {
	Ticks []CameraActionTick
}

// CameraActionPose is the interpolated camera state consumed by the visual
// login backdrop. Position is expressed in the same world coordinates stored
// by the native stCameraTick records.
type CameraActionPose struct {
	Position      [3]float32
	HorizonAngle  float32
	VerticalAngle float32
}

// ParseCameraAction decodifica o formato lido por TMScene::ReadCameraPos no
// client 7.48: int32 count seguido por count registros stCameraTick de 28 bytes.
// A validação estrita de tamanho evita aceitar arquivos truncados ou misturados.
func ParseCameraAction(data []byte) (CameraAction, error) {
	if len(data) < 4 {
		return CameraAction{}, ErrInvalidCameraAction
	}
	count := int32(binary.LittleEndian.Uint32(data[:4]))
	if count < 0 || count > maxCameraActionTicks {
		return CameraAction{}, fmt.Errorf("%w: invalid tick count %d", ErrInvalidCameraAction, count)
	}
	expected := 4 + int(count)*cameraActionRecordSize
	if len(data) != expected {
		return CameraAction{}, fmt.Errorf("%w: size %d, want %d", ErrInvalidCameraAction, len(data), expected)
	}

	ticks := make([]CameraActionTick, int(count))
	for i := range ticks {
		offset := 4 + i*cameraActionRecordSize
		ticks[i] = CameraActionTick{
			Tick:          binary.LittleEndian.Uint32(data[offset : offset+4]),
			Local:         int16(binary.LittleEndian.Uint16(data[offset+4 : offset+6])),
			X:             math.Float32frombits(binary.LittleEndian.Uint32(data[offset+8 : offset+12])),
			Y:             math.Float32frombits(binary.LittleEndian.Uint32(data[offset+12 : offset+16])),
			Z:             math.Float32frombits(binary.LittleEndian.Uint32(data[offset+16 : offset+20])),
			HorizonAngle:  math.Float32frombits(binary.LittleEndian.Uint32(data[offset+20 : offset+24])),
			VerticalAngle: math.Float32frombits(binary.LittleEndian.Uint32(data[offset+24 : offset+28])),
		}
		if ticks[i].Local != 0 && ticks[i].Local != 1 {
			return CameraAction{}, fmt.Errorf("%w: tick %d has invalid local flag %d", ErrInvalidCameraAction, i, ticks[i].Local)
		}
		if !finiteCameraActionTick(ticks[i]) {
			return CameraAction{}, fmt.Errorf("%w: tick %d contains non-finite values", ErrInvalidCameraAction, i)
		}
		if i > 0 && ticks[i].Tick < ticks[i-1].Tick {
			return CameraAction{}, fmt.Errorf("%w: tick %d is out of order", ErrInvalidCameraAction, i)
		}
	}
	return CameraAction{Ticks: ticks}, nil
}

// Pose reproduces the native camera evaluator's +100 ms sampling bias and
// linear interpolation between surrounding stCameraTick records. A Local tick
// is offset by localPosition; DemoCamAction4 itself uses only absolute ticks.
func (a CameraAction) Pose(elapsed time.Duration, localPosition [3]float32) (CameraActionPose, bool) {
	if len(a.Ticks) == 0 {
		return CameraActionPose{}, false
	}
	sampleMS := elapsed.Milliseconds() + 100
	if sampleMS < 0 {
		sampleMS = 0
	}
	if sampleMS <= int64(a.Ticks[0].Tick) {
		return cameraActionPose(a.Ticks[0], localPosition), true
	}
	last := a.Ticks[len(a.Ticks)-1]
	if sampleMS >= int64(last.Tick) {
		return cameraActionPose(last, localPosition), true
	}
	for i := 1; i < len(a.Ticks); i++ {
		next := a.Ticks[i]
		if sampleMS > int64(next.Tick) {
			continue
		}
		previous := a.Ticks[i-1]
		from := cameraActionPose(previous, localPosition)
		to := cameraActionPose(next, localPosition)
		span := int64(next.Tick) - int64(previous.Tick)
		if span <= 0 {
			return to, true
		}
		t := float32(sampleMS-int64(previous.Tick)) / float32(span)
		return CameraActionPose{
			Position: [3]float32{
				from.Position[0] + (to.Position[0]-from.Position[0])*t,
				from.Position[1] + (to.Position[1]-from.Position[1])*t,
				from.Position[2] + (to.Position[2]-from.Position[2])*t,
			},
			HorizonAngle:  from.HorizonAngle + (to.HorizonAngle-from.HorizonAngle)*t,
			VerticalAngle: from.VerticalAngle + (to.VerticalAngle-from.VerticalAngle)*t,
		}, true
	}
	return cameraActionPose(last, localPosition), true
}

func cameraActionPose(tick CameraActionTick, localPosition [3]float32) CameraActionPose {
	position := [3]float32{tick.X, tick.Y, tick.Z}
	if tick.Local == 1 {
		position[0] += localPosition[0]
		position[1] += localPosition[1]
		position[2] += localPosition[2]
	}
	return CameraActionPose{Position: position, HorizonAngle: tick.HorizonAngle, VerticalAngle: tick.VerticalAngle}
}

func finiteCameraActionTick(tick CameraActionTick) bool {
	return !math.IsNaN(float64(tick.X)) && !math.IsInf(float64(tick.X), 0) &&
		!math.IsNaN(float64(tick.Y)) && !math.IsInf(float64(tick.Y), 0) &&
		!math.IsNaN(float64(tick.Z)) && !math.IsInf(float64(tick.Z), 0) &&
		!math.IsNaN(float64(tick.HorizonAngle)) && !math.IsInf(float64(tick.HorizonAngle), 0) &&
		!math.IsNaN(float64(tick.VerticalAngle)) && !math.IsInf(float64(tick.VerticalAngle), 0)
}

func LoadCameraActionFile(path string) (CameraAction, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return CameraAction{}, err
	}
	return ParseCameraAction(data)
}
