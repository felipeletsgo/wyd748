# WYD-Go 7.48 — arquitetura e roadmap

Documento de direção do emulador em `wyd-go/`. O estado comprovado em código e
testes está em `wyd-go/DOCS/IMPLEMENTATION_STATUS.md`; detalhes byte a byte ficam
em `wyd-go/DOCS/EMULATOR.md`.

## Objetivo arquitetural

O servidor é modular e server-authoritative. Alterar uma fórmula, uma skill ou
um sistema não deve exigir ler milhares de linhas nem confiar em cálculo do
client. As fronteiras atuais são:

```text
cmd/server          composição e configuração
internal/model      domínio puro
internal/wire       protocolo 7.48 byte a byte
internal/net        sockets, sessão, filas
internal/data       catálogos, NPCGener, terreno, templates
internal/store      persistência de contas
internal/account    cadastro, autenticação e hash
internal/game       ator World e sistemas de jogo fragmentados
```

Uma única goroutine de `World` serializa mutações. Isso fornece ordem natural
para inventário, loot, trade, party e combate e reduz a superfície de dupe. I/O
de socket e leitura de login ficam fora do caminho crítico; mensagens retornam
ao ator por comandos.

JSON continua adequado para desenvolvimento e servidor pequeno porque é legível
e já possui escrita atômica. Lua poderá ser incorporado depois para regras que
precisem hot reload (quests, eventos, NPCs e tabelas de balanceamento), mas não
deve substituir o estado persistente nem os invariantes Go. Banco SQL será uma
nova implementação de `store.Store`, sem reescrever `game`.

## Estado funcional consolidado

- conta via CLI/API, PBKDF2, JSON estrito e bloqueio de login duplo;
- criação de até quatro personagens, nickname ASCII único de 4–13 letras e
  templates das quatro classes;
- login/enter-world completo, autosave de 3 s e JSON atômico;
- área de interesse ±32 para players, mobs, NPCs e itens do chão;
- movimento interpolado, velocidade pública, colisão e terreno nativo;
- NPCGener persistente no mundo, patrulha, respawn e população por gerador;
- stats/equipamentos server-side, distribuição de pontos, EXP e level-up;
- combate player↔mob, mob↔player e PvP de skills, morte, recall e reentrada;
- party de até 13 membros, ordem estável e EXP integral com +2% por membro;
- inventário visível de 63 slots, drop/loot, loja, consumíveis e volatiles
  (45 de 116 códigos `EF_VOLATILE` com comportamento real — restauro, gold,
  teleporte, buff/comida, EXP, salvar/voltar posição, transformação de rosto,
  refino Ori/Lac/Molar, tintura/repliction e o sistema completo de montaria;
  cobertura e pendências em `DOCS/IMPLEMENTATION_STATUS.md` §Consumíveis);
- catálogo server-side de itens e skills;
- compra/aprendizado, atalhos, buffs, debuffs, passivas, summons e execução das
  96 skills básicas com caminhos separados por classe;
- efeitos públicos por CreateMob/0x336 e ícones privados por 0x3B9.

## Invariantes de protocolo

1. Todo pacote é construído manualmente em `internal/wire`; structs Go nunca são
   despejadas diretamente no socket.
2. `CreateMob` leva Score@100 e Affect[16]@66.
3. `UpdateScore 0x336` leva 16 affects compactos @42..73 e é público.
4. `UpdateAffect 0x3B9` leva 16 affects completos, usa tempo em blocos de 8 s e
   é privado ao dono para ícones/timers.
5. Inventário mantém 64 estruturas no wire, porém o slot 63 é reservado.
6. Respostas de skill usam 0x39D (1), 0x39E (2) ou 0x36C (até 13 alvos).
7. O client envia intenção; preço, dano, efeito, cooldown, alcance, alvo,
   propriedade e posição são revalidados no servidor.

## Próximas fatias de features

### 1. Fechar e estabilizar as 96 skills

- montar uma matriz de teste por conta/classe e skill, registrando: conjuração,
  alvo, visual, dano/cura, affect, duração, cooldown, MP e persistência;
- comparar cada divergência com W2PP/7.59 e portar apenas a fórmula;
- completar efeitos visuais específicos ainda sem equivalência confirmada;
- adicionar testes determinísticos por família de skill e expiração de affect.

Critério de pronto: cada índice 0..95 possui comportamento funcional confirmado
no client, teste server-side e nenhuma corrupção de ExtendedScore/Chaos/pontos.

### 2. Trade seguro entre jogadores

- sessão de trade com estados invite/open/locked/confirmed/cancelled;
- reservar itens e gold enquanto a janela estiver aberta;
- validar novamente ambos os lados no commit único;
- cancelar em distância, morte, desconexão ou mudança de inventário;
- persistir as duas contas antes de confirmar visualmente.

Critério de pronto: testes de corrida/desconexão não duplicam nem perdem itens.

### 3. Quest, crafting e progressão avançada

- **volatiles restantes (Fase D)**: summon scrolls (41-48, `Hired_Soldier_Contract`/
  `Guard_Summon_Letter`, reusar `castSummon`) e livros de skill Sephira (32-36,
  via `SecondaryLearnedSkill`); + o buff faltante 58 e verificar 6/195. O refino,
  a tintura, a repliction e a montaria já estão entregues (Fases A/B/C);
- extração e composição/alquimia usando efeitos do catálogo (crafting ainda pendente);
- quests e NPCs de missão com pré-condições server-side;
- evolução Arch/Celestial, limites e tabelas de experiência correspondentes;
- guild, guerra e sistemas sociais ainda ausentes.

As regras configuráveis são candidatas a Lua com carregamento versionado e troca
atômica. O runtime Go continua dono de inventário, transações e validações.

### 4. Escala e operação

- substituir buscas lineares restantes por grid espacial quando a população real
  do NPCGener completo justificar;
- fila assíncrona/coalescida de persistência sem violar a ordem do ator;
- métricas de tick, fila, latência de save e sessões;
- implementação SQL de `store.Store`, migração e backup;
- separação futura em canais/mapas somente depois de medir o limite de um World.

## Ordem recomendada imediata

1. Executar a matriz manual das quatro classes sobre a revisão de affects atual.
2. Corrigir as divergências de skills encontradas, uma família por vez.
3. Criar o trade com modelo transacional e testes de anti-dupe.
4. Só então iniciar crafting/quests; esses sistemas dependem de inventário e
   transações já estáveis.

## Fontes de referência

- `SERVER W2PP/SOURCE GAME`: verdade sobre leitura e apresentação no client;
- `SERVER W2PP/SOURCE SERVER` e `source 7.59`: algoritmos e fórmulas completas;
- `SOURCE(secrets) para referência/Descompilação`: comportamento próximo do 7.54;
- dumps reais do Micronics: confirmação final dos offsets 7.48.

Regra permanente: portar algoritmos, nunca layouts de struct, offsets de memória
ou endereços de outra versão.
