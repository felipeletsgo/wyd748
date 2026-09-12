# WYD-Go — regras únicas do repositório

Este é o único arquivo de regras do repositório. Não criar outro `AGENTS.md`,
`CLAUDE.md` ou arquivo equivalente dentro de uma source. Regras históricas
ficam arquivadas como documentação, mas não são instruções ativas.

## Arquitetura ativa

- `tmproject/` é o client C++: `TMProject748/` contém a source baseada no
  TMProject 7.69 e `tmproject/client748/` contém runtime e assets do
  client 7.48.
- `wydgo748/` é o servidor Go e seu `go.mod` é a raiz do módulo. O servidor é
  autoritativo; o client envia intenções, recebe estado validado e nunca é a
  fonte da verdade do jogo.
- `DOCS/` contém a documentação durável do projeto. A entrada e o inventário
  estão em [`DOCS/README.md`](DOCS/README.md) e
  [`DOCS/documentation-map.md`](DOCS/documentation-map.md).
- `.agents/skills/` contém somente skills de trabalho. `.agents/research/` e
  `.agents/handoffs/` contêm evidências e registros de continuidade, não
  runtime nem documentação espalhada nas sources.
- `references/client748/` preserva binários e ferramentas históricos como
  evidência somente leitura. `tools/repository/` concentra validadores do layout.

O contrato client/server deve ser explícito, versionado e testável nos dois
lados. A existência de arquitetura mais completa no TMProject 7.69 não prova
que um endereço, layout, asset, opcode ou comportamento pertence ao 7.48.

## Regra de versão e evidência

Cada delta funcional deve ser classificado uma vez como um destes modos;
documentação, organização e tooling sem mudança de contrato não exigem ficha nativa:

- `PARIDADE_NATIVA`: comportamento comprovado do client nativo 7.48;
- `MODERNIZACAO_COMPATIVEL`: melhoria interna que preserva o contrato 7.48;
- `EXTENSAO_COORDENADA`: contrato novo implementado no client e no servidor.

Para adaptar uma fronteira legada do client ou afirmar paridade, usar evidência
do binário nativo 7.48 e do Ghidra antes da edição. Reutilizar fichas comprovadas
quando cobrirem o contrato e os inputs atuais; investigar apenas a lacuna.
Modernização interna com contrato preservado não exige nova pesquisa nativa.
Extensão coordenada exige contrato novo e estudo das fronteiras legadas que
intercepta, não um equivalente nativo para a novidade. Mudança interna do
servidor, documentação ou build sem alteração dessas fronteiras não aciona Ghidra.
O TMProject 7.69 é referência secundária de arquitetura e algoritmos. W2PP,
Secrets e Micronics são fontes excluídas desta campanha e não podem orientar,
validar ou contradizer uma decisão.

O catálogo usa os estados `UNMAPPED`, `LOCATED`, `TRACED`, `CONTRACT`,
`IMPLEMENTED` e `CLIENT_TESTED`. Uma função catalogada não está compreendida
por isso. `CLIENT_TESTED` exige execução real do fluxo no client construído;
build, teste estático ou teste isolado não substituem essa validação.

## Regras de implementação

1. Antes de uma tarefa técnica não trivial, ler a skill aplicável em
   `.agents/skills/`, verificar este arquivo, `git status --short` e `HEAD`, e
   preservar mudanças alheias.
2. Para uma fronteira legada do client, seguir
   `catálogo -> callgraph -> fluxo observável -> adaptação -> validação`.
   Não portar pseudocódigo do 7.69 às cegas.
3. Para features que atravessam os dois projetos, definir o packet/estado,
   validação autoritativa, falhas e lifecycle antes de conectar UI ou efeitos.
4. Alterações ativas do client são feitas em source e assets. Binários
   históricos em `references/client748/` são somente
   evidência; não usar patches binários como implementação.
5. Fazer ciclos pequenos: patch focado, teste focado, próximo patch. Não
   refatorar código não necessário para a tarefa.
6. Não apagar ou substituir código/asset existente apenas porque não existe no
   nativo 7.48; demonstrar incompatibilidade no wire, ABI, recurso, lifecycle
   ou fluxo real antes de remover.

## Execução sem loops

- Fazer uma única entrada por tarefa: regras/skills aplicáveis, `status + HEAD
  + diff scoped` e handoff apenas quando for continuação. Reutilizar leituras
  já feitas nesta sessão; carregar referências somente pela decisão atual.
- Depois da entrada, avançar com patch, teste focado ou diagnóstico de uma
  lacuna concreta. Não reiniciar inventário, pesquisa ou auditoria a cada patch,
  troca de skill ou compactação. A árvore atual prevalece sobre o handoff.
- Reutilizar evidência com inputs, dependências, ambiente relevante e cobertura
  conhecidos e ainda válidos. Repetir apenas a parte invalidada por mudança,
  resultado insuficiente ou gate de integração. Hash de corpus imutável não é
  ritual de retomada; falha anterior nunca equivale a aprovação.
- Uma investigação deve fechar a transição necessária à tarefa, não o corpus
  inteiro. Ao atingir o gate dessa fronteira, implementar o lote liberado.
  Manter lacunas independentes registradas sem bloquear trabalho seguro.
- Duas tentativas consecutivas sobre a mesma dúvida sem evidência nova exigem
  mudar a hipótese/método com motivo concreto ou relatar o bloqueio. Não repetir
  comando falho sem corrigir input/ambiente ou identificar causa transitória.
  Pesquisa que produz evidência nova continua; não confundir duração com loop.
- Atualizar o registro existente quando mudar decisão, validação ou próximo
  passo. Não criar relatório/handoff por patch nem copiar logs para a source.

## Organização de arquivos

- Nenhum `.md`, script de pesquisa, dump, log, binário gerado ou arquivo
  temporário deve ser criado dentro de `tmproject/` ou `wydgo748/` sem ser
  parte necessária do produto. Documentação vai para `DOCS/`; evidência de
  agente vai para `.agents/research/` ou `.agents/handoffs/`.
- Manter somente as duas sources ativas: `tmproject/` e `wydgo748/`.
- Builds e artefatos locais devem ficar em diretórios ignorados (`build/`,
  `bin/`, `obj/` ou equivalentes já previstos no `.gitignore`). Não comitar
  executáveis gerados.
- Ao mover documentação, corrigir links e registrar a mudança no mapa central.
  Não duplicar o mesmo documento em dois lugares.

## Skills por escopo

| Escopo | Skill |
| --- | --- |
| Organização, documentação, regras e skills | `repo-architecture` |
| Servidor e features integradas | `wydgo748-feature` |
| Contratos e comportamento legado do client | `wyd-client748-research`; adicionar `wydgo748-feature` se afetar servidor/contrato integrado |
| Censo nativo e seleção de raízes | `wyd-client748-catalog` |
| Assets e recursos do runtime | `wyd-client748-assets` |

As skills detalham procedimentos; este arquivo concentra as regras globais.

## Validação proporcional

Escolher o gate pelo risco e pelos consumidores alterados, não pela quantidade
de arquivos. Teste focado acompanha cada patch lógico; validação integrada roda
no fechamento do lote afetado, sem repetir resultado ainda válido. Falha,
dependência compartilhada ou cobertura insuficiente justifica ampliar o gate.

| Alteração | Gate necessário |
| --- | --- |
| Apenas documentação/regras/skills | Links/layout uma vez no lote; validar skills alteradas; `git diff --check`. Sem Go, C++, Ghidra ou runtime. |
| Script ou build/CI | Testar o caminho alterado e uma falha relevante; build do produto somente se sua produção/resolução de dependências mudou. |
| Servidor interno | Testes do pacote/fluxo alterado; incluir consumidores. Suíte Go completa e vet no lote transversal ou de integração, não a cada patch. |
| Source C++ | Teste focado aplicável e compilação incremental do alvo afetado; build integrado no lote que altera o executável/projetos. Rebuild limpo só se necessário para dependências/configuração. |
| Wire/ABI ou feature coordenada | Contrato e testes dos dois lados, incluindo rejeições/tamanhos/falhas; builds afetados e fluxo integrado para declarar funcionamento ponta a ponta. |
| Persistência/economia/concorrência | Testar também rejeição, rollback, repetição e concorrência aplicáveis; usar banco/race quando esse caminho depender deles. |
| Apenas assets | Identidade/formato/IDs e loader dos recursos alterados; fluxo visual no client. Recompilar somente se houver geração, empacotamento ou dependência de build afetada. |

Testes ignorados por fixture/serviço ausente não comprovam o caminho. Registrar
o gate pendente; não promover a entrega a validada nem repetir a suíte esperando
que o pré-requisito apareça. Auditoria ampla e publicação não são etapas
automáticas de uma edição comum.

### Comandos de integração

Do root do repositório:

```powershell
Push-Location .\wydgo748
go test -count=1 ./...
Pop-Location
```

Para construir e instalar o candidato no runtime local (sobrescreve
`tmproject/client748/project.exe`; usar somente quando a tarefa incluir essa
validação, não como simples checagem documental):

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File .\tmproject\Build-Client.ps1
```

O script deve produzir/verificar `tmproject/client748/project.exe` sem alterar
as evidências históricas. A validação deve declarar separadamente
`STATICALLY VERIFIED`, `AUTOMATED TESTED` e `CLIENT-TESTED`; nunca elevar o
estado apenas porque houve build.

## Git e entrega

Esta campanha trabalha diretamente em `main`; não criar branch, worktree ou PR
para dividir o mapeamento. Não executar `git reset --hard`, `git checkout --`
ou remoção ampla. Antes de uma exclusão, resolver os caminhos exatos e
preservar o que for evidência. Ao concluir, reportar arquivos alterados,
arquivos removidos, comandos executados e qualquer gate ainda não validado.
