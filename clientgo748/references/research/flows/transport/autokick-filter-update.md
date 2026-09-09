---
id: autokick-filter-update
title: Atualização remota e consumo dos filtros AutoKick
subsystem: transport
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Atualização remota e consumo dos filtros AutoKick

## Pergunta

O que o opcode `0x2C8` configura no client nativo 7.48, como os dados são
consumidos durante o tick da FieldScene e esse comportamento deve ser ativado
no TMProject atual?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter; referência histórica somente leitura.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; xrefs focados de
  `FUN_00492E7D`, `FUN_004929FE`, `FUN_0047E8E2`, `FUN_0047E9B6`,
  `FUN_004776C3`, construtor `FUN_004343A4`, destrutor `FUN_004358DA` e
  deleting destructor `FUN_00493BC0`.
- Assets: N/A; o fluxo não carrega recurso visual.
- Source atual: `TMFieldScene.cpp`, `TMFieldScene.h` e `Basedef.h` em
  `client-source/tmproject/Projects/TMProject/`.
- Servidor: busca completa pelos opcodes/dispatchers atuais do WYD-Go; não há
  produtor de `0x2C8`.

## Fluxo nativo 7.48

### Entrada observável

- Evento: a FieldScene recebe do servidor um packet de opcode `0x2C8`.
- Precondições e estado inicial: existe uma FieldScene ativa; o construtor
  inicializou cinco buffers internos de 96 bytes e o timestamp da última
  atualização/inspeção.
- Saída observável: um dos cinco filtros é substituído e, em ticks posteriores,
  pode causar envio de logout, espera de quatro segundos e retorno forçado para
  fora do mundo.

### Callers

`FUN_00492E7D` é o dispatcher virtual de packets da FieldScene. O branch
`0x2C8`, no callsite `0x004935A5`, chama diretamente `FUN_004929FE`.
`FUN_00492E7D` aparece como referência `DATA` em `0x005A4298`, slot 1 da
vtable cuja base é `0x005A4294`.

O consumidor `FUN_0047E9B6` não é chamado pelo handler. Ele é chamado pelo
tick virtual `FUN_004776C3`, no callsite `0x0047DEC9`. O tick aparece no slot 8
da mesma vtable, em `0x005A42B4`.

### Função principal

`FUN_004929FE` atualiza `FieldScene+0x274D8` com o tick atual, limita o índice
em `packet+0x0C` ao intervalo `0..4`, aplica XOR `0x61` aos 96 bytes em
`packet+0x10`, força `packet[0x6F] = 0` e entrega índice/texto a
`FUN_0047E8E2`.

`FUN_0047E8E2` normaliza o texto para uppercase e substitui exatamente um dos
buffers em `FieldScene+0x28C28`, `+0x28C88`, `+0x28CE8`, `+0x28D48` ou
`+0x28DA8`. Não há alocação, fila, callback ou cópia com ownership transferido.

### Callees

- `FUN_0059E5FE`: normalização para uppercase antes de armazenar/comparar.
- `FUN_0058F078`: cópia do filtro normalizado para o buffer selecionado.
- `FUN_0047E9B6`: consumidor periódico; consulta TCP, arquivos/caminhos,
  títulos e classes de janelas.
- `GetTcpTable`, `GetSystemDirectoryA`, `GetDesktopWindow`, `GetWindow`,
  `GetWindowTextA` e `GetClassNameA`: fontes observadas pelo consumidor.
- `FUN_0055F2DD`: envia packets, inclusive `0x215/12` quando há match.
- `FUN_00403DF2`: espera quatro segundos antes da saída forçada.
- `FUN_00431159`: registra `[CharacterLogout] - Auto`.

### Saídas e erros

Índice negativo vira zero e índice acima de quatro vira quatro; não existe
rejeição explícita. O byte final é sempre zerado após o XOR, portanto o texto
consumido fica terminado mesmo se os 96 bytes recebidos não contiverem NUL.

Falha ao alocar a tabela TCP encerra somente aquela inspeção. APIs de arquivo
ou janela que não encontram correspondência deixam o usuário conectado. Uma
correspondência envia `0x215/12`, espera quatro segundos, invoca o caminho
virtual de saída e registra o logout automático.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| construir Field | nova cena | `FUN_004343A4` | cinco filtros e timers inicializados | inclui defaults legados em parte dos filtros | nenhuma configuração remota exigida |
| receber `0x2C8` | Field ativa | `FUN_00492E7D -> FUN_004929FE` | um filtro substituído | XOR, NUL e uppercase | índice é saturado para `0..4` |
| tick antes da janela | Field ativa | `FUN_004776C3` | estado preservado | nenhum scan AutoKick completo | retorna ao tick normal |
| tick de inspeção sem match | janela temporal atingida | `FUN_0047E9B6` | timestamp atualizado | enumera TCP/arquivo/janelas | alocação TCP pode abortar a rodada |
| tick de inspeção com match | algum critério coincide | `FUN_0047E9B6` | saída do mundo iniciada | envia `0x215`, espera 4 s e registra logout | não há rollback local |
| destruir Field | troca/saída de cena | `FUN_004358DA` | buffers morrem com a cena | cleanup geral da Field | nenhum teardown AutoKick separado |

### Vtables, vptrs e receptores

O receptor é a FieldScene ativa. `FUN_004343A4` instala o vptr
`0x005A4294`; `FUN_004358DA` restaura o mesmo vptr durante a destruição e
`FUN_00493BC0`, no slot zero, executa o destrutor e opcionalmente libera o
objeto. O dispatcher está no slot 1 e o tick no slot 8. Os filtros são campos
inline da própria cena, não objetos apontados por uma vtable própria.

### Ownership

A FieldScene possui os cinco buffers e o timestamp. O packet é memória
emprestada pelo dispatcher e é decodificado in-place antes da cópia. A tabela
TCP temporária pertence à rodada de inspeção e é liberada quando alocada. Não
há estado AutoKick persistido no servidor Go atual.

### Falha parcial

Uma atualização altera somente um filtro. Se a sessão/cena terminar em
seguida, a configuração morre com a FieldScene. Falha da enumeração TCP não
limpa filtros nem inicia logout. Depois que um match dispara, o nativo aguarda
de forma bloqueante e segue para logout; não foi observado cancelamento entre
envio e saída.

### Cleanup e teardown

Os buffers são inline e não exigem `free`. `FUN_004358DA` executa o teardown
geral da FieldScene; não há handle, thread, timer ou callback AutoKick
registrado para remover. A alocação temporária de `GetTcpTable` é liberada na
própria inspeção.

### Shutdown

O shutdown destrói a FieldScene pelo caminho virtual normal. Como o fluxo não
possui worker ou owner global próprio, não há teardown AutoKick adicional. A
espera bloqueante de quatro segundos pertence ao caminho nativo de match, não
a um recurso que sobreviva à cena.

### Logout e relogin

O match envia `0x215/12` e entra no fluxo já fechado em
[`../lifecycle/character-logout-selectchar-relogin.md`](../lifecycle/character-logout-selectchar-relogin.md).
Ao relogin, uma nova FieldScene reconstrói os defaults; updates recebidos pela
cena anterior não são persistidos. O servidor Go não envia `0x2C8`, portanto o
client recompilado não possui configuração remota a restaurar.

## Wire, ABI e recursos

Direção observada: servidor para client. Opcode: `0x2C8`.

O handler acessa até `packet+0x6F`, comprovando somente um tamanho mínimo de
112 bytes. O tamanho exato do packet, padding e eventual conteúdo após esse
offset permanecem não confirmados porque o emissor/construtor nativo ainda não
foi localizado.

| Offset | Tamanho comprovado | Semântica observada |
| --- | ---: | --- |
| `0x00` | 12 | header padrão usado pelo dispatcher |
| `0x0C` | 4 | índice signed saturado para `0..4` |
| `0x10` | 96 | texto ofuscado com XOR `0x61`; último byte forçado a NUL |

`STRUCT_AUTOKICK { char route[4][128]; }` da source herdada mede 512 bytes,
tem quatro entradas e não corresponde aos cinco buffers de 96 bytes nem ao
layout mínimo consumido pelo handler. Ela não é aceita como ABI de `0x2C8`.
Não há asset associado.

## Mapeamento atual

### Source recompilável

O dispatcher reconhece `0x2C8`, mas `TMFieldScene::OnPacketAutoKick` retorna
zero e não executa inspeção do Windows. `STRUCT_AUTOKICK` permanece uma
definição herdada sem uso vivo. Esse sink inerte é deliberadamente mais seguro
que portar uma política remota sem produtor, contrato exato ou testes.

### WYD-Go

Não existe opcode, builder, gate de tamanho, dispatcher ou emissor `0x2C8` no
servidor atual. Segurança e autoridade de gameplay permanecem server-side; o
servidor não delega ao client a inspeção de processos, janelas ou arquivos.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| dispatch `0x2C8` | branch e caller confirmados | branch chama sink inerte | definição herdada divergente | sem produtor | `PARIDADE_NATIVA`, documentar |
| filtros internos | cinco buffers de 96 bytes | não materializados | `4 x 128` incompatível | N/A | não portar ABI presumida |
| inspeção AutoKick | TCP, arquivo e janelas no tick | desativada | código ausente/inativo | segurança server-side | não implementar |
| logout após match | `0x215`, 4 s e saída virtual | fluxo geral de logout existe | N/A | `0x215` já é legado | não criar gatilho remoto |

## Decisões

- Classificar a descoberta como `PARIDADE_NATIVA` em estado `TRACED`.
- Não implementar o consumidor AutoKick nem um emissor `0x2C8`: isso
  introduziria inspeção remota do Windows sem configuração, política,
  necessidade server-side ou testes de integração.
- Manter `OnPacketAutoKick` inerte e não usar `STRUCT_AUTOKICK` como wire/ABI.
- Não promover para `CONTRACT` até existir evidência do construtor/emissor que
  feche tamanho exato, padding e direção completa do protocolo.

## Lacunas

- Localizar o emissor/construtor histórico de `0x2C8` se algum trabalho futuro
  realmente depender de compatibilidade wire com esse opcode.
- O fluxo não requer implementação para o objetivo atual de tornar o
  TMProject funcional; a próxima raiz deve ter impacto observável no client.
- Client real não foi executado, e nenhuma alegação `CLIENT_TESTED` é feita.

## Validação

- Pesquisa: export headless read-only terminou com exit zero, sem
  `SCRIPT ERROR`, e confirmou o hash, callers, callees, duas referências de
  vtable, construtor, destrutores, efeitos e saída por logout.
- Automação: `validate_research.py` e `git diff --check` devem passar após a
  inclusão desta ficha e do índice.
- Client real: não executado; comportamento deliberadamente não portado.
