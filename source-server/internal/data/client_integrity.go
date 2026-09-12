package data

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"os"

	"wydgo/internal/model"
)

func LoadClientIntegrity(path string) (model.ClientIntegrityFile, error) {
	b, err := os.ReadFile(path)
	if err != nil {
		return model.ClientIntegrityFile{}, err
	}
	var file model.ClientIntegrityFile
	decoder := json.NewDecoder(bytes.NewReader(b))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&file); err != nil {
		return file, fmt.Errorf("data: parse %s: %w", path, err)
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		if err == nil {
			err = fmt.Errorf("conteudo JSON adicional")
		}
		return file, fmt.Errorf("data: parse %s: %w", path, err)
	}
	if err := file.Validate(); err != nil {
		return file, fmt.Errorf("data: %s: %w", path, err)
	}
	return file, nil
}
