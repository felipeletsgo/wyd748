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
		"environment variable containing the PostgreSQL URL")
	username := flag.String("username", "", "account name (never pass the password as an argument)")
	noPause := flag.Bool("no-pause", false, "do not wait for ENTER before closing")
	flag.Parse()

	reader := bufio.NewReader(os.Stdin)
	databaseURL := os.Getenv(*databaseURLEnv)
	if databaseURL == "" {
		fmt.Fprintf(os.Stderr, "Error: environment variable %s is empty.\n", *databaseURLEnv)
		os.Exit(1)
	}
	st, err := store.NewPostgresStore(context.Background(), store.PostgresConfig{
		URL: databaseURL, MaxConns: 2,
	})
	if err != nil {
		fmt.Fprintf(os.Stderr, "Could not connect to PostgreSQL: %v\n", err)
		os.Exit(1)
	}
	defer st.Close()
	exitCode := run(reader, st, *username)
	if !*noPause {
		fmt.Print("\nPress ENTER to close...")
		_, _ = reader.ReadString('\n')
	}
	os.Exit(exitCode)
}

func run(reader *bufio.Reader, st account.RegistrationStore, presetUsername string) int {
	fmt.Println("WYD-Go - Account creation")
	fmt.Println("The password will be stored only as a secure hash.")
	fmt.Println()

	username := strings.TrimSpace(presetUsername)
	if username == "" {
		var err error
		username, err = readLine(reader, "Account name: ")
		if err != nil {
			fmt.Fprintf(os.Stderr, "Could not read the account name: %v\n", err)
			return 1
		}
		username = strings.TrimSpace(username)
	}

	password, err := readPassword(reader, "Password: ")
	if err != nil {
		fmt.Fprintf(os.Stderr, "Could not read the password: %v\n", err)
		return 1
	}
	confirmation, err := readPassword(reader, "Confirm password: ")
	if err != nil {
		fmt.Fprintf(os.Stderr, "Could not read the password confirmation: %v\n", err)
		return 1
	}

	_, err = account.Create(st, username, password, confirmation)
	if err == nil {
		fmt.Printf("\nAccount %q created successfully.\n", username)
		fmt.Println("Account confirmed in PostgreSQL.")
		return 0
	}

	var validationErr *account.ValidationError
	switch {
	case errors.As(err, &validationErr):
		fmt.Fprintf(os.Stderr, "\nInvalid input: %s\n", validationErr.Message)
	case errors.Is(err, account.ErrUsernameUnavailable):
		fmt.Fprintln(os.Stderr, "\nThat account already exists.")
	default:
		fmt.Fprintf(os.Stderr, "\nCould not create the account: %v\n", err)
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
