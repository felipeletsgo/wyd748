package game

import "testing"

func TestNative748CelestialExperienceDivisorCoversEveryLevelBand(t *testing.T) {
	tests := []struct {
		name  string
		level uint32
		want  uint32
	}{
		{name: "below 40", level: 40, want: 4000},
		{name: "41 through 80", level: 41, want: 5000},
		{name: "81 through 100", level: 81, want: 5500},
		{name: "101 through 150", level: 101, want: 6000},
		{name: "151 through 190", level: 151, want: 6500},
		{name: "191 through 210", level: 191, want: 7000},
		{name: "211 through 230", level: 211, want: 7500},
		{name: "231 through 255", level: 231, want: 8000},
		{name: "256 through 280", level: 256, want: 8500},
		{name: "281 through 300", level: 281, want: 8800},
		{name: "301 through 320", level: 301, want: 8900},
		{name: "321 through 340", level: 321, want: 9000},
		{name: "341 through 350", level: 341, want: 9100},
		{name: "351 through 355", level: 351, want: 9300},
		{name: "356 through 360", level: 356, want: 9500},
		{name: "361 through 365", level: 361, want: 10000},
		{name: "366 through 370", level: 366, want: 11000},
		{name: "371 through 375", level: 371, want: 12000},
		{name: "376 through 380", level: 376, want: 13000},
		{name: "381 through 385", level: 381, want: 14000},
		{name: "386 through 390", level: 386, want: 15000},
		{name: "391 through 395", level: 391, want: 16000},
		{name: "396 through 398", level: 396, want: 17000},
		{name: "above 398", level: 399, want: 18000},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			if got := native748CelestialEXPDivisor(tc.level); got != tc.want {
				t.Fatalf("native748CelestialEXPDivisor(%d) = %d, want %d", tc.level, got, tc.want)
			}
		})
	}
}
