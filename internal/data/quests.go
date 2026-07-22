package data

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"os"

	"wydgo/internal/model"
)

// LoadQuests le data/quests.json com o mesmo contrato estrito dos demais
// arquivos: versao exata, sem campo desconhecido e sem conteudo extra.
//
// Arquivo ausente e estado valido (servidor sem quest configurada): devolve um
// arquivo vazio na versao corrente. Arquivo presente e invalido derruba o boot,
// porque quest silenciosamente ignorada e pior que servidor que nao sobe.
func LoadQuests(path string) (model.QuestFile, error) {
	file := model.QuestFile{Version: model.QuestFileVersion}
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
