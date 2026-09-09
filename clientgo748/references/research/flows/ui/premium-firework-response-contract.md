---
id: premium-firework-response-contract
title: Contrato da publicacao de PremiumFirework
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Publicacao de PremiumFirework

## Pergunta

Qual envelope publica o bitmap do PremiumFirework sem permitir que um frame
parcial seja entregue ao efeito visual?

## Fronteira de evidência

- `UTILIZADA`: decompilacao nativa 7.48, source `MSG_PremiumFirework` e
  consumidor `TMHuman::OnPacketPremiumFireWork`.
- `UTILIZADA`: WYD-Go `wire.PremiumFirework`, fluxo de consumo/publicacao e
  testes.
- `NAO APLICAVEL`: guia KR e assets nao definem este envelope. Sources 7.54,
  W2PP, Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_00492E7D` encaminha `0x3CA` ao consumidor de efeito premium. O validador
`FUN_0055890A` exige exatamente 36 bytes.

### Callees

O consumidor usa o bitmap em `+20` para configurar `TMEffectFireWork`; os oito
bytes anteriores sao reservados e o efeito passa ao container da cena.

## Estado e lifecycle

O gate valida o frame antes de criar o efeito. O packet e emprestado durante o
callback; o efeito resultante pertence ao container e segue o teardown da cena.

## Wire, ABI e recursos

`0x3CA`, S->C, possui Header 12B, Reserved[8] em `+12` e Bitmap[16] em `+20`.
O gate central agora valida tamanho real/declarado e Type/opcode antes de criar o
efeito. O frame nao transfere ownership e o efeito passa ao container da cena.

## Mapeamento atual

`ReceivedPacketDispatch::ExpectedSize` usa `sizeof(MSG_PremiumFirework)` e o
builder Go ja limita a publicacao ao array de 16 bytes. A fixture C++ protege
truncamento, excesso, nulo, divergencias, entrega unica, offsets e preservacao.

## Matriz de delta

| Claim | Nativo 7.48 | Source/WYD-Go | Estado atual | Decisão |
| --- | --- | --- | --- | --- |
| envelope | `0x3CA/36B` | mesmo layout | gate e asserts | `PARIDADE_NATIVA` |
| bitmap | 16 bytes em `+20` | mesmo offset | coberto byte a byte | manter |

## Decisões

- Manter o envelope nativo de 36 bytes e o bitmap em `+20` (`PARIDADE_NATIVA`).

## Lacunas

- Executar desenho, publicacao ao observador, cooldown, fechamento e relogin no
  `project.exe`; ainda nao e `CLIENT_TESTED`.

## Validação

- Automacao: fixture C++ e testes Go do builder cobrem o contrato completo;
  `go test -count=1 ./...` passou.
- Build Release passou pelo `Build-Client.ps1` com 26887 checks e asserts
  estaticos. XML, `git diff --check` e validador de pesquisa passaram. Candidato
  instalado: `BB904B8E2AC97663F2BC18D38A5A86009A80173EDE93BAD727A570D47AF5A849`.
- Estado `AUTOMATED TESTED` / `STATICALLY VERIFIED`; ainda nao e
  `CLIENT_TESTED`.
