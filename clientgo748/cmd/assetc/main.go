// Command assetc converte um asset oficial para o formato interno do client.
// Esta primeira versão processa uma textura WYT por execução; o pacote e a
// assinatura de distribuição serão adicionados em uma unidade posterior.
package main

import (
	"errors"
	"flag"
	"fmt"
	"os"
	"path/filepath"

	"wydclient748/internal/assets"
)

func main() {
	if err := run(); err != nil {
		fmt.Fprintln(os.Stderr, "assetc:", err)
		os.Exit(1)
	}
}

func run() error {
	inPath := flag.String("in", "", "official WYT input path")
	outPath := flag.String("out", "", "canonical output path")
	flag.Parse()
	if *inPath == "" || *outPath == "" {
		return errors.New("both -in and -out are required")
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
	encoded, err := assets.EncodeTexture(texture)
	if err != nil {
		return err
	}
	if err := writeAtomic(outAbs, encoded); err != nil {
		return err
	}
	fmt.Printf("converted %s -> %s (%d bytes)\n", inAbs, outAbs, len(encoded))
	return nil
}

func writeAtomic(path string, data []byte) error {
	if err := os.MkdirAll(filepath.Dir(path), 0o755); err != nil {
		return fmt.Errorf("create output directory: %w", err)
	}
	tmp, err := os.CreateTemp(filepath.Dir(path), ".assetc-*")
	if err != nil {
		return fmt.Errorf("create temporary output: %w", err)
	}
	tmpPath := tmp.Name()
	defer os.Remove(tmpPath)
	if err := tmp.Chmod(0o644); err != nil {
		tmp.Close()
		return fmt.Errorf("set output mode: %w", err)
	}
	if _, err := tmp.Write(data); err != nil {
		tmp.Close()
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
