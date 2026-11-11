package main

import (
	"flag"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
)

func TestLoadtestMainRejectsInvalidStartupArguments(t *testing.T) {
	missingMap := filepath.Join(t.TempDir(), "missing-map.dat")
	for _, test := range []struct {
		name, want string
		args       []string
	}{
		{name: "bot count below minimum", args: []string{"-bots=0"}, want: "bots must be between 1 and 990"},
		{name: "bot count above maximum", args: []string{"-bots=991"}, want: "bots must be between 1 and 990"},
		{name: "zero ramp", args: []string{"-ramp=0s"}, want: "ramp and duration must be positive"},
		{name: "zero duration", args: []string{"-duration=0s"}, want: "ramp and duration must be positive"},
		{name: "source slot below sentinel", args: []string{"-source-slot=-2"}, want: "source-slot must be -1 or a nonnegative slot"},
		{name: "missing terrain", args: []string{"-heightmap=" + missingMap, "-attributemap=" + missingMap}, want: "load terrain maps for load test movement"},
	} {
		t.Run(test.name, func(t *testing.T) {
			cmd := exec.Command(os.Args[0], "-test.run=^TestLoadtestMainHelperProcess$")
			cmd.Env = append(os.Environ(), "WYD_LOADTEST_HELPER=1",
				"WYD_LOADTEST_ARGS="+strings.Join(test.args, "\x1f"))
			output, err := cmd.CombinedOutput()
			if err == nil || !strings.Contains(string(output), test.want) {
				t.Fatalf("loadtest accepted %s: err=%v output=%s", test.name, err, output)
			}
		})
	}
}

func TestLoadtestMainValidatesCredentialsAndDatabaseAfterTerrain(t *testing.T) {
	mapDir, err := filepath.Abs(filepath.Join("..", "..", "data", "world", "maps"))
	if err != nil {
		t.Fatal(err)
	}
	passwordEnv := "WYD_TEST_LOADTEST_PASSWORD_COVERAGE"
	databaseEnv := "WYD_TEST_LOADTEST_DATABASE_COVERAGE"
	for _, test := range []struct {
		name, password, database, want string
	}{
		{name: "missing password", want: "environment variable " + passwordEnv + " is empty"},
		{name: "invalid password", password: "abc", want: "must contain 4 to 10 visible ASCII characters"},
		{name: "missing database", password: "abcd", want: "environment variable " + databaseEnv + " is empty"},
		{name: "invalid database URL", password: "abcd", database: "invalid://database", want: "open load-test PostgreSQL database"},
	} {
		t.Run(test.name, func(t *testing.T) {
			args := []string{"-heightmap=" + filepath.Join(mapDir, "HeightMap.dat"),
				"-attributemap=" + filepath.Join(mapDir, "AttributeMap.dat"),
				"-bot-password-env=" + passwordEnv, "-database-url-env=" + databaseEnv}
			cmd := exec.Command(os.Args[0], "-test.run=^TestLoadtestMainHelperProcess$")
			cmd.Env = append(os.Environ(), "WYD_LOADTEST_HELPER=1",
				"WYD_LOADTEST_ARGS="+strings.Join(args, "\x1f"),
				passwordEnv+"="+test.password, databaseEnv+"="+test.database)
			output, err := cmd.CombinedOutput()
			if err == nil || !strings.Contains(string(output), test.want) {
				t.Fatalf("loadtest did not reject %s as expected: err=%v output=%s", test.name, err, output)
			}
		})
	}
}

func TestLoadtestMainHelperProcess(t *testing.T) {
	if os.Getenv("WYD_LOADTEST_HELPER") != "1" {
		return
	}
	flag.CommandLine = flag.NewFlagSet("loadtest", flag.ExitOnError)
	args := strings.Split(os.Getenv("WYD_LOADTEST_ARGS"), "\x1f")
	os.Args = append([]string{"loadtest"}, args...)
	main()
}
