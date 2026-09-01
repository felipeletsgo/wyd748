---
id: bone-animation-array-probe
title: Consulta de byte do array de animação
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Consulta de byte do array de animação `0x1C1/0x2C2`

## Pergunta

Como o client nativo 7.48 trata a consulta server-to-client `0x1C1`, escolhe
um buffer `.bon`, lê um byte e devolve a resposta client-to-server `0x2C2`?

## Fronteira de evidência

- Native: `WYD.exe` do hash do frontmatter, projeto
  `WYD748Native_20260821.gpr`; dispatcher `FUN_00492E7D`, gate
  `FUN_0055890A`, handler `FUN_004927DD`, loader `FUN_004B00E4` e sender
  `FUN_0055F2DD`.
- Source: `TMFieldScene::OnPacketEvent`,
  `TMFieldScene::OnPacketREQArray`, `MeshManager::InitBoneAnimation` e
  `NewApp::Finalize`.
- Assets ativos: `Mesh/BoneAni4.txt`, `Mesh/ValidIndex.bin` e os buffers
  `<nome>.bon` selecionados pelo catálogo.
- Server: a árvore atual não possui opcode, handler ou emissor para `0x1C1`
  nem consumidor para `0x2C2`.

## Fluxo nativo 7.48

### Entrada observável

O dispatcher de packets `FUN_00492E7D` encaminha `0x1C1` somente depois do
gate central `FUN_0055890A` aceitar exatamente 24 bytes. O packet recebido
contém categoria, offset bruto e um campo de valor que será sobrescrito na
resposta.

### Callers

`FUN_00492E7D` é o caller direto confirmado de `FUN_004927DD` para o opcode
`0x1C1`. O fluxo é uma reação a packet S->C; não há botão, frame tick ou
callback de UI chamando diretamente o handler.

### Função principal

`FUN_004927DD` copia os 24 bytes recebidos. A categoria é usada somente como
índice local: valores fora de `0..99` selecionam a categoria zero, sem alterar
o campo original copiado. O offset também é usado somente como índice local:
valor negativo ou maior/igual ao tamanho bruto do `.bon` selecionado vira
zero. O byte é lido como `char` assinado e promovido com extensão de sinal
para `int32`.

Depois da leitura, o handler troca o opcode para `0x2C2`, substitui o ID pelo
ID do humano local, grava o valor promovido no offset `0x14`, envia os mesmos
24 bytes e retorna `1`.

### Callees

- `FUN_0055F2DD`: envia a resposta de 24 bytes pelo socket ativo.
- `FUN_004B00E4`: durante o bootstrap, materializa o catálogo de animações e
  conserva os bytes brutos dos `.bon` usados pelo handler.

Não há alocação, parser de animação, mutação de cena nem callback assíncrono
dentro de `FUN_004927DD`.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| packet válido | `0x1C1`, 24 bytes, humano e buffer presentes | `FUN_004927DD` | nenhum estado persistente | lê um byte e envia `0x2C2` | retorna `1` |
| categoria inválida | categoria fora de `0..99` | clamp local | usa categoria zero | campos recebidos permanecem no eco | responde normalmente |
| offset inválido | negativo ou `>=` tamanho bruto | clamp local | usa offset zero | campos recebidos permanecem no eco | responde normalmente |
| bootstrap parcial da source | humano presente, buffer ausente/vazio | guard compatível | nenhum estado novo | responde valor zero | evita acesso nulo |
| packet truncado ou excedente | tamanho diferente de 24 | `FUN_0055890A` | handler não executa | nenhum envio `0x2C2` | rejeitado no gate |

### Vtables, vptrs e receptores

O receptor é a `TMFieldScene` ativa alcançada pelo dispatcher de packets. O
humano local fornece apenas o ID da resposta. O catálogo é estático em
`MeshManager::m_BoneAnimationList`; não há chamada virtual, vtable adicional
ou offset de objeto nativo portado para a source.

### Ownership

O `MeshManager` possui os buffers `.bon` alocados no bootstrap. O handler
empresta uma referência somente-leitura ao item selecionado e envia uma cópia
local do packet. O socket copia/consome a mensagem no envio; nenhum ponteiro
para o buffer de animação atravessa o wire.

### Falha parcial e rollback

O handler não muta gameplay, animação ou persistência, portanto não existe
rollback transacional. A source responde zero quando o buffer está ausente ou
vazio, proteção necessária para bootstrap parcial; com assets completos, os
clamps e a leitura são equivalentes ao nativo.

### Cleanup e teardown

O request/response é síncrono e não cria timer, fila, callback ou estado
pendente. A cópia local desaparece no retorno. Os buffers continuam pertencendo
ao `MeshManager` e são liberados por `NewApp::Finalize`.

### Shutdown

Depois que o socket deixa de aceitar mensagens, não há consulta pendente para
cancelar. `NewApp::Finalize` libera `pBone`, `matAnimation` e `matQuaternion`
de todas as cem categorias durante o shutdown global.

### Logout e relogin

O fluxo não armazena resposta na conta, personagem ou cena. Logout elimina a
Field sem deixar consulta pendente. Relogin reutiliza os buffers globais ainda
carregados no mesmo processo; um novo processo os materializa novamente no
bootstrap. Nenhum `Value`, categoria ou offset atravessa sessões.

## Wire, ABI e recursos

Request: servidor para client, opcode `0x1C1`, exatamente 24 bytes. Resposta:
client para servidor, opcode `0x2C2`, exatamente 24 bytes.

| Offset | Tamanho | Campo | Tipo/semântica |
| --- | ---: | --- | --- |
| `0x00` | 12 | `Header` | `MSG_STANDARD`; Type muda de `0x1C1` para `0x2C2` |
| `0x0C` | 4 | `Category` | `int32`; índice efetivo é clampado para `0..99` |
| `0x10` | 4 | `ByteOffset` | `int32`; índice efetivo inválido vira zero |
| `0x14` | 4 | `Value` | `int32`; byte assinado promovido com extensão de sinal |

Na resposta, `Header.ID` recebe o ID do humano local; `Category` e
`ByteOffset` preservam os valores originais recebidos, inclusive quando o
índice efetivo foi clampado. A source fixa tamanho e offsets com
`static_assert` Win32.

## Assets e tamanho bruto

`MeshManager::InitBoneAnimation` lê `Mesh/BoneAni4.txt`, opcionalmente carrega
`Mesh/ValidIndex.bin` e conserva cada `<nome>.bon` integralmente em `pBone`.
O campo herdado `numBone = tamanho / 8` representa pares de IDs e não é um
tamanho byte-exato. Entre os 82 `.bon` ativos, `ed.bon` possui 7.178.971 bytes
(resto 3) e `tn.bon` possui 8.777.247 bytes (resto 7); reconstruir o limite por
`numBone * 8` descartaria bytes válidos. Por isso a adaptação acrescenta
`numBoneBytes` ao final de `stBoneAni`, preservando os offsets herdados e
registrando o tamanho exato entregue pelo loader.

## Mapeamento atual

### Source recompilável

O stub vivo de `TMFieldScene::OnPacketREQArray` agora copia o packet tipado,
aplica os clamps em variáveis locais, lê `std::int8_t`, responde `0x2C2` com o
ID do humano e retorna `1`. O dispatcher usa os opcodes nomeados. Buffer
ausente produz valor zero sem mascarar a transição principal.

### WYD-Go

Não existe emissor `0x1C1` nem consumidor `0x2C2` no servidor atual. Este lote
restaura a capacidade nativa do client sem inventar uso server-side. Qualquer
uso futuro deve nascer como contrato coordenado separado, com propósito,
autorização, limites e testes próprios.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Decisão |
| --- | --- | --- | --- |
| wire `0x1C1/0x2C2` | 24 bytes e offsets confirmados | opcode literal e handler no-op | `PARIDADE_NATIVA` |
| categoria/offset | clamps locais, eco preservado | ausente | `PARIDADE_NATIVA` |
| signedness | `char` assinado para `int32` | ausente | `PARIDADE_NATIVA` |
| limite do `.bon` | tamanho bruto em bytes | apenas `tamanho / 8` | `MODERNIZACAO_COMPATIVEL` interna |
| buffer ausente | bootstrap nativo pressupõe assets | stub não respondia | proteção compatível, valor zero |
| suporte server-side | não inferido pelo handler client | inexistente | fora deste lote |

## Decisões

- Restaurar o request/response como `PARIDADE_NATIVA` em estado `CONTRACT`.
- Conservar o tamanho bruto ao final de `stBoneAni`; não alterar os campos e
  offsets usados pelo renderer herdado.
- Preservar os campos recebidos no eco e aplicar clamps somente aos índices de
  leitura.
- Não criar opcode ou handler no WYD-Go sem uma feature coordenada concreta.

## Lacunas

- O servidor atual não emite a consulta; portanto o fluxo não pode ser
  exercitado end-to-end sem adicionar uma funcionalidade fora deste lote.
- Executar um teste client real apenas quando houver consumidor coordenado e
  seguro para `0x2C2`.
- Até esse cenário, não alegar `CLIENT_TESTED`.

## Validação

- Pesquisa: dispatcher, gate, handler, loader, sender, wire, signedness,
  ownership, teardown, shutdown e relogin foram fechados no hash nativo.
- Build: `Build-Client.ps1` passou em Release/Win32 com toolset v145, zero
  erros e 31 warnings preexistentes; o output foi instalado e conferido em
  `client748/project.exe`, SHA-256
  `DA9F578E6AEF2A6F2ED923E893F412717F7966AC861A21F3A17D939EDF70EE3F`.
- Automação documental: `validate_research.py --repo .` reconheceu esta ficha
  como `CONTRACT` e `git diff --check` passou sem erro de whitespace.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
