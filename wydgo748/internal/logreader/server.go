package logreader

import (
	"bufio"
	"context"
	"encoding/json"
	"errors"
	"io"
	"log"
	"net"
	"os"
	"strings"
	"time"
)

// Journal reads one unit newest first, starting after cursor when set, and
// stops after scanLimit entries. more reports that older entries remain.
type Journal interface {
	Read(ctx context.Context, unit string, r Request, scanLimit int) (entries []Entry, more bool, err error)
}

type Server struct {
	Journal Journal
	// Authorize checks the connecting process (SO_PEERCRED on Linux).
	Authorize     func(net.Conn) error
	BackupStatus  string
	RestoreStatus string
	MaxConcurrent int
}

// Serve accepts connections until the listener closes. Each connection
// carries one request and one response with a hard deadline.
func (s *Server) Serve(listener net.Listener) error {
	slots := make(chan struct{}, max(s.MaxConcurrent, 1))
	for {
		conn, err := listener.Accept()
		if err != nil {
			if errors.Is(err, net.ErrClosed) {
				return nil
			}
			return err
		}
		select {
		case slots <- struct{}{}:
			go func() {
				defer func() { <-slots }()
				s.handle(conn)
			}()
		default:
			// Read the bounded request before replying so closing the socket
			// cannot reset the connection ahead of the answer.
			go func() {
				defer conn.Close()
				_ = conn.SetDeadline(time.Now().Add(time.Second))
				_, _ = readRequest(conn)
				_ = writeResponse(conn, Response{Version: 1, Error: "busy"})
			}()
		}
	}
}

func writeResponse(conn net.Conn, response Response) error {
	return json.NewEncoder(conn).Encode(response)
}

// readRequest decodes one bounded request from the connection.
func readRequest(conn net.Conn) (Request, error) {
	var request Request
	decoder := json.NewDecoder(io.LimitReader(conn, MaxRequestBytes))
	decoder.DisallowUnknownFields()
	err := decoder.Decode(&request)
	return request, err
}

func (s *Server) handle(conn net.Conn) {
	defer conn.Close()
	_ = conn.SetDeadline(time.Now().Add(5 * time.Second))
	// The request is read before the peer check only so a refusal reaches the
	// caller intact; nothing in it is used for an unauthorized peer.
	request, readErr := readRequest(conn)
	if s.Authorize == nil || s.Authorize(conn) != nil {
		_ = writeResponse(conn, Response{Version: 1, Error: "forbidden"})
		return
	}
	if readErr != nil {
		_ = writeResponse(conn, Response{Version: 1, Error: "invalid_request"})
		return
	}
	if err := request.Validate(); err != nil {
		_ = writeResponse(conn, Response{Version: 1, Error: err.Error()})
		return
	}
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()
	var response Response
	if request.Kind == KindBackup {
		response = s.backup()
	} else {
		response = s.logs(ctx, request)
	}
	if err := writeResponse(conn, response); err != nil {
		log.Printf("log reader reply: %v", err)
	}
}

func (s *Server) logs(ctx context.Context, request Request) Response {
	scanLimit := min(request.Limit*20, 5000)
	entries, more, err := s.Journal.Read(ctx, Sources[request.Source], request, scanLimit)
	if errors.Is(err, ErrCursorExpired) {
		return Response{Version: 1, Error: "cursor_expired"}
	}
	if err != nil {
		log.Printf("log reader journal source=%s: %v", request.Source, err)
		return Response{Version: 1, Error: "journal_unavailable"}
	}
	response := Response{Version: 1, Entries: []Entry{}}
	query := strings.ToLower(request.Query)
	// Budget the encoded size of the entries actually sent: JSON escaping
	// (quotes, <, >, &) can grow a message up to six times. responseEnvelope
	// covers the fields around the list, including a full-length cursor.
	size := responseEnvelope
	for i, entry := range entries {
		entry.Message = Sanitize(entry.Message)
		if query != "" && !strings.Contains(strings.ToLower(entry.Message), query) {
			continue
		}
		encoded, err := json.Marshal(entry)
		if err != nil {
			continue
		}
		if size+len(encoded)+1 > MaxResponseBytes {
			// This entry is not sent; the next page starts right after the
			// previous scanned one, so nothing is skipped.
			if i > 0 {
				response.NextCursor = entries[i-1].Cursor
			}
			response.Truncated = true
			return response
		}
		size += len(encoded) + 1
		response.Entries = append(response.Entries, entry)
		if len(response.Entries) == request.Limit {
			if i < len(entries)-1 || more {
				response.NextCursor = entry.Cursor
			}
			return response
		}
	}
	if more && len(entries) > 0 {
		// The scan budget ended before the page filled: continue from there.
		response.NextCursor, response.Truncated = entries[len(entries)-1].Cursor, true
	}
	return response
}

func readReceipt(path string) *Receipt {
	if path == "" {
		return nil
	}
	file, err := os.Open(path)
	if err != nil {
		return nil
	}
	defer file.Close()
	var receipt Receipt
	if json.NewDecoder(io.LimitReader(file, 4096)).Decode(&receipt) != nil || receipt.At.IsZero() {
		return nil
	}
	receipt.Archive = Sanitize(receipt.Archive)
	return &receipt
}

func (s *Server) backup() Response {
	return Response{Version: 1, Backup: readReceipt(s.BackupStatus), RestoreTest: readReceipt(s.RestoreStatus)}
}

// Client sends one request to the helper socket.
type Client struct{ Path string }

var ErrUnavailable = errors.New("logreader: helper unavailable")

func (c Client) Do(ctx context.Context, request Request) (Response, error) {
	if c.Path == "" {
		return Response{}, ErrUnavailable
	}
	if err := request.Validate(); err != nil {
		return Response{}, err
	}
	var dialer net.Dialer
	conn, err := dialer.DialContext(ctx, "unix", c.Path)
	if err != nil {
		return Response{}, ErrUnavailable
	}
	defer conn.Close()
	deadline, ok := ctx.Deadline()
	if !ok {
		deadline = time.Now().Add(5 * time.Second)
	}
	_ = conn.SetDeadline(deadline)
	if err := json.NewEncoder(conn).Encode(request); err != nil {
		return Response{}, ErrUnavailable
	}
	var response Response
	// The server keeps replies within MaxResponseBytes; the extra room only
	// absorbs a trailing newline, never a larger reply.
	reader := bufio.NewReader(io.LimitReader(conn, MaxResponseBytes+64))
	if err := json.NewDecoder(reader).Decode(&response); err != nil || response.Version != 1 {
		return Response{}, ErrUnavailable
	}
	return response, nil
}
