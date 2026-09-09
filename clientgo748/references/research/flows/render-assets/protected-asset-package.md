---
id: protected-asset-package
title: protected canonical asset package
subsystem: render-assets
status: UNMAPPED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-09
---

# protected canonical asset package

> Native maturity: `UNMAPPED`. `WYDPKG01` is a new local distribution format
> with no equivalent native 7.48 function or resource contract. Delivery status
> for this unit is `IMPLEMENTED / AUTOMATED TESTED`; protected package to
> renderer integration is implemented and the executable bootstrap was run
> with a materialized protected package. The protected and development paths
> rendered the same official logo in the real executable.

## Pergunta

Como distribuir assets canônicos sem permitir substituição silenciosa,
truncamento, traversal de paths ou liberação de dados não autenticados ao
renderer, mantendo a chave privada fora do client?

## Fronteira de evidência

- Executável/hash: `references/ghidra/input/WYD.exe`, SHA-256 registrado no
  front matter; usado apenas para confirmar que o pacote novo não é um formato
  nativo alegado.
- Assets oficiais: entrada dos conversores canônicos; permanecem somente
  leitura.
- Source atual: `internal/assets/canonical.go`, `internal/assets/package.go`,
  `internal/assets/cache.go` e os testes correspondentes.
- WYD-Go/servidor: não aplicável ao formato local; a chave de conteúdo será
  integrada ao contrato de sessão em unidade posterior.
- TMProject: comparação estrutural secundária, sem autoridade sobre este
  formato novo.
- Guias: não aplicáveis ao envelope local.
- W2PP, Secrets e Micronics: excluídas.

## Fluxo nativo 7.48

### Entrada observável

Não há evidência de um pacote `WYDPKG01` no executável nativo. O loader nativo
continua recebendo os assets históricos; este lote introduz somente uma etapa
offline/runtime interna ao novo client.

### Callers

Não aplicável ao nativo. No client Go, `cmd/assetpack` é o caller offline e
`NewProtectedAssetCache` é o caller runtime de `OpenProtectedPackage`.

### Função principal

`BuildProtectedPackage` ordena inputs por path, comprime cada payload, gera um
nonce por chunk, calcula SHA-256, assina o manifesto e serializa o envelope.
`OpenProtectedPackage` valida framing e canonicalização, verifica a assinatura,
descriptografa, descomprime, confere tamanho/hash e entrega cópias dos assets.

### Callees

As dependências são apenas bibliotecas padrão Go: `compress/zlib`,
`crypto/aes`, `crypto/cipher`, `crypto/ed25519`, `crypto/sha256` e JSON.
Nenhum ponteiro, handle gráfico ou chamada de servidor atravessa o pacote.

### Saídas e erros

Saída válida começa com `WYDPKG01`, versão `1`, manifesto assinado e chunks
AES-GCM. Entradas inválidas ou adulteradas retornam erro antes de liberar
qualquer payload ao chamador.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| construir | inputs válidos, chave de conteúdo e signer offline | `BuildProtectedPackage` | bytes completos em memória | cópia/sort dos inputs | rejeita limite, identidade, path ou nonce |
| verificar | bytes recebidos e chave pública | `OpenProtectedPackage` | manifesto validado | nenhuma mutação externa | rejeita magic, versão, assinatura ou canonicalização |
| liberar asset | assinatura e AEAD válidos | `OpenProtectedPackage` | `ProtectedPackage` | cria cópia do payload | rejeita chunk, zlib, tamanho ou SHA inválido |
| consumir | pacote aberto | `Assets`/`LoadTexture` | cópias independentes | nenhuma mutação do pacote | asset ausente, tipo incorreto ou cache fechado |
| encerrar | cache em uso | `ProtectedAssetCache.Close` | cache fechado | zera/libera buffers | operação idempotente |

### Vtables, vptrs e receptores

Não aplicável: a unidade é Go e não porta vtables nativas.

### Ownership

O builder copia e ordena a lista de inputs sem reter os buffers originais. O
pacote aberto mantém seu próprio payload; `Assets` copia novamente os bytes para
que o chamador possa alterá-los sem corromper o cache interno.

### Falha parcial

Qualquer falha de validação, assinatura, AEAD, descompressão ou hash aborta a
abertura inteira. Não existe estado parcialmente liberado.

### Cleanup e teardown

`ProtectedAssetCache` é o owner runtime dos payloads autenticados. A
`Application` fecha o cache depois do renderer e antes da janela; a textura é
decodificada em uma cópia e o renderer assume sua própria cópia gráfica.

### Shutdown/logout/relogin

Ainda não conectado à sessão. O contrato futuro deve descartar a chave de
conteúdo, o cache temporário e os assets de sessão no logout/relogin.

## Wire, ABI e recursos

O envelope é local e little-endian; não é packet do servidor:

| Campo | Tamanho |
| --- | ---: |
| magic `WYDPKG01` | 8 |
| versão/reservado | 4 |
| tamanho do manifesto | 4 |
| quantidade de chunks | 4 |
| tamanho total | 8 |
| reservado | 4 |
| assinatura Ed25519 | 64 |
| manifesto JSON canônico | variável |
| header de chunk | 28 |
| ciphertext AES-GCM | variável |

Cada entrada do manifesto contém identidade, tipo, path, revisão, tamanhos,
nonce e SHA-256. Os limites de pacote, manifesto, chunks e payload são
validados antes de alocações derivadas.

## Mapeamento atual

### Source recompilável

`internal/assets/package.go` contém o builder/leitor, `cache.go` contém a
fronteira `TextureSource`/`ProtectedAssetCache` e `cmd/assetpack` materializa
uma textura WYT sem gravar a chave privada. Os testes cobrem round-trip de
múltiplos assets, assinatura e manifesto adulterados, chave de conteúdo
incorreta, path traversal, nonce repetido, truncamento, cópia independente,
asset ausente, tipo incorreto, fechamento e integração da `Application`.

### WYD-Go

Não aplicável nesta unidade: nenhum estado de personagem, regra de jogo ou
packet server-authoritative é alterado. A chave de conteúdo continua sendo
configuração local temporária até a unidade de sessão.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| entrada | assets históricos | `WYDASSET` validado | comparação estrutural | não aplicável | manter origem |
| distribuição | sem pacote equivalente localizado | `WYDPKG01` | não aplicável | não aplicável | modernizar internamente |
| autenticidade | sem contrato equivalente localizado | Ed25519 | não aplicável | não aplicável | proteger |
| confidencialidade | sem contrato equivalente localizado | AES-256-GCM por chunk | não aplicável | chave de sessão futura | proteger |
| autoridade de gameplay | não aplicável | nenhum estado de jogo | não aplicável | servidor continua autoritativo | manter |

## Decisões

- Classificar a unidade como `MODERNIZACAO_COMPATIVEL` interna e não como
  paridade nativa.
- Verificar a assinatura antes de descriptografar ou entregar qualquer asset.
- Manter a chave privada somente no empacotador offline.
- Exigir nonces distintos e rejeitar manifestos fora da ordem canônica.
- Não alterar assets oficiais nem introduzir dependência no wire do jogo.

## Lacunas

- vincular a chave de conteúdo à sessão autenticada;
- definir rotação/revogação de `KeyID`;

## Validação

- Automação: testes focados aprovados para pacote, cache e `Application`,
  incluindo todos os casos de integridade listados nesta ficha.
- Executável: `cmd/assetpack` materializou `bin/protected-logo.wydpkg` a partir
  de `CLIENT OFICIAL 7.48/UI/logo1.wyt`; `tools/Test-ClientBootstrap.ps1` foi
  executado nos modos default e protegido por `-EnvironmentFile`, com
  `WYD_ASSET_PACKAGE`, `WYD_ASSET_CONTENT_KEY_HEX`,
  `WYD_ASSET_PUBLIC_KEY_HEX` e `WYD_ASSET_LOGO_PATH=UI/logo1.wydasset`. Ambos
  passaram por criação da janela, resize, `WM_CLOSE` e `Alt+F4`.
- Visual: capturas do client area dos dois caminhos foram inspecionadas e
  exibiram o mesmo logo oficial, com dimensões e enquadramento equivalentes.
- Pesquisa: a unidade foi marcada `UNMAPPED` porque o formato é novo e não há
  função nativa equivalente comprovada.
- Client real: `CLIENT TESTED` para o bootstrap e render do logo protegido.
