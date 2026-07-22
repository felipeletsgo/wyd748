package model

const (
	TerrainWidth       = 4096
	TerrainHeight      = 4096
	TerrainCells       = TerrainWidth * TerrainHeight
	AttributeWidth     = 1024
	AttributeHeight    = 1024
	AttributeCells     = AttributeWidth * AttributeHeight
	TerrainBlockedByte = byte(127)
	SpawnHeightRange   = 50
	RouteHeightRange   = 8
)

// TerrainMap guarda os dois mapas nativos do TMSrv. Height possui uma celula
// por coordenada do mundo; Attribute possui uma celula para cada bloco 4x4.
type TerrainMap struct {
	Height    []byte
	Attribute []byte
}

func (m TerrainMap) Loaded() bool {
	return len(m.Height) == TerrainCells && len(m.Attribute) == AttributeCells
}

// Walkable reproduz BASE_ApplyAttribute/GetEmptyMobGrid: altura 127 e o bit
// CantGo (0x02) representam celulas nas quais mobs/NPCs nao podem existir.
func (m TerrainMap) Walkable(x, y uint16) bool {
	if !m.Loaded() {
		return true // facilita mundos isolados em testes; producao carrega os .dat
	}
	if int(x) >= TerrainWidth || int(y) >= TerrainHeight {
		return false
	}
	if m.Height[int(y)*TerrainWidth+int(x)] == TerrainBlockedByte {
		return false
	}
	attribute := m.Attribute[(int(y)/4)*AttributeWidth+int(x)/4]
	return attribute&0x02 == 0
}

func (m TerrainMap) heightCompatible(fromX, fromY, toX, toY uint16, limit int) bool {
	if !m.Walkable(toX, toY) {
		return false
	}
	if !m.Loaded() {
		return true
	}
	if int(fromX) >= TerrainWidth || int(fromY) >= TerrainHeight {
		return true
	}
	from := m.Height[int(fromY)*TerrainWidth+int(fromX)]
	if from == TerrainBlockedByte {
		// Permite que o buscador corrija um ponto de origem invalido.
		return true
	}
	to := m.Height[int(toY)*TerrainWidth+int(toX)]
	// HeightMap e char[4096][4096] no TMSrv x86. Valores 128..255 sao alturas
	// negativas, nao alturas positivas altas.
	delta := int(int8(from)) - int(int8(to))
	if delta < 0 {
		delta = -delta
	}
	return delta < limit
}

// HeightCompatible aplica a faixa de GetEmptyMobGridValid ao procurar um
// substituto proximo para spawn/recall.
func (m TerrainMap) HeightCompatible(fromX, fromY, toX, toY uint16) bool {
	return m.heightCompatible(fromX, fromY, toX, toY, SpawnHeightRange)
}

// RouteHeightCompatible aplica o MH=8 estrito de BASE_GetRoute. O client usa a
// mesma regra ao reconstruir a rota visual.
func (m TerrainMap) RouteHeightCompatible(fromX, fromY, toX, toY uint16) bool {
	return m.heightCompatible(fromX, fromY, toX, toY, RouteHeightRange)
}
