---
id: keepalive-ping
title: Keepalive periódico 0x3A0
subsystem: transport-session
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-06
---

# Keepalive periódico 0x3A0

## Pergunta

Qual frame mantém a sessão ativa nas cenas Field e SelectChar do client 7.48,
e o servidor precisa responder?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; export focado
  `exports/keepalive-ping-flow.tsv`, SHA-256
  `A04C174449DF52C63786AAAA6695EDA033F67BDAEFA0EFDB62BBE343D02E3A12`.
- Assets: `NÃO APLICÁVEL`; o fluxo não usa UI ou recurso.
- Source atual: `TMFieldScene.cpp`, `TMSelectCharScene.cpp` e `Basedef.h`.
- Servidor: `internal/game/protocol_misc.go`, `security.go`, `world.go` e testes.
- TMProject posterior/guias: `NÃO APLICÁVEL`; W2PP, Secrets e Micronics foram
  excluídos da decisão.

## Fluxo nativo 7.48

### Entrada observável

- Evento: tick periódico em Field ou SelectChar.
- Precondições: cena e sessão ativas; intervalo maior que 250 s na Field ou
  300 s na seleção desde o marcador global.
- Saída: frame `0x3A0/12`; não existe resposta específica.

### Callers

`FUN_0047E9B6` é o tick auxiliar da FieldScene e `FUN_004A2AEF` o tick da
SelectCharScene. Ambos são alcançados pelo lifecycle de frame das respectivas
cenas e constroem o mesmo opcode.

### Função principal

`FUN_0047E9B6` zera 12 bytes, grava ID do personagem e `0x3A0`, então chama
`FUN_0055F2DD`. `FUN_004A2AEF` zera o mesmo tamanho, usa ID zero e chama
`FUN_0042550E`. A diferença de ID preserva a fase; não altera o contrato.

### Callees

- `FUN_0055F2DD` envia o header na Field.
- `FUN_0042550E` envia o header na SelectChar.
- `FUN_0055890A` aceita `0x3A0` somente com 12 bytes.

### Saídas e erros

Antes do intervalo não há envio. O frame não carrega timestamp, score, posição
ou desafio e não solicita resposta. Falha de transporte segue o teardown da
sessão; o ping não possui retry próprio.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| tick Field | >250 s e humano ativo | `FUN_0047E9B6 -> FUN_0055F2DD` | sessão observada ativa | envia ID local | intervalo menor não envia |
| tick SelectChar | >300 s e seleção ativa | `FUN_004A2AEF -> FUN_0042550E` | sessão observada ativa | envia ID zero | intervalo menor não envia |
| servidor recebe | qualquer fase válida | `onPing` | sem mutação | atividade já foi observada pelo transporte | não responde |
| disconnect | transporte falha | lifecycle geral | cena retorna/encerra | cleanup do socket | ping não faz retry |

### Vtables, vptrs e receptores

Os ticks pertencem às cenas Field e SelectChar já documentadas; este corte não
altera slots. `0x3A0` é C->S e não possui receptor no client.

### Ownership

O buffer de 12 bytes vive na pilha e é emprestado ao emissor. Não há payload,
objeto, recurso ou estado persistente de ownership.

### Falha parcial

Falha de envio não cria estado parcial. O servidor reconhece o frame depois do
gate global; `onPing` não altera gameplay nem publica packet.

### Cleanup e teardown

O ping não retém memória. O socket e a cena seguem o teardown geral.

### Shutdown

Ao iniciar shutdown, a sessão deixa de processar novos comandos; nenhum ping
pendente precisa ser drenado ou persistido.

### Logout e relogin

O opcode é aceito na seleção e no mundo. Após logout, a nova cena usa seu
próprio intervalo e ID; nenhum estado do ping anterior é restaurado.

## Wire, ABI e recursos

Direção C->S, opcode `0x3A0`, exatamente 12 bytes: somente `MSG_STANDARD`.
Field usa `Header.ID` do personagem; SelectChar usa zero. O gate nativo e o
servidor exigem o mesmo tamanho. Não há resposta S->C nem asset.

## Mapeamento atual

### Source recompilável

Os dois emissores já usam `MSG_Ping_Opcode` e `MSG_STANDARD`; o opcode ainda
fica em Basedef. A extração pode preservar todos os bytes e timers.

### WYD-Go

`OpPing` é permitido em qualquer fase, exige 12 bytes e `onPing` não responde.
Teste existente confirma que a fila de saída permanece inalterada.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| wire | header 12B | equivalente | não decide ABI | gate equivalente | extrair |
| fase/ID | Field=char, SelectChar=0 | preservado | N/A | ID não é autoridade | manter |
| resposta | nenhuma | nenhuma | N/A | nenhuma | manter |

## Decisões

- Classificar como `PARIDADE_NATIVA/CONTRACT`.
- Extrair `KeepalivePingPacket.h` com opcode e assert do header.
- Preservar os dois timers, IDs e emissores.
- Não adicionar entrada ao dispatcher recebido.

## Lacunas

- Permanência real em Field/SelectChar, disconnect e relogin ainda não foram
  exercitados no `project.exe`.

## Validação

- Pesquisa: Ghidra read-only terminou sem `SCRIPT ERROR`, com hash esperado e
  funções/callers/callees dos dois emissores, transportes e gate.
- Automação: `go test -count=1 ./...` passou; Debug e Release via
  `Build-Client.ps1` passaram com 1930 checks/asserts. Fixtures cobrem Field com
  ID local e SelectChar com ID zero. Release instalado com SHA-256
  `F80936677E75630FE31EF47D799DC3672ACFF1C8F9531C24864B8934A9B67DBA`.
- Client real: não executado; não `CLIENT_TESTED`.
