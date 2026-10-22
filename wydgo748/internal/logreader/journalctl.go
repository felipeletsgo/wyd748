package logreader

import (
	"bufio"
	"context"
	"encoding/json"
	"errors"
	"os/exec"
	"strconv"
	"time"
)

// ErrCursorExpired reports a cursor the journal no longer resolves, usually
// after rotation removed its entry.
var ErrCursorExpired = errors.New("logreader: cursor expired")

// Journalctl reads the journal through a fixed journalctl argument list. Only
// the unit (from Sources), numeric bounds and a validated cursor are passed,
// each as one argv element; no shell is involved.
type Journalctl struct {
	Path string
}

type journalLine struct {
	Cursor   string          `json:"__CURSOR"`
	Realtime string          `json:"__REALTIME_TIMESTAMP"`
	Priority string          `json:"PRIORITY"`
	Message  json.RawMessage `json:"MESSAGE"`
}

// journalMessage decodes MESSAGE, which journald emits as a string or, for
// non-UTF-8 payloads, as an array of byte values.
func journalMessage(raw json.RawMessage) string {
	var text string
	if json.Unmarshal(raw, &text) == nil {
		return text
	}
	var bytes []byte
	var values []int
	if json.Unmarshal(raw, &values) == nil {
		for _, v := range values {
			if v >= 0 && v < 256 {
				bytes = append(bytes, byte(v))
			}
		}
	}
	return string(bytes)
}

func journalArgs(unit string, r Request) []string {
	args := []string{"--unit=" + unit, "--output=json", "--no-pager", "--quiet", "--reverse",
		"--since=@" + strconv.FormatInt(r.Since.Unix(), 10), "--priority=0.." + strconv.Itoa(r.Priority)}
	if !r.Until.IsZero() {
		args = append(args, "--until=@"+strconv.FormatInt(r.Until.Unix(), 10))
	}
	if r.Cursor != "" {
		args = append(args, "--cursor="+r.Cursor)
	}
	return args
}

func (j Journalctl) Read(ctx context.Context, unit string, r Request, scanLimit int) ([]Entry, bool, error) {
	path := j.Path
	if path == "" {
		path = "journalctl"
	}
	ctx, cancel := context.WithCancel(ctx)
	defer cancel()
	cmd := exec.CommandContext(ctx, path, journalArgs(unit, r)...)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return nil, false, err
	}
	if err := cmd.Start(); err != nil {
		return nil, false, err
	}
	scanner := bufio.NewScanner(stdout)
	scanner.Buffer(make([]byte, 64*1024), 1024*1024)
	entries := make([]Entry, 0, min(scanLimit, 512))
	more := false
	for scanner.Scan() {
		var line journalLine
		if json.Unmarshal(scanner.Bytes(), &line) != nil || line.Cursor == "" {
			continue
		}
		// With --cursor and --reverse journalctl starts at the cursor entry.
		if line.Cursor == r.Cursor {
			continue
		}
		if len(entries) == scanLimit {
			more = true
			break
		}
		micros, _ := strconv.ParseInt(line.Realtime, 10, 64)
		priority, _ := strconv.Atoi(line.Priority)
		entries = append(entries, Entry{Cursor: line.Cursor, Time: time.UnixMicro(micros).UTC(), Priority: priority,
			Message: journalMessage(line.Message)})
	}
	scanErr := scanner.Err()
	cancel()
	waitErr := cmd.Wait()
	if scanErr != nil {
		return nil, false, scanErr
	}
	// Cancelling after a full page kills journalctl; that is not a failure.
	if waitErr != nil && !more {
		var exitErr *exec.ExitError
		if r.Cursor != "" && errors.As(waitErr, &exitErr) {
			return nil, false, ErrCursorExpired
		}
		return nil, false, waitErr
	}
	return entries, more, nil
}
