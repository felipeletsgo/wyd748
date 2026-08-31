---
id: toto-buy
title: Compra e materialização do bilhete TOTO
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# Compra e materialização do bilhete TOTO

## Pergunta

Como o client nativo 7.48 valida uma aposta TOTO, monta e envia o packet
`0x3CE`, fecha o painel e como o WYD-Go transforma essa intenção em um bilhete
persistente sem confiar em preço, loja ou inventário enviados pelo client?

## Fronteira de evidência

- Native: `WYD.exe` do hash do frontmatter, projeto
  `WYD748Native_20260821.gpr`; `FUN_004727CC`, caller
  `FUN_004662C5` e callees `FUN_00472966`, `FUN_0055F2DD`,
  `FUN_005909D2` e `FUN_00591258`.
- Native/UI: `FUN_00410A91` prepara a mensagem-base ao usar o item `4147`;
  seleção e fechamento estão em
  [`../ui/toto-selection-close.md`](../ui/toto-selection-close.md).
- Source: `Basedef.h :: MSG_BuyToto`, `SGrid.cpp :: BuyItem` e
  `TMFieldScene.cpp :: TotoBuy`.
- Server: `internal/wire/opcodes.go :: OpBuyToto` e
  `internal/game/handlers.go :: onBuyToto`.
- Data: `data/itemlist.csv`, item `4147`, preço autoritativo atual `500000`.

## Fluxo nativo 7.48

### Entrada observável

O uso do item de loja `4147` entra em `FUN_00410A91`, prepara uma
`MSG_STANDARD` com tipo `0x3CE`, conserva NPC, posição da loja e destino no
Carry, abre o painel e entrega foco ao número da partida. O botão comprar
`8978` chega a `FUN_004727CC` pelo dispatcher de controles
`FUN_004662C5`.

### Callers

`FUN_004662C5` é o caller direto confirmado de `FUN_004727CC`. No mesmo
dispatcher, os IDs `8964` e `8966` chamam seleção e fechamento; o ID `8978`
chama a compra. A preparação anterior da mensagem é feita por
`FUN_00410A91`, mas ela não chama diretamente a função de compra.

### Função principal

`FUN_004727CC @ 0x004727CC` exige painel/controles utilizáveis, seleção de
partida válida e os dois placares em formato decimal aceito. Ela escreve
partida e placares na mensagem preparada na abertura, envia exatamente 36
bytes e chama o fechamento local. Falha de validação retorna sem envio e sem
fechar o painel, permitindo correção dos campos.

### Callees

- `FUN_005909D2`: converte o texto decimal dos campos.
- `FUN_00591258`: valida a forma textual usada na entrada numérica.
- `FUN_0055F2DD`: envia a mensagem montada ao socket.
- `FUN_00472966`: remove foco, oculta o painel e zera a seleção depois do
  envio aceito.

O fluxo é síncrono. Não há callback de confirmação, fila TOTO própria ou
estado de pedido pendente entre envio e fechamento.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| abrir compra | item 4147 em loja e destino livre | `FUN_00410A91` | painel visível, mensagem-base pronta | conserva NPC/slots | controle ausente não envia |
| comprar válido | partida e dois placares válidos | `FUN_004727CC` | painel fechado, seleção zero | envia `0x3CE/36` | sem confirmação TOTO dedicada |
| placar/partida inválido | painel aberto | validações locais | painel permanece aberto | nenhum packet | usuário pode corrigir |
| rejeição server-side | contexto ou valor diverge | `onBuyToto` | personagem inalterado | `MessagePanel` | não persiste nem publica inventário |
| sucesso server-side | loja/item/destino/gold válidos | `onBuyToto` | bilhete e gold persistidos | `UpdateCarry` + `UpdateEtc` | publica depois do save |
| falha de persistência | mutação preparada | rollback | gold/inventário restaurados | `MessagePanel` | nenhum snapshot otimista publicado |

### Vtables, vptrs e receptores

O receptor nativo é a `TMFieldScene` ativa. Campos e textos são acessados por
seus controles virtuais; a adaptação usa os tipos recompiláveis e não porta
vptrs nem offsets de objeto. No servidor, o receptor é o `Player` associado à
sessão e ao único `World` escritor de gameplay.

### Ownership

A Field possui a mensagem-base e ponteiros emprestados da árvore de controles.
O envio copia 36 bytes e não transfere ownership desses controles. No servidor,
o inventário e gold pertencem à conta; o item materializado recebe UID antes
do save e só é publicado ao client após persistência bem-sucedida.

### Falha parcial e rollback

Falha local não altera a mensagem publicada nem fecha o painel. O servidor
revalida jogador vivo/in-world, merchant aberto, identidade, alcance e
visibilidade do NPC, posição da loja, item `4147`, destino vazio, partida,
placares, definição e gold. `Coin` é ignorado. Se o save falha após a mutação
em memória, bilhete e gold retornam integralmente aos valores anteriores.

### Cleanup e teardown

Depois do envio nativo, `FUN_00472966` remove foco, oculta o painel e zera a
seleção. Não existe heap, handle, timer ou callback próprio da compra. Rejeição
server-side também não cria estado TOTO pendente.

### Shutdown

Shutdown da Field elimina a árvore de controles e a mensagem-base com a cena.
Como não há operação assíncrona TOTO, nenhum cancelamento adicional é exigido.
O servidor persiste antes de responder; uma sessão que termina depois do save
reencontra o estado salvo no próximo login.

### Logout e relogin

Uma nova Field começa com painel oculto, seleção zero e uma nova mensagem-base
somente quando o item é usado novamente. No servidor, sucesso já está na conta;
relogin restaura bilhete/UID/gold. Rejeição ou falha de save não deixa bilhete,
desconto ou pedido pendente para atravessar o relogin.

## Wire, ABI e recursos

Direção: client para servidor. Opcode: `0x3CE`. Tamanho: 36 bytes.

| Offset | Tamanho | Campo | Tipo/semântica |
| --- | ---: | --- | --- |
| `0x00` | 12 | `Header` | `MSG_STANDARD`; size 36, type `0x3CE` |
| `0x0C` | 2 | `TargetID` | unsigned; NPC esperado |
| `0x0E` | 2 | `TargetCarryPos` | signed short; posição visual da loja |
| `0x10` | 2 | `MyCarryPos` | signed short; destino no Carry |
| `0x12` | 2 | padding | alinhamento antes dos inteiros |
| `0x14` | 4 | `Coin` | valor não confiável; ignorado pelo servidor |
| `0x18` | 4 | `Gindex` | partida decimal |
| `0x1C` | 4 | `A_Score` | placar A decimal |
| `0x20` | 4 | `B_Score` | placar B decimal |

`MSG_BuyToto` possui `static_assert` de tamanho e de todos os offsets. O teste
server-side constrói o packet byte a byte. Partida aceita `1..80`; placares
aceitam `0..127` porque o item persistente usa pares de efeitos byte
`{64,partida,65,placarA,66,placarB}`.

## Mapeamento atual

### Source recompilável

`SGridControl::BuyItem` prepara explicitamente a mensagem sem copiar uma struct
de compra diferente. `TMFieldScene::TotoBuy` valida decimal estrito, limites,
mensagem-base e socket, envia 36 bytes e fecha somente após envio aceito. O
tooltip do item `4147` lê os efeitos 64/65/66 do bilhete materializado.

### WYD-Go

`OpBuyToto` participa do gate exato de 36 bytes, métricas e dispatcher. O
handler trata o packet como intenção: ignora `Coin`, recupera o preço da
definição autoritativa, revalida a loja e materializa UID/efeitos. O protocolo
nativo não tem confirmação TOTO dedicada; sucesso publica os snapshots
autoritativos `UpdateCarry` e `UpdateEtc`, enquanto rejeição/falha publica
somente `MessagePanel`.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| wire `0x3CE` | 36 bytes e offsets confirmados | struct/asserts e envio equivalentes | gate/dispatch byte-exato | `PARIDADE_NATIVA` |
| validação local/fechamento | valida e fecha depois do envio | decimal estrito e guards compatíveis | N/A | `PARIDADE_NATIVA` + proteção |
| preço/autoridade | client carrega `Coin` | campo preservado no wire | ignora e usa ItemList | `EXTENSAO_COORDENADA` |
| identidade do bilhete | não fecha autoridade server-side | exibe efeitos 64/65/66 | materializa UID/efeitos | `EXTENSAO_COORDENADA` |
| transação | sem confirmação dedicada | recebe snapshots existentes | persist-before-publish e rollback | `EXTENSAO_COORDENADA` |

## Decisões

- Manter o wire/UI `0x3CE` como `PARIDADE_NATIVA` em estado `CONTRACT`.
- Tratar `Coin`, NPC, slots, item, partida e placares como intenção não
  confiável; o servidor é a única autoridade sobre preço e mutação.
- Usar o formato de efeitos já suportado pelo item para materializar a aposta,
  com UID persistente, como `EXTENSAO_COORDENADA` explícita.
- Não inventar confirmação TOTO S->C; reutilizar snapshots e painel de mensagem
  já compreendidos pelo client.

## Lacunas

- Executar no `client748/project.exe` a abertura, seleção, compra válida,
  placar inválido, gold insuficiente, destino ocupado e fechamento.
- Confirmar em jogo o tooltip do bilhete e o mesmo UID/efeitos após relogin.
- Até esses cenários, o estado máximo é `STATICALLY VERIFIED`/
  `AUTOMATED TESTED`; não alegar `CLIENT_TESTED`.

## Validação

- Pesquisa: entrada, caller, callees, wire, signedness, fechamento, ausência de
  pending state, teardown, shutdown e relogin foram fechados no hash nativo.
- Client: `Build-Client.ps1` passou em Release/Win32 com 23 warnings existentes
  e zero erros; instalou `client748/project.exe` SHA-256
  `E7C6307886B29C7D727F7D8558B81B439953D58A08877FA58B1D8F793F129F94`.
- Servidor: testes focados cobrem sucesso, preço autoritativo, UID/efeitos,
  persist-before-publish, rejeições sem mutação e rollback de save.
- Integração: `go test -count=1 ./...`, `validate_research.py` e
  `git diff --check` passaram após a documentação final do lote.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
