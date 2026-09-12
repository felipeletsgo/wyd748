package wire

import (
	"encoding/binary"
	"errors"
	"io"
)

// HeaderSize -- os 12 bytes do cabecalho _MSG de todo pacote WYD.
const HeaderSize = 12

// MaxPacketSize reproduz MAX_MESSAGE_SIZE do CPSock nativo. Aceitar o WORD
// inteiro (65535) permite que uma conexao hostil force alocacoes grandes antes
// mesmo de o opcode ser validado.
const MaxPacketSize = 8192

var ErrBadSize = errors.New("wire: tamanho de pacote invalido")

// Header -- cabecalho _MSG (little-endian).
type Header struct {
	Size     uint16
	KeyWord  byte
	CheckSum byte
	Type     uint16
	ID       uint16
	Tick     uint32
}

// ParseHeader le os 12 bytes de cabecalho de um buffer.
func ParseHeader(b []byte) Header {
	return Header{
		Size:     binary.LittleEndian.Uint16(b[0:2]),
		KeyWord:  b[2],
		CheckSum: b[3],
		Type:     binary.LittleEndian.Uint16(b[4:6]),
		ID:       binary.LittleEndian.Uint16(b[6:8]),
		Tick:     binary.LittleEndian.Uint32(b[8:12]),
	}
}

// ReadPacket le um pacote completo do stream (frame pelo campo Size) e o decifra
// in-place. Retorna o buffer decifrado + se o checksum bateu.
func ReadPacket(r io.Reader) (buf []byte, okChecksum bool, err error) {
	var sz [2]byte
	if _, err = io.ReadFull(r, sz[:]); err != nil {
		return nil, false, err
	}
	size := int(binary.LittleEndian.Uint16(sz[:]))
	if size < HeaderSize || size > MaxPacketSize {
		return nil, false, ErrBadSize
	}
	buf = make([]byte, size)
	buf[0], buf[1] = sz[0], sz[1]
	if _, err = io.ReadFull(r, buf[2:]); err != nil {
		return nil, false, err
	}
	okChecksum = Decrypt(buf)
	return buf, okChecksum, nil
}

// FinishPacket preenche Size e cifra o buffer pronto para envio (iKey aleatorio
// deve ser passado; use um contador/rand). Assume Type/ID/Tick/body ja escritos.
func FinishPacket(buf []byte, iKey byte) {
	binary.LittleEndian.PutUint16(buf[0:2], uint16(len(buf)))
	Encrypt(buf, iKey)
}
