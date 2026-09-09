// Command wydclient é o ponto de entrada do futuro client Go 7.48.
//
// Nesta fase ele apenas confirma que a fundação foi compilada. Não abre uma
// janela, não conecta ao servidor e não tenta carregar assets até que essas
// fronteiras tenham implementação e validação próprias.
package main

import (
	"fmt"

	"wydgo/clientgo748/internal/config"
	"wydgo/clientgo748/internal/lifecycle"
)

func main() {
	cfg := config.Default()
	if err := cfg.Validate(); err != nil {
		panic(err)
	}

	var machine lifecycle.Machine
	fmt.Printf("WYD Client Go %s foundation (%dx%d)\n", cfg.ProtocolVersion, cfg.WindowWidth, cfg.WindowHeight)
	fmt.Printf("initial lifecycle state: %d\n", machine.State())
}
