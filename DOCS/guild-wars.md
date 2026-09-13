# Guerras de guilda: torre, cidades e fama

Atualizado em 13/09/2026. Implementação do servidor: `MODERNIZACAO_COMPATIVEL`.
Estado: `STATICALLY VERIFIED / AUTOMATED TESTED`; não é `CLIENT_TESTED`.

As regras de calendário e disputa foram comparadas com a referência externa
W2PP solicitada pelo usuário (`CWarTower`, timer e resultado de cidades).
Essa procedência não prova paridade nativa 7.48. Os envelopes utilizados já
existem no par ativo; não foi introduzido opcode, item de recompensa ou valor
econômico informado pelo client. A confirmação via NPC permanece separada.

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
cidade não podem desafiar outra. A confirmação `0x28E/0x28F` não cobra taxa e
apenas orienta o uso do comando, evitando cobrança por confirmação antiga.

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
  não foram inventados esses pagamentos. Tributação/coleta não foi implementada.
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
desliga agenda e gates; reativá-la aplica as regras de recuperação acima.

Implementação principal: `wydgo748/internal/game/guild_wars.go`;
modelo: `wydgo748/internal/model/guild_wars.go`; testes:
`guild_scheduled_wars_test.go`, `guild_wars_test.go`, testes de persistência,
configuração e `WarInfo`. Integrações em combate, movimento, teleporte,
guildas e tick reaproveitam o World autoritativo, sem goroutines novas.

## Validação e limites da entrega

Passaram nesta rodada:

- `go test -count=1 ./...` e `go vet ./...`;
- testes focados de calendário/avisos, captura, recompensa única, inscrição,
  rollback, cancelamento/reembolso, empate, limite de convocação, gates,
  ausência de penalidade comum e posições no terreno instalado;
- round-trip JSON e PostgreSQL real de fama/resultado, incluindo falha de
  exportação derivada sem desfazer o commit autoritativo;
- testes focados com `go test -race` em game/model/store, incluindo
  calendário, avisos, inscrição, fama e persistência PostgreSQL;
- `ArchitectureTests` Release/Win32: 35.439 checks e asserts estáticos;
- build Go em `wydgo748/bin/guild-war-server.exe`, sem reiniciar o servidor.

Ainda pendentes, portanto **não declarar a mecânica inteira finalizada**:

1. Janela nativa de inscrição pelo NPC: falta provar o prompt `0x18D` e
   ligar o fluxo de confirmação com contexto/expiração. A skill
   `wyd-client748-research` exige essa evidência antes da adaptação; o ABI
   conhecido de `0x28F` sozinho não prova o prompt.
2. Tributação, tesouro e coletor das cidades; recompensa econômica além do
   domínio exige implementação transacional própria, não um pagamento presumido.
3. Teste no `project.exe` com guildas adversárias: avisos/cores, seleção de
   alvo, chegada/barreiras, morte, skills/DoT, derrubadas sucessivas, resultado,
   fama, relogin e reinício. Build e simulação de relógio não substituem esse gate.
4. RvR/Akelonia-Hekalotia e castelo de Noatum continuam fora deste lote.

Nenhum arquivo foi removido por esta implementação. Não houve instalação de
executável do client, alteração do W2PP, commit/push ou reinício do runtime.
