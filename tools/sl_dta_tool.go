package main

import (
	"encoding/binary"
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
	"strings"
)

type loginEntry struct {
	Name      string
	Address   string
	Port      string
	FullUsers string
	BusyUsers string
}

func main() {
	if len(os.Args) < 2 {
		usage()
		os.Exit(1)
	}

	switch os.Args[1] {
	case "decode":
		if err := decodeCmd(os.Args[2:]); err != nil {
			fmt.Fprintf(os.Stderr, "decode failed: %v\n", err)
			os.Exit(1)
		}
	case "encode":
		if err := encodeCmd(os.Args[2:]); err != nil {
			fmt.Fprintf(os.Stderr, "encode failed: %v\n", err)
			os.Exit(1)
		}
	default:
		usage()
		os.Exit(1)
	}
}

func usage() {
	fmt.Println("sl_dta_tool: encode/decode Last Chaos sl.dta login config")
	fmt.Println("")
	fmt.Println("Usage:")
	fmt.Println("  go run client-source/tools/sl_dta_tool.go decode -in sl.dta")
	fmt.Println("  go run client-source/tools/sl_dta_tool.go encode -out sl.dta -entry \"Local 127.0.0.1 5500 1000 300\"")
}

func decodeCmd(args []string) error {
	fs := flag.NewFlagSet("decode", flag.ContinueOnError)
	inPath := fs.String("in", "", "input sl.dta path")
	if err := fs.Parse(args); err != nil {
		return err
	}
	if *inPath == "" {
		return errors.New("missing -in")
	}

	f, err := os.Open(*inPath)
	if err != nil {
		return err
	}
	defer f.Close()

	entries, err := decodeSLDTA(f)
	if err != nil {
		return err
	}

	for i, e := range entries {
		fmt.Printf("%d: name=%q address=%q port=%q full=%q busy=%q\n",
			i, e.Name, e.Address, e.Port, e.FullUsers, e.BusyUsers)
	}
	return nil
}

func encodeCmd(args []string) error {
	fs := flag.NewFlagSet("encode", flag.ContinueOnError)
	outPath := fs.String("out", "", "output sl.dta path")
	entryArg := fs.String("entry", "", "entry as: \"name address port full busy\"")
	if err := fs.Parse(args); err != nil {
		return err
	}
	if *outPath == "" {
		return errors.New("missing -out")
	}
	if *entryArg == "" {
		return errors.New("missing -entry")
	}

	parts := strings.Fields(*entryArg)
	if len(parts) != 5 {
		return errors.New("-entry must contain exactly 5 fields: name address port full busy")
	}

	entry := loginEntry{
		Name:      parts[0],
		Address:   parts[1],
		Port:      parts[2],
		FullUsers: parts[3],
		BusyUsers: parts[4],
	}

	f, err := os.Create(*outPath)
	if err != nil {
		return err
	}
	defer f.Close()

	if err := encodeSLDTA(f, []loginEntry{entry}); err != nil {
		return err
	}
	return nil
}

func decodeSLDTA(r io.Reader) ([]loginEntry, error) {
	// The client reads six bytes first, then key/int pairs in little-endian form.
	six := make([]byte, 6)
	if _, err := io.ReadFull(r, six); err != nil {
		return nil, err
	}

	var iKey int32
	if err := binary.Read(r, binary.LittleEndian, &iKey); err != nil {
		return nil, err
	}

	key, err := readByte(r)
	if err != nil {
		return nil, err
	}

	var encodedServerCount int32
	if err := binary.Read(r, binary.LittleEndian, &encodedServerCount); err != nil {
		return nil, err
	}

	serverCount := int(encodedServerCount - iKey)
	iKey = encodedServerCount
	if serverCount < 0 || serverCount > 1024 {
		return nil, fmt.Errorf("invalid server count %d", serverCount)
	}

	out := make([]loginEntry, 0, serverCount)
	for i := 0; i < serverCount; i++ {
		var encodedLen int32
		if err := binary.Read(r, binary.LittleEndian, &encodedLen); err != nil {
			return nil, err
		}
		lineLen := int(encodedLen - iKey)
		iKey = encodedLen

		if lineLen <= 0 || lineLen > 4096 {
			return nil, fmt.Errorf("invalid record size %d", lineLen)
		}

		plain := make([]byte, lineLen)
		for j := 0; j < lineLen; j++ {
			data, err := readByte(r)
			if err != nil {
				return nil, err
			}
			plain[j] = data - key
			key = data
		}

		fields := strings.Fields(string(plain))
		if len(fields) != 5 {
			return nil, fmt.Errorf("record %d has %d fields, expected 5", i, len(fields))
		}

		out = append(out, loginEntry{
			Name:      fields[0],
			Address:   fields[1],
			Port:      fields[2],
			FullUsers: fields[3],
			BusyUsers: fields[4],
		})
	}

	return out, nil
}

func encodeSLDTA(w io.Writer, entries []loginEntry) error {
	// Header bytes are consumed but not validated by the client reader.
	header := []byte{0x11, 0x22, 0x33, 0x44, 0x55, 0x66}
	if _, err := w.Write(header); err != nil {
		return err
	}

	iKey := int32(0x10203040)
	if err := binary.Write(w, binary.LittleEndian, iKey); err != nil {
		return err
	}

	key := byte(0x5A)
	if _, err := w.Write([]byte{key}); err != nil {
		return err
	}

	encodedServerCount := int32(len(entries)) + iKey
	if err := binary.Write(w, binary.LittleEndian, encodedServerCount); err != nil {
		return err
	}
	iKey = encodedServerCount

	for _, e := range entries {
		line := fmt.Sprintf("%s %s %s %s %s", e.Name, e.Address, e.Port, e.FullUsers, e.BusyUsers)
		plain := []byte(line)

		encodedLen := int32(len(plain)) + iKey
		if err := binary.Write(w, binary.LittleEndian, encodedLen); err != nil {
			return err
		}
		iKey = encodedLen

		for _, p := range plain {
			data := p + key
			if _, err := w.Write([]byte{data}); err != nil {
				return err
			}
			key = data
		}
	}
	return nil
}

func readByte(r io.Reader) (byte, error) {
	var b [1]byte
	if _, err := io.ReadFull(r, b[:]); err != nil {
		return 0, err
	}
	return b[0], nil
}
