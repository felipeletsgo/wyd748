package store

import (
	"context"
	"fmt"
	"regexp"
	"strings"
)

var loadtestPrefixRE = regexp.MustCompile(`^[a-z]{2,8}$`)

// ResetLoadtestAccounts remove somente contas cujo nome comeca pelo prefixo
// alfabetico informado. O metodo existe para a base descartavel de carga; a
// validacao impede transformar um erro de configuracao em um DELETE amplo.
func (s *PostgresStore) ResetLoadtestAccounts(ctx context.Context, prefix string) (int64, error) {
	if s.readOnly {
		return 0, fmt.Errorf("store: PostgreSQL somente leitura")
	}
	prefix = strings.ToLower(strings.TrimSpace(prefix))
	if !loadtestPrefixRE.MatchString(prefix) || !strings.HasPrefix(prefix, "bot") {
		return 0, fmt.Errorf("prefixo de loadtest invalido %q", prefix)
	}
	if ctx == nil {
		ctx = context.Background()
	}
	tag, err := s.pool.Exec(ctx,
		`DELETE FROM accounts WHERE name_key LIKE $1 AND name_key ~ $2`,
		prefix+"%", "^"+prefix+`[0-9]{4}$`)
	if err != nil {
		return 0, err
	}
	return tag.RowsAffected(), nil
}
