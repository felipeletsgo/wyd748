# Quests e eventos — síntese técnica em português

Fonte: páginas oficiais em `https://www.wyd2.co.kr/introduction/`.

---

## 1. Regras gerais de quests

**Fonte:** https://www.wyd2.co.kr/introduction/quest.asp

Ao entrar no jogo, o client exibe um aviso/ícone de quest quando existe conteúdo adequado ao level atual. A janela pode ser aberta pelo aviso ou pelo atalho **X**.

A página organiza as quests em três grupos históricos:

- quests de progressão até 256;
- event quests;
- quests de personagem Trans.

Quests disponíveis aparecem com destaque diferente das que ainda não atendem o level exigido.

### Tipos

- **repetível:** pode ser refeita enquanto os requisitos forem atendidos;
- **single/one-shot:** só pode ser concluída uma vez.

Itens obtidos em quests com faixa de level podem ter uso limitado à mesma faixa. Algumas quests possuem janela de horário; o site alerta para diferença de 1–3 minutos entre servidores. O minimapa (`M`) ajuda a localizar NPCs.

### Implicação server-side

O servidor deve manter autoridade sobre:

- elegibilidade;
- conclusão anterior;
- faixa de level;
- horário;
- consumo/entrega de item;
- recompensa.

O aviso do client é apenas apresentação.

---

## 2. Orc Castle e Magical Pill

**Fonte:** https://www.wyd2.co.kr/introduction/quest_256_03.asp

A quest ocorre em Orc Castle. Para avançar até Orc Lord, o grupo precisa eliminar guardiões dos portões, obter chaves e abrir as passagens. As chaves são de uso único e os portões voltam a fechar após determinado período.

O Orc Lord possui a **Magical Pill**, que aumenta Skill Points sem level-up. A página informa bônus de **9 Skill Points** e uso efetivo apenas uma vez por personagem.

Party summon não é permitido dentro de Orc Castle segundo o guia.

### Pontos para o WYD-Go

- Magical Pill precisa de flag persistida one-shot;
- key consumption deve ser atômico;
- gate deve voltar ao estado fechado por timer server-side;
- summon precisa consultar a região autoritativa.

---

## 3. Quests avançadas de personagem Trans

**Fonte:** https://www.wyd2.co.kr/introduction/quest_js.asp

Personagem Trans a partir de **level 355** pode consultar a janela de quests avançadas.

O fluxo descrito:

1. caçar quest monsters dentro de Kefra Dungeon;
2. obter os itens necessários;
3. os itens podem ser coletados em qualquer ordem;
4. a utilização/conclusão deve ocorrer sequencialmente, da primeira para as posteriores;
5. cada etapa consome **100.000.000 EXP**;
6. a conclusão aumenta atributos;
7. após a etapa, o personagem volta a evoluir até preencher novamente o requisito para a próxima;
8. cada uma das quatro etapas é one-shot.

A página menciona requisito de Trans 355+ e EXP mínima alta para efetivar a etapa, além de necessidade histórica de reconexão para voltar a receber EXP após a quest. O comando `/qst` é indicado para consultar estado.

---

## 4. Seven Trials, Runes, Secret Stones e Ideal Stone

**Fonte:** https://www.wyd2.co.kr/introduction/quest_js_02.asp

Após a derrota de Kefra, o servidor libera acesso ao Unknown Continent segundo o fluxo histórico descrito.

As **Seven Trials** são sete zonas voltadas a party. O líder precisa portar a **Clue of Rune** e registrar a party no NPC Uxmal durante a janela de espera. A página informa mínimo de quatro membros para o fluxo geral e capacidade total de **21 parties** registradas.

A Clue of Rune pode ser obtida com Odin mediante **7 Lactolerium Powders** na regra documentada.

### Ciclo do evento

O período de uma hora é descrito em três blocos de:

```text
15 min atividade
5 min intervalo
```

As parties registradas são distribuídas entre as sete zonas. Cada prova possui condição própria; o prêmio é um rune item.

### Sete provas

| Rune / prova | Parties mín. | Parties máx. | Estrutura resumida |
|---|---:|---:|---|
| Ansuz | 2 | 2 | área cercada por muralhas móveis |
| Uraz | 1 | 3 | três torres |
| Thurisaz | 1 | 3 | arena circular central |
| Gebo | 1 | 3 | quatro gates/portais, cinco campos |
| Hagalaz | 1 | 3 | oito salas de boss |
| Isa | 1 | 3 | labirinto/salas fechadas, boss central |
| Jara | 1 | 3 | armazém com três acessos |

Existem **24 runes**. Sete runes são combinadas em Odin por **2.000.000 gold**; a ordem determina chance e tipo do Secret Stone obtido.

Quatro Secret Stones, Sephirot e Eternal Stone formam a **Ideal Stone** por interação com o king.

A página descreve a Ideal Stone como componente da transição para Reclass, condicionada a Mortal 400 e Trans 356+ no fluxo histórico. O level do Trans influencia stats iniciais e a qualidade da Circlet recebida:

- 356–380: Silver Circlet;
- 381–399: Gold Circlet;
- 400: Mystic Circlet.

A página também registra regras de desconexão/morte dentro das trials e competição de recompensa entre parties.

---

## 5. Incursão ao reino inimigo / Token of Courage

**Fonte:** https://www.wyd2.co.kr/introduction/quest_event_07.asp

O evento descreve incursões entre Acheronia e Hekalotia. Cada reino possui guardas e torres que reagem aos cavaleiros inimigos.

O objetivo relevante para a cadeia de quests é matar o **king do reino inimigo**. O vencedor recebe **Token of Courage**, item que habilita o portador e sua party a entrar na última sala da Secret Room.

### Implementação

O Token deve ser concedido pelo evento server-side associado ao king kill; não deve ser produzido a partir de estado informado pelo client.

---

## 6. Secret Room

**Fonte:** https://www.wyd2.co.kr/introduction/quest_event_08.asp

A Secret Room é uma instância/evento de quatro andares/salas. A party entra pelo acesso em Noatun Castle usando uma **Invitation**; somente o líder precisa portar/usar o item no fluxo descrito.

Para chegar à última sala em determinadas dificuldades, o líder precisa estar com o **Token of Courage** equipado no slot indicado pelo guia.

### Fluxo

1. entrada da party;
2. spawn de monstros da sala;
3. combate por tempo limitado;
4. matar todos pode antecipar a transição nas primeiras salas;
5. a party avança para a próxima;
6. fragmentos/rune tablets de cada andar formam uma sequência;
7. se a sequência final representar `W.Y.D`, o boss final pode aparecer;
8. mesmo com boss, o drop especial é probabilístico.

A página documenta **220 segundos por andar** e populações históricas de 100 mobs nos três primeiros e 20 + boss no quarto, variando as famílias de monstros entre Normal/Mystic/Arcane.

Spirit Stones especiais podem ser obtidas do boss em condições adequadas, com variantes de Absolute Damage, Damage Absorption e HP.

Morte, recall/warp ou disconnect encerram a participação segundo a regra da página. Outra party não pode ocupar simultaneamente a mesma Secret Room no modelo descrito.

---

## 7. Eternal Stone

**Fonte:** https://www.wyd2.co.kr/introduction/quest_event_09.asp

A quest fica disponível a partir de **level 280** no Oracle do reino correspondente.

O jogador caça monstros específicos para obter:

- Phoenix Soul;
- Unicorn Soul.

Com ambos e **10 Sapphires**, o Oracle combina os componentes em **Eternal Stone**.

A página exige posicionamento correto dos souls no inventário para a composição histórica.

O Eternal Stone é componente central da progressão Trans/Sephira e é usado no slot de orb para continuar a cadeia de transformação.

### Relevância

No emulador, o layout exigido do inventário deve ser confirmado pela source/protocolo antes de ser flexibilizado; se reproduzido, a ordem/posição deve ser validada server-side.

---

## 8. Sephira stone / Sephirot

**Fonte:** https://www.wyd2.co.kr/introduction/quest_event_10.asp

Sephira raw stones são obtidas de monstros específicos. Refinar a raw stone com **Oriharukon Powder** pode convertê-la em um dos atributos Sephira.

A página apresenta uma matriz de probabilidades por monstro para atributos como:

- Keter;
- Binah;
- Chesed;
- Geburah;
- Tiphereth;
- Netzach;
- Yesod;
- Malkuth.

O custo de combinação com Skill Master é documentado como **30.000.000 gold**.

Depois de obter Sephirot e Eternal Stone, o jogador continua a cadeia de transformação no king do próprio reino.

### Implementação

A matriz de probabilidades é conteúdo; se a mecânica for implementada, os valores devem vir de fonte de dados explícita, não ser duplicados em handlers.

---

## 9. Chaos Cube / Cube de 25 estágios

**Fonte:** https://www.wyd2.co.kr/introduction/quest_event_11.asp

A entrada usa **Cube Invitation**, vendida por Arnold em Azran nas variantes:

| Convite | Preço citado | Faixa documentada |
|---|---:|---|
| Normal | 100.000 gold | Trans 1–200 |
| Mystic | 400.000 gold | Trans 1–300 |
| Arcane | 1.000.000 gold | Trans 1–400 |

O acesso ocorre na área de construção na entrada de Azran e consome/usa o convite segundo o fluxo da página.

### Mecânica

O Cube possui **25 estágios**.

Em cada estágio:

1. monstros aparecem;
2. todos precisam ser eliminados;
3. um quiz é mostrado aos participantes;
4. há aproximadamente **10 segundos** para selecionar o portal `O` ou `X` correspondente à resposta;
5. o resultado define a progressão.

No estágio 25, o boss **Orc Trooper** é o objetivo final. A página cita como recompensas possíveis um item consumível que concede EXP e drops aleatórios como White Lover Ring ou Sapphire.

### Observação importante para o WYD-Go

A página oficial confirma claramente o núcleo O/X e 25 estágios, mas não fornece em texto recuperado todas as perguntas, todos os spawns e toda a máquina de estados necessária para reconstrução completa. O servidor não deve inventar os detalhes ausentes.

---

## 10. Relações entre as quests

O site oficial permite montar a seguinte cadeia conceitual:

```text
progressão Mortal
↓
Orc Castle / Magical Pill e outras quests de progressão
↓
Eternal Stone
+ Sephirot
↓
Trans
↓
quests avançadas / Kefra
↓
Unknown Continent
↓
Seven Trials
↓
Runes
↓
Secret Stones
↓
Ideal Stone
↓
Reclass
```

Secret Room, Kingdom incursion e Cube são cadeias/eventos paralelos, mas reutilizam itens, reinos e recompensas que se cruzam com a progressão avançada.