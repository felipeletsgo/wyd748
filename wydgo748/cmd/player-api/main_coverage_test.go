package main

import (
	"flag"
	"io"
	"os"
	"os/exec"
	"strings"
	"testing"
)

func TestRunRejectsUnsafeListenerBeforeOpeningDependencies(t *testing.T) {
	oldFlags, oldArgs := flag.CommandLine, os.Args
	flags := flag.NewFlagSet("player-api-test", flag.ContinueOnError)
	flags.SetOutput(io.Discard)
	flag.CommandLine = flags
	os.Args = []string{"player-api", "-addr", "0.0.0.0:8083"}
	t.Cleanup(func() { flag.CommandLine, os.Args = oldFlags, oldArgs })
	if err := run(); err == nil {
		t.Fatal("non-loopback listener accepted")
	}
}

func TestPlayerAPIMainReportsFailureAndExits(t *testing.T) {
	cmd := exec.Command(os.Args[0], "-test.run=^TestPlayerAPIMainHelperProcess$")
	cmd.Env = append(os.Environ(), "WYD_PLAYER_API_MAIN_HELPER=1")
	output, err := cmd.CombinedOutput()
	if err == nil || !strings.Contains(string(output), "expected literal loopback") {
		t.Fatalf("player-api main did not report startup failure: err=%v output=%s", err, output)
	}
}

func TestPlayerAPIMainHelperProcess(t *testing.T) {
	if os.Getenv("WYD_PLAYER_API_MAIN_HELPER") != "1" {
		return
	}
	flag.CommandLine = flag.NewFlagSet("player-api", flag.ExitOnError)
	os.Args = []string{"player-api", "-addr", "0.0.0.0:8083"}
	main()
}
