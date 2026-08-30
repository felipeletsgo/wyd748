# Ledger de evidência da transição de cenas 7.48

Este ledger versionado registra a rodada Ghidra posterior ao commit `149205b7`. Ele existe
para que export gerado não seja confundido com descoberta interpretada e para
que uma retomada comece na última lacuna, não numa releitura ampla.

## Proveniência e regra de uso

- Executável: `client748/wyd.exe nativo+patches/WYD.exe`.
- SHA-256: `8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593`.
- Projeto: `WYD748Native_20260821.gpr`, Ghidra 12.1.3, consultas read-only com
  `ExportWydFlow.java`.
- Diretório regenerável: `%TEMP%\codex-wyd748-lifecycle-149205b7`.
- Inventário: 47 arquivos TSV, aproximadamente 33,22 MiB; logs irmãos guardam
  as consultas completas. Os binários e scripts históricos não foram alterados
  nem executados.
- Distribuição conferida nas 47 linhas do inventário: 22
  `CONCLUSÃO CONFIRMADA`, 17 `PISTA LOCALIZADA`, 2
  `AINDA NÃO INTERPRETADO` e 6 `LACUNA SEGUINTE`.
- Último recorte interpretado: `scene5-select-enter-focused.tsv`, 1.978.822
  bytes, SHA-256
  `074F55D599977F1A0D3045DEC0B23428FAD8C11A86A64383862673210B8E9906`;
  o TSV contém o hash nativo esperado e nenhum `SCRIPT ERROR`.
- Documento canônico das conclusões:
  `flows/lifecycle/scene-transition.md`, que permanece `LOCATED`.

Classificação usada abaixo:

- `CONCLUSÃO CONFIRMADA`: instruções/xrefs suficientes já foram interpretados e
  a conclusão foi transcrita para a ficha principal.
- `PISTA LOCALIZADA`: o export aponta raízes ou relações úteis, mas ainda não
  fecha a pergunta observável.
- `AINDA NÃO INTERPRETADO`: arquivo gerado e preservado no inventário, sem
  revisão suficiente para produzir claim.
- `LACUNA SEGUINTE`: export diretamente ligado ao próximo xref que precisa ser
  resolvido.

## Conclusões transcritas

1. A matriz correta de `FUN_004B3500` é `0 -> FUN_004343A4`,
   `5 -> FUN_0049EE30`, `7 -> FUN_004A8CCF` e `8 -> FUN_00431D00`.
2. A cena do estado `5` usa vtable `0x005A44B4`; seu deleting destructor é
   `FUN_004A8910`, packet handler `FUN_004A626E`, initializer `FUN_0049F0E7` e
   control/event handler `FUN_004A32DD`.
3. `FUN_00493E70 -> FUN_0040C2CD` cria em `scene+0x28` o container de
   controles, instala o vptr principal `0x005A3F34`, constrói em
   `container+0x24` o receptor `0x005A3F30` e guarda a cena em
   `container+0x2C`.
4. Para o registro tipo `2`, `FUN_004974EC` lê `0x28` bytes, cria um `SButton`
   de `0x208` bytes, guarda o ID em `SButton+0x44` e associa em `SButton+0x5C`
   o receptor `scene[10]+0x24` por `FUN_0040C030`.
5. No release `0x202`, `FUN_004032E8` chama o slot zero do receptor. Esse slot
   é `FUN_0040CDA4`, que lê a cena em `[this+8]`, retorna `0` se ela for nula e
   encaminha ID/ação ao slot virtual `+0x58` da cena no callsite `0x0040CDCA`.
   Para a vtable `0x005A44B4`, o destino é `FUN_004A32DD`.
6. `FUN_0049F0E7` carrega `UI_SelCharScene.txt`/`UI_SelCharScene2.txt`, procura
   o controle `0x1204` no container e o guarda em `scene[0x9B94]`. A cadeia
   concreta fica `SButton release -> FUN_0040CDA4 -> FUN_004A32DD -> 0x1204`.
7. O evento `0x1204` valida índice `0..3`, aplica debounce, monta o packet
   `0x213` de `0x24` bytes e o envia no callsite `0x004A3422`.
8. `FUN_0042550E -> FUN_00424C2C -> FUN_00424DFE -> FUN_00425266` fecha o
   enqueue, cifragem, buffer de `0x20000` bytes e tentativa de flush do `0x213`.
9. `FUN_004B263E`, slot `ObjectManager+0x08`, inicia na raiz ativa
   `DAT_0067CF38` e chama o slot de packet `+0x04` em profundidade até um nó
   retornar `1`. A cena `0` instala `0x005A4294 +0x04 -> FUN_00492E7D`; a cena
   `5` instala `0x005A44B4 +0x04 -> FUN_004A626E`. São overrides alternativos,
   não consumidores sequenciais de `0x114`; na cena 5 o retorno `1` após a troca
   para estado `0` encerra o dispatch sem entregar o mesmo packet à nova raiz.
10. `FUN_004B21C9` grava as flags de deleção e `FUN_004B16C0` é o consumidor que
   chama o deleting destructor e limpa previous scene/flag do manager.
11. Os cleanups específicos das cenas `0/5/7/8` convergem em `FUN_00494C00`, que
   por sua vez converge em `FUN_0054AA45` para filhos e detach da árvore.
12. `FUN_004B1EA9` inicializa o `ObjectManager`; `FUN_004B3A20 -> FUN_004B2155`
   desmonta sua raiz, cena global e opcionalmente o próprio manager.
13. O timer usa vtable `0x005A4688`, é publicado em `DAT_0092E654` e atualiza
   `DAT_0092E658`.
14. O slot `0x005A4294 +0x4C` da cena `0` aponta para `FUN_00435B13` e é
    chamado indiretamente por `FUN_004B3500` em `0x004B370F`. O único retorno
    normal do initializer grava `EAX=1`; por isso o ramo
    `"Initialize Scene Fail."` não é alcançável por retorno falso desse
    override. Falhas não locais não são convertidas em zero e permanecem fora
    dessa conclusão.
15. O slot `0x005A44B4 +0x4C` da cena `5` aponta para `FUN_0049F0E7` e usa o
    mesmo caller indireto. A função possui 2.509 instruções e um único `RET`.
    Se `FUN_00541065` retorna zero, ela registra `"DataFile Not Found  "`, mostra
    `MessageBoxA("DataFile Not Found.", "File Lost", 0)`, agenda `WM_CLOSE` e
    retorna `0`; todos os demais caminhos normais que chegam ao epílogo passam
    por `MOV EAX,1`. Ao receber zero, `FUN_004B3500` destrói a cena parcial,
    mostra `"Initialize Scene Fail."` e agenda um segundo `WM_CLOSE`. A semântica
    específica de `FUN_00541065` continua não confirmada.

## Pistas qualificadas já transcritas

- `FUN_0055EE1E` grava seu argumento em `app+0xF8` e em `DAT_013B71E8`.
- No branch `0x464` de `FUN_0055DAB8`, o dispatch em `0x0055E80D` usa
  `app+0xF8`, enquanto o dispatch em `0x0055E8CA` usa `app+0xF4`.
- Esses dois acessos são fatos de instrução já registrados na ficha, mas a
  classe do segundo receptor, seu ownership, o significado do evento e o
  teardown continuam abertos. Por isso os exports correspondentes permanecem
  `PISTA LOCALIZADA`, sem promoção de maturidade.

## Inventário integral dos exports

| Export TSV | Pergunta da consulta | Classificação | Resultado preservado / próxima decisão |
| --- | --- | --- | --- |
| `app-f4-f8-fc-ownership-focused.tsv` | ownership de `app+0xF4/+0xF8/+0xFC` | `PISTA LOCALIZADA` | raízes de acesso reunidas; atribuição, invalidação e destruição ainda não fechadas |
| `application-entry-lifecycle.tsv` | bootstrap e entrada observável | `PISTA LOCALIZADA` | ancora `FUN_0055BC0A`, `FUN_0055CA18` e `FUN_0055DAB8`; ordem parcial |
| `application-shutdown-focused.tsv` | teardown da aplicação | `LACUNA SEGUINTE` | ancora `FUN_0055D066`; falta ordenar cenas, socket, timer, foco e janela |
| `application-vslots-callers.tsv` | callers dos slots da aplicação | `PISTA LOCALIZADA` | vtable `0x005A6104` localizada; chamadas indiretas ainda incompletas |
| `application-wrapper-finalize-xrefs.tsv` | wrappers/finalizers da aplicação | `PISTA LOCALIZADA` | xrefs coletados; papel de cada wrapper ainda não consolidado |
| `control-container-0040cda4.tsv` | container em `FUN_0040CDA4` | `CONCLUSÃO CONFIRMADA` | receptor em `container+0x24`; cena em `[this+8]`; dispatch à cena pelo slot `+0x58` |
| `control-container-ownership-focused.tsv` | ownership dos containers UI | `PISTA LOCALIZADA` | owner concreto da cena 5 fechado; ownership amplo dos demais containers continua aberto |
| `control-container-vtable-loader-focused.tsv` | vtables/loaders de containers | `CONCLUSÃO CONFIRMADA` | vptrs `0x005A3F34/0x005A3F30`, loader tipo `2` e ID `0x1204` ligados à cena 5 |
| `control-vtables-callers-destructors-focused.tsv` | callers e destrutores de controles | `AINDA NÃO INTERPRETADO` | callback direto de `0x1204` já transcrito por outros exports; destrutores deste corpus ainda não interpretados |
| `control-vtables-constructors-focused.tsv` | construtores de controles | `CONCLUSÃO CONFIRMADA` | `FUN_00493E70 -> FUN_0040C2CD` cria o container e `FUN_004974EC -> FUN_00402F01` cria o `SButton` |
| `dat-013b7220-owners-focused.tsv` | owners de `DAT_013B7220` | `PISTA LOCALIZADA` | muitos acessos reunidos; owner final e relação com logout pendentes |
| `dat-013b7220-xrefs-verified.tsv` | xrefs verificados de `DAT_013B7220` | `PISTA LOCALIZADA` | conjunto focado preservado; sem conclusão de lifecycle |
| `dat-013b7220-xrefs.tsv` | xrefs iniciais de `DAT_013B7220` | `PISTA LOCALIZADA` | duplicata de exploração mantida para rastreabilidade |
| `fieldscene-vtable-005a4294-focused.tsv` | vtable candidata de FieldScene | `PISTA LOCALIZADA` | vtable localizada; slots completos e lifecycle ainda pendentes |
| `lifecycle-flag-1b090-focused.tsv` | significado de `manager+0x1B090` | `PISTA LOCALIZADA` | acessos reunidos; sem semântica final comprovada |
| `lifecycle-roots.tsv` | raízes gerais do lifecycle | `PISTA LOCALIZADA` | mapa amplo para retomada; não é claim por si só |
| `loader-sbutton-branch.tsv` | branch do loader de `SButton` | `CONCLUSÃO CONFIRMADA` | registro tipo `2`, tamanho `0x28`, ID em `+0x44` e receptor em `+0x5C` fechados |
| `logout-relogin-next-roots.tsv` | próximas raízes de logout/relogin | `LACUNA SEGUINTE` | fila de xrefs para reconstrução da sessão/cena |
| `logout-relogin-roots-focused.tsv` | raízes focadas de logout/relogin | `LACUNA SEGUINTE` | transição observável ainda não fechada |
| `logout-ui-vtable-roots.tsv` | vtables UI usadas no logout | `LACUNA SEGUINTE` | candidatos amplos; ordem de teardown pendente |
| `logout-ui-vtable-table-window.tsv` | tabela/vtable/janela no logout | `LACUNA SEGUINTE` | precisa resolver receptores e chamadas indiretas |
| `message-box-vtable-005a379c-focused.tsv` | vtable de message box | `AINDA NÃO INTERPRETADO` | não atribuir papel na seleção sem cadeia de evento |
| `objectmanager-cleanup-004b21c9-focused.tsv` | semântica do slot `+0x64` | `CONCLUSÃO CONFIRMADA` | `scene+0x14=1` e `manager+0x1B08C=1` quando scene existe |
| `objectmanager-destructor-004b3a20-focused.tsv` | destrutor do manager | `CONCLUSÃO CONFIRMADA` | `FUN_004B3A20 -> FUN_004B2155` desmonta raiz e zera cena global |
| `objectmanager-ownership-lifecycle-focused.tsv` | construção/ownership/cleanup | `CONCLUSÃO CONFIRMADA` | `FUN_004B1EA9` inicializa vptr, estado, raiz e ownership; consumidor localizado |
| `objectmanager-vtable-005a45fc-focused.tsv` | slots da vtable efetiva | `CONCLUSÃO CONFIRMADA` | base `0x005A45FC`; slots `+0x54..+0x68` transcritos na ficha |
| `sbutton-methods-next.tsv` | métodos e callbacks de `SButton` | `CONCLUSÃO CONFIRMADA` | release `0x202` em `FUN_004032E8` chama o receptor com ID e ação `0` |
| `sbutton-vtable-event-loader-focused.tsv` | vtable, evento e loader de `SButton` | `CONCLUSÃO CONFIRMADA` | cadeia `SButton -> FUN_0040CDA4 -> scene+0x58 -> FUN_004A32DD` demonstrada |
| `scene-base-cleanup-0054aa45-focused.tsv` | cleanup base e detach | `CONCLUSÃO CONFIRMADA` | destrói filhos, religa pai/irmãos e desanexa o nó |
| `scene-class-lifecycle.tsv` | lifecycle amplo das classes de cena | `PISTA LOCALIZADA` | corpus de 4,57 MiB; consultar pontualmente, não reler por inteiro |
| `scene-cleanup-destructors-focused.tsv` | destrutores das quatro cenas | `CONCLUSÃO CONFIRMADA` | cadeias específicas `0/5/7/8` convergem em `FUN_00494C00` |
| `scene-control-event-handlers-focused.tsv` | handlers virtuais de controles/eventos | `CONCLUSÃO CONFIRMADA` | localiza `FUN_004A32DD`, evento `0x1204` e side effects pós-envio |
| `scene-lifecycle-shutdown-next.tsv` | ligação cenas-shutdown | `LACUNA SEGUINTE` | raízes reunidas; ordem completa continua aberta |
| `scene-owner-unwind-actions-focused.tsv` | ações de unwind dos owners | `CONCLUSÃO CONFIRMADA` | uso em unwind corrobora `FUN_0054AA45` como cleanup base |
| `scene-owner-vtable-eh-focused.tsv` | vtables e exception handlers de owners | `PISTA LOCALIZADA` | candidatos preservados; não generalizar além dos cleanups confirmados |
| `scene-owner-wrapper-funcinfo-focused.tsv` | wrappers e FuncInfo de owner | `PISTA LOCALIZADA` | útil para unwind; wrappers restantes sem semântica fechada |
| `scene-packet-forwarders-focused.tsv` | encaminhamento dos packets da cena | `CONCLUSÃO CONFIRMADA` | fecha enqueue/cifra/flush de `FUN_0042550E` até `FUN_00425266` |
| `scene-teardown-application-focused.tsv` | convergência scene/app teardown | `PISTA LOCALIZADA` | relações coletadas; sequência global não demonstrada |
| `scene-vtables-focused.tsv` | vtables e slots das cenas | `CONCLUSÃO CONFIRMADA` | resolve cena 5 em `0x005A44B4` e apoia as quatro cadeias de cleanup |
| `scene0-initialize-00435b13-focused.tsv` | retorno e caminho de falha do initializer `+0x4C` da cena 0 | `CONCLUSÃO CONFIRMADA` | slot `0x005A4294+0x4C`; caller indireto em `0x004B370F`; único retorno normal grava `EAX=1` |
| `scene5-select-enter-focused.tsv` | seleção/entrada e initializer da cena 5 | `CONCLUSÃO CONFIRMADA` | evento `0x1204`, packet `0x213`, resposta `0x114` e retorno `0/1` de `FUN_0049F0E7` com falha dupla localizados |
| `scene5-vtable-app-timer-focused.tsv` | cena 5, aplicação e timer | `CONCLUSÃO CONFIRMADA` | liga vtable `0x005A44B4` e timer publicado em `DAT_0092E654` |
| `timer-destructor-focused.tsv` | destrutor do timer | `CONCLUSÃO CONFIRMADA` | deleting destructor `FUN_004BAEB0` localizado |
| `timer-vtable-005a4688-focused.tsv` | slots do timer | `CONCLUSÃO CONFIRMADA` | slots `+0x00..+0x10` e atualização de `DAT_0092E658` transcritos |
| `virtual-slot-04-all.tsv` | dispatch virtual de packets | `CONCLUSÃO CONFIRMADA` | 116 hits; `FUN_004B263E` fornece o receptor `DAT_0067CF38` e separa os overrides das cenas 0 e 5 |
| `virtual-slot-58-all.tsv` | todos os usos de slot virtual `+0x58` | `PISTA LOCALIZADA` | conjunto amplo; filtrar por receptor antes de atribuir semântica |
| `wm-close-sources-focused.tsv` | origens de fechamento da janela | `PISTA LOCALIZADA` | apoia caminhos de falha/close; shutdown completo continua aberto |

## Ponto exato de retomada

Usar o ciclo aprovado `HEAD/status/hash -> última evidência -> próximo xref`.
Não reler os exports já inventariados. A próxima unidade é:

```text
1 xref: resolver no scene-vtables-focused.tsv os slots +0x4C das cenas 7/8
1 conclusão: fechar retorno e falha do primeiro initializer restante
1 escrita: atualizar este ledger e scene-transition.md no mesmo ciclo
```

Depois do segundo initializer, fechar a ordem de teardown. Shutdown e
logout/relogin permanecem as lacunas seguintes. Nenhuma edição em
`client-source/` é permitida enquanto a ficha estiver `LOCATED`.
