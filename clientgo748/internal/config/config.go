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
	"strings"
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

// ServerEntry is one endpoint offered by the modern server-selection screen.
// The client treats the list as presentation/configuration only; the server
// remains authoritative after the selected endpoint is connected.
type ServerEntry struct {
	Name    string
	Channel string
	Address string
}

// ServerEntriesFromEnv loads an optional semicolon-separated list using the
// format "Group|host:port;Another Group|host:port" or
// "Group|Channel|host:port". The two-field form remains compatible and uses
// "Channel 1" as its single channel. Keeping this
// outside the executable lets operators publish several channels without
// rebuilding the client. When unset, the configured default remains the one
// safe local entry used by development builds.
func ServerEntriesFromEnv(defaultAddress string) ([]ServerEntry, error) {
	raw := strings.TrimSpace(os.Getenv("WYD_SERVER_LIST"))
	if raw == "" {
		return []ServerEntry{{Name: "WYD-Go Server", Channel: "Channel 1", Address: defaultAddress}}, nil
	}
	parts := strings.Split(raw, ";")
	if len(parts) > 32 {
		return nil, errors.New("clientgo748: server list contains more than 32 entries")
	}
	entries := make([]ServerEntry, 0, len(parts))
	for index, part := range parts {
		fields := strings.Split(part, "|")
		if len(fields) != 2 && len(fields) != 3 {
			return nil, fmt.Errorf("clientgo748: server list entry %d must use group|host:port or group|channel|host:port", index+1)
		}
		name := strings.TrimSpace(fields[0])
		channel := "Channel 1"
		addressField := fields[1]
		if len(fields) == 3 {
			channel = strings.TrimSpace(fields[1])
			addressField = fields[2]
		}
		if name == "" || len([]rune(name)) > 64 {
			return nil, fmt.Errorf("clientgo748: server list entry %d has an invalid name", index+1)
		}
		if channel == "" || len([]rune(channel)) > 64 {
			return nil, fmt.Errorf("clientgo748: server list entry %d has an invalid channel", index+1)
		}
		address := strings.TrimSpace(addressField)
		if err := ValidateServerAddress(address); err != nil {
			return nil, fmt.Errorf("clientgo748: server list entry %d: %w", index+1, err)
		}
		entries = append(entries, ServerEntry{Name: name, Channel: channel, Address: address})
	}
	if len(entries) == 0 {
		return nil, errors.New("clientgo748: server list is empty")
	}
	return entries, nil
}
