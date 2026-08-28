# Source-built WYD client

Ler primeiro o `AGENTS.md` da raiz. Para qualquer comportamento, protocolo,
ABI, UI, input, render, asset ou lifecycle do client, usar nesta ordem
`wyd-client748-research` e `wyd-go-feature`. Em continuidade de trabalho, ler
também somente o handoff correspondente em `.agents/handoffs/` e verificar seus
fatos contra a worktree atual.

Esta arvore e a unica implementacao ativa do client. Paridade funcional e
visual ainda precisa ser comprovada no `client748/project.exe`, mas o binario
historico nao volta a ser produto ou fallback durante essa migracao.

## Gate de evidencia 7.48

Antes da primeira edicao de codigo que altere comportamento do client:

- criar ou atualizar a ficha da transicao em
  `.agents/research/client748/flows/`;
- confirmar o SHA-256 do `WYD.exe` historico analisado e reabrir no projeto
  Ghidra a entrada, os callers/callees e o lifecycle relevantes;
- exigir ficha `TRACED` para comportamento, UI, input, render, assets e
  lifecycle;
- exigir ficha `CONTRACT` para packet, wire, ABI, struct, offset, packing,
  signedness ou loader;
- manter a tarefa em pesquisa/documentacao enquanto a ficha estiver
  `UNMAPPED` ou `LOCATED`.

O corpus textual exportado e apenas um acelerador. Funcao ou caller ausente do
indice precisa ser resolvido no projeto Ghidra ou permanecer `nao confirmado`.
TMProject 7.59/7.69+ pode sugerir nomes e semantica, mas nunca fornece IDs,
recursos, offsets, layouts, packets, ABI ou lifecycle do 7.48.

## Versao unica

Esta source deve compilar exclusivamente o WYD 7.48. Ela nao deve conservar uma
segunda implementacao para TMProject 7.59+ nem funcionar como client
multi-versao.

- confirmar primeiro o caminho nativo no Ghidra do binario historico
  `client748/wyd.exe nativo+patches/WYD.exe`;
- portar o comportamento suportado para uma unica implementacao 7.48;
- remover o branch, recurso, opcode, struct ou loader exclusivo de versao mais
  nova assim que seus callers 7.48 estiverem cobertos;
- eliminar flags `compat` quando todos os seus usos no subsistema tiverem sido
  reduzidos ao caminho 7.48;
- manter sources posteriores somente como referencia externa de algoritmo;
- traduzir dados novos para a representacao 7.48, nunca manter o formato 7.59
  em paralelo dentro do executavel.

Nao fazer exclusao em massa por heuristica. Cada corte precisa registrar no
comentario da source qual contrato 7.48 substituiu o caminho removido e deve
passar pelo build e pelo fluxo real afetado.

## Contratos

- Toda alteracao nesta source deve possuir comentario explicativo junto ao
  codigo editado, registrando intencao, contrato e motivo da compatibilidade.
  Comentarios devem explicar o "por que", nao apenas repetir a operacao.
- `Score` no servidor continua autoritativo.
- Estruturas C++ e packets devem possuir `static_assert`/testes de tamanho e
  offsets; nunca depender de `sizeof` implicito sem teste do wire.
- Campos recebidos do client sao intencoes e nunca restauram estado de score.
- Manter Win32 durante a fase de compatibilidade. Uma migracao x64 altera ABI,
  alinhamento e dependencias e deve ser um projeto separado.
- Alteracoes visuais e importacao de assets continuam seguindo
  `client748/skills/wyd-client-assets/SKILL.md` e suas evidencias.
- Nao copiar structs, offsets ou enderecos de W2PP/Secrets/Micronics. Portar
  semantica e confirmar o layout nesta source e no servidor Go.
- O build deve passar exclusivamente por `tmproject/Build-Client.ps1`; nao
  versionar a pasta `build/` nem aceitar compilacao manual como candidato.

## Artefato e implantacao

- `tmproject/Build-Client.ps1` produz o artefato transitório em
  `build/<Configuration>/` e, em todo build bem-sucedido, instala e confere
  automaticamente `client748/project.exe`, o único candidato executável e de
  validação. Falha de cópia ou divergência de hash deve falhar o build.
- `client748/wyd.exe nativo+patches/` guarda somente binarios e scripts
  historicos para estudo/Ghidra; nao executar nem editar seu conteudo.
- Toda melhoria ativa deve ser implementada nesta source ou nos assets com
  evidencia 7.48, nunca transportada como patch ou offset binario.

## UI 7.48

Qualquer mudança em HUD, janela, mensagem, input, grid, inventário,
equipamento, drag ou fechamento deve ler, nesta ordem:

```text
.agents/skills/wyd-go-feature/references/ghidra-client748.md
.agents/skills/wyd-go-feature/references/client-ui-748.md
```

Registrar funções nativas, recursos/IDs, lifecycle e diferenças da source antes
da primeira edição. Screenshot serve para composição visual; não substitui o
fluxo recuperado no Ghidra.

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

Um campo de controle herdado da source 7.59 pode nao ter ID correspondente no
recurso carregado pelo 7.48. Confirmar a criacao/binding no Ghidra e procurar o
ID exato e sua conversao para a largura consumida pelo loader. Se o widget nao
existir, tratar o ponteiro como opcional, proteger todos os callers e manter a
transicao de estado principal. Nao criar UI moderna apenas para evitar crash.

Migrar um grupo pequeno de packets por vez e validar cada grupo no
`client748/project.exe` com owner e observer. O binario historico nao e produto,
fallback nem alvo executavel; serve somente para estudo/Ghidra. Tambem nao
preservar codigo 7.59 como caminho alternativo na source recompilavel.
