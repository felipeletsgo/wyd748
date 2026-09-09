package loginflow

import "testing"

func TestLoginLayoutMatchesTMProjectViewportComposition(t *testing.T) {
	for _, test := range []struct {
		width, height       int32
		panelX, panelY      int32
		logoY               int32
	}{
		{800, 600, 272, 172, 10},
		{1024, 768, 384, 256, 32},
		{1280, 960, 512, 352, 56},
		{1600, 900, 672, 322, 15},
	} {
		layout := loginLayoutFor(viewportProbe{width: test.width, height: test.height})
		if layout.panel.X != test.panelX || layout.panel.Y != test.panelY {
			t.Fatalf("viewport %dx%d panel=%+v, want (%d,%d)", test.width, test.height, layout.panel, test.panelX, test.panelY)
		}
		if layout.logoLeft.X != test.width/2-loginLogoWidth || layout.logoRight.X != test.width/2 {
			t.Fatalf("viewport %dx%d logos=(%+v,%+v) not centered on viewport", test.width, test.height, layout.logoLeft, layout.logoRight)
		}
		if layout.logoLeft.Y != test.logoY || layout.logoRight.Y != test.logoY {
			t.Fatalf("viewport %dx%d logoY=(%d,%d), want %d", test.width, test.height, layout.logoLeft.Y, layout.logoRight.Y, test.logoY)
		}
		if layout.account.X != layout.panel.X+72 || layout.password.X != layout.panel.X+72 || layout.submit.X != layout.panel.X+68 {
			t.Fatalf("viewport %dx%d controls lost panel anchoring: panel=%+v account=%+v password=%+v submit=%+v", test.width, test.height, layout.panel, layout.account, layout.password, layout.submit)
		}
	}
}
