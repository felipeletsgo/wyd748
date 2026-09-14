---
id: quiz-choice-event
title: Pergunta recebida e clique nas quatro alternativas do quiz
subsystem: ui
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-14
---

# Pergunta recebida e clique nas quatro alternativas do quiz

## Pergunta

Qual painel nativo permite quatro respostas e qual fronteira deve ser preservada
ao adicionar um evento matematico seguro, com timeout autoritativo?

## Fronteira de evidência

- UTILIZADA: referencia `references/client748/WYD.exe`, identidade confirmada
  pelo projeto `WYD748Native_20260821.gpr` em modo read-only; export focado
  [quiz-choice-boundary.tsv](../../exports/quiz-choice-boundary.tsv), sem erro de script.
- UTILIZADA: corpus desse projeto, funcoes `00435b13`, `004662c5`, `004928a4`,
  `00492e7d`, `0055890a`; nao copiar o parser moderno como prova de wire nativo.
- UTILIZADA: source TMProject748, `TMFieldScene.cpp`, caminho compativel e
  ownership de SControlContainer/SPanel. TMProject posterior e referencia
  secundaria, CONTRADITORIA no tamanho do texto (128 contra 96 bytes nativos).
- Assets: NAO APLICAVEL para materializar este root; o nativo o cria dinamicamente
  apos carregar FieldScene/FieldScene2, nao depende de um ID serializado ausente.
- UTILIZADA: WYD-Go, dispatcher, persistencia de conta e lifecycle de personagem.

## Fluxo nativo 7.48

### Callers

Ghidra confirma `0x00493584 -> 0x004928a4`, caller `0x00492e7d`, ramo `0x1C6`.
`00435b13` tem xref DATA no slot `005A42E0`; `004662c5` e o receptor de
controles da mesma FieldScene (vtable `005A4294`, slots ja registrados em
[party-panel-layout-lifecycle.md](party-panel-layout-lifecycle.md)).

### Função principal

`00435b13` cria root `896`, texto e quatro botoes `897..900`, registra root no
container e filhos no painel; cada botao usa listener do container. Dois branches
de layout nativo possuem o mesmo grupo logico. `004928a4` escreve pergunta e
alternativas e torna root visivel. O callback `004662c5` esconde o root e envia
`0x2C7`, 16 bytes, parametro de 0 a 3. Nao confundir com root `880`, texto de aviso.

### Callees

Construcao: `00401466` (panel), `00401d03` (text), `00402f01` (button),
`0054ac09` (child); envio `0055f2dd`. O export de instrucoes do handler comprova
slots virtuais `+0x80`/`+0x88` para texto e `+0x60` para visibilidade. Receptores
vem dos membros `Field+28610` (root), `+28614` (pergunta), `+28618..28624` (botoes).

### Saídas e erros

O handler verifica ponteiros antes de escrever; o callback antigo nao carrega
token de rodada. O nativo nao prova seguranca no servidor e nao autoriza rewards.

## Estado e lifecycle

Na extensao, o root dinamico pertence ao container, filhos ao root; os membros
da FieldScene sao emprestados. Sem root/container a inicializacao retorna sem
abrir UI. Criar o conjunto uma vez apos LoadRC no caminho compativel. A cena
possui token e timer por instancia; clique consome o token local e fecha; timeout
usa relogio monotonico. Close recebido so afeta o token correspondente.

Cleanup/shutdown/logout: reutilizar ownership de cena comprovado em Party e
[character-logout-selectchar-relogin.md](../lifecycle/character-logout-selectchar-relogin.md).
Nao ha thread nem singleton do quiz. O servidor invalida a participacao ao sair
do mundo, mesmo na mesma sessao/slot. Relogin espera a proxima pergunta.

## Wire, ABI e recursos

Nativo observado: header 12; pergunta em +12, terminador em +107 (96 bytes),
alternativas em +108/+140/+172/+204 (32 cada). Resposta 16 com indice em +12.
O framing completo legado nao foi promovido a CONTRACT: nao sera reutilizado.

EXTENSAO_COORDENADA v1 (little endian, inteiros sem sinal salvo alternativas):

| Direcao/opcode | Bytes | Campos |
| --- | --- | --- |
| S->C `0x7F10` | 148 | header 0..11; version u16 @12=1; kind u16 @14 (0 close,1 show); token128 @16; timeoutMS u32 @32=10000; pergunta ASCII96 @36; alternativas i32[4] @132 |
| C->S `0x7F11` | 36 | header; version u16 @12=1; choice u16 @14=0..3; token128 @16; reservado u32 @32=0 |

Busca na source dos dois projetos nao encontrou uso previo de `0x7F10/0x7F11`.
Frames errados sao rejeitados antes da UI/reward; opcode antigo nunca aciona
recompensa. Client antigo nao participa: atualizar client e servidor juntos.
Nenhuma resposta correta, item ou quantidade e aceito do client.

## Mapeamento atual

`InitializeCompatFieldScene` retorna antes da criacao moderna do quiz.
O root dinamico foi implementado com os IDs nativos e handlers da extensao, sem
importar o ABI divergente de MSG_RandomQuiz. O evento v1 vive no World e no
painel local; nao muda a Control API HTTP read-only.

Recepcao: `ObjectManager::OnPacketView` valida o frame e despacha primeiro na
cena atual, sem roteamento por Header.ID para entidades; FieldScene consome
`0x7F10` e valida o ID do personagem. Clique envia somente `0x7F11` no quiz
coordenado. `QuizEventPacket.h` define tamanhos, parser e estado de timer/token.

## Matriz de delta

| Claim | Nativo | Source atual | Server | Decisao |
| --- | --- | --- | --- | --- |
| Quatro alternativas | root896/botoes897..900 | root materializado no bootstrap compativel | envia pergunta e alternativas | reutilizar grupo nativo |
| Timeout10s/cadencia60s | nao reivindicado | timer monotonico e close por token | prazo/cadencia autoritativos | extensao coordenada |
| Rodada segura | resposta so indice | token v1 consumido no primeiro clique | participante/prazo/unicidade validados | novos opcodes versionados |
| Recompensa | nao reivindicada | sem autoridade | item/quantidade da configuracao | persistir antes de publicar |

## Decisões

Extensao implementada, sem declarar paridade do parser legado. Token aleatorio
128 bits por participante/rodada; uma tentativa por conta/rodada; prazo medido
ao processar no World. Nao aceitar timestamp do client. Replay, login tardio,
resposta de outro jogador, logout e encerramento nao concedem recompensa.
WPE pode forjar UI local, mas isso nao cria participacao. Bots que resolvem
perguntas legitimas e contas multiplas nao sao eliminados por esse mecanismo.

## Lacunas

Teste visual/in-game, resolucao, quatro botoes, timeout, logout/relogin e rede
lenta. Framing e todas as indirections do quiz legado continuam LOCATED;
nao sao precondicao para o novo wire documentado e testado separadamente.

## Validação

- Pesquisa: export headless read-only com hash esperado, caller e instrucoes presentes.
- STATICALLY VERIFIED: contrato v1 com static_assert C++ e layouts Go; gate
  nativo legado permanece LOCATED, independente da implementacao coordenada.
- AUTOMATED TESTED: `go test -count=1 ./...`, `go vet ./...`, testes focados
  Quiz/GlobalDrop com race; replay, framing, fase, prazo, sessao, logout, falha
  de entrega/rollback e persistencia antes de publicacao. Banco de teste simulado
  nao comprova a entrega integrada no PostgreSQL real.
- C++: `pwsh -NoProfile -ExecutionPolicy Bypass -File tmproject/Build-Client.ps1
  -NoDeploy`, arquitetura/contrato 35.621 checks e build Release aprovados.
  Candidato `tmproject/build/TMProject748/Release/WYD.exe`; runtime nao substituido.
- Painel: OpenAPI generate, Astro check (zero erros/warnings) e build aprovados.
- Client real: pendente; nao e CLIENT_TESTED.
