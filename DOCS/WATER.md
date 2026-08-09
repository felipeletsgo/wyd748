# Water — cadeia de instâncias e Silver Angel

## Estado entregue

Normal, Mystic e Arcane usam o mesmo motor `instance_ticket`, com uma
`VolatileInstance` independente para cada sala e para o boss. Cada
transição cria um novo `RuntimeID`; salas privadas concorrentes não
compartilham mobs, visibilidade, loot ou estado.

Ao zerar `Remaining`, a sala entrega `RewardItem` somente ao líder e
persiste a recompensa junto ao estado da instância antes de publicar.
Inventário cheio mantém o fallback nativo no chão. Depois da conclusão
existe uma janela de saída de dez segundos em que somente o próximo
ticket autoritativo da cadeia pode ser usado.

As três sequências são:

- Normal: `3173 -> ... -> 3180`, depois `3181` (boss), e `3173` fecha o ciclo;
- Mystic: `777 -> ... -> 784`, depois `785`, e `777` fecha o ciclo;
- Arcane: `3182 -> ... -> 3189`, depois `3190`, e `3182` fecha o ciclo.

O boss não entrega Room 1. `ChainNextItem` expressa apenas que um
Room 1 Scroll já existente pode iniciar novo ciclo durante a saída.
`RewardItem` continua sendo a recompensa das salas que realmente
concedem pergaminho.

## Auto-avanço com Fada Prateada

A automação é exclusivamente server-side. No momento em que uma sala
termina:

1. o servidor materializa o próximo scroll com UID próprio;
2. grava conta + estado da instância;
3. publica o reward ao líder;
4. verifica se o líder está com Silver Angel (`3914`) ativa em
   `Equip[13]`;
5. se estiver, usa somente o UID recém-concedido pela mesma rotina de
   domínio usada pelo clique manual;
6. a nova sala é persistida antes de teleporte/spawn/publicação.

A fada no inventário não ativa automação. Equipá-la depois do reward
não causa avanço retroativo. A fada de outro membro da party não conta.
Reward no chão por Carry cheio não avança. Reward `0` não procura um
scroll antigo. Se o commit da nova sala falhar, o runtime novo é
revertido e o scroll concedido no primeiro commit permanece no Carry.

O servidor não varre o inventário, não escolhe por índice e não cria um
pacote `0x373`: o recibo interno carrega `slot + item index + UID`, e o
caminho automático exige que esses três valores ainda coincidam com o
estado autoritativo.

## Persistência e isolamento

O agregado Water privado persiste UIDs dos personagens, UID do líder,
`RewardGranted` e `ExitAt`. Logout remove o ClientID vivo, não a
identidade persistente; o personagem pode ser reanexado pelo
CharacterUID. Stores incapazes de confirmar conta+instância em uma
transação recusam operações mistas em vez de dividir o estado.

Visibilidade, combate, IA e itens de chão usam o mesmo `RuntimeID`.
Quando uma party encadeia para a sala seguinte, a associação antiga é
destacada dentro do snapshot durável; o cleanup posterior da sala velha
não pode teleportar membros que já estão no novo runtime.

## Client 7.48

`/macropergaon` e `/macropergaoff` foram removidos. O antigo
`Patch-WYD748-WaterMacro.ps1`, seu teste e `tools/watermacrotable`
também foram removidos. `Patch-WYD748-Macro.ps1` permanece: ele é o
macro normal de skills/buffs e não depende da automação Water.

A cadeia suportada do executável agora termina no patch da Lindy. Um
binário legado com WaterMacro é reconhecido pelo orquestrador e
reconstruído a partir do original, preservando os cinco patches
anteriores.

## Cobertura

`internal/game/water_test.go` mantém a cobertura de sequência manual,
exit grace, anti-skip, Room 8 -> Boss, Boss -> Room 1, runtime privado,
rollback e isolamento. `internal/game/water_auto_test.go` cobre Silver
ativa, fada ausente/no inventário/no membro, ausência de retroatividade,
UID duplicado do mesmo índice, Carry cheio, falha no grant, falha no
segundo commit e reward zero.

Validação do client:

```powershell
cd client748
.\Apply-WYD748.ps1 -VerifyOnly
```
