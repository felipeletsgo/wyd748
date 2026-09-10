package loginflow

import (
	"testing"
	"wydclient748/internal/assets"
)

func TestLoginLayoutMatchesTMProjectViewportComposition(t *testing.T) {
	for _, test := range []struct {
		width, height       int32
		panelX, panelY      int32
		logoY               int32
	}{
		{800, 600, 292, 223, 10},
		{1024, 768, 404, 307, 32},
		{1280, 960, 532, 403, 56},
		{1600, 900, 692, 373, 15},
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

func TestLoginLayoutUsesNativeHitboxes(t *testing.T) {
	l := loginLayoutFor(viewportProbe{width: 800, height: 600})
	controls := []assets.SceneControl{{Kind: 1, Words: []int32{4608, 0, 0, 0, 0, 210, 153}}, {Kind: 13, Words: []int32{5121, 4608, 0, 82, 34, 100, 13}}, {Kind: 13, Words: []int32{5122, 4608, 0, 82, 57, 100, 13}}, {Kind: 2, Words: []int32{4609, 4608, 0, 22, 86, 70, 20}}}
	l.applyNativeControls(controls)
	if l.account.X != l.panel.X+82 || l.password.Y != l.panel.Y+57 || l.submit.Width != 70 { t.Fatalf("native hitboxes not applied: %+v", l) }
}
