---
id: update-score-source-contract
title: Contrato coordenado de UpdateScore do source client
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Contrato coordenado de UpdateScore

## Pergunta

Qual envelope `0x336` deve transportar o snapshot autoritativo do WYD-Go para
o TMProject748, considerando que os formatos historicos encontrados na
decompilacao nao correspondem ao `STRUCT_SCORE` atual?

## Fronteira de evidência

- `UTILIZADA`: executavel 7.48 estudado do hash acima, projeto Ghidra
  `WYD748Native_20260821.gpr` e decompilacao de `FUN_0052EAA9`,
  `FUN_0052B97D`, `FUN_013C0000` e `FUN_0055890A`.
- `UTILIZADA`: source atual `STRUCT_SCORE`, `MSG_UpdateScore`,
  `TMHuman::OnPacketUpdateScore` e `ReceivedPacketDispatch`.
- `UTILIZADA`: WYD-Go `wire.UpdateScore`, `MobScore`, `EncodeClientScore`,
  emissores de progressao/equipamento/combate e testes byte a byte.
- `NAO APLICAVEL`: assets e guia KR nao definem este ABI. Sources 7.54, W2PP,
  Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_0052EAA9 @ 0x0052EAA9` encaminha `0x336` ao wrapper
`FUN_013C0000 @ 0x013C0000`, que termina em `FUN_0052B97D`.
`FUN_0055890A @ 0x0055890A` registra o envelope base de 92 bytes.

### Callees

`FUN_0052B97D` consome o score compacto, 16 affects, guild e campos auxiliares
dentro dos 92 bytes; atualiza ficha, grupo, movimento, guild mark, affects e
UI. O wrapper `FUN_013C0000` reconhece apenas frames acima de 235 bytes com o
marcador `XSC2` em `+232`, captura a cauda DWORD em `+92..+228` e depois chama
o handler base. Assim a decompilacao diferencia o formato base de 92 bytes e a
extensao historica de 236 bytes; nenhum deles prova o frame atual de 232 bytes.

## Estado e lifecycle

No source client, `Header.ID` seleciona o `TMHuman`. O handler aplica o
`STRUCT_SCORE`, sincroniza affects, guild/rank, custos pendentes e seletor de
efeito; depois atualiza party, barras, velocidade, montaria, guild mark e ficha.
O personagem local tambem atualiza o snapshot do ObjectManager. Os controles
opcionais continuam protegidos pelo lifecycle ja existente.

O transporte empresta o buffer apenas durante o callback. A cena e as
entidades nao retêm o frame. Troca de mapa e relogin recriam os objetos a partir
dos snapshots enviados pelo servidor.

## Wire, ABI e recursos

O par source/WYD-Go usa uma `EXTENSAO_COORDENADA` S->C de 232 bytes:

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`; `ID@6` seleciona o personagem |
| `0x0C` | 140 | `STRUCT_SCORE` canonico |
| `0x98` | 64 | `Affect[32]`, cada entrada u16 |
| `0xD8` | 2 | Guild u16 |
| `0xDA` | 2 | GuildLevel u16 |
| `0xDC` | 4 | ReqHp u32 |
| `0xE0` | 4 | ReqMp u32 |
| `0xE4` | 1 | LearnedSkill u8 |
| `0xE5` | 3 | padding MSVC zerado pelo builder |

Nao ha negociacao ou fallback: a unica implementacao ativa exige 232 bytes. O
gate rejeita os envelopes historicos de 92 e 236 bytes antes do cast. Nenhum
asset ou controle novo participa.

## Mapeamento atual

`UpdateScoreContract.h` nomeia opcode, tamanho e limites dos blocos.
`Basedef.h` fixa tamanho, offsets e contagem de affects. O gate central valida
tamanho real/declarado e Type/opcode; `TMHuman` usa a mesma constante no
dispatch. No Go, `UpdateScore` e `MobScore` constroem o frame e os testes
conferem Score, affects, guild, custos, seletor e padding.

## Matriz de delta

| Claim | Decompilacao 7.48 | Source/WYD-Go antes | Estado atual | Decisao |
| --- | --- | --- | --- | --- |
| base | 92B | nao usado | rejeitado | manter como evidencia |
| XSC2 | 236B, marcador em `+232` | nao usado | rejeitado | nao portar |
| snapshot ativo | sem equivalente exato | 232B coordenados | nomeado, assertado e gated | `EXTENSAO_COORDENADA` |
| efeitos | score/affects/guild/UI | implementados no handler atual | preservados | manter |

## Decisões

- Manter o frame de 232 bytes compartilhado pelo source client e WYD-Go.
- Registrar sua procedencia coordenada sem chama-lo de estrutura nativa.
- Rejeitar formatos historicos e preservar uma unica implementacao ativa.
- Nao alterar a ordem nem os efeitos do handler neste lote de envelope.

## Lacunas

- Executar level up, mudanca de equipamento/affect, guild/rank e custos de
  skill com owner/observer.
- Confirmar party, barras, guild mark, montaria, velocidade e relogin no
  `project.exe`.

## Validação

- Pesquisa: base, extensao XSC2, dispatch e consumidores foram diferenciados do
  contrato atual.
- Automacao: a fixture C++ cobre 232 prefixos truncados, excesso, XSC2 236B,
  nulo, Type/Size divergentes, receptor, limites dos blocos, entrega unica e
  preservacao. Testes Go conferem Score, affects, guild/rank, custos, seletor e
  padding; `go test -count=1 ./...` passou.
- Build Release passou pelo `Build-Client.ps1` com 25074 checks e asserts
  estaticos. XML, registro unico do header, `git diff --check` e validador de
  pesquisa passaram. Candidato instalado:
  `86766AA9FBF2EF15F76A7038883F2A945AA2160153D6224F2500269429E605AB`.
- Estado `AUTOMATED TESTED` / `STATICALLY VERIFIED`; o fluxo ainda nao e
  `CLIENT_TESTED`.
