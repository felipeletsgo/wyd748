// Command network-admission compila listas CIDR locais para a politica usada
// pelo servidor. Downloads continuam sendo uma operacao externa do operador.
package main

import (
	"flag"
	"fmt"
	"io"
	"os"
	"strings"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/networkpolicy"
)

type stringList []string

func (values *stringList) String() string { return strings.Join(*values, ",") }
func (values *stringList) Set(value string) error {
	value = strings.TrimSpace(value)
	if value == "" {
		return fmt.Errorf("input vazio")
	}
	*values = append(*values, value)
	return nil
}

func main() {
	if err := run(os.Args[1:], os.Stdout); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

func run(args []string, stdout io.Writer) error {
	flags := flag.NewFlagSet("network-admission", flag.ContinueOnError)
	flags.SetOutput(io.Discard)
	var inputs stringList
	basePath := flags.String("base", "data/network_admission.json", "politica existente a preservar")
	outPath := flags.String("out", "data/network_admission.json", "politica compilada")
	source := flags.String("source", "", "identificador estavel da fonte (ex.: aws)")
	action := flags.String("action", "deny", "allow, deny ou limit")
	maxClients := flags.Int("max-clients", 0, "teto por rede quando action=limit")
	reason := flags.String("reason", "", "motivo interno da regra")
	dryRun := flags.Bool("dry-run", false, "valida e mostra o total sem gravar")
	flags.Var(&inputs, "input", "arquivo texto/JSON de CIDRs; pode ser repetido")
	if err := flags.Parse(args); err != nil {
		return err
	}

	base, err := data.LoadNetworkAdmission(*basePath)
	if err != nil {
		return fmt.Errorf("carregar base: %w", err)
	}
	policy, err := networkpolicy.Merge(base, networkpolicy.Import{
		Source:     strings.TrimSpace(*source),
		Action:     model.NetworkAdmissionAction(strings.ToLower(strings.TrimSpace(*action))),
		MaxClients: *maxClients, Reason: strings.TrimSpace(*reason), Paths: inputs,
	})
	if err != nil {
		return fmt.Errorf("compilar politica: %w", err)
	}
	if !*dryRun {
		if err := networkpolicy.WriteAtomic(*outPath, policy); err != nil {
			return fmt.Errorf("gravar %s: %w", *outPath, err)
		}
	}
	fmt.Fprintf(stdout, "politica valida: %d regra(s), source=%q substituida, dry-run=%t\n",
		len(policy.Rules), *source, *dryRun)
	return nil
}
