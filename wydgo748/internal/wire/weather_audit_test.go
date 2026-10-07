package wire

import (
	"bytes"
	"encoding/binary"
	"testing"
)

func TestEnterWorldWeatherWritesOnlyTheContractField(t *testing.T) {
	for size := 0; size <= 1250; size++ {
		frame := bytes.Repeat([]byte{0xA5}, size)
		want := bytes.Clone(frame)
		if size >= 1246 {
			binary.LittleEndian.PutUint16(want[1244:1246], 3)
		}
		PutEnterWorldWeather(frame, 3)
		if !bytes.Equal(frame, want) {
			t.Fatalf("size %d: weather changed bytes outside [1244,1246)", size)
		}
	}
	PutEnterWorldWeather(nil, 3)
}

func TestWeatherFrameUsesTheSameValueAsLogin(t *testing.T) {
	for weather := uint32(0); weather <= 3; weather++ {
		frame := Weather(0x1234, weather)
		// Builders leave Size/Tick unset; Session.Send finishes the header.
		if len(frame) != 16 || binary.LittleEndian.Uint16(frame[:2]) != 0 || binary.LittleEndian.Uint16(frame[4:6]) != OpWeather || binary.LittleEndian.Uint16(frame[6:8]) != 0x1234 || binary.LittleEndian.Uint32(frame[12:16]) != weather {
			t.Fatalf("weather %d: invalid 0x18B standard parameter layout", weather)
		}
		login := make([]byte, 1250)
		PutEnterWorldWeather(login, uint16(weather))
		if uint32(binary.LittleEndian.Uint16(login[1244:1246])) != binary.LittleEndian.Uint32(frame[12:16]) {
			t.Fatal("login and periodic weather disagree")
		}
		other := Weather(0x1234, weather)
		frame[12] = 0xFF
		if binary.LittleEndian.Uint32(other[12:16]) != weather {
			t.Fatal("weather frames share mutable storage")
		}
	}
}
