package main

import (
	"encoding/binary"
	"encoding/json"
	"flag"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
)

var hellGateFixturePairs = [][2]string{
	{"Skeleton_", "HG_CorrSkel"}, {"SkeletonArcher", "HG_SkelArch"},
	{"SkeletonMage", "HG_SkelMage"}, {"Guer_Caveira", "Guer_Caveira"},
	{"SpearSkeleton", "HG_SkelSpear"}, {"EsqGuerreiro", "HG_SkelGuard"},
}

func writeHellGateFixture(t *testing.T, source string) {
	t.Helper()
	if err := os.Mkdir(filepath.Join(source, "npc"), 0o700); err != nil {
		t.Fatal(err)
	}
	for _, pair := range hellGateFixturePairs {
		definition := make([]byte, 1024)
		binary.LittleEndian.PutUint32(definition[108:], 100)
		binary.LittleEndian.PutUint32(definition[112:], 50)
		if err := os.WriteFile(filepath.Join(source, "npc", pair[0]), definition, 0o600); err != nil {
			t.Fatal(err)
		}
	}
}

func TestRunHellGateExtractsSixEventInfantry(t *testing.T) {
	source, output := t.TempDir(), t.TempDir()
	writeHellGateFixture(t, source)
	if err := runHellGate(source, output); err != nil {
		t.Fatal(err)
	}
	for _, pair := range hellGateFixturePairs {
		var npc struct {
			Name string `json:"name"`
		}
		body, err := os.ReadFile(filepath.Join(output, pair[1]+".json"))
		if err != nil || json.Unmarshal(body, &npc) != nil || npc.Name != pair[1] {
			t.Fatalf("extracted definition %q=%q err=%v", pair[1], body, err)
		}
	}
	var manifest map[string]string
	body, err := os.ReadFile(filepath.Join(output, "sources.json"))
	if err != nil || json.Unmarshal(body, &manifest) != nil || len(manifest) != len(hellGateFixturePairs) {
		t.Fatalf("manifest=%s err=%v", body, err)
	}
}

func TestRunHellGateMainSelectsExtractorInSubprocess(t *testing.T) {
	source, outputDir := t.TempDir(), t.TempDir()
	writeHellGateFixture(t, source)
	cmd := exec.Command(os.Args[0], "-test.run=^TestRunHellGateMainHelperProcess$")
	cmd.Env = append(os.Environ(), "WYD_RUNETRACK_MAIN_HELPER=hell-gate",
		"WYD_RUNETRACK_MAIN_SOURCE="+source, "WYD_RUNETRACK_MAIN_OUTPUT="+outputDir)
	combined, err := cmd.CombinedOutput()
	if err != nil {
		t.Fatalf("Hell Gate command failed: %v: %s", err, combined)
	}
	var manifest map[string]string
	body, err := os.ReadFile(filepath.Join(outputDir, "sources.json"))
	if err != nil || json.Unmarshal(body, &manifest) != nil || len(manifest) != len(hellGateFixturePairs) {
		t.Fatalf("command did not stage six sources: manifest=%s err=%v output=%s", body, err, combined)
	}
}

func TestRunetrackMainRequiresSourceAndOutput(t *testing.T) {
	cmd := exec.Command(os.Args[0], "-test.run=^TestRunHellGateMainHelperProcess$")
	cmd.Env = append(os.Environ(), "WYD_RUNETRACK_MAIN_HELPER=missing")
	combined, err := cmd.CombinedOutput()
	exitErr, ok := err.(*exec.ExitError)
	if !ok || exitErr.ExitCode() != 2 || !strings.Contains(string(combined), "Both -source and -output are required") {
		t.Fatalf("missing source/output exit=%v output=%s", err, combined)
	}
}

func TestRunHellGateMainHelperProcess(t *testing.T) {
	switch os.Getenv("WYD_RUNETRACK_MAIN_HELPER") {
	case "hell-gate":
		os.Args = []string{"runetrack759", "-source", os.Getenv("WYD_RUNETRACK_MAIN_SOURCE"),
			"-output", os.Getenv("WYD_RUNETRACK_MAIN_OUTPUT"), "-hell-gate"}
	case "missing":
		os.Args = []string{"runetrack759"}
	default:
		return
	}
	flag.CommandLine = flag.NewFlagSet("runetrack759", flag.ExitOnError)
	main()
}

func TestRunHellGateRejectsMissingOrObsoleteSource(t *testing.T) {
	if err := runHellGate(t.TempDir(), t.TempDir()); err == nil {
		t.Fatal("missing infantry input was accepted")
	}
	source, output := t.TempDir(), t.TempDir()
	if err := os.Mkdir(filepath.Join(source, "npc"), 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(source, "npc", "Skeleton_"), []byte("old layout"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := runHellGate(source, output); err == nil {
		t.Fatal("obsolete source layout was accepted")
	}
}
