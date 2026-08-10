// Package networkpolicy compila feeds de CIDR baixados pelo operador para a
// politica estrita consumida pelo servidor. O package nao acessa a rede.
package networkpolicy

import (
	"bufio"
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"net/netip"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"wydgo/internal/model"
)

type Import struct {
	Source     string
	Action     model.NetworkAdmissionAction
	MaxClients int
	Reason     string
	Paths      []string
}

// ReadPrefixes aceita uma lista texto (um CIDR por linha, com # para
// comentarios) ou JSON. No JSON somente campos conhecidos de feeds publicos
// sao considerados; outras strings nao viram regras por acidente.
func ReadPrefixes(path string) ([]netip.Prefix, error) {
	contents, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	trimmed := bytes.TrimSpace(contents)
	if len(trimmed) == 0 {
		return nil, fmt.Errorf("%s: arquivo vazio", path)
	}
	var prefixes []netip.Prefix
	if trimmed[0] == '{' || trimmed[0] == '[' {
		var value any
		decoder := json.NewDecoder(bytes.NewReader(trimmed))
		decoder.UseNumber()
		if err := decoder.Decode(&value); err != nil {
			return nil, fmt.Errorf("%s: JSON invalido: %w", path, err)
		}
		if err := decoder.Decode(&struct{}{}); err != io.EOF {
			if err == nil {
				err = fmt.Errorf("conteudo JSON adicional")
			}
			return nil, fmt.Errorf("%s: %w", path, err)
		}
		if err := collectJSONPrefixes(value, false, &prefixes); err != nil {
			return nil, fmt.Errorf("%s: %w", path, err)
		}
	} else {
		scanner := bufio.NewScanner(bytes.NewReader(contents))
		for line := 1; scanner.Scan(); line++ {
			value := strings.TrimSpace(strings.SplitN(scanner.Text(), "#", 2)[0])
			if value == "" {
				continue
			}
			prefix, err := parsePrefix(value)
			if err != nil {
				return nil, fmt.Errorf("%s:%d: %w", path, line, err)
			}
			prefixes = append(prefixes, prefix)
		}
		if err := scanner.Err(); err != nil {
			return nil, fmt.Errorf("%s: ler lista: %w", path, err)
		}
	}
	if len(prefixes) == 0 {
		return nil, fmt.Errorf("%s: nenhum CIDR reconhecido", path)
	}
	return uniqueSortedPrefixes(prefixes), nil
}

var singularPrefixFields = map[string]bool{
	"cidr": true, "prefix": true, "ip_prefix": true, "ipv4prefix": true,
	"ipv6_prefix": true, "ipv6prefix": true,
}

var pluralPrefixFields = map[string]bool{
	"addressprefixes": true, "cidrs": true, "prefixes": true,
}

func collectJSONPrefixes(value any, prefixContext bool, out *[]netip.Prefix) error {
	switch typed := value.(type) {
	case map[string]any:
		for key, child := range typed {
			normalized := strings.ToLower(strings.ReplaceAll(key, "-", "_"))
			if singularPrefixFields[normalized] {
				text, ok := child.(string)
				if !ok {
					return fmt.Errorf("campo %q deve ser string", key)
				}
				prefix, err := parsePrefix(text)
				if err != nil {
					return fmt.Errorf("campo %q: %w", key, err)
				}
				*out = append(*out, prefix)
				continue
			}
			if err := collectJSONPrefixes(child, pluralPrefixFields[normalized], out); err != nil {
				return err
			}
		}
	case []any:
		for _, child := range typed {
			if err := collectJSONPrefixes(child, prefixContext, out); err != nil {
				return err
			}
		}
	case string:
		if prefixContext {
			prefix, err := parsePrefix(typed)
			if err != nil {
				return err
			}
			*out = append(*out, prefix)
		}
	}
	return nil
}

func parsePrefix(value string) (netip.Prefix, error) {
	prefix, err := netip.ParsePrefix(strings.TrimSpace(value))
	if err != nil {
		return netip.Prefix{}, fmt.Errorf("CIDR %q invalido", value)
	}
	if prefix.Addr().Is4In6() {
		return netip.Prefix{}, fmt.Errorf("CIDR %q usa IPv4-mapped IPv6", value)
	}
	return prefix.Masked(), nil
}

func uniqueSortedPrefixes(prefixes []netip.Prefix) []netip.Prefix {
	seen := make(map[netip.Prefix]struct{}, len(prefixes))
	out := make([]netip.Prefix, 0, len(prefixes))
	for _, prefix := range prefixes {
		prefix = prefix.Masked()
		if _, ok := seen[prefix]; ok {
			continue
		}
		seen[prefix] = struct{}{}
		out = append(out, prefix)
	}
	sort.Slice(out, func(i, j int) bool {
		left, right := out[i], out[j]
		if left.Addr().Is4() != right.Addr().Is4() {
			return left.Addr().Is4()
		}
		if compared := left.Addr().Compare(right.Addr()); compared != 0 {
			return compared < 0
		}
		return left.Bits() < right.Bits()
	})
	return out
}

// Merge substitui somente as regras pertencentes a Import.Source. Prefixos
// identicos de outra fonte podem coexistir apenas quando a decisao e identica.
func Merge(base model.NetworkAdmissionFile, in Import) (model.NetworkAdmissionFile, error) {
	if err := base.Validate(); err != nil {
		return model.NetworkAdmissionFile{}, fmt.Errorf("politica base: %w", err)
	}
	template := model.NetworkAdmissionRule{
		CIDR: netip.MustParsePrefix("192.0.2.0/24"), Action: in.Action,
		MaxClients: in.MaxClients, Source: in.Source, Reason: in.Reason,
	}
	probe := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion,
		Rules: []model.NetworkAdmissionRule{template}}
	if err := probe.Validate(); err != nil {
		return model.NetworkAdmissionFile{}, fmt.Errorf("parametros da importacao: %w", err)
	}
	if in.Source == "" {
		return model.NetworkAdmissionFile{}, fmt.Errorf("source e obrigatorio")
	}
	if len(in.Paths) == 0 {
		return model.NetworkAdmissionFile{}, fmt.Errorf("ao menos um input e obrigatorio")
	}

	result := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion}
	byPrefix := make(map[netip.Prefix]model.NetworkAdmissionRule, len(base.Rules))
	for _, rule := range base.Rules {
		if rule.Source == in.Source {
			continue
		}
		result.Rules = append(result.Rules, rule)
		byPrefix[rule.CIDR] = rule
	}
	var imported []netip.Prefix
	for _, path := range in.Paths {
		prefixes, err := ReadPrefixes(path)
		if err != nil {
			return model.NetworkAdmissionFile{}, err
		}
		imported = append(imported, prefixes...)
	}
	for _, prefix := range uniqueSortedPrefixes(imported) {
		rule := template
		rule.CIDR = prefix
		if existing, ok := byPrefix[prefix]; ok {
			if existing.Action != rule.Action || existing.MaxClients != rule.MaxClients {
				return model.NetworkAdmissionFile{}, fmt.Errorf(
					"CIDR %s conflita com source %q", prefix, existing.Source)
			}
			continue
		}
		result.Rules = append(result.Rules, rule)
		byPrefix[prefix] = rule
	}
	sort.Slice(result.Rules, func(i, j int) bool {
		left, right := result.Rules[i].CIDR, result.Rules[j].CIDR
		if left.Addr().Is4() != right.Addr().Is4() {
			return left.Addr().Is4()
		}
		if compared := left.Addr().Compare(right.Addr()); compared != 0 {
			return compared < 0
		}
		return left.Bits() < right.Bits()
	})
	if err := result.Validate(); err != nil {
		return model.NetworkAdmissionFile{}, err
	}
	return result, nil
}

// WriteAtomic usa temp+fsync+rename no mesmo diretorio. A troca e atomica
// somente onde o sistema operacional e o filesystem garantem essa propriedade;
// ela nao promete durabilidade contra power loss alem dessas garantias.
func WriteAtomic(path string, policy model.NetworkAdmissionFile) error {
	if err := policy.Validate(); err != nil {
		return err
	}
	encoded, err := json.MarshalIndent(policy, "", "  ")
	if err != nil {
		return err
	}
	encoded = append(encoded, '\n')
	dir := filepath.Dir(path)
	tmp, err := os.CreateTemp(dir, ".network-admission-*.tmp")
	if err != nil {
		return err
	}
	tmpPath := tmp.Name()
	defer os.Remove(tmpPath)
	if err := tmp.Chmod(0o644); err != nil {
		tmp.Close()
		return err
	}
	if _, err := tmp.Write(encoded); err != nil {
		tmp.Close()
		return err
	}
	if err := tmp.Sync(); err != nil {
		tmp.Close()
		return err
	}
	if err := tmp.Close(); err != nil {
		return err
	}
	return os.Rename(tmpPath, path)
}
