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

## O dano viaja na escala do alvo

O prefixo nativo guarda HP em 16 bits, então `MaxHP` acima de 30.000 é
**escalado** por `ceil(MaxHP/30000)`. O WORD de dano dos pacotes de ataque
(`0x39D`, `0x39E`, `0x36C`) é subtraído pelo client desse `CurHP` **já
escalado** — portanto precisa da **mesma escala**.

Mandar o dano cru fazia a barra cair `scale` vezes mais rápido que o correto e
saltar de volta quando chegava o `0x181`. Com `MaxHP` 200.000 (escala 7), um
golpe de 3.000 derrubava 3.000 de 28.572 em vez de 429.

Os dois valores do pacote são **propositalmente diferentes**:

| campo | conteúdo | quem lê |
| --- | --- | --- |
| WORD `@46` | dano **projetado** (`model.ProjectHPDelta`) | a barra de HP |
| cauda `uint32 @48` | dano **real** | o número flutuante (client patcheado) |

A conversão é única, em `wire.wireDamage`. Antes havia duas saturações
divergentes do mesmo campo — 65535 no melee e 32767 na skill.

Cura usa o mesmo campo como **short negativo** e recebe a mesma projeção; por
isso `wire.SkillTarget` tem um campo `Heal` próprio, para o sinal ser explícito
em vez de vir embutido num `uint16` já convertido.

O arredondamento é para **cima**: com `MaxHP` alto, truncar deixaria todo golpe
menor que a escala invisível na barra.

**O client não precisa de patch novo para isto** — a correção é inteiramente
server-side.

## O base é limitado pelo próprio teto, não pelo efetivo

`CurHP ≤ MaxHP` vale em **cada** escala separadamente. A base é o score sem
equipamento e sem affect: o `MaxHP` dela é o único teto válido para o `CurHP`
dela.

Duas escritas limitavam pelo teto **efetivo** e gravavam o resultado na **base**:
o recálculo de equipamento, ao copiar o `CurHP` do runtime, e
`setPlayerCurHP`/`setPlayerCurMP`. Com a Aura da Vida ativa isso produzia
`curHP 1202183 / maxHP 1000000` **no disco**, no autosave — 20% acima do teto,
e crescendo a cada recast.

As duas agora aplicam `minU32(valor, base.MaxHP)`.

Limitar a base **não** custa HP em jogo, e essa é a metade que importa: se
custasse, todo personagem com equipamento de HP passaria a aparecer abaixo de
100% depois de qualquer recálculo — um bug trocado por outro. O valor vivo é o
do runtime, e ele sobrevive porque `recalcExtendedPlayer` captura
`oldHP`/`wasFullHP` a partir do **runtime**, nunca da base.

O MP foi o que denunciou a causa: nenhum affect aumenta `MaxMP`, então um
`curMP > maxMP` só podia vir do teto efetivo do equipamento.

**Onde editar valores à mão:** `extendedScore` no JSON da conta é a **base**.
Todo cálculo é `base + equipamento + affects`, e o resultado vive só em
`ExtendedRuntime`, que nunca é persistido.

`/hpdebug` imprime base, efetivo e projetado lado a lado com as proporções —
comando de beta, some quando a barra do client estiver resolvida.

## Regra para futuras alterações

Todo novo atributo deve entrar primeiro em `ExtendedScore`, receber fórmula
server-side e, se precisar ser exibido, ganhar um campo XSC2 ou uma projeção
explícita. Nunca adicione um segundo estado persistido para contornar o client.
