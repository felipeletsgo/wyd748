# Guias de jogabilidade — síntese técnica em português

Fonte: páginas oficiais em `https://www.wyd2.co.kr/guide/`.

---

## 1. Controles, atalhos e interface

**Fonte:** https://www.wyd2.co.kr/guide/firstguide02_05.asp

A página reúne os atalhos e controles operacionais do client. Entre as funções documentadas estão seleção e registro de skills no quick slot, alteração do número de skills executadas automaticamente no modo de ataque automático (faixa de 2 a 10), uso de poções de HP/MP, Courage Elixir/Courage Potion, alimentação do mount, Recall Scroll e exibição do nome dos itens no chão.

A captura de tela é salva na pasta `ScreenShot` da instalação, no formato `capture0000.jpg`. Também há atalhos para janelas de personagem, inventário, skills, party, minimapa e quests.

O client oferece auto-run, mudança de velocidade do mount, PK mode, auto-attack, exibição de nomes e alternância do C.C entre modo mágico e físico. A câmera pode alternar entre quarter view e third-person/back view; zoom e deslocamento dependem de wheel/Alt conforme o mouse.

A página ainda referencia chat, data/hora/coordenadas, guild board, help, integração com site/amigos e emotes específicos por classe.

### Relevância para o emulador

- confirma que o client possui C.C nativo;
- confirma múltiplas skills configuráveis no auto-attack;
- confirma auto-run e controle de mount;
- reforça que coordenadas exibidas são informação de UI, não autoridade de movimento.

---

## 2. Party

**Fonte:** https://www.wyd2.co.kr/guide/firstguide07.asp

A party suporta até **13 membros**. Depois de formada, somente o líder pode convidar novos participantes. O convite é feito pela interação com o personagem (a página descreve `Ctrl + botão direito`) e a aceitação ocorre pela janela de party. O líder também pode remover membros.

A documentação oficial informa bônus de EXP conforme o número de membros elegíveis:

| Membros | EXP indicada pela página |
|---:|---:|
| 2 | 180% |
| 3 | 200% |
| 4 | 220% |
| 5 | 240% |
| 6 | 260% |
| 7 | 280% |
| 8 ou mais | 300% |

A distribuição interna não é descrita como divisão igual: personagens de nível maior recebem parcela maior segundo a página.

A Foema possui ações de summon/transfer de party em áreas permitidas, por clique ou comandos equivalentes a `/소환 personagem` e `/전이 personagem`.

A página menciona que summons do Beast Master podem aparecer associados à party.

### Pontos para o servidor

- limite 13 deve permanecer autoritativo;
- somente líder convida após formação;
- elegibilidade e distribuição de EXP precisam ser verificadas contra source antes de copiar percentuais literalmente para o código;
- summon/transfer deve validar região server-side.

---

## 3. Guild

**Fonte:** https://www.wyd2.co.kr/guide/firstguide08.asp

Ao contrário de party, a guild persiste após logout. A criação é realizada pela janela de guild e a página informa custo de **100.000.000 gold**.

A hierarquia inclui guild master e submaster. A nomeação/entrada de membro é associada a um custo de **4.000.000 gold** no guia. Existem restrições relacionadas a reino/cidadania e capas, que devem ser confirmadas na source antes de implementação literal.

O master pode registrar avisos, transferir a liderança e administrar membros. A transferência de guild master é descrita com custo de **100.000.000 gold**.

O emblema identifica visualmente a guild e sua borda varia conforme posição/hierarquia. Há regras para saída, expulsão e dissolução; o guia informa que uma guild detentora de zona de guild não pode simplesmente ser dissolvida pelo master enquanto essa condição existir.

Aliança é descrita como declaração unilateral. Para uma relação efetivamente mútua, ambas as guilds precisam declarar a outra.

Comandos citados incluem chat de guild/aliança, `guildtax [5-10]`, convocação, remoção/transferência de membro e encerramento de guerra/aliança.

---

## 4. Chat e comunicação

**Fonte:** https://www.wyd2.co.kr/guide/firstguide09.asp

Canais documentados:

| Sintaxe | Função |
|---|---|
| `Enter + texto` | chat local |
| `/personagem texto` | whisper |
| `/ㄷ texto` ou `/r texto` | responder último whisper |
| `= texto` | party |
| `- texto` | guild |
| `-- texto` | guild aliada |
| `/외 texto` | shout premium entre canais do mesmo grupo de servidor |
| `@@ texto` | cidadãos do canal/reino conforme o sistema do site |

A função de nickname usa `/tab ...` e é documentada para personagens de nível **70+**.

Existe ainda sistema de recado para personagens, com sintaxe envolvendo `/personagem ! mensagem`; a página alerta que essas mensagens desaparecem ao desconectar. O modo ausente usa `/snd mensagem` e responde automaticamente quando alguém envia whisper.

O item premium de shout permite que a mensagem apareça em todos os canais do mesmo grupo de servidor.

---

## 5. C.C — auto-combat / auto-farm

**Fonte:** https://www.wyd2.co.kr/guide/firstguide12.asp

O C.C é o sistema oficial de automação de combate do client.

### Modos

- **Physical:** ataca automaticamente monstros dentro do alcance e pode aplicar buffs configurados.
- **Magic:** utiliza as skills de ataque registradas/configuradas e buffs.
- **Not attack:** não ataca; mantém principalmente o uso de poções/configurações de sobrevivência.

A página descreve uso das skills selecionadas no shortcut/quick slot. A posição de caça pode ser mantida e há configuração relacionada a deslocamento no farm; o guia ressalta restrições específicas para o modo mágico.

O C.C também gerencia poções e alimentação do mount. O comportamento documentado inclui alimentar o mount automaticamente em condições de HP/comida baixos; a página cita valores de referência como HP abaixo de 6000 ou food <= 5 no comportamento automático descrito.

Os limiares de poção/alimentação podem ser configurados em incrementos de 10%.

O guia afirma que auto-attack não opera em PvP e que determinadas skills especiais, como Soul of Limits, não são executadas automaticamente.

### Implicações para testes de farm

O emulador deve suportar naturalmente o padrão de tráfego gerado por C.C:

```text
target morto
→ seleção rápida do próximo
→ ataque físico/skill
→ buff periódico
→ potion
→ alimentação de mount
→ possível retorno/manutenção de coordenada
```

Rejeitar pacotes legítimos dessa rotação por gates globais artificiais gera divergência direta do client oficial.

---

# Sistemas intermediários

## 6. Refino +0 a +9

**Fonte:** https://www.wyd2.co.kr/guide/midguide01.asp

A progressão de refino usa principalmente **Oriharukon Powder** e **Lactolerium Powder**.

A página descreve crescimento de atributos por nível de refino e ressalta queda da chance de sucesso conforme a progressão.

Oriharukon pode ser usado até +6; o guia o descreve como garantido nos primeiros níveis e progressivamente menos seguro depois. Lactolerium vai até +9 e possui faixa garantida maior antes de começar a falhar.

Em falhas nos níveis superiores, o item pode perder um nível de refino conforme a faixa.

### Bônus especiais em +9 descritos no guia

- chest/pants: defesa adicional;
- gloves: redução de delay de skill, com exceções para skills já muito rápidas;
- boots: velocidade de movimento;
- shield: defesa;
- spear/polearm/staff: bônus de ataque mágico;
- outras armas: bônus de dano;
- acessórios possuem tratamento próprio.

Essas descrições são referência de comportamento; fórmula e encoding devem continuar vindo da source e dos efeitos de `data/`.

---

## 7. Gemas e combinação de opções

**Fonte:** https://www.wyd2.co.kr/guide/midguide01_02.asp

Gemas documentadas:

| Gema | Efeito citado |
|---|---|
| Diamond | Drop +8% |
| Emerald | Absolute Damage +40 |
| Coral | EXP +2% |
| Garnet | Damage Absorption +40 |

A combinação exige itens/refinos compatíveis e, na página recuperada, custo de **50.000.000 gold**. Para Ancient, a família da gema precisa ser compatível com o item usado.

A página diferencia claramente sucesso e falha e indica consumo distinto de materiais. A operação é econômica e deve permanecer persist-before-publish.

---

## 8. Refino +12 a +15 com Odin

**Fonte:** https://www.wyd2.co.kr/guide/midguide01_05.asp

A combinação avançada utiliza:

- item de refino alto (+11 ou superior no fluxo descrito);
- dois pacotes de Lactolerium;
- Secret Stone;
- NPC Odin.

Em sucesso, o item sobe um nível. Em falha, existe possibilidade de redução de refino ou destruição conforme o mecanismo.

A página fornece multiplicadores de capacidade para os níveis avançados, culminando em +15. Também documenta o uso de **Refined Blessing** para melhorar proteção/chance e **Protection Pendant** no mecanismo de proteção/extração quando ocorre destruição.

O consumo dos materiais auxiliares ocorre conforme as regras da tentativa independentemente do resultado.

---

## 9. Mount — crescimento e ciclo de vida

**Fonte:** https://www.wyd2.co.kr/guide/midguide03_02.asp

O mount passa por ovo, jovem e forma adulta.

### Ovo

Cada mount possui um limiar de refino/incubação. Existe tempo para nova tentativa e o tempo de incubação só avança enquanto o ovo está equipado.

### Life

Ao nascer, a vida/longevity é sorteada na faixa **10–30**. Pode ser consumida quando o mount morre, fica sem comida e, na forma adulta, também quando o personagem montado morre.

### HP

Feed restaura HP. Mount com HP zero fica morto/inativo e precisa ser restaurado por Mount Master ou mecanismo equivalente documentado.

### Juvenil

Níveis **1–100**. O nível 100 torna o mount montável. O passo 99→100 depende do growth liquid correspondente.

O mount ganha Hunt EXP ao matar diretamente monstros de nível igual ou superior ao próprio. Cada kill elegível fornece 1 ponto segundo a página.

A página cita curvas de crescimento diferentes para Pig, Tamed Boar, Tamed Wolf, Dragon Puppy, Tamed Bear e Smilodon.

### Adulto

Pode evoluir até **120** com growth liquid. Quanto maior o nível, maior a chance de falha; a página cita chance de 20% de perder 1 de life quando o growth liquid falha.

Dano e magic attack crescem com o nível; ao chegar a 120 o guia informa **Damage Absorption +10%**.

---

## 10. Tintura de armadura

**Fonte:** https://www.wyd2.co.kr/guide/midguide05.asp

O sistema permite pintar armaduras e shields; armas/acessórios não participam como peças normais de tintura.

O fluxo combina Phial e Oriharukon Powder para gerar Enamel, aplicado então ao equipamento.

O item precisa possuir refino mínimo compatível (a página indica pelo menos +1). Reaplicar a mesma cor não deve alterar refino/efeitos. Remover Enamel restaura a cor original sem remover a progressão do equipamento.

---

## 11. Subpersonagem

**Fonte:** https://www.wyd2.co.kr/guide/midguide07.asp

A página descreve o sistema de Sub Character para personagens de evolução avançada. O material recuperado indica requisito de nível **120+** para o personagem avançado no fluxo de criação e permite criar um subpersonagem avançado de outra classe iniciando em nível 1.

A alternância Main/Sub utiliza **Mystery Stone**.

A versão recuperada desta página é parcial. Regras adicionais não são inferidas aqui.

---

# Sistemas avançados

## 12. Ancient

**Fonte:** https://www.wyd2.co.kr/guide/highguide01.asp

A combinação Ancient utiliza item +9 e gemas. A página recuperada trata especialmente de armas e das quatro gemas principais:

- Diamond: drop;
- Emerald: absolute damage;
- Coral: EXP;
- Garnet: absorption.

Itens auxiliares de refino podem aumentar a chance; o guia apresenta bônus conforme +7/+8/+9/+10 e exige compatibilidade de grade.

Falha destrói os materiais envolvidos no fluxo descrito; sucesso gera Ancient com refino e opção derivados da combinação.

---

## 13. Opções em armadura Trans

**Fonte:** https://www.wyd2.co.kr/guide/highguide02.asp

NPC: **Akada**, em Azran.

Fluxo documentado:

```text
+9 equipamento Trans
+ +9 equipamento normal compatível
+ 4 Glittering Stones
→ tentativa de transferência de opções
```

Os dois equipamentos devem respeitar compatibilidade de classe/parte conforme a página. Em sucesso, o resultado Trans fica em +7 e recebe as opções normais transferíveis.

O guia apresenta chances diferentes por tier/raridade e bônus de chance para itens +10/+11.

Opções especiais de +10/Ancient como drop, absolute damage, EXP e absorption **não são transferidas**; somente as opções normais previstas pelo sistema.

Recombinar é permitido, substituindo opções anteriores em caso de sucesso.

---

## 14. Opções em arma Trans

**Fonte:** https://www.wyd2.co.kr/guide/highguide02_02.asp

NPC: **Tiny**, em Karden Outpost.

A combinação descrita usa:

- Ancient +9 Trans weapon;
- Ancient +9 normal weapon de classe compatível e grade mínima;
- outro +9 de grade mínima;
- custo de **100.000.000 gold**.

Em sucesso, opções são transferidas e o resultado passa a +7. As probabilidades variam pelo tier e refino.

Opções especiais de Ancient/+10 não são transferidas como opções normais; o guia também define precedência para opções de gema existentes na arma.

Accessories/shields não entram no slot auxiliar indicado pela página.

---

## 15. Spirit Seal / selamento de personagem

**Fonte:** https://www.wyd2.co.kr/guide/highguide04.asp

Permite selar personagens Trans e Reclass em um item negociável e posteriormente restaurá-los, inclusive com novo nome.

O **Spirit Seal** é comprado do potion merchant Creta no Unknown Continent. Equipamentos usados pelo personagem devem ser movidos para o cargo antes do selamento; money e cape possuem tratamento próprio, enquanto guild mark é removido.

Dados preservados incluem classe, level, stats, skills, mastery, kill count, estado da quest Spirit Contract e cidadania. A superfície do item exibe apenas parte dessas informações.

A página informa reset de alguns estados após unseal, incluindo recursos/contadores específicos de gameplay.

### Restrições

- cargo precisa ter espaço;
- guild medal é removida;
- cape volta para branca mantendo refino;
- guild master necklace impede selamento;
- Seal pode ser trade/Auto Trade;
- não pode ser dropado nem vendido a NPC shop.

### Unseal

Requer personagem normal **281+** e slot de personagem livre. O body/base class governa a armadura compatível. Bonus stats são recalculados conforme o personagem normal de maior nível na nova conta. Reclass com subpersonagem também pode participar.

---

## 16. Equipamento Reclass

**Fonte:** https://www.wyd2.co.kr/guide/highguide05.asp

A combinação avançada usa:

- +9 sealed item;
- +15 Trans item da mesma parte;
- Dark Jewel para weapon **ou** Moonstone para armor;
- quatro Secret Stones: Water, Land, Sun e Wind, em ordem definida.

### Sucesso

Todos os inputs são consumidos e nasce um **+9 Reclass item**, preservando as opções do +15 Trans usado.

### Falha

O +15 Trans é preservado; os demais materiais da tentativa são consumidos segundo o guia.

Sealed item precisa estar +9, é negociável mas não equipável, e pode ser destruído em tentativas de refino anteriores.

---

# PvP e guerra

## 17. C.Point

**Fonte:** https://www.wyd2.co.kr/guide/pvpguide01.asp

O C.Point representa uma penalidade associada à morte PvP/PK.

Ao morrer em PvP, o personagem mantém/segura a EXP conforme o mecanismo e acumula C.Point relacionado à penalidade que teria sido aplicada. C.Point é reduzido através de progressão/combate posterior; quando chega a zero, o bloqueio de EXP associado termina.

Morte para mob não aumenta C.Point segundo a página. Há teto ligado à EXP necessária e penalidade adicional de HP quando o C.Point se aproxima do limite. A página também descreve interações com kills quando o alvo está no máximo e regras especiais na faixa 300–355.

As fórmulas exatas devem ser confirmadas na source antes de codificação.

---

## 18. Cerco — Thor, cannon, crown e mercenários

**Fonte:** https://www.wyd2.co.kr/guide/pvpguide04_03.asp

O guia descreve componentes do siege:

- **Thor tower:** ataca invasores e pode reagir a defensores que a atacam;
- **Cannon:** ligado à skill Sephira de conjuração, com faixa de uso documentada;
- **Attila Crown:** pertence ao guild master vencedor do castelo e possui stats próprios; transfere-se conforme a posse do castelo;
- **Mercenary contracts:** evocam NPCs por tempo limitado, vendidos por merchants do reino.

Esses elementos devem ser modelados server-side como entidades/objetos autoritativos, não como efeitos decididos pelo client.

---

## 19. Kingdom War

**Fonte:** https://www.wyd2.co.kr/guide/pvpguide07.asp

Guerra entre **Hekalotia** e **Acheronia**. O objetivo principal é matar o rei inimigo.

Os reinos possuem estruturas/NPCs militares e o ciclo dia/noite afeta produção/organização das forças conforme o guia. NPCs importantes influenciam defesas e Fame.

Jogadores de reinos opostos podem combater sem o PK mode tradicional durante o evento; o guia diferencia essa guerra do sistema normal de chaos/PK.

### Capacidade documentada

A página descreve seleção/capacidade por guild de maior Fame e cavaleiros adicionais, chegando a um total de **78 participantes** no desenho indicado.

### Horário

**Sábado, 20:00–20:50** na regra documentada pela página. O canal/servidor efetivo depende do ciclo de siege/propriedade descrito no site.

### Resultado

Matar o king inimigo define a vitória; sem kill do rei ocorre draw. O guia lista Sapphire, Guild Fame e valores econômicos ligados ao resultado.

---

## 20. Kingdom War — NPCs e recompensas

**Fonte:** https://www.wyd2.co.kr/guide/pvpguide07_02.asp

A página detalha os dois lados com reis, cannons, training camps, commanders, Ents, guardians, fame merchants, spawn areas e guardian towers.

O último hit no rei concede Fame relevante à guild; estruturas/NPCs secundários concedem quantidades menores conforme o tipo.

Itens da Fame/Honor shop citados incluem:

- King’s Call — retorno para a formação/reino;
- Commander Seal — bônus alto de stats;
- Berserker Knight Seal — Critical;
- Intellectual Mage Talisman — Magic Attack.

Seal/talisman usam slot de familiar e possuem regras próprias de proteção/refino descritas pela página.