---
id: ui-select-character-delete-password
title: Confirmacao e senha na exclusao de personagem
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-22
---

# Confirmacao e senha na exclusao de personagem

## Pergunta

O painel de senha pertence ao 7.48 e quais controles levam da confirmacao de
exclusao ao envio do pedido `0x211`?

## Fronteira de evidência

- UTILIZADA: `references/client748/WYD.exe`, SHA-256 acima; disassembly x86
  read-only das funcoes `FUN_0049F0E7` e `FUN_004A32DD` via dumpbin.
- UTILIZADA: projeto Ghidra `WYD748Native_20260821.gpr`, exports ja versionados
  `exports/select-character-transfer-bindings.tsv` e
  `exports/select-character-transfer-focused.tsv`: vtable `0x005A44B4`,
  initializer pelo slot `0x005A4500` e handler de controles pelo slot
  `0x005A450C`; o handler chama o emissor `FUN_0042550E`.
- UTILIZADA: `tmproject/client748/UI/SelCharScene2.bin`, SHA-256
  `3ED90543644E3AF96EEC99C16FDE36DCA21BE91632EFEACE31A92065BD6F1E3B`;
  leitura conforme `internal/ui/UIBinary.h`: 85 registros, EOF em 4820 bytes.
- UTILIZADA: `TMSelectCharScene.cpp`, `Basedef.h`, `SControl.cpp` e
  `wydgo748/internal/game/character_session.go`, `security.go`.
- CONTRADITORIA: comentario anterior da source afirmava que o painel 626 era
  posterior ao 7.48; binario, RC e handler nativo mostram o oposto.
- NAO APLICAVEL: TMProject 7.69 como autoridade para IDs do recurso 7.48.

## Fluxo nativo 7.48

### Entrada observável

Botao de exclusao 4615, com slot valido, abre a mensagem de confirmacao. Ao
aceitar, o handler mostra o painel de senha e foca o edit. Confirmar no painel
envia `0x211`; cancelar fecha sem enviar.

### Callers

Os slots Ghidra `0x005A4500` e `0x005A450C` da vtable da cena de selecao
chamam, respectivamente, `FUN_0049F0E7` e `FUN_004A32DD`. O dispatcher de
controles entra em `FUN_004A32DD` para os IDs abaixo.

### Função principal

- `FUN_0049F0E7 @ 0x0049F45D/0x0049F485`: encontra edit 627 (`0x273`) e
  painel 626 (`0x272`), guarda-os em `scene+0x26E90`/`scene+0x26E8C`, oculta
  o painel, ativa modal e cifragem visual do edit.
- `FUN_004A32DD @ 0x004A386C`: o botao 4615 abre confirmacao.
- `FUN_004A32DD @ 0x004A42E3..0x004A4350`: confirmacao aceita somente com
  slot 0..3 mostra `scene+0x26E8C` e foca `scene+0x26E90`.
- `FUN_004A32DD @ 0x004A412A..0x004A423F`: controle 1024 (`0x400`) envia
  pacote de 44 bytes, limpa o edit, esconde o painel e devolve o foco.
- `FUN_004A32DD @ 0x004A4244..0x004A4285`: controle 921 (`0x399`) fecha
  painel e devolve o foco sem enviar.

### Callees

O handler usa o texto do edit pelo slot virtual `+0x88`, visibilidade do
painel por `+0x60`, foco do container por `+0x40` e envia via
`FUN_0042550E` com tamanho `0x2C`. A confirmacao usa o mesmo receptor de
painel ligado pelo initializer; 629 (`0x275`) pertence ao rename, nao ao
cancelamento da senha.

### Saídas e erros

Cancelar nao emite packet. A senha errada e rejeitada pelo servidor, sem
excluir o personagem. O handler nativo limpa quatro bytes do buffer do edit;
o candidato limpa o buffer inteiro como modernizacao interna compativel.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| Iniciar cena | RC carregado | `FUN_0049F0E7` | painel 626 oculto/modal, edit 627 ligado | nenhum packet | recurso ausente nao autoriza dereferencia |
| Excluir | slot 0..3 | 4615, confirmacao | mensagem visivel | nenhum packet | negar fecha mensagem |
| Aceitar | mensagem 4615 | handler de confirmacao | painel 626 visivel, foco 627 | nenhum packet | slot invalido nao abre |
| Confirmar senha | painel aberto | controle 1024 | painel fechado, foco liberado | `0x211/44` | servidor valida senha e slot |
| Cancelar senha | painel aberto | controle 921 | painel fechado, foco liberado | nenhum packet | N/A |

### Vtables, vptrs e receptores

`0x005A44B4` e a vtable da cena; os slots acima foram exportados do Ghidra.
O initializer resolve os controles no container da propria cena; os acessos
`scene+0x26E8C/0x26E90` no handler alcancam os ponteiros assim guardados.

### Ownership

O container da cena possui os controles do RC. O handler apenas guarda
ponteiros para painel/edit; nao aloca nem transfere ownership.

### Falha parcial

Se binding ou slot estiver invalido, o candidato nao envia nem desreferencia.
Senha que exceda os 12 bytes do campo wire nao e copiada para o packet.

### Cleanup e teardown

Confirmar/cancelar oculta o painel e libera foco; o candidato apaga todos os
256 bytes do edit. O teardown da cena usa o container existente; nao foi
alterado por este delta.

### Shutdown

N/A: nenhuma thread ou alocacao independente e criada pelo modal.

### Logout e relogin

Ao recriar a cena, o initializer liga novamente os controles e inicia o modal
oculto. O fluxo real de logout/relogin ainda precisa de teste no candidato.

## Wire, ABI e recursos

- C->S `0x211`, 44 bytes, `MSG_STANDARD` de 12 bytes; `Slot` int32 em +12,
  `MobName[16]` em +16 e `Password[12]` em +32. Layout protegido por
  `static_assert` em `Basedef.h` e aceito pelo gate exato do WYD-Go.
- RC: painel 626 root (offset 4400); filhos confirmar 1024 (4444), cancelar
  921 (4488), texto 630 (4532), edit 627 (4588) e painel 631 (4776).
  O edit tem `nMaxStringLength=10` e `nPassword=1`.
- Reproducao: ler registros de `SelCharScene2.bin` com os tamanhos de
  `UIBinary.h`, comparar IDs/parent, e inspecionar os enderecos nativos acima.

## Mapeamento atual

### Source recompilável

`TMSelectCharScene::Init` agora encontra 626/627, registra o modal e o
posiciona. `OnControlEvent` trata 1024/921, valida slot e limite de senha,
monta o packet e limpa o edit. O bloqueio anterior tratava incorretamente 626
como painel posterior e ligava 65885/65889/65886/65887, IDs ausentes do RC.

### WYD-Go

`World.onDeleteCharacter` aceita apenas selecao de personagem e 44 bytes,
valida slot, nome e senha da conta, persiste a exclusao e devolve atualizacao
dos quatro slots. `security.go` exige o tamanho exato para `0x211`.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| Painel de senha | 626/627 no RC e no initializer | ocultava 626 como posterior; procurava 65885/65889 | IDs de versao posterior | nao interfere na UI | PARIDADE_NATIVA: ligar 626/627 |
| Acoes do modal | 1024 envia, 921 cancela | esperava 65886/65887 | fluxo posterior | recebe `0x211` | PARIDADE_NATIVA: rotear IDs reais |
| Higiene de senha | limpa quatro bytes | limpava quatro bytes | herdado | valida hash | MODERNIZACAO_COMPATIVEL: limpar buffer inteiro |

## Decisões

Portar somente IDs e lifecycle demonstrados pelo nativo. Manter o wire de
44 bytes e o servidor autoritativo sem alteracao. Nao remover a UI posterior
nao relacionada apenas por estar ausente no RC.

## Lacunas

O clique real, a rejeicao visual de senha errada, a resposta de exclusao e o
retorno apos logout ainda nao foram exercitados no candidato recompilado.

## Validação

- Pesquisa: disassembly nativo, exports Ghidra e parse RC convergentes.
- Automação: build Release e 51.715 checks C++ passaram; Go executou testes
  de exclusao/senha errada e tamanho de packet. SHA-256 do candidato:
  `BB7C0D0F94AB029FA0A0A0AEE95D0C7B2F7982853ECAB7327B2052C260BA8B17`.
- Client real: pendente; esta ficha nao e `CLIENT_TESTED`.
