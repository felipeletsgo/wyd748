package model

import "testing"

func TestTerrainWalkableUsesHeightAndCantGo(t *testing.T) {
	height := make([]byte, TerrainCells)
	attribute := make([]byte, AttributeCells)
	terrain := TerrainMap{Height: height, Attribute: attribute}
	if !terrain.Walkable(2115, 2088) {
		t.Fatal("celula normal foi bloqueada")
	}
	height[2088*TerrainWidth+2119] = TerrainBlockedByte
	if terrain.Walkable(2119, 2088) {
		t.Fatal("altura 127 deveria bloquear a celula")
	}
	attribute[(2100/4)*AttributeWidth+2200/4] = 0x02
	if terrain.Walkable(2200, 2100) {
		t.Fatal("atributo CantGo deveria bloquear o bloco")
	}
}

func TestTerrainHeightCompatibilityRejectsCliffs(t *testing.T) {
	height := make([]byte, TerrainCells)
	attribute := make([]byte, AttributeCells)
	terrain := TerrainMap{Height: height, Attribute: attribute}
	height[100*TerrainWidth+100] = 10
	height[100*TerrainWidth+101] = 59
	if !terrain.HeightCompatible(100, 100, 101, 100) {
		t.Fatal("delta 49 deveria ser caminhavel")
	}
	height[100*TerrainWidth+101] = 60
	if terrain.HeightCompatible(100, 100, 101, 100) {
		t.Fatal("delta 50 deveria ser bloqueado")
	}
}

func TestTerrainRouteUsesSignedHeightAndNativeRange(t *testing.T) {
	height := make([]byte, TerrainCells)
	attribute := make([]byte, AttributeCells)
	terrain := TerrainMap{Height: height, Attribute: attribute}

	height[100*TerrainWidth+100] = 253 // int8(-3)
	height[100*TerrainWidth+101] = 4
	if !terrain.RouteHeightCompatible(100, 100, 101, 100) {
		t.Fatal("delta assinado 7 deveria ser caminhavel")
	}
	height[100*TerrainWidth+101] = 5
	if terrain.RouteHeightCompatible(100, 100, 101, 100) {
		t.Fatal("delta assinado 8 deveria ser bloqueado")
	}
	// A busca de spawn possui a faixa mais ampla de GetEmptyMobGridValid.
	if !terrain.HeightCompatible(100, 100, 101, 100) {
		t.Fatal("delta 8 deveria continuar valido para reposicionamento")
	}
}
