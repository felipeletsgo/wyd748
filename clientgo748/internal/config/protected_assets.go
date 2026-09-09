package config

import (
	"crypto/ed25519"
	"encoding/hex"
	"errors"
	"fmt"
	"os"
)

// ProtectedAssetSettings descreve somente a configuração pública necessária
// para abrir um pacote autenticado. A chave privada não pertence ao client.
type ProtectedAssetSettings struct {
	PackagePath string
	LogoPath    string
	ContentKey  [32]byte
	PublicKey   ed25519.PublicKey
}

// LoadProtectedAssetSettingsFromEnv habilita assets protegidos apenas quando
// os três valores do contrato estão presentes. Configuração parcial falha em
// vez de cair silenciosamente para um caminho não autenticado.
func LoadProtectedAssetSettingsFromEnv() (*ProtectedAssetSettings, bool, error) {
	packagePath, packageSet := os.LookupEnv("WYD_ASSET_PACKAGE")
	contentHex, contentSet := os.LookupEnv("WYD_ASSET_CONTENT_KEY_HEX")
	publicHex, publicSet := os.LookupEnv("WYD_ASSET_PUBLIC_KEY_HEX")
	if !packageSet && !contentSet && !publicSet {
		return nil, false, nil
	}
	if packagePath == "" || contentHex == "" || publicHex == "" {
		return nil, false, errors.New("WYD_ASSET_PACKAGE, WYD_ASSET_CONTENT_KEY_HEX and WYD_ASSET_PUBLIC_KEY_HEX must be set together")
	}
	var settings ProtectedAssetSettings
	settings.PackagePath = packagePath
	settings.LogoPath = os.Getenv("WYD_ASSET_LOGO_PATH")
	if settings.LogoPath == "" {
		settings.LogoPath = "UI/logo1.wydasset"
	}
	contentKey, err := hex.DecodeString(contentHex)
	if err != nil || len(contentKey) != len(settings.ContentKey) {
		return nil, false, errors.New("WYD_ASSET_CONTENT_KEY_HEX must contain exactly 32 bytes")
	}
	copy(settings.ContentKey[:], contentKey)
	publicKey, err := hex.DecodeString(publicHex)
	if err != nil || len(publicKey) != ed25519.PublicKeySize {
		return nil, false, fmt.Errorf("WYD_ASSET_PUBLIC_KEY_HEX must contain exactly %d bytes", ed25519.PublicKeySize)
	}
	settings.PublicKey = append(ed25519.PublicKey(nil), publicKey...)
	return &settings, true, nil
}
