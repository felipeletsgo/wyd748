package data

import (
	"fmt"
	"os"

	"wydgo/internal/model"
)

// LoadTerrain carrega os mapas binarios originais do TMSrv sem conversao.
func LoadTerrain(heightPath, attributePath string) (model.TerrainMap, error) {
	height, err := os.ReadFile(heightPath)
	if err != nil {
		return model.TerrainMap{}, fmt.Errorf("ler HeightMap %q: %w", heightPath, err)
	}
	if len(height) != model.TerrainCells {
		return model.TerrainMap{}, fmt.Errorf("HeightMap %q possui %d bytes; esperado %d",
			heightPath, len(height), model.TerrainCells)
	}
	attribute, err := os.ReadFile(attributePath)
	if err != nil {
		return model.TerrainMap{}, fmt.Errorf("ler AttributeMap %q: %w", attributePath, err)
	}
	if len(attribute) != model.AttributeCells {
		return model.TerrainMap{}, fmt.Errorf("AttributeMap %q possui %d bytes; esperado %d",
			attributePath, len(attribute), model.AttributeCells)
	}
	return model.TerrainMap{Height: height, Attribute: attribute}, nil
}
