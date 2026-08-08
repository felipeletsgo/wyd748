# Plano de correção completa das instâncias

**Projeto:** WYD-Go / client 7.48
**Data:** 2026-08-01
**Status:** infraestrutura e fases 1–6 aplicadas; núcleo O/X do Big Cube habilitado e configurável
**Escopo:** Water, Magic Chamber, Cube, Nightmare, Hell Gate e Big Cube

## Objetivo

Corrigir as instâncias para que reproduzam o comportamento nativo confirmado
nas fontes W2PP, Secrets 7.54, Micronics e client 7.48, preservando as
invariantes server-side do WYD-Go:

- `World` continua sendo o único escritor de gameplay;
- o cliente envia somente intenções e nunca determina transições, posições,
  consumo, recompensas ou dano;
- toda operação econômica segue `validar -> alterar cópia -> persistir ->
  publicar`;
- falhas restauram item, posição, contadores e estado da instância;
- visibilidade, IA e persistência participam da mesma implementação;
- arquivos em `data/` permanecem autoritativos.

## Problemas confirmados

1. Nightmare e Hell Gate foram representados como instâncias exclusivas, mas
   nativamente são zonas compartilhadas controladas por horário.
2. O limite de três parties do Nightmare não pode ser atingido porque a
   segunda execução é recusada pelo mesmo ID de instância.
3. Nightmare termina quando todos os mobs morrem, enquanto o original regenera
   os grupos e mantém a zona até o encerramento da janela.
4. Hell Gate perdeu Tarântula, portais, quatro quadrantes, combinação aleatória
   dos Lichs e corredor final.
5. Cube Mystic e Arcane usam atualmente o layout, mobs e coordenadas do Cube
   Normal.
6. Magic Chamber possui uma quinta sala artificial para o chefe e renova o
   prazo, embora o chefe deva nascer na quarta sala usando o tempo restante.
7. O deadline antigo da sala pode cancelar uma transição ou pergunta que já
   começou.
8. A exclusividade global da Magic Chamber depende incidentalmente da presença
   física dos jogadores, em vez de um lock explícito comum a N/M/A.
9. A configuração atual do Big Cube usa grupos de NPCGener pertencentes a
   outras regiões e ainda não possui comprovação nativa suficiente.
10. Parte dos testes consolida o comportamento incorreto e, por isso, a suíte
    verde não representa fidelidade ao jogo.

## Arquitetura de destino

O sistema será dividido entre infraestrutura comum e controladores específicos.

```text
internal/game/
├── item_instance_core.go
├── item_instance_private.go
├── timed_zone.go
├── water_instance.go
├── magic_chamber.go
├── cube_instance.go
├── nightmare.go
├── hell_gate.go
└── big_cube.go
```

Os testes seguirão a mesma separação:

```text
internal/game/
├── item_instance_core_test.go
├── water_instance_test.go
├── magic_chamber_test.go
├── cube_instance_test.go
├── nightmare_test.go
├── hell_gate_test.go
└── big_cube_test.go
```

Os dados serão retirados do arquivo monolítico e separados por mecânica:

```text
data/instances/
├── water.json
├── magic_chamber.json
├── cube.json
├── nightmare.json
├── hell_gate.json
└── big_cube.json
```

`data/volatiles.json` continuará associando item, volatile, ação e
`instanceRef`, sem carregar toda a máquina de estados dentro da regra do item.

## Modelos de execução

### `private_chain`

Usado por Water e Magic Chamber. Cada execução possui membros, mobs e
progressão próprios e não compartilha combate com outras parties.

### `private_shared_entry`

Usado pelo Cube. A execução é privada, mas cada participante entra e consome o
próprio convite enquanto a primeira sala ainda aceita inscrições.

### `shared_timed_zone`

Usado por Nightmare. Diferentes parties e jogadores individuais entram na
mesma zona, enxergam os mesmos mobs e permanecem até o fim da janela.

### `state_machine`

Usado por Hell Gate e Big Cube, cujas transições dependem de chefes, portais,
respostas, combinações e objetos de mundo, e não apenas da contagem de mobs.

## Estado e deadlines

Substituir o deadline único por prazos independentes:

- `CombatDeadline` — prazo para concluir o combate atual;
- `TransitionDeadline` — espera entre salas;
- `QuizDeadline` — prazo exclusivo da pergunta;
- `HardDeadline` — limite absoluto da execução;
- `ScheduleEnd` — encerramento da janela horária;
- `ExitDeadline` — janela entre conclusão e recolhimento.

Adicionar políticas explícitas:

- `finish_on_clear`;
- `finish_on_timeout`;
- `advance_on_clear`;
- `respawn_until_timeout`;
- `exclusive_group`;
- `allow_chain_during_exit_grace`.

Quando o combate for concluído, `CombatDeadline` deixa de governar a execução.
Uma transição ou pergunta iniciada recebe seu próprio prazo e não pode ser
cancelada pelo deadline da fase anterior.

## Fase 1 — Infraestrutura comum

### Implementação

1. Criar os modelos de execução e fase.
2. Extrair admissão, snapshots, consumo, persistência e teleporte do arquivo
   atual.
3. Implementar locks por `exclusive_group`, separados do ID da configuração.
4. Tornar a janela de cadeia Water explícita nos dados.
5. Separar participantes de zonas compartilhadas dos membros de instâncias
   privadas.
6. Adicionar RNG e relógio injetáveis para testes determinísticos.
7. Manter mobs preparados sem publicação até o commit de entrada.
8. Centralizar cleanup de mobs, jogadores, timers e objetos temporários.

### Critérios de conclusão

- nenhum item é consumido se o conteúdo inicial não puder ser criado;
- falha do PostgreSQL restaura todas as contas envolvidas;
- nenhum mob é visível antes da persistência;
- uma fase não avança depois do seu timeout;
- uma transição válida não é cancelada pelo prazo da fase anterior;
- não existe código paralelo antigo executando a mesma mecânica.

## Fase 2 — Water

### Regras

- manter as 24 salas regulares e as três salas Nessus;
- manter 120 segundos por sala;
- somente o líder recebe o pergaminho seguinte;
- sala 8 entrega a carta de Nessus;
- concluir Nessus encerra a cadeia sem gerar item zero;
- inventário cheio derruba a recompensa no chão;
- permitir o pergaminho seguinte durante os dez segundos de saída;
- cleanup anterior nunca retira o jogador da sala seguinte;
- manter restrições próprias de Normal, Mystic e Arcane;
- manter exclusividade física por sala.

### Testes

- cadeia completa de cada dificuldade;
- recompensa no inventário e no chão;
- falha de persistência da recompensa;
- próximo pergaminho durante `ExitDeadline`;
- morte, recall, logout e desconexão;
- duas parties tentando ocupar a mesma sala;
- ausência de teleporte causado pelo encerramento de uma sala anterior.

## Fase 3 — Magic Chamber

### Regras

- criar `exclusive_group = "magic-chamber"` para N/M/A;
- permitir somente uma execução global entre as três dificuldades;
- manter quatro salas de 100 mobs;
- usar 180 segundos por sala;
- conceder dez segundos completos entre salas;
- após os 100 mobs da quarta sala, gerar o chefe na mesma sala;
- o chefe usa o tempo restante da quarta sala;
- matar o chefe conclui diretamente a execução;
- timeout recolhe todos e libera o lock global.

### Testes

- Normal, Mystic e Arcane individualmente;
- uma dificuldade bloqueando as outras;
- chefe sem renovação do prazo;
- transição concluída no último segundo válido;
- falha no spawn do chefe;
- liberação do lock por sucesso, falha, recall e desconexão;
- party transportada somente pela ação do líder.

## Fase 4 — Cube Normal, Mystic e Arcane

### Pesquisa e dados

Extrair das tabelas nativas:

- 25 salas de cada dificuldade;
- coordenadas de entrada e das salas;
- templates e quantidades dos mobs;
- coordenadas O/X de `CuboCoordN`, `CuboCoordM` e `CuboCoordA`;
- perguntas, respostas e EXP;
- condição e coordenada final.

Criar configurações independentes:

- `cube-normal`;
- `cube-mystic`;
- `cube-arcane`.

### Regras

- máximo de seis participantes por execução;
- cada participante consome seu próprio convite;
- entrada permitida somente antes de a primeira sala avançar;
- convite nunca transporta automaticamente a party;
- remover o prazo artificial de 180 segundos para matar os mobs, salvo se uma
  fonte 7.48 confirmar esse prazo;
- ao concluir o combate, iniciar 14 segundos exclusivos para a pergunta;
- plataforma válida exatamente em `[X-3..X] x [Y-3..Y]`;
- jogador incorreto é retirado;
- jogador correto recebe EXP e avança;
- dificuldades não compartilham mobs, coordenadas nem ocupação sem confirmação
  nativa.

### Testes

- layouts distintos para N/M/A;
- seis entradas individuais com seis itens consumidos;
- sétima entrada recusada;
- entrada tardia recusada;
- acerto, erro e ausência de resposta;
- pergunta iniciada perto de qualquer outro deadline;
- EXP e level-up transacionais;
- conclusão e eliminação do último participante.

## Fase 5 — Nightmare

### Modelo

Converter cada dificuldade em uma zona compartilhada por janela:

- Normal: minutos 00, 20 e 40;
- Mystic: minutos 05, 25 e 45;
- Arcane: minutos 10, 30 e 50;
- duração de 240 segundos.

### Admissão

- até três entradas de party por tier e janela;
- ticket pessoal não usa o contador de party;
- várias parties e jogadores individuais coexistem;
- todos veem os mesmos jogadores e mobs;
- aplicar requisitos nativos de evolução e level;
- cobrar NT de Celestiais elegíveis no Arcane;
- persistir ticket, NT, posição e contador antes da publicação.

### Gameplay

- matar todos os mobs não conclui a zona;
- grupos são regenerados conforme o NPCGener nativo;
- a zona permanece ativa até `ScheduleEnd`;
- o final da janela remove mobs e recolhe os participantes;
- o contador de party muda somente com a chave da próxima janela.

### Persistência

Persistir no PostgreSQL:

- tier;
- chave da janela;
- número de entradas de party;
- estado ativo e horário final.

Isso impede reinício do servidor para contornar o limite de três parties.

### Testes

- três parties coexistindo e quarta recusada;
- jogadores pessoais coexistindo com as parties;
- mobs compartilhados entre todos;
- regeneração após mortes;
- cobrança e rollback de NT;
- virada da hora;
- reinício dentro da mesma janela;
- cleanup completo no encerramento.

## Fase 6 — Hell Gate

Hell Gate será uma zona compartilhada com máquina de estados global.

### Estado `entry`

- tickets pessoal e party entram na mesma execução;
- janela total de 240 segundos;
- mobs iniciais regeneram durante a fase;
- portais permanecem fechados;
- gerar Tarântulas e a Tarântula controladora.

### Estado `tarantula_defeated`

Ao matar a Tarântula controladora:

- marcar `TarantulaKilled`;
- atualizar visual e colisão dos portais;
- gerar simultaneamente os quatro quadrantes;
- sortear a combinação correta dos Lichs;
- publicar a abertura para todos os participantes.

### Estado `lich_puzzle`

- cada quadrante possui seus mobs e Lich;
- mortes dos Lichs atualizam o estado autoritativo;
- somente a combinação correta abre o caminho final;
- RNG deve ser injetável e persistido durante a janela;
- clientes nunca determinam qual Lich é válido.

### Estado `final`

- abrir o corredor final;
- gerar os grupos finais;
- criar o Aki na posição nativa;
- enviar mensagens nativas de conclusão;
- manter ou encerrar a zona conforme confirmação final na fonte 7.48.

### Reset

No encerramento da janela:

- fechar todos os portais;
- remover mobs e NPCs temporários;
- recolher participantes;
- limpar Tarântula, Lichs, combinação e flags;
- persistir o estado limpo da próxima janela.

### Testes

- entradas pessoal e party na mesma zona;
- respawn dos mobs iniciais;
- Tarântula liberando portais;
- as duas combinações de Lich;
- combinação errada sem liberar o final;
- criação do Aki;
- timeout em cada estado;
- reinício com estado parcialmente concluído;
- rollback de ticket e posição.

## Fase 7 — Big Cube

O núcleo atual não cria mobs nem reutiliza grupos de NPCGener: ele abre a
pergunta O/X, valida a plataforma e recolhe os participantes. A cadeia
completa de rodadas, portões e recompensas continua dependente da tabela nativa
que ainda falta confirmar.

### Pesquisa obrigatória

Consultar, nesta ordem:

1. client 7.48;
2. NPCGener Micronics;
3. InitItem e objetos de portão;
4. W2PP: `BigCubo`, `SetBigCuboDoor`, áreas O/X e quiz;
5. Secrets/decompilação;
6. `wyd-dev-knowledge` e WebCheats apenas para lacunas restantes.

### Artefato de pesquisa

Antes do código, produzir uma tabela source-confirmed contendo:

- item e requisitos de entrada;
- coordenadas;
- portões e estados;
- áreas O/X;
- perguntas e respostas;
- ciclo de rodadas;
- regras de eliminação;
- recompensas;
- conclusão e reset.

### Regra de implementação

Nenhuma população, coordenada ou recompensa será mantida apenas por semelhança
com outro evento. A implementação começa somente depois da tabela confirmada.

## Fase 8 — Visibilidade e IA

### Instâncias privadas

- mobs visíveis somente aos membros inscritos;
- IA seleciona somente membros vivos da execução;
- mobs não atravessam o limite da sala;
- `CreateMob`, movimento, dano, affects, morte e remoção usam a mesma fronteira.

### Zonas compartilhadas

- todos os participantes veem os mesmos mobs e jogadores;
- parties não recebem cópias próprias de Nightmare ou Hell Gate;
- posição física forjada não concede participação;
- somente admissão autoritativa libera interação;
- `REQMobByID` respeita a mesma política de visibilidade.

## Fase 9 — Transações e anti-dupe

Toda admissão seguirá a ordem:

1. validar item, UID, slot, posição, horário e requisitos;
2. validar participantes e limites;
3. preparar conteúdo sem publicação;
4. criar snapshots das contas e do evento;
5. consumir ticket, NT ou contador na cópia autoritativa;
6. atualizar posições;
7. persistir todas as contas e o estado global em uma transação;
8. registrar os participantes em memória;
9. publicar teleporte, mobs, objetos e interface;
10. restaurar tudo se a persistência falhar.

Falhas obrigatoriamente testadas:

- spawn parcial;
- falta de IDs;
- terreno ou entrada bloqueada;
- PostgreSQL indisponível;
- mais de um participante da mesma conta;
- desconexão durante transição;
- reinício durante uma janela ativa.

## Fase 10 — Limpeza e documentação

Depois que todos os controladores forem migrados:

1. remover campos genéricos sem consumidores;
2. remover fluxo antigo de `item_instances.go`;
3. remover testes que validam comportamento incorreto;
4. remover configurações provisórias do Big Cube;
5. conferir se não existem dois caminhos para a mesma volatile;
6. atualizar `README.md`, `DOCS/IMPLEMENTATION_STATUS.md` e este documento;
7. registrar como concluído somente o que foi validado no client.

## Matriz de validação

Cada mecânica precisa cobrir:

- entrada válida e inválida;
- item empilhado;
- UID e slot incorretos;
- ticket duplicado;
- horário inválido;
- party incompleta;
- limite de participantes;
- falha de persistência;
- logout e relogin;
- morte e recall;
- transição no último segundo;
- timeout;
- visibilidade externa;
- IA atacando apenas participantes;
- encerramento e próxima execução.

Comandos finais:

```powershell
go test -count=1 ./internal/game ./internal/data
go test ./...
go vet ./...
go build -o tmp/tm-check.exe ./cmd/server
git diff --check
```

Enquanto o linker MinGW local não disponibilizar `-lsynchronization`, o Race
Detector deverá ser executado no Ubuntu ou em CI:

```bash
go test -race ./internal/game
```

## Ordem de entrega

1. Infraestrutura comum e regressão da Water.
2. Magic Chamber.
3. Cube Normal, Mystic e Arcane.
4. Nightmare compartilhado.
5. Hell Gate completo.
6. Pesquisa e implementação do Big Cube.
7. Auditoria de visibilidade, IA, persistência e anti-exploit.
8. Testes manuais com múltiplos clients.
9. Atualização da documentação.
10. Commit e push após validação in-game.

## Critério final de aceite

Uma instância será considerada concluída somente quando:

- reproduzir a regra nativa confirmada;
- consumir e recompensar de forma transacional;
- resistir a pacotes forjados e tentativas de duplicação;
- sincronizar participantes e observadores corretamente;
- sobreviver a falha de persistência sem estado dividido;
- encerrar e limpar todos os recursos;
- passar pelos testes automatizados;
- funcionar no client 7.48 em teste multiplayer.

## Registro da rodada de implementação (2026-08-01)

Aplicado nesta rodada:

- Water: áreas de entrada autoritativas por conjunto, janela explícita de dez
  segundos entre a recompensa e o próximo pergaminho, cleanup sem expulsão
  cruzada e rollback do primeiro spawn;
- Magic Chamber: três dificuldades sob `exclusiveGroup` comum, quatro salas
  de 100 mobs e chefe materializado na quarta sala sem renovar o prazo;
- Cube: variantes Normal/Mystic/Arcane resolvidas como configurações
  independentes, entrada individual até seis participantes e plataforma O/X
  4×4 (`[X-3..X] × [Y-3..Y]`);
- Nightmare: zona compartilhada por tier/janela, regeneração até o timeout,
  limite de três entradas de party e cobrança transacional de NT no Arcano;
- Hell Gate: controlador, quatro quadrantes, combinação autoritativa de Lichs,
  corredor final e Aki, com cleanup de mobs/NPCs temporários;
- prazos independentes de combate, transição, quiz, saída e janela absoluta;
- validação de templates, terreno, IDs, rollback de spawn e testes de entrada,
  timeout, regeneração e máquina de estados.

Big Cube possui o núcleo O/X habilitado: o ticket 1737 abre uma pergunta privada,
O representa Sim, X representa Não e os sobreviventes retornam a Armia em
`(2100,2100)`. A cadeia completa de rodadas, portões e recompensas ainda não
foi confirmada; a configuração inicial contém uma pergunta de teste em
`data/volatiles.json`.

Regressão final desta rodada: a janela de saída da Water continua permitindo
o próximo pergaminho, mas nunca é ignorada pelo cleanup ao verificar uma
associação duplicada. Assim, uma sala antiga não pode teleportar um jogador
que já esteja registrado em uma sala nova. `go test ./...`, `go vet ./...` e o
build do servidor passaram; o `-race` permanece dependente do linker MinGW
com `-lsynchronization` (no Ubuntu/CI deve ser executado conforme a matriz).

## Auditoria complementar e correcoes (2026-08-01)

Esta revisao confirmou a implementacao das fases aplicadas e corrigiu dois
vazamentos que nao eram cobertos pela suite anterior:

- `REQMobByID` agora aplica `mobVisibleToPlayer` antes de rematerializar um mob.
  Um cliente fora da sala nao consegue recuperar o `CreateMob` de uma instancia
  privada mesmo conhecendo o ID.
- O dano e a barra de HP de ataques fisicos contra mobs usam `sendToMobView`,
  assim como skills e IA. O `broadcast` global foi removido desse caminho, logo
  observadores externos nao recebem hit, overkill ou HP de uma sala privada.
- Ingresso em uma zona compartilhada grava o `ScheduleEnd` efetivo junto com o
  consumo do ticket, a posicao e os membros. Falha restaura tambem esse prazo.
- Transicoes de salas persistem posicao dos membros e `CurrentStage` no mesmo
  commit; respostas do Cube persistem EXP, remocoes e a proxima transicao juntas,
  evitando que um crash reabra uma sala ja concluida.
- O agregado de instancias e os contadores do Nightmare sao persistidos em
  `instance_state.json` (JSON) ou na tabela `instance_state` (PostgreSQL). A
  abertura e o ingresso compartilhado usam commit conjunto com as contas.
- Reinicio do processo reconstroi mobs de dados autoritativos. Hell Gate agora
  restaura a fase `entry`, o puzzle de Lichs (variante, mascaras e Lichs ja
  materializados) e o corredor `final`; IDs de entidades e HP individuais nao
  sao confiados apos o reinicio.
- Cube permanece sem `CombatDeadline` artificial: `noCombatTimeout=true` deixa
  somente quiz, transicao e termino da execucao governarem a progressao.
- Nightmare mantem `maxPlayers=13` como tamanho nativo de cada party e calcula
  capacidade fisica de `13 * partyRunLimit` (39) para as tres parties da janela;
  tickets pessoais nao consomem o contador de parties.

Limitacoes deliberadas que ainda nao podem ser declaradas como fidelidade 100%:

- O Big Cube possui o núcleo O/X habilitado; novas rodadas e recompensas ficam
  em `data/volatiles.json` até a tabela nativa 7.48 ser confirmada.
- Hell Gate restaura a fase e os contadores persistidos, mas mobs mortos antes
  do ultimo snapshot podem reaparecer apos uma queda; o agregado nao persiste
  HP/UID de cada mob por desenho. As populacoes e coordenadas usadas continuam
  sendo as de `data/volatiles.json` e devem ser comparadas com a tabela nativa
  antes de habilitar o evento publicamente.

Testes adicionais nesta rodada cobrem recuperacao de `REQMobByID` em instancia
privada e restauracao do puzzle de Lichs. A validacao final executada foi:

```powershell
go test -count=1 ./...
go vet ./...
go build -o tmp/tm-check.exe ./cmd/server
git diff --check
```

A exceção de saída também aceita somente o item de recompensa da sala anterior;
outros tickets continuam recusados até a janela terminar.

A auditoria final também cobre falha durante a restauração do puzzle: se um
quadrante posterior não puder materializar seu template, todas as ondas que já
foram criadas são removidas antes de descartar a instância. O caso é garantido
por `TestRestoreHellGateRuntimeRollsBackPartialWave`.
