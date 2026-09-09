---
id: shop-list-contract
title: Contrato da lista de loja NPC
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Lista de loja NPC

## Pergunta

Como o client 7.48 recebe `0x17C`, abre a loja e materializa as 27 celulas sem
deixar um frame incompleto alcancar ordenacao ou insercao visual?

## Fronteira de evidência

- `UTILIZADA`: binario nativo 7.48 do hash acima, projeto Ghidra e
  decompilacao de `FUN_00492E7D`, `FUN_004875C0` e `FUN_0055890A`.
- `UTILIZADA`: source atual `MSG_ShopList`, `OnPacketShopList`, grids de loja e
  `ReceivedPacketDispatch`.
- `UTILIZADA`: WYD-Go `wire.ShopList`, handlers de requisicao/compra e testes.
- `NAO APLICAVEL`: guia KR e assets nao definem o envelope. Sources 7.54,
  W2PP, Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_00492E7D @ 0x00492E7D` encaminha `0x17C` a
`FUN_004875C0 @ 0x004875C0`. `FUN_0055890A @ 0x0055890A` exige `0xEC`/236 bytes.

### Callees

O handler limpa a grade apropriada conforme `ShopType`, percorre exatamente 27
itens desde `+16`, cria cada visual valido e posiciona em 9 colunas; grava Tax
de `+232` e abre a janela. Para loja de skill, tambem seleciona textos por
faixa de indices. O pacote e consumido como snapshot; nenhum item do frame e
retido apos a materializacao.

## Estado e lifecycle

`OnPacketShopList` na source escolhe a grade normal ou skill, esvazia visuais
anteriores, insere apenas itens validos e atualiza o estado fiscal. O servidor
mantem a lista autoritativa e compacta no mesmo limite. Fechar a loja usa o
lifecycle existente; um novo `0x17C` substitui a apresentacao anterior.

## Wire, ABI e recursos

`0x17C`, S->C, 236 bytes:

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD` |
| `0x0C` | 4 | ShopType u32 |
| `0x10` | 216 | `STRUCT_ITEM List[27]` |
| `0xE8` | 4 | Tax u32 |

`ShopListContract.h` nomeia opcode, tamanho, offsets e contagem. `Basedef.h`
fixa o layout por asserts. O gate exige 236 bytes, Size declarado e
Type/opcode coerentes antes de `OnPacketShopList`. Nao ha recurso novo.

## Mapeamento atual

`wire.ShopList` grava no maximo 27 itens e deixa o restante zerado; callers com
listas maiores nao ultrapassam Tax. `ReceivedPacketDispatch::ExpectedSize`
protege o cast, e a source usa a constante do opcode no dispatch. O slot visual
continua sendo a topologia nativa 9x3 por pagina, conforme o tipo de loja.

## Matriz de delta

| Claim | Nativo 7.48 | Source/Go antes | Estado atual | Decisao |
| --- | --- | --- | --- | --- |
| envelope | `0x17C/236` | 236B sem gate | gate e asserts | `PARIDADE_NATIVA` |
| itens | 27 em `+16` | mesmo limite | preservado e limitado | manter |
| Tax | DWORD em `+232` | mesmo campo | preservado | manter |

## Decisões

- Validar 236 bytes antes de limpar ou materializar a loja.
- Manter o limite de 27 itens e o tail Tax intacto.
- Preservar o lifecycle normal/skill existente.

## Lacunas

- Executar loja normal, skill, lista vazia, imposto, compra e fechamento no
  `project.exe`.
- Repetir abertura e relogin para confirmar ausencia de itens fantasmas.

## Validação

- Pesquisa: caller, consumidor, tamanho, offsets, limite e lifecycle registrados.
- Automacao: fixture C++ cobre 236 prefixos truncados, excesso, nulo,
  Type/Size divergentes, receptor, limites das 27 celulas, Tax, entrega unica e
  preservacao. O teste Go usa 28 itens e confirma truncamento antes de Tax;
  `go test -count=1 ./...` passou.
- Build Release passou pelo `Build-Client.ps1` com 26785 checks e asserts
  estaticos. XML, registro unico do header, `git diff --check` e validador de
  pesquisa passaram. Candidato instalado:
  `39B9FC79FE86388E54DB32FB31F0EADAC5E0E81012CE4D3960A705CC708427A7`.
- Estado `AUTOMATED TESTED` / `STATICALLY VERIFIED`; o fluxo ainda nao e
  `CLIENT_TESTED`.
