//go:build !windows

package main

import "bufio"

func readPassword(reader *bufio.Reader, prompt string) (string, error) {
	return readLine(reader, prompt)
}
