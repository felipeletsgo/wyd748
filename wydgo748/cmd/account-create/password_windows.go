//go:build windows

package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
	"syscall"
	"unsafe"
)

const enableEchoInput = 0x0004

var (
	kernel32           = syscall.NewLazyDLL("kernel32.dll")
	getConsoleModeProc = kernel32.NewProc("GetConsoleMode")
	setConsoleModeProc = kernel32.NewProc("SetConsoleMode")
)

func readPassword(reader *bufio.Reader, prompt string) (string, error) {
	fmt.Print(prompt)
	handle := uintptr(os.Stdin.Fd())
	var mode uint32
	ok, _, _ := getConsoleModeProc.Call(handle, uintptr(unsafe.Pointer(&mode)))
	if ok == 0 {
		line, err := reader.ReadString('\n')
		if err != nil && len(line) == 0 {
			return "", err
		}
		return strings.TrimRight(line, "\r\n"), nil
	}

	if changed, _, callErr := setConsoleModeProc.Call(handle, uintptr(mode&^enableEchoInput)); changed == 0 {
		return "", callErr
	}
	defer setConsoleModeProc.Call(handle, uintptr(mode))

	line, err := reader.ReadString('\n')
	fmt.Println()
	if err != nil && len(line) == 0 {
		return "", err
	}
	return strings.TrimRight(line, "\r\n"), nil
}
