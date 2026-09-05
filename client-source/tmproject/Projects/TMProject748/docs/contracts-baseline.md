# Baseline de contratos 7.48

## Estado atual

Os contratos de mensagens ainda estão declarados principalmente em
`internal/core/Basedef.h`; `internal/wire/` ainda não possui implementação
independente. Portanto, a Fase 0 não moveu structs nem alterou includes.

O header já contém assertions de tamanho e offsets para os contratos mais
sensíveis, incluindo `MSG_STANDARDPARM2`, `MSG_ResultGamble`, `STRUCT_ITEM`,
`MSG_Action`, `MSG_BuyToto` e `MSG_CNFRemoveServer`. Essas assertions são o
primeiro bloqueio contra regressões de ABI durante a migração.

## Gate registrado

- Projeto: `TMProject748.vcxproj`
- Configuração: `Debug | Win32`
- Resultado: build concluído com `EXIT_CODE=0`
- Executável: `build/TMProject748/Debug/WYDestiny.exe`

## Próxima ação

Catalogar opcodes e pontos de serialização antes de criar `wire` independente.
Cada contrato migrado deverá manter a definição legada como referência,
adicionar assertion equivalente e passar por build antes da remoção de qualquer
declaração duplicada.
