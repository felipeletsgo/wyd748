package main

import (
	"bufio"
	"bytes"
	"errors"
	"flag"
	"io"
	"os"
	"os/exec"
	"strings"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

type registrationStoreFixture struct {
	exists    bool
	existsErr error
	createErr error
	created   *model.Account
}

func (s *registrationStoreFixture) AccountNameExists(string) (bool, error) {
	return s.exists, s.existsErr
}

func (s *registrationStoreFixture) CreateAccount(account *model.Account) error {
	if s.createErr != nil {
		return s.createErr
	}
	copy := *account
	s.created = &copy
	return nil
}

func TestAccountCreateRunCoversRegistrationOutcomes(t *testing.T) {
	t.Run("success with prompted username", func(t *testing.T) {
		backend := &registrationStoreFixture{}
		input := bufio.NewReader(bytes.NewBufferString("NewUser\nPass123!\nPass123!\n"))
		if code := run(input, backend, ""); code != 0 {
			t.Fatalf("exit code=%d", code)
		}
		if backend.created == nil || backend.created.Name != "NewUser" || backend.created.PasswordHash == "" || len(backend.created.Chars) != 0 {
			t.Fatalf("created account=%+v", backend.created)
		}
	})

	t.Run("invalid input", func(t *testing.T) {
		backend := &registrationStoreFixture{}
		input := bufio.NewReader(bytes.NewBufferString("Pass123!\nOther123!\n"))
		if code := run(input, backend, "NewUser"); code != 1 || backend.created != nil {
			t.Fatalf("exit code=%d account=%+v", code, backend.created)
		}
	})

	t.Run("duplicate account", func(t *testing.T) {
		backend := &registrationStoreFixture{exists: true}
		input := bufio.NewReader(bytes.NewBufferString("Pass123!\nPass123!\n"))
		if code := run(input, backend, "NewUser"); code != 1 || backend.created != nil {
			t.Fatalf("exit code=%d account=%+v", code, backend.created)
		}
	})

	t.Run("store error", func(t *testing.T) {
		backend := &registrationStoreFixture{existsErr: errors.New("database unavailable")}
		input := bufio.NewReader(bytes.NewBufferString("Pass123!\nPass123!\n"))
		if code := run(input, backend, "NewUser"); code != 1 || backend.created != nil {
			t.Fatalf("exit code=%d account=%+v", code, backend.created)
		}
	})

	t.Run("account name read failure", func(t *testing.T) {
		backend := &registrationStoreFixture{}
		if code := run(bufio.NewReader(bytes.NewReader(nil)), backend, ""); code != 1 || backend.created != nil {
			t.Fatalf("exit code=%d account=%+v", code, backend.created)
		}
	})

	t.Run("password read failure", func(t *testing.T) {
		backend := &registrationStoreFixture{}
		if code := run(bufio.NewReader(bytes.NewReader(nil)), backend, "NewUser"); code != 1 || backend.created != nil {
			t.Fatalf("exit code=%d account=%+v", code, backend.created)
		}
	})

	t.Run("confirmation read failure", func(t *testing.T) {
		backend := &registrationStoreFixture{}
		input := bufio.NewReader(bytes.NewBufferString("Pass123!\n"))
		if code := run(input, backend, "NewUser"); code != 1 || backend.created != nil {
			t.Fatalf("exit code=%d account=%+v", code, backend.created)
		}
	})

	t.Run("create collision", func(t *testing.T) {
		backend := &registrationStoreFixture{createErr: store.ErrAccountExists}
		input := bufio.NewReader(bytes.NewBufferString("Pass123!\nPass123!\n"))
		if code := run(input, backend, "NewUser"); code != 1 || backend.created != nil {
			t.Fatalf("exit code=%d account=%+v", code, backend.created)
		}
	})

	t.Run("unexpected create error", func(t *testing.T) {
		backend := &registrationStoreFixture{createErr: io.ErrClosedPipe}
		input := bufio.NewReader(bytes.NewBufferString("Pass123!\nPass123!\n"))
		if code := run(input, backend, "NewUser"); code != 1 || backend.created != nil {
			t.Fatalf("exit code=%d account=%+v", code, backend.created)
		}
	})
}

func TestAccountCreateMainRejectsMissingDatabaseURL(t *testing.T) {
	cmd := exec.Command(os.Args[0], "-test.run=^TestAccountCreateMainHelperProcess$")
	cmd.Env = append(os.Environ(), "WYD_ACCOUNT_CREATE_MAIN_HELPER=1")
	output, err := cmd.CombinedOutput()
	if err == nil || !strings.Contains(string(output), "environment variable WYD_ACCOUNT_CREATE_TEST_DB is empty") {
		t.Fatalf("account-create accepted a missing database URL: err=%v output=%s", err, output)
	}
}

func TestAccountCreateMainRejectsMalformedDatabaseURL(t *testing.T) {
	cmd := exec.Command(os.Args[0], "-test.run=^TestAccountCreateMainHelperProcess$")
	cmd.Env = append(os.Environ(), "WYD_ACCOUNT_CREATE_MAIN_HELPER=1",
		"WYD_ACCOUNT_CREATE_TEST_DB_VALUE=invalid://database")
	output, err := cmd.CombinedOutput()
	if err == nil || !strings.Contains(string(output), "Could not connect to PostgreSQL") {
		t.Fatalf("account-create accepted a malformed database URL: err=%v output=%s", err, output)
	}
}

func TestAccountCreateMainHelperProcess(t *testing.T) {
	if os.Getenv("WYD_ACCOUNT_CREATE_MAIN_HELPER") != "1" {
		return
	}
	if value, ok := os.LookupEnv("WYD_ACCOUNT_CREATE_TEST_DB_VALUE"); ok {
		_ = os.Setenv("WYD_ACCOUNT_CREATE_TEST_DB", value)
	} else {
		_ = os.Unsetenv("WYD_ACCOUNT_CREATE_TEST_DB")
	}
	os.Args = []string{"account-create", "-database-url-env=WYD_ACCOUNT_CREATE_TEST_DB"}
	flag.CommandLine = flag.NewFlagSet("account-create", flag.ExitOnError)
	main()
}
