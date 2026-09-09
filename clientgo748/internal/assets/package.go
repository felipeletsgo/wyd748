package assets

import (
	"bytes"
	"compress/zlib"
	"crypto/aes"
	"crypto/cipher"
	"crypto/ed25519"
	"crypto/sha256"
	"encoding/binary"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"path"
	"sort"
	"strings"
)

// O pacote protegido é uma unidade de distribuição local. Ele não altera o
// wire do jogo: apenas transporta assets canônicos já convertidos.
const (
	protectedPackageMagic       = "WYDPKG01"
	protectedPackageVersion     = uint16(1)
	protectedPackageHeaderSize  = 32
	protectedManifestHeaderSize = ed25519.SignatureSize
	protectedChunkHeaderSize    = 28
	protectedNonceSize          = 12
	protectedAEADOverhead       = 16
	protectedMaxManifestSize    = 4 << 20
	protectedMaxChunks          = 4096
	protectedMaxChunkSize       = 64 << 20
	protectedMaxPackageSize     = 512 << 20
)

var (
	ErrInvalidProtectedPackage = errors.New("clientgo748: invalid protected asset package")
	ErrUnsupportedProtected    = errors.New("clientgo748: unsupported protected asset package")
	ErrProtectedIntegrity      = errors.New("clientgo748: protected asset integrity failure")
	ErrProtectedKey            = errors.New("clientgo748: invalid protected asset key")
)

// PackageInput contém um asset canônico e a identidade que será assinada no
// manifesto. Data é copiado durante a construção e nunca é mantido pelo
// pacote após BuildProtectedPackage retornar.
type PackageInput struct {
	ID       string
	Kind     AssetKind
	Path     string
	Revision uint32
	Data     []byte
}

// ProtectedPackage contém somente assets que passaram pela assinatura,
// descriptografia, descompressão e verificação de hash. O chamador recebe
// cópias dos bytes para que o pacote mantenha ownership explícito.
type ProtectedPackage struct {
	AssetVersion string
	KeyID        string
	entries      []PackageInput
}

// PackageOptions controla metadados e a fonte de nonce. Random é injetável
// para testes; em produção deve permanecer crypto/rand.Reader.
type PackageOptions struct {
	AssetVersion string
	KeyID        string
	Random       io.Reader
}

type protectedManifest struct {
	FormatVersion uint16                   `json:"format_version"`
	AssetVersion  string                   `json:"asset_version"`
	KeyID         string                   `json:"key_id"`
	Chunks        []protectedManifestChunk `json:"chunks"`
}

type protectedManifestChunk struct {
	Index          uint32                   `json:"index"`
	ID             string                   `json:"id"`
	Kind           AssetKind                `json:"kind"`
	Path           string                   `json:"path"`
	Revision       uint32                   `json:"revision"`
	PlainSize      uint32                   `json:"plain_size"`
	CompressedSize uint32                   `json:"compressed_size"`
	CipherSize     uint32                   `json:"cipher_size"`
	Nonce          [protectedNonceSize]byte `json:"nonce"`
	SHA256         [sha256.Size]byte        `json:"sha256"`
}

type protectedChunk struct {
	index          uint32
	plainSize      uint32
	compressedSize uint32
	cipherSize     uint32
	nonce          [protectedNonceSize]byte
	ciphertext     []byte
}

// BuildProtectedPackage comprime, cifra e assina um conjunto de assets.
// A chave privada é usada somente durante a construção offline; ela não é
// serializada no resultado. O client precisa apenas da chave pública e da
// chave de conteúdo entregue pelo fluxo de sessão autorizado.
func BuildProtectedPackage(inputs []PackageInput, contentKey [32]byte, signer ed25519.PrivateKey, options PackageOptions) ([]byte, error) {
	if len(inputs) == 0 || len(inputs) > protectedMaxChunks {
		return nil, fmt.Errorf("%w: chunk count %d", ErrInvalidProtectedPackage, len(inputs))
	}
	if len(signer) != ed25519.PrivateKeySize || isZeroBytes(signer) {
		return nil, ErrProtectedKey
	}
	if isZeroBytes(contentKey[:]) {
		return nil, ErrProtectedKey
	}
	if options.AssetVersion == "" || options.KeyID == "" {
		return nil, fmt.Errorf("%w: asset version and key id are required", ErrInvalidProtectedPackage)
	}
	random := options.Random
	if random == nil {
		return nil, errors.New("clientgo748: package nonce source is nil")
	}

	ordered := append([]PackageInput(nil), inputs...)
	if err := validateAndSortInputs(ordered); err != nil {
		return nil, err
	}
	block, err := aes.NewCipher(contentKey[:])
	if err != nil {
		return nil, fmt.Errorf("create package cipher: %w", err)
	}
	aead, err := cipher.NewGCM(block)
	if err != nil {
		return nil, fmt.Errorf("create package AEAD: %w", err)
	}

	manifest := protectedManifest{
		FormatVersion: protectedPackageVersion,
		AssetVersion:  options.AssetVersion,
		KeyID:         options.KeyID,
		Chunks:        make([]protectedManifestChunk, len(ordered)),
	}
	compressed := make([][]byte, len(ordered))
	seenNonces := make(map[[protectedNonceSize]byte]struct{}, len(ordered))
	for index, input := range ordered {
		if len(input.Data) == 0 || len(input.Data) > protectedMaxChunkSize {
			return nil, fmt.Errorf("%w: asset %q size %d", ErrInvalidProtectedPackage, input.Path, len(input.Data))
		}
		packed, err := compressProtected(input.Data)
		if err != nil {
			return nil, fmt.Errorf("compress asset %q: %w", input.Path, err)
		}
		if len(packed) > protectedMaxChunkSize {
			return nil, fmt.Errorf("%w: compressed asset %q", ErrInvalidProtectedPackage, input.Path)
		}
		var nonce [protectedNonceSize]byte
		if _, err := io.ReadFull(random, nonce[:]); err != nil {
			return nil, fmt.Errorf("read package nonce: %w", err)
		}
		if _, exists := seenNonces[nonce]; exists {
			return nil, fmt.Errorf("%w: repeated nonce", ErrInvalidProtectedPackage)
		}
		seenNonces[nonce] = struct{}{}
		digest := sha256.Sum256(input.Data)
		cipherSize := len(packed) + aead.Overhead()
		if len(input.Data) > int(^uint32(0)) || len(packed) > int(^uint32(0)) || cipherSize > int(^uint32(0)) {
			return nil, fmt.Errorf("%w: asset %q exceeds wire size", ErrInvalidProtectedPackage, input.Path)
		}
		manifest.Chunks[index] = protectedManifestChunk{
			Index:          uint32(index),
			ID:             input.ID,
			Kind:           input.Kind,
			Path:           input.Path,
			Revision:       input.Revision,
			PlainSize:      uint32(len(input.Data)),
			CompressedSize: uint32(len(packed)),
			CipherSize:     uint32(cipherSize),
			Nonce:          nonce,
			SHA256:         digest,
		}
		compressed[index] = packed
	}
	body, err := marshalProtectedManifest(manifest)
	if err != nil {
		return nil, err
	}
	signature := ed25519.Sign(signer, body)
	manifestBlock := make([]byte, protectedManifestHeaderSize+len(body))
	copy(manifestBlock, signature)
	copy(manifestBlock[protectedManifestHeaderSize:], body)

	chunks := make([]protectedChunk, len(ordered))
	for index, packed := range compressed {
		entry := manifest.Chunks[index]
		ciphertext := aead.Seal(nil, entry.Nonce[:], packed, body)
		if len(ciphertext) != int(entry.CipherSize) {
			return nil, fmt.Errorf("%w: encrypted size mismatch", ErrInvalidProtectedPackage)
		}
		chunks[index] = protectedChunk{
			index:          entry.Index,
			plainSize:      entry.PlainSize,
			compressedSize: entry.CompressedSize,
			cipherSize:     entry.CipherSize,
			nonce:          entry.Nonce,
			ciphertext:     ciphertext,
		}
	}

	totalSize := protectedPackageHeaderSize + len(manifestBlock)
	for _, chunk := range chunks {
		totalSize += protectedChunkHeaderSize + len(chunk.ciphertext)
	}
	if totalSize > protectedMaxPackageSize || totalSize > int(^uint32(0)) {
		return nil, fmt.Errorf("%w: package size %d", ErrInvalidProtectedPackage, totalSize)
	}
	output := make([]byte, totalSize)
	copy(output[:8], protectedPackageMagic)
	binary.LittleEndian.PutUint16(output[8:10], protectedPackageVersion)
	binary.LittleEndian.PutUint32(output[12:16], uint32(len(manifestBlock)))
	binary.LittleEndian.PutUint32(output[16:20], uint32(len(chunks)))
	binary.LittleEndian.PutUint64(output[20:28], uint64(totalSize))
	copy(output[protectedPackageHeaderSize:], manifestBlock)
	offset := protectedPackageHeaderSize + len(manifestBlock)
	for _, chunk := range chunks {
		writeProtectedChunkHeader(output[offset:offset+protectedChunkHeaderSize], chunk)
		offset += protectedChunkHeaderSize
		copy(output[offset:offset+len(chunk.ciphertext)], chunk.ciphertext)
		offset += len(chunk.ciphertext)
	}
	return output, nil
}

// OpenProtectedPackage verifica assinatura antes de liberar qualquer asset.
// Os dados retornados são independentes da entrada e só existem após todos os
// checks de framing, AEAD, descompressão e SHA-256.
func OpenProtectedPackage(data []byte, contentKey [32]byte, publicKey ed25519.PublicKey) (*ProtectedPackage, error) {
	if len(data) < protectedPackageHeaderSize || len(data) > protectedMaxPackageSize {
		return nil, ErrInvalidProtectedPackage
	}
	if string(data[:8]) != protectedPackageMagic {
		return nil, fmt.Errorf("%w: magic", ErrInvalidProtectedPackage)
	}
	if binary.LittleEndian.Uint16(data[8:10]) != protectedPackageVersion {
		return nil, ErrUnsupportedProtected
	}
	if binary.LittleEndian.Uint16(data[10:12]) != 0 || binary.LittleEndian.Uint32(data[28:32]) != 0 {
		return nil, fmt.Errorf("%w: reserved header", ErrInvalidProtectedPackage)
	}
	if len(publicKey) != ed25519.PublicKeySize || isZeroBytes(publicKey) || isZeroBytes(contentKey[:]) {
		return nil, ErrProtectedKey
	}
	manifestSize := binary.LittleEndian.Uint32(data[12:16])
	chunkCount := binary.LittleEndian.Uint32(data[16:20])
	totalSize := binary.LittleEndian.Uint64(data[20:28])
	if manifestSize < protectedManifestHeaderSize || manifestSize > protectedMaxManifestSize || chunkCount == 0 || chunkCount > protectedMaxChunks || totalSize != uint64(len(data)) {
		return nil, fmt.Errorf("%w: header bounds", ErrInvalidProtectedPackage)
	}
	manifestEnd := protectedPackageHeaderSize + int(manifestSize)
	if manifestEnd > len(data) {
		return nil, ErrInvalidProtectedPackage
	}
	signature := data[protectedPackageHeaderSize : protectedPackageHeaderSize+protectedManifestHeaderSize]
	body := data[protectedPackageHeaderSize+protectedManifestHeaderSize : manifestEnd]
	var manifest protectedManifest
	if err := json.Unmarshal(body, &manifest); err != nil {
		return nil, fmt.Errorf("%w: manifest JSON: %v", ErrInvalidProtectedPackage, err)
	}
	canonicalBody, err := marshalProtectedManifest(manifest)
	if err != nil {
		return nil, err
	}
	if !bytes.Equal(canonicalBody, body) {
		return nil, fmt.Errorf("%w: non-canonical manifest", ErrInvalidProtectedPackage)
	}
	if err := validateProtectedManifest(manifest, int(chunkCount)); err != nil {
		return nil, err
	}
	if !ed25519.Verify(publicKey, body, signature) {
		return nil, ErrProtectedIntegrity
	}
	block, err := aes.NewCipher(contentKey[:])
	if err != nil {
		return nil, ErrProtectedKey
	}
	aead, err := cipher.NewGCM(block)
	if err != nil {
		return nil, fmt.Errorf("create package AEAD: %w", err)
	}

	entries := make([]PackageInput, len(manifest.Chunks))
	offset := manifestEnd
	for index, expected := range manifest.Chunks {
		if offset+protectedChunkHeaderSize > len(data) {
			return nil, ErrInvalidProtectedPackage
		}
		chunk, err := readProtectedChunk(data[offset:])
		if err != nil {
			return nil, err
		}
		if chunk.index != expected.Index || chunk.plainSize != expected.PlainSize || chunk.compressedSize != expected.CompressedSize || chunk.cipherSize != expected.CipherSize || chunk.nonce != expected.Nonce {
			return nil, fmt.Errorf("%w: chunk metadata", ErrProtectedIntegrity)
		}
		end := offset + protectedChunkHeaderSize + int(chunk.cipherSize)
		if end > len(data) {
			return nil, ErrInvalidProtectedPackage
		}
		chunk.ciphertext = data[offset+protectedChunkHeaderSize : end]
		packed, err := aead.Open(nil, chunk.nonce[:], chunk.ciphertext, body)
		if err != nil {
			return nil, ErrProtectedIntegrity
		}
		if len(packed) != int(chunk.compressedSize) {
			return nil, fmt.Errorf("%w: compressed size", ErrProtectedIntegrity)
		}
		plain, err := decompressProtected(packed, int(chunk.plainSize))
		if err != nil {
			return nil, err
		}
		digest := sha256.Sum256(plain)
		if digest != expected.SHA256 {
			return nil, ErrProtectedIntegrity
		}
		entries[index] = PackageInput{ID: expected.ID, Kind: expected.Kind, Path: expected.Path, Revision: expected.Revision, Data: plain}
		offset = end
	}
	if offset != len(data) {
		return nil, fmt.Errorf("%w: trailing package bytes", ErrInvalidProtectedPackage)
	}
	return &ProtectedPackage{AssetVersion: manifest.AssetVersion, KeyID: manifest.KeyID, entries: entries}, nil
}

// Assets devolve metadados e cópias dos bytes validados.
func (p *ProtectedPackage) Assets() []PackageInput {
	if p == nil {
		return nil
	}
	assets := make([]PackageInput, len(p.entries))
	for index, entry := range p.entries {
		assets[index] = entry
		assets[index].Data = append([]byte(nil), entry.Data...)
	}
	return assets
}

func validateAndSortInputs(inputs []PackageInput) error {
	sort.SliceStable(inputs, func(i, j int) bool { return inputs[i].Path < inputs[j].Path })
	for index, input := range inputs {
		if input.ID == "" || len(input.ID) > 128 || input.Kind == KindUnknown || input.Kind > KindData {
			return fmt.Errorf("%w: invalid identity for %q", ErrInvalidProtectedPackage, input.Path)
		}
		if err := validateProtectedPath(input.Path); err != nil {
			return err
		}
		if index > 0 && inputs[index-1].Path == input.Path {
			return fmt.Errorf("%w: duplicate path %q", ErrInvalidProtectedPackage, input.Path)
		}
	}
	return nil
}

func validateProtectedManifest(manifest protectedManifest, chunkCount int) error {
	if manifest.FormatVersion != protectedPackageVersion || manifest.AssetVersion == "" || manifest.KeyID == "" || len(manifest.Chunks) != chunkCount {
		return fmt.Errorf("%w: manifest metadata", ErrInvalidProtectedPackage)
	}
	inputs := make([]PackageInput, len(manifest.Chunks))
	seenNonces := make(map[[protectedNonceSize]byte]struct{}, len(manifest.Chunks))
	for index, chunk := range manifest.Chunks {
		if chunk.Index != uint32(index) || chunk.PlainSize == 0 || chunk.PlainSize > protectedMaxChunkSize || chunk.CompressedSize == 0 || chunk.CompressedSize > protectedMaxChunkSize || chunk.CipherSize != chunk.CompressedSize+protectedAEADOverhead || chunk.Kind == KindUnknown || chunk.Kind > KindData {
			return fmt.Errorf("%w: manifest chunk %d", ErrInvalidProtectedPackage, index)
		}
		if _, exists := seenNonces[chunk.Nonce]; exists {
			return fmt.Errorf("%w: repeated manifest nonce", ErrInvalidProtectedPackage)
		}
		seenNonces[chunk.Nonce] = struct{}{}
		inputs[index] = PackageInput{ID: chunk.ID, Kind: chunk.Kind, Path: chunk.Path, Revision: chunk.Revision}
	}
	return validateAndSortManifestInputs(inputs)
}

func validateAndSortManifestInputs(inputs []PackageInput) error {
	paths := make([]string, len(inputs))
	for index, input := range inputs {
		if input.ID == "" || len(input.ID) > 128 {
			return fmt.Errorf("%w: manifest identity", ErrInvalidProtectedPackage)
		}
		if err := validateProtectedPath(input.Path); err != nil {
			return err
		}
		paths[index] = input.Path
	}
	sorted := append([]string(nil), paths...)
	sort.Strings(sorted)
	for index := range paths {
		if paths[index] != sorted[index] {
			return fmt.Errorf("%w: manifest order", ErrInvalidProtectedPackage)
		}
		if index > 0 && paths[index-1] == paths[index] {
			return fmt.Errorf("%w: duplicate manifest path", ErrInvalidProtectedPackage)
		}
	}
	return nil
}

func validateProtectedPath(value string) error {
	if value == "" || strings.Contains(value, "\\") || strings.HasPrefix(value, "/") || strings.ContainsRune(value, 0) || path.Clean(value) != value || value == "." || value == ".." || strings.HasPrefix(value, "../") {
		return fmt.Errorf("%w: unsafe asset path %q", ErrInvalidProtectedPackage, value)
	}
	return nil
}

func marshalProtectedManifest(manifest protectedManifest) ([]byte, error) {
	body, err := json.Marshal(manifest)
	if err != nil {
		return nil, fmt.Errorf("marshal package manifest: %w", err)
	}
	if len(body) > protectedMaxManifestSize-protectedManifestHeaderSize {
		return nil, fmt.Errorf("%w: manifest too large", ErrInvalidProtectedPackage)
	}
	return body, nil
}

func compressProtected(data []byte) ([]byte, error) {
	var output bytes.Buffer
	writer, err := zlib.NewWriterLevel(&output, zlib.BestCompression)
	if err != nil {
		return nil, err
	}
	if _, err := writer.Write(data); err != nil {
		_ = writer.Close()
		return nil, err
	}
	if err := writer.Close(); err != nil {
		return nil, err
	}
	return output.Bytes(), nil
}

func decompressProtected(data []byte, expectedSize int) ([]byte, error) {
	reader, err := zlib.NewReader(bytes.NewReader(data))
	if err != nil {
		return nil, fmt.Errorf("%w: compressed stream", ErrProtectedIntegrity)
	}
	decompressed, readErr := io.ReadAll(io.LimitReader(reader, int64(expectedSize)+1))
	closeErr := reader.Close()
	if readErr != nil || closeErr != nil || len(decompressed) != expectedSize {
		return nil, fmt.Errorf("%w: decompressed size", ErrProtectedIntegrity)
	}
	return decompressed, nil
}

func writeProtectedChunkHeader(dst []byte, chunk protectedChunk) {
	binary.LittleEndian.PutUint32(dst[0:4], chunk.index)
	binary.LittleEndian.PutUint32(dst[4:8], chunk.plainSize)
	binary.LittleEndian.PutUint32(dst[8:12], chunk.compressedSize)
	binary.LittleEndian.PutUint32(dst[12:16], chunk.cipherSize)
	copy(dst[16:28], chunk.nonce[:])
}

func readProtectedChunk(data []byte) (protectedChunk, error) {
	if len(data) < protectedChunkHeaderSize {
		return protectedChunk{}, ErrInvalidProtectedPackage
	}
	return protectedChunk{
		index:          binary.LittleEndian.Uint32(data[0:4]),
		plainSize:      binary.LittleEndian.Uint32(data[4:8]),
		compressedSize: binary.LittleEndian.Uint32(data[8:12]),
		cipherSize:     binary.LittleEndian.Uint32(data[12:16]),
		nonce:          [protectedNonceSize]byte(data[16:28]),
	}, nil
}

func isZeroBytes(value []byte) bool {
	for _, item := range value {
		if item != 0 {
			return false
		}
	}
	return true
}

// Keep hex imported in this unit's documentation-facing API checks. The
// helper is intentionally tiny and makes future key/manifest tooling able to
// render digests without introducing a second encoding convention.
func protectedDigestHex(value [sha256.Size]byte) string { return hex.EncodeToString(value[:]) }
