package main

import (
	"flag"
	"io"
	"log"
	"net/url"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestLoadtestMainOpensIsolatedDatabasesInProvisionOnlyMode(t *testing.T) {
	databaseURL := os.Getenv("WYD_TEST_POSTGRES_URL")
	if databaseURL == "" {
		t.Skip("requires the isolated PostgreSQL test database")
	}

	parsed, err := url.Parse(databaseURL)
	if err != nil || (parsed.Scheme != "postgres" && parsed.Scheme != "postgresql") {
		t.Skip("requires PostgreSQL URL syntax for a read-only source connection")
	}
	query := parsed.Query()
	query.Set("application_name", "wyd-loadtest-source-coverage")
	parsed.RawQuery = query.Encode()
	sourceURL := parsed.String()
	if sourceURL == databaseURL {
		t.Fatal("source database URL did not get a distinct connection identity")
	}

	mapDir, err := filepath.Abs(filepath.Join("..", "..", "data", "world", "maps"))
	if err != nil {
		t.Fatal(err)
	}
	args := []string{
		"-database-url-env=WYD_TEST_LOADTEST_DATABASE_COVERAGE",
		"-source-database-url-env=WYD_TEST_LOADTEST_SOURCE_DATABASE_COVERAGE",
		"-heightmap=" + filepath.Join(mapDir, "HeightMap.dat"),
		"-attributemap=" + filepath.Join(mapDir, "AttributeMap.dat"),
		"-provision-only",
	}
	t.Setenv("WYD_TEST_LOADTEST_DATABASE_COVERAGE", databaseURL)
	t.Setenv("WYD_TEST_LOADTEST_SOURCE_DATABASE_COVERAGE", sourceURL)

	oldArgs, oldFlags, oldLog := os.Args, flag.CommandLine, log.Writer()
	t.Cleanup(func() {
		os.Args, flag.CommandLine = oldArgs, oldFlags
		log.SetOutput(oldLog)
	})
	flags := flag.NewFlagSet("loadtest-coverage-test", flag.ContinueOnError)
	flags.SetOutput(io.Discard)
	flag.CommandLine = flags
	os.Args = append([]string{"loadtest"}, args...)
	log.SetOutput(databaseRedactionWriter{writer: oldLog, secrets: []string{databaseURL, sourceURL}})
	main()
}

type databaseRedactionWriter struct {
	writer  io.Writer
	secrets []string
}

func (w databaseRedactionWriter) Write(data []byte) (int, error) {
	message := string(data)
	for _, secret := range w.secrets {
		message = strings.ReplaceAll(message, secret, "[database URL]")
	}
	if _, err := io.WriteString(w.writer, message); err != nil {
		return 0, err
	}
	return len(data), nil
}
