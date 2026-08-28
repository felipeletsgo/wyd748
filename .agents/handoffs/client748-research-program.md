# Handoff: programa de pesquisa do client 7.48

Atualizado em: 2026-08-28
Estado geral: `STATICALLY VERIFIED`

## Objetivo e limites

Transformar a descompilação do client nativo WYD 7.48 em conhecimento
reproduzível antes de qualquer implementação no WYD-Go ou em
`client-source/tmproject`. O programa separa contrato 7.48 comprovado de
semântica herdada do TMProject 7.59/7.69+ e exige uma ficha por transição.

Este handoff cobre somente a infraestrutura de pesquisa, suas duas fichas
iniciais e as regras que bloqueiam edição prematura. A paridade funcional e
visual já implementada possui estado próprio em
`.agents/handoffs/client748-parity.md`; não duplicar esse histórico aqui.

Nenhum código ativo do client ou servidor foi alterado neste escopo. Não houve
build, startup ou teste in-game.

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
- `LOCATED` autoriza somente pesquisa/documentação. Comportamento exige ficha
  `TRACED`; packet, wire, ABI, struct, offset, packing, signedness e loader
  exigem `CONTRACT`.
- `transport/packet-size-gate.md` e
  `ui/control-focus-ime-lifecycle.md` passaram no validador estrutural, mas
  permanecem `LOCATED` porque callers/callees indiretos, erros e lifecycle não
  foram completamente fechados no projeto Ghidra.
- `query_corpus.py stats --repo .` encontrou 4.146 funções e 108 funções
  nativas citadas no repositório. `FUN_00452733` e `FUN_0047E4D6` não existem
  no índice textual atual; essa ausência não prova ausência no binário.
- `FUN_004AFAC0` e `FUN_004AFBA0` gravam a vtable `0x005A45F0`, confirmando a
  instância nativa do `ObjectManager`. Nessa vtable, os slots `+0x60`, `+0x64`
  e `+0x68` apontam para `FUN_004B3500`, `FUN_004B37C9` e `FUN_004B3952`.
- `FUN_004B3500` localiza a transição de estado/cena: o estado solicitado `9`
  vira estado interno `0`; somente `0`, `5`, `7` e `8` instanciam cenas; o
  client reinicializa a câmera, registra a nova cena, inicializa-a pelo slot
  virtual `+0x4C`, destrói-a e fecha a janela em falha, ou a anexa à árvore em
  `manager+0x1B07C` por `FUN_0054AC09` em sucesso. Callers e teardown das quatro
  classes ainda não estão fechados, portanto o fluxo permanece `LOCATED`.
- `FUN_004B37C9` troca `DAT_0067CF38`, marca a cena anterior para deleção,
  transfere o estado de dois controles em `scene+0x84`, copia estado e tempo e
  chama novamente o slot `ObjectManager+0x64` com a cena anterior. O receptor
  da chamada final foi resolvido pelas instruções como o próprio manager; a
  semântica de encadeamento/teardown ainda exige os callers e o consumidor de
  `m_cDeleted` antes de promover a ficha.
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
skill wyd-client748-research             | STATICALLY VERIFIED | quick_validate.py e script headless passaram
infraestrutura e schema das fichas       | STATICALLY VERIFIED | scripts e template revisados; validador passou
gate de tamanho por opcode               | LOCATED             | entrada nativa localizada; caller/direção pendentes
foco, IME e lifecycle de controles       | LOCATED             | fluxo principal localizado; xrefs/teardown pendentes
transição e troca de cenas                | LOCATED             | criação/initialize/falha/registro localizados; callers e teardown pendentes
código ativo do client/servidor          | NÃO ALTERADO         | nenhum build ou teste funcional necessário nesta etapa
client748/project.exe no fluxo real      | NÃO TESTADO          | proibido declarar CLIENT-TESTED
```

## Worktree e arquivos ativos

- `AGENTS.md` — gate da pesquisa 7.48, política `gpt-5.6-sol/xhigh` quando
  disponível e proibição de contrato vindo do TMProject moderno.
- `client-source/AGENTS.md` — source de versão única e maturidade mínima antes
  de qualquer edição comportamental.
- `.agents/skills/wyd-go-feature/SKILL.md` — consome fichas já maduras; não
  promove hipótese do TMProject.
- `.agents/skills/wyd-client748-research/` — nova skill, referências, metadados
  e ferramentas reproduzíveis.
- `.agents/research/client748/` — README, template, dois exports focados e as
  duas fichas iniciais. Seis exports exploratórios amplos e não citados foram
  removidos da worktree e preservados temporariamente em
  `%TEMP%\wyd748-broad-exports-20260828`; são regeneráveis pelo projeto Ghidra.
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
resultado: exit 0; duas fichas válidas; LOCATED=2

python %USERPROFILE%/.codex/skills/.system/skill-creator/scripts/quick_validate.py .agents/skills/wyd-client748-research
resultado: exit 0; Skill is valid!

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

forward-test somente leitura com gpt-5.6-sol/xhigh
resultado: classificou a ficha como LOCATED, bloqueou edição e exigiu CONTRACT

hashes SHA-256 da referência e do candidato
resultado: 8AA2F918...15F593 e F8251714...A380B

git diff --check
resultado: exit 0; somente avisos informativos LF/CRLF do Git
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

## Próximo passo executável

1. Continuar o fluxo de cenas a partir de `FUN_004B3500` e `FUN_004B37C9` no
   projeto `WYD748Native_20260821.gpr`: resolver callers diretos/indiretos,
   vtables e destrutores das cenas criadas por `FUN_004343A4`, `FUN_0049EE30`,
   `FUN_004A8CCF` e `FUN_00431D00`.
2. Rastrear o consumidor de `TreeNode+0x14 = 1`, a remoção da cena antiga da
   árvore em `manager+0x1B07C`, logout/relogin e todos os caminhos de falha.
3. Comparar o fluxo integral com `ObjectManager.cpp/.h`, `TMScene.cpp/.h` e
   `TreeNode.cpp/.h`, então criar
   `.agents/research/client748/flows/lifecycle/scene-transition.md`.
4. Promover a ficha somente quando entrada, callers, callees, estado, erros,
   ownership e teardown estiverem fechados. Só depois adaptar o delta
   comprovado `estado 9 -> estado interno 0` na source recompilável.
5. Retomar `packet-size-gate.md` após o fluxo de cenas ou em campanha paralela;
   sua maturidade continua `LOCATED`, sem autorização para alterar wire/ABI.
6. Reexecutar `validate_research.py`, `query_corpus.py stats`,
   `quick_validate.py`, `git diff --check` e atualizar este handoff quando
   houver nova evidência ou mudança de estado.

## Critérios de aceite pendentes

- Outra sessão consegue reproduzir cada claim sem recorrer à conversa.
- Nenhuma edição comportamental começa com ficha `LOCATED`.
- Toda decisão futura separa comportamento 7.48 confirmado, ABI/recurso
  confirmado, semântica moderna apenas sugerida e lacunas bloqueantes.
- `CLIENT-TESTED` aparece somente depois do fluxo real no hash registrado de
  `client748/project.exe`.
