package data

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"os"

	"wydgo/internal/model"
)

// LoadNetworkAdmission le a politica local de reputacao de rede. Ela nunca e
// consultada por HTTP durante o login: indisponibilidade de terceiros nao pode
// bloquear nem atrasar o ator World.
func LoadNetworkAdmission(path string) (model.NetworkAdmissionFile, error) {
	b, err := os.ReadFile(path)
	if err != nil {
		return model.NetworkAdmissionFile{}, err
	}
	var file model.NetworkAdmissionFile
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
