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

// LineOfSight verifica o segmento inteiro entre duas celulas. O TMSrv nativo
// valida distancia e regioes, mas nao oferece uma LOS geral em todas as skills;
// esta e uma camada adicional de hardening contra ataque atraves de paredes.
// Em mapas nao carregados (testes isolados), conserva o comportamento permissivo.
func (m TerrainMap) LineOfSight(fromX, fromY, toX, toY uint16) bool {
	if !m.Loaded() {
		return true
	}
	if !m.Walkable(fromX, fromY) || !m.Walkable(toX, toY) {
		return false
	}

	x0, y0 := int(fromX), int(fromY)
	x1, y1 := int(toX), int(toY)
	dx, dy := absTerrain(x1-x0), absTerrain(y1-y0)
	stepX, stepY := -1, -1
	if x0 < x1 {
		stepX = 1
	}
	if y0 < y1 {
		stepY = 1
	}
	err := dx - dy

	for x0 != x1 || y0 != y1 {
		previousX, previousY := x0, y0
		twiceErr := 2 * err
		if twiceErr > -dy {
			err -= dy
			x0 += stepX
		}
		if twiceErr < dx {
			err += dx
			y0 += stepY
		}
		if x0 < 0 || y0 < 0 || x0 >= TerrainWidth || y0 >= TerrainHeight {
			return false
		}
		ux, uy := uint16(x0), uint16(y0)
		if !m.Walkable(ux, uy) ||
			!m.RouteHeightCompatible(uint16(previousX), uint16(previousY), ux, uy) {
			return false
		}
		// Nao atravesse o canto fechado por duas paredes.
		if x0 != previousX && y0 != previousY &&
			!m.Walkable(uint16(x0), uint16(previousY)) &&
			!m.Walkable(uint16(previousX), uint16(y0)) {
			return false
		}
	}
	return true
}

func absTerrain(value int) int {
	if value < 0 {
		return -value
	}
	return value
}
