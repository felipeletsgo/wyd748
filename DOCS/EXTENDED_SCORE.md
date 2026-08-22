# ExtendedScore v2

## Regra arquitetural

`model.ExtendedScore` v2 é a única estrutura de atributos do servidor:

- `model.Char.Extended` é a base persistida do jogador;
- `model.Char.ExtendedRuntime` é a cópia efetiva após equipamentos, passivas e
  affects;
- `model.NPCDef.Extended` é a base persistida de NPCs e monstros;
- `game.Mob.HP` mantém o HP atual do monstro em `uint32`.

Combate, requisitos, recursos, regeneração, progressão e skills leem somente
essas estruturas. Não existem `Score`, `BaseScore`, atributos planos, magia ou
resistências paralelos nos JSONs.

## Formato persistido

Contas, templates de criação e os 476 NPCs usam `extendedScore` com
`"version": 2`. O contrato contém level, ataque físico, ataque mágico, defesa,
HP/MP, STR/INT/DEX/CON, accuracy, evasion, parry, critical, range, quatro
resistências, save mana, magic amp, regen HP/MP, pontos de status/mastery/skill,
quatro masteries, AttackRun e Merchant.

Os loaders usam `DisallowUnknownFields`, exigem exatamente a versão 2 e rejeitam
conteúdo JSON adicional. Contas exigem `passwordHash`; inventários de personagem
exigem os 64 slots estruturais. Um slot vazio da lista de personagens é
representado por `null`, nunca por um personagem parcial. Não há compatibilidade
automática, importador antigo nem migrador mantido no repositório.
O cargo exige 128 entradas estruturais; os slots sem UI `120..127` devem estar
vazios, assim como o slot reservado `63` do inventário.

Valores efetivos são calculados com intermediários `int64` e limitados a
2.000.000.000. Campos percentuais continuam limitados pela regra do jogo
(resistências e parry em 100, save mana em 99 e regen em 255).

## ABI inevitável do client 7.48

O executável usa `TMHuman+0x444` como início de um `STRUCT_SCORE` fixo de 28
bytes. A auditoria estática encontrou 255 acessos nessa região para jogador
local, outras entidades, NPCs e monstros. Alterar o tamanho dessa struct quebra
offsets internos do binário.

Por isso `model.WireScore` permanece somente como tipo de ABI. Ele nunca é
persistido e nunca alimenta cálculo. No envio,
`ExtendedScore.CompatibilityScore()` cria uma projeção descartável:

- ataque, defesa e atributos: no máximo 1.000;
- HP/MP: proporção preservada com máximo 30.000;
- mastery: no máximo 255;
- level, Merchant e AttackRun: representação compatível.

Para NPCs, `NPCDef.MakeExtendedScore()` clona a estrutura wide e injeta apenas o
HP atual. Para jogadores, o wire usa `ExtendedRuntime` quando disponível. Essa
projeção estreita nunca é copiada de volta para o servidor.

## Pacote `0x336` XSC2

O prefixo nativo de 92 bytes permanece intacto e a cauda wide produz um pacote
de 236 bytes:

| Offset | Campo |
|---:|---|
| 92..152 | MATK, ATK, DEF, HP/MP, atributos, accuracy/evasion e pontos |
| 156 | Level |
| 160..172 | Mastery 0..3 |
| 176..184 | Critical, Parry e Range |
| 188..200 | Resistências |
| 204..216 | SaveMana, MagicAmp, RegenHP e RegenMP |
| 220..224 | AttackRun e Merchant |
| 228 | Versão (`2`) |
| 232 | Assinatura `XSC2` |

O patch do `WYD.exe` captura a cauda para o personagem local e também executa o
handler nativo do prefixo. O sidecar do client guarda os números `uint32`; o
prefixo proporcional mantém animação, affects e barras internas compatíveis.

O `0x181` segue a mesma separação: 20 bytes nativos mais HP/MP reais na cauda,
totalizando 36 bytes. Pacotes de ataque recebem somente a projeção de mana no
campo WORD. Assim, nenhum campo estreito sobrescreve os recursos autoritativos.

## Client recompilável

A obrigação de manter `STRUCT_SCORE` em 28 bytes pertence ao executável binário
atual em `client748/`. A source recompilável em `client-source/tmproject/` usa
um score de 48 bytes, com ataque, defesa e HP/MP em 32 bits. O servidor possui
um encoder separado e byte-exato para esse layout, mas os packets vivos ainda
permanecem no protocolo 7.48 até serem migrados individualmente.

Mesmo no client novo, o score de 48 bytes continua sendo projeção. Atributos que
não cabem nele permanecem no `ExtendedScore` e na extensão `XSC2`; nada recebido
do client retorna aos cálculos do servidor.

## Regra para futuras alterações

Todo novo atributo deve entrar primeiro em `ExtendedScore`, receber fórmula
server-side e, se precisar ser exibido, ganhar um campo XSC2 ou uma projeção
explícita. Nunca adicione um segundo estado persistido para contornar o client.
