---
id: socket-disconnect-return-selectserver
title: TCP disconnect, scene notification and return to server selection
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# TCP disconnect, scene notification and return to server selection

## Pergunta

Quantos sockets o client nativo WYD 7.48 possui, qual mensagem Win32 recebe
seus eventos, como `FD_READ` e `FD_CLOSE` alcançam a cena atual e qual owner
deve sobreviver ao login inicial, à migração e ao retorno à seleção?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto Ghidra: `WYD748Native_20260821.gpr`; xrefs e fluxo focados em
  `FUN_00423D7A`, `FUN_00423F37`, `FUN_004245DB`, `FUN_00484D44`,
  `FUN_0049889A`, `FUN_004AC985`, `FUN_0055D066` e `FUN_0055DAB8`.
- Assets: N/A; a transição não carrega recurso visual.
- Source atual: `CPSock.cpp`, `NewApp.cpp`, `TMGlobal.cpp`, `TMGlobal.h`,
  `TMScene.cpp`, `TMSelectServerScene.cpp`, `TMFieldScene.cpp`, `Basedef.h` e
  `WYD748Compat.cpp` em `client-source/tmproject/Projects/TMProject/`.
- Servidor: o contrato TCP existente recebe o login `0x20D`; nenhuma fronteira
  server-side exige um segundo socket ou uma segunda mensagem Win32.

Esta ficha fecha ownership, callback, disconnect e login/relogin do transporte.
Framing, decrypt, checksum e todos os opcodes recebidos continuam fora deste
recorte. O estado `CONTRACT` decorre do login wire `0x20D/0x74` e não implica
execução real do client recompilado.

## Fluxo nativo 7.48

### Entrada observável

- Evento: login inicial, migração de servidor/canal ou notificação Winsock
  assíncrona no WndProc da aplicação.
- Precondições e estado inicial: o único SocketManager pertence à aplicação em
  `app+0xFC`; o connect registra a mensagem `0x464` com máscara `0x21`
  (`FD_READ | FD_CLOSE`).
- Saída observável: leitura entrega packets ao `ObjectManager`/cena atual;
  fechamento normal notifica `OnPacketEvent(0, nullptr)`, que retorna à seleção
  ou consome o replay de migração; shutdown destrói o mesmo owner uma vez.

### Callers

- `FUN_004AC985` abre a conexão inicial na porta `0x2059`/8281 e usa callback
  `0x464` antes de enviar o login.
- `FUN_00484D44` reutiliza o mesmo SocketManager e o mesmo callback durante a
  migração; não cria um canal de login separado.
- `FUN_0055FA89 -> FUN_0055DAB8` é o WndProc da aplicação. O branch `0x464`
  recebe os eventos do socket registrado.
- `FUN_0055D066` é o teardown global e destrói somente o SocketManager em
  `app+0xFC`.

### Função principal

`FUN_00423F37` fecha uma conexão anterior, cria/conecta o socket, registra
`WSAAsyncSelect(socket, hwnd, 0x464, 0x21)` e envia os quatro bytes de
handshake `11 F3 11 1F`. O objeto conserva filas, posições e o handle do único
canal TCP usado pelo client.

No branch `0x464` de `FUN_0055DAB8`:

1. `FD_READ` chama `FUN_004245DB` e, quando há dados válidos, percorre as
   mensagens e as entrega ao dispatcher de cena;
2. retorno `0` de `FUN_004245DB` fecha o socket sem fabricar uma notificação de
   desconexão naquele branch;
3. `FD_CLOSE` chama `FUN_00423D7A` e depois despacha
   `OnPacketEvent(0, nullptr)`;
4. não existe branch nativo para `0x401` (`WM_USER + 1`).

`FUN_004245DB` preserva uma semântica não booleana: retorna `0` em
`SOCKET_ERROR`, `-1` quando o buffer disponível foi preenchido e `1` nos
demais casos, inclusive `recv == 0`. O WndProc testa apenas zero como falha;
o fechamento observável é consolidado pelo evento `FD_CLOSE`.

### Callees

- `FUN_00423D7A` fecha o handle e zera posições, filas e estado do socket.
- O dispatcher de packets usa o ObjectManager em `app+0xF8`, que encaminha à
  cena atual.
- `FUN_0049889A` trata o buffer nulo: no disconnect comum, mostra a mensagem
  aplicável e retorna à seleção de servidor; na migração, pode reproduzir uma
  vez o packet `0x52A` dentro da janela de 15 segundos.
- `FUN_004AC985` monta login `0x20D/0x74` com versão 748 depois da conexão
  inicial; `FUN_00484D44` usa o mesmo envelope na reconexão de migração.

### Saídas e erros

- Falha de criação, bind, connect ou `WSAAsyncSelect` fecha o handle parcial e
  não publica um segundo owner.
- `SOCKET_ERROR` em `recv` fecha o socket sem notificar antecipadamente a cena;
  `FD_CLOSE` é o caminho de notificação observável.
- Buffer cheio retorna `-1`, que é sucesso para o teste booleano do WndProc e
  permite que o parser consuma o que já foi recebido.
- `recv == 0` não é transformado localmente em erro; o callback de close fecha
  a transição.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| login inicial | seleção ativa | `FUN_004AC985 -> FUN_00423F37` | socket conectado | callback `0x464`; handshake; login `0x20D` | falha mantém seleção |
| `FD_READ` | socket conectado | `FUN_0055DAB8 -> FUN_004245DB` | filas atualizadas | packets chegam à cena | retorno `0` fecha sem notify |
| `FD_CLOSE` | qualquer cena conectada | `FUN_0055DAB8 -> FUN_00423D7A` | socket fechado | `OnPacketEvent(0, nullptr)` | cena decide seleção/replay |
| migração | Field e `0x52A` | `FUN_00484D44` | mesmo owner reconectado | login `0x20D/0x74`; replay one-shot | janela máxima de 15 s |
| shutdown | app finalizando | `FUN_0055D066` | owner destruído | socket e buffers liberados | init parcial tolerado |

### Vtables, vptrs e receptores

O receiver do transporte é o objeto concreto em `app+0xFC`; o dispatcher de
cena é o ObjectManager em `app+0xF8`. A notificação nula termina no slot de
packet da cena atual, não em uma segunda instância de socket. Nenhuma vtable,
global ou branch do nativo sustenta um receiver equivalente a
`g_LoginSocket`.

### Ownership

A aplicação possui exatamente um SocketManager. Login inicial, tráfego de jogo
e migração reutilizam esse owner; filas e chaves são reinicializadas no mesmo
objeto quando o protocolo exige. A aplicação também é a única responsável por
destruí-lo no teardown global.

### Falha parcial

O connect fecha handles criados quando uma etapa posterior falha. Como nenhum
segundo SocketManager é publicado, não existe teardown paralelo nem ordem
ambígua entre socket de login e socket de jogo. A notificação de cena só ocorre
no caminho nativo que conhece o fechamento assíncrono.

### Cleanup e teardown

`FUN_00423D7A` pode ser chamado em reconexão, erro de receive, `FD_CLOSE` e
shutdown sem conservar posições antigas. `FUN_0055D066` destrói somente
`app+0xFC`; não há segundo destructor ou global de login a limpar.

### Shutdown

O shutdown terminal continua detalhado em
`../lifecycle/application-close-global-shutdown.md`. Para esta ficha, o fato
decisivo é que o teardown global possui um único SocketManager e fecha o mesmo
canal que serviu login e Field.

### Logout e relogin

No disconnect comum, `FUN_0049889A` retorna à seleção de servidor e uma nova
tentativa passa novamente por `FUN_004AC985` usando o mesmo owner. Na migração,
o relogin usa `FUN_00484D44`, o envelope `0x20D/0x74` e o replay `0x52A`
documentado em `../lifecycle/field-scene-rebuild-after-server-move.md`.

## Wire, ABI e recursos

- Callback Win32: mensagem `0x464`, máscara Winsock `0x21`.
- Porta de conexão: 8281 (`0x2059`).
- Handshake inicial: bytes `11 F3 11 1F`.
- Login client -> servidor: opcode `0x20D`, tamanho `0x74`/116 bytes,
  `ClientVersion = 748`; a source protege tamanho e offset do marcador com
  `static_assert` em `WYD748Compat.cpp`.
- Nenhum packet, struct, packing, asset ou ID adicional é introduzido pela
  remoção do socket redundante.

## Mapeamento atual

### Source recompilável

- `g_pSocketManager` é publicado a partir de `NewApp::m_pSocketManager`, recebe
  `WSAInitialize`, conecta no login e na migração com callback decimal 1124
  (`0x464`) e é destruído por `NewApp::Finalize`.
- `NewApp::WndProc` em `WM_USER + 100` preserva `FD_READ`, `FD_CLOSE`, entrega
  de packets e notificação nula.
- `TMScene::OnPacketEvent(nullptr)` preserva replay de migração, mensagem de
  disconnect e retorno à seleção.
- Antes deste delta, `g_LoginSocket` era apenas declarado, definido, alocado e
  consultado por um branch `WM_USER + 1`; não possuía `WSAInitialize`, connect,
  send, consumidor ou destruction path. Esse caminho não podia receber um
  evento real.
- A implementação removeu esse global, sua alocação e o branch inalcançável. O
  único caminho restante continua sendo `g_pSocketManager` em
  `WM_USER + 100`, sem mudança de callback, wire ou ownership.

### WYD-Go

O servidor recebe a única conexão TCP e o login 7.48 já usados pelo caminho
ativo. Remover o objeto local inalcançável não altera framing, autenticação,
sessão, persistência nem reconexão server-side.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| ownership TCP | um owner em `app+0xFC` | owner ativo + global morto | redundância herdada | uma sessão por conexão | remover redundância |
| callback | somente `0x464` | ativo em `WM_USER+100`; morto em `+1` | branch posterior sem produtor | N/A | manter apenas `0x464` |
| disconnect | `FD_CLOSE` notifica cena | caminho ativo equivalente | bloco duplicado | socket close encerra sessão | manter ativo |
| login/migração | mesmo socket, `0x20D/0x74` | `g_pSocketManager` nos dois fluxos | estrutura ativa compatível | login 7.48 existente | modernizar sem mudar fronteira |
| teardown | um destructor | somente owner ativo é destruído | `g_LoginSocket` sem cleanup | sessão fecha com TCP | remover objeto morto |

## Decisões

- Classificar o delta como `MODERNIZACAO_COMPATIVEL`: remover declaração,
  definição, alocação e branch `WM_USER + 1` de `g_LoginSocket`.
- Preservar integralmente `g_pSocketManager`, `WM_USER + 100`, callback 1124,
  handshake, filas, login, migração, disconnect, retorno à seleção e shutdown.
- Não alterar o servidor nem criar abstração multi-socket para imitar código
  posterior que nunca integra o fluxo 7.48.
- Classificar a entrega no máximo como `STATICALLY VERIFIED` até executar
  disconnect, retorno à seleção, novo login e migração no `project.exe`.

## Lacunas

- Executar no client recompilado: queda TCP normal, mensagem/retorno à seleção,
  login seguinte e migração entre servidores/canais.
- Registrar o hash do candidato e os resultados antes de promover para
  `CLIENT_TESTED`.
- Framing, decrypt, checksum e política ampla de retry permanecem em fichas
  futuras de transporte.

## Validação

- Pesquisa: callers, callees, callback, ownership, teardown e wire foram
  resolvidos no projeto Ghidra do hash registrado; a ausência de `0x401` foi
  conferida no WndProc, e os caminhos de login/migração convergem no mesmo
  SocketManager.
- Automação: `validate_research.py` passou com `CONTRACT=4` e `LOCATED=3`;
  `Build-Client.ps1` terminou com zero erros e instalou o candidato de SHA-256
  `484580A681FB12226660084DAFBB1DACB93665C4F06C4A0853AEFFD13660069D`;
  `rg` confirmou a ausência de `g_LoginSocket` e do case exato
  `WM_USER + 1`; `git diff --check` passou.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
