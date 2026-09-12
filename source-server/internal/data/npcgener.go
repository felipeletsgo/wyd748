package data

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"

	"wydgo/internal/model"
)

// LoadNPCGener le o NPCGener.txt tradicional do TMSrv. A numeracao efetiva e a
// ordem das secoes, como em CNPCGenerator::ReadNPCGenerator; o numero entre [] e
// apenas decorativo no parser original. Secoes iniciadas por #* ficam desativadas.
func LoadNPCGener(path string) ([]model.NPCGener, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	var out []model.NPCGener
	var cur *model.NPCGener
	scanner := bufio.NewScanner(f)
	scanner.Buffer(make([]byte, 1024), 1024*1024)
	lineNo := 0
	for scanner.Scan() {
		lineNo++
		line := strings.TrimSpace(scanner.Text())
		if line == "" || strings.HasPrefix(line, "//") {
			continue
		}
		if strings.HasPrefix(line, "#") {
			out = append(out, model.NPCGener{
				Index: len(out), Enabled: !strings.HasPrefix(line, "#*"),
				MinuteGenerate: -1, MaxNumMob: 1,
			})
			cur = &out[len(out)-1]
			continue
		}
		if cur == nil {
			continue // o TMSrv tambem ignora texto antes da primeira secao
		}
		fields := strings.Fields(line)
		if len(fields) < 2 {
			continue
		}
		key, value := fields[0], fields[1]
		if err := setGenerField(cur, key, value); err != nil {
			return nil, fmt.Errorf("NPCGener %s:%d: %w", path, lineNo, err)
		}
	}
	if err := scanner.Err(); err != nil {
		return nil, err
	}
	for i := range out {
		g := &out[i]
		if !g.Enabled {
			continue
		}
		if g.Leader == "" || g.Leader == "0" {
			return nil, fmt.Errorf("NPCGener %s: gerador %d sem Leader valido", path, g.Index)
		}
		if g.Follower == "" || g.Follower == "0" {
			g.Follower = g.Leader
		}
		if g.MaxGroup < g.MinGroup {
			// Quirk comum em NPCGener editados a mao (8 casos no arquivo do
			// Micronics): teto abaixo do piso. O TMSrv nativo nao rejeita o
			// arquivo por isso; normalizamos para grupo de tamanho fixo
			// (MaxGroup = MinGroup) em vez de abortar o boot.
			g.MaxGroup = g.MinGroup
		}
		if g.MaxNumMob <= 0 {
			return nil, fmt.Errorf("NPCGener %s: gerador %d MaxNumMob deve ser positivo", path, g.Index)
		}
	}
	return out, nil
}

func setGenerField(g *model.NPCGener, key, value string) error {
	switch key {
	case "Leader:":
		g.Leader = value
		return nil
	case "Follower:":
		g.Follower = value
		return nil
	}
	if strings.HasSuffix(key, "Action:") {
		if idx, ok := generSegmentIndex(key); ok {
			g.Segments[idx].Action = value
		}
		return nil
	}

	// Daqui pra baixo o campo e inteiro. Uma chave FORA do vocabulario (ex.: o
	// comentario malformado "/ Capa Verde" com uma barra so, ou lixo de edicao)
	// e ignorada SEM tentar converter -- e o que o CNPCGenerator nativo faz:
	// linhas nao reconhecidas nao sao erro. Chave reconhecida com valor invalido
	// continua sendo erro.
	seg, isSeg := generSegmentIndex(key)
	isSegInt := isSeg && (strings.HasSuffix(key, "X:") || strings.HasSuffix(key, "Y:") ||
		strings.HasSuffix(key, "Range:") || strings.HasSuffix(key, "Wait:"))
	switch key {
	case "MinuteGenerate:", "MaxNumMob:", "MinGroup:", "MaxGroup:", "RouteType:", "Formation:":
	default:
		if !isSegInt {
			return nil // linha desconhecida -> ignora
		}
	}

	n, err := strconv.Atoi(value)
	if err != nil {
		return fmt.Errorf("%s espera inteiro, recebeu %q", key, value)
	}
	switch key {
	case "MinuteGenerate:":
		g.MinuteGenerate = n
	case "MaxNumMob:":
		g.MaxNumMob = n
	case "MinGroup:":
		g.MinGroup = n
	case "MaxGroup:":
		g.MaxGroup = n
	case "RouteType:":
		g.RouteType = n
	case "Formation:":
		g.Formation = n
	default: // campo de segmento inteiro (seg ja resolvido acima)
		switch {
		case strings.HasSuffix(key, "X:"):
			if n <= 0 || n >= model.TerrainWidth {
				return fmt.Errorf("%s fora do mapa: %d", key, n)
			}
			g.Segments[seg].X = uint16(n)
		case strings.HasSuffix(key, "Y:"):
			if n <= 0 || n >= model.TerrainHeight {
				return fmt.Errorf("%s fora do mapa: %d", key, n)
			}
			g.Segments[seg].Y = uint16(n)
		case strings.HasSuffix(key, "Range:"):
			if n < 0 || n > 65535 {
				return fmt.Errorf("%s invalido: %d", key, n)
			}
			g.Segments[seg].Range = uint16(n)
		case strings.HasSuffix(key, "Wait:"):
			g.Segments[seg].Wait = n
		}
	}
	return nil
}

func generSegmentIndex(key string) (int, bool) {
	switch {
	case strings.HasPrefix(key, "Start"):
		return 0, true
	case strings.HasPrefix(key, "Segment1"):
		return 1, true
	case strings.HasPrefix(key, "Segment2"):
		return 2, true
	case strings.HasPrefix(key, "Segment3"):
		return 3, true
	case strings.HasPrefix(key, "Dest"):
		return 4, true
	default:
		return 0, false
	}
}
