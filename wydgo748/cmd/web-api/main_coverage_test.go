package main

import (
	"flag"
	"io"
	"net/netip"
	"os"
	"os/exec"
	"strings"
	"testing"
)

func setCommandArgs(t *testing.T, args ...string) {
	t.Helper()
	oldFlags, oldArgs := flag.CommandLine, os.Args
	flags := flag.NewFlagSet("web-api-test", flag.ContinueOnError)
	flags.SetOutput(io.Discard)
	flag.CommandLine = flags
	os.Args = append([]string{"web-api"}, args...)
	t.Cleanup(func() { flag.CommandLine, os.Args = oldFlags, oldArgs })
}

func TestParseTrustedProxiesMasksAndRejectsCIDRs(t *testing.T) {
	if prefixes, err := parseTrustedProxies(" 192.0.2.7/24, 2001:db8::7/64 "); err != nil || len(prefixes) != 2 ||
		prefixes[0] != netip.MustParsePrefix("192.0.2.0/24") || prefixes[1] != netip.MustParsePrefix("2001:db8::/64") {
		t.Fatalf("prefixes=%v err=%v", prefixes, err)
	}
	if prefixes, err := parseTrustedProxies("  "); err != nil || prefixes != nil {
		t.Fatalf("empty proxies=%v err=%v", prefixes, err)
	}
	if _, err := parseTrustedProxies("192.0.2.1"); err == nil {
		t.Fatal("address without CIDR mask accepted")
	}
}

func TestRunRejectsUnsafeListenerAndMissingRequiredOptions(t *testing.T) {
	t.Run("unsafe listener", func(t *testing.T) {
		setCommandArgs(t, "-addr", "0.0.0.0:8082")
		if err := run(); err == nil {
			t.Fatal("non-loopback listener accepted")
		}
	})
	t.Run("missing required options", func(t *testing.T) {
		setCommandArgs(t, "-addr", "127.0.0.1:8082")
		if err := run(); err == nil {
			t.Fatal("missing origin and staff file accepted")
		}
	})
}

func TestWebAPIMainReportsFailureAndExits(t *testing.T) {
	cmd := exec.Command(os.Args[0], "-test.run=^TestWebAPIMainHelperProcess$")
	cmd.Env = append(os.Environ(), "WYD_WEB_API_MAIN_HELPER=1")
	output, err := cmd.CombinedOutput()
	if err == nil || !strings.Contains(string(output), "expected literal loopback") {
		t.Fatalf("web-api main did not report startup failure: err=%v output=%s", err, output)
	}
}

func TestWebAPIMainHelperProcess(t *testing.T) {
	if os.Getenv("WYD_WEB_API_MAIN_HELPER") != "1" {
		return
	}
	flag.CommandLine = flag.NewFlagSet("web-api", flag.ExitOnError)
	os.Args = []string{"web-api", "-addr", "0.0.0.0:8082"}
	main()
}
