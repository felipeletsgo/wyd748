---
id: create-mob-source-contract
title: Contrato coordenado de materializacao de entidades
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Materializacao coordenada de entidades

## Pergunta

Quais envelopes de `CreateMob` e `CreateMobTrade` permitem que o WYD-Go
materialize entidades, score, affects e titulo de AutoTrade no TMProject748 sem
confundir o formato menor do executavel nativo?

## Fronteira de evidencia

- `UTILIZADA`: executavel nativo 7.48 do hash acima, projeto Ghidra
  `WYD748Native_20260821.gpr` e decompilacao de `FUN_00492E7D`,
  `FUN_004829F2` e `FUN_0055890A`.
- `UTILIZADA`: source atual `MSG_CreateMob`, `MSG_CreateMobTrade`,
  `OnPacketCreateMobCompat`, `OnPacketCreateMob` e asserts de `WYD748Compat`.
- `UTILIZADA`: WYD-Go `createMob`, `CreateMobVisual`,
  `CreateMobWithGuildRank`, `CreateMobTrade`, emissores de visibilidade/login e
  testes.
- `NAO APLICAVEL`: assets e guia KR nao definem os envelopes. Sources 7.54,
  W2PP, Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_00492E7D @ 0x00492E7D` encaminha `0x364` e `0x363` a
`FUN_004829F2 @ 0x004829F2`. O validador `FUN_0055890A` registra `0x364/0xB0`
para o formato base estudado. O source client ativo usa os snapshots
coordenados maiores descritos abaixo.

### Callees

`FUN_004829F2` procura a entidade por `MobID@+16`, copia equipamentos, affects,
nome, score e campos de guild/estado, cria ou atualiza `TMHuman` e, no ramo
`0x363`, consome a descricao de AutoTrade no tail. A materializacao tambem
calcula posicao, direcao, montaria, efeitos e estado de party/mercador.

## Estado e lifecycle

O source compat separa o self (ja materializado por `0x114`) dos observadores,
evita duplicar `TMHuman`, copia os blocos antes de `InitObject`, preserva titulo
de AutoTrade e insere o objeto no container da Field. O frame e emprestado
somente durante o callback; a entidade passa a ser dona de suas copias.

Relogin, troca de mapa e reentrada na visao criam outro snapshot. O servidor
continua autoritativo para score, equipamento projetado e affects; o client
apenas materializa.

## Wire, ABI e recursos

O par source/WYD-Go usa extensoes coordenadas:

| Opcode | Tamanho | Campos relevantes |
| --- | ---: | --- |
| `0x364` | 328 | Pos `+12`, ID `+16`, Name `+18`, Equip18 `+34`, Affect32 `+70`, Guild `+134`, Score140 `+140`, Type `+280`, Equip2 `+282`, Nick `+300`, Server `+326` |
| `0x363` | 352 | mesmo prefixo, Desc24 `+326`, Server `+350` |

`CreateMobContract.h` nomeia os dois opcodes, tamanhos, offsets e capacidades.
`Basedef.h` fixa cada bloco por asserts. O gate rejeita frames curtos, longos ou
com Type/opcode divergentes antes da criacao da entidade. Nenhum recurso novo e
criado por este contrato.

## Mapeamento atual

`ReceivedPacketDispatch::ExpectedSize` diferencia `0x364` e `0x363`.
`createMob` limita mesh ao array de 18 e grava os blocos nos offsets assertados;
`CreateMobTrade` copia apenas o titulo para `+326..+349`. Os testes Go conferem
posicao, ID, nome, mesh, guild, score, CreateType, AnctCode, titulo e padding.

## Matriz de delta

| Claim | Nativo estudado | Source/WYD-Go antes | Estado atual | Decisao |
| --- | --- | --- | --- | --- |
| `0x364` base | 176B/validador `0xB0` | 328B coordenados | gate de 328B | `EXTENSAO_COORDENADA` |
| `0x363` titulo | ramo nativo usa tail de descricao | 352B coordenados | gate de 352B | `EXTENSAO_COORDENADA` |
| lifecycle | cria/atualiza TMHuman | compat preserva ownership | inalterado | manter |

## Decisões

- Manter os snapshots 328/352 usados pelo único source client ativo.
- Rejeitar o envelope base menor para evitar casts ambíguos e leituras parciais.
- Preservar materializacao, ownership, AutoTrade e relogin existentes.
- Não misturar os dois tamanhos sob o mesmo opcode.

## Lacunas

- Executar spawn de player, NPC, mob e AutoTrade com owner/observer.
- Confirmar duplicata, remocao, troca de mapa e relogin no `project.exe`.

## Validação

- Pesquisa: caller, consumidor, offsets, tamanhos nativos e diferenca coordenada
  foram registrados.
- Automacao: fixtures C++ cobrem os dois tamanhos, todos os prefixos truncados,
  excesso, nulo, Type/Size divergentes, receptor, blocos de entidade, tail de
  servidor/titulo, entrega unica e preservacao. Testes Go conferem posicao, ID,
  nome, mesh, guild, score, CreateType, AnctCode, titulo e padding;
  `go test -count=1 ./...` passou.
- Build Release passou pelo `Build-Client.ps1` com 26538 checks e asserts
  estaticos. XML, registro unico dos headers, `git diff --check` e validador de
  pesquisa passaram. Candidato instalado:
  `EF6F220965F9FC74BB045DD1CA703D04C8E4728FE4BA7BCC78DBE2FCF7215557`.
- Estado `AUTOMATED TESTED` / `STATICALLY VERIFIED`; o fluxo ainda nao e
  `CLIENT_TESTED`.
