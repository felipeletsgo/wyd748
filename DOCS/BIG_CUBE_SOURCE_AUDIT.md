# Big Cube — auditoria de fonte (client 7.48)

**Data:** 2026-08-01
**Estado:** núcleo O/X implementado; rodadas e recompensas continuam configuráveis

O `Entrance_Ticket` (`item 1737`, `EF_VOLATILE=51`) agora abre uma execução
privada configurada. A pergunta é enviada somente aos membros inscritos; a
plataforma O representa Sim e X representa Não. O servidor valida a posição,
elimina quem escolheu a plataforma incorreta e recolhe os sobreviventes para
Armia `(2100,2100)`.

## Evidência encontrada

- O W2PP possui `BigCubo`/`SetBigCuboDoor` e comandos administrativos para
  abrir/fechar os portões, mas esses trechos não fornecem a tabela completa de
  rodadas, população, O/X, eliminação e recompensa necessária para reproduzir
  o evento com segurança.
- `ProcessSecMinTimer.cpp` apenas mantém a rotina de fechamento dos portões
  (`g_pInitItem`), e `_MSG_Imple.cpp` expõe limpeza manual das áreas O/X. Isso
  não é uma especificação suficiente para criar uma execução autoritativa.
- Não foi encontrada, nas fontes locais consultadas, uma tabela 7.48 que
  relacione de forma inequívoca o ticket a todos os portões, salas, perguntas,
  respostas e recompensas. Por isso, o núcleo foi implementado sem inventar
  uma cadeia de combate: perguntas, coordenadas O/X, tempo e EXP vivem em
  `data/volatiles.json` e podem ser ampliados quando a tabela completa for
  confirmada.

## Decisão de runtime

O código 51 usa `instance_ticket` com `stateMachine=big_cube`, não cria mobs e
abre a pergunta somente após o consumo persistido do ticket. A configuração
atual contém uma pergunta de teste; a validação server-side continua exigindo
as áreas O/X e não aceita respostas vindas de um pacote do cliente.

## Dados obrigatórios antes da implementação

Uma futura implementação só pode ser habilitada depois de preencher uma
tabela source-confirmed com:

1. coordenadas e estados dos portões;
2. salas, rodadas, NPCs e quantidades;
3. áreas O/X e perguntas/respostas;
4. regras de eliminação, tempo e reset;
5. recompensas e requisitos de entrada;
6. objetos permanentes usados para abrir/fechar o evento.

Quando a tabela completa for confirmada para o 7.48, novas rodadas, perguntas,
recompensas e portões devem ser adicionados somente aos dados, preservando o
controlador `state_machine` e os testes de rollback/timeout/limite.
