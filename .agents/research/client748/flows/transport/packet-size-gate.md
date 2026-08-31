---
id: packet-size-gate
title: Validar tamanho fixo pelo opcode do packet
subsystem: transport
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# Validar tamanho fixo pelo opcode do packet

## Pergunta

Onde o client nativo 7.48 compara o tamanho declarado no header com o tamanho
fixo esperado para cada opcode, e em que ponto do transporte essa rejeição é
aplicada?

## Fronteira de evidência

- Executável: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 acima.
- Projeto: `WYD748Native_20260821`; corpus `GhidraAnalysis/20260821/decompiled`.
- Nativo localizado: `FUN_0055890A @ 0x0055890A`.
- Exports focados:
  `.agents/research/client748/exports/packet-size-gate-pointer-search.tsv` e
  `.agents/research/client748/exports/packet-size-gate-flow.tsv` e
  `.agents/research/client748/exports/packet-size-gate-bodyrefs.tsv`.
- Source: `CPSock::ReadMessage`, `NewApp` message loop e
  `WYD748Compat.cpp`.
- Servidor: `internal/game/security.go` e contratos de `internal/wire`.

## Fluxo nativo 7.48

### Entrada

`CONFIRMED`: a função recebe um ponteiro cujo `uint16` no offset `0` é o
tamanho e cujo `uint16` no offset `4` é o opcode do header legado.

### Callers

`UNRESOLVED`: o export textual e os xrefs tipados não contêm uma chamada
direta. Uma busca headless exata pelo VA bruto `0x0055890A` em toda a memória
carregada encontrou zero hits. Uma segunda busca percorreu 441.614 instruções
decodificadas e resolveu 24.880 candidatos `CALL/JMP rel32`, também sem branch
direto para a função. O scanner encontrou os callsites conhecidos
`0x005599EF -> FUN_0058F078` e `0x00559A05 -> FUN_00423B20` quando executado
contra esses callees.

`CONFIRMED`: a busca por xrefs externos destinados a qualquer instrução do
corpo encontrou `external_flow_hits=0`, `external_other_hits=0` e
`thunk_hits=0`. Os 117 xrefs `DATA` são seis jump tables internas contíguas:
`0x00559A14-0x00559A54` (17 entradas), `0x00559ABD-0x00559AFD` (17),
`0x00559BA0-0x00559BE0` (17), `0x00559C8E-0x00559CCA` (16),
`0x00559D7B-0x00559DFF` (34) e `0x00559EB5-0x00559EF1` (16). Elas descrevem o
CFG do switch e não são callers. A função contém 1.094 instruções no corpo
`0x0055890A-0x00559A13`; a instrução anterior em `0x00558909` é `RET`, sem
fallthrough para a entrada.

`CONFIRMED`: o scan bruto de 1.859.564 bytes nos cinco blocos executáveis
inicializados resolveu 34.511 candidatos `E8/E9 rel32`, com `errors=0` e zero
destinos para `FUN_0055890A`. No mesmo export, os controles positivos
`FUN_00423B20` e `FUN_0058F078` recuperaram respectivamente 16 e 757 xrefs
`FLOW` no corpo e os mesmos 16 e 757 hits brutos, todos classificados como
`instruction`.

Os resultados eliminam ponteiros literais de 32 bits, branches `E8/E9`
decodificados, branches `E8/E9` brutos no escopo varrido, xrefs `FLOW` externos
para o corpo e thunks reconhecidos. Chamada ou branch indireto, destino derivado,
tabela construída em runtime ou mecanismo não materializado pelo Ghidra
continuam possíveis.

`CONFIRMED`: a imagem PE não possui Export Directory (`RVA=0`, `size=0`) e
`dumpbin /exports` não lista símbolos. Portanto, `FUN_0055890A` também não é
alcançada como exportação do executável. Essa ausência não decide entradas
internas calculadas em runtime.

### Função principal

`CONFIRMED`: `FUN_0055890A` compara dezenas de pares opcode/tamanho. Exemplos:
`0x182 -> 0x18`, `0x337 -> 0x24`, `0x373 -> 0x24`. Retorna zero quando o par é
aceito e valor não zero quando diverge.

`CONFIRMED`: as seis jump tables registradas acima pertencem à própria função e
implementam os branches internos da tabela de opcodes; elas não evidenciam uma
entrada externa para o gate.

`CONFIRMED`: em divergência, formata a mensagem
`sys : Type check missing type : %d size : %d` por `FUN_0058F078` e então chama
`FUN_00423B20`.

### Callees

- `FUN_0058F078`: `CONFIRMED` formatter; escreve no buffer fornecido e aplica
  terminação nula por meio de `FUN_00594029`.
- `FUN_00423B20`: `CONFIRMED` stub que retorna imediatamente; no binário
  analisado, não há side effect próprio observável depois do diagnóstico.

### Saídas e erros

`CONFIRMED`: `nullptr` retorna zero. Par conhecido com tamanho divergente retorna
não zero e registra diagnóstico. O efeito exercido pelo caller sobre o packet
permanece `UNRESOLVED`.

## Estado e lifecycle

Não há mutação visível no packet. O lifecycle de recepção/rejeição ainda depende
do caller não resolvido; portanto a ficha permanece `LOCATED`.

`PROBABLE`: a rotina está inalcançável nesta build pelos mecanismos estáticos
inspecionados: exportação PE, xref `FLOW`, thunk, ponteiro VA literal e
`CALL/JMP rel32` decodificado ou bruto. Isso não autoriza classificá-la como
código morto, pois um destino ainda pode ser derivado ou construído em runtime.

`UNRESOLVED`: alcançabilidade em runtime por chamada/branch indireto, destino
derivado, tabela construída em runtime ou outro mecanismo não materializado
pelo projeto Ghidra atual.

## Wire, ABI e recursos

- Header observado: tamanho `uint16` em `+0`, opcode `uint16` em `+4`.
- Direção e ponto exato de aplicação ainda `UNRESOLVED`.
- A função é evidência direta de tamanhos fixos, mas não prova offsets internos
  além do header nem substitui o consumidor específico do packet.

## Mapeamento atual

### Source recompilável

`CPSock::ReadMessage` valida framing mínimo/máximo, disponibilidade dos bytes,
keyword e checksum, mas a investigação atual não localizou chamada equivalente
ao gate opcode/tamanho. `WYD748Compat.cpp` protege diversos layouts com
`static_assert`; isso é proteção de build, não validação runtime de entrada.

`REJECTED` como correspondência semântica: a correlação diferencial contra o
`project.exe` de SHA-256
`F8251714775601720307940598522E6D2924E5C61DAB300728F949FE0C8A380B` retornou
somente candidatos fracos e não recíprocos: `D3DX::jpeg_idct_float` em
`0x0043C550` (`NO_MATCH`, score `10.51`), `TMHouse::Render @ 0x004E5110`
(`CANDIDATE`, `10.48`) e `TMHuman::CheckWeapon @ 0x00507D40` (`CANDIDATE`,
`10.36`), todos sem suporte de vizinhos. Nenhum deles implementa um gate de
packet; o cache apenas descarta uma correspondência estrutural confiável nessa
source antiga e não prova ausência na source atual.

### WYD-Go

`internal/game/security.go` contém o gate server-side de tamanho permitido para
packets recebidos do client. Os construtores de `internal/wire` definem packets
enviados ao client. A equivalência completa por opcode ainda não foi comparada.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| Gate fixo por opcode | tabela runtime em `FUN_0055890A` | não localizado no receive atual | herança ainda não auditada | gate C->S existe | não implementar até resolver caller/direção |
| `0x373` tem 36 bytes | comparação direta | `static_assert(sizeof(MSG_UseItem)==36)` | layout homônimo não decide | handler espera contrato 7.48 | manter como claim confirmada, offsets internos dependem do consumidor |
| `0x337` tem 36 bytes | comparação direta | `static_assert(sizeof(MSG_UpdateEtc)==36)` | valores modernos são risco | construtor S->C existente | auditar consumidor antes de promover a contrato completo |

## Decisões

- Não adicionar um novo gate ao socket apenas por semelhança; primeiro resolver
  caller, direção e comportamento de rejeição no nativo.
- Usar a tabela nativa como índice para auditar opcodes, nunca como prova isolada
  de todos os campos de cada packet.

## Lacunas

- xrefs/caller real de `FUN_0055890A`;
- direção e fase exatas (antes/depois de decrypt/dispatch);
- comportamento do caller quando a função retorna não zero;
- comparação completa da tabela nativa com client-source e `internal/wire`;
- origem de chamadas indiretas, thunks ou regiões ainda não analisadas.
- possível alcançabilidade por destino derivado ou mecanismo construído em
  runtime que os scans estáticos atuais não materializam.

## Validação

- Pesquisa: pseudocódigo e hash conferidos em 2026-08-27.
- Automação: `validate_research.py --repo .` aprovou a estrutura desta ficha em
  2026-08-28. `ExportWydFlow.java` compilou e executou em modo headless/read-only
  no Ghidra 12.1.3. `pointers:0055890a` registrou `hits=0`; o scan de 441.614
  instruções registrou `hits=0` entre 24.880 `CALL/JMP rel32`, com `errors=0`.
  `bodyrefs:0055890a` registrou zero `FLOW`/thunk e classificou 117 xrefs `DATA`
  nas seis jump tables; `rawrelatives:0055890a` registrou zero hits entre
  34.511 candidatos em 1.859.564 bytes de cinco blocos executáveis. Controles
  positivos recuperaram 16 hits para `FUN_00423B20` e 757 para `FUN_0058F078`
  nos dois modos. O export versionado tem SHA-256
  `E51351C895E0F9439AF37E97DDE30F12F98A3822E728EA691115B10E9542EAF6`.
  Depois de tornar a expansão das jump tables segura contra wrap no limite do
  address space, o script recompilou e reproduziu o arquivo byte a byte. O
  `dumpbin /headers /exports` confirmou `RVA=0` e `size=0` para a Export
  Directory. A correlação diferencial cacheada do candidato
  `F8251714...A380B` retornou um `NO_MATCH` e dois `CANDIDATE` sem reciprocidade
  ou suporte de callgraph; nenhum corresponde semanticamente ao gate. O caller
  permanece pendente e impede promoção além de `LOCATED`.
- Client real: não executado; nenhuma mudança comportamental foi feita.
