package data

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"os"

	"wydgo/internal/model"
)

// LoadQuestZones le data/quest_zones.json com o contrato estrito dos demais
// arquivos: versao exata, sem campo desconhecido, sem conteudo extra. Arquivo
// ausente e estado valido (servidor sem zona de reset): devolve arquivo vazio.
func LoadQuestZones(path string) (model.QuestZoneFile, error) {
	file := model.QuestZoneFile{Version: model.QuestZoneFileVersion}
	b, err := os.ReadFile(path)
	if err != nil {
		if os.IsNotExist(err) {
			return file, nil
		}
		return file, err
	}
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
