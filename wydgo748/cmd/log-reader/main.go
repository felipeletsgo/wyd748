// Command log-reader is the wyd-log-reader helper: it serves allowlisted
// journal units and backup receipts to the staff panel over a Unix socket
// (DOCS/server/staff-operations.md). It has no network listener.
package main

import (
	"errors"
	"flag"
	"fmt"
	"log"
	"net"
	"os"
	"os/user"
	"strconv"

	"wydgo/internal/logreader"
)

// systemdListener adopts the socket passed by a systemd .socket unit
// (LISTEN_FDS=1, fd 3), so the socket's owner and mode come from systemd.
func systemdListener() (net.Listener, error) {
	if os.Getenv("LISTEN_PID") != strconv.Itoa(os.Getpid()) || os.Getenv("LISTEN_FDS") != "1" {
		return nil, nil
	}
	file := os.NewFile(3, "systemd-socket")
	defer file.Close()
	return net.FileListener(file)
}

func main() {
	socketPath := flag.String("socket", "", "listen on this Unix socket when not socket-activated")
	allowUser := flag.String("allow-user", "wyd-game", "only this local user may connect")
	backupStatus := flag.String("backup-status", "/var/lib/wyd-go-backup/status.json", "backup receipt")
	restoreStatus := flag.String("restore-status", "/var/lib/wyd-go-backup/restore.json", "restore test receipt")
	flag.Parse()

	account, err := user.Lookup(*allowUser)
	if err != nil {
		log.Fatalf("allowed user: %v", err)
	}
	allowedUID, err := strconv.ParseUint(account.Uid, 10, 32)
	if err != nil {
		log.Fatalf("allowed user ID: %v", err)
	}
	listener, err := systemdListener()
	if err == nil && listener == nil {
		if *socketPath == "" {
			err = errors.New("no systemd socket and no -socket path")
		} else {
			listener, err = net.Listen("unix", *socketPath)
		}
	}
	if err != nil {
		log.Fatalf("listen: %v", err)
	}
	server := &logreader.Server{
		Journal:       logreader.Journalctl{},
		BackupStatus:  *backupStatus,
		RestoreStatus: *restoreStatus,
		MaxConcurrent: 2,
		Authorize: func(conn net.Conn) error {
			uid, err := logreader.PeerUID(conn)
			if err != nil {
				return err
			}
			if uint64(uid) != allowedUID {
				return fmt.Errorf("peer uid %d is not allowed", uid)
			}
			return nil
		},
	}
	log.Printf("log reader ready for user %s", *allowUser)
	if err := server.Serve(listener); err != nil {
		log.Fatal(err)
	}
}
