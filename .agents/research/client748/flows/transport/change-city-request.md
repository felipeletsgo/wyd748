---
id: change-city-request
title: Pedido de troca de cidade 0x291
subsystem: transport-session
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-06
---

# Pedido de troca de cidade 0x291

## Pergunta

Qual frame o client 7.48 envia quando entra ou troca de cidade, e quais
condições locais permitem que a informação de vila seja enviada ao servidor?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; export focado
  `exports/change-city-request-flow.tsv`, SHA-256
  `6C48347D35B3F33AF678BD9525090BF99029525FA8FFADB68EA52F4A740D5B18`.
- Assets: `NÃO APLICÁVEL`; o frame contém somente header e um DWORD.
- Source atual: `TMFieldScene.cpp`, `Basedef.h` e
  `internal/wire/ChangeCityPacket.h`.
- Servidor: `internal/wire/opcodes.go`, `internal/game/security.go`,
  `internal/game/handlers.go`, `world.go` e testes de movimento.
- TMProject posterior/guias: `NÃO APLICÁVEL` para o ABI; W2PP, Secrets e
  Micronics foram excluídos da decisão.

## Fluxo nativo 7.48

### Entrada observável

O tick de `FUN_004776C3` calcula a vila a partir da posição do humano. Há dois
emissores no mesmo fluxo: o primeiro reage à entrada/alteração do estado da
cidade e o segundo acompanha a alteração de vila no caminho de campo. Cada ramo
zera um buffer de 16 bytes, grava o ID do personagem no header, escreve
`Type=0x291`, coloca o valor calculado em `+12` e só envia quando a vila é menor
que quatro.

### Callers

O export focado confirma `FUN_0047E49D -> FUN_004776C3` no tick de Field e o
ponteiro de função `0x005A42B4` para a rotina. Os dois pontos de construção
observáveis estão em `0x00478B2B` e `0x00478F7A`.

### Callees

- `FUN_00552F42` calcula a vila a partir das coordenadas convertidas.
- `FUN_0055F2DD` recebe os 16 bytes e aplica o throttle/enquadramento do envio;
  os dois branches chamam-no em `0x00478B8A` e `0x00478FC7`.
- `FUN_0055890A` contém o caso `0x291` e rejeita qualquer tamanho diferente de
  `0x10` (`0055890a_FUN_0055890a.c`, linha 177 do export estudado).

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Pré-condição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| entrada/alteração de cidade | humano ativo, vila calculada `<4` | `FUN_004776C3 -> FUN_0055F2DD` | sessão recebe a intenção de vila | atualiza o ponto local de HomeTown depois do envio | vila `>=4` não envia |
| mudança de vila no campo | `m_nVillage`/marcador nativo mudou | mesmo tick e frame | marcador local acompanha a vila | HomeTown é atualizado nos bytes da seleção | valor fora do intervalo não envia |
| servidor recebe | sessão autenticada e frame 16B | `World.onChangeCity` | posição autoritativa é registrada | log e persistência seguem o modelo atual | sessão/player ausente ou frame curto é ignorado |
| desconexão | transporte falha | teardown geral | cena/sessão encerrada | cleanup do socket | o packet não retém estado próprio |
| logout/relogin | nova cena e novo personagem | lifecycle geral | marcador é reconstruído | nenhum request antigo é repetido | sem replay automático |

O buffer nativo vive na pilha e é emprestado ao transporte. O client atual
mantém `m_nVillage` e grava `HomeTownX/Y` somente no mesmo caminho que já existia;
a extração não altera timers, música, cena ou ordem de efeitos. O servidor
continua autoritativo: o DWORD recebido não restaura score nem substitui a
posição publicada pelo movimento.

## Wire, ABI e recursos

Direção C->S, opcode `0x291`, exatamente 16 bytes:

| Offset | Campo | Tipo | Semântica |
| ---: | --- | --- | --- |
| `+0` | `Header` | `MSG_STANDARD` | envelope WYD 7.48 |
| `+12` | `Village` | `int32` little-endian | índice de vila calculado pelo client |

O nativo grava o ID do personagem em `Header.ID`, preserva os demais bytes
zerados e não possui resposta S->C específica. A source e o servidor usam o
mesmo tamanho; `security.go` ainda aceita o valor `4` para compatibilidade,
enquanto os emissores nativos só enviam valores `<4`. Não há asset ou recurso.

## Mapeamento atual

Os dois emissores de `TMFieldScene.cpp` agora usam `MSG_ChangeCity` e
`MSG_ChangeCity_Opcode`; a fachada `Basedef.h` apenas reexporta o header próprio.
O WYD-Go já declara `OpChangeCity`, exige 16 bytes no gate e chama
`onChangeCity`, que registra a posição autoritativa corrente sem aceitar frame
curto ou player inexistente. Nenhuma rota foi adicionada ao dispatcher recebido
do client porque o contrato é somente C->S.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| wire | `0x291/16`, `Village@12` | antes usava `MSG_STANDARDPARM` e literal | gate/handler equivalentes | extrair tipo dedicado |
| emissores | dois branches em `FUN_004776C3` | dois caminhos preservados | um handler autoritativo | manter |
| estado | HomeTown atualizado depois do envio | comportamento preservado | posição do servidor continua autoridade | manter |
| resposta | nenhuma específica | nenhuma | nenhuma | manter |

## Decisões

- Classificar como `PARIDADE_NATIVA/CONTRACT`.
- Criar `ChangeCityPacket.h` com opcode, estrutura de 16 bytes e asserts de
  tamanho/offset.
- Substituir somente o tipo/literal dos dois emissores, sem modificar a
  condição `<4`, timers, música, posição ou persistência local.
- Não adicionar rota ao `ReceivedPacketDispatch`, pois `0x291` é C->S.

## Lacunas

- Entrada real em cidade, troca de vila, disconnect, troca de cena e relogin
  ainda não foram executados no `client748/project.exe`; a ficha não é
  `CLIENT_TESTED`.

## Validação

- Pesquisa: Ghidra read-only usou o hash nativo esperado; export focado registra
  os dois writes de opcode, os dois envios de 16 bytes, caller/callee e o gate.
- Automação: fixture C++ verifica opcode, ID, tamanho, offset e bytes zerados;
  `go test -count=1 ./...`, XML/caminhos, build Debug/Release e
  `git diff --check` serão registrados no handoff após a edição.
- Client real: ainda não executado; não usar `CLIENT_TESTED`.
