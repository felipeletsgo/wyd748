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
	"sync"
	"sync/atomic"
	"time"

	"wydgo/internal/wire"
)

const (
	// Limite operacional, nao de gameplay. Um client 7.48 normal fica muito
	// abaixo disso; a folga absorve rajadas de movimento e carregamento.
	maxInboundPacketsPerSecond = 256
	maxInboundBytesPerSecond   = 512 * 1024
	defaultHandshakeTimeout    = 5 * time.Second
	defaultSessionIdleTimeout  = 10 * time.Minute
	defaultFrameReadTimeout    = 10 * time.Second
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
	ID                      int64
	conn                    stdnet.Conn
	remoteIP                string
	out                     chan []byte
	done                    chan struct{}
	doneOnce                sync.Once
	handshakeTimeout        time.Duration
	idleTimeout             time.Duration
	frameReadTimeout        time.Duration
	maxInboundPacketsPerSec int
	maxInboundBytesPerSec   int
}

// IsClosed informa se Serve concluiu o ciclo de I/O. Sessões de teste sem
// canal de vida são consideradas ativas até serem substituídas pelo teste.
func (s *Session) IsClosed() bool {
	if s == nil || s.done == nil {
		return false
	}
	select {
	case <-s.done:
		return true
	default:
		return false
	}
}

func (s *Session) closeDone() {
	if s == nil || s.done == nil {
		return
	}
	s.doneOnce.Do(func() { close(s.done) })
}

// Send finaliza o pacote (Tick@8 + Size@0 + cifra) e o enfileira SEM bloquear.
// Se a fila de saida estourar (client lento), desconecta em vez de travar o loop.
// Seguro chamar de qualquer goroutine; na pratica so o game loop chama.
func (s *Session) Send(pkt []byte) {
	if s == nil || len(pkt) < wire.HeaderSize || len(pkt) > wire.MaxPacketSize || s.out == nil {
		// Builders sao internos, mas uma regressao neles nao pode virar um panic
		// dentro da goroutine unica do World. Falhe fechado nesta sessao; o log
		// do handler/metricas continua apontando o pacote que originou o envio.
		if s != nil {
			log.Printf("[#%d] pacote de saida invalido: %d bytes", s.ID, len(pkt))
			s.Close()
		}
		return
	}
	binary.LittleEndian.PutUint32(pkt[8:12], tick())
	wire.FinishPacket(pkt, byte(sendKey.Add(1)))
	select {
	case s.out <- pkt:
	default:
		if s.conn != nil {
			_ = s.conn.Close()
		}
	}
}

// NewTestSession devolve uma Session utilizavel em testes de outros pacotes
// (ex.: internal/game): Send() enfileira normalmente ate a capacidade do
// buffer sem tocar numa conexao real. `conn` fica nil de proposito -- so e
// usada se o buffer estourar, o que os testes evitam dimensionando bufSize.
func NewTestSession(id int64, bufSize int) *Session {
	return &Session{ID: id, out: make(chan []byte, bufSize), done: make(chan struct{})}
}

// NewTestSessionWithRemoteIP cria uma sessao de teste com a origem que o
// listener de producao registraria. Isso permite testar limites por IP sem
// expor uma conexao falsa nem confiar em campos de packet.
func NewTestSessionWithRemoteIP(id int64, bufSize int, remoteIP string) *Session {
	return &Session{
		ID: id, remoteIP: remoteIP, out: make(chan []byte, bufSize), done: make(chan struct{}),
	}
}

// QueuedPacketsForTest informa quantos pacotes NewTestSession recebeu. O
// conteudo permanece cifrado na fila; testes de game usam apenas a contagem
// para detectar envios extras que alteram estado visual, como CreateMob.
func (s *Session) QueuedPacketsForTest() int {
	if s == nil || s.out == nil {
		return 0
	}
	return len(s.out)
}

// RemoteAddr expoe o endereco remoto (log).
func (s *Session) RemoteAddr() string {
	if s == nil || s.conn == nil || s.conn.RemoteAddr() == nil {
		return ""
	}
	return s.conn.RemoteAddr().String()
}

// RemoteIP devolve somente o IP remoto, sem porta. Limites de conexao e
// autenticacao nunca devem usar a string host:port, pois a porta muda em toda
// tentativa e tornaria o limitador ineficaz.
func (s *Session) RemoteIP() string {
	if s == nil {
		return ""
	}
	if s.remoteIP != "" {
		return s.remoteIP
	}
	addr := s.RemoteAddr()
	host, _, err := stdnet.SplitHostPort(addr)
	if err != nil {
		return addr
	}
	return host
}

// Close encerra a conexao. Usado pelo game depois de enviar uma rejeicao de
// autenticacao; o pequeno atraso fica no chamador para a fila conseguir sair.
func (s *Session) Close() {
	if s.conn != nil {
		_ = s.conn.Close()
	}
	s.closeDone()
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
	defer handler(s, nil)
	// Sinaliza o fim antes de entregar o comando ao World. Se a fila estiver
	// cheia, uma nova tentativa de login ainda consegue detectar que esta
	// reserva pertence a um socket morto.
	defer s.closeDone()

	r := bufio.NewReader(s.conn)
	handshakeTimeout := s.handshakeTimeout
	if handshakeTimeout <= 0 {
		handshakeTimeout = defaultHandshakeTimeout
	}
	if err := s.conn.SetReadDeadline(time.Now().Add(handshakeTimeout)); err != nil {
		log.Printf("[#%d] configurar prazo do InitCode: %v", s.ID, err)
		return
	}

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

	idleTimeout := s.idleTimeout
	if idleTimeout <= 0 {
		idleTimeout = defaultSessionIdleTimeout
	}
	packetLimit := s.maxInboundPacketsPerSec
	if packetLimit <= 0 {
		packetLimit = maxInboundPacketsPerSecond
	}
	byteLimit := s.maxInboundBytesPerSec
	if byteLimit <= 0 {
		byteLimit = maxInboundBytesPerSecond
	}
	windowStarted := time.Now()
	packetCount, byteCount := 0, 0
	for {
		// O primeiro deadline aceita o heartbeat espaçado do client 7.48.
		if err := s.conn.SetReadDeadline(time.Now().Add(idleTimeout)); err != nil {
			log.Printf("[#%d] configurar prazo de leitura: %v", s.ID, err)
			return
		}
		var sizeBytes [2]byte
		if _, err := io.ReadFull(r, sizeBytes[:]); err != nil {
			if !errors.Is(err, io.EOF) {
				log.Printf("[#%d] read: %v", s.ID, err)
			}
			log.Printf("[#%d] desconectado", s.ID)
			return
		}
		size := int(binary.LittleEndian.Uint16(sizeBytes[:]))
		if size < wire.HeaderSize || size > wire.MaxPacketSize {
			log.Printf("[#%d] read: %v", s.ID, wire.ErrBadSize)
			return
		}
		frameTimeout := s.frameReadTimeout
		if frameTimeout <= 0 {
			frameTimeout = defaultFrameReadTimeout
		}
		if err := s.conn.SetReadDeadline(time.Now().Add(frameTimeout)); err != nil {
			// O peer pode fechar logo depois de entregar o frame inteiro (comum em
			// net.Pipe e shutdown). Ainda tente drenar os bytes ja recebidos; o
			// ReadFull seguinte encerra normalmente se o corpo estiver incompleto.
			log.Printf("[#%d] configurar prazo do frame: %v", s.ID, err)
		}
		buf := make([]byte, size)
		copy(buf[:2], sizeBytes[:])
		_, err := io.ReadFull(r, buf[2:])
		if err != nil {
			if !errors.Is(err, io.EOF) {
				log.Printf("[#%d] read: %v", s.ID, err)
			}
			log.Printf("[#%d] desconectado", s.ID)
			return
		}
		checksumOK := wire.Decrypt(buf)
		if !checksumOK {
			log.Printf("[#%d] checksum invalido; conexao encerrada", s.ID)
			return
		}
		now := time.Now()
		if now.Sub(windowStarted) >= time.Second {
			windowStarted, packetCount, byteCount = now, 0, 0
		}
		packetCount++
		byteCount += len(buf)
		if packetCount > packetLimit || byteCount > byteLimit {
			log.Printf("[#%d] flood de entrada: pacotes=%d bytes=%d/s", s.ID, packetCount, byteCount)
			return
		}
		handler(s, buf)
	}
}
