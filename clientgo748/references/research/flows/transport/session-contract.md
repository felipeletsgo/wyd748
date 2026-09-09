---
id: native-748-transport-session
title: Native 7.48 transport session from connect to framed packet teardown
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-09
---

# Native 7.48 transport session from connect to framed packet teardown

## Pergunta

Qual é o contrato observável entre a conexão TCP do client 7.48 e o primeiro
packet recebido/enviado, incluindo handshake, framing, criptografia, checksum,
fragmentação, limites e teardown?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 acima.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr` e corpus
  `C:/Users/felipe/Tools/GhidraAnalysis/20260821/decompiled`.
- Assets: não aplicável; o transporte não interpreta assets.
- Source atual: `clientgo748/internal/protocol/protocol.go` e
  `clientgo748/internal/protocol/crypt.go`.
- Servidor: `internal/wire/packet.go`, `internal/wire/crypt.go` e
  `internal/net/session.go`.

### Matriz de fontes

| Fonte | Estado | Justificativa |
| --- | --- | --- |
| Binário nativo WYD 7.48 | UTILIZADA | confirmou o parser, os limites, o handshake e o teardown. |
| Descompilação estudada no Ghidra | UTILIZADA | forneceu callers/callees e os branches de socket e checksum. |
| WYD-Go atual | UTILIZADA | servidor e testes definem o contrato operacional que o novo client precisa consumir. |
| Assets oficiais 7.48 | NÃO APLICÁVEL | nenhuma leitura de asset ocorre nesta fronteira. |
| TMProject | UTILIZADA | somente comparação semântica de organização; não forneceu ABI ou bytes. |
| Guias/documentação secundária | NÃO APLICÁVEL | não substituem a evidência nativa nesta fronteira. |
| W2PP, Secrets e Micronics | NÃO APLICÁVEL | fontes excluídas desta campanha. |

## Fluxo nativo 7.48

### Entrada observável

1. O socket TCP é conectado.
2. O client envia os quatro bytes `11 F3 11 1F` antes de qualquer `_MSG`.
3. O parser recebe um stream que pode fragmentar ou concatenar frames.
4. Cada frame começa com `Size` little-endian no offset `0`.

### Callers

- `FUN_0055DAB8` (`0055dab8_FUN_0055dab8.c`) despacha os eventos do socket,
  incluindo `FD_READ` e `FD_CLOSE`.
- Os caminhos de conexão `FUN_00423F37`, `FUN_00424146` e `FUN_00424337`
  encaminham a criação/registro do socket para o dispatcher.

### Função principal

- `FUN_004245DB` (`004245db_FUN_004245db.c`) lê dados do socket e encaminha o
  buffer para o parser de `_MSG`.
- `FUN_00424642` (`00424642_FUN_00424642.c`) valida o tamanho, decifra do
  offset `4`, acumula o checksum e libera o packet somente quando a soma bate.

### Callees

- `FUN_00424C2C` (`00424c2c_FUN_00424c2c.c`) calcula a chave/posição usada pela
  tabela `DAT_005B1D10/DAT_005B1D11`.
- `FUN_00424DFE` (`00424dfe_FUN_00424dfe.c`) escreve, cifra e enfileira um
  `_MSG` de saída.
- `FUN_00423D7A` (`00423d7a_FUN_00423d7a.c`) encerra a sessão e libera o estado
  do socket.

### Saídas e erros

- Tamanho menor que `12` ou maior que `0x1B58` é rejeitado antes de alocar o
  corpo.
- Checksum inválido rejeita o frame.
- O stream é consumido em frames completos; bytes incompletos permanecem para
  o próximo evento de leitura.
- `FD_CLOSE` e falha de leitura entram no mesmo caminho de teardown.
- A fila de saída nativa possui limite de `0x20000` bytes; overflow termina a
  sessão em vez de bloquear indefinidamente.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| Novo socket | sessão inexistente | `FUN_00423F37`/`FUN_00424146` | conectado | registra socket e callbacks | falha de conexão encerra sem packet |
| Handshake | TCP conectado | envio `11 F3 11 1F` | transporte habilitado | servidor pode iniciar leitura | handshake ausente/inválido encerra |
| `FD_READ` parcial | buffer incompleto | `FUN_004245DB` → `FUN_00424642` | bytes pendentes | preserva fragmento | nenhum dispatch parcial |
| Frame completo | `Size` válido | `FUN_00424642` | packet decifrado | valida checksum e entrega ao dispatcher | tamanho/checksum inválido encerra |
| `FD_CLOSE` | socket ativo | `FUN_0055DAB8` | fechado | remove callbacks e libera fila | teardown idempotente |
| falha de envio | fila/socket ativo | `FUN_00424DFE` | fechado ou drenando | descarta/encerra conforme fila | não bloquear o loop principal |

### Vtables, vptrs e receptores

Não há vtable exposta nesta fronteira. O receptor observável é o dispatcher de
eventos do socket (`FUN_0055DAB8`), que encaminha `FD_READ` e `FD_CLOSE` para o
estado da sessão.

### Ownership

- O buffer de leitura pertence à sessão até o parser produzir um frame completo.
- O packet entregue ao dispatcher precisa ter armazenamento próprio; nenhum
  handler pode reter um slice do buffer de recv.
- A fila de saída pertence à sessão e só é liberada no teardown.
- A implementação Go mantém `Packet.Raw` como cópia independente do buffer TCP.

### Falha parcial

- Conexão aceita mas handshake falha: fecha o socket sem criar estado de jogo.
- Frame incompleto: aguarda mais bytes, respeitando o deadline.
- Frame inválido: limpa o acumulador e encerra a sessão.
- Falha ao enviar handshake: fecha a conexão e não permite reutilização.

### Cleanup e teardown

O fechamento deve ser idempotente, fechar o socket, interromper leituras
pendentes e liberar a fila. A sessão Go inicia uma única goroutine de leitura,
que publica packets e desconexão numa fila limitada. A thread principal drena
essa fila antes do update da cena; a goroutine do socket não chama gameplay nem
muta estado de login diretamente.

### Shutdown

O shutdown da aplicação fecha a sessão antes de destruir a cena e os recursos
do renderer. Nenhum handler de packet deve executar depois de `Close`. A
integração é feita por `internal/app.Application`, que chama `Connect` após
inicializar o renderer e `Close` antes de destruir cenas e renderer.

### Logout e relogin

O objeto de sessão fechado não é reutilizado. Relogin cria uma nova sessão,
repete o handshake e começa com um acumulador vazio; estado de personagem,
dispatcher e cena serão conectados em fichas posteriores.

## Wire, ABI e recursos

- Cabeçalho: 12 bytes; `Size:u16`, `KeyWord:u8`, `CheckSum:u8`, `Type:u16`,
  `ID:u16`, `Tick:u32`, todos little-endian onde aplicável.
- Bytes `0..3` permanecem no frame; a cifra começa no offset `4`.
- `KeyWord` indexa a tabela nativa de 512 bytes, cujo SHA-256 é
  `e47996fe5e92de5d86d503d5415665f1f464bf344370c709be450607dd97cf8f`.
- A soma é `encryptedSum - plainSum` em aritmética de byte.
- A implementação Go usa `MaxFrameSize = 0x1B58`, conforme o parser nativo.
- O servidor atual ainda declara `internal/wire.MaxPacketSize = 8192`; essa
  divergência permanece aberta e não será alterada nesta unidade sem teste de
  compatibilidade server/client.
- A fila nativa de saída tem limite de `0x20000`; a primeira sessão Go usa
  escrita síncrona e não declara uma fila assíncrona antes de haver contrato de
  backpressure.

## Mapeamento atual

### Source recompilável

- `clientgo748/internal/protocol/protocol.go` contém framing, stream decoder,
  handshake e `ClientSession`.
- `clientgo748/internal/protocol/crypt.go` contém a tabela e as operações
  byte-a-byte.
- `clientgo748/internal/protocol/protocol_test.go` cobre round-trip, fingerprint,
  fragmentação, checksum, tamanho, handshake, escrita parcial e leitura.
- `clientgo748/internal/app/application.go` conecta a sessão opcional ao
  lifecycle da aplicação; `application_test.go` verifica `Connect`, os
  callbacks de estado conectado/desconectado, falhas parciais, `Close` e a
  ordem de fechamento da sessão antes do renderer.

### WYD-Go

- `internal/wire/packet.go` e `internal/wire/crypt.go` são a implementação
  server-side de referência para os bytes do contrato.
- `internal/net/session.go` confirma o handshake, os deadlines e o lifecycle
  da sessão do servidor.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| Header de 12 bytes | confirmado | implementado | comparação apenas | confirmado | PARIDADE_NATIVA |
| Handshake `11 F3 11 1F` | confirmado | implementado | não usado como ABI | confirmado | PARIDADE_NATIVA |
| Cifra/checksum | confirmado | implementado com tabela fingerprintada | sem autoridade | confirmado | PARIDADE_NATIVA |
| Stream fragmentado/concatenado | confirmado pelo parser/eventos | implementado | comparação estrutural | parser por leitura completa | MODERNIZACAO_COMPATIVEL |
| Ownership e fechamento Go | não possui a mesma ABI pública | explícito | não reutilizado | sessão autoritativa | MODERNIZACAO_COMPATIVEL |
| Opcode de login | ainda não rastreado nesta ficha | não implementado | não autoriza invenção | contratos existentes aguardam ficha | CONTRATO POSTERIOR |

## Decisões

1. O transporte será uma camada independente do dispatcher de gameplay.
2. Nenhum opcode de login ou gameplay será adicionado nesta unidade.
3. A tabela nativa será mantida somente porque sua igualdade byte a byte foi
   comprovada; não será substituída por uma tabela do TMProject.
4. A sessão fechada não será reutilizada; relogin criará um novo owner.
5. Toda mensagem exibida ao usuário será definida em inglês nas camadas de UI,
   não no transporte.

## Lacunas

- O fluxo nativo completo de autenticação ainda precisa de uma ficha própria.
- A política de escolha de `KeyWord` para cada packet de saída precisa ser
  comparada com a sequência observável do client e do servidor.
- A divergência `0x1B58` versus `8192` precisa de decisão coordenada.
- A política de escolha de `KeyWord` para cada packet de saída precisa ser
  confirmada no fluxo de autenticação.
- O fluxo nativo completo de autenticação, seleção e entrada no mundo precisa de
  uma ficha própria antes da implementação dos opcodes.
- O teste com o executável Go e o servidor real ainda não foi executado.

## Validação

- Pesquisa: parser e callers/callees conferidos no executável nativo e no corpus
  Ghidra identificado pelo SHA-256 acima.
- Automação: testes de protocolo e lifecycle da aplicação foram adicionados; os
  gates do client serão executados antes do commit desta unidade.
- Client real: pendente; somente será marcado `CLIENT_TESTED` após conexão real,
  handshake, um packet válido, logout e relogin no executável Go.
