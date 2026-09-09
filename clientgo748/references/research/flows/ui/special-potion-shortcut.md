---
id: special-potion-shortcut
title: Use special potion through E shortcut
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# Use special potion through E shortcut

## Pergunta

Como o client nativo 7.48 trata `E/e`, escolhe o item especial no Carry,
publica `MSG_UseItem` e antecipa visualmente o consumo sem perder o estado
autoritativo quando o servidor rejeita a intenção?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; exports focados
  `0044f88f_FUN_0044f88f.c`, `00452661_FUN_00452661.c`,
  `00453c59_FUN_00453c59.c`, `004541f3_FUN_004541f3.c` e
  `004662c5_FUN_004662c5.c`.
- Source: `TMFieldScene::UsePPotion`, `OnKeyPPotion`, `OnCharEvent` e o ABI de
  `MSG_UseItem` em `client-source/tmproject/Projects/TMProject/`.
- Servidor: `internal/game/consumables.go` e o teste de rejeição/cooldown em
  `internal/game/consumables_handler_test.go`.
- Assets: nenhum asset novo; os controles `0x417` e `0x9C98` apenas fornecem
  entradas observáveis adicionais.

## Fluxo nativo 7.48

### Entrada observável

- Evento: tecla `E/e`, controle de ação `0x417` ou controle de atalho
  `0x9C98`.
- Precondições: personagem vivo no dispatcher de teclado, Carry disponível,
  item com ability `38 == 230`, índice diferente de `3378` e cooldown
  compartilhado livre.
- Saída: packet `MSG_UseItem 0x373/36`, atualização otimista da pilha/slot,
  som 41 e refresh de score UI 16.

### Callers

- `FUN_00453C59` é o dispatcher de caracteres da Field. Dentro do gate
  `CurHP > 0`, chama `FUN_00452661` depois dos atalhos Q e W e encerra quando
  a tecla é consumida.
- `FUN_00452661` aceita somente `E/e`, chama `FUN_0044F88F` e retorna `1`;
  qualquer outra tecla retorna `0`.
- `FUN_004662C5` trata o controle `0x417`, chama diretamente
  `FUN_0044F88F` e consome o evento.
- `FUN_004541F3`, quando o conjunto de controles está ativo e o personagem
  está vivo, traduz o controle `0x9C98` para `FUN_00452661('e', 0)`.

### Função principal

`FUN_0044F88F` percorre o Carry com `X = 8..0` no laço externo e
`Y = 6..0` no interno. Para no primeiro item existente cuja ability 38 seja
230 e cujo índice não seja 3378. A exclusão ocorre durante a busca; portanto
um 3378 não bloqueia um candidato válido posterior.

Com candidato válido, consulta o tempo do servidor. Um timestamp anterior não
zero com diferença inferior a 200 ms encerra a função sem packet ou mutação.
Não existe o gate `m_cCancel` presente nos atalhos Q/W.

O grid normaliza tipo e posição por chamadas virtuais. Se a posição for `-1`,
o fallback é `x + 9*y`. O packet zerado recebe ID do personagem, opcode
`0x373`, `SourType = 1`, `SourPos`, coordenadas atuais e `ItemID = 0`, e é
publicado com 36 bytes. Só então o timestamp compartilhado avança.

Depois do envio, a quantidade é lida. Para quantidade menor que 2, o item é
retirado do grid, qualquer vínculo idêntico do cursor é limpo, o cursor é
desanexado e o slot de Carry é zerado. Para uma pilha maior, a quantidade e o
texto do grid são decrementados. O fluxo termina com som 41 e
`UpdateScoreUI(16)`.

### Callees

- `FUN_0054CD07`: ability 38 usada para identificar o item.
- Slots virtuais `+0xB4`, `+0xA4`, `+0xC0` e `+0xC4` do grid: leitura da
  célula, retirada do item, normalização e posição do Carry.
- Método virtual `+0x08` de `DAT_0092E654`: tempo do servidor.
- `FUN_0055F2DD`: envio do packet de 36 bytes.
- `FUN_0054E93B` e `FUN_0054E9AB`: leitura e alteração da quantidade.
- `FUN_0058F078`/`FUN_00421B8F`: atualização do texto da pilha.
- `FUN_00429A6D`/`FUN_0042AD2B`: som 41.
- `FUN_004431E4(16)`: refresh da score UI.

### Saídas e erros

- Tecla diferente retorna não consumida; `E/e` é consumida mesmo quando a
  ação não encontra item ou está em cooldown.
- Sem candidato, não há packet, som, refresh ou mutação.
- Cooldown é testado antes de normalizar o slot e antes de consumir.
- Slot inválido não existe no caminho nativo esperado; a source recompilável
  rejeita posições fora do Carry estrutural para impedir acesso indevido.
- Depois do envio, a mutação local é deliberadamente otimista. Rejeição do
  WYD-Go republica o slot autoritativo por `SendItem`.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| tecla não `E/e` | Field recebe caractere | `FUN_00452661` | inalterado | nenhum | retorna `0` |
| `E/e` sem candidato | personagem vivo | busca 9x7 | inalterado | nenhum | evento consumido |
| candidato 3378 | ability 230 | busca continua | inalterado | nenhum | tenta próxima célula |
| candidato em cooldown | timestamp recente | timer | inalterado | nenhum | evento consumido |
| pilha maior que 1 | slot válido | envia e decrementa | aguarda servidor com N-1 local | packet, texto, som, UI | servidor confirma ou recompõe |
| unidade final | slot válido | envia e remove | slot local vazio | packet, cursor, som, UI | servidor confirma ou recompõe |
| controle `0x417` | evento de controle | caller direto | igual a `E` | igual à ação principal | retorna `1` |
| controle `0x9C98` | controles ativos e vivo | traduz para `e` | igual a `E` | igual à ação principal | retorna handler |

### Vtables, vptrs e receptores

`FUN_0044F88F` recebe a Field. `Field + 0x2873C` é o receptor do grid e expõe
os slots virtuais usados para lookup, pickup e normalização. A source não
porta offsets/vptrs: reutiliza `SGridControl`, `GetCarryGridForSlot` e
`GetCarryCellForSlot`, que já materializam o único grid 9x7 do produto 7.48.
Timer, cursor, object manager, áudio e transporte são serviços externos já
possuídos pela cena/aplicação.

### Ownership

Antes do envio, o item pertence ao grid/Carry da Field. O packet é local à
pilha. A remoção otimista destrói somente o objeto visual retirado do grid e
zera a cópia local de Carry; o servidor continua proprietário do estado real
e, depois de resolver um slot ocupado válido, republica esse slot na
confirmação ou rejeição.

### Falha parcial

Todas as recusas locais anteriores ao envio não mutam estado. Após o envio, a
source pode mostrar pilha reduzida ou slot vazio. Quando `onUseItem` consegue
resolver um slot ocupado válido, reenvia `SendItem` no cooldown, item
desconhecido/não registrado, regra recusada e falha de persistência. Packet
inválido, personagem indisponível ou slot já vazio terminam antes dessa
resolução e não têm item autoritativo a republicar. O teste focado exige
explicitamente o reenvio no cooldown, cobrindo a reconciliação desta transição.

### Cleanup e teardown

Não há alocação persistente nem callback novo. Na unidade final, o item
retirado do grid é destruído, o cursor não conserva o ponteiro e o agregado
Carry local é zerado. A confirmação `SendItem` usa o lifecycle normal de
recriação/substituição do item visual.

### Shutdown

Não há trabalho assíncrono próprio. O packet já foi entregue ao transporte ou
morre com a sessão; grid, cursor e Field seguem o teardown global existente.

### Logout e relogin

O timestamp pertence à instância atual da Field e não atravessa sua
reconstrução. Carry e grid são novamente materializados a partir do estado do
servidor. Nenhum item retirado, ponteiro de cursor ou quantidade otimista
sobrevive ao logout/relogin.

## Wire, ABI e recursos

- Direção: client -> servidor.
- Opcode/tamanho: `0x373`, 36 bytes.
- Layout: header `0..11`; `SourType @12`; `SourPos @16`; `DestType @20`;
  `DestPos @24`; `GridX @28`; `GridY @30`; `ItemID @32`.
- Valores: `SourType = 1`, posição normalizada/fallback, destinos e ItemID
  zerados, coordenadas atuais convertidas para 16 bits.
- Seleção: ability 38 = 230; índice 3378 explicitamente excluído.
- Recursos: controles `0x417` e `0x9C98`, som 41 e refresh 16; nenhum asset
  importado.

## Mapeamento atual

### Source recompilável

`OnCharEvent` e `OnKeyPPotion` já continham o dispatch correto. `UsePPotion`
agora reproduz a ordem da busca, exclusão 3378, cooldown, ABI, consumo local,
cursor, som e refresh. A topologia superior da source é preservada: a
conversão slot/célula permanece centralizada e o layout do packet continua
protegido pelos `static_assert` de `WYD748Compat.cpp`.

### WYD-Go

`onUseItem` revalida personagem, packet, slot, catálogo, volatile, cooldown,
efeito e persistência. O servidor consome/persiste antes de publicar o novo
slot; recusas republicam o item autoritativo. O teste de cooldown agora prova
que uma rejeição enfileira exatamente uma ressincronização do slot.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject posterior | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| dispatch | `E/e`, `0x417`, `0x9C98` | entradas já vivas | apenas pista nominal | N/A | manter entradas atuais |
| seleção | X externo, Y interno; ability 230; exclui 3378 | reproduzido | não decide regra | revalida item real | paridade nativa |
| cooldown | 200 ms local compartilhado | reproduzido | arquitetura auxiliar | 100 ms autoritativo | manter ambos os gates |
| consumo | otimista após envio | reproduzido com helpers seguros | comportamento similar | persiste e confirma/recompõe | preservar UX e autoridade |
| wire | `0x373/36` | ABI já assertada | não decide layout | parser exato de 36 bytes | manter ABI 7.48 |

## Decisões

- Classificar o delta como `PARIDADE_NATIVA`.
- Preservar a estrutura atual superior de tradução de slots, sem copiar
  offsets ou vtables do executável.
- Reproduzir a exclusão do item 3378 e a ordem X/Y literalmente, pois alteram
  qual item é escolhido.
- Manter o consumo otimista comprovado para este atalho; sua segurança depende
  da ressincronização server-authoritative, agora coberta por teste focado.
- Manter status `CONTRACT`; build e testes automatizados não são uso real no
  client.

## Lacunas

- Executar `E/e`, `0x417` e `0x9C98` no `client748/project.exe` com unidade,
  pilha, item 3378 e cooldown.
- Capturar o packet e a confirmação/rejeição em sessão real.
- Confirmar visualmente a reconstrução da pilha após rejeição. Essas lacunas
  bloqueiam somente `CLIENT_TESTED`.

## Validação

- Pesquisa: função principal, dispatchers, callers adicionais, callees,
  ownership, falha parcial, teardown, shutdown e relogin fechados no corpus do
  hash registrado.
- Automação: teste focado de cooldown, `go test -count=1 ./...`,
  `validate_research.py` e `git diff --check` passaram.
- Build: `Build-Client.ps1` passou em Release/Win32 com 13 warnings já
  existentes e zero erros. O `client748/project.exe` instalado tem SHA-256
  `813DB51D963D14728A3D3ECB014759DAB77ADBDECA10D3AF4C9D80355039CB2A`.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
