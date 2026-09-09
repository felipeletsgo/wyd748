# Guia oficial WYD2 — índice em português

Fonte primária: https://www.wyd2.co.kr/

Mapeamento realizado em 12/08/2026 sobre o site oficial coreano do WYD/With Your Destiny.

## Escopo e regra editorial

Esta pasta organiza, em português, o conteúdo técnico e de gameplay encontrado nas páginas oficiais do site. Por copyright, os textos coreanos não são reproduzidos nem traduzidos integralmente; cada entrada é uma **síntese técnica detalhada e fiel**, mantendo requisitos, números, regras, itens, NPCs e relações entre sistemas sempre que a página os expõe.

O site é legado, usa páginas ASP e sua indexação pública é incompleta. Por isso cada URL recebe um status:

- **Confirmada/documentada** — a página foi localizada e seu conteúdo pôde ser recuperado.
- **Confirmada/parcial** — a URL existe ou aparece na navegação/indexação, mas nem todo o conteúdo ficou recuperável.
- **Não confirmada** — apenas um padrão de numeração sugeriria a página; não é tratada como fonte até ser realmente encontrada.

Não se preenche lacuna do site com conhecimento geral do jogo. Quando uma regra não aparece na fonte recuperada, ela é marcada como não determinada.

## Documentos

- [MAPA_DE_URLS.md](MAPA_DE_URLS.md) — inventário de URLs e estado da coleta.
- [GUIAS_JOGABILIDADE.md](GUIAS_JOGABILIDADE.md) — primeiros passos, sistemas intermediários, avançados e PvP.
- [QUESTS_EVENTOS.md](QUESTS_EVENTOS.md) — quests, evolução, Cube, Secret Room, Seven Trials e sistemas relacionados.
- [ITENS_REFERENCIA.md](ITENS_REFERENCIA.md) — itens, armas, equipamentos Trans e consumíveis.

## Famílias oficiais identificadas

### `/guide/firstguide*`

Guias de uso básico do client e sistemas fundamentais: atalhos, party, guild, comunicação e C.C/auto-farm.

### `/guide/midguide*`

Sistemas intermediários: refino, combinação, mount, tintura e subpersonagem.

### `/guide/highguide*`

Sistemas avançados: Ancient, opções de equipamentos Trans, Spirit Seal e combinação de equipamentos Reclass.

### `/guide/pvpguide*`

PvP, C.Point, cerco e Kingdom War.

### `/guide/tip*`

Guias práticos de progressão de personagem.

### `/introduction/quest*`

Documentação de quests e eventos. Esta família contém mecânicas relevantes ao servidor, não apenas texto introdutório.

### `/introduction/item*`

Catálogos e regras de itens, armas, equipamentos Trans e consumíveis.

### `/premium/*`

Guias de Cash/Premium. São periféricos ao core do emulador e foram catalogados separadamente no mapa de URLs quando localizados.

## Uso para o WYD-Go

Estas páginas devem ser tratadas como **referência de comportamento observado/documentado**, não como substituto das fontes de implementação. Para mecânica nativa, a prioridade do projeto continua:

1. implementação atual;
2. W2PP;
3. Secrets 7.54;
4. Micronics/client 7.48.

O site oficial é especialmente útil para:

- requisitos e custos exibidos ao jogador;
- regras de acesso a quests/eventos;
- recompensas e limites;
- fluxo de interação com NPCs;
- comportamento esperado do C.C/auto-farm;
- composição de party/guild;
- regras de combinação/refino;
- horários e capacidade de eventos;
- descrições de itens e efeitos.

Quando o site divergir do código ou de uma source C++, a divergência deve ser registrada e investigada; não deve ser corrigida silenciosamente.