package main

import (
	"bufio"
	"errors"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"wydgo/internal/account"
	"wydgo/internal/store"
)

func main() {
	accountsDir := flag.String("accounts", filepath.FromSlash("data/accounts"), "diretorio dos arquivos de conta")
	username := flag.String("username", "", "nome da conta (a senha nunca deve ser passada por argumento)")
	noPause := flag.Bool("no-pause", false, "nao aguarda ENTER antes de fechar")
	flag.Parse()

	reader := bufio.NewReader(os.Stdin)
	exitCode := run(reader, *accountsDir, *username)
	if !*noPause {
		fmt.Print("\nPressione ENTER para fechar...")
		_, _ = reader.ReadString('\n')
	}
	os.Exit(exitCode)
}

func run(reader *bufio.Reader, accountsDir, presetUsername string) int {
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

	accountsPath, err := filepath.Abs(accountsDir)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Diretorio de contas invalido: %v\n", err)
		return 1
	}
	_, err = account.Create(store.NewJSONStore(accountsPath), username, password, confirmation)
	if err == nil {
		fmt.Printf("\nConta %q criada com sucesso.\n", username)
		fmt.Printf("Arquivo salvo em: %s\n", accountsPath)
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
