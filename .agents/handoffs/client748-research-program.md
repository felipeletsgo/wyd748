# Handoff: programa de pesquisa do client 7.48

Atualizado em: 2026-08-30
Estado geral: `STATICALLY VERIFIED`

## Objetivo e limites

Transformar a descompilação do client nativo WYD 7.48 em conhecimento
reproduzível antes de qualquer implementação no WYD-Go ou em
`client-source/tmproject`. O programa separa contrato 7.48 comprovado de
semântica herdada do TMProject 7.59/7.69+ e exige uma ficha por transição.

Este handoff cobre somente a infraestrutura de pesquisa, suas três fichas
iniciais, a estratégia de cobertura e as regras que bloqueiam edição prematura.
A paridade funcional e visual já implementada possui estado próprio em
`.agents/handoffs/client748-parity.md`; não duplicar esse histórico aqui.

Nenhum código ativo do client ou servidor foi alterado neste escopo. Não houve
build, startup ou teste in-game.

## Estratégia formalizada nesta etapa

O fluxo de trabalho agora é uma regra do repositório:

```text
catálogo -> callgraph -> fluxo observável -> adaptação -> validação
```

`wyd-client748-catalog` valida o censo das 4.146 funções, ordena raízes por
evidência disponível e entrega uma fila reproduzível. `wyd-client748-research`
fecha uma transição no projeto Ghidra, com callers/callees, estado, efeitos,
erros e teardown. `wyd-go-feature` só adapta o delta comprovado pela ficha
`TRACED`/`CONTRACT`. O triador não promove estado, não cria stubs e não usa o
TMProject 7.69+ como contrato 7.48.

## Fontes e artefatos

```text
client748/wyd.exe nativo+patches/WYD.exe | referência histórica Ghidra | 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
client748/project.exe                    | candidato source volátil    | F8251714775601720307940598522E6D2924E5C61DAB300728F949FE0C8A380B
%USERPROFILE%\Tools\GhidraProjects\WYD748Native_20260821.gpr | projeto Ghidra | descobrir no perfil
%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled       | corpus auxiliar | 4.146 funções
```

Os hashes foram recalculados em 2026-08-28. O hash de `project.exe` deve ser
recalculado após qualquer build. O corpus textual acelera buscas, mas não
substitui xrefs, chamadas indiretas, tipos, stack ou lifecycle no projeto
Ghidra.

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
- `wyd-client748-research` e `wyd-go-feature` agora exigem o catálogo antes de
  escolher uma raiz client/protocolo e documentam a separação entre os estados
  de pesquisa (`UNMAPPED`/`LOCATED`/`TRACED`/`CONTRACT`) e entrega
  (`IMPLEMENTED`/`CLIENT_TESTED`). As três skills passaram no
  `quick_validate.py`.
- `LOCATED` autoriza somente pesquisa/documentação. Comportamento exige ficha
  `TRACED`; packet, wire, ABI, struct, offset, packing, signedness e loader
  exigem `CONTRACT`.
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
- `FUN_004B1EA9` inicializa o `ObjectManager` com vptr `0x005A45FC`, cena global
  nula e estado `-1`; `FUN_004B3A20 -> FUN_004B2155` desmonta a raiz e zera
  `DAT_0067CF38`. O timer usa vtable `0x005A4688`, é publicado em
  `DAT_0092E654` e atualiza `DAT_0092E658`.
- Os 47 exports TSV desta rodada estão inventariados, classificados e ligados
  às conclusões/lacunas em
  `.agents/research/client748/inventory/scene-transition-evidence-log.md`.
  Os aproximadamente 33,22 MiB regeneráveis permanecem fora do Git em
  `%TEMP%\codex-wyd748-lifecycle-149205b7`.
- A vtable da aplicação em `0x005A6104` contém, nos slots `+0x00..+0x1C`,
  `FUN_0055F3E0`, `FUN_0055BC0A`, `FUN_0055D066`, `FUN_0055D345`,
  `FUN_0055EDF7`, `FUN_0055D6E6`, `FUN_0055EE1E` e `FUN_0055EE45`.
  `FUN_0055EE1E` grava o argumento em `app+0xF8` e em `DAT_013B71E8`.
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
- O TMProject 7.59/7.69+ fornece somente pistas semânticas. IDs, recursos,
  packets, offsets, endereços, layouts, loaders e lifecycle precisam ser
  comprovados no 7.48.
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
transição e troca de cenas                | LOCATED             | origem UI/packet/resposta/cleanup localizados; ordem do 0x114, shutdown e relogin pendentes
código ativo do client/servidor          | NÃO ALTERADO         | nenhum build ou teste funcional necessário nesta etapa
client748/project.exe no fluxo real      | NÃO TESTADO          | proibido declarar CLIENT-TESTED
```

## Worktree e arquivos ativos

- `AGENTS.md` — gate da pesquisa 7.48, política `gpt-5.6-sol/xhigh` quando
  disponível e proibição de contrato vindo do TMProject moderno.
- `.agents/skills/wyd-client748-catalog/` — skill, estratégia, metadados e
  triador determinístico do corpus.
- `client-source/AGENTS.md` — source de versão única e maturidade mínima antes
  de qualquer edição comportamental.
- `.agents/skills/wyd-go-feature/SKILL.md` — consome fichas já maduras; não
  promove hipótese do TMProject.
- `.agents/skills/wyd-client748-research/` — nova skill, referências, metadados
  e ferramentas reproduzíveis, incluindo export e correlação diferencial de
  fingerprints com testes determinísticos.
- `.agents/research/client748/` — README, template, quatro exports focados e as
  três fichas iniciais, incluindo `flows/lifecycle/scene-transition.md`; o
  inventário README inclui o procedimento do triador. O ledger
  `inventory/scene-transition-evidence-log.md` preserva a rodada de 45
  exports sem versionar os recortes amplos.
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
resultado: exit 0; reexecutado em 2026-08-30 após mover o ledger para
inventory/; três fichas válidas; LOCATED=3

conferência do ledger scene-transition-evidence-log.md contra
%TEMP%\codex-wyd748-lifecycle-149205b7\*.tsv
resultado: 47/47 exports presentes no ledger; nenhum ausente dos dois lados;
22 CONCLUSÃO CONFIRMADA, 17 PISTA LOCALIZADA, 2 AINDA NÃO INTERPRETADO e
6 LACUNA SEGUINTE; volume aproximado 33,22 MiB

scene5-select-enter-focused.tsv
resultado: 1.978.822 bytes; SHA-256
074F55D599977F1A0D3045DEC0B23428FAD8C11A86A64383862673210B8E9906;
hash nativo embutido correto; 2.509 instruções de FUN_0049F0E7; um RET;
nenhum SCRIPT ERROR

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

- Resolver no projeto Ghidra os xrefs e o caller real de `FUN_0055890A`, a
  direção do packet, a fase do transporte e a rejeição aplicada pelo caller.
  A evidência atual permite somente a conclusão `PROBABLE` de que não há entrada
  estática identificada por xref `FLOW`, thunk ou `E8/E9 rel32`; alcançabilidade
  indireta/runtime continua `UNRESOLVED` e não autoriza declarar código morto.
- Reabrir no Ghidra a cadeia de entrada, foco, IME, árvore e teardown registrada
  em `ui/control-focus-ime-lifecycle.md`; fechar alocação parcial, troca de cena,
  logout/relogin e controles opcionais.
- Resolver `FUN_00452733` e `FUN_0047E4D6` diretamente no projeto Ghidra ou
  mantê-las explicitamente como não confirmadas. Não fabricar entrada no corpus.
- Não promover nenhuma ficha por compilação, semelhança de source ou pressão de
  implementação. Claims `HYPOTHESIS` não integram contrato.
- Não iniciar build ou teste do client apenas para validar documentação. Quando
  houver implementação futura, usar `Build-Client.ps1` e testar o fluxo real no
  novo hash de `project.exe`.
- Não converter os 88 matches estruturais em nomes ou estado de pesquisa por
  lote. Usá-los como âncoras de localização e fechar cada transição observável
  com xrefs, estado, erro, ownership e lifecycle.
- Não reler por padrão os exports já inventariados do lifecycle. Consultar o ledger, abrir
  somente o export ligado à lacuna atual e escrever a conclusão no mesmo ciclo.
- A ficha de cenas permanece `LOCATED`: os receivers/retornos `+0x4C` das cenas
  `0/5` estão fechados; faltam as cenas `7/8`, a ordem global de teardown, shutdown e
  logout/relogin.

## Próximo passo executável

1. Abrir `scene-vtables-focused.tsv`, resolver os endereços dos initializers
   `+0x4C` das cenas `7/8` e fechar retorno/falha de um deles no mesmo ciclo.
2. Fechar o outro receiver `+0x4C` e a ordem integral entre
   `FUN_004B16C0`, os cleanups específicos, `FUN_00494C00` e `FUN_0054AA45`.
3. Continuar `FUN_0055D066`, shutdown e logout/relogin usando somente os exports
   marcados `LACUNA SEGUINTE` no ledger.
4. Promover a ficha somente quando entrada, callers, callees, estado, erros,
   ownership e teardown estiverem fechados. Só depois adaptar delta comprovado.
5. Retomar `packet-size-gate.md` após o fluxo de cenas ou em campanha paralela;
   sua maturidade continua `LOCATED`, sem autorização para alterar wire/ABI.
6. Reexecutar `validate_research.py`, triagem, `git diff --check` e atualizar
   este handoff quando
   houver nova evidência ou mudança de estado.

## Critérios de aceite pendentes

- Outra sessão consegue reproduzir cada claim sem recorrer à conversa.
- Nenhuma edição comportamental começa com ficha `LOCATED`.
- Toda decisão futura separa comportamento 7.48 confirmado, ABI/recurso
  confirmado, semântica moderna apenas sugerida e lacunas bloqueantes.
- `CLIENT-TESTED` aparece somente depois do fluxo real no hash registrado de
  `client748/project.exe`.
