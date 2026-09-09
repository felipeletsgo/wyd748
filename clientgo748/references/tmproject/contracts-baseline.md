# Baseline de contratos 7.48

## Estado atual

Os contratos de mensagens ainda estão declarados principalmente em
`internal/core/Basedef.h`. As definicoes de MSG_STANDARD, MSG_CharacterLogin
e MSG_ReqTransper agora pertencem a headers independentes em `internal/wire/`;
Basedef os reexporta sem duplicar structs nem alterar os consumidores legados.

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

Continuar catalogando opcodes e pontos de serializacao por contrato.
O gate de recepcao atual valida especificamente 0xFAA/52 bytes antes dos
callbacks legados; nao cobre os demais payloads. Cada extracao deve preservar
uma definicao unica, assertions de layout e testes de entrada truncada,
valida e fallback, antes de integrar o consumidor.
