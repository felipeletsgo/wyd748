package model

import (
	"fmt"
	"path/filepath"
	"regexp"
	"strings"
)

const (
	ClientIntegrityVersion   = 1
	MaxClientIntegrityProbes = 100
)

var clientIntegritySHA256 = regexp.MustCompile(`^[0-9A-F]{64}$`)

// ClientIntegrityProbe describes one byte in a source-client asset. It is an
// operational compatibility check, not a cryptographic anti-cheat primitive.
type ClientIntegrityProbe struct {
	Category      int32  `json:"category"`
	ByteOffset    int32  `json:"byteOffset"`
	ExpectedValue int32  `json:"expectedValue"`
	Asset         string `json:"asset"`
	Size          int64  `json:"size"`
	SHA256        string `json:"sha256"`
}

type ClientIntegrityFile struct {
	Version        int                    `json:"version"`
	TimeoutSeconds int                    `json:"timeoutSeconds"`
	Probes         []ClientIntegrityProbe `json:"probes"`
}

func (f ClientIntegrityFile) Validate() error {
	if f.Version != ClientIntegrityVersion {
		return fmt.Errorf("version=%d, esperada %d", f.Version, ClientIntegrityVersion)
	}
	if f.TimeoutSeconds < 1 || f.TimeoutSeconds > 60 {
		return fmt.Errorf("timeoutSeconds deve ficar entre 1 e 60")
	}
	if len(f.Probes) > MaxClientIntegrityProbes {
		return fmt.Errorf("probes possui %d entradas, maximo %d", len(f.Probes), MaxClientIntegrityProbes)
	}
	seen := make(map[[2]int32]struct{}, len(f.Probes))
	for i, probe := range f.Probes {
		if probe.Category < 0 || probe.Category > 99 {
			return fmt.Errorf("probes[%d].category deve ficar entre 0 e 99", i)
		}
		if probe.ByteOffset < 0 || int64(probe.ByteOffset) >= probe.Size {
			return fmt.Errorf("probes[%d].byteOffset deve apontar para um byte dentro do asset", i)
		}
		if probe.ExpectedValue < -128 || probe.ExpectedValue > 127 {
			return fmt.Errorf("probes[%d].expectedValue deve caber em int8", i)
		}
		asset := strings.TrimSpace(probe.Asset)
		cleaned := filepath.Clean(asset)
		if asset == "" || cleaned == "." || filepath.IsAbs(cleaned) || filepath.VolumeName(cleaned) != "" ||
			cleaned == ".." || strings.HasPrefix(cleaned, ".."+string(filepath.Separator)) {
			return fmt.Errorf("probes[%d].asset deve ser um caminho relativo seguro", i)
		}
		if probe.Size <= 0 {
			return fmt.Errorf("probes[%d].size deve ser positivo", i)
		}
		if !clientIntegritySHA256.MatchString(probe.SHA256) {
			return fmt.Errorf("probes[%d].sha256 deve conter 64 hexadecimais maiusculos", i)
		}
		key := [2]int32{probe.Category, probe.ByteOffset}
		if _, duplicate := seen[key]; duplicate {
			return fmt.Errorf("probes[%d] duplica category/byteOffset", i)
		}
		seen[key] = struct{}{}
	}
	return nil
}
