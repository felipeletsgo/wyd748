package net

import (
	"log"
	stdnet "net"
	"sync/atomic"
)

var connSeq atomic.Int64

const defaultOutputQueueSize = 256

// ListenWithQueue permite ajustar a folga de saida em ambientes com uma
// visibilidade inicial grande (por exemplo, o loadtest com centenas de bots).
// A fila continua limitada e nao bloqueante; o valor maior evita que a rajada
// inicial de CreateMob desconecte um cliente antes de ele conseguir drenar.
func ListenWithQueue(addr string, queueSize int, onConn func(*Session)) error {
	if queueSize < 1 {
		queueSize = defaultOutputQueueSize
	}
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
			out:  make(chan []byte, queueSize),
			done: make(chan struct{}),
		}
		go onConn(s)
	}
}
