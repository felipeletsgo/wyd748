# WYD-Go

## Objetivo e versão

Emulador Go server-authoritative para o client WYD 7.48. O client envia
intenções e apresenta estado; nunca é fonte de verdade.

`client-source/tmproject` produz exclusivamente o client 7.48 deste
repositório. Não manter branches, controles, packets, layouts ou loaders
exclusivos de 7.54/7.59 por “compatibilidade futura”. Referências mais novas
podem fornecer semântica, mas qualquer dado novo deve ser projetado no formato
comprovado do 7.48.

## Política de modelo

Preferir `gpt-5.6-sol` com esforço `xhigh` quando o ambiente oferecer essa
seleção. Caso contrário, usar o maior nível disponível e nunca afirmar que um
modelo indisponível foi usado. Skills e subagentes não reduzem o rigor exigido.

## Ordem obrigatória de trabalho

Antes de qualquer tarefa técnica não trivial:

1. inspecionar `.agents/skills` e as skills oferecidas pela sessão;
2. ler integralmente o `SKILL.md` aplicável antes de analisar ou editar;
3. ler o `AGENTS.md` mais específico de cada subtree afetado;
4. verificar `git status --short` e preservar mudanças alheias;
5. localizar o fluxo vivo com `rg`, incluindo callers, callees e testes;
6. carregar somente as referências indicadas pela skill para o assunto atual;
7. se tocar client/protocolo/ABI/UI/input/render/asset/lifecycle, concluir a
   ficha de pesquisa 7.48 no estado exigido antes de editar;
8. implementar, validar e registrar o estado real da validação.

Se uma skill obrigatória estiver ausente ou ilegível, informar a limitação. Não
fingir que ela foi consultada nem declarar a tarefa concluída.

## Roteamento de skills

| Escopo | Skill obrigatória | Referência adicional |
| --- | --- | --- |
| Qualquer comportamento do servidor/client WYD | `.agents/skills/wyd-go-feature/SKILL.md` | Para escopo client/contrato, `wyd-client748-research` vem antes |
| Client, packet, ABI, UI, input, render, asset ou lifecycle 7.48 | `wyd-client748-research` → `wyd-go-feature` | ficha `TRACED`; `CONTRACT` para wire/ABI/loader |
| UI, HUD, grid, inventário, equipamento ou mensagens 7.48 | `wyd-client748-research` → `wyd-go-feature` | `references/ghidra-client748.md` e `references/client-ui-748.md` |
| Asset visual sob `client748/` | `wyd-go-feature` + `client748/skills/wyd-client-assets/SKILL.md` | `client748/AGENTS.md` |
| Hook nativo no plugin Micronics | `add-hook` | Usar `build-deploy` se houver build/deploy da DLL |
| Build/deploy de `FunctionsV02.dll` | `build-deploy` | Seguir a ordem kill/build/copy/start da skill |
| Histórico, fórmulas ou tooling WYD | `wyd-dev-knowledge` | Somente depois do código atual e da evidência 7.48 exigida |
| Auditoria ou revisão | `wyd-go-feature` | `references/audit.md` |
| Continuação entre sessões | `wyd-go-feature` | `references/session-continuity.md` + handoff do escopo |

Para GitHub, CI, PR, publicação ou integrações, usar também a skill específica
oferecida pela sessão. Uma skill genérica não substitui uma mais específica.

## Hierarquia de evidência

1. código atual do `wyd-go`;
2. testes atuais;
3. dados autoritativos em `data/`;
4. client WYD 7.48 e sua descompilação;
5. W2PP;
6. Secrets 7.54;
7. Micronics e outras referências compatíveis;
8. base histórica `wyd-dev-knowledge`.

Relatório, comentário, teste isolado, conversa anterior ou handoff são pistas,
não prova. Abrir a implementação atual e seguir o fluxo real. Contratos
técnicos detalhados ficam em
`.agents/skills/wyd-go-feature/references/repository-contracts.md` e devem ser
lidos apenas na seção indicada pela skill, não por padrão.

## Programa obrigatório de cobertura do client 7.48

O objetivo de mapear as 4.146 funções deve ser alcançado por catálogo,
callgraph e fluxos observáveis, e não por tradução linear de pseudocódigo ou
por análise isolada de uma função. `functions.tsv` é o censo do corpus; uma
entrada catalogada não é uma função compreendida.

O pipeline obrigatório é:

```text
catálogo -> callgraph -> fluxo observável -> adaptação -> validação
```

Cada seta é um gate: o catálogo não substitui xrefs; o callgraph não substitui
estado/lifecycle; a semelhança com o TMProject não substitui a adaptação
decidida por claim; build não substitui validação do fluxo. Em lifecycle,
`TRACED` ou superior exige entrada observável, matriz de transições,
vtables/vptrs/receptores, ownership, falha parcial, cleanup/teardown, shutdown
e logout/relogin resolvidos ou marcados como não aplicáveis com justificativa.

Toda retomada ou nova frente de pesquisa deve seguir três trilhas:

1. `wyd-client748-catalog`: conferir o hash, atualizar a triagem determinística
   e escolher raízes por subsistema, fan-out, referências na source e callbacks;
2. `wyd-client748-research`: rastrear uma transição observável completa no
   projeto Ghidra, com callers, callees, estado, efeitos, erros e teardown;
3. `wyd-go-feature`: adaptar no TMProject ou no WYD-Go somente o delta
   comprovado pela ficha `TRACED`/`CONTRACT`, validar e registrar o resultado.

As frentes devem ser priorizadas nesta ordem, salvo evidência que justifique
outra sequência: bootstrap/lifecycle, transporte e login, dispatcher e cenas,
tick/input, movimento, entidades, inventário/equipamento, NPC/lojas/trade,
combate/skills, UI e render/assets, shutdown/reconexão. Cada ficha cobre uma
transição; não criar uma ficha genérica que misture vários fluxos.

O catálogo deve distinguir `UNMAPPED`, `LOCATED`, `TRACED`, `CONTRACT`,
`IMPLEMENTED` e `CLIENT_TESTED`. O avanço da contagem só pode ocorrer quando a
evidência correspondente existir. Funções sem caller textual devem ser
revisadas no Ghidra como possíveis callbacks, vtables, thunks ou entradas
indiretas; zero resultados em um export não prova código morto.

Nenhum código do client, packet, ABI, UI, lifecycle ou comportamento observado
pode ser alterado enquanto a ficha do fluxo permanecer `LOCATED`. O TMProject
7.69+ só pode sugerir semântica; não pode fornecer contrato, endereço, offset,
layout, recurso ou packet do 7.48. Uma função auxiliar sem impacto observável
pode ser catalogada e classificada sem uma tradução artificial para aumentar a
cobertura.

### Política Git desta campanha

O trabalho desta campanha é feito diretamente em `main`. Não criar branches,
worktrees ou PRs para dividir o mapeamento. Cada commit deve conter uma unidade
documental, de pesquisa ou de implementação verificável e deve ser publicado
em `origin/main` quando a validação correspondente estiver concluída.

## Gate Ghidra do client 7.48

Qualquer mudança em `client-source/`, protocolo, ABI, structs, UI, input,
render, assets ou comportamento do executável exige, antes da primeira edição:

- usar `.agents/skills/wyd-client748-research/SKILL.md` e registrar a ficha do
  fluxo; `TRACED` é o mínimo para comportamento e `CONTRACT` para wire/ABI,
  structs, offsets, packing, signedness ou loaders;
- ler `wyd-go-feature/references/ghidra-client748.md`;
- confirmar o SHA-256 do executável analisado;
- registrar funções nativas, callers/callees e lifecycle relevantes;
- comprovar offsets, tamanhos, packing e signedness quando houver wire/ABI;
- comparar o comportamento nativo com a source atual;
- separar semântica portada de layout/ABI deliberadamente não portados.

Não corrigir por tentativa visual quando IDs, recursos, lifecycle ou condições
nativas podem ser recuperados da descompilação. Hipótese insuficiente deve ficar
marcada como `não confirmada`, nunca promovida silenciosamente a contrato.
Ausência de uma função ou caller no export textual também não prova ausência no
binário; resolver xrefs e chamadas indiretas diretamente no projeto Ghidra.

### Papéis dos executáveis

```text
client748/wyd.exe nativo+patches/WYDoriginal.exe  stock histórico imutável
client748/wyd.exe nativo+patches/WYD.exe          referência histórica Ghidra
build/.../WYD.exe                                saída transitória da source
client748/project.exe                            único candidato executável e de validação
```

Os scripts e patches em `client748/wyd.exe nativo+patches/` são somente material
de estudo: nunca executá-los, editá-los nem usá-los para produzir um client.
Toda alteração ativa deve ocorrer em `client-source/` ou nos assets. Toda
compilação deve passar por `client-source/tmproject/Build-Client.ps1`; o próprio
build deve instalar e validar automaticamente a saída recompilada como
`client748/project.exe`, sem patch binário.

Controle herdado da 7.59 sem ID/recurso no 7.48 pode ser legitimamente nulo.
Proteger acessos e preservar a transição principal; não fabricar widget moderno
para satisfazer um ponteiro.

## Invariantes globais

- `World` é o único escritor de gameplay.
- `Score` é autoritativo; wire legado nunca volta ao cálculo.
- Toda intenção do client é revalidada no servidor.
- Anti-dupe: validar → mutar cópia → persistir → publicar.
- A feature sincroniza dono, observers, visibilidade e persistência.
- Consultas locais usam índice espacial; `Merchant != 0` nunca entra em combate.
- Inventário: 64 slots, 63 utilizáveis; cargo: 128 slots, 120 utilizáveis.
- Handlers roteiam; regra de negócio fica no módulo da feature.
- Não copiar ABI, structs, offsets, endereços ou packets de outra versão.
- TMProject 7.69+ e referências posteriores são apenas fontes semânticas
  secundárias; não fornecem contrato 7.48 nem justificam branches de
  compatibilidade na source única.

## Edição e validação

Toda edição de código deve deixar comentário próximo ao trecho alterado
explicando intenção, contrato ou motivo técnico não óbvio. Comentários que apenas
repetem a operação não atendem à regra.

Em implementação ou correção:

- validar sucesso, rejeição, rollback e relogin quando aplicável;
- procurar caminhos alternativos que contornem a regra;
- conferir publicação a observers e persistência;
- executar testes focados antes da suíte ampla;
- rodar build/vet/testes compatíveis com o escopo e `git diff --check`;
- não apagar ou reformatar mudanças alheias numa worktree suja.

Classificar evidência sem inflar o resultado:

```text
IMPLEMENTED          código alterado, ainda sem validação suficiente
STATICALLY VERIFIED  inspeção/build estático concluído
AUTOMATED TESTED     teste automatizado relevante passou
CLIENT-TESTED        fluxo real executado no client afetado
```

Build verde não prova paridade visual ou comportamental. `CLIENT-TESTED` só
pode ser usado depois do fluxo real.

## Continuidade entre sessões

Handoffs em `.agents/handoffs/` guardam contexto operacional compacto. Eles não
substituem código, testes, Ghidra, dados autoritativos nem este arquivo.

Ao continuar trabalho existente:

1. ler somente o handoff do escopo e as skills roteadas;
2. comparar o handoff com `git status`, arquivos atuais e hashes baratos;
3. reutilizar apenas fatos ainda verificáveis;
4. atualizar o handoff quando houver nova evidência, alteração material,
   bloqueio ou ponto claro de retomada;
5. não copiar transcrições de chat para o handoff.

O formato e as regras completas ficam em
`.agents/skills/wyd-go-feature/references/session-continuity.md`. Use
`.agents/handoffs/TEMPLATE.md` para novos escopos. Handoff stale deve ser
corrigido ou marcado, nunca seguido cegamente.

## Gate de conclusão

Antes de concluir:

```text
[ ] skills e AGENTS scoped foram lidos
[ ] fluxo real, callers/callees e caminhos de erro foram inspecionados
[ ] gate Ghidra foi cumprido quando aplicável
[ ] mudanças alheias foram preservadas
[ ] comentários explicativos acompanham o código editado
[ ] validações obrigatórias foram executadas
[ ] estado de validação foi descrito com precisão
[ ] documentação/handoff afetado foi atualizado sem duplicar o código
```

Se algum item obrigatório estiver pendente, a tarefa não está concluída.
