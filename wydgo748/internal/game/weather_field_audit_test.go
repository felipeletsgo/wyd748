package game

import "testing"

func TestWeatherDamageContractAcrossAllMapFields(t *testing.T) {
	for weather := 0; weather <= 3; weather++ {
		for x := 0; x < 32; x++ {
			for y := 0; y < 32; y++ {
				snow := x >= 27 && x <= 30 && y >= 21 && y <= 24
				want := weather
				if snow || weather == 3 {
					want = 2
				}
				for _, offset := range [][2]int{{0, 0}, {127, 0}, {0, 127}, {127, 127}} {
					px, py := uint16(x*128+offset[0]), uint16(y*128+offset[1])
					if got := skillWeatherAt(weather, px, py); got != want {
						t.Fatalf("weather %d at (%d,%d): got %d, want %d", weather, px, py, got, want)
					}
				}
			}
		}
	}
}
