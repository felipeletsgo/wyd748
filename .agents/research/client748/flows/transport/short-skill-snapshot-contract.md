---
id: short-skill-snapshot-contract
title: Snapshot autoritativo da barra de vinte skills
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-30
---

# Snapshot autoritativo da barra de vinte skills

## Pergunta

Qual envelope `0x378/SetShortSkill` o client ativo pode entregar ao handler da
cena sem permitir que um frame truncado seja convertido para
`MSG_SetShortSkill` e lido alem do buffer?

## Fronteira de evidência

- UTILIZADA: ficha nativa `flows/ui/skill-belt-page-selection.md`, baseada no
  projeto Ghidra `WYD748Native_20260821`, para o envio `0x378/32` dos vinte
  atalhos e para a divisao das paginas `[0..9]` e `[10..19]`.
- UTILIZADA: source atual `TMFieldScene::SetShortSkill`, dispatcher de Field,
  `OnPacketSetShortSkill`, `MSG_SetShortSkill` e o gate central de recepcao.
- UTILIZADA: `World.onSetShortSkill`, `filterShortSkills`,
  `wire.SetShortSkill` e os testes de layout/dispatch do WYD-Go.
- NAO APLICAVEL: assets; nenhum controle, ID ou recurso foi alterado.
- NAO CONSULTADAS: TMProject posterior, sources 7.54 e fontes legadas externas.

## Fluxo nativo 7.48

### Entrada observável

- Evento C->S: ao aceitar uma troca de atalho, o client envia `0x378/32` com o
  array completo de vinte bytes em `+12`.
- Evento S->C no par ativo: o WYD-Go devolve o mesmo envelope depois de remover
  atalhos que nao correspondem a skills aprendidas.
- Saida observavel: o client substitui seu array local, normaliza indices para
  a classe atual e reconstroi as duas paginas da barra.

### Callers

A pesquisa nativa ja registrada liga o drop aceito na barra a
`FUN_00416196`, que atualiza o slot e envia o array completo por
`FUN_0055F2DD`. Na source atual, `TMFieldScene::OnPacketEvent` reconhece
`0x378` e chama diretamente `OnPacketSetShortSkill` com o buffer convertido.

### Função principal

`OnPacketSetShortSkill` copia incondicionalmente `Skill[20]` para
`ObjectManager::m_cShortSkill`, normaliza os vinte indices conforme a classe e
chama `UpdateSkillBelt`. Antes deste corte, o opcode nao estava em
`ReceivedPacketDispatch::ExpectedSize`; assim, um prefixo com header valido
podia chegar ao `memcpy`.

### Callees

`UpdateSkillBelt` materializa o estado ja copiado nas grades das duas paginas.
No servidor, `onSetShortSkill` copia `pkt[12:32]`, chama `filterShortSkills` e
responde com `wire.SetShortSkill`.

### Saídas e erros

O frame valido e entregue uma unica vez e o buffer continua emprestado. Nulo,
qualquer prefixo abaixo de 32 bytes, excesso, `Header.Size` ou `Header.Type`
divergentes sao rejeitados antes do callback. Nao ha resposta C++ para frame
invalido; o estado e a UI permanecem inalterados.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| drop aceito | Field e belt ativos | `SetShortSkill` | array local proposto | envia `0x378/32` | falha de UI nao envia |
| request valido | sessao/personagem ativos | `onSetShortSkill` | array filtrado | atualiza o personagem em memoria e responde | atalhos invalidos viram `0xFF` |
| snapshot valido | Field ativo | `OnPacketSetShortSkill` | vinte indices substituidos | rebuild das duas paginas | retorna `1` |
| envelope invalido | qualquer | `ReceivedPacketDispatch` | estado preservado | nenhum callback | retorna `false` |

### Vtables, vptrs e receptores

N/A para o envelope: o dispatcher e o callback sao chamadas diretas. O
receptor logico e a FieldScene e o estado pertence ao ObjectManager.

### Ownership

O dispatcher empresta o frame somente durante o callback. O handler copia os
vinte bytes para armazenamento proprio; nao conserva ponteiro para o packet.

### Falha parcial

O gate ocorre antes da primeira escrita do handler. Logo, truncamento nao pode
produzir array parcialmente novo nem disparar rebuild com bytes externos ao
frame. A filtragem server-side conclui antes do eco autoritativo.

### Cleanup e teardown

O contrato nao cria alocacao, timer ou callback. Os itens visuais continuam
sob ownership das grades e seguem o teardown existente da FieldScene.

### Shutdown

N/A: nao ha recurso proprio. Disconnect apenas impede a chegada do eco; o
estado autoritativo continua no personagem mantido pelo servidor.

### Logout e relogin

No proximo enter-world, o WYD-Go publica novamente `SetShortSkill` a partir do
personagem carregado. O gate de 32 bytes vale tanto para esse snapshot inicial
quanto para ecos de alteracao, compra/reset de skills e mudanca celestial.

## Wire, ABI e recursos

Direcao C<->S, opcode `0x378`, tamanho fixo de 32 bytes:

| Campo | Offset | Largura | Semântica |
| --- | ---: | ---: | --- |
| `MSG_STANDARD` | 0 | 12 | size, key, checksum, type, id e tick |
| `Skill` | 12 | 20 | array integral das duas paginas |

`ShortSkillSnapshotContract.h` centraliza opcode, tamanho, offset e contagem.
`MSG_SetShortSkill` possui asserts para os tres invariantes. Nao ha asset,
packing novo ou alteracao de payload.

## Mapeamento atual

### Source recompilável

`ReceivedPacketDispatch::ExpectedSize` agora registra `0x378/32`. A validacao
central compara opcode externo, `Header.Type`, `Header.Size` e tamanho real
antes de `TMFieldScene::OnPacketEvent` converter o buffer.

### WYD-Go

`wire.SetShortSkill` ja constroi exatamente 32 bytes e copia os vinte atalhos
para `[12:32]`. O request e registrado com tamanho 32; o handler rejeita
prefixo, filtra o array e devolve o estado autoritativo. Enter-world, compra ou
reset de skills, consumiveis de inicializacao e mudanca celestial reutilizam o
mesmo builder.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| opcode/tamanho | emissor `0x378/32` comprovado | struct 32B | builder e dispatch 32B | manter |
| payload | vinte atalhos em `+12` | `Skill[20]` | `[12:32]` | manter |
| autoridade | envio integral apos mudanca | aplica resposta integral | filtra e ecoa | manter servidor autoritativo |
| gate S->C | nao alegado nesta ficha | opcode ausente do gate incremental | tamanho unico | exigir 32B exatos |

## Decisões

- Classificar o gate como `MODERNIZACAO_COMPATIVEL`: ele preserva o wire 7.48
  comprovado e endurece apenas a entrada do peer ativo.
- Rejeitar frames maiores e menores em vez de copiar um prefixo ou ignorar
  cauda, mantendo o snapshot indivisivel.
- Nao alterar a normalizacao de indices, os controles das paginas nem a
  filtragem autoritativa do servidor neste corte.

## Celestial form shortcut synchronization — 2026-10-30

Classification: `MODERNIZACAO_COMPATIVEL`. Reuse the recorded 0x378 envelope
and two-page ownership. This correction claims no new native parity.
Static inspection found two causes when Mysterious Stone changes the class:
the client retained its login class, and UpdateEtc could send old shortcuts
back while the new form snapshot was arriving. Its local filter also shifted
by invalid bit indices for empty or foreign-class slots.

The server now sends the committed face item before score, learned skills,
affects and shortcuts. Existing S->C 0x182 remains 24 bytes: equipment type 0
at +12, slot 0 at +14, item at +16. `CharacterFaceItem` retains the cosmetic
emblem projection. The local SendItem consumer maps the catalog face ability
1/2/4/8 to skill class 0/1/2/3 before class-dependent UI updates. Other face
abilities retain the current class; observer and invalid-item guards remain
before mutation. Visual-only equipment packets do not change the skill class.

UpdateEtc still installs learned skills and counters. It no longer changes or
echoes shortcuts. The following 0x378 replaces all twenty slots and rebuilds
both pages using the new class. User shortcut edits retain their existing
request path. No wire sizes, resources, persisted fields or locale content change.

Each Celestial form retains its own learned mask and shortcut array. The
critical path prepares a detached account and publishes only after commit.
Its inventory refresh now follows form synchronization, so its additional
UpdateEtc cannot precede the class. Rejection or save failure retains both
forms, shortcut arrays and stone amount. Scene ownership, teardown and shutdown
remain unchanged; relogin uses the existing class and shortcut snapshots.

Regression cases cover all sixteen class pairs, four repeated switches, both
pages, local/global skill IDs, empty slots, special skills and account snapshot
reconstruction. Critical commit tests cover detached state and failed saves.
Client tests cover all class mappings, transformed/invalid abilities, owner-only
ordering and removal of the stale UpdateEtc echo. Real-client switching,
selection, casting and relogin acceptance remain pending.

`STATICALLY VERIFIED`: face/catalog class binding, owner-only mutation and
publication order. `AUTOMATED TESTED`: full Go suite, vet and focused race
checks pass. ArchitectureTests passed 3,183,478 checks; socket (281), crash
capture (147) and launcher (58) checks pass. Incremental Release build through
`Build-Client.ps1 -NoDeploy` passes. Logs: `build/celestial-shortcuts/`.
Candidate SHA-256:
`3F14E9759B624782895346DD8E981FA85C18B73F28867A67DC7D9031B36C0E86`.
`CLIENT-TESTED`: pending. No installation or publication occurred. Allocate
the next mandatory client revision and server minimum during release preparation.

## Lacunas

- Falta exercitar no `project.exe` real drag nas duas paginas, eco de atalho
  invalido, compra/reset de skill e logout/relogin.
- A funcao receptora equivalente no binario nativo nao foi isolada nesta
  investigacao; por isso a paridade afirmada cobre o wire emitido, nao um gate
  nativo S->C.

## Validação

- Pesquisa: reutilizada a ficha nativa da selecao de pagina e cruzados todos os
  produtores/consumidores ativos do opcode.
- Automacao: teste C++ cobre tamanho publicado, nulo, todos os prefixos,
  excesso, discriminantes divergentes, entrega unica, vinte bytes e
  imutabilidade; testes Go existentes cobrem layout, tamanho de dispatch e
  filtragem autoritativa.
- Client real: pendente; nao promover a `CLIENT_TESTED` apenas por build.
