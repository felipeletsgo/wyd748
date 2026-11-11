package main

import (
	"bytes"
	"flag"
	"net"
	"os"
	"os/exec"
	"os/user"
	"path/filepath"
	"strconv"
	"strings"
	"testing"
	"time"
)

func TestSystemdListenerRequiresTheExpectedActivationEnvironment(t *testing.T) {
	oldPID, hadPID := os.LookupEnv("LISTEN_PID")
	oldFDs, hadFDs := os.LookupEnv("LISTEN_FDS")
	t.Cleanup(func() {
		if hadPID {
			_ = os.Setenv("LISTEN_PID", oldPID)
		} else {
			_ = os.Unsetenv("LISTEN_PID")
		}
		if hadFDs {
			_ = os.Setenv("LISTEN_FDS", oldFDs)
		} else {
			_ = os.Unsetenv("LISTEN_FDS")
		}
	})
	for _, tc := range []struct{ pid, fds string }{
		{pid: "0", fds: "1"},
		{pid: strconv.Itoa(os.Getpid()), fds: "0"},
		{pid: strconv.Itoa(os.Getpid()), fds: "2"},
	} {
		_ = os.Setenv("LISTEN_PID", tc.pid)
		_ = os.Setenv("LISTEN_FDS", tc.fds)
		listener, err := systemdListener()
		if listener != nil || err != nil {
			t.Fatalf("LISTEN_PID=%s LISTEN_FDS=%s: listener=%v err=%v", tc.pid, tc.fds, listener, err)
		}
	}
}

func TestLogReaderMainRejectsUnknownAllowedUser(t *testing.T) {
	cmd := exec.Command(os.Args[0], "-test.run=^TestLogReaderMainHelperProcess$")
	cmd.Env = append(os.Environ(), "WYD_LOG_READER_HELPER=1")
	output, err := cmd.CombinedOutput()
	if err == nil || !strings.Contains(string(output), "allowed user") {
		t.Fatalf("log-reader accepted an unknown allowed user: err=%v output=%s", err, output)
	}
}

func TestLogReaderMainRejectsMissingAndUnusableSocketPaths(t *testing.T) {
	current, err := user.Current()
	if err != nil {
		t.Fatal(err)
	}
	if _, err := strconv.ParseUint(current.Uid, 10, 32); err != nil {
		t.Skip("the host account has no numeric UID for the log-reader authorization path")
	}
	missingParent := filepath.Join(t.TempDir(), "missing", "log-reader.sock")
	for _, test := range []struct {
		name, socketPath, want string
	}{
		{name: "no activation or socket path", want: "no systemd socket and no -socket path"},
		{name: "unusable socket path", socketPath: missingParent, want: "listen:"},
	} {
		t.Run(test.name, func(t *testing.T) {
			cmd := exec.Command(os.Args[0], "-test.run=^TestLogReaderMainHelperProcess$")
			cmd.Env = append(os.Environ(), "WYD_LOG_READER_HELPER=1",
				"WYD_LOG_READER_ALLOW_USER="+current.Username,
				"WYD_LOG_READER_SOCKET_PATH="+test.socketPath)
			output, err := cmd.CombinedOutput()
			if err == nil || !strings.Contains(string(output), test.want) {
				t.Fatalf("log-reader accepted %s: err=%v output=%s", test.name, err, output)
			}
		})
	}
}

func TestLogReaderMainReportsNonNumericAllowedUserID(t *testing.T) {
	current, err := user.Current()
	if err != nil {
		t.Fatal(err)
	}
	if _, err := strconv.ParseUint(current.Uid, 10, 32); err == nil {
		t.Skip("the host account has a numeric UID")
	}
	cmd := exec.Command(os.Args[0], "-test.run=^TestLogReaderMainHelperProcess$")
	cmd.Env = append(os.Environ(), "WYD_LOG_READER_HELPER=1",
		"WYD_LOG_READER_ALLOW_USER="+current.Username,
		"WYD_LOG_READER_SOCKET_PATH=")
	output, err := cmd.CombinedOutput()
	if err == nil || !strings.Contains(string(output), "allowed user ID") {
		t.Fatalf("log-reader accepted a nonnumeric UID: err=%v output=%s", err, output)
	}
}

func TestLogReaderMainStartsOnConfiguredUnixSocket(t *testing.T) {
	current, err := user.Current()
	if err != nil {
		t.Fatal(err)
	}
	if _, err := strconv.ParseUint(current.Uid, 10, 32); err != nil {
		t.Skip("the host account has no numeric UID for the log-reader authorization path")
	}
	dir, err := os.MkdirTemp(os.TempDir(), "wyd-lr-")
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = os.RemoveAll(dir) })
	socketPath := filepath.Join(dir, "reader.sock")
	cmd := exec.Command(os.Args[0], "-test.run=^TestLogReaderMainHelperProcess$")
	cmd.Env = append(os.Environ(), "WYD_LOG_READER_HELPER=1",
		"WYD_LOG_READER_ALLOW_USER="+current.Username,
		"WYD_LOG_READER_SOCKET_PATH="+socketPath)
	var stdout, stderr bytes.Buffer
	cmd.Stdout, cmd.Stderr = &stdout, &stderr
	if err := cmd.Start(); err != nil {
		t.Fatal(err)
	}
	stopped := false
	stopProcess := func() {
		if stopped {
			return
		}
		stopped = true
		_ = cmd.Process.Kill()
		_ = cmd.Wait()
	}
	t.Cleanup(stopProcess)

	deadline := time.Now().Add(5 * time.Second)
	for {
		if _, err := os.Stat(socketPath); err == nil {
			break
		}
		if time.Now().After(deadline) {
			stopProcess()
			t.Fatalf("log-reader did not create its socket: stdout=%s stderr=%s", stdout.String(), stderr.String())
		}
		time.Sleep(10 * time.Millisecond)
	}
	conn, err := net.DialTimeout("unix", socketPath, time.Second)
	if err != nil {
		stopProcess()
		t.Fatalf("log-reader socket did not accept a local connection: %v; stderr=%s", err, stderr.String())
	}
	_ = conn.Close()
}

func TestLogReaderMainHelperProcess(t *testing.T) {
	if os.Getenv("WYD_LOG_READER_HELPER") != "1" {
		return
	}
	flag.CommandLine = flag.NewFlagSet("log-reader", flag.ExitOnError)
	allowUser := os.Getenv("WYD_LOG_READER_ALLOW_USER")
	if allowUser == "" {
		allowUser = "wyd-go-test-user-does-not-exist-8f36b"
	}
	os.Args = []string{"log-reader", "-allow-user", allowUser}
	if socketPath := os.Getenv("WYD_LOG_READER_SOCKET_PATH"); socketPath != "" {
		os.Args = append(os.Args, "-socket", socketPath)
	}
	main()
}
