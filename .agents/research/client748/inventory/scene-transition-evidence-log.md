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
- Inventário: 54 arquivos TSV, aproximadamente 34,87 MiB; 36 logs irmãos
  guardam as consultas completas. Os binários e scripts históricos não foram
  alterados nem executados.
- Distribuição conferida nas 54 linhas do inventário: 29
  `CONCLUSÃO CONFIRMADA`, 17 `PISTA LOCALIZADA`, 1
  `AINDA NÃO INTERPRETADO` e 7 `LACUNA SEGUINTE`.
- Últimos recortes interpretados: `fun-00423c61-shutdown-focused.tsv`, SHA-256
  `5799FB5705F68DBB188B53D2989E635DBD1C146AA559161696F844B438676991`, e
  `windows-hook-lifecycle-focused.tsv`, SHA-256
  `ACC5288A10463BABD0FA2DD83058F78BB2B65130A52D1AA90169B0397EA97D43`.
  Eles fecham o lifecycle do hook global de teclado entre o bootstrap e o ramo
  `WM_CLOSE`, inclusive callback, globals, falha parcial e finalizador.
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
16. `FUN_004A8CCF` instala o vptr `0x005A4544` da cena `7`; seu slot `+0x4C`,
    armazenado em `0x005A4590`, aponta para `FUN_004A8F14` e usa o mesmo caller
    indireto. A função possui 1.626 instruções e um único `RET`. Se
    `FUN_00541065` retorna zero, ela registra `"DataFile Not Found  "`, mostra
    `MessageBoxA("DataFile Not Found.", "File Lost", 0)`, agenda `WM_CLOSE`,
    evita o `MOV EAX,1` do epílogo e retorna `0`; todos os demais caminhos
    normais retornam `1`. O caller genérico repete destruição parcial,
    diagnóstico e `WM_CLOSE`. A semântica específica do helper não foi
    atribuída.
17. `FUN_00431D00` instala o vptr `0x005A421C` da cena `8`; seu slot `+0x4C`,
    armazenado em `0x005A4268`, aponta para `FUN_00432181` e usa o caller
    indireto `FUN_004B3500:0x004B370F`. A função possui 431 instruções e um
    único `RET`. Se `FUN_00541065` retorna zero, ela registra
    `"DataFile Not Found  "`, mostra
    `MessageBoxA("DataFile Not Found.", "File Lost", 0)`, agenda `WM_CLOSE`,
    evita o `MOV EAX,1` do epílogo e retorna `0`; todos os demais caminhos
    normais retornam `1`. O caller genérico repete destruição parcial,
    diagnóstico e `WM_CLOSE`. A semântica específica do helper não foi
    atribuída. Assim, os initializers `0/5/7/8` estão fechados no CFG normal.
18. `FUN_004B16C0` captura `DAT_0067CF38` como raiz/sentinela, percorre apenas
    essa subárvore e interrompe a subida antes de seguir irmãos da raiz. Depois
    processa separadamente `manager+0x1B088`. Quando o deleting destructor da
    cena anterior alcança `FUN_00494C00`, esse campo é zerado antes da reentrada
    em `FUN_004B16C0`; portanto a cena antiga não pode ser selecionada novamente
    por esse caminho. `FUN_0054AA45` efetua o detach depois da coleta reentrante.
19. `FUN_0054A9E0` inicializa a raiz com vptr `0x005A5CFC`; o slot zero é
    `FUN_0054AE10 -> FUN_0054AA45`, com liberação condicional a `flag & 1`.
    `FUN_00401152` não é esse deleting destructor: ele instala `0x005A3420`,
    cujo slot zero é `FUN_0040B980 -> FUN_00401152`. `FUN_0054AC09` insere o
    filho no início da lista intrusiva e atualiza parent, primeiro filho e
    irmãos anterior/seguinte.
20. Depois do tick, `FUN_0055D345` testa `manager+0x1B08C`, carrega
    `ECX = app+0xF8` e chama `FUN_004B16C0` em `0x0055D6C5`. Esse é o caller
    periódico confirmado do coletor de cenas marcadas.
21. `FUN_0055D066` desmonta, na ordem observada, três recursos em 100 registros,
    `DAT_013B71E8`, `app+0xF4`, `+0xE0`, `+0xE4`, `+0xFC`, `+0xEC`, `+0xE8`,
    `+0xF0`, zera `DAT_013B71E4`, chama `DeleteObject(app+0x110)` e, quando
    presente, `FUN_0059DFE0(DAT_013B7364)`. A ordem de instruções está
    preservada; classes/ownership continuam abertas e o caller efetivo foi
    fechado posteriormente na conclusão 26.
22. `FUN_0055F7F9` aloca `0x114` bytes, chama `FUN_0055B18F`, executa o slot
    `+0x04` de bootstrap e, após sucesso, o slot `+0x0C` do loop principal.
    Quando o loop retorna, chama o slot `+0x00` com flag `1`; não há chamada
    direta ao slot `+0x08/FUN_0055D066` nesse wrapper porque ela ocorre antes,
    dentro do window procedure. Falhas anteriores ou do bootstrap retornam sem
    cleanup explícito no CFG, o que não basta para afirmar leak fora desse
    recorte.
23. `FUN_0055FBB9:0x0055FC85` chama `+0x08` sobre `local_8`, vindo do slot
    `+0x18` de `param_1`, sem ligação de dados com `DAT_013B71E0`.
24. Os hits `+0x08` de `FUN_0055D345` em `0x0055D415/0x0055D4DF` recebem
    `app+0xF4/app+0xF8`; `FUN_0055EE59:0x0055EFF7` recebe o timer global
    `DAT_0092E654`, de vtable `0x005A4688`. Os três são candidatos rejeitados
    ao caller de `FUN_0055D066`: o mesmo offset virtual não prova a classe.
25. `FUN_0055F3E0 -> FUN_0055B26F` reinstala a vtable da aplicação, chama
    `FUN_004312DD` para fechar/zerar o handle global de logging
    `DAT_005CCFAC` e libera o bloco somente quando `flag & 1`. Depois,
    `FUN_0055F7F9 -> FUN_004B428E` desmonta a camada de DLL/callback, buffers,
    critical section e filtro de exceção. Nenhuma dessas funções chama
    `FUN_0055D066`; são finalizadores distintos e posteriores ao shutdown.
26. No ramo `WM_CLOSE` de `FUN_0055DAB8`, `0x0055EB9C` carrega o vptr e o
    receptor do próprio `param_1`. Como `FUN_0055FA89` fornece
    `DAT_013B71E0` e a vtable `0x005A6104` resolve `+0x08` para
    `FUN_0055D066`, a cadeia efetiva é `WM_CLOSE -> app vslot +0x08 ->
    FUN_0055D066`. Antes do callsite, `0x0055EB5A/0x0055EB74` obtêm
    `app+0xF4` e chamam o slot `+0x08` desse objeto; `0x0055EB77..0x0055EB85`
    comparam o retorno com `DAT_013B7220 + 0xBB8` e só prosseguem quando o
    limite de 3000 unidades é atingido. A unidade não foi atribuída. Depois do
    retorno, o ramo testa `DAT_013B71F0`, chama `FUN_0058F75F` quando positivo,
    zera o global, grava `DAT_013B7228 = 1`, chama `FUN_00423C61`,
    `DestroyWindow`, `PostQuitMessage(0)` e zera `DAT_013B7220`. O shutdown
    ocorre dentro do window procedure antes do retorno do loop e do deleting
    destructor em `FUN_0055F7F9`.
27. `FUN_00423C61` possui um único caller em `FUN_0055DAB8:0x0055EBCB`. Se
    `DAT_005CCF84 == 0`, retorna; caso contrário, chama `FUN_00423B25`, remove
    `DAT_005CCF80` com `UnhookWindowsHookEx` quando o helper retorna zero e
    sempre termina com `DAT_005CCF84 = 0`. Isso fecha o finalizador entre
    `FUN_0055D066` e `DestroyWindow`.
28. `FUN_0055F7F9:0x0055FA08 -> FUN_00423C1F` instala, depois do bootstrap e
    antes do loop, um `WH_KEYBOARD_LL` global com callback `FUN_00423B74`.
    O instalador é idempotente pelo flag `DAT_005CCF84`, grava o handle em
    `DAT_005CCF80` e marca o flag mesmo quando `SetWindowsHookExA` retorna
    nulo. `FUN_00423B25` separa `dwPlatformId == 1`; nessa plataforma as APIs
    de instalação/remoção são puladas, mas o flag lógico transiciona. O callback
    bloqueia `Ctrl+Esc`, `Alt+Esc`, `0x5B`, `0x5C` e `0x5D`, encaminhando os
    demais casos por `CallNextHookEx`. Os xrefs dos dois globals estão fechados.
29. O slot `+0x14` da vtable da aplicação `0x005A6104` resolve para
    `FUN_0055D6E6`. `FUN_0055D345:0x0055D5BC` chama esse slot usando a própria
    aplicação como receptor quando `app+0xE8 != 0` e
    `[app+0xE8]+0x10C != 2`. A função usa `app+0xE0`, `app+0xF8`,
    `DAT_0067CF38`, `FUN_0040C0F0`, `FUN_00494DCF`, `FUN_004AFFF7`,
    `FUN_004B7F0D` e `__ftol`, caracterizando uma etapa de frame/render. Em
    `0x0055D8F7`, ela chama o slot `+0x48` do objeto em `scene+0x28`; não chama
    estruturalmente os slots de cena `+0x14` ou `+0x20`.

## Pistas qualificadas já transcritas

- `FUN_0055EE1E` grava seu argumento em `app+0xF8` e em `DAT_013B71E8`.
- No branch `0x464` de `FUN_0055DAB8`, o dispatch em `0x0055E80D` usa
  `app+0xF8`, enquanto o dispatch em `0x0055E8CA` usa `app+0xF4`.
- Esses dois acessos são fatos de instrução já registrados na ficha, mas a
  classe do segundo receptor, seu ownership, o significado do evento e o
  teardown continuam abertos. Por isso os exports correspondentes permanecem
  `PISTA LOCALIZADA`, sem promoção de maturidade.
- O loop `FUN_0055D345`, a sequência interna de `FUN_0055D066`, seu caller
  pelo ramo `WM_CLOSE` e o lifecycle do hook foram interpretados até os limites
  descritos nas conclusões 20, 21 e 26–28. A identidade e o ownership dos
  campos desmontados, as fontes de `WM_CLOSE` e a convergência integral do
  teardown continuam abertos.

## Inventário integral dos exports

| Export TSV | Pergunta da consulta | Classificação | Resultado preservado / próxima decisão |
| --- | --- | --- | --- |
| `app-f4-f8-fc-ownership-focused.tsv` | ownership de `app+0xF4/+0xF8/+0xFC` | `PISTA LOCALIZADA` | raízes de acesso reunidas; atribuição, invalidação e destruição ainda não fechadas |
| `application-entry-lifecycle.tsv` | bootstrap e entrada observável | `PISTA LOCALIZADA` | ancora `FUN_0055BC0A`, `FUN_0055CA18` e `FUN_0055DAB8`; ordem parcial |
| `application-frame-0055d6e6-focused.tsv` | estágio de frame/render da aplicação | `CONCLUSÃO CONFIRMADA` | `FUN_0055D345:0x0055D5BC` chama `app+0x14/FUN_0055D6E6`; a função usa cena/UI/render e chama `scene+0x28` slot `+0x48`, não os slots de cena `+0x14/+0x20` |
| `application-shutdown-focused.tsv` | teardown da aplicação | `LACUNA SEGUINTE` | ordem interna de `FUN_0055D066` transcrita; caller fechado em export posterior; faltam classes/ownership e convergência com socket, timer, foco e janela |
| `application-vslots-callers.tsv` | callers dos slots da aplicação | `PISTA LOCALIZADA` | wrapper `FUN_0055F7F9` fechado para bootstrap, loop e deleting destructor; hits `+0x08` de `FUN_0055D345/FUN_0055EE59` rejeitados pelo receptor; caller correto foi fechado depois no ramo `WM_CLOSE` |
| `application-wrapper-finalize-xrefs.tsv` | wrappers/finalizers da aplicação | `PISTA LOCALIZADA` | `FUN_0055FA89` fecha o thunk do dispatcher; `FUN_0055FBB9:0x0055FC85` é rejeitado; deleting destructor fecha logging e `FUN_004B428E` fecha DLL/exceção, ambos sem chamar o shutdown |
| `control-container-0040cda4.tsv` | container em `FUN_0040CDA4` | `CONCLUSÃO CONFIRMADA` | receptor em `container+0x24`; cena em `[this+8]`; dispatch à cena pelo slot `+0x58` |
| `control-container-ownership-focused.tsv` | ownership dos containers UI | `PISTA LOCALIZADA` | owner concreto da cena 5 fechado; ownership amplo dos demais containers continua aberto |
| `control-container-vtable-loader-focused.tsv` | vtables/loaders de containers | `CONCLUSÃO CONFIRMADA` | vptrs `0x005A3F34/0x005A3F30`, loader tipo `2` e ID `0x1204` ligados à cena 5 |
| `control-vtables-callers-destructors-focused.tsv` | callers e destrutores de controles | `CONCLUSÃO CONFIRMADA` | distingue `0x005A5CFC -> FUN_0054AE10` de `0x005A3420 -> FUN_0040B980`; corrige a atribuição de `FUN_00401152` |
| `control-vtables-constructors-focused.tsv` | construtores de controles | `CONCLUSÃO CONFIRMADA` | `FUN_00493E70 -> FUN_0040C2CD` cria o container e `FUN_004974EC -> FUN_00402F01` cria o `SButton` |
| `dat-013b7220-owners-focused.tsv` | owners de `DAT_013B7220` | `PISTA LOCALIZADA` | muitos acessos reunidos; owner final e relação com logout pendentes |
| `dat-013b7220-xrefs-verified.tsv` | xrefs verificados de `DAT_013B7220` | `PISTA LOCALIZADA` | conjunto focado preservado; sem conclusão de lifecycle |
| `dat-013b7220-xrefs.tsv` | xrefs iniciais de `DAT_013B7220` | `PISTA LOCALIZADA` | duplicata de exploração mantida para rastreabilidade |
| `fieldscene-vtable-005a4294-focused.tsv` | vtable candidata de FieldScene | `PISTA LOCALIZADA` | vtable localizada; slots completos e lifecycle ainda pendentes |
| `fun-00423c61-shutdown-focused.tsv` | função entre shutdown e `DestroyWindow` | `CONCLUSÃO CONFIRMADA` | único caller em `0x0055EBCB`; remove o hook quando aplicável e sempre zera `DAT_005CCF84` |
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
| `scene-base-cleanup-0054aa45-focused.tsv` | cleanup base e detach | `CONCLUSÃO CONFIRMADA` | destrói filhos, religa pai/irmãos, desanexa o nó e fecha o deleting destructor da raiz em `0x005A5CFC` |
| `scene-class-lifecycle.tsv` | lifecycle amplo das classes de cena | `PISTA LOCALIZADA` | corpus de 4,57 MiB; consultar pontualmente, não reler por inteiro |
| `scene-cleanup-destructors-focused.tsv` | destrutores das quatro cenas | `CONCLUSÃO CONFIRMADA` | cadeias `0/5/7/8` convergem em `FUN_00494C00`; zero de `+0x1B088` antes da reentrada impede dupla seleção da cena antiga |
| `scene-control-event-handlers-focused.tsv` | handlers virtuais de controles/eventos | `CONCLUSÃO CONFIRMADA` | localiza `FUN_004A32DD`, evento `0x1204` e side effects pós-envio |
| `scene-lifecycle-shutdown-next.tsv` | ligação cenas-shutdown | `LACUNA SEGUINTE` | fecha `FUN_0055D345 -> FUN_004B16C0` após o tick; caller de `FUN_0055D066` foi fechado depois, mas a ordem global continua aberta |
| `scene-owner-unwind-actions-focused.tsv` | ações de unwind dos owners | `CONCLUSÃO CONFIRMADA` | uso em unwind corrobora `FUN_0054AA45` como cleanup base |
| `scene-owner-vtable-eh-focused.tsv` | vtables e exception handlers de owners | `PISTA LOCALIZADA` | candidatos preservados; não generalizar além dos cleanups confirmados |
| `scene-owner-wrapper-funcinfo-focused.tsv` | wrappers e FuncInfo de owner | `PISTA LOCALIZADA` | útil para unwind; wrappers restantes sem semântica fechada |
| `scene-packet-forwarders-focused.tsv` | encaminhamento dos packets da cena | `CONCLUSÃO CONFIRMADA` | fecha enqueue/cifra/flush de `FUN_0042550E` até `FUN_00425266` |
| `scene-teardown-application-focused.tsv` | convergência scene/app teardown | `PISTA LOCALIZADA` | relações coletadas; sequência global não demonstrada |
| `scene-vtables-focused.tsv` | vtables e slots das cenas | `CONCLUSÃO CONFIRMADA` | resolve cena 5 em `0x005A44B4` e apoia as quatro cadeias de cleanup |
| `scene0-initialize-00435b13-focused.tsv` | retorno e caminho de falha do initializer `+0x4C` da cena 0 | `CONCLUSÃO CONFIRMADA` | slot `0x005A4294+0x4C`; caller indireto em `0x004B370F`; único retorno normal grava `EAX=1` |
| `scene5-select-enter-focused.tsv` | seleção/entrada e initializer da cena 5 | `CONCLUSÃO CONFIRMADA` | evento `0x1204`, packet `0x213`, resposta `0x114` e retorno `0/1` de `FUN_0049F0E7` com falha dupla localizados |
| `scene5-vtable-app-timer-focused.tsv` | cena 5, aplicação e timer | `CONCLUSÃO CONFIRMADA` | liga vtable `0x005A44B4` e timer publicado em `DAT_0092E654` |
| `scene7-initialize-004a8f14-focused.tsv` | retorno e caminho de falha do initializer `+0x4C` da cena 7 | `CONCLUSÃO CONFIRMADA` | vptr `0x005A4544`; slot em `0x005A4590`; retorno `0/1` de `FUN_004A8F14` e falha dupla localizados |
| `scene8-initialize-00432181-focused.tsv` | retorno e caminho de falha do initializer `+0x4C` da cena 8 | `CONCLUSÃO CONFIRMADA` | vptr `0x005A421C`; slot em `0x005A4268`; retorno `0/1` de `FUN_00432181` e falha dupla localizados |
| `timer-destructor-focused.tsv` | destrutor do timer | `CONCLUSÃO CONFIRMADA` | deleting destructor `FUN_004BAEB0` localizado |
| `timer-vtable-005a4688-focused.tsv` | slots do timer | `CONCLUSÃO CONFIRMADA` | slots `+0x00..+0x10` e atualização de `DAT_0092E658` transcritos |
| `virtual-slot-04-all.tsv` | dispatch virtual de packets | `CONCLUSÃO CONFIRMADA` | 116 hits; `FUN_004B263E` fornece o receptor `DAT_0067CF38` e separa os overrides das cenas 0 e 5 |
| `virtual-slot-58-all.tsv` | todos os usos de slot virtual `+0x58` | `PISTA LOCALIZADA` | conjunto amplo; filtrar por receptor antes de atribuir semântica |
| `virtual-slots-14-20-all.tsv` | todos os usos estruturais dos slots `+0x14/+0x20` | `LACUNA SEGUINTE` | 34 e 53 hits após 8.173 candidatos/441.614 instruções por scan; `FUN_004B29B9:0x004B29F3` e `FUN_004B2D35:0x004B2D86` são candidatos próximos do manager, ainda sem classe ou semântica atribuída |
| `windows-hook-lifecycle-focused.tsv` | instalação, callback, globals e teardown do hook | `CONCLUSÃO CONFIRMADA` | fecha `FUN_00423C1F/FUN_00423B74/FUN_00423C61`, inclusive o flag marcado após falha da API |
| `wm-close-dispatch-instructions.tsv` | dispatch e ordem do ramo `WM_CLOSE` | `CONCLUSÃO CONFIRMADA` | `0x0055EB9C` chama `app+0x08/FUN_0055D066`; depois seguem `FUN_00423C61`, `DestroyWindow`, `PostQuitMessage(0)` e zero de `DAT_013B7220` |
| `wm-close-sources-focused.tsv` | origens de fechamento da janela | `PISTA LOCALIZADA` | apoia caminhos de falha/close; shutdown completo continua aberto |

## Ponto exato de retomada

Usar o ciclo aprovado `HEAD/status/hash -> última evidência -> próximo xref`.
Não reler os exports já inventariados. A próxima unidade é:

```text
1 xref: abrir FUN_004B29B9 e FUN_004B2D35 com callers, bodyrefs e instruções
1 conclusão: resolver receptor, vptr e slot efetivo dos dois traversals
1 escrita: atualizar este ledger e scene-transition.md no mesmo ciclo
```

`FUN_0055D6E6` já está fechado como etapa de frame/render e não deve ser
reinvestigado. A varredura global prova apenas o uso estrutural dos offsets:
não nomear `+0x14/+0x20` como eventos ou frame sem resolver o receptor. Após os
dois traversals próximos do `ObjectManager`, avançar para
`logout-relogin-next-roots.tsv` e fechar a reconstrução da sessão/cena.
Nenhuma edição em `client-source/` é permitida enquanto a ficha estiver
`LOCATED`.
