package wire

import (
	"encoding/binary"
	"errors"
	"io"
)

// HeaderSize is the 12-byte _MSG header used by every WYD packet.
const HeaderSize = 12

// MaxPacketSize matches MAX_MESSAGE_SIZE in the native CPSock. Accepting the
// full WORD range (65535) would let a hostile connection force large allocations
// before the opcode is validated.
const MaxPacketSize = 8192

var ErrBadSize = errors.New("wire: invalid packet size")

// Header is the little-endian _MSG header.
type Header struct {
	Size     uint16
	KeyWord  byte
	CheckSum byte
	Type     uint16
	ID       uint16
	Tick     uint32
}

// ParseHeader reads the 12-byte header from a buffer.
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

// ReadPacket reads a complete Size-framed packet and decrypts it in place.
// It returns the decrypted buffer and whether the checksum matched.
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

// FinishPacket fills Size and encrypts a buffer ready for sending. Pass a
// varying iKey (for example, from a counter or random source). Type, ID, Tick,
// and the body must already be written.
func FinishPacket(buf []byte, iKey byte) {
	binary.LittleEndian.PutUint16(buf[0:2], uint16(len(buf)))
	Encrypt(buf, iKey)
}
