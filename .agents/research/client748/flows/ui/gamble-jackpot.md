---
id: gamble-jackpot
title: Aposta, rolagem e resultado Gamble/Jackpot
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# Aposta, rolagem e resultado Gamble/Jackpot

## Pergunta

Como o client nativo 7.48 envia a intenção de aposta, recebe `0x1BF`, anima os
cinco resultados e encerra o painel sem tornar saldo, prêmio ou jackpot
autoritativos no client?

## Fronteira de evidência

- Native 7.48: `WYD.exe` do hash do frontmatter, projeto
  `WYD748Native_20260821.gpr`; `FUN_0040A189`, `FUN_0040A8C2`,
  `FUN_0040BD00`, `FUN_0040A8DE`, `FUN_0040A9CC`, `FUN_0040AC07`,
  `FUN_0040ACD2`, `FUN_0040B67A`, `FUN_00435B13`, `FUN_00489882` e
  `FUN_00492E7D`.
- Source: `Basedef.h`, `SControl.h/.cpp` e `TMFieldScene.h/.cpp`.
- Assets: controles `6400..6422` e imagens Jackpot já materializados no
  client ativo.
- Server: ainda sem dispatcher/handler Gamble no início desta ficha.
- Referência secundária: `TMSRVIN.exe` 7.59, SHA-256
  `27EE313BB4B4D4A0D897DAC9A188D8249068CE701311CBDA1E24AD28C5E43642`,
  branch `0x00554337..0x00554826`. Esse executável não prova o servidor 7.48.

## Fluxo nativo 7.48

### Entrada observável

O painel Gamble visível monta e envia uma intenção C->S `0x2BE/20`, com a
aposta `uint32` em `+0x10`. O client conserva a animação e a apresentação, mas
o resultado chega do servidor em `0x1BF/36`. `FUN_00492E7D` despacha a
resposta para `FUN_00489882`, que entrega os cinco símbolos, as três posições
de parada, prêmio e jackpot ao `SReelPanel`.

### Callers

- `FUN_00435B13` cria os reels e vincula o painel à árvore da Field.
- `FUN_00492E7D` é o dispatcher confirmado de `0x1BF`.
- `FUN_00489882` aplica a resposta ao estado visual da Field e do ReelPanel.
- O tick da cena chama o avanço visual, que chega a `FUN_0040ACD2`.

### Função principal

`FUN_0040AC07` instala o resultado recebido e prepara a animação. Cada reel
recebe símbolo e posição final; `FUN_0040ACD2` avança o estado até a parada e
então apresenta prêmio/jackpot. A resposta não recalcula o prêmio localmente.

### Callees

- `FUN_0040A8DE`: prepara/inicia uma rolagem individual.
- `FUN_0040A9CC`: propaga visibilidade de painel e filhos.
- `FUN_0040B67A`: atualiza a apresentação de jackpot.
- `FUN_0040A8C2` e `FUN_0040BD00`: destrutor e deleting destructor do painel.

### Saídas e erros

Packet inválido ou controle ausente não deve iniciar uma rolagem parcial. O
saldo final é recebido pelos snapshots autoritativos usuais; a UI Gamble não
deve descontar a aposta otimisticamente. Uma rejeição server-side termina por
mensagem e deixa a próxima tentativa partir do saldo real.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| abrir Gamble | controles/recurso disponíveis | lifecycle da Field | painel visível e reels ociosos | mostra saldo/aposta | controle opcional ausente não derruba a cena |
| enviar aposta | painel ocioso e aposta válida | envio `0x2BE/20` | aguarda resultado | nenhum desconto local | rejeição preserva estado autoritativo |
| receber resultado | `0x1BF/36` válido | `FUN_00492E7D` -> `FUN_00489882` | reels em rolagem | instala símbolos/paradas/prêmio/jackpot | packet curto não é aplicado |
| terminar rolagem | cinco reels chegaram ao stop | `FUN_0040ACD2` | resultado estável | mostra prêmio/jackpot | sem mutação server-side local |
| fechar painel | painel visível | `SetVisible(0)` | painel oculto | remove interação | resultado anterior não vira pedido pendente |
| falha de persistência | servidor preparou mutação | rollback server-side | saldo/jackpot anteriores | mensagem apenas | não publica `0x1BF` nem saldo otimista |

### Vtables, vptrs e receptores

O receptor nativo da animação é `SReelPanel`; os slots virtuais de destruição
são cobertos por `FUN_0040A8C2`/`FUN_0040BD00`. A adaptação usa os tipos C++
recompiláveis e não porta vptrs ou offsets internos do objeto nativo.

### Ownership

A árvore de controles da Field possui painel e reels. O packet copia seu
conteúdo para estado visual; não transfere ownership. No servidor, conta,
saldo e jackpot pertencem ao `World`, único escritor de gameplay.

### Falha parcial

Uma resposta só é aplicada depois de validar o tamanho integral. No servidor,
aposta, saldo e jackpot são mutados como uma unidade e revertidos se a
persistência falhar. Publicação acontece apenas após persistência.

### Cleanup e teardown

Ocultar Gamble interrompe interação e oculta filhos. A destruição da Field
destrói o ReelPanel pela árvore de controles. Não existe ownership de packet,
thread ou callback externo que sobreviva à cena.

### Shutdown

Shutdown elimina a Field e seus controles. O jackpot é estado do servidor e
não depende da vida do painel ou da sessão; o shutdown deve salvar o valor já
confirmado sem aguardar animação client-side.

### Logout e relogin

Logout descarta qualquer animação visual em andamento. Relogin começa com
reels ociosos e saldo persistido; o jackpot volta na próxima resposta Gamble.
Rejeição ou falha de save não deixa aposta, prêmio ou rolagem pendentes.

## Wire, ABI e recursos

### Intenção C->S

Opcode `0x2BE`, tamanho 20. A aposta é `uint32` em `+0x10`; os demais campos
seguem o `MSG_STANDARDPARM2` legado e não dão autoridade ao client.

### Resultado S->C

Opcode `0x1BF`, tamanho 36.

| Offset | Tamanho | Campo | Tipo/semântica |
| --- | ---: | --- | --- |
| `0x00` | 12 | `Header` | `MSG_STANDARD`; size 36, type `0x1BF` |
| `0x0C` | 5 | `Result` | cinco símbolos `uint8` |
| `0x11` | 3 | `StopPosition` | três posições `uint8` |
| `0x14` | 8 | `reserved` | bytes preservados/zerados |
| `0x1C` | 4 | `prize` | `int32` little-endian |
| `0x20` | 4 | `jackpot` | `uint32` little-endian |

Recursos/controles `6400..6422` e assets Jackpot já existem na source atual;
ausência de algum filho no recurso carregado deve ser tratada como controle
opcional, não como autorização para fabricar um widget diferente.

## Mapeamento atual

### Source recompilável

`SReelPanel` e o lifecycle Gamble já estão declarados, porém seus métodos e
`TMFieldScene::OnPacketRESULTGAMBLE` ainda são stubs. Há um desconto visual
otimista baseado em `m_nBet` que deve ser removido em favor do saldo recebido.

### WYD-Go

Ainda não possui `0x2BE`, resposta `0x1BF`, RNG ou jackpot persistente. O fluxo
TOTO existente fornece a disciplina de validar, mutar, persistir e só então
publicar, mas não fornece a regra Gamble.

## Referência de algoritmo 7.59

O dispatcher 7.59 exige feature habilitada, aposta `1.000..100.000`, saldo e,
para usuário normal, item `3916`. Ele desconta antes do sorteio, chama
`FUN_004D9100`, responde por `FUN_00506DF0` e paga por `FUN_00579210`.

Tabela de multiplicadores em `0x00640830`:

```text
[0, 2, 2, 2, 2, 5, 14, 50, 10, 20, 30, 50, 100, 150, 300, 500, 1000, 1500, 80]
```

O prêmio usa o reel central integral e metade dos quatro laterais:

```text
bet * multiplier(Result[1])
+ floor(bet * multiplier(Result[0]) / 2)
+ floor(bet * multiplier(Result[2]) / 2)
+ floor(bet * multiplier(Result[3]) / 2)
+ floor(bet * multiplier(Result[4]) / 2)
```

O jackpot monetário recebe `bet/1000`, limitado a 2 bilhões; um pool auxiliar
recebe `(bet/100)*3`, limitado a 4 bilhões. `prize/1000` sai do pool monetário.
Se `Result[1] == 14`, o jackpot entra no prêmio e é reiniciado. Overflow de
Coin vira itens `4010` (100 milhões) e `4011` (1 bilhão). Esses números são
referência de compatibilidade 7.59, não prova nativa 7.48.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | Referência 7.59 | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| wire `0x2BE/0x1BF` | tamanhos/offsets confirmados | símbolos e stubs presentes | compatível | ausente | `PARIDADE_NATIVA` |
| animação/lifecycle | funções e destrutores confirmados | estrutura superior já integrada | estrutura semelhante | N/A | `PARIDADE_NATIVA` com guards |
| RNG/pagamento | client não decide | não deve decidir | algoritmo recuperado | ausente | `MODERNIZACAO_COMPATIVEL` |
| jackpot persistente | apenas apresenta | apenas apresenta | pool global | ausente | autoridade Go persistente |
| saldo | snapshots do servidor | desconto otimista indevido | servidor paga | autoritativo | remover mutação local |

## Decisões

- Preservar exatamente o wire/UI 7.48 e completar a implementação source.
- Adotar a arquitetura server-authoritative e a fórmula 7.59 somente onde ela
  é compatível com o contrato 7.48, com RNG e persistência testáveis no Go.
- Nunca confiar em aposta, saldo, prêmio, símbolos ou jackpot vindos do client.
- Publicar `0x1BF` e os snapshots de saldo apenas após persistência; rollback
  integral em falha de save.
- Tratar controles Gamble herdados como opcionais durante teardown/atalhos.

## Lacunas

- Confirmar no `client748/project.exe` a abertura, aposta válida, rejeições,
  rolagem, fechamento, jackpot, overflow, logout durante animação e relogin.
- A distribuição exata de símbolos de `FUN_004D9100` no servidor 7.59 não é
  prova do 7.48; a implementação Go deve explicitar e testar sua distribuição.
- Até teste dentro do mundo, não alegar `CLIENT_TESTED`.

## Validação

- Pesquisa: wire, dispatcher, receptores, destrutores, animação, teardown,
  shutdown e relogin foram fechados para o contrato do client 7.48.
- Automação: pendente de testes Go, `validate_research.py`, build client e
  `git diff --check` deste lote.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
