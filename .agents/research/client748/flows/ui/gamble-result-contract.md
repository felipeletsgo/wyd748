---
id: gamble-result-contract
title: Contrato do resultado de Gamble
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Resultado de Gamble

## Fronteira de evidência

- `UTILIZADA`: decompilacao nativa 7.48, `FUN_00492E7D`,
  `FUN_0055890A` e a struct `MSG_ResultGamble` ja existente.
- `UTILIZADA`: source `OnPacketRESULTGAMBLE`, builder WYD-Go e testes.
- `NAO APLICAVEL`: guia KR e assets nao definem este envelope. Sources 7.54,
  W2PP, Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

`FUN_00492E7D` encaminha `0x1BF` ao handler de Gamble e
`FUN_0055890A` exige exatamente 36 bytes. O consumidor le cinco resultados em
`+12`, tres paradas em `+17`, premio assinado em `+28` e jackpot em `+32`,
usando os oito bytes entre eles como reserva.

## Wire e lifecycle

O gate central agora exige 36 bytes, Size declarado e Type/opcode coerentes
antes de `OnPacketRESULTGAMBLE`. O handler valida faixas de simbolos/paradas,
atualiza os paineis de rolagem e encerra o estado pendente; nenhum ownership de
payload e retido.

## Mapeamento atual

`ReceivedPacketDispatch::ExpectedSize` usa `sizeof(MSG_ResultGamble)`. A
fixture C++ protege truncamento, excesso, nulo, divergencias, entrega unica e
todos os campos. O builder Go ja monta o mesmo layout e possui teste byte a byte.

## Decisão e lacunas

- Manter o envelope nativo de 36 bytes (`PARIDADE_NATIVA`).
- Executar aposta, rolagem, resultado invalido, fechamento e relogin no
  `project.exe`; ainda nao e `CLIENT_TESTED`.

## Validação

- Automacao: fixture C++ e testes Go cobrem o contrato completo; resultado de
  `go test -count=1 ./...` passou.
- Build Release passou pelo `Build-Client.ps1` com 26933 checks e asserts
  estaticos. XML, `git diff --check` e validador de pesquisa passaram. Candidato
  instalado: `C2C6B90409D1E23DC7BC70B609D636174AD9D6468F187E2ACC44509CF3A2CA7A`.
- Estado `AUTOMATED TESTED` / `STATICALLY VERIFIED`; ainda nao e
  `CLIENT_TESTED`.
