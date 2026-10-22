//go:build !linux

package logreader

import (
	"errors"
	"net"
)

// PeerUID is only available on Linux; the helper runs only on the VPS.
func PeerUID(net.Conn) (uint32, error) {
	return 0, errors.New("logreader: peer credentials need Linux")
}
