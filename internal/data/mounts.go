package data

import (
	"encoding/json"
	"fmt"
	"os"

	"wydgo/internal/model"
)

// LoadMounts le data/mounts.json: os atributos base de cada TIPO de montaria
// (0..29) portados da tabela g_pMountBonus do W2PP. Arquivo ausente NAO e erro
// -- devolve um catalogo vazio, e a montaria simplesmente nao adiciona bonus.
func LoadMounts(path string) (model.MountCatalog, error) {
	b, err := os.ReadFile(path)
	if err != nil {
		if os.IsNotExist(err) {
			return model.MountCatalog{Types: map[int]model.MountStats{}}, nil
		}
		return model.MountCatalog{}, err
	}
	var file struct {
		Types map[int]model.MountStats `json:"types"`
	}
	if err := json.Unmarshal(b, &file); err != nil {
		return model.MountCatalog{}, fmt.Errorf("data: parse %s: %w", path, err)
	}
	if file.Types == nil {
		file.Types = map[int]model.MountStats{}
	}
	for t := range file.Types {
		if t < 0 || t >= model.MountTypeCount {
			return model.MountCatalog{}, fmt.Errorf("data: %s tipo de montaria invalido %d (0..%d)", path, t, model.MountTypeCount-1)
		}
	}
	return model.MountCatalog{Types: file.Types}, nil
}
