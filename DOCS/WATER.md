# Water — cadeia de instâncias e macro do client

## Estado entregue

Normal, Mystic e Arcane usam o mesmo motor `instance_ticket`, com uma
`VolatileInstance` independente para cada sala e para o boss. Não existe um
stage global Room 1--Boss. Cada transição cria um novo `RuntimeID`; quando o
template anterior ainda está em exit grace, o runtime recebe um sufixo livre
(`:1`, `:2`, ...), sem reutilizar a sala antiga.

Ao zerar `Remaining`, a sala é concluída, entrega o `RewardItem` ao líder (ou
derruba no chão se o carry estiver cheio), e permanece fisicamente no local por
dez segundos. Durante essa janela somente o próximo ticket é aceito. A falha
de persistência restaura inventário, posição, membro e runtime; nada é
publicado parcialmente. Expiração da sala ou da janela de saída usa `ExitX/Y`.

As três sequências autoritativas são:

- Normal: `3173 -> ... -> 3180`, depois `3181` (boss), e `3173` fecha o ciclo;
- Mystic: `777 -> ... -> 784`, depois `785`, e `777` fecha o ciclo;
- Arcane: `3182 -> ... -> 3189`, depois `3190`, e `3182` fecha o ciclo.

O boss não entrega o ticket da Room 1. `ChainNextItem` no boss expressa que um
ticket Room 1 já existente pode iniciar uma nova instância depois da conclusão.
`RewardItem` continua sendo a recompensa normal das salas 1--8. Os três tiers
possuem áreas distintas de uso; o boss ticket só é válido na plataforma da
Room 8 e o Room 1 ticket também aceita a área do boss. A posição usada na
validação é sempre a posição autoritativa do `World`, nunca `TargetXY` do
cliente.

O agregado de uma sala Water privada tambem persiste os UIDs dos personagens,
o UID do lider, `RewardGranted` e a janela `ExitAt`. Em logout/desconexao o
ID de mundo e removido, mas o UID fica pendente; no proximo login o personagem
e reanexado antes do primeiro `EnterWorld`, ja na coordenada da sala. Uma sala
privada sem UID estavel nao e gravada como instancia recuperavel. Stores sem
uma operacao atomica conta+instancia recusam o commit misto em vez de dividir
o estado.

A visibilidade de jogadores tambem respeita o `RuntimeID` privado: membros da
mesma sala continuam se vendo, enquanto jogadores de outra sala ou do mundo
publico nao recebem `CreateMob`, movimento, HP/MP, affects ou morte, mesmo que
as coordenadas fisicas se sobreponham. Ao sair ou encadear, o par antigo e
removido antes de qualquer nova materializacao.

O fallback de recompensa no chao tambem carrega o `RuntimeID`: somente os
membros da sala podem ve-lo/coleta-lo enquanto ela existe. Quando a sala e
encerrada definitivamente, o item e liberado para o mundo publico em vez de
ficar preso a uma instancia removida.

As comparacoes dos retangulos no cave usam a semantica unsigned correta:
`cmp min,current` sai com `JB` quando a posicao esta abaixo do minimo e
`cmp max,current` sai com `JA` quando esta acima do maximo. O teste estatico
confirma os quatro saltos (X/Y minimo e maximo), evitando que a macro rejeite
todas as areas nao degeneradas.

## Macro opcional do client 7.48

O estado inicia desligado. `/macropergaon` e `/macropergaoff` são interceptados
localmente e exibem a confirmação em inglês; não há packet de chat para esses
comandos. No tick do client, o scanner percorre exclusivamente os slots
visíveis `0..62` de uma única grade 9x7, filtra os Water Scrolls pela tabela
gerada de `data/volatiles.json` e chama a rotina nativa de uso manual. O
primeiro slot válido vence. Não há lógica de número de sala, contagem de mobs
ou prioridade de boss no client.

O script `client748/Patch-WYD748-WaterMacro.ps1` é um elo separado, posterior
ao patch da Lindy. Ele exige SHA, bytes dos hooks e uma cave zero livre; nunca
edita o executável fora da cadeia. `tools/watermacrotable` é a única origem da
tabela item-área e gera saída determinística. O servidor continua validando
slot, item, área, tier, sequência, prazo, spawn e persistência.

No binário 7.48, a chamada de grade e a rotina de uso de item empilham a
coordenada Y antes da coordenada X; o patch reproduz essa ordem dos call sites
nativos. O comparador do comando preserva ESI/EDI entre as tentativas ON/OFF,
e o teste estático valida o epílogo em `0x470D31` e o `thiscall` da mensagem em
`0x493AA1`, além dos dois hooks e da cave. A confirmação local só é chamada
quando o objeto de UI em `EBP-0x1AD4` está presente; durante teardown o comando
continua sendo consumido sem desreferenciar um ponteiro nulo.

## Cobertura

Os testes tambem cobrem RuntimeIDs privados concorrentes, anti-skip sem
predecessor, logout/reconexao por CharacterUID, recall definitivo e os quatro
ramos unsigned dos retangulos no cave (`JB` abaixo do minimo, `JA` acima do
maximo em X/Y).

`internal/game/water_test.go` cobre sequência normal, rejeição de qualquer
ticket não seguinte, novo RuntimeID e remoção da associação antiga somente
depois do commit. `internal/game/item_instances_test.go` cobre Room 8 -> Boss,
Boss -> Room 1, timeout, rollback, recompensa no chão e regressões de Cube e
Hunting Scroll. `tools/watermacrotable/main_test.go` garante descoberta dos
tickets Water, exclusão de não-Water, áreas válidas e serialização determinística.

Validação do elo do client:

```powershell
cd client748
.\Test-WYD748-WaterMacro.ps1
```
