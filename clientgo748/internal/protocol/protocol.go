// Package protocol será a fronteira de transporte do client Go.
//
// Os opcodes e estruturas não são duplicados nesta fundação. Antes de copiá-
// los, cada contrato deve ser correlacionado com internal/wire, a ficha nativa
// 7.48 e os testes byte a byte existentes no servidor.
package protocol

import (
	"context"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"net"
	"sync"
	"time"
)

// Version identifica a família de protocolo esperada pelo client.
// Ele é apenas um rótulo de configuração; não autoriza aceitar packets de uma
// versão diferente sem validação explícita.
const Version = "7.48"

const (
	// HeaderSize é o tamanho do cabeçalho _MSG observado no parser nativo.
	HeaderSize = 12
	// MaxFrameSize é o maior tamanho aceito por FUN_00424642: size < 0x1B59.
	MaxFrameSize = 0x1B58
	// InitCode é enviado pelo client imediatamente depois de conectar o TCP.
	InitCode uint32 = 0x1F11F311
)

var (
	ErrBadSize            = errors.New("protocol: invalid frame size")
	ErrBadChecksum        = errors.New("protocol: invalid frame checksum")
	ErrNotConnected       = errors.New("protocol: session is not connected")
	ErrReceiveLoopRunning = errors.New("protocol: receive loop is running")
	ErrInvalidEventBuffer = errors.New("protocol: invalid session event buffer")
)

const (
	DefaultSessionEventBuffer = 128
	MaxSessionEventBuffer     = 4096
)

// Header representa os campos já decifrados do _MSG. Os bytes do wire são
// little-endian e Type/ID/Tick ficam protegidos junto com o restante do frame.
type Header struct {
	Size     uint16
	KeyWord  byte
	CheckSum byte
	Type     uint16
	ID       uint16
	Tick     uint32
}

// Packet possui a cópia decifrada entregue ao dispatcher. Raw pertence ao
// Packet e permanece válido até ele deixar de ser usado; Body aponta para Raw.
type Packet struct {
	Header Header
	Raw    []byte
	Body   []byte
}

// SessionEventKind distingue um packet válido do encerramento da conexão.
// Nenhum callback é executado pela goroutine do socket: o owner deve drenar a
// fila na thread principal e então atualizar estado, cenas e UI.
type SessionEventKind uint8

const (
	SessionPacket SessionEventKind = iota + 1
	SessionDisconnected
)

// SessionEvent transfere ownership de Packet para o consumidor. Err só é
// preenchido para SessionDisconnected e deve ser usado para diagnóstico ou
// mensagem visível, nunca para preservar estado de uma sessão encerrada.
type SessionEvent struct {
	Kind   SessionEventKind
	Packet Packet
	Err    error
}

// Decode transforma um frame completo recebido do socket em um Packet. A
// entrada nunca é alterada: isso evita que o buffer de leitura compartilhado
// seja corrompido em caso de falha no checksum.
func Decode(frame []byte) (Packet, error) {
	if len(frame) < HeaderSize || len(frame) > MaxFrameSize {
		return Packet{}, ErrBadSize
	}
	size := int(binary.LittleEndian.Uint16(frame[:2]))
	if size != len(frame) || size < HeaderSize || size > MaxFrameSize {
		return Packet{}, ErrBadSize
	}
	decoded := append([]byte(nil), frame...)
	if !decrypt(decoded) {
		return Packet{}, ErrBadChecksum
	}
	header := Header{
		Size:     binary.LittleEndian.Uint16(decoded[0:2]),
		KeyWord:  decoded[2],
		CheckSum: decoded[3],
		Type:     binary.LittleEndian.Uint16(decoded[4:6]),
		ID:       binary.LittleEndian.Uint16(decoded[6:8]),
		Tick:     binary.LittleEndian.Uint32(decoded[8:12]),
	}
	return Packet{Header: header, Raw: decoded, Body: decoded[HeaderSize:]}, nil
}

// Encode finaliza um _MSG em claro e devolve o frame protegido. O argumento
// deve conter o cabeçalho lógico de 12 bytes seguido do corpo; Size, KeyWord e
// CheckSum são recalculados aqui, como no FUN_00424DFE nativo.
func Encode(plain []byte, key byte) ([]byte, error) {
	if len(plain) < HeaderSize || len(plain) > MaxFrameSize {
		return nil, ErrBadSize
	}
	frame := append([]byte(nil), plain...)
	binary.LittleEndian.PutUint16(frame[:2], uint16(len(frame)))
	frame[2] = key
	frame[3] = 0
	encrypt(frame, key)
	return frame, nil
}

// NewPacket cria somente o envelope lógico. O transporte ainda precisa chamar
// Encode, que escolhe a palavra-chave e calcula o checksum do frame.
func NewPacket(opcode, id uint16, tick uint32, body []byte) []byte {
	packet := make([]byte, HeaderSize+len(body))
	binary.LittleEndian.PutUint16(packet[4:6], opcode)
	binary.LittleEndian.PutUint16(packet[6:8], id)
	binary.LittleEndian.PutUint32(packet[8:12], tick)
	copy(packet[HeaderSize:], body)
	return packet
}

// StreamDecoder acumula bytes TCP e emite frames completos. Ele aceita tanto
// fragmentação quanto vários frames no mesmo recv, sem expor slices do buffer
// interno ao chamador.
type StreamDecoder struct {
	pending []byte
}

func (d *StreamDecoder) Feed(chunk []byte) ([]Packet, error) {
	if d == nil {
		return nil, errors.New("protocol: nil stream decoder")
	}
	d.pending = append(d.pending, chunk...)
	var packets []Packet
	for len(d.pending) >= 2 {
		size := int(binary.LittleEndian.Uint16(d.pending[:2]))
		if size < HeaderSize || size > MaxFrameSize {
			d.pending = nil
			return nil, ErrBadSize
		}
		if len(d.pending) < size {
			break
		}
		frame := append([]byte(nil), d.pending[:size]...)
		d.pending = append([]byte(nil), d.pending[size:]...)
		packet, err := Decode(frame)
		if err != nil {
			d.pending = nil
			return nil, err
		}
		packets = append(packets, packet)
	}
	return packets, nil
}

// PendingBytes existe apenas para testes e telemetria; o valor não concede
// ownership do buffer interno.
func (d *StreamDecoder) PendingBytes() int {
	if d == nil {
		return 0
	}
	return len(d.pending)
}

// Session representa o contrato mínimo de lifecycle usado pela Application.
type Session interface {
	Connect() error
	Close() error
}

// EventSession adiciona recepção assíncrona ao lifecycle mínimo. DrainEvents
// nunca bloqueia e deve ser chamado somente pelo owner na thread principal.
type EventSession interface {
	Session
	StartReceiving() error
	DrainEvents(limit int) []SessionEvent
}

// SessionOptions controla deadlines e permite injetar um dialer nos testes.
// Mensagens de gameplay não fazem parte desta camada.
type SessionOptions struct {
	Address        string
	ConnectTimeout time.Duration
	IOTimeout      time.Duration
	EventBuffer    int
	DialContext    func(context.Context, string) (net.Conn, error)
}

// ClientSession é uma sessão TCP de um único owner. O socket só é exposto
// através de Send/Receive; Close é idempotente e encerra qualquer conexão
// criada pela própria sessão.
type ClientSession struct {
	mu          sync.Mutex
	conn        net.Conn
	options     SessionOptions
	closed      bool
	events      chan SessionEvent
	receiving   bool
	receiveStop chan struct{}
	receiveDone chan struct{}
}

func NewSession(address string, options SessionOptions) *ClientSession {
	options.Address = address
	return &ClientSession{options: options}
}

func NewConnectedSession(conn net.Conn, options SessionOptions) (*ClientSession, error) {
	if conn == nil {
		return nil, fmt.Errorf("protocol: nil connection")
	}
	return &ClientSession{conn: conn, options: options}, nil
}

// Connect abre o TCP quando necessário e envia o InitCode exatamente uma vez.
func (s *ClientSession) Connect() error {
	if s == nil {
		return ErrNotConnected
	}
	s.mu.Lock()
	if s.closed {
		s.mu.Unlock()
		return ErrNotConnected
	}
	if s.conn != nil {
		s.mu.Unlock()
		return nil
	}
	address := s.options.Address
	dial := s.options.DialContext
	timeout := s.options.ConnectTimeout
	if timeout <= 0 {
		timeout = 5 * time.Second
	}
	s.mu.Unlock()
	if address == "" {
		return fmt.Errorf("protocol: session address is required")
	}
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()
	if dial == nil {
		d := net.Dialer{}
		dial = func(ctx context.Context, address string) (net.Conn, error) {
			return d.DialContext(ctx, "tcp", address)
		}
	}
	conn, err := dial(ctx, address)
	if err != nil {
		return fmt.Errorf("protocol: connect: %w", err)
	}
	s.mu.Lock()
	if s.closed {
		s.mu.Unlock()
		_ = conn.Close()
		return ErrNotConnected
	}
	s.conn = conn
	s.mu.Unlock()
	if err := s.writeAll([]byte{0x11, 0xF3, 0x11, 0x1F}); err != nil {
		_ = s.Close()
		return fmt.Errorf("protocol: send handshake: %w", err)
	}
	return nil
}

// Send protege e escreve um frame completo. O chamador continua dono de
// plain; a sessão mantém somente a cópia produzida por Encode durante Write.
func (s *ClientSession) Send(plain []byte, key byte) error {
	frame, err := Encode(plain, key)
	if err != nil {
		return err
	}
	if err := s.writeAll(frame); err != nil {
		return fmt.Errorf("protocol: send frame: %w", err)
	}
	return nil
}

// Receive lê exatamente um frame. O tamanho é validado antes da alocação para
// impedir que um peer remoto force uma reserva fora do limite nativo.
func (s *ClientSession) Receive() (Packet, error) {
	conn, err := s.connectionForReceive()
	if err != nil {
		return Packet{}, err
	}
	return receivePacket(conn, s.options.IOTimeout)
}

// StartReceiving inicia no máximo uma rotina leitora para a conexão atual. A
// rotina apenas produz eventos; ela não conhece dispatcher, cenas ou UI.
func (s *ClientSession) StartReceiving() error {
	if s == nil {
		return ErrNotConnected
	}
	bufferSize, err := sessionEventBufferSize(s.options.EventBuffer)
	if err != nil {
		return err
	}
	s.mu.Lock()
	if s.closed || s.conn == nil {
		s.mu.Unlock()
		return ErrNotConnected
	}
	if s.receiving {
		s.mu.Unlock()
		return nil
	}
	if s.events == nil {
		s.events = make(chan SessionEvent, bufferSize)
	}
	conn := s.conn
	stop := make(chan struct{})
	done := make(chan struct{})
	s.receiveStop = stop
	s.receiveDone = done
	s.receiving = true
	timeout := s.options.IOTimeout
	s.mu.Unlock()

	go s.receiveLoop(conn, timeout, stop, done)
	return nil
}

// DrainEvents transfere até limit eventos já disponíveis sem aguardar rede.
// Um limite não positivo não drena nada, evitando trabalho ilimitado em um
// único frame.
func (s *ClientSession) DrainEvents(limit int) []SessionEvent {
	if s == nil || limit <= 0 {
		return nil
	}
	s.mu.Lock()
	events := s.events
	s.mu.Unlock()
	if events == nil {
		return nil
	}
	drained := make([]SessionEvent, 0, min(limit, len(events)))
	for len(drained) < limit {
		select {
		case event := <-events:
			drained = append(drained, event)
		default:
			return drained
		}
	}
	return drained
}

func (s *ClientSession) receiveLoop(conn net.Conn, timeout time.Duration, stop, done chan struct{}) {
	defer func() {
		_ = conn.Close()
		s.mu.Lock()
		if s.receiveDone == done {
			s.receiving = false
			s.receiveStop = nil
			s.receiveDone = nil
			if s.conn == conn {
				s.conn = nil
			}
		}
		s.mu.Unlock()
		close(done)
	}()

	for {
		packet, err := receivePacket(conn, timeout)
		if err != nil {
			select {
			case <-stop:
				return
			default:
			}
			_ = s.enqueueEvent(SessionEvent{Kind: SessionDisconnected, Err: err}, stop)
			return
		}
		if !s.enqueueEvent(SessionEvent{Kind: SessionPacket, Packet: packet}, stop) {
			return
		}
	}
}

func (s *ClientSession) enqueueEvent(event SessionEvent, stop <-chan struct{}) bool {
	s.mu.Lock()
	events := s.events
	s.mu.Unlock()
	if events == nil {
		return false
	}
	select {
	case events <- event:
		return true
	case <-stop:
		return false
	}
}

func receivePacket(conn net.Conn, timeout time.Duration) (Packet, error) {
	if timeout > 0 {
		_ = conn.SetReadDeadline(time.Now().Add(timeout))
	}
	var sizeBytes [2]byte
	if _, err := io.ReadFull(conn, sizeBytes[:]); err != nil {
		return Packet{}, err
	}
	size := int(binary.LittleEndian.Uint16(sizeBytes[:]))
	if size < HeaderSize || size > MaxFrameSize {
		return Packet{}, ErrBadSize
	}
	frame := make([]byte, size)
	copy(frame[:2], sizeBytes[:])
	if _, err := io.ReadFull(conn, frame[2:]); err != nil {
		return Packet{}, err
	}
	return Decode(frame)
}

func (s *ClientSession) writeAll(data []byte) error {
	conn, err := s.connection()
	if err != nil {
		return err
	}
	if timeout := s.options.IOTimeout; timeout > 0 {
		_ = conn.SetWriteDeadline(time.Now().Add(timeout))
	}
	for len(data) > 0 {
		n, err := conn.Write(data)
		if err != nil {
			return err
		}
		if n == 0 {
			return io.ErrShortWrite
		}
		data = data[n:]
	}
	return nil
}

func (s *ClientSession) connection() (net.Conn, error) {
	if s == nil {
		return nil, ErrNotConnected
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.closed || s.conn == nil {
		return nil, ErrNotConnected
	}
	return s.conn, nil
}

func (s *ClientSession) connectionForReceive() (net.Conn, error) {
	if s == nil {
		return nil, ErrNotConnected
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.closed || s.conn == nil {
		return nil, ErrNotConnected
	}
	if s.receiving {
		return nil, ErrReceiveLoopRunning
	}
	return s.conn, nil
}

func sessionEventBufferSize(configured int) (int, error) {
	if configured == 0 {
		return DefaultSessionEventBuffer, nil
	}
	if configured < 0 || configured > MaxSessionEventBuffer {
		return 0, fmt.Errorf("%w: got %d, range 1..%d", ErrInvalidEventBuffer, configured, MaxSessionEventBuffer)
	}
	return configured, nil
}

// Close pode ser chamado várias vezes e fecha somente o socket que a sessão
// possui. Quando a recepção assíncrona está ativa, o fechamento do socket
// desbloqueia a leitura e o método aguarda a goroutine terminar.
func (s *ClientSession) Close() error {
	if s == nil {
		return nil
	}
	s.mu.Lock()
	if s.closed {
		s.mu.Unlock()
		return nil
	}
	s.closed = true
	conn := s.conn
	s.conn = nil
	stop := s.receiveStop
	done := s.receiveDone
	if stop != nil {
		close(stop)
	}
	s.mu.Unlock()
	var err error
	if conn != nil {
		err = conn.Close()
	}
	if done != nil {
		<-done
	}
	return err
}
