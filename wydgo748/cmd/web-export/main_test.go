package main

import (
	"flag"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
)

func TestWebExportMainValidatesArgumentsAndWritesCatalog(t *testing.T) {
	repoData, err := filepath.Abs(filepath.Join("..", "..", "data"))
	if err != nil {
		t.Fatal(err)
	}
	outputDir := t.TempDir()
	missingData := filepath.Join(outputDir, "missing-data")
	missingOutput := filepath.Join(outputDir, "missing", "catalog.json")
	positionalOutput := filepath.Join(outputDir, "positional.json")
	failedOutput := filepath.Join(outputDir, "failed.json")
	successOutput := filepath.Join(outputDir, "catalog.json")

	for _, test := range []struct {
		name, want string
		args       []string
		exitCode   int
	}{
		{name: "output path required", args: []string{"-data-dir", repoData}, want: "requires -output", exitCode: 2},
		{name: "positional arguments rejected", args: []string{"-output", positionalOutput, "extra"}, want: "requires -output", exitCode: 2},
		{name: "catalog errors are reported", args: []string{"-data-dir", missingData, "-output", failedOutput}, want: "Catalog export failed:", exitCode: 1},
		{name: "catalog is written", args: []string{"-data-dir", repoData, "-output", successOutput}, want: "Public catalog exported:", exitCode: 0},
	} {
		t.Run(test.name, func(t *testing.T) {
			cmd := exec.Command(os.Args[0], "-test.run=^TestWebExportMainHelperProcess$")
			cmd.Env = append(os.Environ(), "WYD_WEB_EXPORT_HELPER=1",
				"WYD_WEB_EXPORT_ARGS="+strings.Join(test.args, "\x1f"))
			output, runErr := cmd.CombinedOutput()
			if test.exitCode == 0 {
				if runErr != nil || !strings.Contains(string(output), test.want) {
					t.Fatalf("catalog export failed: err=%v output=%s", runErr, output)
				}
				if info, err := os.Stat(successOutput); err != nil || info.Size() == 0 {
					t.Fatalf("catalog artifact missing or empty: info=%v err=%v", info, err)
				}
				return
			}
			exitErr, ok := runErr.(*exec.ExitError)
			if !ok || exitErr.ExitCode() != test.exitCode || !strings.Contains(string(output), test.want) {
				t.Fatalf("web-export exit=%v output=%s, want code %d with %q", runErr, output, test.exitCode, test.want)
			}
		})
	}
	if _, err := os.Stat(missingOutput); !os.IsNotExist(err) {
		t.Fatalf("argument rejection created an output file: %v", err)
	}
}

func TestWebExportMainHelperProcess(t *testing.T) {
	if os.Getenv("WYD_WEB_EXPORT_HELPER") != "1" {
		return
	}
	flag.CommandLine = flag.NewFlagSet("web-export", flag.ExitOnError)
	args := strings.Split(os.Getenv("WYD_WEB_EXPORT_ARGS"), "\x1f")
	os.Args = append([]string{"web-export"}, args...)
	main()
}
