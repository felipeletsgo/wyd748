package main

import (
	"flag"
	"fmt"
	"os"

	"wydgo/internal/webcatalog"
)

func main() {
	dir := flag.String("data-dir", "data", "Authoritative game tables directory")
	output := flag.String("output", "", "Public JSON output file (required)")
	flag.Parse()
	if *output == "" || flag.NArg() != 0 {
		fmt.Fprintln(os.Stderr, "web-export requires -output and accepts no positional arguments")
		os.Exit(2)
	}
	if err := webcatalog.Write(*dir, *output); err != nil {
		fmt.Fprintln(os.Stderr, "Catalog export failed:", err)
		os.Exit(1)
	}
	fmt.Println("Public catalog exported:", *output)
}
