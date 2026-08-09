# Auditoria do WYD-Go

## Código morto

Execute:

```powershell
go run golang.org/x/tools/cmd/deadcode@latest -test ./...
```

Classifique cada resultado com evidência:

- **Scaffolding:** possui TODO/pendência ou pertence a sistema incompleto.
  Manter e confirmar antes de remover.
- **Morto real:** foi substituído por caminho ativo. Confirmar com `rg` e
  histórico antes de propor remoção.

Não remova stubs sem autorização. `vet` e `deadcode` limpos não detectam
transações duplicadas; procure grants, saves e rollbacks copiados.

## Símbolos intencionalmente sem uso

- `internal/game/mob_combat.go: World.mobHasActivePlayer`: aguarda rework da IA.
- `internal/game/combat.go: magicDamage`: marcador até portar dano mágico e
  resistências; o caminho vivo usa `skillFinalDamage`/`skillUsesMagic`.
- `cmd/npcconvert/main.go: cstr/indexByte`: utilitários de conversão pontual.

Antes de alterar esta lista, confira se o complemento já foi implementado.
