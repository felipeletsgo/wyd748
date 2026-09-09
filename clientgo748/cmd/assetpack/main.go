// Command assetpack cria, offline, um pacote protegido a partir de uma
// textura WYT oficial. A chave privada existe somente na máquina de build e
// nunca é gravada no pacote ou lida pelo executável do client.
package main

import (
	"crypto/ed25519"
	"crypto/rand"
	"encoding/hex"
	"errors"
	"flag"
	"fmt"
	"os"
	"path/filepath"

	"wydclient748/internal/assets"
)

func main() {
	if err := run(); err != nil {
		fmt.Fprintln(os.Stderr, "assetpack:", err)
		os.Exit(1)
	}
}

func run() error {
	inPath := flag.String("in", "", "official WYT input path")
	outPath := flag.String("out", "", "protected package output path")
	assetID := flag.String("id", "", "asset identifier")
	assetPath := flag.String("path", "", "manifest asset path")
	assetVersion := flag.String("asset-version", "", "asset release identifier")
	keyID := flag.String("key-id", "", "content key identifier")
	revision := flag.Uint("revision", 1, "asset revision")
	contentKeyHex := flag.String("content-key-hex", "", "32-byte AES key in hexadecimal")
	privateKeyHex := flag.String("private-key-hex", "", "Ed25519 private key or seed in hexadecimal")
	flag.Parse()
	if *inPath == "" || *outPath == "" || *assetID == "" || *assetPath == "" || *assetVersion == "" || *keyID == "" {
		return errors.New("-in, -out, -id, -path, -asset-version and -key-id are required")
	}
	if *revision > uint(^uint32(0)) {
		return fmt.Errorf("revision %d exceeds uint32", *revision)
	}
	contentKey, err := decodeContentKey(*contentKeyHex)
	if err != nil {
		return fmt.Errorf("content key: %w", err)
	}
	privateKey, err := decodePrivateKey(*privateKeyHex)
	if err != nil {
		return fmt.Errorf("private key: %w", err)
	}
	inAbs, err := filepath.Abs(*inPath)
	if err != nil {
		return fmt.Errorf("resolve input: %w", err)
	}
	outAbs, err := filepath.Abs(*outPath)
	if err != nil {
		return fmt.Errorf("resolve output: %w", err)
	}
	if filepath.Clean(inAbs) == filepath.Clean(outAbs) {
		return errors.New("input and output must be different files")
	}
	texture, err := assets.LoadWYTFile(inAbs)
	if err != nil {
		return err
	}
	canonical, err := assets.EncodeTexture(texture)
	if err != nil {
		return err
	}
	packageData, err := assets.BuildProtectedPackage([]assets.PackageInput{{
		ID: *assetID, Kind: assets.KindTexture, Path: *assetPath, Revision: uint32(*revision), Data: canonical,
	}}, contentKey, privateKey, assets.PackageOptions{
		AssetVersion: *assetVersion,
		KeyID:        *keyID,
		Random:       rand.Reader,
	})
	if err != nil {
		return err
	}
	if err := writeAtomic(outAbs, packageData); err != nil {
		return err
	}
	publicKey := privateKey.Public().(ed25519.PublicKey)
	fmt.Printf("protected package written: %s (%d bytes)\n", outAbs, len(packageData))
	fmt.Printf("public-key-hex: %s\n", hex.EncodeToString(publicKey))
	return nil
}

func decodeContentKey(value string) ([32]byte, error) {
	var key [32]byte
	decoded, err := hex.DecodeString(value)
	if err != nil || len(decoded) != len(key) {
		return key, errors.New("must be exactly 32 bytes of hexadecimal")
	}
	copy(key[:], decoded)
	return key, nil
}

func decodePrivateKey(value string) (ed25519.PrivateKey, error) {
	decoded, err := hex.DecodeString(value)
	if err != nil {
		return nil, errors.New("must be hexadecimal")
	}
	switch len(decoded) {
	case ed25519.SeedSize:
		return ed25519.NewKeyFromSeed(decoded), nil
	case ed25519.PrivateKeySize:
		return ed25519.PrivateKey(decoded), nil
	default:
		return nil, errors.New("must be a 32-byte seed or 64-byte private key")
	}
}

func writeAtomic(path string, data []byte) error {
	if err := os.MkdirAll(filepath.Dir(path), 0o755); err != nil {
		return fmt.Errorf("create output directory: %w", err)
	}
	tmp, err := os.CreateTemp(filepath.Dir(path), ".assetpack-*")
	if err != nil {
		return fmt.Errorf("create temporary output: %w", err)
	}
	tmpPath := tmp.Name()
	defer os.Remove(tmpPath)
	if err := tmp.Chmod(0o644); err != nil {
		_ = tmp.Close()
		return fmt.Errorf("set output mode: %w", err)
	}
	if _, err := tmp.Write(data); err != nil {
		_ = tmp.Close()
		return fmt.Errorf("write output: %w", err)
	}
	if err := tmp.Close(); err != nil {
		return fmt.Errorf("close output: %w", err)
	}
	if err := os.Rename(tmpPath, path); err != nil {
		return fmt.Errorf("install output: %w", err)
	}
	return nil
}
