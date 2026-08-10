package data

import (
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestLoadNetworkAdmissionStrict(t *testing.T) {
	path := filepath.Join(t.TempDir(), "network_admission.json")
	contents := `{"version":1,"rules":[
        {"cidr":"203.0.113.0/24","action":"deny","reason":"hosting"},
        {"cidr":"2001:db8::/32","action":"limit","maxClients":1}
    ]}`
	if err := os.WriteFile(path, []byte(contents), 0o600); err != nil {
		t.Fatal(err)
	}
	file, err := LoadNetworkAdmission(path)
	if err != nil {
		t.Fatal(err)
	}
	if file.Version != model.NetworkAdmissionVersion || len(file.Rules) != 2 ||
		file.Rules[1].MaxClients != 1 {
		t.Fatalf("politica carregada incorretamente: %+v", file)
	}
}

func TestLoadNetworkAdmissionRejectsUnknownAndTrailingData(t *testing.T) {
	for _, contents := range []string{
		`{"version":1,"unknown":true,"rules":[]}`,
		`{"version":1,"rules":[]} {}`,
		`{"version":1,"rules":[{"cidr":"203.0.113.1/24","action":"deny"}]}`,
	} {
		path := filepath.Join(t.TempDir(), "network_admission.json")
		if err := os.WriteFile(path, []byte(contents), 0o600); err != nil {
			t.Fatal(err)
		}
		if _, err := LoadNetworkAdmission(path); err == nil {
			t.Fatalf("politica invalida aceita: %s", contents)
		}
	}
}

func TestRepositoryNetworkAdmissionFileIsValid(t *testing.T) {
	file, err := LoadNetworkAdmission("../../data/network_admission.json")
	if err != nil {
		t.Fatal(err)
	}
	if file.Version != model.NetworkAdmissionVersion {
		t.Fatalf("version=%d", file.Version)
	}
}
