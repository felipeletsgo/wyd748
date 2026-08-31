---
id: character-logout-selectchar-relogin
title: Character logout, select-character return, and relogin
subsystem: lifecycle
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# Character logout, select-character return, and relogin

## Pergunta

Como o client nativo 7.48 inicia a saída do mundo pelos controles do
`FieldScene2.bin`, retorna à seleção de personagem sem encerrar a sessão e
entra novamente no mundo?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; raízes focadas de logout
  e relogin inspecionadas diretamente no projeto, sem usar a source moderna
  como prova do wire.
- Assets: `FieldScene2.bin`, apenas para os IDs nativos `633..636`; nenhum
  recurso visual foi criado ou alterado.
- Source atual: `TMFieldScene.cpp` e `ObjectManager.cpp` em
  `client-source/tmproject/Projects/TMProject/`.
- Servidor: `internal/game/character_session.go`, dispatcher e contratos em
  `internal/wire/`.

## Fluxo nativo 7.48

### Entrada observável

- Evento: o menu System da cena Field recebe `633` (selecionar servidor),
  `634` (voltar à seleção de personagem), `635` (sair do jogo) ou `636`
  (cancelar).
- Precondições e estado inicial: personagem ativo em Field e nenhum dos timers
  de saída ainda dentro do bloqueio de seis segundos de `FUN_004662C5`.
- Saída observável: `633..635` enviam a intenção System Quit e armam o timer
  correspondente; `636` apenas fecha o menu. Para `634`, cinco segundos depois
  o client envia Character Logout, recebe a confirmação e muda para a seleção.

### Callers

- `FUN_004662C5` é o handler de controles da Field e consome diretamente os
  IDs `633..636` do recurso nativo.
- `FUN_004776C3` é o consumidor temporal dos três timers armados pelo handler.
- `FUN_0055890A` confirma no gate de framing que a resposta `0x116` possui
  exatamente `0x0C` bytes antes do dispatch para a cena.
- `FUN_00492E7D` é o dispatcher de packets da Field; o branch `0x116` chama
  `FUN_00484C8A`.

### Função principal

`FUN_004662C5` trata os quatro controles sem traduzi-los para IDs de versões
posteriores. Os casos `633`, `634` e `635` montam `MSG_STANDARDPARM` zerado,
preenchem o ID do personagem, opcode `0x3AE` e `Parm = 0`, enviam os 16 bytes e
armam, respectivamente, seleção de servidor, logout de personagem ou saída da
aplicação. O caso `636` somente oculta o painel.

### Callees

- `FUN_004776C3`, após cinco segundos, muda para a cena de seleção de servidor
  no caso `633`, envia `0x215/0x0C` no caso `634` ou posta `WM_CLOSE` no caso
  `635`.
- `FUN_00484C8A` aceita `0x116` somente quando o ID confirma o personagem
  atual; então copia `Score` (`0x1C` bytes) e `Equip` (`0x80` bytes) para o slot
  selecionado e solicita estado `5`.
- Na seleção, `0x213` escolhe o personagem. A confirmação `0x114` reconstrói a
  Field e conclui a transição do estado `5` para `0`.

### Saídas e erros

- Um `0x116` com ID divergente não copia o agregado e não muda o estado.
- O bloqueio inicial de seis segundos impede que outro controle de saída arme
  timers concorrentes enquanto a contagem está ativa.
- No WYD-Go, falha ao persistir conta e charstate não publica `0x116`; o
  personagem permanece no mundo e recebe apenas uma mensagem de erro.
- A volta à seleção preserva a conexão e a autenticação da conta. Fechamento de
  socket, falha de `0x213` e shutdown global continuam nos fluxos próprios.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| controle `633` | Field; timers livres | `FUN_004662C5` | contagem para servidor | envia `0x3AE/0x10`; arma seleção | demais controles bloqueados por 6 s |
| controle `634` | Field; timers livres | `FUN_004662C5` | contagem para logout | envia `0x3AE/0x10`; arma logout | demais controles bloqueados por 6 s |
| timer de `634` | 5 s transcorridos | `FUN_004776C3` | pedido de logout | envia `0x215/0x0C`; zera timer | aguarda confirmação do servidor |
| confirmação `0x116` | ID do personagem | `FUN_00492E7D -> FUN_00484C8A` | estado `5` | copia Score/Equip do personagem atual | ID divergente não altera estado |
| seleção `0x213`, resposta `0x114` | sessão autenticada; slot válido | dispatcher das cenas | estado `0`, Field ativa | materializa personagem no mundo | rejeição fica no fluxo de login |
| controle `635` | Field; timers livres | `FUN_004662C5 -> FUN_004776C3` | fechamento solicitado | envia `0x3AE/0x10`; após 5 s posta `WM_CLOSE` | segue shutdown global |
| controle `636` | menu aberto | `FUN_004662C5` | Field inalterada | fecha menu System | sem packet |

### Vtables, vptrs e receptores

O evento nasce no receptor da `FieldScene`; `FUN_00492E7D` é seu override de
packet. A troca solicitada por `FUN_00484C8A` converge no `ObjectManager`, cuja
vtable efetiva e lifecycle geral estão registrados em `scene-transition.md`.
Este contrato não importa vptr, offset de objeto ou layout de outra versão.

### Ownership

O personagem visível e seus `Score/Equip` pertencem à Field atual. A cópia de
confirmação atualiza o slot correspondente no agregado de seleção pertencente
ao `ObjectManager`. No servidor, o `Player` perde somente o estado efêmero do
personagem; `Session` e `Account` permanecem autenticados para o próximo
`0x213`.

### Falha parcial

O servidor persiste conta e charstate antes de remover o personagem do mundo e
antes de publicar `0x116`. Se a persistência falha, não confirma uma transição
que apagaria o runtime local. Depois do commit, a remoção limpa índices,
visibilidade, trade, party, summons e estado específico do personagem.

### Cleanup e teardown

O logout explícito remove o personagem do mundo e notifica observers, mas não
destrói a sessão TCP nem a conta autenticada. A cena anterior segue o mecanismo
de marca, coleta e deleting destructor documentado na ficha geral de cenas.

### Shutdown

O controle `635` chega ao `WM_CLOSE` após a mesma contagem de cinco segundos.
A desmontagem posterior pertence ao contrato
`application-close-global-shutdown.md`; esta ficha não duplica essa etapa.

### Logout e relogin

O ciclo coberto é completo para a transição personagem em mundo -> seleção de
personagem -> personagem em mundo, na mesma sessão autenticada. Troca de conta,
reconexão TCP e migração entre servidores não fazem parte deste contrato.

## Wire, ABI e recursos

- System Quit: client -> servidor, opcode `0x3AE`, `MSG_STANDARDPARM` de 16
  bytes, ID do personagem e `Parm = 0`.
- Character Logout: client -> servidor, opcode `0x215`, `MSG_STANDARD` de 12
  bytes e ID do personagem.
- CNF Character Logout: servidor -> client, opcode `0x116`, 12 bytes e ID do
  personagem.
- Character Login: client -> servidor, opcode `0x213`; a confirmação `0x114`
  volta à Field. Seus demais campos permanecem regidos pelas fichas de login.
- Controles do `FieldScene2.bin`: `633`, `634`, `635`, `636`. IDs modernos e
  seus grids não integram o recurso 7.48.

## Mapeamento atual

### Source recompilável

- `TMFieldScene::OnControlEvent` preserva os IDs nativos, arma os timers,
  envia `0x3AE` e fecha corretamente o menu sem alcançar handlers modernos.
- `TMFieldScene::TimeDelay` já implementava os três consumidores de cinco
  segundos e o envio posterior de `0x215`.
- `TMFieldScene::OnPacketCNFCharacterLogout` já validava o ID, copiava
  `Score/Equip` e solicitava `TM_SELECTCHAR_STATE`.
- O dispatcher já roteava `0x116`, e a seleção/`0x114` já retornava à Field.

### WYD-Go

`World.onCharacterLogout` exige sessão em mundo e packet de 12 bytes, persiste
estado autoritativo, remove o runtime do personagem e só então envia `0x116`.
Os testes cobrem framing, dispatch, limpeza de referências e recusa da
confirmação quando a persistência atômica falha.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject posterior | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| IDs do menu | `633..636` | recurso 7.48 usa esses IDs | usa IDs e grids diferentes | N/A | preservar dispatch 7.48 |
| Intenção inicial | `0x3AE/0x10`, ID e `Parm=0` | faltava nos IDs nativos | caminho moderno era incompatível | tolera intenção sem confiar nela | portar paridade nativa |
| Logout após timer | `0x215/0x0C` após 5 s | já implementado | apenas pista semântica | handler autoritativo existente | manter |
| Confirmação | `0x116/0x0C`, ID guard, estado 5 | já implementada | não decide ABI | só após persistência | manter |
| Relogin | `0x213 -> 0x114`, estado 5 -> 0 | já implementado | não decide wire | valida e recria personagem | manter |

## Decisões

- Classificar a correção como `PARIDADE_NATIVA`: somente o dispatch dos
  controles `633..635` estava ausente.
- Manter `TimeDelay`, confirmação, seleção e servidor atuais porque já
  preservam o contrato 7.48 e têm estrutura superior sem alterar a fronteira.
- Não reutilizar o handler moderno `65881`: ele limpa grids ausentes no
  `FieldScene2.bin` e pode desreferenciar controles nulos.
- Manter esta ficha em `CONTRACT`; implementação e build não equivalem a
  `CLIENT_TESTED`.

## Lacunas

- Executar no `client748/project.exe` os quatro controles, a volta à seleção e
  o relogin do mesmo e de outro slot.
- Validar em runtime falha de save, rejeição de login e queda de conexão entre
  `0x215` e `0x116`.
- Troca de conta e reconexão TCP permanecem em fichas separadas. A convergência
  terminal está fechada em `application-close-global-shutdown.md`.

## Validação

- Pesquisa: funções `FUN_004662C5`, `FUN_004776C3`, `FUN_0055890A`,
  `FUN_00492E7D` e `FUN_00484C8A` fechadas para esta transição no projeto
  Ghidra do hash registrado.
- Automação: `validate_research.py` aprovado com `CONTRACT=3` e `LOCATED=3`;
  `triage_catalog.py` confirmou 4.146 funções, 40 `STATICALLY_EVIDENCED`, 23
  `LOCATED` e 4.083 `UNMAPPED`.
- Servidor: `go test ./internal/game ./internal/wire` aprovado.
- Client source: `Build-Client.ps1` aprovado com 31 warnings preexistentes e
  zero erros; candidato instalado em `client748/project.exe`, SHA-256
  `746A2913FA62DD56892319BD136CD15810A4B74092AD75F354E60C6B3FFC5BBD`.
- Higiene: `git diff --check` aprovado; somente avisos informativos de LF/CRLF
  do Git no Windows.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
