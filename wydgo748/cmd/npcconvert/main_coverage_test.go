package main

import (
	"encoding/binary"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestConvertClearsCorruptExperienceAndEquipment(t *testing.T) {
	b := make([]byte, mobFileSize)
	putU16(b, offStatus+8, 100)
	putU16(b, offEquip+3*8, model.ItemListSize)
	binary.LittleEndian.PutUint32(b[28:32], maxSaneNPCExp+1)

	got, report := convert("Corrupt", b)
	if got.Equip.Calca != (model.Item{}) || got.ExpReward != 0 ||
		report.invalidItems != 1 || report.discardedExp != 1 {
		t.Fatalf("converted=%+v report=%+v", got, report)
	}
}

func TestDecodeNPCSelectsLegacyAnd759MonsterOrSkillShop(t *testing.T) {
	t.Run("legacy exact layout", func(t *testing.T) {
		b := make([]byte, mobFileSize)
		putU16(b, offStatus+8, 100)
		got, _, err := decodeNPC("Legacy", b)
		if err != nil || got.Name != "Legacy" || got.Tipo != model.TipoMonstro {
			t.Fatalf("got=%+v err=%v", got, err)
		}
	})
	t.Run("759 monster", func(t *testing.T) {
		b := make([]byte, 1024)
		binary.LittleEndian.PutUint32(b[108:], 100)
		putU16(b, 268, 685)
		got, _, err := decodeNPC("Monster759", b)
		if err != nil || got.Tipo != model.TipoMonstro || len(got.Carry) != 1 || got.Carry[0].Index != 685 {
			t.Fatalf("got=%+v err=%v", got, err)
		}
	})
	t.Run("759 skill master", func(t *testing.T) {
		b := make([]byte, 1024)
		b[17] = 19
		binary.LittleEndian.PutUint32(b[108:], 100)
		putU16(b, 268, 5000)
		got, _, err := decodeNPC("Master759", b)
		if err != nil || got.Tipo != model.TipoNPC || len(got.Vende) != 27 || got.Vende[0].Index != 5000 {
			t.Fatalf("got=%+v err=%v", got, err)
		}
	})
	t.Run("invalid score", func(t *testing.T) {
		b := make([]byte, 1024)
		binary.LittleEndian.PutUint32(b[100:], ^uint32(0))
		if _, _, err := decodeNPC("InvalidScore", b); err == nil || !strings.Contains(err.Error(), "score.attack") {
			t.Fatalf("err=%v", err)
		}
	})
}

func TestSafeFileNameReplacesInvalidCharactersAndEmptyNames(t *testing.T) {
	if got, want := safeFileName(`  bad/name\with:*?"<>|  `), "bad_name_with_______"; got != want {
		t.Fatalf("safe name=%q want=%q", got, want)
	}
	if got := safeFileName(" \t "); got != "sem_nome" {
		t.Fatalf("empty safe name=%q", got)
	}
}

func TestRunConvertsSelectedRecordAndReportsSanitization(t *testing.T) {
	root := t.TempDir()
	inDir := filepath.Join(root, "in")
	outDir := filepath.Join(root, "out")
	if err := os.MkdirAll(inDir, 0o755); err != nil {
		t.Fatal(err)
	}
	writeMobRecord(t, filepath.Join(inDir, "candidate.mob"), func(b []byte) {
		putU16(b, offStatus+8, 100)
		putU16(b, offCarry, model.ItemListSize)
		binary.LittleEndian.PutUint32(b[24:28], maxSaneNPCGold+1)
		binary.LittleEndian.PutUint32(b[28:32], maxSaneNPCExp+1)
	})
	writeMobRecord(t, filepath.Join(inDir, "ignored.mob"), nil)
	if err := os.WriteFile(filepath.Join(inDir, "library.DLL"), []byte("ignored"), 0o644); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(filepath.Join(inDir, "directory"), 0o755); err != nil {
		t.Fatal(err)
	}

	var stdout, stderr strings.Builder
	if err := run([]string{"-in", inDir, "-out", outDir, "-only", "candidate.mob"}, &stdout, &stderr); err != nil {
		t.Fatalf("run: %v; stderr=%s", err, stderr.String())
	}
	if _, err := os.Stat(filepath.Join(outDir, "candidate.mob.json")); err != nil {
		t.Fatalf("converted output missing: %v", err)
	}
	if _, err := os.Stat(filepath.Join(outDir, "ignored.mob.json")); !os.IsNotExist(err) {
		t.Fatalf("unselected output exists or stat failed: %v", err)
	}
	for _, want := range []string{"converted=1", "skipped=0", "removed invalid item indexes=1", "gold=1 exp=1"} {
		if !strings.Contains(stdout.String(), want) {
			t.Errorf("stdout missing %q: %s", want, stdout.String())
		}
	}
	if stderr.Len() != 0 {
		t.Errorf("unexpected stderr: %s", stderr.String())
	}
}

func TestRunSkipsInvalidInputsAndOutputFailures(t *testing.T) {
	root := t.TempDir()
	inDir := filepath.Join(root, "in")
	outDir := filepath.Join(root, "out")
	if err := os.MkdirAll(inDir, 0o755); err != nil {
		t.Fatal(err)
	}
	if err := os.MkdirAll(filepath.Join(outDir, "blocked.json"), 0o755); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(inDir, "corrupt.mob"), []byte{1, 2, 3}, 0o644); err != nil {
		t.Fatal(err)
	}
	writeMobRecord(t, filepath.Join(inDir, "blocked"), nil)
	writeMobRecord(t, filepath.Join(inDir, "good"), nil)
	if err := os.WriteFile(filepath.Join(inDir, "ignored.dll"), []byte("ignored"), 0o644); err != nil {
		t.Fatal(err)
	}

	var stdout, stderr strings.Builder
	if err := run([]string{"-in", inDir, "-out", outDir}, &stdout, &stderr); err != nil {
		t.Fatalf("run: %v", err)
	}
	if !strings.Contains(stdout.String(), "converted=1") || !strings.Contains(stdout.String(), "skipped=2") {
		t.Fatalf("unexpected summary: %s", stdout.String())
	}
	for _, want := range []string{"SKIP corrupt.mob", "SKIP blocked: write:"} {
		if !strings.Contains(stderr.String(), want) {
			t.Errorf("stderr missing %q: %s", want, stderr.String())
		}
	}

	var selectedOut, selectedErr strings.Builder
	err := run([]string{"-in", inDir, "-out", filepath.Join(root, "selected"), "-only", "absent"}, &selectedOut, &selectedErr)
	if err == nil || !strings.Contains(err.Error(), `selected record "absent" was not converted`) {
		t.Fatalf("missing selection error: %v", err)
	}
}

func TestRunRejectsInvalidFlagsAndDirectories(t *testing.T) {
	root := t.TempDir()
	var stdout, stderr strings.Builder
	if err := run(nil, &stdout, &stderr); err == nil || !strings.Contains(err.Error(), "provide -in and -out") {
		t.Fatalf("missing required flags error: %v", err)
	}
	if err := run([]string{"-unknown"}, &stdout, &stderr); err == nil {
		t.Fatal("unknown flag accepted")
	}
	if err := run([]string{"-in", filepath.Join(root, "missing"), "-out", filepath.Join(root, "out")}, &stdout, &stderr); err == nil || !strings.Contains(err.Error(), "read input directory") {
		t.Fatalf("missing input directory error: %v", err)
	}
	file := filepath.Join(root, "output-file")
	if err := os.WriteFile(file, nil, 0o644); err != nil {
		t.Fatal(err)
	}
	if err := run([]string{"-in", root, "-out", filepath.Join(file, "child")}, &stdout, &stderr); err == nil || !strings.Contains(err.Error(), "create output directory") {
		t.Fatalf("invalid output directory error: %v", err)
	}
}

func TestMainRunsCLIWithProcessArguments(t *testing.T) {
	root := t.TempDir()
	inDir := filepath.Join(root, "in")
	if err := os.Mkdir(inDir, 0o755); err != nil {
		t.Fatal(err)
	}
	writeMobRecord(t, filepath.Join(inDir, "main.mob"), nil)
	oldArgs := os.Args
	os.Args = []string{"npcconvert", "-in", inDir, "-out", filepath.Join(root, "out")}
	t.Cleanup(func() { os.Args = oldArgs })
	main()
}

func writeMobRecord(t *testing.T, path string, edit func([]byte)) {
	t.Helper()
	b := make([]byte, mobFileSize)
	if edit != nil {
		edit(b)
	}
	if err := os.WriteFile(path, b, 0o644); err != nil {
		t.Fatal(fmt.Errorf("write record %s: %w", path, err))
	}
}
