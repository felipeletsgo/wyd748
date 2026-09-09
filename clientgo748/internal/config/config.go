// Package config contém a configuração validada do client Go.
//
// A configuração é deliberadamente pequena nesta fase. Endpoints, credenciais
// e chaves de assets serão adicionados quando seus contratos forem definidos;
// não devem ser lidos de globals ou usados para alterar regras do servidor.
package config

import (
	"errors"
	"fmt"
	"net"
	"os"
	"strconv"
)

// Config reúne somente valores necessários para construir a fundação do
// processo. AssetRoot aponta para dados locais e não é uma fonte de autoridade
// de gameplay.
type Config struct {
	ProtocolVersion string
	AssetRoot       string
	// ServerAddress is the default endpoint shown by the server-selection
	// scene. The transport remains closed until the player confirms CONNECT.
	ServerAddress string
	WindowWidth   int
	WindowHeight  int
	SecureAssets  bool
}

// Default retorna uma configuração segura para desenvolvimento local. Ela não
// abre uma janela nem inicia uma conexão por si só.
func Default() Config {
	return Config{
		ProtocolVersion: "7.48",
		AssetRoot:       "assets/current",
		ServerAddress:   "127.0.0.1:8281",
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
	if err := ValidateServerAddress(c.ServerAddress); err != nil {
		return err
	}
	if c.WindowWidth < 320 || c.WindowHeight < 240 {
		return fmt.Errorf("clientgo748: window is too small: %dx%d", c.WindowWidth, c.WindowHeight)
	}
	return nil
}

// ValidateServerAddress checks the host:port boundary before the transport
// is created. An empty address is valid while callers use a configured default.
func ValidateServerAddress(address string) error {
	if address == "" {
		return nil
	}
	host, port, err := net.SplitHostPort(address)
	if err != nil || host == "" || port == "" {
		return fmt.Errorf("clientgo748: invalid server address %q", address)
	}
	number, err := strconv.Atoi(port)
	if err != nil || number < 1 || number > 65535 {
		return fmt.Errorf("clientgo748: invalid server port in %q", address)
	}
	return nil
}

// ServerAddressFromEnv returns an optional endpoint override. An empty value
// keeps the configured default; malformed values fail before window creation.
func ServerAddressFromEnv(defaultAddress string) (string, error) {
	address := os.Getenv("WYD_SERVER_ADDRESS")
	if address == "" {
		return "", nil
	}
	if err := ValidateServerAddress(address); err != nil {
		return "", err
	}
	return address, nil
}
