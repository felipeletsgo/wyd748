package data

import (
	"bufio"
	"encoding/csv"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"

	"wydgo/internal/model"
)

// LoadTeleports le o formato Teleports.ini usado pelo Micronics:
// SourceX,SourceY,DestX,DestY,Price,Name.
func LoadTeleports(path string) ([]model.Teleport, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, fmt.Errorf("abrir teleportes %q: %w", path, err)
	}
	defer f.Close()

	var result []model.Teleport
	scanner := bufio.NewScanner(f)
	for line := 1; scanner.Scan(); line++ {
		text := strings.TrimSpace(scanner.Text())
		if text == "" || strings.HasPrefix(text, "#") || strings.HasPrefix(text, ";") {
			continue
		}
		reader := csv.NewReader(strings.NewReader(text))
		reader.TrimLeadingSpace = true
		reader.FieldsPerRecord = -1
		fields, err := reader.Read()
		if err != nil && err != io.EOF {
			return nil, fmt.Errorf("%s:%d: %w", path, line, err)
		}
		if len(fields) < 5 || len(fields) > 6 {
			return nil, fmt.Errorf("%s:%d: esperado SourceX,SourceY,DestX,DestY,Price[,Name]", path, line)
		}
		values := [5]uint64{}
		for i := range values {
			value, err := strconv.ParseUint(strings.TrimSpace(fields[i]), 10, 32)
			if err != nil {
				return nil, fmt.Errorf("%s:%d: valor numerico invalido %q", path, line, fields[i])
			}
			values[i] = value
		}
		if values[0] == 0 || values[0] >= 4096 || values[1] == 0 || values[1] >= 4096 ||
			values[2] == 0 || values[2] >= 4096 || values[3] == 0 || values[3] >= 4096 {
			return nil, fmt.Errorf("%s:%d: coordenada fora do mapa", path, line)
		}
		name := ""
		if len(fields) == 6 {
			name = strings.TrimSpace(fields[5])
		}
		result = append(result, model.Teleport{
			SourceX: uint16(values[0]), SourceY: uint16(values[1]),
			DestX: uint16(values[2]), DestY: uint16(values[3]),
			Price: uint32(values[4]), Name: name,
		})
	}
	if err := scanner.Err(); err != nil {
		return nil, fmt.Errorf("ler teleportes %q: %w", path, err)
	}
	if len(result) == 0 {
		return nil, fmt.Errorf("%s: nenhum teleporte ativo", path)
	}
	return result, nil
}
