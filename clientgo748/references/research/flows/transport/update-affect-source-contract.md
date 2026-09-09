---
id: update-affect-source-contract
title: Contrato do snapshot de affects do source client
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Snapshot de affects do source client

## Pergunta

Qual envelope `0x3B9` deve chegar ao TMProject748 e quais partes do registro de
oito bytes sao provadas pelo consumidor 7.48 ou definidas pelo par source/WYD-Go?

## Fronteira de evidência

- `UTILIZADA`: binario nativo do hash acima, projeto Ghidra
  `WYD748Native_20260821.gpr` e decompilacao de `FUN_0052EAA9`,
  `FUN_0052B72A` e `FUN_0055890A`.
- `UTILIZADA`: source atual `STRUCT_AFFECT`, `MSG_UpdateAffect`,
  `TMHuman::OnPacketUpdateAffect` e `ReceivedPacketDispatch`.
- `UTILIZADA`: WYD-Go `wire.UpdateAffects`, `putSourceAffects`, projecao dos
  tipos visuais e testes byte a byte.
- `NAO APLICAVEL`: assets e guia KR nao definem este wire. Sources 7.54, W2PP,
  Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_0052EAA9 @ 0x0052EAA9` encaminha `0x3B9` a
`FUN_0052B72A @ 0x0052B72A`. O handler roda para o personagem local na Field e
percorre exatamente 16 registros. A busca textual em `FUN_0055890A` nao
encontrou literal `0x3B9`; portanto o gate atual nao e atribuido a esse
validador.

### Callees

`FUN_0052B72A` acessa Type em `entry+0`, Time DWORD em `entry+4`, compara a
expiracao contra o relogio da cena e copia os oito bytes quando o estado muda.
O ultimo registro comeca em `+132` e termina no byte 139, provando Header de 12
bytes mais 16 entradas de oito. Os dois bytes centrais sao copiados, mas essa
funcao nao decide isoladamente sua semantica de Level/Value.

## Estado e lifecycle

O source ativo interpreta cada registro como `Type:u8`, `Level:u8`,
`Value:i16`, `Time:i32`. O handler atualiza timers, estado visual de joias,
`m_stAffect`, `CheckAffect` e ficha do owner. O WYD-Go satura Value em i16,
converte duracao para unidades do client e limita o snapshot aos 16 registros.

O frame e emprestado apenas durante o callback. Os registros sao copiados para
o estado do personagem; nenhum ponteiro ou ownership do buffer e retido.
Relogin e troca de mapa substituem o estado pelo snapshot autoritativo seguinte.

## Wire, ABI e recursos

`0x3B9`, S->C, 140 bytes no contrato source/WYD-Go:

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`; `ID@6` seleciona o personagem |
| `0x0C` | 128 | `STRUCT_AFFECT Affect[16]` |
| `entry+0` | 1 | Type u8 |
| `entry+1` | 1 | Level u8, coordenado |
| `entry+2` | 2 | Value i16, coordenado |
| `entry+4` | 4 | Time i32/unidades do client |

`UpdateAffectContract.h` nomeia opcode, tamanho, contagem e offsets.
`Basedef.h` fixa a struct e o array por asserts. `ReceivedPacketDispatch` exige
140 bytes, Size declarado e Type/opcode coerentes antes do cast. Nao ha recurso
novo.

## Mapeamento atual

`TMHuman` usa a constante do contrato no dispatch. `wire.UpdateAffects` chama
`putSourceAffects`, que limita o numero de entradas, converte o tipo visual,
satura Value e respeita o fim do buffer. Os testes Go cobrem o primeiro e o
decimo sexto registro, inclusive Type/Level/Value/Time.

## Matriz de delta

| Claim | Decompilacao 7.48 | Source/WYD-Go antes | Estado atual | Decisao |
| --- | --- | --- | --- | --- |
| contagem/tamanho | 16 x 8; termina em byte 139 | 140B sem gate | gate e asserts | `MODERNIZACAO_COMPATIVEL` |
| Type/Time | acessados em `+0/+4` | mesmos offsets | preservados | manter |
| Level/Value | bytes copiados, sem semantica fechada no handler | `u8/i16` coordenados | explicitados e testados | `EXTENSAO_COORDENADA` |
| validador 0055890A | sem literal `0x3B9` | sem gate | gate local | nao atribuir ao nativo |

## Decisões

- Exigir 140 bytes com base no limite realmente consumido e no contrato Go.
- Preservar a interpretacao Level/Value ja compartilhada pelo source e servidor.
- Nao afirmar que `FUN_0055890A` valida este opcode.
- Manter efeitos, timers e lifecycle do handler inalterados.

## Lacunas

- Executar aplicacao, renovacao e expiracao de affects com owner/observer.
- Confirmar icones, joias, relogin e troca de mapa no `project.exe` antes de
  classificar o fluxo como `CLIENT_TESTED`.

## Validação

- Pesquisa: dispatch, loop, limite final, offsets provados e fronteira
  coordenada foram diferenciados.
- Automacao: a fixture C++ cobre 140 prefixos truncados, excesso, nulo,
  Type/Size divergentes, receptor, campos da primeira/ultima entrada, entrega
  unica e preservacao. O teste Go confere os mesmos campos nas entradas 0 e 15;
  `go test -count=1 ./...` passou.
- Build Release passou pelo `Build-Client.ps1` com 25836 checks e asserts
  estaticos. XML, registro unico do header, `git diff --check` e validador de
  pesquisa passaram. Candidato instalado:
  `557CDB4A9295B0C86C0E7F7531DECBA045FF1E2754E228C53D7AF6EC0FAE90BA`.
- Estado `AUTOMATED TESTED` / `STATICALLY VERIFIED`; o fluxo ainda nao e
  `CLIENT_TESTED`.
