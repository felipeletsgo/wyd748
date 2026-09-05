# WYD-Go

## Objetivo e versão

Emulador Go server-authoritative para o client WYD 7.48. O client envia
intenções e apresenta estado; nunca é fonte de verdade.

`client-source/tmproject` produz o único client ativo deste ecossistema 7.48,
mas não é uma réplica arqueológica do executável nativo. O 7.48 histórico é a
baseline dos contratos legados; estruturas, funções e assets posteriores ou
implementados manualmente podem permanecer quando forem superiores e
compatíveis. Como client e servidor pertencem ao mesmo projeto, extensões
coordenadas são permitidas quando o contrato novo é explícito e testado nos
dois lados.

Não manter duas implementações apenas por versão. Para cada delta, escolher um
único caminho ativo e classificá-lo como `PARIDADE_NATIVA`,
`MODERNIZACAO_COMPATIVEL` ou `EXTENSAO_COORDENADA`. Ausência no nativo 7.48,
isoladamente, nunca autoriza remover código ou asset existente.

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
5. classificar o delta nos três modos acima e localizar o fluxo vivo com `rg`;
6. carregar somente as referências indicadas pela skill para o assunto atual;
7. se tocar uma fronteira legada do client, reutilizar ou concluir a ficha 7.48
   no estado exigido; se for extensão coordenada, documentar o contrato novo e
   inspecionar no Ghidra apenas as fronteiras nativas que ela intercepta;
8. implementar, validar e registrar o estado real da validação.

Se uma skill obrigatória estiver ausente ou ilegível, informar a limitação. Não
fingir que ela foi consultada nem declarar a tarefa concluída.

Em continuação, não reiniciar a investigação. Handoff, `git status`, diff
scoped e fingerprints dos inputs bastam para retomar o próximo passo. Não
reler referência já lida, recalcular hash imutável, rerodar triagem global ou
revalidar artefato não alterado na mesma sessão. Repetir uma verificação apenas
quando seu input mudou, a evidência registrada é insuficiente ou ela é gate da
alteração atual.

### Proteção contra loops de continuidade

- Compactação é um reset de contexto, não um pedido para reiniciar a tarefa.
  Usar o handoff e avançar diretamente para o próximo comando executável.
- Em uma retomada, fazer no máximo uma checagem curta de `status + diff scoped`.
  Não usar `list_threads`, `read_thread` ou busca ampla para recuperar a mesma
  sessão quando o handoff e a árvore já identificam o escopo.
- Depois da checagem inicial, o próximo passo deve ser `apply_patch`, teste
  focado ou uma pergunta de bloqueio. Duas rodadas consecutivas sem patch,
  teste ou evidência nova encerram a tentativa; relatar o bloqueio em vez de
  repetir a inspeção.
- Mensagem de progresso não conta como avanço. Cada ciclo deve produzir uma
  alteração, uma validação nova ou uma causa de bloqueio verificável.
- Uma interrupção encerra o ciclo atual. Só retomar após nova solicitação
  explícita do usuário, e sem reemitir o plano ou as inspeções já concluídas.

## Roteamento de skills

| Escopo | Skill obrigatória | Referência adicional |
| --- | --- | --- |
| Qualquer comportamento do servidor/client WYD | `.agents/skills/wyd-go-feature/SKILL.md` | Para escopo client/contrato, `wyd-client748-research` vem antes |
| Client, packet, ABI, UI, input, render, asset ou lifecycle | `wyd-client748-research` → `wyd-go-feature` | classificar o delta; paridade usa `TRACED`/`CONTRACT`, extensão usa contrato client/server explícito |
| Paridade ou integração legada de UI, HUD, grid, inventário, equipamento ou mensagens | `wyd-client748-research` → `wyd-go-feature` | `references/ghidra-client748.md` e `references/client-ui-748.md` |
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

## Programa de cobertura e migração do client 7.48

O objetivo de mapear as 4.146 funções deve ser alcançado por catálogo,
callgraph e fluxos observáveis, e não por tradução linear de pseudocódigo ou
por análise isolada de uma função. `functions.tsv` é o censo do corpus; uma
entrada catalogada não é uma função compreendida.

Para claims de paridade nativa e mudanças em fronteiras legadas, o pipeline é:

```text
catálogo -> callgraph -> fluxo observável -> adaptação -> validação
```

Cada seta é um gate de paridade: o catálogo não substitui xrefs; o callgraph não substitui
estado/lifecycle; a semelhança com o TMProject não substitui a adaptação
decidida por claim; build não substitui validação do fluxo. Em lifecycle,
`TRACED` ou superior exige entrada observável, matriz de transições,
vtables/vptrs/receptores, ownership, falha parcial, cleanup/teardown, shutdown
e logout/relogin resolvidos ou marcados como não aplicáveis com justificativa.

Uma nova raiz de paridade segue três trilhas:

1. `wyd-client748-catalog`: reutilizar o censo/hash válidos, atualizar a
   triagem somente se seus inputs mudaram e escolher raízes por subsistema,
   fan-out, referências na source e callbacks;
2. `wyd-client748-research`: rastrear uma transição observável completa no
   projeto Ghidra, com callers, callees, estado, efeitos, erros e teardown;
3. `wyd-go-feature`: adaptar o delta comprovado, ou implementar uma extensão
   coordenada separada do claim nativo, validar e registrar o resultado.

Na migração, o caminho inverso é preferencial: partir de uma função ou feature
viva da source, localizar a candidata nativa e propagar pelos vizinhos do
callgraph. Seeds já confirmados, vtables e correlações em lote devem ser
reutilizados. Não pesquisar as 4.146 funções em sequência antes de fazer o
client funcionar; fechar primeiro o caminho crítico
bootstrap → login → cenas → dispatcher → mundo → logout/relogin e implementar
lotes pequenos assim que cada fronteira estiver decidida.

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

Uma ficha `LOCATED` bloqueia somente a edição que depende de um claim de
paridade ainda não rastreado. Ela não bloqueia uma modernização interna que
preserva um contrato já comprovado nem uma extensão coordenada que não se
apresenta como comportamento nativo. TMProject 7.69+ pode fornecer arquitetura,
algoritmos e assets candidatos; nunca fornece, por semelhança, endereço, offset,
layout, recurso ou packet do 7.48. Uma função auxiliar sem impacto observável
pode ser catalogada sem tradução artificial.

Código e assets manuais existentes são trabalho intencional do usuário.
Preservá-los por padrão e só remover, substituir ou rebaixar após demonstrar
incompatibilidade em wire/ABI, recurso, lifecycle, server-side ou fluxo real.

### Regra de seleção de UI posterior

Quando uma função disponível no TMProject 7.69+ não existir no nativo 7.48,
investigar primeiro se o próprio TMProject já possui uma UI compatível e
reutilizável. Se não houver, comparar adaptações de UIs existentes com a mesma
semântica — por exemplo Odin (7 slots) ou Composer (6 slots) — considerando
recursos, grids, callbacks, input, ownership, fechamento e contrato client/
server. Só criar uma UI nova ou remover/desabilitar a função depois de registrar
por que as alternativas existentes não são seguras ou úteis para o 7.48. A
ausência no nativo, sozinha, não é motivo para remoção.

### Política Git desta campanha

O trabalho desta campanha é feito diretamente em `main`. Não criar branches,
worktrees ou PRs para dividir o mapeamento. Cada commit deve conter uma unidade
documental, de pesquisa ou de implementação verificável e deve ser publicado
em `origin/main` quando a validação correspondente estiver concluída.

## Gate Ghidra e extensões coordenadas

Mudança de paridade nativa ou de uma fronteira legada em `client-source/`,
protocolo, ABI, structs, UI, input, render ou assets exige, antes da edição:

- usar `.agents/skills/wyd-client748-research/SKILL.md` e registrar a ficha do
  fluxo; `TRACED` é o mínimo para comportamento e `CONTRACT` para wire/ABI,
  structs, offsets, packing, signedness ou loaders;
- ler `wyd-go-feature/references/ghidra-client748.md`;
- confirmar a identidade do executável analisado; reutilizar o SHA-256
  registrado se caminho, tamanho e mtime não mudaram;
- registrar funções nativas, callers/callees e lifecycle relevantes;
- comprovar offsets, tamanhos, packing e signedness quando houver wire/ABI;
- comparar o comportamento nativo com a source atual;
- separar semântica portada de layout/ABI deliberadamente não portados.

Para `MODERNIZACAO_COMPATIVEL`, provar que o contrato externo e o lifecycle
afetados permanecem equivalentes e reutilizar a ficha existente. Para
`EXTENSAO_COORDENADA`, registrar formato, ownership, IDs/opcodes, fallback,
teardown, rollback/relogin e testes client/server. Consultar o Ghidra somente
para verificar colisões e pontos de integração com o legado; não exigir que o
binário nativo prove uma feature que deliberadamente não possuía.

Antes de escolher a estrutura 7.48 ou a posterior, comparar wire/ABI, assets
materializados, lifecycle/ownership, suporte server-side, manutenção e
evolução coordenada. Preferir a estrutura tecnicamente superior quando esses
limites forem compatíveis. Se necessário, adaptar ambos os lados em um único
contrato, sem manter branches históricos paralelos.

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

Controle herdado sem ID/recurso carregado pode ser legitimamente nulo. Proteger
acessos e preservar a transição principal. Um controle moderno só deve ser
materializado como extensão deliberada, com recurso, binding, lifecycle e teste
completos; nunca apenas para mascarar um ponteiro nulo.

## Invariantes globais

- `World` é o único escritor de gameplay.
- `Score` é autoritativo; wire legado nunca volta ao cálculo.
- Toda intenção do client é revalidada no servidor.
- Anti-dupe: validar → mutar cópia → persistir → publicar.
- A feature sincroniza dono, observers, visibilidade e persistência.
- Consultas locais usam índice espacial; `Merchant != 0` nunca entra em combate.
- Inventário: 64 slots, 63 utilizáveis; cargo: 128 slots, 120 utilizáveis.
- Handlers roteiam; regra de negócio fica no módulo da feature.
- Não copiar ABI, structs, offsets, endereços ou packets de outra versão para
  uma fronteira legada. Contrato coordenado novo deve ser projetado e testado,
  não inferido por cópia.
- A source é única. Estruturas posteriores podem tornar-se o único caminho
  ativo quando compatíveis; não manter branches mortos de múltiplas versões.

## Edição e validação

Toda decisão não óbvia de contrato, compatibilidade ou ownership deve deixar
comentário próximo ao trecho alterado. Edição mecânica ou autoexplicativa não
exige comentário artificial; comentários que repetem a operação são proibidos.

### Documentação didática do código

O client é também material de estudo do projeto. Todo arquivo do client criado,
convertido, adaptado ou funcionalmente alterado deve sair do lote com
documentação em português suficiente para explicar o que existe e por que:

- resumir no próprio arquivo a responsabilidade da unidade e sua relação com o
  fluxo do client;
- documentar classes, estruturas, métodos e campos públicos, incluindo
  parâmetros, retorno, nulabilidade e ownership quando relevantes;
- comentar no corpo as fases, invariantes, efeitos colaterais, fallbacks e
  decisões que não sejam evidentes pela leitura do código;
- registrar na ficha ou documentação do lote a origem conhecida (`7.48`,
  `TMProject 7.69+` ou implementação local), a classificação do delta e a
  evidência usada; não atribuir origem nativa sem prova;
- documentar placeholders como tal e a evidência ainda necessária, sem criar
  comportamento apenas para preencher um arquivo vazio.

Comentários devem complementar nomes claros, não traduzir cada instrução nem
congelar suposições. Identificadores, opcodes e termos canônicos podem permanecer
em inglês; a explicação ao mantenedor deve ser em português. A migração da
documentação é incremental: ao tocar um arquivo, atualizar também comentários
obsoletos no trecho afetado, sem reformatar em massa código alheio.

Refatorar durante a adaptação quando isso reduzir duplicação, explicitar
ownership/lifecycle ou remover ambiguidade verificável. A refatoração deve
preservar comportamento, wire, ABI, packing, offsets, recursos e ordem de
efeitos; qualquer mudança observável é um delta separado e segue seu gate de
evidência.

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

Retomar diretamente do próximo comando ou símbolo registrado. Hashes, exports,
triagem e validações anteriores são cacheáveis por seus inputs: se os inputs
não mudaram, reutilizar o resultado e avançar. Executar apenas validações cujas
fronteiras foram alteradas; a suíte ampla continua sendo gate de integração,
não ritual de cada edição documental.

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
