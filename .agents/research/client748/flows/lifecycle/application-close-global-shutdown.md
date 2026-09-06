---
id: application-close-global-shutdown
title: Application close and global shutdown
subsystem: lifecycle
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# Application close and global shutdown

## Pergunta

Como o client nativo WYD 7.48 transforma `WM_CLOSE` em fechamento imediato ou
adiado, notifica o servidor quando existe uma Field ativa e desmonta os owners
globais antes de destruir a janela e encerrar o message loop?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto Ghidra: `WYD748Native_20260821.gpr`; fluxo focado em
  `FUN_0055B18F`, `FUN_0055B26F`, `FUN_0055BC0A`, `FUN_0055D066`,
  `FUN_0055DAB8`, `FUN_0055FA89`, `FUN_00423C1F` e `FUN_00423C61`.
- Exports regeneráveis: `application-shutdown-focused.tsv`,
  `wm-close-dispatch-instructions.tsv`, `fun-00423c61-shutdown-focused.tsv` e
  `windows-hook-lifecycle-focused.tsv` em
  `%TEMP%\codex-wyd748-lifecycle-149205b7`; todos registram o hash nativo e
  nenhum contém `SCRIPT ERROR`.
- Source recompilável: `NewApp.cpp` e contratos de packet em
  `client-source/tmproject/Projects/TMProject/`.
- Servidor: `internal/game/handlers.go`, `internal/wire/codec.go`,
  `internal/wire/opcodes.go` e o gate de tamanho em `internal/game/security.go`.

Esta ficha cobre o fechamento global da aplicação. As fontes específicas que
postam `WM_CLOSE`, logout para seleção, troca de conta e reconexão TCP pertencem
a fluxos próprios. O estado `CONTRACT` decorre do wire `0x3AE/0x10`; não implica
teste real do executável recompilado.

## Fluxo nativo 7.48

### Entrada observável

`FUN_0055BC0A` registra `FUN_0055FA89` em `WNDCLASSA.lpfnWndProc`.
`FUN_0055FA89` copia os quatro argumentos do callback, carrega
`ECX = DAT_013B71E0`, chama `FUN_0055DAB8` e retorna com `RET 0x10`.
`FUN_0055B18F` constrói e publica exatamente esse singleton da aplicação em
`DAT_013B71E0`, instalando o vptr `0x005A6104`.

A entrada observável deste contrato é, portanto:

```text
message loop -> DispatchMessageA -> FUN_0055FA89
-> DAT_013B71E0/FUN_0055DAB8 -> case WM_CLOSE (0x10)
```

### Callers

A vtable efetiva da aplicação começa em `0x005A6104`:

| Slot | Função nativa | Papel fechado nesta ficha |
| --- | --- | --- |
| `+0x00` | `FUN_0055F3E0` | deleting destructor; fora do teardown interno |
| `+0x04` | `FUN_0055BC0A` | bootstrap e registro do WndProc |
| `+0x08` | `FUN_0055D066` | `Finalize` global |
| `+0x0C` | `FUN_0055D345` | run/message loop |
| `+0x10` | `FUN_0055EDF7` | não necessário a esta transição |
| `+0x14` | `FUN_0055D6E6` | estágio de frame; não necessário a esta transição |
| `+0x18` | `FUN_0055EE1E` | setter do ObjectManager/global |
| `+0x1C` | `FUN_0055EE45` | não necessário a esta transição |

No ramo `WM_CLOSE`, `FUN_0055DAB8:0x0055EB9C` carrega o vptr diretamente do
receptor recebido em `param_1` e chama o slot `+0x08`. Como o thunk fornece o
singleton com vtable `0x005A6104`, o caller efetivo fica resolvido como:

```text
WM_CLOSE -> FUN_0055DAB8:0x0055EB9C
-> app vslot +0x08 -> FUN_0055D066
```

`FUN_0055F7F9` não repete esse `Finalize` depois que o message loop retorna; ele
prossegue para o deleting destructor. `FUN_0055B26F` reinstala a mesma vtable no
caminho de destruição e fecha o ownership final do objeto da aplicação.

### Função principal: branch WM_CLOSE

O branch distingue Field ativa de todas as demais cenas. A condição nativa é
`DAT_0067CF38 != 0 && DAT_0067CF38[9] == 30000`.

No primeiro `WM_CLOSE` em Field:

1. lê o tempo pelo objeto em `app+0xF4`, virtual `+0x08`;
2. grava o valor em `DAT_013B7220`;
3. zera e monta um packet de 16 bytes;
4. envia o packet por `FUN_0042550E`;
5. retorna sem destruir a aplicação.

Nos fechamentos seguintes em Field, o mesmo provedor de tempo é comparado com
`DAT_013B7220 + 3000`. Enquanto o limite não é atingido, o branch retorna sem
efeitos adicionais. A source identifica esse owner como `TimerManager` e trata
o limite como três segundos; estaticamente, o nativo comprova 3.000 unidades do
mesmo relógio, mas esta ficha não alega medição runtime independente.

Quando o limite é atingido, ou imediatamente quando a cena atual não é Field,
o branch:

1. chama `FUN_0055D066` pelo slot `+0x08` da aplicação;
2. fecha o packet dump quando aberto;
3. marca o término global;
4. finaliza o hook de teclado por `FUN_00423C61`;
5. chama `DestroyWindow`;
6. chama `PostQuitMessage(0)`;
7. zera `DAT_013B7220`.

O ack do servidor não libera esse gate: o nativo aguarda o relógio e fecha após
o limite. A queda real da sessão ocorre quando o socket é desmontado/fechado.

### Callees

`FUN_0055D066` primeiro libera três buffers de animação em cada um dos 100
registros globais. Depois destrói o ObjectManager em `DAT_013B71E8` e desmonta
os owners da aplicação nesta ordem observada:

| Ordem | Campo/global | Correlação atual |
| --- | --- | --- |
| 1 | `DAT_013B71E8` / `app+0xF8` | ObjectManager |
| 2 | `app+0xF4` | TimerManager |
| 3 | `app+0xE0` | RenderDevice |
| 4 | `app+0xE4` | SoundManager |
| 5 | `app+0xFC` | SocketManager |
| 6 | `app+0xEC` | BGMManager |
| 7 | `app+0xE8` | AviPlayer |
| 8 | `app+0xF0` | EventTranslator |

Os nomes são correlacionados pelo uso no WndProc, setters, ordem de
inicialização e ordem equivalente da source; os offsets nativos não são
importados como ABI da source. Cada owner destruído é zerado. Em seguida,
`DAT_013B71E4` é zerado, o bitmap em `app+0x110` é destruído e uma janela
auxiliar é finalizada quando `DAT_013B736C == 1`.

A destruição do ObjectManager converge no cleanup da árvore e das cenas já
documentado em `scene-transition.md`. Assim, cenas e socket deixam de existir
antes de `DestroyWindow` e `PostQuitMessage`.

### Hook global de teclado

`FUN_0055F7F9 -> FUN_00423C1F` instala o hook após bootstrap bem-sucedido e
antes do loop principal. `FUN_00423C1F` publica o handle em `DAT_005CCF80` e
marca `DAT_005CCF84 = 1`.

`FUN_00423C61` possui um único caller, `FUN_0055DAB8:0x0055EBCB`. Se a flag
está inativa, retorna. Caso contrário, consulta `FUN_00423B25`, chama
`UnhookWindowsHookEx(DAT_005CCF80)` quando aplicável e sempre termina zerando
`DAT_005CCF84`. Esse cleanup ocorre depois de `Finalize` e antes da destruição
da janela.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Ação | Estado resultante | Erro/saída |
| --- | --- | --- | --- | --- |
| primeiro `WM_CLOSE` | Field ativa; timer zerado | envia `0x3AE/0x10`; grava relógio | fechamento adiado | packet não é repetido |
| `WM_CLOSE` durante gate | Field; menos de 3.000 unidades | consulta relógio | aplicação permanece ativa | retorna sem teardown |
| `WM_CLOSE` após gate | Field; limite atingido | `Finalize`, hook, janela, quit | processo sai do loop | ack não é precondição |
| `WM_CLOSE` fora de Field | qualquer outra cena | teardown imediato | processo sai do loop | nenhum `0x3AE` |
| init parcial na source | owner obrigatório ausente | pula notificação/espera insegura | teardown imediato seguro | guardas evitam null dereference |

### Vtables, vptrs e receptores

O thunk `FUN_0055FA89` fornece o singleton `DAT_013B71E0` como receptor de
`FUN_0055DAB8`. Seu vptr é `0x005A6104`, e o branch `WM_CLOSE` resolve o slot
virtual `+0x08` para `FUN_0055D066`. O deleting destructor
`FUN_0055F3E0`/`FUN_0055B26F` preserva essa mesma identidade até liberar o
objeto da aplicação; nenhum vptr de versão posterior é importado.

### Ownership

O singleton da aplicação possui os managers desmontados por `FUN_0055D066`.
A ordem completa está registrada na tabela de callees acima: ObjectManager,
TimerManager, RenderDevice, SoundManager, SocketManager, BGMManager, AviPlayer
e EventTranslator, seguidos pelo bitmap e pela janela auxiliar opcional. Cada
owner é zerado ao ser destruído, e a source conserva essa relação por nomes em
vez de copiar offsets nativos.

### Falha parcial

O nativo assume os owners esperados quando a Field está ativa. A source atual
é mais defensiva: exige timer, socket, ObjectManager e personagem local antes
de armar a espera e enviar o packet. Se esse conjunto não existe durante um
bootstrap/unwind parcial, ela continua para o teardown imediato. Essas guardas
não mudam o fluxo normal e evitam transformar um pedido de fechamento em
desreferência nula.

`SAFE_DELETE` também preserva o shutdown quando um owner nunca foi construído.
Não há retry de destruição nem rollback depois que `Finalize` começou; o estado
terminal é o encerramento da aplicação.

### Cleanup e teardown

O cleanup começa dentro de `FUN_0055D066`, inclui a árvore do ObjectManager e
o SocketManager e termina todos os owners antes da janela. Depois do `Finalize`,
`FUN_0055DAB8` fecha o dump opcional, limpa o hook por `FUN_00423C61`, chama
`DestroyWindow`, publica `PostQuitMessage(0)` e zera o gate temporal. O deleting
destructor posterior não repete o teardown interno.

### Shutdown

O controle System `635`, erros de inicialização e outras fontes podem postar
`WM_CLOSE`, mas convergem nesta mesma fronteira terminal. Field aguarda o gate
de 3.000 unidades após notificar `0x3AE`; as demais cenas desmontam de imediato.

### Logout e relogin

O logout de personagem por `634` é diferente: ele preserva socket e conta
autenticada e está fechado em `character-logout-selectchar-relogin.md`.

Depois do shutdown global não existe relogin dentro da mesma aplicação. Um novo
login exige novo processo/conexão; troca de conta e reconexão TCP ainda devem
ser rastreadas em fichas próprias.

## Wire, ABI e recursos

Packet client -> servidor no primeiro fechamento em Field:

| Campo | Offset | Tamanho | Valor |
| --- | --- | --- | --- |
| header base | `0` | 12 bytes | contrato `MSG_STANDARD` 7.48 |
| `Type` | `4` | 2 bytes | `0x3AE` |
| `ID` | `6` | 2 bytes | ID do personagem local |
| `Parm` | `12` | 4 bytes | `0` |
| total | — | — | 16 bytes |

O WYD-Go aceita `OpSysQuit = 0x3AE` somente com 16 bytes, persiste a posição e
devolve `wire.SysQuit(id)`, também com 16 bytes. O opcode é sobrecarregado no
7.48 e aparece em outras transições; por isso o servidor não encerra sessão,
trade ou loja ao recebê-lo. A desconexão real continua sendo determinada pela
queda do socket.

O envio nativo comprova os 16 bytes e `Parm=0`. `FUN_0055890A` não enumera
`0x3AE`; o gate exato da resposta S->C pertence ao contrato coordenado entre a
source atual e o WYD-Go e não é apresentado como validação nativa de framing.

## Mapeamento atual

### Source recompilável

`NewApp::WndProc(WM_CLOSE)` preserva o ramo Field, o primeiro envio de
`MSG_SysQuit`, `Type = 942/0x3AE`, `Parm = 0`, o gate temporal, `Finalize`,
fechamento do dump, flag terminal, `EnableSysKey`, `DestroyWindow`,
`PostQuitMessage(0)` e reset do timer.

`DelayStartPacket.h` concentra o ABI compartilhado de 16 bytes e oferece os
nomes `MSG_SysQuit` (`Parm=0`) e `MSG_DelayStart` (transições `Parm=1/2`) sem
duplicar a estrutura. `ReceivedPacketDispatch` valida a resposta coordenada antes do
callback existente da Field, preservando o mesmo buffer emprestado.

`NewApp::Finalize()` libera os três buffers de animação e destrói os mesmos
owners na mesma ordem essencial. A source usa nomes e `SAFE_DELETE` em vez de
copiar offsets do objeto nativo.

### WYD-Go

`World.onSysQuit` trata `0x3AE` como intenção sobrecarregada: sincroniza e
persiste a posição, responde com 16 bytes e deixa a desconexão real para o
socket. Esse comportamento evita interpretar teleporte, morte ou saída como
logout prematuro.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| entrada | thunk WndProc -> singleton | `WndProc` registrado | N/A | manter |
| Field close | envia uma vez e aguarda 3.000 unidades | mesmo fluxo e guardas de owners | aceita 16 bytes | manter |
| fora de Field | teardown imediato | mesmo fluxo | N/A | manter |
| ownership | ObjectManager, timer, render, sound, socket, BGM, AVI, input | mesmos owners e ordem essencial | socket determina disconnect | manter |
| falha parcial | pressupõe Field íntegra | null guards e `SAFE_DELETE` | save falho não gera ack | manter modernização segura |
| hook global | remove antes de destruir janela | `EnableSysKey` no mesmo ponto | N/A | manter |
| resposta `0x3AE` | gate nativo não enumera | handler existente, frame de 16 bytes | builder de 16 bytes | extensão coordenada com gate local |

## Decisões

- Evidência: `CONFIRMED` para a transição descrita e `CONTRACT` para o wire.
- Classificação de adaptação: `MODERNIZACAO_COMPATIVEL`.
- Ação: `manter` a source e o servidor atuais; não existe delta comprovado que
  justifique alteração funcional nesta unidade.
- Não copiar offsets, vtables ou globals nativos para o TMProject.
- Não promover para `CLIENT_TESTED` sem fechar a aplicação real a partir de
  Field e de uma cena não-Field no `client748/project.exe` hasheado.

## Lacunas

- Executar o primeiro close/espera/close final em Field e o close imediato
  fora de Field no candidato recompilado.
- Confirmar em runtime a saída quando o servidor não responde ao `0x3AE` e a
  persistência quando responde.
- Cobrir separadamente troca de conta, reconexão TCP e fontes individuais de
  `WM_CLOSE`; elas não reduzem a maturidade deste contrato estreito.

## Validação

- Pesquisa estática: callers, receptor virtual, branch Field/não-Field,
  packet, ordem de ownership, hook e estado terminal fechados no projeto
  Ghidra do hash registrado.
- Comparação: `NewApp.cpp`, Field, grid, wire e handler atuais inspecionados;
  a extração tipou o contrato compartilhado sem alterar timers ou efeitos.
- Automação: Debug e Release passaram pelo `Build-Client.ps1` com 1735
  checks/asserts em cada configuração. Os testes Go focados de SysQuit e os
  gates XML/caminhos/diff passaram. O candidato Release instalado possui
  SHA-256 `96646D17ED2F52F7CD4D87F5412BC995253DE14AF2C386B9C24D062590FB1800`.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
