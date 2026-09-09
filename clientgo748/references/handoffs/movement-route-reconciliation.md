# Handoff: reconciliacao de movimento 0x366 e 0x2CB

Atualizado em: 2026-08-26
Estado geral: `AUTOMATED TESTED`

## Objetivo e limites

Eliminar falsos positivos de rota durante movimento normal do client WYD 7.48
sem aceitar teleporte, velocidade fornecida pelo pacote ou reposicionamento por
MoveStop. O teste real no client ainda nao foi executado neste estado.

## Fontes e artefatos

```text
client748/wyd.exe nativo+patches/WYD.exe | referencia nativa Ghidra | SHA-256 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
client-source/tmproject/Projects/TMProject/Basedef.cpp | BASE_GetSpeed 7.48 | limite 1..7 verificado
internal/game/security.go | validacao e reconciliacao server-side | worktree 2026-08-26
internal/game/player_movement.go | relogio autoritativo | worktree 2026-08-26
```

## Evidência confirmada

- `FUN_00524bbb` monta o 0x366 de 52 bytes com Pos, Speed, Target e Route.
- `FUN_00520216` substitui continuamente a rota usando a posicao visual atual.
- `FUN_0046087b`, chamada por `FUN_004625da`, copia a posicao visual para o
  0x2CB antes de determinados ataques.
- O servidor limitava AttackRun a 6 enquanto o client 7.48 usa ate 7; isso
  acumulava atraso de aproximadamente um tile por segundo.
- O primeiro pacote recusado estava 25 tiles a frente, apenas um acima do teto
  antigo de Route[24], e a recusa impedia instalar qualquer plano de recuperacao.

## Estado atual

```text
AttackRun 7 avanca 7 tiles/s | AUTOMATED TESTED | TestMovementUsesNative748MaximumSpeedSeven
ponte visual de 25 tiles sem teleporte | AUTOMATED TESTED | TestMovementAcceptsObservedTwentyFiveTileVisualLeadWithoutTeleport
sequencia real de 11 replans aceita | AUTOMATED TESTED | TestMovementAcceptsObservedSpeedSevenReplanSequence
fila acima de 48 e terreno bloqueado rejeitados | AUTOMATED TESTED | security_runtime_regression_test.go
MoveStop visual plausivel nao reposiciona nem viola | AUTOMATED TESTED | TestMoveStopAcceptsNativeVisualPositionWithoutRepositioning
```

## Worktree e arquivos ativos

- `internal/game/security.go` — velocidade 7, ponte limitada pela fila de 48 e
  validacao segura da posicao visual do 0x2CB.
- `internal/game/player_movement.go` — catch-up usa o teto nativo 7.48; passos
  futuros continuam usando o Score autoritativo.
- `internal/game/visibility.go` — comentario de contrato corrigido para 1..7.
- `internal/game/security_test.go` e
  `internal/game/security_runtime_regression_test.go` — regressões deste escopo.
- Demais modificacoes listadas por `git status --short` sao alheias e foram
  preservadas, inclusive client, lojas e persistencia de buffs.

## Última validação

```text
comando: go test -count=1 ./internal/game
resultado: PASS

comando: go test -count=1 ./internal/wire ./internal/game
resultado: PASS

comando: go vet ./...
resultado: PASS

comando: go test ./...
resultado: pacotes de movimento PASS; suite global bloqueada somente pela ausencia alheia de client748/Mounts-KR.json e client748/Costumes-KR.json em internal/data

comando: git diff --check -- arquivos deste escopo
resultado: PASS
```

## Pendências e riscos

- Executar caminhada longa, curvas e interrupcao por ataque no client real antes
  de promover o estado para `CLIENT-TESTED`.
- A suite global depende dos dois JSONs ausentes do escopo de lojas/dados.

## Próximo passo executável

Reiniciar o servidor, caminhar continuamente por mais de um minuto com
AttackRun 7 e interromper a caminhada com ataques; confirmar que nao surgem
recusas 0x366/0x2CB nem desconexao.
