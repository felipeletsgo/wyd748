# Handoff: consumiveis, buffs e persistencia

Atualizado em: 2026-08-26
Estado geral: `AUTOMATED TESTED`

## Objetivo e limites

Restaurar o funcionamento dos consumiveis com affects distintos, consumir
Laktorerium Powder em toda tentativa valida confirmada e manter UID, prazo de
buff e expiracao de item rastreaveis no PostgreSQL. Este escopo nao altera
client, protocolo, ABI, UI ou assets; portanto o gate Ghidra 7.48 nao se aplica.

## Fontes e artefatos

```text
artefato | papel | hash/versao verificada
HEAD 56a25c04 | base atual da worktree | 2026-08-26
data/volatiles.json | regras de consumiveis e pacotes de affects | codigo atual
internal/store/postgres.go | SavePlayerState e ledger item_instances | codigo atual
```

## Evidencia confirmada

- Buffs persistem `SourceItemUID`, `SourceItemIndex` e `ExpiresUnix` no charstate — `TestBuffConsumablesPersistSourceUIDAndAbsoluteExpiration`, 2026-08-26.
- Itens 4140, 498 e 4145 alteram os pipelines autoritativos de EXP e combate conforme seus affects — `internal/game/consumables_handler_test.go`, 2026-08-26.
- Consumo e criacao do buff usam o mesmo commit, com rollback integral em falha — `TestAccountAndCharStateUseAtomicStoreWhenAvailable` e testes do handler, 2026-08-26.
- Expiracao de affect dispara persistencia imediata para impedir ressurreicao em relog — `TestTickPlayerAffectsRegenPoisonAreaAndExpiration`, 2026-08-26.
- Laktorerium e consumido em sucesso ou falha de rolagem valida, preservado em alvo invalido/teto e restaurado em falha PostgreSQL — `TestLaktoreriumPowderConsumesOnlyCommittedValidAttempts`, 2026-08-26.
- A ultima unidade consumida fica com slot vazio e auditoria `restante=0` — subteste `item 413 last unit disappears after successful commit`, 2026-08-26.
- Ledger PostgreSQL de itens temporarios preserva UID, ativacao e prazo absoluto — testes de charstate/PostgreSQL, 2026-08-26.
- `ShopTest` e `ShopTest2` entregavam 412/413/4140 com `EF_AMOUNT=120`; os blueprints agora sao unitarios e a regressao `TestOneShotConsumablesDoNotLeaveTestShopsAsArtificialStacks` protege esse contrato, 2026-08-26.
- Os UIDs confirmados de 413 e 4140 na conta `felipetr` foram reparados por `PostgresStore.SaveAccount`, preservando UID e sincronizando `accounts.payload` com `item_instances`; releitura confirmou efeitos zerados, 2026-08-26.

## Estado atual

```text
item | estado de validacao | evidencia
buffs com affects distintos | AUTOMATED TESTED | internal/game e formulas autoritativas
UID e expiracao absoluta de buff | AUTOMATED TESTED | charstate e payload PostgreSQL
consumo de Laktorerium | AUTOMATED TESTED | sucesso, falha, rejeicao e rollback
estoque unitario 412/413/4140 | AUTOMATED TESTED | dados reais de ShopTest/ShopTest2 e regressao
expiracao de itens temporarios | AUTOMATED TESTED | item_instances e relog
fluxo real no client | pendente | ainda nao executado em jogo
```

## Worktree e arquivos ativos

- `internal/game/affects.go`, `charstate.go`, `consumables.go` e testes associados — mudancas deste escopo.
- `internal/model/charstate.go`, `model.go` — origem UID e deadline persistidos.
- `internal/store/charstate_test.go`, `postgres_test.go` — contrato PostgreSQL coberto.
- `data/npcs/ShopTest.json`, `ShopTest2.json` e `internal/data/test_shop_coverage_test.go` — remocao das pilhas artificiais e regressao.
- `.agents/handoffs/client748-parity.md`, `client-source/tmproject/Projects/TMProject/TMFieldScene.cpp` e `client748/project.exe` — mudancas alheias preservadas e nao editadas neste escopo.

## Ultima validacao

```text
comando: go test ./internal/game ./internal/store ./internal/model -count=1; go vet ./...; go build ./...; git diff --check
resultado: passou integralmente em 2026-08-26

comando: go test ./internal/data -run 'TestOneShotConsumablesDoNotLeaveTestShopsAsArtificialStacks|TestCadaItemVolatileTemLojaDeTeste' -count=1
resultado: passou integralmente em 2026-08-26

comando: go test ./... -count=1
resultado: pacotes do escopo passaram; internal/data falhou apenas porque client748/Mounts-KR.json e client748/Costumes-KR.json nao existem na worktree
```

## Pendencias e riscos

- Executar no client real 412, 413 e 4140 recomprados e os UIDs reparados de 413/4140 antes de classificar o consumo como `CLIENT-TESTED`.
- A suite global continuara vermelha enquanto os dois assets KR alheios estiverem ausentes.

## Proximo passo executavel

Iniciar servidor com esta worktree, usar os UIDs reparados de 413/4140 e
recomprar 412/413/4140 nas lojas corrigidas. Confirmar remocao apos um uso,
relogar e conferir `accounts.payload` e `item_instances`; depois repetir 498 e
4145 para confirmar que as pilhas legitimas continuam decrementando.
