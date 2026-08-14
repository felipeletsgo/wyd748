package main

import "testing"

func TestConfigPathFromArgs(t *testing.T) {
	tests := []struct {
		name string
		args []string
		want string
	}{
		{name: "default", want: "data/server.txt"},
		{name: "short separate", args: []string{"-config", "data/vps.txt"}, want: "data/vps.txt"},
		{name: "long separate", args: []string{"--config", "data/vps.txt"}, want: "data/vps.txt"},
		{name: "short equals", args: []string{"-config=data/vps.txt"}, want: "data/vps.txt"},
		{name: "long equals", args: []string{"--config=data/vps.txt"}, want: "data/vps.txt"},
		{name: "among flags", args: []string{"-addr", "127.0.0.1:8281", "-config", "custom.txt"}, want: "custom.txt"},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := configPathFromArgs(tt.args); got != tt.want {
				t.Fatalf("configPathFromArgs(%q) = %q; want %q", tt.args, got, tt.want)
			}
		})
	}
}
