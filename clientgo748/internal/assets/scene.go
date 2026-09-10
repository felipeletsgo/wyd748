package assets

import (
	"encoding/binary"
	"fmt"
)

// SceneControl contém dados próprios do registro RC, sem ponteiros nativos.
// Words preserva signedness e campos específicos; captions indexadas ainda
// precisam ser resolvidas na tabela de strings antes da renderização.
type SceneControl struct {
	Kind  int32
	Words []int32
}

// ID returns the native resource-control identifier (the first field of the
// record). It is deliberately kept separate from Kind: Kind selects the
// record layout while ID selects the widget binding/callback.
func (c SceneControl) ID() (int32, bool) {
	if len(c.Words) == 0 {
		return 0, false
	}
	return c.Words[0], true
}

// FindControl resolves a control by its native ID without exposing the
// backing slice. Callers must treat the returned record as read-only.
func FindControl(controls []SceneControl, id int32) (SceneControl, bool) {
	for _, control := range controls {
		if got, ok := control.ID(); ok && got == id {
			return control, true
		}
	}
	return SceneControl{}, false
}

// ParseScene decodifica o formato indexado de SelServerScene2.bin.
// Procedência: leitor 7.48 FUN_004974ec e asset local; implementação Go nova.
// Não aceita a variante inline de LoginScene2 nem tenta ressincronizar bytes.
// Em erro retorna nil para impedir publicação de uma cena parcialmente lida.
func ParseScene(data []byte) ([]SceneControl, error) {
	var controls []SceneControl
	if len(data) == 0 {
		return nil, fmt.Errorf("empty scene resource")
	}
	for offset := 0; offset < len(data); {
		start := offset
		if len(data)-offset < 4 {
			return nil, fmt.Errorf("truncated scene type at %d", offset)
		}
		kind := int32(binary.LittleEndian.Uint32(data[offset:]))
		offset += 4
		size := 0
		switch kind {
		case 1, 2, 16:
			size = 40
		case 3:
			size = 32
		case 6, 12:
			size = 52
		case 10:
			size = 48
		case 13:
			size = 184
		case 15:
			size = 28
		default:
			return nil, fmt.Errorf("unsupported scene type %d at %d", kind, start)
		}
		if len(data)-offset < size {
			return nil, fmt.Errorf("truncated scene record at %d", start)
		}
		words := make([]int32, size/4)
		for i := range words {
			words[i] = int32(binary.LittleEndian.Uint32(data[offset+i*4:]))
		}
		controls = append(controls, SceneControl{Kind: kind, Words: words})
		offset += size
	}
	return controls, nil
}
