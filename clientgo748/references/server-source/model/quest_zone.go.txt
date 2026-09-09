package model

import "fmt"

// QuestZoneFileVersion segue o mesmo contrato estrito dos demais arquivos de
// dados: versao exata, sem campo desconhecido, sem conteudo extra.
const QuestZoneFileVersion = 1

// QuestZone e um retangulo do mundo (inclusive nas bordas) que sofre reset
// periodico: todo jogador dentro dele e recolhido para a cidade a cada ciclo.
// Porta o mecanismo ClearArea do W2PP (as areas de quest de nivel expulsam o
// jogador apos ~10 minutos, revivendo-o se estiver morto).
type QuestZone struct {
	Name string `json:"name"`
	X1   uint16 `json:"x1"`
	Y1   uint16 `json:"y1"`
	X2   uint16 `json:"x2"`
	Y2   uint16 `json:"y2"`
}

// Contains diz se a coordenada esta dentro do retangulo (inclusive as bordas,
// como o `>=`/`<=` do nativo).
func (z QuestZone) Contains(x, y uint16) bool {
	return x >= z.X1 && x <= z.X2 && y >= z.Y1 && y <= z.Y2
}

func (z *QuestZone) Validate() error {
	if z == nil {
		return fmt.Errorf("zona de quest ausente")
	}
	if z.Name == "" {
		return fmt.Errorf("zona de quest sem nome")
	}
	if z.X2 < z.X1 || z.Y2 < z.Y1 {
		return fmt.Errorf("zona %q com retangulo invertido (%d,%d)-(%d,%d)",
			z.Name, z.X1, z.Y1, z.X2, z.Y2)
	}
	return nil
}

// QuestZoneFile e o conteudo de data/quest_zones.json.
type QuestZoneFile struct {
	Version int         `json:"version"`
	Zones   []QuestZone `json:"zones"`
}

func (f *QuestZoneFile) Validate() error {
	if f == nil {
		return fmt.Errorf("arquivo de zonas ausente")
	}
	if f.Version != QuestZoneFileVersion {
		return fmt.Errorf("arquivo de zonas versao %d; esperado %d", f.Version, QuestZoneFileVersion)
	}
	names := make(map[string]struct{}, len(f.Zones))
	for i := range f.Zones {
		if err := f.Zones[i].Validate(); err != nil {
			return err
		}
		if _, dup := names[f.Zones[i].Name]; dup {
			return fmt.Errorf("zona %q duplicada", f.Zones[i].Name)
		}
		names[f.Zones[i].Name] = struct{}{}
	}
	return nil
}
