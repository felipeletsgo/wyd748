package main

import (
	"flag"
	"net/netip"
	"os"
	"os/exec"
	"strings"
	"testing"
)

func TestAccountAPIMainRejectsMissingDatabaseAndInvalidURL(t *testing.T) {
	for _, test := range []struct {
		name, databaseURL, want string
	}{
		{name: "missing database URL", want: "variable WYD_TEST_ACCOUNT_API_DATABASE_URL is empty"},
		{name: "invalid database URL", databaseURL: "invalid://database", want: "open PostgreSQL"},
	} {
		t.Run(test.name, func(t *testing.T) {
			cmd := exec.Command(os.Args[0], "-test.run=^TestAccountAPIHelperProcess$")
			cmd.Env = append(os.Environ(), "WYD_ACCOUNT_API_HELPER=1",
				"WYD_TEST_ACCOUNT_API_DATABASE_URL="+test.databaseURL)
			output, err := cmd.CombinedOutput()
			if err == nil || !strings.Contains(string(output), test.want) {
				t.Fatalf("account-api accepted %s: err=%v output=%s", test.name, err, output)
			}
		})
	}
}

func TestAccountAPIHelperProcess(t *testing.T) {
	if os.Getenv("WYD_ACCOUNT_API_HELPER") != "1" {
		return
	}
	flag.CommandLine = flag.NewFlagSet("account-api", flag.ExitOnError)
	os.Args = []string{"account-api", "-database-url-env", "WYD_TEST_ACCOUNT_API_DATABASE_URL"}
	main()
}

func TestParseTrustedProxiesSkipsEmptyEntriesAndRejectsInvalidCIDRs(t *testing.T) {
	prefixes, err := parseTrustedProxies(" 127.0.0.0/8, , ::1/128 ")
	if err != nil || len(prefixes) != 2 || prefixes[0] != netip.MustParsePrefix("127.0.0.0/8") ||
		prefixes[1] != netip.MustParsePrefix("::1/128") {
		t.Fatalf("parsed trusted proxies = %v, err=%v", prefixes, err)
	}
	empty, err := parseTrustedProxies(" , ")
	if err != nil || len(empty) != 0 {
		t.Fatalf("empty proxy list = %v, err=%v", empty, err)
	}
	if _, err := parseTrustedProxies("192.0.2.1"); err == nil || !strings.Contains(err.Error(), "trusted proxy") {
		t.Fatalf("invalid proxy CIDR error = %v", err)
	}
}
