---
id: hp-mp-source-contract
title: Contrato coordenado de HP e MP do source client
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Contrato coordenado de HP e MP

## Pergunta

Qual envelope `0x181` deve ser usado pelo unico source client deste ecossistema
para receber HP/MP completos do WYD-Go, considerando que a decompilacao
estudada contem dois layouts historicos diferentes?

## Fronteira de evidência

- `UTILIZADA`: executavel 7.48 estudado do hash acima, projeto Ghidra
  `WYD748Native_20260821.gpr` e decompilacao de `FUN_0052EAA9`,
  `FUN_0052C5EB`, `FUN_013C0800` e `FUN_0055890A`.
- `UTILIZADA`: source atual `MSG_SetHpMp`, `TMHuman::OnPacketSetHpMp` e
  `ReceivedPacketDispatch`.
- `UTILIZADA`: WYD-Go `wire.SetHpMp`, `MobHpMp`, `HpMp`, testes e emissores de
  combate, affects, visibilidade, equipamento e AutoTrade.
- `NAO APLICAVEL`: assets e guia KR nao definem este ABI. Sources 7.54, W2PP,
  Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_0052EAA9 @ 0x0052EAA9` encaminha `0x181` ao wrapper
`FUN_013C0800 @ 0x013C0800`, que termina chamando `FUN_0052C5EB`.
`FUN_0055890A @ 0x0055890A` registra o envelope base de 20 bytes.

### Callees

`FUN_0052C5EB` consome quatro `WORD` em `+12`, `+14`, `+16` e `+18`, atualiza
recursos e barras e limita os valores atuais. O wrapper `FUN_013C0800` so
carrega seus quatro `DWORD` quando `Header.Size > 35`; nesse caminho os valores
ficam em `+20`, `+24`, `+28` e `+32`, formando 36 bytes. Portanto a
decompilacao confirma os layouts historicos de 20 e 36 bytes; ela nao prova o
layout coordenado de 28 bytes.

## Estado e lifecycle

No source client ativo, `Header.ID` seleciona o `TMHuman`. O handler aplica
CurHP/CurMP, atualiza maxima quando nao zero, limita os atuais, redesenha barras
e textos opcionais e remove o estado de morto quando recebe HP positivo. Para
o personagem local, o snapshot autoritativo tambem e copiado ao ObjectManager.

O frame e emprestado somente durante o callback. Nao existe ownership de
payload nem retencao apos a chamada. Na troca de cena/relogin, os personagens e
seus recursos sao recriados pelos snapshots normais do servidor.

## Wire, ABI e recursos

O ecossistema source/WYD-Go escolhe uma `EXTENSAO_COORDENADA` S->C de 28 bytes:

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`; `ID@6` seleciona o personagem |
| `0x0C` | 4 | `CurHP`, u32 |
| `0x10` | 4 | `CurMP`, u32 |
| `0x14` | 4 | `MaxHP`, u32 |
| `0x18` | 4 | `MaxMP`, u32 |

Nao ha fallback ou negociacao por peer: o projeto compila uma unica
implementacao e o WYD-Go envia este mesmo formato para todos os clients ativos.
O gate rejeita inclusive 20 e 36 bytes para impedir interpretacao ambigua sob o
mesmo opcode. Nenhum recurso visual novo e criado.

## Mapeamento atual

`HpMpContract.h` nomeia opcode, tamanho e quatro offsets. `Basedef.h` fixa o
layout de `MSG_SetHpMp`; `ReceivedPacketDispatch::ExpectedSize` valida o frame
antes da travessia de entidades; `TMHuman` usa o mesmo nome no dispatch. O
builder Go escreve os quatro `uint32` nos offsets contratados e ja possui teste
byte a byte.

## Matriz de delta

| Claim | Decompilacao 7.48 | Source/WYD-Go antes | Estado atual | Decisao |
| --- | --- | --- | --- | --- |
| envelope base | 20B com quatro WORD | nao usado | rejeitado | manter apenas como evidencia |
| extensao estudada | 36B com DWORD em `+20..+32` | nao usada | rejeitada | nao portar |
| contrato ativo | sem equivalente exato | 28B com quatro DWORD em `+12..+24` | nomeado, assertado e gated | `EXTENSAO_COORDENADA` |
| efeitos do handler | barras, clamp e estado de vida | preservados | preservados | manter |

## Decisões

- Manter o formato de 28 bytes ja compartilhado pelo source client e WYD-Go.
- Declarar sua procedencia como extensao coordenada, sem atribui-la ao nativo.
- Rejeitar comprimentos historicos em vez de manter caminhos multiversao.
- Preservar a semantica e o lifecycle existentes do handler para frames validos.

## Lacunas

- Executar dano, cura, regeneracao, morte e retorno a vida com owner/observer.
- Confirmar barras do alvo, grupo, guild battle e textos opcionais no
  `project.exe` em relogin e troca de mapa.

## Validação

- Pesquisa: os dois layouts historicos, seu dispatch e seus consumidores foram
  diferenciados do contrato atual.
- Automacao: a fixture C++ cobre todos os prefixos truncados, excesso, nulo,
  Type/Size divergentes, rejeicao implicita dos comprimentos 20/36, receptor,
  quatro offsets, entrega unica e preservacao. O teste Go existente confere os
  mesmos quatro `uint32`; `go test -count=1 ./...` passou.
- Build Release passou pelo `Build-Client.ps1` com 24830 checks e asserts
  estaticos. XML, registro unico do header, `git diff --check` e validador de
  pesquisa passaram. Candidato instalado:
  `6482779E56D623E672C5CB2328F280B525EDA19EEB0512CAB51A7EBDB4BF0341`.
- Estado `AUTOMATED TESTED` / `STATICALLY VERIFIED`; o fluxo ainda nao e
  `CLIENT_TESTED`.
