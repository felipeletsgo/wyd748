# Handoff: programa de pesquisa do client 7.48

Atualizado em: 2026-08-31
Estado geral: `STATICALLY VERIFIED`

## Objetivo e limites

Transformar a descompilação do client nativo WYD 7.48 em conhecimento
reproduzível para claims de paridade e fronteiras legadas, sem bloquear
modernizações internas ou extensões coordenadas do WYD-Go e de
`client-source/tmproject`. O programa separa fatos do 7.48, estrutura posterior
compatível e contratos novos deliberadamente implementados nos dois lados.

Este handoff cobre a infraestrutura de pesquisa, as fichas do programa e os
contratos nativos aplicados à source recompilável.
A paridade funcional e visual já implementada possui estado próprio em
`.agents/handoffs/client748-parity.md`; não duplicar esse histórico aqui.

O commit `60c57760` preservou a migração nativa de servidor/canal. Esta unidade
também restaura na source o dispatch 7.48 dos controles System `633..635` para
seleção de servidor, logout de personagem e saída. Nenhum código do servidor
foi alterado; a validação permanece estática e não houve teste in-game.

## Estratégia formalizada nesta etapa

O fluxo de trabalho agora é uma regra do repositório:

```text
catálogo -> callgraph -> fluxo observável -> adaptação -> validação
```

`wyd-client748-catalog` mantém o censo das 4.146 funções e entrega uma fila
reproduzível. `wyd-client748-research` fecha claims nativos no projeto Ghidra.
`wyd-go-feature` classifica cada delta como `PARIDADE_NATIVA`,
`MODERNIZACAO_COMPATIVEL` ou `EXTENSAO_COORDENADA`: paridade depende da ficha
`TRACED`/`CONTRACT`; modernização prova somente as fronteiras preservadas;
extensão especifica e testa o contrato client/server novo sem fabricar um
equivalente nativo.

Na migração, partir preferencialmente da função ou feature viva da source,
localizar a candidata nativa por seed/fingerprint/string/vtable e seguir apenas
os vizinhos do callgraph que decidem o delta. Reutilizar hash, triagem, exports,
fichas e validações quando seus inputs não mudaram. Código e assets manuais são
presumidos intencionais; ausência no nativo 7.48 não autoriza remoção.

## Fontes e artefatos

```text
client748/wyd.exe nativo+patches/WYD.exe | referência histórica Ghidra | 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
client748/project.exe                    | candidato source volátil    | 746A2913FA62DD56892319BD136CD15810A4B74092AD75F354E60C6B3FFC5BBD
%USERPROFILE%\Tools\GhidraProjects\WYD748Native_20260821.gpr | projeto Ghidra | descobrir no perfil
%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled       | corpus auxiliar | 4.146 funções
```

O hash nativo foi recalculado em 2026-08-28 e o de `project.exe` após o build
de 2026-08-31. O candidato deve ser recalculado após qualquer novo build. O
corpus textual acelera buscas, mas não substitui xrefs, chamadas indiretas,
tipos, stack ou lifecycle no projeto Ghidra.

## Evidência confirmada

- `wyd-client748-research` é o gate anterior a `wyd-go-feature` para client,
  protocolo, wire/ABI, UI, input, render, assets e lifecycle. A skill foi
  validada em 2026-08-28 pelo `quick_validate.py` do `skill-creator`.
- `wyd-client748-catalog` é a trilha anterior de censo e priorização. O
  triador validou as 4.146 entradas e produziu as lanes
  `CORPUS_TRIAGE=1831`, `DOCUMENTED_FIRST=62`, `HIGH_FANOUT=304` e
  `INDIRECT_OR_CALLBACK=1949`; isso é prioridade, não compreensão.
- O triador foi corrigido e validado nos formatos `summary`, `json` e `tsv`.
  Com `--top 5`, o JSON mantém `functions=4146` e informa
  `selected_functions=5`; o TSV completo produz 4.147 linhas com cabeçalho.
- `wyd-client748-research` e `wyd-go-feature` separam estados de pesquisa
  (`UNMAPPED`/`LOCATED`/`TRACED`/`CONTRACT`) de entrega
  (`IMPLEMENTED`/`CLIENT_TESTED`). O catálogo é usado ao abrir raiz sem fila
  válida ou quando seus inputs mudam. As três skills passaram no
  `quick_validate.py`.
- `LOCATED` bloqueia somente edição dependente do claim nativo incompleto.
  Paridade comportamental exige ficha `TRACED`; packet, wire, ABI, struct,
  offset, packing, signedness e loader legados exigem `CONTRACT`. Extensão
  independente exige contrato client/server e testes dos dois lados.
- `transport/packet-size-gate.md` e
  `ui/control-focus-ime-lifecycle.md` passaram no validador estrutural, mas
  permanecem `LOCATED` porque callers/callees indiretos, erros e lifecycle não
  foram completamente fechados no projeto Ghidra.
- `query_corpus.py stats --repo .` encontrou 4.146 funções e 108 referências
  nativas no repositório: 106 resolvidas no índice textual e 2 ausentes.
  `FUN_00452733` e `FUN_0047E4D6` permanecem não confirmadas; essa ausência não
  prova ausência no binário.
- `FUN_004AFAC0` e `FUN_004AFBA0` instalam o vptr efetivo `0x005A45FC`,
  confirmando a instância nativa do `ObjectManager`. Nessa vtable, os slots
  `+0x54`, `+0x58` e `+0x5C` apontam para `FUN_004B3500`, `FUN_004B37C9` e
  `FUN_004B3952`; os slots `+0x60`, `+0x64` e `+0x68` apontam para
  `FUN_004B21F0`, `FUN_004B21C9` e `FUN_004B27DA`. O endereço `0x005A45F0`
  pertence ao preâmbulo adjacente e não é a base dos slots desta classe.
- `FUN_004B3500` localiza a transição de estado/cena: o estado solicitado `9`
  vira estado interno `0`; somente `0`, `5`, `7` e `8` instanciam cenas; o
  client reinicializa a câmera, registra a nova cena, inicializa-a pelo slot
  virtual `+0x4C`, destrói-a e fecha a janela em falha, ou a anexa à árvore em
  `manager+0x1B07C` por `FUN_0054AC09` em sucesso. Callers e teardown das quatro
  classes ainda não estão fechados, portanto o fluxo permanece `LOCATED`.
- `FUN_004B37C9` troca `DAT_0067CF38`, marca a cena anterior para deleção,
  transfere o estado de dois controles em `scene+0x84`, copia estado e tempo e
  chama novamente o slot `ObjectManager+0x64` com a cena anterior. O receptor
  da chamada final foi resolvido pelas instruções como o próprio manager. O
  consumidor de `m_cDeleted` foi localizado posteriormente em `FUN_004B16C0`;
  a ordem integral de encadeamento/detach ainda impede promover a ficha.
- A matriz de construtores foi corrigida diretamente nas instruções de
  `FUN_004B3500`: estado `0 -> FUN_004343A4`, `5 -> FUN_0049EE30`,
  `7 -> FUN_004A8CCF` e `8 -> FUN_00431D00`. A associação anterior na ficha
  estava invertida e não deve ser reutilizada.
- A cena do estado `5` instala a vtable `0x005A44B4`. Os slots confirmados são
  `+0x00 FUN_004A8910` (deleting destructor), `+0x04 FUN_004A626E` (packet),
  `+0x4C FUN_0049F0E7` (initialize), `+0x58 FUN_004A32DD` (controle/evento) e
  `+0x64 FUN_0049AD57`.
- A cena do estado `0` instala a vtable `0x005A4294`; seu slot `+0x4C`,
  armazenado em `0x005A42E0`, é `FUN_00435B13`. O caller indireto fica em
  `FUN_004B3500:0x004B370F`. O único retorno normal do initializer grava
  `EAX=1`, portanto o ramo `"Initialize Scene Fail."` não é alcançável por
  retorno falso desse override. Falhas não locais não são convertidas em zero.
- O initializer `+0x4C` da cena `5`, `FUN_0049F0E7`, possui um único `RET`. Se
  `FUN_00541065` retorna zero, ele registra `"DataFile Not Found  "`, mostra
  `MessageBoxA("DataFile Not Found.", "File Lost", 0)`, agenda `WM_CLOSE` e
  retorna `0`; todos os demais caminhos normais retornam `1`. O caller genérico
  então destrói a cena parcial, mostra `"Initialize Scene Fail."` e agenda outro
  `WM_CLOSE`. A semântica específica de `FUN_00541065` não foi atribuída.
- A cena `7` instala o vptr `0x005A4544`; o slot `+0x4C`, armazenado em
  `0x005A4590`, aponta para `FUN_004A8F14` e usa o caller indireto
  `FUN_004B3500:0x004B370F`. O initializer possui 1.626 instruções e um único
  `RET`. Falha de `FUN_00541065` produz log, `MessageBoxA`, `WM_CLOSE` e retorno
  `0`; os demais caminhos normais retornam `1`. O caller genérico destrói a
  cena parcial e repete diagnóstico/`WM_CLOSE`. Não foi atribuída semântica
  específica ao helper.
- A cena `8` instala o vptr `0x005A421C`; o slot `+0x4C`, armazenado em
  `0x005A4268`, aponta para `FUN_00432181` e usa o caller indireto
  `FUN_004B3500:0x004B370F`. O initializer possui 431 instruções e um único
  `RET`. Falha de `FUN_00541065` produz log, `MessageBoxA`, `WM_CLOSE` e retorno
  `0`; os demais caminhos normais retornam `1`. O caller genérico destrói a
  cena parcial e repete diagnóstico/`WM_CLOSE`. Não foi atribuída semântica
  específica ao helper. Os initializers `0/5/7/8` estão fechados no CFG normal.
- `FUN_004A32DD`, no evento `0x1204`, aplica debounce de 2 s, valida índice
  assinado `0..3` e personagem habilitado, monta packet `0x213` de `0x24` bytes
  com índice em `+0x0C` e envia no callsite `0x004A3422`; depois grava timestamp
  e desabilita três controles.
- A origem UI concreta de `0x1204` foi fechada. `FUN_00493E70 -> FUN_0040C2CD`
  cria em `scene+0x28` o container com vptr `0x005A3F34`, receptor embutido em
  `container+0x24` com vptr `0x005A3F30` e owner em `container+0x2C`.
  `FUN_004974EC`, no registro tipo `2`, cria o `SButton`, guarda ID em `+0x44`
  e receptor em `+0x5C`. No release `0x202`, `FUN_004032E8` chama o slot zero
  do receptor; `FUN_0040CDA4` encaminha ID/ação ao slot `+0x58` da cena, que na
  vtable `0x005A44B4` é `FUN_004A32DD`. `FUN_0049F0E7` carrega
  `UI_SelCharScene.txt`/`UI_SelCharScene2.txt` e localiza o controle `0x1204`.
- O transporte do `0x213` foi seguido por
  `FUN_0042550E -> FUN_00424C2C -> FUN_00424DFE -> FUN_00425266`: enqueue,
  seed/checksum/tempo, cifragem de `+0x04..fim`, limite de `0x20000`, uma
  tentativa de `send` e preservação de pendência em erro parcial. O retorno do
  enqueue é ignorado; a wrapper retorna apenas o flush.
- O dispatch de packets foi fechado em `FUN_004B263E`, slot
  `ObjectManager+0x08`: ele começa pela raiz ativa `DAT_0067CF38`, chama o slot
  `+0x04` e percorre os filhos até um handler retornar `1`. A cena `0` instala
  `0x005A4294 +0x04 -> FUN_00492E7D`; a cena `5` instala
  `0x005A44B4 +0x04 -> FUN_004A626E`. Portanto os consumidores de `0x114` são
  overrides mutuamente exclusivos. Na cena 5, `FUN_004A626E` troca para estado
  `0` e retorna `1`; o mesmo packet não é reenviado à nova raiz. O scan
  `virtual-slot-04-all.tsv` registrou 116 hits, hash correto e resumo
  `virtual_slot_search`, sem `SCRIPT ERROR`.
- `FUN_004B21C9` grava `scene+0x14 = 1` e `manager+0x1B08C = 1`.
  `FUN_004B16C0` consome a marca, chama o deleting destructor e limpa
  `manager+0x1B088/+0x1B08C`. Os cleanups das cenas `0/5/7/8` convergem em
  `FUN_00494C00`, e `FUN_0054AA45` destrói filhos e desanexa o nó da árvore.
- A ordem local de coleta foi fechada. `FUN_004B16C0` captura
  `DAT_0067CF38` como raiz/sentinela, percorre somente essa subárvore e para
  antes de seguir irmãos da raiz; depois processa separadamente
  `manager+0x1B088`. Na destruição da cena anterior, `FUN_00494C00` zera
  `+0x1B088` antes de reentrar em `FUN_004B16C0`, removendo o único caminho
  explícito que poderia selecionar a cena antiga novamente. O detach por
  `FUN_0054AA45` ocorre depois dessa reentrada.
- A raiz é construída por `FUN_0054A9E0` com vptr `0x005A5CFC`; seu slot zero
  é `FUN_0054AE10 -> FUN_0054AA45`, com free condicional a `flag & 1`.
  `FUN_00401152` não é esse deleting destructor: pertence à cadeia
  `0x005A3420 -> FUN_0040B980 -> FUN_00401152`. `FUN_0054AC09` insere filhos
  no início da lista intrusiva, atualizando parent, primeiro filho e irmãos.
- `FUN_0055D345` testa `manager+0x1B08C` depois do tick, carrega
  `ECX = app+0xF8` e chama `FUN_004B16C0` em `0x0055D6C5`; esse é o caller
  periódico fechado do coletor de cenas marcadas.
- A ordem interna observada de `FUN_0055D066` começa por três recursos em 100
  registros, destrói `DAT_013B71E8`, depois desmonta `app+0xF4`, `+0xE0`,
  `+0xE4`, `+0xFC`, `+0xEC`, `+0xE8`, `+0xF0`, zera `DAT_013B71E4`, chama
  `DeleteObject(app+0x110)` e opcionalmente
  `FUN_0059DFE0(DAT_013B7364)`. Receptores, ownership e falha parcial foram
  correlacionados com a source e fechados no contrato estreito de shutdown;
  o caller efetivo do slot é o ramo `WM_CLOSE` descrito abaixo.
- `FUN_004B1EA9` inicializa o `ObjectManager` com vptr `0x005A45FC`, cena global
  nula e estado `-1`; `FUN_004B3A20 -> FUN_004B2155` desmonta a raiz e zera
  `DAT_0067CF38`. O timer usa vtable `0x005A4688`, é publicado em
  `DAT_0092E654` e atualiza `DAT_0092E658`.
- Os 56 exports TSV desta rodada estão inventariados, classificados e ligados
  às conclusões/lacunas em
  `.agents/research/client748/inventory/scene-transition-evidence-log.md`.
  Os aproximadamente 37,39 MiB regeneráveis permanecem fora do Git em
  `%TEMP%\codex-wyd748-lifecycle-149205b7`.
- `FUN_004B29B9` foi fechado como traversal de vtable `0x005A4614`, raiz
  `DAT_0067CF38`, guard `node+0x14` e parada no primeiro retorno `1`.
  `FUN_004B2D35` usa `0x005A4624/+0x20`, preserva `DAT_0067CF3C` e sinaliza
  deleção em `manager+0x1B08C` na condição observada.
- A ficha `field-scene-rebuild-after-server-move.md` está em `CONTRACT`:
  dispatch `FUN_00492E7D:0x00493037 -> FUN_00484D44`, packet `0x52A/0x50`,
  login `0x20D/0x74` versão 748, replay Field-only por até 15 segundos, consumo
  one-shot e reconstrução `9 -> 0`. A source foi adaptada no commit `60c57760`.
- A ficha `character-logout-selectchar-relogin.md` está em `CONTRACT`: os
  controles nativos `633..635` enviam `0x3AE/0x10`; `634` dispara após cinco
  segundos `0x215/0x0C`, o servidor persiste/remove o runtime e responde
  `0x116/0x0C`; `FUN_00492E7D -> FUN_00484C8A` reconstrói a seleção, seguida de
  `0x213 -> 0x114` para retornar ao Field. O dispatch faltante foi restaurado
  na source; `636` continua sendo cancelamento local sem packet.
- A ficha `application-close-global-shutdown.md` está em `CONTRACT`:
  `FUN_0055FA89 -> FUN_0055DAB8` recebe `WM_CLOSE`; Field envia uma vez
  `0x3AE/0x10` e aguarda 3.000 unidades do TimerManager, enquanto outras cenas
  fecham imediatamente. `FUN_0055D066` desmonta os owners, `FUN_00423C61`
  remove o hook e a cadeia termina em `DestroyWindow -> PostQuitMessage(0)`.
  Source e WYD-Go já preservam o contrato, com guardas seguras de init parcial;
  não há delta funcional nem alegação de `CLIENT_TESTED`.
- A vtable da aplicação em `0x005A6104` contém, nos slots `+0x00..+0x1C`,
  `FUN_0055F3E0`, `FUN_0055BC0A`, `FUN_0055D066`, `FUN_0055D345`,
  `FUN_0055EDF7`, `FUN_0055D6E6`, `FUN_0055EE1E` e `FUN_0055EE45`.
- `FUN_0055D345:0x0055D5BC` chama `app+0x14/FUN_0055D6E6` usando a própria
  aplicação como receptor quando `app+0xE8 != 0` e
  `[app+0xE8]+0x10C != 2`. A função consome estado da aplicação/cena e chama o
  slot `+0x48` do objeto em `scene+0x28`, fechando-a como etapa de frame/render;
  ela não chama os slots de cena `+0x14/+0x20`.
- A varredura global desses offsets encontrou 34 hits em `+0x14` e 53 em
  `+0x20`, após 8.173 candidatos/441.614 instruções por scan. Os candidatos
  próximos do manager são `FUN_004B29B9:0x004B29F3` e
  `FUN_004B2D35:0x004B2D86`; receptor, vptr, classe e semântica ainda precisam
  ser resolvidos antes de atribuir nomes aos slots.
- `FUN_0055B18F` instala essa vtable e publica o mesmo objeto em
  `DAT_013B71E0`. `FUN_0055FA89` copia quatro argumentos, carrega esse global
  como `ECX`, chama `FUN_0055DAB8` em `0x0055FAA2` e retorna com `RET 0x10`;
  portanto, o dispatcher recebe o singleton da aplicação.
  `FUN_0055EE1E` grava o argumento em `app+0xF8` e em `DAT_013B71E8`.
- `FUN_0055F7F9` aloca `0x114` bytes, constrói/publica a aplicação, chama o
  slot `+0x04` para bootstrap, o slot `+0x0C` para o loop e, ao retorno, o slot
  `+0x00` com flag `1`. Ele não chama diretamente `+0x08/FUN_0055D066` porque
  o shutdown ocorre dentro do window procedure, antes de o loop retornar.
  Falhas anteriores ou do bootstrap não mostram cleanup explícito no recorte,
  sem que isso autorize concluir leak.
- Três candidatos de slot `+0x08` foram rejeitados por receptor:
  `FUN_0055FBB9:0x0055FC85` usa um objeto vindo de `param_1` slot `+0x18`;
  `FUN_0055D345:0x0055D415/0x0055D4DF` usa `app+0xF4/app+0xF8`; e
  `FUN_0055EE59:0x0055EFF7` usa o timer `DAT_0092E654`. Nenhum possui fluxo de
  dados comprovado até o singleton `DAT_013B71E0`.
- `FUN_0055F3E0 -> FUN_0055B26F` reinstala a vtable da aplicação, fecha o
  handle global de logging `DAT_005CCFAC` por `FUN_004312DD` e, com flag `1`,
  libera o objeto. Depois, `FUN_004B428E` desmonta DLL/callback, buffers,
  critical section e filtro de exceção. Essa cadeia posterior não chama
  `FUN_0055D066`; o shutdown já ocorre antes dela, no ramo `WM_CLOSE`.
- No ramo `WM_CLOSE` de `FUN_0055DAB8`, `0x0055EB9C` carrega o vptr e o
  receptor diretamente de `param_1`. Como `FUN_0055FA89` fornece
  `DAT_013B71E0` e a vtable `0x005A6104` resolve `+0x08` para
  `FUN_0055D066`, a cadeia efetiva é `WM_CLOSE -> 0x0055EB9C -> app vslot
  +0x08 -> FUN_0055D066`. Antes disso, `0x0055EB5A/0x0055EB74` chamam o slot
  `+0x08` do objeto em `app+0xF4`; seu retorno é comparado em
  `0x0055EB77..0x0055EB85` com `DAT_013B7220 + 0xBB8`, e o shutdown só
  prossegue quando esse limite de 3000 unidades é atingido. A unidade não foi
  atribuída. Depois do retorno, `DAT_013B71F0` pode acionar `FUN_0058F75F`, é
  zerado, `DAT_013B7228` recebe `1`, e seguem `FUN_00423C61` em `0x0055EBCB`,
  `DestroyWindow` em `0x0055EBD4`, `PostQuitMessage(0)` em `0x0055EBDC` e
  `DAT_013B7220 = 0` em `0x0055EBE2`.
- `FUN_00423C61` tem um único caller, `FUN_0055DAB8:0x0055EBCB`. Quando
  `DAT_005CCF84 == 0`, retorna. Caso contrário, consulta `FUN_00423B25`, chama
  `UnhookWindowsHookEx(DAT_005CCF80)` quando o helper retorna zero e sempre
  finaliza com `DAT_005CCF84 = 0`. Isso fecha sua função entre o shutdown da
  aplicação e `DestroyWindow`.
- O outro extremo do hook está em `FUN_0055F7F9:0x0055FA08 -> FUN_00423C1F`,
  após bootstrap bem-sucedido e antes do loop principal. O instalador usa
  `WH_KEYBOARD_LL`, callback `FUN_00423B74`, thread id global `0`, grava o
  handle em `DAT_005CCF80` e marca `DAT_005CCF84 = 1` mesmo se a API retornar
  nulo. `FUN_00423B25` testa `dwPlatformId == 1`; nessa plataforma as APIs de
  instalação/remoção são puladas, mas o flag lógico é mantido.
- `FUN_00423B74` é callback indireto comprovado pelo ponteiro em `0x00423C46`.
  Para hook code não negativo, bloqueia `Ctrl+Esc`, `Alt+Esc`, Windows esquerda
  `0x5B`, Windows direita `0x5C` e Apps `0x5D`; encaminha os demais casos por
  `CallNextHookEx`. Somente o instalador escreve `DAT_005CCF80`, e somente
  instalador/finalizador acessam `DAT_005CCF84`.
- No branch `0x464` de `FUN_0055DAB8`, o dispatch em `0x0055E80D` usa
  comprovadamente o objeto em `app+0xF8`, enquanto o dispatch em `0x0055E8CA`
  usa `app+0xF4`. O segundo receptor, seu ownership e o significado exato do
  evento permanecem não confirmados; sem imports, argumentos e teardown esse
  trecho não integra contrato.
- `ExportWydFingerprints.java` foi executado em modo headless/read-only nos
  dois projetos Ghidra: exportou 4.146 funções do binário nativo e 3.135 do
  `project.exe` atual. `correlate_fingerprints.py`, revalidando ambos os hashes,
  classificou `EXACT_MATCH=88`, `CANDIDATE=385` e `NO_MATCH=3673`. Esses
  resultados formam uma fila diferencial; não promovem maturidade nem atribuem
  nomes sem revisão do fluxo no Ghidra.
- O modo headless/read-only `pointers:0055890a` do `ExportWydFlow.java` foi
  executado no Ghidra 12.1.3 e encontrou zero VAs brutos de 32 bits. Isso não
  elimina chamada calculada, thunk ou tabela construída em runtime; a ficha
  continua `LOCATED`.
- O modo `relatives:0055890a` percorreu 441.614 instruções decodificadas e
  resolveu 24.880 candidatos `CALL/JMP rel32`, sem branch direto para a função.
  Controles positivos localizaram 757 branches para `FUN_0058F078` e 16 para
  `FUN_00423B20`, incluindo os callsites `0x005599EF` e `0x00559A05` dentro da
  função principal; o resultado zero não é falha do cálculo de `rel32`.
- O modo `bodyrefs:0055890a` inspecionou as 1.094 instruções do corpo
  `0x0055890A-0x00559A13` e encontrou zero xrefs `FLOW`, zero outros xrefs e
  zero thunks externos. Os 117 xrefs `DATA` são seis jump tables internas do
  switch, não callers. A instrução anterior em `0x00558909` é `RET`, sem
  fallthrough para a entrada.
- O modo `rawrelatives:0055890a` varreu 1.859.564 bytes dos cinco blocos
  executáveis inicializados e resolveu 34.511 candidatos `E8/E9 rel32`, sem hit
  e sem erro. No mesmo export, os controles positivos recuperaram 16 e 757
  xrefs `FLOW` e os mesmos 16 e 757 hits brutos, todos classificados como
  instrução.
- O export focado `packet-size-gate-bodyrefs.tsv` tem SHA-256
  `E51351C895E0F9439AF37E97DDE30F12F98A3822E728EA691115B10E9542EAF6`.
  `ExportWydFlow.java` passou a encerrar a expansão de jump tables no limite do
  address space sem wrap; a versão endurecida recompilou e reproduziu o export
  anterior byte a byte.
- `FUN_0058F078` foi confirmado como formatter que termina o buffer em nulo.
  `FUN_00423B20` retorna imediatamente e não apresenta side effect próprio
  observável no binário analisado. Isso fecha os dois callees, mas não o caller.
- O TMProject 7.59/7.69+ pode fornecer estrutura, algoritmos e assets superiores.
  Para uma fronteira legada, IDs, recursos, packets, offsets, endereços, layouts,
  loaders e lifecycle continuam sendo claims a provar no 7.48. Para extensão
  coordenada, esses elementos são projetados e testados nos consumidores atuais.
- Toda alteração ativa do client pertence a `client-source/` ou aos assets. O
  build oficial é `client-source/tmproject/Build-Client.ps1`, e
  `client748/project.exe` é o único candidato executável e de validação.

## Estado atual

```text
regras globais e scoped                  | STATICALLY VERIFIED | gate de pesquisa e versão única documentados
skill wyd-client748-catalog                | STATICALLY VERIFIED | triagem determinística do corpus e fila por lane
skill wyd-client748-research             | STATICALLY VERIFIED | quick_validate.py e script headless passaram
infraestrutura e schema das fichas       | STATICALLY VERIFIED | scripts e template revisados; validador passou
correlação estrutural native/source       | AUTOMATED TESTED     | Ghidra real + correlator: 88 exact, 385 candidates
gate de tamanho por opcode               | LOCATED             | entrada nativa localizada; caller/direção pendentes
foco, IME e lifecycle de controles       | LOCATED             | fluxo principal localizado; xrefs/teardown pendentes
transição e troca de cenas                | LOCATED             | shutdown/logout fechados à parte; demais callers/transições pendentes
reconstrução Field pós-migração           | STATICALLY VERIFIED | ficha CONTRACT e source no commit 60c57760; fluxo real pendente
logout para seleção e relogin             | STATICALLY VERIFIED | ficha CONTRACT; dispatch 633..635 compilado; fluxo real pendente
fechamento e shutdown global              | STATICALLY VERIFIED | ficha CONTRACT; source atual mantida; fluxo real pendente
código ativo do client                    | IMPLEMENTED         | Basedef/ObjectManager/TMFieldScene/TMScene alterados; servidor preservado
client748/project.exe no fluxo real      | NÃO TESTADO          | proibido declarar CLIENT-TESTED
```

## Worktree e arquivos ativos

- `AGENTS.md` — classificação dos três modos, fast path de continuidade,
  política `gpt-5.6-sol/xhigh` quando disponível e proteção do trabalho manual.
- `.agents/skills/wyd-client748-catalog/` — skill, estratégia, metadados e
  triador determinístico do corpus.
- `client-source/AGENTS.md` — source de implementação única, escolha da
  estrutura superior compatível e gates distintos para paridade e extensão.
- `.agents/skills/wyd-go-feature/SKILL.md` — implementa paridade, modernização
  compatível ou extensão coordenada sem promover hipótese nativa.
- `.agents/skills/wyd-client748-research/` — nova skill, referências, metadados
  e ferramentas reproduzíveis, incluindo export e correlação diferencial de
  fingerprints com testes determinísticos.
- `.agents/research/client748/` — README, template, quatro exports focados e as
  fichas do programa, incluindo `flows/lifecycle/scene-transition.md` e o
  contrato `character-logout-selectchar-relogin.md`, além do contrato terminal
  `application-close-global-shutdown.md`; o
  inventário README inclui o procedimento do triador. O ledger
  `inventory/scene-transition-evidence-log.md` preserva a rodada de 56
  exports e a ficha de migração registra o contrato aplicado.
  Exports exploratórios amplos e não citados foram removidos da worktree e
  preservados temporariamente em
  `%TEMP%\wyd748-broad-exports-20260828-commit`; são regeneráveis pelo projeto
  Ghidra e não pertencem ao commit.
- `.agents/handoffs/client748-research-program.md` — estado operacional deste
  programa.
- `.agents/handoffs/client748-parity.md` — escopo anterior preservado e não
  alterado por este programa.

Todos os arquivos acima, exceto o handoff de paridade já existente, pertencem
ao escopo documental atual. Preservar a worktree inteira e inspecionar o diff
antes de editar; handoff não funciona como lock.

## Última validação

```text
python .agents/skills/wyd-client748-research/scripts/validate_research.py --repo .
resultado: exit 0; reexecutado em 2026-08-31; seis fichas válidas;
CONTRACT=3 e LOCATED=3

python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format summary
resultado: exit 0; 4.146 funções, 46 STATICALLY_EVIDENCED, 22 LOCATED e
4.078 UNMAPPED

go test ./internal/game ./internal/wire
resultado: exit 0

client-source/tmproject/Build-Client.ps1
resultado: exit 0; 31 warnings preexistentes, zero erros; candidato instalado
com SHA-256 746A2913FA62DD56892319BD136CD15810A4B74092AD75F354E60C6B3FFC5BBD

conferência do ledger scene-transition-evidence-log.md contra
%TEMP%\codex-wyd748-lifecycle-149205b7\*.tsv
resultado antes da validação desta retomada: 56 exports inventariados;
31 CONCLUSÃO CONFIRMADA, 17 PISTA LOCALIZADA, 1 AINDA NÃO INTERPRETADO e
7 LACUNA SEGUINTE; volume aproximado 37,39 MiB

scene5-select-enter-focused.tsv
resultado: 1.978.822 bytes; SHA-256
074F55D599977F1A0D3045DEC0B23428FAD8C11A86A64383862673210B8E9906;
hash nativo embutido correto; 2.509 instruções de FUN_0049F0E7; um RET;
nenhum SCRIPT ERROR

scene7-initialize-004a8f14-focused.tsv
resultado: 564.561 bytes; SHA-256
F4DCEB6F8016879FD2D1D15D4D361AE88020B6825463D94AC9D62E1D71E650F6;
hash nativo embutido correto; 1.626 instruções de FUN_004A8F14; um RET;
nenhum SCRIPT ERROR

scene8-initialize-00432181-focused.tsv
resultado: 226.544 bytes; SHA-256
3BC37C89D001D8AEADEA19E4F359A49E111E0D5DEACFEA53B4B3281A09C2BEE1;
hash nativo embutido correto; 431 instruções de FUN_00432181; um RET;
nenhum SCRIPT ERROR; a autoanálise temporária iniciada pela ausência de
-noanalysis foi descartada sem gravação no projeto

application-shutdown-focused.tsv
resultado: 804.704 bytes; SHA-256
5676C66782F8635FF65A59F48C64099DD397F2FC0C977A26CD1BC25A06B0051B;
ordem interna de FUN_0055D066 transcrita; classes/ownership ainda pendentes

wm-close-dispatch-instructions.tsv
resultado: 302.057 bytes; SHA-256
D52A8CE0C54610CBBE2230A4689632481A96D19E65FD40827ECB91EA506F0686;
hash nativo embutido correto; 975 instruções e 649 referências de
FUN_0055DAB8; caller da aplicação em 0x0055EB9C e sequência posterior até
PostQuitMessage(0) transcritos; nenhum SCRIPT ERROR

fun-00423c61-shutdown-focused.tsv
resultado: 304.785 bytes; SHA-256
5799FB5705F68DBB188B53D2989E635DBD1C146AA559161696F844B438676991;
hash nativo embutido correto; 18 instruções, 10 referências, único caller em
0x0055EBCB; pointer hits=0 e rel32/raw hits=1; nenhum SCRIPT ERROR

windows-hook-lifecycle-focused.tsv
resultado: 37.306 bytes; SHA-256
ACC5288A10463BABD0FA2DD83058F78BB2B65130A52D1AA90169B0397EA97D43;
hash nativo embutido correto; lifecycle de instalação/callback/finalização e
xrefs dos globals transcritos; callback por ponteiro imediato confirmado;
nenhum SCRIPT ERROR

application-frame-0055d6e6-focused.tsv
resultado: 259.951 bytes; SHA-256
DD7762EA1384BC1A21E1DF4B58F2F94DF486B912A0C212E9B587B3349A61ED09;
hash nativo embutido correto; caller FUN_0055D345:0x0055D5BC, receptor da
aplicação e estágio de frame/render transcritos; nenhum SCRIPT ERROR

virtual-slots-14-20-all.tsv
resultado: 30.094 bytes; SHA-256
BC4CF4ECDBACD7412DE9F42045B58DCDD2185E919C4E0277E7FC0B2AD4E1FDD6;
hash nativo embutido correto; 34 hits em +0x14, 53 em +0x20, 8.173
candidatos e 441.614 instruções por scan; nenhum SCRIPT ERROR

python %USERPROFILE%/.codex/skills/.system/skill-creator/scripts/quick_validate.py .agents/skills/wyd-client748-research
resultado: exit 0; Skill is valid!

python %USERPROFILE%/.codex/skills/.system/skill-creator/scripts/quick_validate.py .agents/skills/wyd-client748-catalog
python %USERPROFILE%/.codex/skills/.system/skill-creator/scripts/quick_validate.py .agents/skills/wyd-go-feature
resultado: exit 0; ambas as skills adicionais são válidas

python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format summary
resultado: exit 0; reexecutado em 2026-08-30; 4.146 funções, 4 lanes,
4.084 UNMAPPED, 23 LOCATED e 39 STATICALLY_EVIDENCED

python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format json --top 5
resultado: exit 0; functions=4146, selected_functions=5 e top=5

python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format tsv --top 3
resultado: exit 0; cabeçalho e três linhas de dados

python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format tsv --top 0 | Measure-Object -Line
resultado: exit 0; 4.147 linhas incluindo cabeçalho

python .agents/skills/wyd-client748-research/scripts/query_corpus.py stats --repo .
resultado: exit 1 diagnóstico; corpus=4146, referências=108, ausentes=2

ExportWydFlow.java em Ghidra 12.1.3 headless/read-only
resultado: SHA-256 correto; pointer_scan hits=0; rel32_scan hits=0 entre 24.880
candidatos/441.614 instruções; bodyrefs sem FLOW/thunk e 117 DATA de seis jump
tables; raw_rel32 hits=0 entre 34.511 candidatos/1.859.564 bytes; controles
positivos dos dois callees passaram; endurecimento boundary-safe preservou o
export E51351C8...EAF6 byte a byte

ExportWydFlow.java em Ghidra 12.1.3, modos `instructions:` e `table:`
resultado: exit 0; vtable do ObjectManager e instruções de `FUN_004B3500`,
`FUN_004B37C9`, quatro construtores de cena e `FUN_0054AC09` exportadas em
`scene-transition-instructions.tsv`; fluxo ainda sem maturidade `TRACED`

ExportWydFingerprints.java em Ghidra 12.1.3 headless/read-only
resultado: exit 0; 4.146/4.146 funções nativas e 3.135/3.135 funções da source;
logs sem `SCRIPT ERROR`; SHA-256 embutidos iguais aos binários consultados

python .agents/skills/wyd-client748-research/scripts/correlate_fingerprints.py
resultado: exit 0; 4.146 correlacionadas; EXACT_MATCH=88, CANDIDATE=385 e
NO_MATCH=3673; binários revalidados por SHA-256

python -m unittest discover -s .agents/skills/wyd-client748-research/scripts -p 'test_*.py' -v
resultado: exit 0; 12 testes passaram

forward-test somente leitura com gpt-5.6-sol/xhigh
resultado: classificou a ficha como LOCATED, bloqueou edição e exigiu CONTRACT

hashes SHA-256 da referência e do candidato
resultado: 8AA2F918...15F593 e F8251714...A380B

git diff --check
resultado: exit 0; reexecutado em 2026-08-30; somente avisos informativos
LF/CRLF do Git no Windows
```

O `exit 1` de `query_corpus.py stats` é um alerta de cobertura textual, não uma
falha das fichas. A busca por whitespace final nos novos arquivos não encontrou
ocorrências.

## Pendências e riscos

- O branch `0x116` de `FUN_0055890A` está fechado pelo contrato de relogin, mas
  a função completa permanece `LOCATED`: sua alcançabilidade indireta/runtime e
  os demais branches ainda não autorizam promoção integral nem declaração de
  código morto.
- Reabrir no Ghidra a cadeia de entrada, foco, IME, árvore e teardown registrada
  em `ui/control-focus-ime-lifecycle.md`; fechar alocação parcial, troca de cena
  e controles opcionais. O menu System de logout está fechado na ficha própria.
- Resolver `FUN_00452733` e `FUN_0047E4D6` diretamente no projeto Ghidra ou
  mantê-las explicitamente como não confirmadas. Não fabricar entrada no corpus.
- Não promover nenhuma ficha por compilação, semelhança de source ou pressão de
  implementação. Claims `HYPOTHESIS` não integram contrato.
- A implementação de migração exige `Build-Client.ps1`; registrar o novo hash
  de `project.exe`, sem confundir build com teste real do fluxo.
- Não converter os 88 matches estruturais em nomes ou estado de pesquisa por
  lote. Usá-los como âncoras de localização e fechar cada transição observável
  com xrefs, estado, erro, ownership e lifecycle.
- Não reler por padrão os exports já inventariados do lifecycle. Consultar o ledger, abrir
  somente o export ligado à lacuna atual e escrever a conclusão no mesmo ciclo.
- A ficha de cenas permanece `LOCATED`: os receivers/retornos `+0x4C` das cenas
  `0/5/7/8`, a ordem local de coleta/detach, o shutdown terminal e o lifecycle
  do hook global de teclado estão fechados; faltam callers de estado e a
  convergência das demais transições. Logout/relogin explícito e shutdown global
  estão fechados em fichas próprias.
- Os traversals `FUN_004B29B9` e `FUN_004B2D35` estão fechados no recorte do
  `ObjectManager`; não generalizar sua semântica aos demais hits globais.

## Próximo passo executável

1. Executar no candidato hasheado os controles `633`, `634`, `635` e `636`, a
   volta à seleção e o relogin no mesmo personagem e em outro slot; até isso,
   manter o estado máximo `STATICALLY VERIFIED`.
2. Continuar o lifecycle por uma ficha estreita de troca de conta ou reconexão
   TCP, partindo da source viva e abrindo no Ghidra somente os callers/callees
   que decidirem a transição escolhida. Não reabrir o shutdown global sem nova
   evidência de incompatibilidade ou falha runtime.
3. Inspecionar a source atual e classificar o próximo delta concreto. Se ele
   depender de paridade nativa, fechar entrada, callers, callees, estado, erros,
   ownership e teardown antes de adaptá-lo; se for modernização/extensão
   independente, provar a fronteira correspondente e prosseguir sem aguardar a
   promoção de claims não relacionados.
4. Retomar `packet-size-gate.md` após o fluxo de cenas ou em campanha paralela;
   sua maturidade continua `LOCATED`, sem autorização para alterar wire/ABI.
5. Executar `validate_research.py` quando ficha/schema mudar, triagem quando a
   fila/input mudar e `git diff --check` após edições. Atualizar este handoff
   somente quando houver nova evidência, decisão, validação ou ponto de retomada.

## Critérios de aceite pendentes

- Outra sessão consegue reproduzir cada claim sem recorrer à conversa.
- Nenhuma edição de paridade depende de claim `LOCATED`; extensões independentes
  não são bloqueadas nem apresentadas como comportamento nativo.
- Toda decisão futura separa comportamento 7.48 confirmado, estrutura moderna
  compatível, contrato coordenado novo e lacunas realmente bloqueantes.
- Código e assets manuais são preservados até incompatibilidade concreta ou
  caminho substituto validado justificar sua alteração.
- `CLIENT-TESTED` aparece somente depois do fluxo real no hash registrado de
  `client748/project.exe`.
