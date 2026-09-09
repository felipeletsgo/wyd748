# Source-built WYD client

Ler primeiro o `AGENTS.md` da raiz. Classificar cada delta como
`PARIDADE_NATIVA`, `MODERNIZACAO_COMPATIVEL` ou `EXTENSAO_COORDENADA` e usar
`wyd-client748-research` antes de `wyd-go-feature`. Em continuidade, abrir
somente o handoff correspondente, conferir o diff scoped e retomar do próximo
símbolo/comando; não repetir pesquisa cujos inputs continuam idênticos.

Esta arvore e a unica implementacao ativa do client. Ela pode conter melhorias
manuais, funções e assets ausentes no nativo 7.48. Preservar esse trabalho por
padrão: ausência no binário histórico não é defeito. Paridade funcional e
visual só deve ser alegada após prova no `client748/project.exe`; o binário
histórico nunca volta a ser produto ou fallback.

## Modos de mudança e evidência

Para `PARIDADE_NATIVA` ou mudança de fronteira legada:

- criar ou atualizar a ficha da transicao em
  `.agents/research/client748/flows/`;
- reutilizar o SHA-256 e a ficha existentes quando seus inputs não mudaram;
  reabrir no Ghidra somente a entrada, os callers/callees e o lifecycle que
  decidem o delta atual;
- exigir ficha `TRACED` para comportamento, UI, input, render, assets e
  lifecycle;
- exigir ficha `CONTRACT` para packet, wire, ABI, struct, offset, packing,
  signedness ou loader;
- manter em pesquisa a edição que depende do claim enquanto ele estiver
  `UNMAPPED` ou `LOCATED`.

Para `MODERNIZACAO_COMPATIVEL`, partir da source atual e provar apenas que wire,
ABI, recursos e lifecycle externos continuam equivalentes; reutilizar a
evidência nativa já fechada. Para `EXTENSAO_COORDENADA`, a ausência no 7.48 é
esperada: documentar o contrato novo, implementar client e servidor quando
necessário e testar sucesso, rejeição, teardown e relogin. O Ghidra limita-se
aos pontos em que a extensão intercepta ou pode colidir com o legado.

O corpus textual exportado e apenas um acelerador. Funcao ou caller ausente do
indice precisa ser resolvido no projeto Ghidra ou permanecer `nao confirmado`.
TMProject 7.59/7.69+ pode fornecer uma estrutura interna superior e assets
candidatos. Ele não prova fatos do nativo 7.48, mas sua implementação pode ser
adotada como caminho único quando as fronteiras forem compatíveis ou quando um
contrato coordenado substituir deliberadamente ambos os lados.

## Implementação única

Esta source compila o client customizado deste ecossistema 7.48. Ela não deve
conservar duas implementações por versão nem funcionar como client multi-versão.
Isso não obriga a escolher a estrutura mais antiga.

- para paridade, confirmar o caminho nativo no Ghidra do binario historico;
- comparar a opção 7.48, a source atual e a implementação posterior por
  compatibilidade, manutenção, lifecycle, assets e suporte server-side;
- escolher uma única implementação ativa, preferindo a estrutura superior
  quando ela preserva ou substitui de forma coordenada o contrato externo;
- remover branch legado ou moderno apenas após provar que não é o caminho
  selecionado nem uma melhoria manual ativa;
- eliminar flags `compat` quando todos os seus usos no subsistema tiverem sido
  reduzidos ao caminho ativo selecionado;
- permitir features e dados novos como extensões explícitas; quando houver
  compatibilidade com peers legados, traduzir na fronteira em vez de duplicar
  toda a arquitetura interna.

Nao fazer exclusao em massa por heuristica. Código e assets manuais são
presumidos intencionais. Cada corte precisa registrar qual incompatibilidade ou
caminho substituto o justifica e passar pelo build e pelo fluxo real afetado.

### Regra para UIs do TMProject 7.69+

Para uma função do TMProject 7.69+ ausente no nativo 7.48, pesquisar primeiro
uma UI compatível já existente no TMProject. Na falta dela, avaliar a adaptação
de uma UI de composição existente, como Odin (7 slots) ou Composer (6 slots),
comparando recurso, quantidade e semântica dos slots, callbacks, input,
ownership, fechamento e contrato com o servidor. Uma UI nova só pode ser
criada após essa comparação; remover ou desabilitar a função só pode ser a
última opção documentada quando nenhuma adaptação for tecnicamente segura e a
função não tiver utilidade no 7.48.

## Contratos

- Toda decisão não óbvia de contrato, compatibilidade ou ownership deve possuir
  comentário explicando o "por que". Não adicionar comentário a uma edição
  mecânica apenas para satisfazer processo.
- Todo arquivo criado, convertido, adaptado ou funcionalmente alterado deve
  possuir comentários didáticos em português sobre sua responsabilidade; suas
  classes, estruturas e APIs públicas; e os parâmetros, retornos, ownership,
  nulabilidade e lifecycle que não estiverem claros no tipo.
- No corpo das funções, comentar fases, invariantes, efeitos colaterais,
  fallbacks e decisões relevantes. Não narrar linha por linha nem repetir o que
  nomes e tipos já dizem.
- Registrar fora do código, na ficha ou documento do lote, a origem conhecida
  (`7.48`, `TMProject 7.69+` ou local), o modo do delta e a evidência. Não
  transformar hipótese de procedência em comentário factual.
- Placeholders e stubs devem declarar sua condição e a lacuna de evidência;
  não inventar implementação para fazê-los parecer completos.
- Refatorar no mesmo lote somente quando melhorar compreensão, ownership,
  lifecycle ou duplicação sem alterar comportamento, ABI, wire, recursos ou
  ordem de efeitos. Mudança observável deve ser separada e passar pelo gate
  correspondente.
- A documentação do legado é incremental por arquivo tocado. Corrigir
  comentários obsoletos no trecho afetado, mas não promover reformatacão ou
  renomeação em massa de código alheio.
- `Score` no servidor continua autoritativo.
- Estruturas C++ e packets devem possuir `static_assert`/testes de tamanho e
  offsets; nunca depender de `sizeof` implicito sem teste do wire.
- Campos recebidos do client sao intencoes e nunca restauram estado de score.
- Manter Win32 durante a fase de compatibilidade. Uma migracao x64 altera ABI,
  alinhamento e dependencias e deve ser um projeto separado.
- Alteracoes visuais e importacao de assets continuam seguindo
  `client748/skills/wyd-client-assets/SKILL.md` e suas evidencias.
- Nao copiar structs, offsets ou enderecos de W2PP/Secrets/Micronics para o
  contrato legado. Um contrato novo deve ser projetado nesta source e no Go,
  com asserts/testes dos dois lados.
- O build deve passar exclusivamente por `tmproject/Build-Client.ps1`; nao
  versionar a pasta `build/` nem aceitar compilacao manual como candidato.

## Artefato e implantacao

- `tmproject/Build-Client.ps1` produz o artefato transitório em
  `build/<Configuration>/` e, em todo build bem-sucedido, instala e confere
  automaticamente `client748/project.exe`, o único candidato executável e de
  validação. Falha de cópia ou divergência de hash deve falhar o build.
- `client748/wyd.exe nativo+patches/` guarda somente binarios e scripts
  historicos para estudo/Ghidra; nao executar nem editar seu conteudo.
- Toda melhoria ativa deve ser implementada nesta source ou nos assets, com
  evidência nativa para paridade ou contrato client/server para extensão; nunca
  transportada como patch ou offset binario.

## UI 7.48

Mudança de paridade ou integração legada em HUD, janela, mensagem, input, grid,
inventário, equipamento, drag ou fechamento deve ler, nesta ordem:

```text
.agents/skills/wyd-go-feature/references/ghidra-client748.md
.agents/skills/wyd-go-feature/references/client-ui-748.md
```

Registrar funções nativas, recursos/IDs, lifecycle e diferenças da source que
decidem a fronteira. Para UI coordenada nova, registrar em vez disso recurso,
binding, input, ownership, teardown e integração. Screenshot serve para
composição visual; não substitui o fluxo real.

Tratar como regressões bloqueantes no client 7.48:

- footprint lógico e escala visual são contratos distintos. Inventário/cargo
  1×1 devem conter e centralizar a malha numa célula; equipamento e seu drag
  usam a região irregular inteira. Nunca reaproveitar cegamente a escala de um
  contexto no outro;
- ao redimensionar um painel nativo, transformar também os filhos que usam suas
  coordenadas relativas. Esticar apenas o background desloca ou amontoa textos,
  progressos e botões;
- não agrupar janelas porque seus nomes ou funções parecem relacionados. Cada
  root confirmado no recurso/Ghidra mantém lifecycle, owner e toggle próprios;
- uma comparação visual só é aprovada contra screenshot da mesma resolução ou
  após normalizar a geometria pela escala do viewport. Registrar separadamente
  `STATICALLY VERIFIED`, build/startup e `CLIENT-TESTED` dentro do mundo.

Os IDs, funções e fórmulas atualmente confirmados para grid, HUD de quatro
quartos e painéis Skill estão em
`.agents/skills/wyd-go-feature/references/client-ui-748.md`; reabrir a evidência
Ghidra antes de alterar o contrato.

Um campo de controle herdado pode nao ter ID correspondente no recurso
carregado. Se o widget nao existir, tratar o ponteiro como opcional, proteger os
callers e manter a transicao principal. Uma UI moderna pode permanecer ou ser
criada como extensão deliberada se recurso, binding, ownership, input e
teardown forem implementados; não criá-la apenas para evitar crash.

Migrar um grupo pequeno de packets por vez e validar cada grupo no
`client748/project.exe` com owner e observer. O binario historico nao e produto,
fallback nem alvo executavel. Código posterior não permanece como caminho
alternativo por versão; quando escolhido, torna-se a implementação ativa e é
adaptado às fronteiras do ecossistema.
