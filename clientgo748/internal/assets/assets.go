// Package assets define os tipos canônicos que serão usados pelo client Go.
//
// Os loaders ainda não foram implementados. Nesta etapa os tipos documentam a
// fronteira entre os arquivos originais 7.48 e o renderer. A conversão deverá
// ser feita por uma ferramenta separada, sem sobrescrever a fonte original.
package assets

// FormatVersion identifica a versão do formato normalizado do client Go.
// Alterações incompatíveis devem criar uma nova versão e invalidar o cache
// derivado correspondente.
const FormatVersion uint16 = 1

// AssetKind identifica a família lógica de um asset convertido.
type AssetKind uint8

const (
	KindUnknown AssetKind = iota
	KindTexture
	KindMesh
	KindAnimation
	KindMap
	KindUI
	KindData
)

// AssetRef descreve um asset no manifesto sem assumir como ele será carregado
// pela API gráfica. Hash e tamanho são metadados de integridade; não são uma
// implementação de autenticação por si só.
type AssetRef struct {
	ID       string
	Kind     AssetKind
	Path     string
	Size     uint64
	SHA256   [32]byte
	Revision uint32
}
