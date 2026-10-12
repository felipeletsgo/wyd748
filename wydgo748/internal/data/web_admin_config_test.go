package data

import "testing"

func TestAdminAccessPINExactlySixASCIIDigits(t *testing.T) {
	for _, pin := range []string{"000000", "001234", "999999"} {
		if err := ValidateAdminAccessPIN(pin); err != nil {
			t.Fatalf("six-digit PIN rejected: %v", err)
		}
	}
	for _, pin := range []string{"", "12345", "1234567", "00123456", "123456789012", "123 45", "12a456", "１２３４５６"} {
		if err := ValidateAdminAccessPIN(pin); err == nil {
			t.Fatal("invalid PIN accepted")
		}
		if _, err := LoadServerConfig(writeServerConfig(t, "admin_access_pin="+pin+"\n")); err == nil {
			t.Fatal("invalid configured PIN accepted")
		}
	}
}

func TestLoadServerConfigParsesPublishedWebAdmin(t *testing.T) {
	cfg, err := LoadServerConfig(writeServerConfig(t, "web_admin_enabled=true\n"+
		"web_admin_origin=https://wyd.example.com\n"+
		"web_admin_trusted_proxies=127.0.0.1/32, ::1/128\n"+
		"admin_access_pin_env=WYD_ADMIN_PIN\n"))
	if err != nil {
		t.Fatal(err)
	}
	if cfg.WebAdminOrigin != "https://wyd.example.com" || cfg.AdminAccessPINEnv != "WYD_ADMIN_PIN" {
		t.Fatalf("origin=%q pin env=%q", cfg.WebAdminOrigin, cfg.AdminAccessPINEnv)
	}
	proxies, err := ParseTrustedProxies(cfg.WebAdminTrustedProxies)
	if err != nil || len(proxies) != 2 || proxies[0].String() != "127.0.0.1/32" {
		t.Fatalf("trusted proxies=%v err=%v", proxies, err)
	}
}

func TestLoadServerConfigRejectsUnsafeWebAdminOrigin(t *testing.T) {
	for _, origin := range []string{
		"http://wyd.example.com", "https://wyd.example.com/", "https://wyd.example.com/admin",
		"https://user@wyd.example.com", "https://wyd.example.com?x=1", "https://", "wyd.example.com",
	} {
		if _, err := LoadServerConfig(writeServerConfig(t, "web_admin_origin="+origin+"\n")); err == nil {
			t.Fatalf("unsafe origin %q was accepted", origin)
		}
	}
}

func TestLoadServerConfigRejectsInvalidTrustedProxies(t *testing.T) {
	for _, value := range []string{"127.0.0.1", "not-a-cidr", "127.0.0.1/32,,"} {
		if _, err := LoadServerConfig(writeServerConfig(t, "web_admin_trusted_proxies="+value+"\n")); err == nil {
			t.Fatalf("invalid trusted proxies %q were accepted", value)
		}
	}
}
