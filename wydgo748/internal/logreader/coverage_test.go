package logreader

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"io"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
	"time"
)

type journalProcessStub struct {
	output   []byte
	pipeErr  error
	startErr error
	waitErr  error
	started  bool
	waited   bool
}

func (p *journalProcessStub) StdoutPipe() (io.ReadCloser, error) {
	if p.pipeErr != nil {
		return nil, p.pipeErr
	}
	return io.NopCloser(bytes.NewReader(p.output)), nil
}

func (p *journalProcessStub) Start() error {
	p.started = true
	return p.startErr
}

func (p *journalProcessStub) Wait() error {
	p.waited = true
	return p.waitErr
}

func TestJournalctlReadPagesAndDecodesOutput(t *testing.T) {
	r := validRequest()
	r.Cursor = "at-cursor"
	output := strings.Join([]string{
		"not json",
		`{"__CURSOR":""}`,
		`{"__CURSOR":"at-cursor","__REALTIME_TIMESTAMP":"1","PRIORITY":"1","MESSAGE":"cursor row"}`,
		`{"__CURSOR":"c1","__REALTIME_TIMESTAMP":"bad","PRIORITY":"bad","MESSAGE":[72,105,300,-1]}`,
		`{"__CURSOR":"c2","__REALTIME_TIMESTAMP":"2000000","PRIORITY":"4","MESSAGE":"second"}`,
		`{"__CURSOR":"c3","__REALTIME_TIMESTAMP":"3000000","PRIORITY":"5","MESSAGE":"third"}`,
	}, "\n") + "\n"
	process := &journalProcessStub{output: []byte(output), waitErr: errors.New("killed after scan limit")}
	var gotPath string
	var gotArgs []string
	entries, more, err := (Journalctl{}).read(context.Background(), "wyd-game*.service", r, 2,
		func(_ context.Context, path string, args ...string) journalProcess {
			gotPath, gotArgs = path, args
			return process
		})
	if err != nil || !more || len(entries) != 2 {
		t.Fatalf("entries=%+v more=%t err=%v", entries, more, err)
	}
	if gotPath != "journalctl" || len(gotArgs) == 0 || gotArgs[0] != "--unit=wyd-game*.service" {
		t.Fatalf("command path/args: %q %q", gotPath, gotArgs)
	}
	if entries[0].Cursor != "c1" || entries[0].Message != "Hi" || entries[0].Priority != 0 || !entries[0].Time.Equal(time.Unix(0, 0).UTC()) {
		t.Fatalf("first entry: %+v", entries[0])
	}
	if entries[1].Cursor != "c2" || entries[1].Message != "second" || entries[1].Priority != 4 || !entries[1].Time.Equal(time.Unix(2, 0).UTC()) {
		t.Fatalf("second entry: %+v", entries[1])
	}
	if !process.started || !process.waited {
		t.Fatalf("process lifecycle: started=%t waited=%t", process.started, process.waited)
	}
}

func TestJournalctlReadProcessAndOutputFailures(t *testing.T) {
	r := validRequest()
	newStub := func(process *journalProcessStub) func(context.Context, string, ...string) journalProcess {
		return func(context.Context, string, ...string) journalProcess { return process }
	}
	t.Run("stdout pipe", func(t *testing.T) {
		want := errors.New("pipe failed")
		process := &journalProcessStub{pipeErr: want}
		_, _, err := (Journalctl{}).read(context.Background(), "unit", r, 1, newStub(process))
		if !errors.Is(err, want) || process.started {
			t.Fatalf("err=%v started=%t", err, process.started)
		}
	})
	t.Run("start", func(t *testing.T) {
		want := errors.New("start failed")
		process := &journalProcessStub{startErr: want}
		_, _, err := (Journalctl{}).read(context.Background(), "unit", r, 1, newStub(process))
		if !errors.Is(err, want) || !process.started || process.waited {
			t.Fatalf("err=%v started=%t waited=%t", err, process.started, process.waited)
		}
	})
	t.Run("scanner limit", func(t *testing.T) {
		output := append(bytes.Repeat([]byte{'x'}, 1024*1024+1), '\n')
		process := &journalProcessStub{output: output}
		_, _, err := (Journalctl{}).read(context.Background(), "unit", r, 1, newStub(process))
		if err == nil || !strings.Contains(err.Error(), "token too long") || !process.waited {
			t.Fatalf("err=%v waited=%t", err, process.waited)
		}
	})
	t.Run("cursor expired", func(t *testing.T) {
		request := r
		request.Cursor = "expired"
		process := &journalProcessStub{waitErr: &exec.ExitError{}}
		_, _, err := (Journalctl{}).read(context.Background(), "unit", request, 1, newStub(process))
		if !errors.Is(err, ErrCursorExpired) {
			t.Fatalf("err=%v", err)
		}
	})
	t.Run("journal command failed", func(t *testing.T) {
		want := errors.New("journalctl failed")
		process := &journalProcessStub{waitErr: want}
		_, _, err := (Journalctl{}).read(context.Background(), "unit", r, 1, newStub(process))
		if !errors.Is(err, want) {
			t.Fatalf("err=%v", err)
		}
	})
	t.Run("real command cannot start", func(t *testing.T) {
		path := filepath.Join(t.TempDir(), "missing-journalctl")
		_, _, err := (Journalctl{Path: path}).Read(context.Background(), "unit", r, 1)
		if err == nil {
			t.Fatal("expected process start failure")
		}
	})
}

func TestRequestValidationRejectsUnknownKind(t *testing.T) {
	r := validRequest()
	r.Kind = "shell"
	if err := r.Validate(); err == nil || err.Error() != "invalid_request" {
		t.Fatalf("err=%v", err)
	}
}

func TestPeerUIDUnavailableOutsideLinux(t *testing.T) {
	if _, err := PeerUID(nil); err == nil {
		t.Fatal("expected platform limitation")
	}
}

func TestReadReceiptRejectsEmptyAndMalformedStatus(t *testing.T) {
	dir := t.TempDir()
	for name, contents := range map[string]string{
		"malformed": `{`,
		"zero time": `{"at":"0001-01-01T00:00:00Z","archive":"dump"}`,
	} {
		path := filepath.Join(dir, name+".json")
		if err := os.WriteFile(path, []byte(contents), 0o600); err != nil {
			t.Fatal(err)
		}
		if got := readReceipt(path); got != nil {
			t.Fatalf("%s: %+v", name, got)
		}
	}
	if got := readReceipt(""); got != nil {
		t.Fatalf("empty path: %+v", got)
	}
}

func TestServerHandlesMalformedRequestsAndJournalFailures(t *testing.T) {
	client := serveTest(t, &Server{Journal: &fakeJournal{}, Authorize: func(net.Conn) error { return nil }})
	if got := rawLogReaderRequest(t, client.Path, "{]"); got.Error != "invalid_request" {
		t.Fatalf("malformed request: %+v", got)
	}
	if got := rawLogReaderRequest(t, client.Path, `{"version":2,"kind":"logs"}`); got.Error != "invalid_request" {
		t.Fatalf("invalid request: %+v", got)
	}

	unavailable := serveTest(t, &Server{Journal: &fakeJournal{err: errors.New("journal down")}, Authorize: func(net.Conn) error { return nil }})
	response, err := unavailable.Do(context.Background(), validRequest())
	if err != nil || response.Error != "journal_unavailable" {
		t.Fatalf("response=%+v err=%v", response, err)
	}

	forbidden := serveTest(t, &Server{Journal: &fakeJournal{}})
	response, err = forbidden.Do(context.Background(), validRequest())
	if err != nil || response.Error != "forbidden" {
		t.Fatalf("response=%+v err=%v", response, err)
	}
}

func TestServerHandlesReplyWriteFailure(t *testing.T) {
	payload, err := json.Marshal(validRequest())
	if err != nil {
		t.Fatal(err)
	}
	conn := &writeFailConn{
		reader:   bytes.NewReader(append(payload, '\n')),
		writeErr: errors.New("peer closed"),
	}
	server := &Server{Journal: &fakeJournal{}, Authorize: func(net.Conn) error { return nil }}
	server.handle(conn)
	if conn.writes == 0 {
		t.Fatal("server did not attempt to send its response")
	}
}

type writeFailConn struct {
	reader   *bytes.Reader
	writeErr error
	writes   int
}

func (c *writeFailConn) Read(p []byte) (int, error)     { return c.reader.Read(p) }
func (c *writeFailConn) Write([]byte) (int, error)      { c.writes++; return 0, c.writeErr }
func (*writeFailConn) Close() error                     { return nil }
func (*writeFailConn) LocalAddr() net.Addr              { return &net.UnixAddr{Name: "local", Net: "unix"} }
func (*writeFailConn) RemoteAddr() net.Addr             { return &net.UnixAddr{Name: "remote", Net: "unix"} }
func (*writeFailConn) SetDeadline(time.Time) error      { return nil }
func (*writeFailConn) SetReadDeadline(time.Time) error  { return nil }
func (*writeFailConn) SetWriteDeadline(time.Time) error { return nil }

func TestServerContinuesAfterShortScanWithMoreEntries(t *testing.T) {
	journal := shortScanJournal{}
	client := serveTest(t, &Server{Journal: journal, Authorize: func(net.Conn) error { return nil }})
	response, err := client.Do(context.Background(), validRequest())
	if err != nil || len(response.Entries) != 1 || !response.Truncated || response.NextCursor != "last-scanned" {
		t.Fatalf("response=%+v err=%v", response, err)
	}
}

type shortScanJournal struct{}

func (shortScanJournal) Read(context.Context, string, Request, int) ([]Entry, bool, error) {
	return []Entry{{Cursor: "last-scanned", Message: "one"}}, true, nil
}

func TestServeRejectsConcurrentRequestAndReturnsAcceptErrors(t *testing.T) {
	journal := &blockingJournal{started: make(chan struct{}), release: make(chan struct{})}
	client := serveTest(t, &Server{Journal: journal, Authorize: func(net.Conn) error { return nil }, MaxConcurrent: 1})
	first := make(chan error, 1)
	go func() {
		response, err := client.Do(context.Background(), validRequest())
		if err == nil && response.Error != "" {
			err = errors.New(response.Error)
		}
		first <- err
	}()
	select {
	case <-journal.started:
	case <-time.After(2 * time.Second):
		close(journal.release)
		t.Fatal("first request did not enter the journal")
	}
	response, err := client.Do(context.Background(), validRequest())
	if err != nil || response.Error != "busy" {
		close(journal.release)
		t.Fatalf("response=%+v err=%v", response, err)
	}
	close(journal.release)
	select {
	case err := <-first:
		if err != nil {
			t.Fatal(err)
		}
	case <-time.After(2 * time.Second):
		t.Fatal("first request did not finish")
	}

	want := errors.New("accept failed")
	if err := (&Server{}).Serve(failingListener{err: want}); !errors.Is(err, want) {
		t.Fatalf("Serve err=%v", err)
	}
	if err := (&Server{}).Serve(failingListener{err: net.ErrClosed}); err != nil {
		t.Fatalf("closed listener: %v", err)
	}
}

type blockingJournal struct {
	started chan struct{}
	release chan struct{}
}

func (j *blockingJournal) Read(context.Context, string, Request, int) ([]Entry, bool, error) {
	close(j.started)
	<-j.release
	return []Entry{}, false, nil
}

type failingListener struct{ err error }

func (l failingListener) Accept() (net.Conn, error) { return nil, l.err }
func (failingListener) Close() error                { return nil }
func (failingListener) Addr() net.Addr              { return &net.UnixAddr{Name: "test", Net: "unix"} }

func TestClientRejectsMissingPathInvalidRequestAndBadResponses(t *testing.T) {
	if _, err := (Client{}).Do(context.Background(), validRequest()); !errors.Is(err, ErrUnavailable) {
		t.Fatalf("missing path err=%v", err)
	}
	if _, err := (Client{Path: "unused"}).Do(context.Background(), Request{Version: 2}); err == nil {
		t.Fatal("invalid request reached the socket")
	}
	for _, payload := range []string{"not json\n", `{"version":2}\n`} {
		path := socketTestPath(t, "reply.sock")
		listener, err := net.Listen("unix", path)
		if err != nil {
			t.Skip("unix sockets unavailable:", err)
		}
		done := make(chan struct{})
		go func() {
			defer close(done)
			defer listener.Close()
			conn, acceptErr := listener.Accept()
			if acceptErr != nil {
				return
			}
			defer conn.Close()
			_, _ = readRequest(conn)
			_, _ = io.WriteString(conn, payload)
		}()
		if _, err := (Client{Path: path}).Do(context.Background(), validRequest()); !errors.Is(err, ErrUnavailable) {
			t.Fatalf("payload %q err=%v", payload, err)
		}
		<-done
	}
}

func rawLogReaderRequest(t *testing.T, path, payload string) Response {
	t.Helper()
	conn, err := net.Dial("unix", path)
	if err != nil {
		t.Fatal(err)
	}
	defer conn.Close()
	if _, err := io.WriteString(conn, payload); err != nil {
		t.Fatal(err)
	}
	var response Response
	if err := json.NewDecoder(conn).Decode(&response); err != nil {
		t.Fatal(err)
	}
	return response
}
