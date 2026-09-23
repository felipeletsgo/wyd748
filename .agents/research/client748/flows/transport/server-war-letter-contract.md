---
id: server-war-letter-contract
title: Cartas de guerra entre canais 0xED7 e 0xED8
subsystem: transport-ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-21
---

# Cartas de guerra entre canais 0xED7 e 0xED8

## Pergunta

Qual fluxo o client nativo 7.48 executa ao usar os itens 4030 e 4031, e qual
payload envia somente depois que o jogador confirma o canal alvo?

## Fronteira de evidência

- Executável/hash: `references/client748/WYD.exe`, SHA-256 do frontmatter.
- Binário nativo: disassembly read-only das regiões `0x0042097D`,
  `0x0044C947`, `0x0044C9D6`, `0x0046A019` e `0x0046A077`.
- Assets: `tmproject/client748/UI/strdef.bin`, entradas 374 e 375; controles
  nativos 627 (edit), 630 (texto), 625 (painel), 626 (OK) e 628 (cancelar).
- Source atual: `SGrid.cpp`, `TMFieldScene.cpp`, `Basedef.h` e
  `internal/wire/ServerWarLetterContract.h`.
- Servidor: `wydgo748/internal/game`, configuração de `ChannelID` e modelo de
  `World`; não existe handler de `0xED7/0xED8` nem coordenador entre Worlds.
- TMProject 7.69: usado apenas para localizar nomes; não fundamenta o contrato.

## Fluxo nativo 7.48

### Entrada observável

Usar a carta 4030 abre a pergunta localizada 374 para declarar guerra a outro
canal. Usar 4031 abre a pergunta 375 para negar/recusar a guerra. Nenhum pacote
é enviado no clique inicial; o canal digitado só cruza a rede após OK/Enter.

### Callers

O roteamento de item em `0x0042097D` compara os índices 4030 e 4031. O primeiro
chama `FUN_0044C947`; o segundo chama `FUN_0044C9D6`; ambos retornam sem passar
pelo emissor genérico de uso de item.

### Função principal

`FUN_0044C947` seleciona o modo 9 e prepara o modal compartilhado com a string
374. `FUN_0044C9D6` seleciona o modo 10 e usa a string 375. Os dois resolvem o
texto 630 e o edit 627, focam o campo e tornam o painel visível.

Na confirmação, os ramos em `0x0046A019` e `0x0046A077` montam um
`MSG_STANDARDPARM` de 16 bytes: modo 9 usa `0xED7`, modo 10 usa `0xED8`, e o
canal convertido ocupa o DWORD em `+12`.

### Callees

- O modal compartilhado converte o texto decimal e despacha o caso 9 ou 10.
- O transporte recebe o buffer temporário de 16 bytes após a confirmação.
- Cancelar apenas oculta o modal; não existe envio com canal zero no clique.

### Saídas e erros

Campo vazio ou conversão inválida não deve inventar um canal. Cancelar não
emite pacote. A aceitação autoritativa, consumo do item e estado da guerra são
responsabilidades do servidor e não estão implementados pelo peer atual.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| usar 4030 | FieldScene e item válidos | `0x0042097D -> FUN_0044C947` | modal no modo 9 | texto 374 e foco no edit | sem packet |
| usar 4031 | FieldScene e item válidos | `0x0042097D -> FUN_0044C9D6` | modal no modo 10 | texto 375 e foco no edit | sem packet |
| confirmar modo 9 | canal digitado | ramo `0x0046A019` | modal fecha | C->S `0xED7/16` | canal em `+12` |
| confirmar modo 10 | canal digitado | ramo `0x0046A077` | modal fecha | C->S `0xED8/16` | canal em `+12` |
| cancelar | modal visível | controle 628/Esc | modal oculto | nenhum | sem packet |

### Vtables, vptrs e receptores

O fluxo usa o `SControlContainer` e os tipos concretos de texto/edit já
existentes. Não altera vtable ou slot virtual.

### Ownership

Os controles pertencem à FieldScene. O packet vive na pilha e é emprestado ao
transporte durante a chamada de envio.

### Falha parcial

Se painel, texto ou edit não forem encontrados, a source recompilável retorna
sem abrir o modal. Isso evita dereferência nula sem criar fallback de wire.

### Cleanup e teardown

O modal compartilhado é ocultado pelo caminho existente; não cria alocação,
timer ou estado persistente próprio.

### Shutdown

Encerrar a cena descarta foco e texto junto com os demais controles. Não há
buffer de packet retido.

### Logout e relogin

Uma nova FieldScene reconstrói o painel e começa sem prompt visível. O canal
digitado não atravessa logout ou relogin.

## Wire, ABI e recursos

Os dois opcodes usam o mesmo layout C->S:

| Offset | Campo | Tipo | Semântica |
| ---: | --- | --- | --- |
| `+0` | `Header` | `MSG_STANDARD` | size 16, opcode e ID local |
| `+12` | `Parm` | `int32` | canal alvo informado pelo jogador |

Recursos nativos relevantes:

- item 4030: carta de declaração, prompt 374, modo 9, opcode `0xED7`;
- item 4031: carta de recusa, prompt 375, modo 10, opcode `0xED8`;
- painel 625, OK 626, edit 627, cancel 628 e texto 630.

## Mapeamento atual

### Source recompilável

Antes da correção, o clique de 4030 montava imediatamente `0xED7` com
`Parm=0`, enquanto 4031 caía no uso genérico; os métodos do modal procuravam
somente IDs modernos inexistentes em `FieldScene2.bin`. Agora ambos os itens
abrem o prompt compatível, OK/Enter e cancel são traduzidos para os handlers
compartilhados, e os opcodes nomeados só são enviados após a confirmação.

`ServerWarLetterContract.h` fixa itens, modos, opcodes, tamanho e offset.
`Basedef.h` comprova que `MSG_STANDARDPARM` conserva os 16 bytes e `Parm/+12`.
O modal agora rejeita zero e valores acima de `INT32_MAX` antes do cast para
`Parm`; assim uma entrada decimal longa não vira outro canal por truncamento.
Essa guarda é `MODERNIZACAO_COMPATIVEL` do emissor e não implica aceitação no
servidor. A faixa de canais existentes continua decisão autoritativa dele.

### WYD-Go

O servidor possui `ChannelID`, mas cada `World` representa uma instância
isolada. Não há handler de `0xED7/0xED8`, barramento entre canais, ownership de
Noatun compartilhado ou lifecycle semanal de guerra. A implementação integrada
fica bloqueada até existir coordenação autoritativa entre instâncias; aceitar o
packet e consumir a carta sem esse estado seria um falso suporte.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| clique em 4030 | abre modo 9 | enviava `ED7/Parm=0` | sem handler | restaurar prompt |
| clique em 4031 | abre modo 10 | uso genérico | sem handler | restaurar prompt |
| controles compatíveis | 627/630 e painel compartilhado | buscava IDs modernos | N/A | resolver IDs 7.48 |
| confirmação | `ED7/ED8`, 16 bytes, canal `+12` | branches já existiam | rejeita desconhecido | preservar client; não fingir suporte |
| autoridade | servidor decide/coordena | client só envia canal | arquitetura monocanal por World | coordenador é pré-requisito |

## Decisões

- Classificar o fluxo do client como `PARIDADE_NATIVA`.
- Não enviar `0xED7` ou `0xED8` no clique inicial.
- Compartilhar o modal existente sem impor limite textual não observado no
  binário nativo.
- Não adicionar handler permissivo no WYD-Go até existir contrato de
  coordenação entre canais, validação do líder/dono, consumo atômico e rollback.

## Lacunas

- O fluxo ainda não foi exercitado no `project.exe`; portanto não é
  `CLIENT_TESTED`.
- Faltam evidência e arquitetura para o lifecycle autoritativo entre canais no
  servidor. Essa lacuna não invalida o contrato C->S do client.

## Validação

- Pesquisa: disassembly read-only do binário canônico confirmou item, modal,
  modos, opcodes, tamanho e offset; `strdef.bin` confirmou os prompts.
- Automação: `ArchitectureTests` verifica opcodes, ABI, itens e modos; o build
  Release/Win32 passou 51.699 checks, incluindo limites zero, `INT32_MAX` e
  entradas longas que truncariam o DWORD. O candidato instalado tem SHA-256
  `5B908910F0388A71834B620BF65D52C7B90DE93EA7C5809BAAD5FFCE718E041A`.
- Client real: pendente; testar as duas cartas, OK/Enter, cancel/Esc, canal
  vazio/inválido, troca de cena e relogin sem alegar aceitação server-side.
