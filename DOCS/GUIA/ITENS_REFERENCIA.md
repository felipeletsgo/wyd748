# Itens e referências — síntese técnica em português

Fonte: páginas oficiais em `https://www.wyd2.co.kr/introduction/`.

---

## 1. Espadas de uma mão

**Fonte:** https://www.wyd2.co.kr/introduction/item_weapon01.asp

O catálogo oficial lista armas de uma mão em variantes Normal e Ancient, com requisitos, dano e grade. A família inclui armas como Dagger, Short Sword, Stiletto, Rapier, Long Sword e Scimitar.

A função básica indicada pelo guia é permitir o uso em uma mão, deixando a outra disponível para outra arma ou shield.

### Relevância

Os valores individuais de item devem continuar vindo de `data/itemlist.csv`; esta página serve para validação cruzada de nomes, classes de arma, requisito e expectativa visual do jogador.

---

## 2. Staffs e dual wield

**Fonte:** https://www.wyd2.co.kr/introduction/item_weapon04.asp

O catálogo inclui staffs de uma e duas mãos e descreve o papel das armas mágicas.

A informação de maior interesse para o cálculo de combate é a regra oficial de **dual wield físico** registrada nesta família de referência:

- a arma com maior dano contribui integralmente;
- a arma de menor dano contribui parcialmente;
- quando ambas pertencem à mesma família/tipo compatível, a contribuição secundária aumenta;
- opções que não são o dano principal continuam podendo ser consideradas de ambas conforme as regras do sistema.

A síntese histórica do site corresponde à ideia de 100% da arma principal e 30% da secundária, subindo para 50% em combinação equivalente.

### Relevância para o código

Essa página é útil para validar `equipmentDamage` e os modificadores de passivas. Os números finais devem ser reconciliados com `BASE_GetMobAbility`/source, especialmente quando uma classe possui passive que altera o aproveitamento da segunda arma.

---

## 3. Ultimate Weapons

**Fonte:** https://www.wyd2.co.kr/introduction/item_weapon08.asp

A página cataloga a primeira família de **Ultimate Weapons**, descritas como armas originalmente usadas por monstros e entregues a jogadores por eventos.

O guia destaca ausência de requisito de level em parte dessa família, exigindo apenas os stats necessários, e dano superior ao de armas normais equivalentes.

Exemplos exibidos incluem Cutlass e Doom Axe, além de outras armas da categoria.

### Relevância

A ausência de requisito de level não deve ser implementada por exceção hardcoded de nome. O servidor deve obter requisitos e efeitos exclusivamente do conteúdo carregado de `itemlist.csv`.

---

## 4. Equipamentos Trans

**Fonte:** https://www.wyd2.co.kr/introduction/item_js01.asp

Equipamentos exclusivos de personagem Trans usam, segundo a página, a mesma lógica geral de opções/aplicação dos itens normais.

O ponto mais importante é a regra de **body/base class** para armadura:

> a armadura compatível é determinada pela classe do corpo/personagem Mortal de origem, e não necessariamente pela classe atual de skills do Trans.

O exemplo do site: personagem Mortal Trans Knight que se tornou Trans Hunter continua usando armor correspondente ao corpo Trans Knight.

A página lista sets como Initiate Armor e seus efeitos, incluindo HP, regen, critical, resist e move speed.

### Relevância para WYD-Go

Confirma a separação arquitetural já necessária entre:

```text
classe que governa skills
vs
classe corporal que governa armor
```

Isso deve continuar server-side em `canEquip`.

---

## 5. Consumíveis e itens utilitários

**Fonte:** https://www.wyd2.co.kr/introduction/item_consumer.asp

A página é um catálogo amplo de consumíveis e materiais.

### HP e MP

Inclui herbs, Healing Potions, Mana Potions e caixas em diferentes quantidades.

### Potions de combate

São descritas três etapas/tipos de potion temporária que aumentam ataque físico/mágico com duração e intensidade progressivas.

### Courage

- **Courage Elixir:** aumenta dano contra monstros;
- **Courage Potion:** versão mais forte, também voltada a PvE.

O guia explicita que o bônus é aplicado ao combate contra monstros, não como bônus PvP genérico.

### Blood Jewel

Efeito de life drain, comparado pelo site ao efeito Live Drain.

### Shield Jewel

Aumenta Max HP em relação a defesa/level conforme a mecânica correspondente.

### Angels

A página descreve Silver Angel e Gold Angel como versões derivadas do Red Angel, com bônus de drop/EXP e função de conversão rápida de itens do inventário em gold por interação específica do client.

O Gold Angel é descrito com bônus superiores ao Silver/Red no material atual.

### Silver Wydon

Itens que representam grandes valores de gold, como 100 milhões/1 bilhão, para contornar limites de saldo em inventário/cargo do sistema legado. Usar o item reconverte o valor em gold.

### EXP Box

Ao usar, concede por **2 horas** o dobro de EXP do last hit segundo a página. Usos adicionais acumulam duração.

### Revival Horse

Revive mount morto sem necessidade de visitar presencialmente o Mount Master, reproduzindo o efeito do NPC.

### Warrior Seal Book

Item associado ao aumento de Fame e outras condições de progressão/cape documentadas.

### Spirit Seal

Item que encapsula um personagem avançado para trade; detalhes completos estão em `GUIAS_JOGABILIDADE.md`.

### Nightmare Book

A página informa liberação de **24 entradas** de Nightmare para Reclass e cooldown de **20 horas** desde o primeiro uso para reutilização.

---

## 6. Materiais de refino

**Fonte:** https://www.wyd2.co.kr/introduction/item_consumer.asp

### Oriharukon Scrap / Powder

- 10 scraps podem ser convertidos em powder por NPC alquimista;
- a página cita custo de **1.000.000 gold**;
- powder é usado no refino até +6;
- primeiros níveis possuem sucesso garantido conforme a documentação histórica.

### Lactolerium Scrap / Powder

- usado para refino de nível mais alto, até +9;
- também aparece em combinações avançadas, inclusive transições superiores;
- a página registra faixa garantida maior que Oriharukon antes de a chance cair.

### Legendary stones

O catálogo referencia raw stones como Spinel, Beryl, Tectite e Adamantite para transformar famílias específicas de armor em Legendary, com possibilidade de falha.

### Regra de arquitetura

Essas relações item→efeito→uso devem vir dos loaders/configuração sempre que representarem conteúdo. Handlers não devem duplicar listas de IDs ou valores que já existem em `data/`.

---

## 7. Gemas

As páginas de refino/Ancient documentam quatro gemas centrais:

| Gema | Efeito de referência do site |
|---|---|
| Diamond | Drop +8% |
| Emerald | Absolute Damage +40 |
| Coral | EXP +2% |
| Garnet | Damage Absorption +40 |

Estes valores são úteis como validação do conteúdo carregado, mas não devem virar uma segunda tabela hardcoded se já estiverem representados em `itemlist.csv`/efeitos.

---

## 8. Relação com o servidor

Para cada item do site oficial, a ordem de autoridade dentro do WYD-Go continua:

```text
itemlist.csv
+ Itemname.csv apenas para nome
+ ItemEffect.h para ID ↔ EF_*
+ efeitos persistidos da instância
→ cálculo server-side
```

O catálogo oficial deve ser usado para detectar divergências de conteúdo, não para criar correções pós-load.