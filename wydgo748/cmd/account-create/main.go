package main

import (
	"bufio"
	"context"
	"errors"
	"flag"
	"fmt"
	"os"
	"strings"

	"wydgo/internal/account"
	"wydgo/internal/store"
)

func main() {
	databaseURLEnv := flag.String("database-url-env", "WYD_DATABASE_URL",
		"variavel de ambiente com a URL PostgreSQL")
	username := flag.String("username", "", "nome da conta (a senha nunca deve ser passada por argumento)")
	noPause := flag.Bool("no-pause", false, "nao aguarda ENTER antes de fechar")
	flag.Parse()

	reader := bufio.NewReader(os.Stdin)
	databaseURL := os.Getenv(*databaseURLEnv)
	if databaseURL == "" {
		fmt.Fprintf(os.Stderr, "Erro: variavel %s esta vazia.\n", *databaseURLEnv)
		os.Exit(1)
	}
	st, err := store.NewPostgresStore(context.Background(), store.PostgresConfig{
		URL: databaseURL, MaxConns: 2,
	})
	if err != nil {
		fmt.Fprintf(os.Stderr, "Erro ao conectar ao PostgreSQL: %v\n", err)
		os.Exit(1)
	}
	defer st.Close()
	exitCode := run(reader, st, *username)
	if !*noPause {
		fmt.Print("\nPressione ENTER para fechar...")
		_, _ = reader.ReadString('\n')
	}
	os.Exit(exitCode)
}

func run(reader *bufio.Reader, st account.RegistrationStore, presetUsername string) int {
	fmt.Println("WYD-Go - Criador de contas")
	fmt.Println("A senha sera armazenada somente como hash seguro.")
	fmt.Println()

	username := strings.TrimSpace(presetUsername)
	if username == "" {
		var err error
		username, err = readLine(reader, "Nome da conta: ")
		if err != nil {
			fmt.Fprintf(os.Stderr, "Erro ao ler o nome da conta: %v\n", err)
			return 1
		}
		username = strings.TrimSpace(username)
	}

	password, err := readPassword(reader, "Senha: ")
	if err != nil {
		fmt.Fprintf(os.Stderr, "Erro ao ler a senha: %v\n", err)
		return 1
	}
	confirmation, err := readPassword(reader, "Confirme a senha: ")
	if err != nil {
		fmt.Fprintf(os.Stderr, "Erro ao confirmar a senha: %v\n", err)
		return 1
	}

	_, err = account.Create(st, username, password, confirmation)
	if err == nil {
		fmt.Printf("\nConta %q criada com sucesso.\n", username)
		fmt.Println("Conta confirmada no PostgreSQL.")
		return 0
	}

	var validationErr *account.ValidationError
	switch {
	case errors.As(err, &validationErr):
		fmt.Fprintf(os.Stderr, "\nDados invalidos: %s\n", validationErr.Message)
	case errors.Is(err, account.ErrUsernameUnavailable):
		fmt.Fprintln(os.Stderr, "\nEssa conta ja existe.")
	default:
		fmt.Fprintf(os.Stderr, "\nNao foi possivel criar a conta: %v\n", err)
	}
	return 1
}

func readLine(reader *bufio.Reader, prompt string) (string, error) {
	fmt.Print(prompt)
	line, err := reader.ReadString('\n')
	if err != nil && len(line) == 0 {
		return "", err
	}
	return strings.TrimRight(line, "\r\n"), nil
}
