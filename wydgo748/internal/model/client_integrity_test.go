package model

import (
	"strings"
	"testing"
)

func validClientIntegrityFile() ClientIntegrityFile {
	return ClientIntegrityFile{
		Version:        ClientIntegrityVersion,
		TimeoutSeconds: 10,
		Probes: []ClientIntegrityProbe{{
			Category: 0, ByteOffset: 0, ExpectedValue: -128,
			Asset: "Effect/cancel.wys", Size: 1, SHA256: strings.Repeat("A", 64),
		}},
	}
}

func TestSafeClientAssetPath(t *testing.T) {
	for _, path := range []string{
		"Effect/cancel.wys", `Effect\cancel.wys`, " data/asset.bin ", "data/../Effect/cancel.wys",
	} {
		if !isSafeClientAssetPath(path) {
			t.Errorf("safe relative asset path %q was rejected", path)
		}
	}
	for _, path := range []string{
		"", " . ", "..", "../asset.bin", `..\asset.bin`, "/asset.bin", `\asset.bin`,
		`C:\asset.bin`, "d:asset.bin", `\\server\share\asset.bin`, "asset\x00.bin",
	} {
		if isSafeClientAssetPath(path) {
			t.Errorf("unsafe asset path %q was accepted", path)
		}
	}
}

func TestClientIntegrityValidationRejectsInvalidManifest(t *testing.T) {
	for _, test := range []struct {
		name   string
		mutate func(*ClientIntegrityFile)
	}{
		{"version", func(f *ClientIntegrityFile) { f.Version++ }},
		{"timeout below range", func(f *ClientIntegrityFile) { f.TimeoutSeconds = 0 }},
		{"timeout above range", func(f *ClientIntegrityFile) { f.TimeoutSeconds = 61 }},
		{"too many probes", func(f *ClientIntegrityFile) { f.Probes = make([]ClientIntegrityProbe, MaxClientIntegrityProbes+1) }},
		{"category below range", func(f *ClientIntegrityFile) { f.Probes[0].Category = -1 }},
		{"category above range", func(f *ClientIntegrityFile) { f.Probes[0].Category = 100 }},
		{"negative offset", func(f *ClientIntegrityFile) { f.Probes[0].ByteOffset = -1 }},
		{"offset at size", func(f *ClientIntegrityFile) { f.Probes[0].ByteOffset = 1 }},
		{"expected value below int8", func(f *ClientIntegrityFile) { f.Probes[0].ExpectedValue = -129 }},
		{"expected value above int8", func(f *ClientIntegrityFile) { f.Probes[0].ExpectedValue = 128 }},
		{"unsafe path", func(f *ClientIntegrityFile) { f.Probes[0].Asset = `..\outside.bin` }},
		{"lowercase hash", func(f *ClientIntegrityFile) { f.Probes[0].SHA256 = strings.Repeat("a", 64) }},
		{"short hash", func(f *ClientIntegrityFile) { f.Probes[0].SHA256 = strings.Repeat("A", 63) }},
		{"duplicate category and offset", func(f *ClientIntegrityFile) {
			f.Probes = append(f.Probes, f.Probes[0])
		}},
	} {
		t.Run(test.name, func(t *testing.T) {
			file := validClientIntegrityFile()
			test.mutate(&file)
			if err := file.Validate(); err == nil {
				t.Fatal("invalid client integrity manifest was accepted")
			}
		})
	}
}

func TestClientIntegrityValidationAcceptsBoundariesAndDistinctOffsets(t *testing.T) {
	file := validClientIntegrityFile()
	file.TimeoutSeconds = 60
	file.Probes[0].Category = 99
	file.Probes[0].ExpectedValue = 127
	file.Probes[0].ByteOffset = 7
	file.Probes[0].Size = 8
	file.Probes = append(file.Probes, ClientIntegrityProbe{
		Category: 99, ByteOffset: 0, ExpectedValue: 0,
		Asset: `data\second.bin`, Size: 1, SHA256: strings.Repeat("F", 64),
	})
	if err := file.Validate(); err != nil {
		t.Fatalf("valid boundary manifest was rejected: %v", err)
	}

	empty := ClientIntegrityFile{Version: ClientIntegrityVersion, TimeoutSeconds: 1}
	if err := empty.Validate(); err != nil {
		t.Fatalf("empty probe list was rejected: %v", err)
	}
}
