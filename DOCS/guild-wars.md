# Guerras de guilda: torre, cidades e fama

Atualizado em 13/09/2026. Implementação do servidor: `MODERNIZACAO_COMPATIVEL`.
Estado: `STATICALLY VERIFIED / AUTOMATED TESTED`; não é `CLIENT_TESTED`.

As regras de calendário e disputa foram comparadas com a referência externa
W2PP solicitada pelo usuário (`CWarTower`, timer e resultado de cidades).
Essa procedência não prova paridade nativa 7.48. Os envelopes utilizados já
existem no par ativo; não foi introduzido opcode, item de recompensa ou valor
econômico informado pelo client. O fluxo nativo de confirmação via NPC foi
fechado separadamente contra o client 7.48 e o servidor mantém a autoridade.

## Calendário padrão

Horário local explícito: `America/Sao_Paulo`, independente do fuso do Windows.

| Evento | Momento | Ação/aviso |
| --- | --- | --- |
| Torre, segunda a sexta | 21:00 e 21:04 | Avisos de 5 minutos e 1 minuto antes do combate |
| Torre | 21:05 | Criação da torre, limpeza da arena e anúncio de início |
| Cada captura | Durante o combate | Nome do personagem/guilda, nova defensora, torre com HP cheio e saída dos participantes |
| Torre | 21:30 e 21:34 | Avisos de 5 minutos e 1 minuto para o encerramento |
| Torre | 21:35 | Última dona recebe 100 de fama; anúncio, remoção da torre e saída da arena |
| Cidades | Sábado, até 00:00 de domingo | Inscrições; anúncios de abertura e fechamento |
| Cidades, domingo | 19:55 e 19:59 | Avisos de 5 minutos e 1 minuto antes do evento |
| Cidades | 20:00 | Anúncio de início do evento e etapas |
| Cidades | 20:03 | Convocação para as quatro arenas e preparação protegida |
| Cidades | 20:05 | Aviso de 1 minuto antes do combate |
| Cidades | 20:06 | Liberação do combate por 15 minutos |
| Cidades | 20:16 e 20:20 | Avisos de 5 minutos e 1 minuto para o encerramento |
| Cidades | 20:21 | Resultado das quatro cidades, domínio/defesas persistidos e saída das arenas |

O servidor precisa estar em execução para publicar avisos. Não envia uma fila
de contagens antigas após reinício: só a janela ainda pertinente. Os textos
de contagem representam essa janela, não um relógio sincronizado no client.

Os anúncios são registrados no log e no chat. O painel nativo aceita 95 bytes
de texto e substitui seu aviso após cerca de quatro segundos. Por isso, os
textos são divididos e apresentados com intervalo de cinco segundos, em fila
limitada a 64 partes. O histórico do chat recebe todas as partes imediatamente;
um pico pode descartar somente apresentações excedentes no painel. A fila é
volátil: não há garantia de reentrega de um anúncio após queda do processo.
Os resultados econômicos não dependem dessa fila.

## Torre

- `/tower`: personagem vivo, de guilda, no espaço público de Erion, durante
  o combate. Destino de entrada `(2450,1855)`.
- Arena: `(2445,1850)..(2546,1920)`. Torre dedicada próxima a `(2495,1885)`.
  Estas posições foram verificadas no terreno instalado, mas não são uma
  alegação de posição exata do binário nativo.
- Usa o template instalado `Torre` (visual/HP/defesa), sem merchant, IA,
  experiência, drop ou progresso de quest. Não reutiliza o gerador 1075 da
  referência: esse índice identifica outro monstro nos dados ativos.
- Somente uma guilda diferente da dona pode causar dano à torre. Dano
  periódico também revalida autor, guilda, arena e horário.
- A captura grava a dona antes de anunciar; falha restaura HP e preserva a
  dona anterior. A torre recupera HP/affects após captura e todos saem para Erion.
- Recompensa: **100 de fama para a última guilda dona**, uma única vez por
  rodada. Sem dona, não há recompensa. Nenhum gold/item é criado.
- Reinício recupera dona e prazo; se ainda ativa, recria a torre com HP cheio.
  Se o prazo já venceu, conclui a premiação pendente sem duplicá-la.

## Cidades e inscrições

`/citywar` mostra no chat as quatro cidades, IDs de dona/desafiante, lance e
defesas. `/citywar 1`, `2`, `3` ou `4` inscreve respectivamente Armia, Azran,
Erion ou Nippleheim. Não usar esse comando numérico para mera consulta.

Inscrição exige líder canônico da guilda, vivo, cidadão do canal, presente
na cidade disputada, no sábado. A guilda precisa de pelo menos 100 de fama.
Seu lance é a fama **antes** da cobrança; vence o maior lance, mantendo a
primeira inscrição em empate. A taxa de 100 também é cobrada de lances que
não se tornam o maior. Repetir o pedido não cobra novamente.

Adaptações explícitas: uma inscrição por guilda/semana; guildas que já possuem
cidade não podem desafiar outra. O fluxo nativo de inscrição por coletor é
`0x28E` (C->S, 16 bytes) -> `0x18D` (S->C, `MSG_STANDARD` exato de 12 bytes)
-> `0x28F` (C->S, 20 bytes). Ao receber `0x28E`, o servidor valida NPC visível,
distância, cidade e dona atual, grava contexto efêmero com NPC/cidade, guilda
dona e TTL de 30 segundos, e só então abre a seleção nativa. O clique válido
em coletor (`onUseNPC`) também prepara esse contexto. `0x28F` consome-o uma
única vez; replay, expiração, troca de dona ou interação com outro NPC invalidam
a confirmação antes de qualquer cobrança ou registro.
Opção inválida e coletor inexistente, incompatível ou fora de alcance também
consomem o pedido anterior: é necessário interagir novamente. A guilda e a
liderança do solicitante são resolvidas no registro canônico na confirmação,
não confiadas ao contexto nem ao packet.

Coletores comprovados nos dados ativos: Balmus -> Armia (`Merchant=6`,
`Level=0`, posição `2107,2144`), Kara -> Azran (`Level=1`, `2538,1714`) e
Empis -> Erion (`Level=2`, `2466,1986`). Não existe coletor de Nippleheim nos
dados atuais; não foi inventado um NPC para essa cidade. O comando `/citywar`
continua disponível como caminho administrativo/jogável equivalente às mesmas
validações autoritativas.

A convocação seleciona até 26 personagens vivos de cada guilda participante,
presentes no espaço público da cidade, sem Loja Fantasma. O desempate de
seleção usa nome em ordem alfabética. Erion permite somente mortais. Não há
convocação para cidade sem desafiante. Trades são cancelados ao convocar.

Durante a preparação, o servidor impede cruzar o centro da arena e bloqueia
PvP. Durante o combate, só participantes adversários da mesma arena podem
atacar. Teleporte, morte, desconexão, saída da arena ou troca de guilda removem
a participação; relogar não concede nova vaga. Não há penalidade comum de
CP/experiência por morte nessas guerras. Dano direto, AoE e dano periódico
usam a autorização da guerra.

O servidor publica a guilda adversária por `WarInfo 0x3A8/24B` no início do
combate e limpa o snapshot na saída, morte e encerramento, reutilizando o
[contrato nativo já estudado](../.agents/research/client748/flows/transport/war-info-contract.md).
As barreiras de preparação são server-side: os dados ativos não contêm
portões nessas arenas, portanto não se inventou um objeto visual.

## Resultado, fama e persistência

- Pontos da cidade: soma dos níveis dos sobreviventes válidos, com acréscimo
  de 399 por não mortal. Empate preserva a dona; ataque precisa superar defesa.
- Conquista troca a dona e zera defesas; defesa bem-sucedida aumenta o contador
  até quatro. Sem desafiante, o domínio é conservado. Cidade sem dona e sem
  ataque pontuando permanece sem dona.
- Prêmio da cidade implementado: **domínio persistente e contador de defesas**.
  O resultado da referência não concede automaticamente fama, gold ou itens;
  não foram inventados esses pagamentos.
- A economia da cidade aplica **10% de imposto** nas compras comuns/TOTO e nas
  vendas a NPCs dentro de uma cidade controlada. Venda a NPC é definitiva; não
  existe fluxo de recompra. Um quarto do valor do imposto é creditado no
  tesouro da cidade. Compra comum, TOTO e venda passam pelo mesmo commit
  econômico autoritativo. Se o crédito ultrapassar o teto de
  `200.000.000.000`, a transação inteira é recusada sem perda de gold/item.
- Mutação do jogador, registro de guildas/guerras e tesouro formam uma unidade
  transacional lógica; falha de persistência restaura todos os estados
  alterados, evitando gold/item ou tesouro parcialmente aplicados.
- O líder da guilda proprietária pode retirar o tesouro. Saldo menor que
  `1.000.000.000` sai como gold direto, respeitando o teto do personagem. A
  partir de `1.000.000.000`, a retirada usa cheques de item `4011`, cada um
  valendo `1.000.000.000`; inventário limitado permite retirada parcial sem
  perder o restante do tesouro.
- `/guildfame` ou `/famaguild` consulta a fama. Registros antigos começam em
  zero; a torre fornece a fonte inicial. Soma rejeita overflow; inscrição
  rejeita saldo insuficiente.
- Fama, resultado, inscrições e marcadores de conclusão pertencem à mesma
  transação do registro de guildas, em JSON ou PostgreSQL. Não há dupla soma
  ou dupla subtração como nos caminhos redundantes da referência.
- Falha de gravação desfaz a mutação; nunca anuncia prêmio antes do commit.
  No encerramento da cidade, os pontos ficam congelados durante as tentativas
  de persistência, para saídas tardias não alterarem o resultado.
- Reinício durante preparação/combate cancela a rodada de cidades, devolve
  100 por inscrição paga (inclusive lances perdedores) e preserva as donas.
  Uma convocação perdida até o horário de combate também cancela: não inicia
  uma rodada reduzida com quem reconectou primeiro.
- Dissolução limpa referências da guilda em torres/cidades/inscrições dentro
  da transação existente, evitando atribuir domínio a um ID reutilizado.
- O estado usa o registro único já existente: não executar múltiplos Worlds
  independentes disputando o mesmo registro. Coordenação multisservidor não
  foi acrescentada neste lote.

## Configuração e arquivos

Chaves opcionais no arquivo de configuração do servidor:

```ini
guild_wars_enabled=true
guild_wars_timezone=America/Sao_Paulo
tower_war_hour=21
city_war_hour=20
```

Horas aceitas: 0..23. Os offsets da tabela permanecem iguais. A configuração
é lida no startup, sem mudança do relógio da máquina. Desabilitar a feature
desliga agenda, inscrições e gates de combate/movimento das guerras; não
remove domínios existentes nem desliga tributação/coleta desses domínios.
Reativá-la aplica as regras de recuperação acima.

Implementação principal: `wydgo748/internal/game/guild_wars.go`;
modelo: `wydgo748/internal/model/guild_wars.go`; testes:
`guild_scheduled_wars_test.go`, `guild_wars_test.go`, testes de persistência,
configuração, `WarInfo` e `guild_city_contract_test.go`. Integrações em combate,
movimento, teleporte, guildas, economia e tick reaproveitam o World
autoritativo, sem goroutines novas.

## Validação e limites da entrega

Validações acumuladas nesta conversa:

- `go test -count=1 ./internal/game`, `go test -count=1 ./...` e `go vet ./...`;
- testes focados do contrato nativo de coletor `0x28E -> 0x18D -> 0x28F`,
  incluindo replay, TTL, troca de dona, troca de NPC e coletores inválidos;
- testes focados de imposto em compra/venda/TOTO, rollback de persistência,
  limite de gold, inventário cheio e retirada parcial por cheque;
- testes focados de calendário/avisos, captura, recompensa única, inscrição,
  rollback, cancelamento/reembolso, empate, limite de convocação, gates,
  ausência de penalidade comum e posições no terreno instalado;
- round-trip JSON e PostgreSQL real de fama/resultado, incluindo falha de
  exportação derivada sem desfazer o commit autoritativo;
- testes focados com `go test -race` em game/model/store, incluindo
  calendário, avisos, inscrição, fama e persistência PostgreSQL;
- `ArchitectureTests` Release/Win32: 35.457 checks e asserts estáticos;
- build Release/x86 do TMProject concluído com `Build-Client.ps1 -NoDeploy`;
  o `tmproject/client748/project.exe` instalado não foi sobrescrito;
- build Go em `wydgo748/bin/guild-war-server.exe`, sem reiniciar o servidor.

Na revisão de fechamento, foi reproduzido e corrigido o contexto que permanecia
ativo após rejeições antecipadas no handler de confirmação. O teste falhou nos
sete casos antes da correção e passou depois, incluindo nova tentativa somente
após um novo pedido. Foram acrescentadas provas de rollback/retry de compra e
TOTO, teto do tesouro, autoridade da coleta, retirada em gold/cheques sem
duplicação e persistência JSON/PostgreSQL de tesouro acima de 32 bits.

Gates executados no fechamento:

- `go test -count=1 ./internal/game ./internal/store ./internal/model ./internal/wire`;
- `go test -race -count=1 -v ./internal/game ./internal/store -run 'TestCityWarRejectedConfirmation|TestCityTaxPurchaseFailure|TestCityTreasuryPersistenceFailure|TestGuildWarFameAndCompletion|TestPostgresGuildExportFailure'`;
  o teste PostgreSQL executou de fato no banco local de testes, sem skip;
- `go vet ./internal/game ./internal/store` e
  `go build -o bin/guild-war-server.exe ./cmd/server`;
- `git diff --check`.

O C++ não foi alterado nesta revisão; o gate anterior não foi repetido.

Gate ainda pendente, portanto **não declarar a mecânica inteira finalizada**:

1. Teste no `project.exe` com guildas adversárias: avisos/cores, seleção de
   alvo, chegada/barreiras, morte, skills/DoT, derrubadas sucessivas, resultado,
   fama, inscrição por coletor, tributação/coleta, relogin e reinício. Build e
   simulação de relógio não substituem esse gate.

RvR/Akelonia-Hekalotia e castelo de Noatum continuam fora deste lote, não são
pendências de implementação das guerras de torre/cidades aqui descritas.

Nenhum arquivo foi removido por esta implementação. Não houve instalação de
executável do client, alteração do W2PP, commit/push ou reinício do runtime.
