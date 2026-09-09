package config

import (
	"encoding/hex"
	"strings"
	"testing"
)

func TestLoadProtectedAssetSettingsFromEnv(t *testing.T) {
	t.Setenv("WYD_ASSET_PACKAGE", "assets/ui.wydpkg")
	t.Setenv("WYD_ASSET_CONTENT_KEY_HEX", strings.Repeat("11", 32))
	t.Setenv("WYD_ASSET_PUBLIC_KEY_HEX", strings.Repeat("22", 32))
	t.Setenv("WYD_ASSET_LOGO_PATH", "UI/logo1.wydasset")
	settings, enabled, err := LoadProtectedAssetSettingsFromEnv()
	if err != nil || !enabled {
		t.Fatalf("LoadProtectedAssetSettingsFromEnv() = %#v, %v, %v", settings, enabled, err)
	}
	if settings.PackagePath != "assets/ui.wydpkg" || settings.LogoPath != "UI/logo1.wydasset" {
		t.Fatalf("settings = %#v", settings)
	}
	if got := hex.EncodeToString(settings.ContentKey[:]); got != strings.Repeat("11", 32) {
		t.Fatalf("content key = %s", got)
	}
}

func TestLoadProtectedAssetSettingsRejectsPartialConfiguration(t *testing.T) {
	t.Setenv("WYD_ASSET_PACKAGE", "assets/ui.wydpkg")
	t.Setenv("WYD_ASSET_CONTENT_KEY_HEX", "")
	t.Setenv("WYD_ASSET_PUBLIC_KEY_HEX", "")
	if _, enabled, err := LoadProtectedAssetSettingsFromEnv(); err == nil || enabled {
		t.Fatalf("partial settings = enabled %v, error %v", enabled, err)
	}
}
