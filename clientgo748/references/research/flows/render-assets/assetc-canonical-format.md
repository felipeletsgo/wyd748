---
id: assetc-canonical-format
title: assetc canonical texture format
subsystem: render-assets
status: UNMAPPED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-09
---

# assetc canonical texture format

> Native maturity: `UNMAPPED`. `WYDASSET` is a new local representation and
> has no native 7.48 function or wire/resource contract to promote to
> `CONTRACT`. Delivery status for this unit is `IMPLEMENTED / AUTOMATED TESTED`.

## Pergunta

Como transformar uma textura oficial `WT10` em uma representação Go
determinística, validável e independente do cabeçalho histórico sem alterar o
asset de origem?

## Fronteira de evidência

- Executável/hash: o mesmo binário 7.48 registrado no front matter; ele define
  a entrada `WT10`, não o formato interno novo.
- Assets oficiais: corpus `.wyt` e `logo1.wyt`, usados como entrada somente
  leitura.
- Source atual: `internal/assets/assets.go`, `internal/assets/canonical.go` e
  `cmd/assetc`.
- TMProject: comparação estrutural secundária; não fornece o formato novo.
- Servidor: não aplicável nesta unidade.
- W2PP, Secrets e Micronics: excluídas.

## Fluxo nativo 7.48

### Entrada observável

O loader nativo recebe `WT10`/TGA e encaminha a imagem para a API gráfica. A
unidade `assetc` preserva esse limite de entrada e não altera o comportamento
do executável histórico.

### Callers

Os callers documentados na ficha `initial-server-logo-texture` continuam sendo
as raízes nativas do carregamento. `assetc` é uma ferramenta offline e não é
chamado pelo executável nativo.

### Função principal

`assets.EncodeTexture` cria o formato interno `WYDASSET` versão `1` a partir de
uma `Texture` já validada. `assets.DecodeTexture` valida e reconstrói a textura.

### Callees

`assetc` chama `LoadWYTFile`, `ParseWYT`, `EncodeTexture` e uma escrita atômica
do arquivo de saída. Não há chamada gráfica ou de servidor.

### Saídas e erros

Saída válida tem cabeçalho de 64 bytes e payload RGBA8. Entradas inválidas,
dimensões inseguras, versão desconhecida, tamanho divergente ou hash alterado
são rejeitados antes do uso pelo renderer.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| converter | WYT existente e saída diferente | `assetc -> LoadWYTFile -> EncodeTexture` | arquivo canônico | cria somente a saída | falha sem sobrescrever entrada |
| instalar saída | diretório permitido | `writeAtomic` | arquivo completo no destino | rename após close | temporário removido em falha |
| carregar canônico | bytes locais | `DecodeTexture` | textura RGBA8 própria | nenhuma mutação externa | rejeita dados adulterados |
| renderer | textura decodificada | integração futura | recurso GPU | upload posterior | não aceita bytes crus |

### Vtables, vptrs e receptores

Não aplicável: a ferramenta é Go offline e não porta vtable nativa.

### Ownership

O encoder cria um buffer novo; o decoder copia o payload. O caller é dono do
resultado e o renderer ainda não recebe o arquivo canônico diretamente.

### Falha parcial

A escrita usa arquivo temporário e rename. Uma falha de leitura, encode ou
instalação não substitui um destino existente por dados incompletos.

### Cleanup e teardown

O arquivo temporário é fechado e removido quando a instalação falha. Não há
recursos gráficos nesta unidade.

### Shutdown

O processo `assetc` encerra após uma conversão; não mantém daemon ou cache
global.

### Logout e relogin

Não aplicável.

## Wire, ABI e recursos

O formato local é little-endian e versionado:

| Offset | Tamanho | Campo |
| ---: | ---: | --- |
| 0 | 8 | `WYDASSET` |
| 8 | 2 | `FormatVersion` |
| 10 | 1 | `AssetKind` (`KindTexture`) |
| 11 | 1 | reservado, zero |
| 12 | 8 | largura/altura `uint32` |
| 20 | 1 | bits de origem |
| 21 | 3 | reservado, zero |
| 24 | 8 | tamanho do payload |
| 32 | 32 | SHA-256 do payload |
| 64 | variável | pixels RGBA8 |

O formato não é wire de servidor e não é criptografado nesta unidade.

## Mapeamento atual

### Source recompilável

`cmd/assetc` aceita `-in` e `-out`, rejeita o mesmo caminho e escreve a saída
atomically. O pacote `assets` impõe limites herdados do parser WYT e verifica o
hash antes de devolver pixels.

### WYD-Go

Não aplicável: nenhum estado de jogo ou contrato server-authoritative é
alterado.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| entrada | `WT10`/TGA | `ParseWYT` validado | comparação de loader | mesma entrada oficial | manter |
| representação | TGA para device | `WYDASSET` RGBA8 local | sem autoridade | formato tipado | modernizar |
| integridade | não há pacote novo | SHA-256 do payload | não aplicável | verificação local | proteger |
| distribuição | arquivo histórico | ainda sem criptografia | não aplicável | próxima unidade | adiar assinatura/pacote |

## Decisões

- Classificar como `MODERNIZACAO_COMPATIVEL`: o formato oficial e o renderer
  observável ainda não são alterados.
- Manter o formato pequeno, determinístico e sem handles gráficos.
- Usar SHA-256 para integridade/cache, não como substituto da assinatura.
- Não incluir chave privada nem declarar segurança de distribuição nesta etapa.

## Lacunas

- manifesto com IDs, versões e hashes de arquivos;
- compressão por chunk;
- assinatura Ed25519;
- criptografia autenticada e rotação de chave;
- carregador runtime que valide o pacote antes do renderer.

## Validação

- Pesquisa: entrada WT10 e formatos confirmados na ficha da primeira unidade.
- Automação: round-trip, cópia independente, adulteração e bytes extras;
  `go test ./...`; conversão real de `logo1.wyt` para arquivo temporário.
- Client real: não aplicável; o formato ainda não foi conectado ao bootstrap.
