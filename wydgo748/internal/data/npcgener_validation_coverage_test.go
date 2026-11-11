package data

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestNPCGenerRejectsMissingLeadersAndInvalidStrictFields(t *testing.T) {
	for _, tc := range []struct {
		name string
		body string
		want string
	}{
		{name: "missing leader", body: "#[1]\n", want: "no valid Leader"},
		{name: "zero leader", body: "#[1]\nLeader: 0\n", want: "no valid Leader"},
		{name: "zero capacity", body: "#[1]\nLeader: Test\nMaxNumMob: 0\n", want: "MaxNumMob must be positive"},
		{name: "negative coordinate", body: "#[1]\nLeader: Test\nStartX: -1\n", want: "outside supported coordinates"},
		{name: "zero coordinate", body: "#[1]\nLeader: Test\nStartY: 0\n", want: "outside supported coordinates"},
		{name: "coordinate above uint16", body: "#[1]\nLeader: Test\nStartX: 65536\n", want: "outside supported coordinates"},
		{name: "range above uint16", body: "#[1]\nLeader: Test\nStartRange: 65536\n", want: "invalid value"},
		{name: "invalid recognized field", body: "#[1]\nLeader: Test\nSegment1Wait: bad\n", want: "expects an integer"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			path := writeNPCGenerCoverageFile(t, tc.body)
			if _, err := LoadNPCGener(path); err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("LoadNPCGener error = %v, want substring %q", err, tc.want)
			}
		})
	}
	if _, err := LoadNPCGener(filepath.Join(t.TempDir(), "missing.txt")); err == nil {
		t.Fatal("LoadNPCGener accepted a missing file")
	}
}

func TestChannelNPCGenerDisablesOutOfBoundsAndEmptyGroups(t *testing.T) {
	path := writeNPCGenerCoverageFile(t, "#[1]\nLeader: Test\nStartX: 4096\nStartY: 100\n#[2]\nLeader: Empty\nMaxNumMob: 0\nStartX: 100\nStartY: 100\n")
	groups, err := LoadChannelNPCGener(path)
	if err != nil {
		t.Fatal(err)
	}
	if len(groups) != 2 || groups[0].Enabled || groups[0].Segments[0].X != 4096 || groups[0].Label != 1 {
		t.Fatalf("out-of-bounds route was not preserved as disabled: %+v", groups)
	}
	if groups[1].Enabled || groups[1].MaxNumMob != 0 {
		t.Fatalf("empty group was not disabled: %+v", groups[1])
	}
}

func TestNPCGeneratorLabelsAndTextBeforeFirstSection(t *testing.T) {
	for _, tc := range []struct {
		line string
		want int
	}{
		{line: "# [ 42 ]", want: 42},
		{line: "# without a number", want: 0},
		{line: "# [bad]", want: 0},
		{line: "# [42", want: 0},
	} {
		if got := generatorLabel(tc.line); got != tc.want {
			t.Errorf("generatorLabel(%q) = %d, want %d", tc.line, got, tc.want)
		}
	}
	path := writeNPCGenerCoverageFile(t, "text before any section\nOne: two\n// comment\n\n")
	groups, err := LoadNPCGener(path)
	if err != nil || len(groups) != 0 {
		t.Fatalf("pre-section text parsed as generators %v, err=%v", groups, err)
	}
}

func writeNPCGenerCoverageFile(t *testing.T, body string) string {
	t.Helper()
	path := filepath.Join(t.TempDir(), "NPCGener.txt")
	if err := os.WriteFile(path, []byte(body), 0o600); err != nil {
		t.Fatal(err)
	}
	return path
}
