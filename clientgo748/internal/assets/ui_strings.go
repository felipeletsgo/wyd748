package assets

import (
	"bufio"
	"bytes"
	"fmt"
	"strconv"
	"strings"
)

// ParseUIStrings lê a tabela textual de captions do pacote. Índices pertencem
// ao recurso; underscores representam espaços. O mapa retornado é próprio.
// Procedência: UIString.txt 7.48; sem copiar o armazenamento global C++.
func ParseUIStrings(data []byte) (map[int32]string, error) {
	result := make(map[int32]string)
	scanner := bufio.NewScanner(bytes.NewReader(data))
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" || strings.HasPrefix(line, "//") {
			continue
		}
		fields := strings.Fields(line)
		if len(fields) < 2 {
			return nil, fmt.Errorf("UI strings: missing caption")
		}
		id, err := strconv.ParseInt(fields[0], 10, 32)
		if err != nil || id <= 0 {
			return nil, fmt.Errorf("UI strings: invalid index %q", fields[0])
		}
		if _, exists := result[int32(id)]; exists {
			return nil, fmt.Errorf("UI strings: duplicate index %d", id)
		}
		result[int32(id)] = strings.ReplaceAll(strings.Join(fields[1:], " "), "_", " ")
	}
	if err := scanner.Err(); err != nil {
		return nil, err
	}
	if len(result) == 0 {
		return nil, fmt.Errorf("UI strings: empty table")
	}
	return result, nil
}
