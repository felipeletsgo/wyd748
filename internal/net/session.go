// Package net e a camada de I/O de socket: 1 conexao TCP = 1 Session, com
// handshake InitCode, loop de leitura e fila de saida nao-bloqueante. Nao conhece
// o game -- recebe/entrega bytes crus e delega o resto via callback.
package net

import (
	"bufio"
	"encoding/binary"
	"errors"
	"io"
	"log"
	stdnet "net"
	"sync/atomic"
	"time"

	"wydgo/internal/wire"
)

var (
	serverStart = time.Now()
	sendKey     atomic.Uint32
)

// tick simula o GetTickCount do servidor (base 60000 pra nunca ser ~0); o client
// usa o tick real do header em decisoes de animacao/morte.
func tick() uint32 {
	return uint32(time.Since(serverStart)/time.Millisecond) + 60000
}

// Session representa 1 conexao TCP com um client.
type Session struct {
	ID   int64
	conn stdnet.Conn
	out  chan []byte
	done chan struct{}
}

// Send finaliza o pacote (Tick@8 + Size@0 + cifra) e o enfileira SEM bloquear.
// Se a fila de saida estourar (client lento), desconecta em vez de travar o loop.
// Seguro chamar de qualquer goroutine; na pratica so o game loop chama.
func (s *Session) Send(pkt []byte) {
	binary.LittleEndian.PutUint32(pkt[8:12], tick())
	wire.FinishPacket(pkt, byte(sendKey.Add(1)))
	select {
	case s.out <- pkt:
	default:
		s.conn.Close()
	}
}

// RemoteAddr expoe o endereco remoto (log).
func (s *Session) RemoteAddr() string { return s.conn.RemoteAddr().String() }

// Close encerra a conexao. Usado pelo game depois de enviar uma rejeicao de
// autenticacao; o pequeno atraso fica no chamador para a fila conseguir sair.
func (s *Session) Close() {
	if s.conn != nil {
		_ = s.conn.Close()
	}
}

// writeLoop drena a fila de saida pro socket. Sai quando o socket falha ou quando
// Serve fecha `done` (evita vazar goroutine numa fila vazia sem fechar `out`).
func (s *Session) writeLoop() {
	for {
		select {
		case pkt := <-s.out:
			if _, err := s.conn.Write(pkt); err != nil {
				return
			}
		case <-s.done:
			return
		}
	}
}

// Serve faz o handshake InitCode e roda o loop de leitura, chamando handler pra
// cada pacote decifrado. handler(s, nil) sinaliza desconexao ao fim.
func (s *Session) Serve(handler func(*Session, []byte)) {
	defer s.conn.Close()
	defer close(s.done)
	defer handler(s, nil)

	r := bufio.NewReader(s.conn)

	// handshake: 4 bytes de InitCode antes de qualquer pacote.
	var ic [4]byte
	if _, err := io.ReadFull(r, ic[:]); err != nil {
		log.Printf("[#%d] sem InitCode: %v", s.ID, err)
		return
	}
	if got := binary.LittleEndian.Uint32(ic[:]); got != uint32(wire.InitCode) {
		log.Printf("[#%d] InitCode invalido: 0x%X", s.ID, got)
		return
	}
	log.Printf("[#%d] conectado: %s (InitCode OK)", s.ID, s.RemoteAddr())

	go s.writeLoop()

	for {
		buf, _, err := wire.ReadPacket(r)
		if err != nil {
			if !errors.Is(err, io.EOF) {
				log.Printf("[#%d] read: %v", s.ID, err)
			}
			log.Printf("[#%d] desconectado", s.ID)
			return
		}
		handler(s, buf)
	}
}
