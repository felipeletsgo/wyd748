package main

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"testing"
)

func writeColosseumSource(t *testing.T, source string, names []string) {
	t.Helper()
	if err := os.MkdirAll(filepath.Join(source, "npc"), 0700); err != nil {
		t.Fatal(err)
	}
	var input strings.Builder
	nameIndex := 0
	for _, file := range colosseumFiles {
		for _, label := range file.labels {
			leader := names[nameIndex%len(names)]
			follower := names[(nameIndex+1)%len(names)]
			nameIndex += 2
			fmt.Fprintf(&input, "# [%d] event generator\nLeader: %s\nFollower: %s\nMinuteGenerate: -1\nMaxNumMob: 4\nUnknown: ignored\nStartX: %d\n", label, leader, follower, label)
		}
	}
	if err := os.WriteFile(filepath.Join(source, "NPCGener.txt"), []byte(input.String()), 0600); err != nil {
		t.Fatal(err)
	}
	for _, name := range names {
		if err := os.WriteFile(filepath.Join(source, "npc", name), make([]byte, 1024), 0600); err != nil {
			t.Fatal(err)
		}
	}
}

func TestRunColosseumExtractsGeneratorsTemplatesAndSourceHashes(t *testing.T) {
	source, output := t.TempDir(), t.TempDir()
	names := make([]string, 0, len(colosseumAliases))
	for name := range colosseumAliases {
		if len(colosseumAliases[name]) <= 12 {
			names = append(names, name)
		}
	}
	sort.Strings(names)
	writeColosseumSource(t, source, names)

	if err := runColosseum(source, output); err != nil {
		t.Fatal(err)
	}
	for _, file := range colosseumFiles {
		data, err := os.ReadFile(filepath.Join(output, file.name))
		if err != nil {
			t.Fatal(err)
		}
		text := string(data)
		for _, label := range file.labels {
			for _, expected := range []string{fmt.Sprintf("#\t[%d]", label), "MinuteGenerate:\t-1", "MaxNumMob:\t4", fmt.Sprintf("StartX:\t%d", label)} {
				if !strings.Contains(text, expected) {
					t.Fatalf("%s does not contain %q", file.name, expected)
				}
			}
		}
		if strings.Contains(text, "Unknown") || !strings.Contains(text, "Event-only:") {
			t.Fatalf("%s did not filter fields or include its event-only note", file.name)
		}
	}

	manifestBytes, err := os.ReadFile(filepath.Join(output, "colosseum-sources.json"))
	if err != nil {
		t.Fatal(err)
	}
	manifest := map[string]string{}
	if err := json.Unmarshal(manifestBytes, &manifest); err != nil {
		t.Fatal(err)
	}
	if len(manifest) != len(names) {
		t.Fatalf("manifest has %d sources, want %d", len(manifest), len(names))
	}
	for _, name := range names {
		original, err := os.ReadFile(filepath.Join(source, "npc", name))
		if err != nil {
			t.Fatal(err)
		}
		hash := sha256.Sum256(original)
		if manifest[name] != hex.EncodeToString(hash[:]) {
			t.Fatalf("manifest hash for %q = %q", name, manifest[name])
		}
	}
}

func TestRunColosseumReportsInputAndOutputFailures(t *testing.T) {
	t.Run("missing input", func(t *testing.T) {
		if err := runColosseum(t.TempDir(), t.TempDir()); err == nil {
			t.Fatal("accepted a missing NPCGener.txt")
		}
	})
	t.Run("overflowing header", func(t *testing.T) {
		source := t.TempDir()
		input := "#[999999999999999999999999999999999999999999]\n"
		if err := os.WriteFile(filepath.Join(source, "NPCGener.txt"), []byte(input), 0600); err != nil {
			t.Fatal(err)
		}
		if err := runColosseum(source, t.TempDir()); err == nil || !strings.Contains(err.Error(), "invalid generator ID") {
			t.Fatalf("expected invalid header error, got %v", err)
		}
	})
	t.Run("output path is a file", func(t *testing.T) {
		source, output := t.TempDir(), filepath.Join(t.TempDir(), "output")
		if err := os.WriteFile(output, []byte("occupied"), 0600); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(filepath.Join(source, "NPCGener.txt"), []byte(""), 0600); err != nil {
			t.Fatal(err)
		}
		if err := runColosseum(source, output); err == nil {
			t.Fatal("accepted an output path occupied by a file")
		}
	})
	t.Run("missing generator", func(t *testing.T) {
		source := t.TempDir()
		if err := os.WriteFile(filepath.Join(source, "NPCGener.txt"), []byte("#[5]\nLeader: ForceCyclops\n"), 0600); err != nil {
			t.Fatal(err)
		}
		if err := runColosseum(source, t.TempDir()); err == nil || !strings.Contains(err.Error(), "generator 6 is missing") {
			t.Fatalf("expected missing generator error, got %v", err)
		}
	})
	t.Run("missing alias", func(t *testing.T) {
		source := t.TempDir()
		if err := os.WriteFile(filepath.Join(source, "NPCGener.txt"), []byte("#[5]\nLeader: UnknownMob\n"), 0600); err != nil {
			t.Fatal(err)
		}
		if err := runColosseum(source, t.TempDir()); err == nil || !strings.Contains(err.Error(), `missing alias for "UnknownMob"`) {
			t.Fatalf("expected missing alias error, got %v", err)
		}
	})
	t.Run("generator write failure", func(t *testing.T) {
		source, output := t.TempDir(), t.TempDir()
		writeColosseumSource(t, source, []string{"ForceCyclops"})
		if err := os.Mkdir(filepath.Join(output, colosseumFiles[0].name), 0700); err != nil {
			t.Fatal(err)
		}
		if err := runColosseum(source, output); err == nil {
			t.Fatal("accepted a generator output path occupied by a directory")
		}
	})
	t.Run("missing source NPC", func(t *testing.T) {
		source, output := t.TempDir(), t.TempDir()
		if err := os.WriteFile(filepath.Join(source, "NPCGener.txt"), []byte(colosseumFixture("ForceCyclops")), 0600); err != nil {
			t.Fatal(err)
		}
		if err := runColosseum(source, output); err == nil || !strings.Contains(err.Error(), "ForceCyclops") {
			t.Fatalf("expected missing source NPC error, got %v", err)
		}
	})
	t.Run("invalid source NPC layout", func(t *testing.T) {
		source, output := t.TempDir(), t.TempDir()
		writeColosseumSource(t, source, []string{"ForceCyclops"})
		if err := os.WriteFile(filepath.Join(source, "npc", "ForceCyclops"), make([]byte, 32), 0600); err != nil {
			t.Fatal(err)
		}
		if err := runColosseum(source, output); err == nil || !strings.Contains(err.Error(), "expected the 1024-byte") {
			t.Fatalf("expected invalid layout error, got %v", err)
		}
	})
	t.Run("wire name exceeds supported limit", func(t *testing.T) {
		source, output := t.TempDir(), t.TempDir()
		writeColosseumSource(t, source, []string{"Ciclope_Arq__"})
		if err := runColosseum(source, output); err == nil || !strings.Contains(err.Error(), "wire name budget") {
			t.Fatalf("expected wire name budget error, got %v", err)
		}
	})
	t.Run("manifest write failure", func(t *testing.T) {
		source, output := t.TempDir(), t.TempDir()
		writeColosseumSource(t, source, []string{"ForceCyclops"})
		if err := os.Mkdir(filepath.Join(output, "colosseum-sources.json"), 0700); err != nil {
			t.Fatal(err)
		}
		if err := runColosseum(source, output); err == nil {
			t.Fatal("accepted a manifest output path occupied by a directory")
		}
	})
}

func colosseumFixture(name string) string {
	var input strings.Builder
	for _, file := range colosseumFiles {
		for _, label := range file.labels {
			fmt.Fprintf(&input, "#[%d]\nLeader: %s\nFollower: %s\n", label, name, name)
		}
	}
	return input.String()
}
