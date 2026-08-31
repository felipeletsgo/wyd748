---
id: set-my-human-magic-noop
title: SetMyHumanMagic vazio no client 7.48
subsystem: gameplay-ui
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# SetMyHumanMagic vazio no client 7.48

## Pergunta

O corpo vazio de `TMFieldScene::SetMyHumanMagic()` é uma lacuna da source ou
corresponde ao comportamento do executável nativo 7.48?

## Fronteira de evidência

- Executável: baseline identificada pelo SHA-256 do frontmatter.
- Projeto/corpus Ghidra: `FUN_004431D9`, corpo de `0x004431D9` até
  `0x004431E3`, com 11 bytes.
- Source: `TMFieldScene::SetMyHumanMagic()` e seu uso antes de
  `BASE_GetSkillDamage` em `TMFieldScene.cpp`.
- Cálculo atual: `Basedef.cpp` lê `STRUCT_MOB::CurrentScore.MagicAmp`.
- Wire/servidor: não há packet, persistência ou mutação server-side nesta
  chamada.

## Evidência nativa

`FUN_004431D9` retorna sem ler ou escrever estado e sem chamar qualquer
callee. O catálogo Ghidra registra três callers diretos:

- `FUN_004431E4`;
- `FUN_004528C5`;
- `FUN_00454763`.

Portanto, a ausência de corpo não é uma implementação esquecida neste ponto:
ela reproduz um no-op nativo. A função pode continuar sendo chamada pelos
fluxos vizinhos sem introduzir um mirror secundário de atributo mágico.

## Mapeamento atual

`TMFieldScene::SetMyHumanMagic()` permanece vazio. O preview de dano que a
chama segue para `BASE_GetSkillDamage`, e o cálculo atual consome o campo
canônico `CurrentScore.MagicAmp`. Recriar o antigo sidecar
`STRUCT_MOB::Magician` duplicaria o mesmo estado e poderia divergir do Score
autoritativo.

## Lifecycle e falhas

A chamada é síncrona, não aloca, não registra callback e não conserva estado.
Não há ownership, falha parcial, cleanup, teardown, shutdown ou estado de
logout/relogin específico. Os callers apenas continuam para seu próximo passo.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | Estrutura posterior/manual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| corpo da função | retorno sem efeito | vazio | sidecar `Magician` foi removido | Score autoritativo | manter no-op |
| amplificação mágica | não é atualizada aqui | `CurrentScore.MagicAmp` no cálculo | mirror seria duplicado | fornece Score | não restaurar mirror |
| callers | três callers diretos | chamada preservada no preview | N/A | N/A | preservar call site |

## Decisões

- Classificar o corpo vazio como `PARIDADE_NATIVA` com evidência estática.
- Marcar `FUN_004431D9` como `STATICALLY_EVIDENCED`/
  `PARITY_STATIC_EVIDENCE` no inventário, sem promover o fluxo a `TRACED` ou
  alegar teste runtime.
- Não restaurar `STRUCT_MOB::Magician`; `CurrentScore.MagicAmp` permanece a
  única representação ativa.

## Lacunas

Nenhuma lacuna bloqueia a manutenção do no-op. Os fluxos maiores dos três
callers continuam sujeitos às próprias fichas antes de qualquer claim de
paridade comportamental.

## Validação

- Pesquisa: corpo, tamanho, ausência de callees e três callers diretos
  conferidos no catálogo Ghidra do hash registrado.
- Automação: pendente da validação documental deste lote.
- Client real: não aplicável ao no-op isolado; nenhum claim `CLIENT_TESTED`.
