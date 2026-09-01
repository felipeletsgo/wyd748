package data

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

const validClientIntegrityJSON = `{
  "version": 1,
  "timeoutSeconds": 10,
  "probes": [{
    "category": 0,
    "byteOffset": 340,
    "expectedValue": 45,
    "asset": "Mesh/ch01.bon",
    "size": 376,
    "sha256": "7A348D6B582CD188CAA638D1857E5ED660971B5D04005E7436940D8726C2221F"
  }]
}`

func loadClientIntegrityText(t *testing.T, text string) error {
	t.Helper()
	path := filepath.Join(t.TempDir(), "client_integrity.json")
	if err := os.WriteFile(path, []byte(text), 0o600); err != nil {
		t.Fatal(err)
	}
	_, err := LoadClientIntegrity(path)
	return err
}

func TestLoadClientIntegrityValidManifest(t *testing.T) {
	if err := loadClientIntegrityText(t, validClientIntegrityJSON); err != nil {
		t.Fatal(err)
	}
}

func TestLoadClientIntegrityRejectsInvalidManifest(t *testing.T) {
	tests := map[string]string{
		"unknown field":   strings.Replace(validClientIntegrityJSON, `"version": 1`, `"version": 1, "extra": true`, 1),
		"additional JSON": validClientIntegrityJSON + ` {}`,
		"int8 overflow":   strings.Replace(validClientIntegrityJSON, `"expectedValue": 45`, `"expectedValue": 128`, 1),
		"duplicate probe": strings.Replace(validClientIntegrityJSON, `  }]`, `  }, {
    "category": 0,
    "byteOffset": 340,
    "expectedValue": 45,
    "asset": "Mesh/ch01.bon",
    "size": 376,
    "sha256": "7A348D6B582CD188CAA638D1857E5ED660971B5D04005E7436940D8726C2221F"
  }]`, 1),
		"absolute path": strings.Replace(validClientIntegrityJSON, `Mesh/ch01.bon`, `C:\\WYD\\Mesh\\ch01.bon`, 1),
		"traversal":     strings.Replace(validClientIntegrityJSON, `Mesh/ch01.bon`, `../Mesh/ch01.bon`, 1),
		"empty asset":   strings.Replace(validClientIntegrityJSON, `Mesh/ch01.bon`, `   `, 1),
		"dot asset":     strings.Replace(validClientIntegrityJSON, `Mesh/ch01.bon`, `.`, 1),
	}
	for name, text := range tests {
		t.Run(name, func(t *testing.T) {
			if err := loadClientIntegrityText(t, text); err == nil {
				t.Fatal("manifesto invalido foi aceito")
			}
		})
	}
}
