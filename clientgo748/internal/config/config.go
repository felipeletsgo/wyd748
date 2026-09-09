// Package config contém a configuração validada do client Go.
//
// A configuração é deliberadamente pequena nesta fase. Endpoints, credenciais
// e chaves de assets serão adicionados quando seus contratos forem definidos;
// não devem ser lidos de globals ou usados para alterar regras do servidor.
package config

import (
	"errors"
	"fmt"
)

// Config reúne somente valores necessários para construir a fundação do
// processo. AssetRoot aponta para dados locais e não é uma fonte de autoridade
// de gameplay.
type Config struct {
	ProtocolVersion string
	AssetRoot       string
	WindowWidth     int
	WindowHeight    int
	SecureAssets    bool
}

// Default retorna uma configuração segura para desenvolvimento local. Ela não
// abre uma janela nem inicia uma conexão por si só.
func Default() Config {
	return Config{
		ProtocolVersion: "7.48",
		AssetRoot:       "assets/current",
		WindowWidth:     800,
		WindowHeight:    600,
		SecureAssets:    false,
	}
}

// Validate rejeita valores impossíveis antes de qualquer recurso externo ser
// criado. Não verifica a existência do diretório para manter a função pura e
// testável.
func (c Config) Validate() error {
	if c.ProtocolVersion != "7.48" {
		return fmt.Errorf("clientgo748: unsupported protocol version %q", c.ProtocolVersion)
	}
	if c.AssetRoot == "" {
		return errors.New("clientgo748: asset root is required")
	}
	if c.WindowWidth < 320 || c.WindowHeight < 240 {
		return fmt.Errorf("clientgo748: window is too small: %dx%d", c.WindowWidth, c.WindowHeight)
	}
	return nil
}
