package net

import (
	"log"
	stdnet "net"
	"sync/atomic"
)

var connSeq atomic.Int64

// Listen abre o accept loop em addr e roda onConn (numa goroutine) por conexao.
// A Session ja vem com fila de saida e canal de vida prontos; o caller decide o
// que fazer com ela (tipicamente s.Serve(world.Enqueue)).
func Listen(addr string, onConn func(*Session)) error {
	ln, err := stdnet.Listen("tcp", addr)
	if err != nil {
		return err
	}
	log.Printf("WYD-Go TMSrv escutando em %s", addr)
	for {
		c, err := ln.Accept()
		if err != nil {
			log.Printf("accept: %v", err)
			continue
		}
		s := &Session{
			ID:   connSeq.Add(1),
			conn: c,
			out:  make(chan []byte, 256),
			done: make(chan struct{}),
		}
		go onConn(s)
	}
}
