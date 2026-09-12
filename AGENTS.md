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

O contrato client/server deve ser explícito, versionado e testável nos dois
lados. A existência de arquitetura mais completa no TMProject 7.69 não prova
que um endereço, layout, asset, opcode ou comportamento pertence ao 7.48.

## Regra de versão e evidência

Todo delta deve ser classificado como exatamente um destes modos:

- `PARIDADE_NATIVA`: comportamento comprovado do client nativo 7.48;
- `MODERNIZACAO_COMPATIVEL`: melhoria interna que preserva o contrato 7.48;
- `EXTENSAO_COORDENADA`: contrato novo implementado no client e no servidor.

Para comportamento, protocolo, ABI, structs, UI, input, render, assets ou
lifecycle, consultar primeiro o binário nativo 7.48 e o estudo no Ghidra. O
TMProject 7.69 é referência secundária de arquitetura e algoritmos. W2PP,
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

## Organização de arquivos

- Nenhum `.md`, script de pesquisa, dump, log, binário gerado ou arquivo
  temporário deve ser criado dentro de `tmproject/` ou `wydgo748/` sem ser
  parte necessária do produto. Documentação vai para `DOCS/`; evidência de
  agente vai para `.agents/research/` ou `.agents/handoffs/`.
- Manter nomes e caminhos das duas sources claros: não reintroduzir
  `source-client`, `source-server` ou um terceiro projeto `clientgo748`.
- Builds e artefatos locais devem ficar em diretórios ignorados (`build/`,
  `bin/`, `obj/` ou equivalentes já previstos no `.gitignore`). Não comitar
  executáveis gerados.
- Ao mover documentação, corrigir links e registrar a mudança no mapa central.
  Não duplicar o mesmo documento em dois lugares.

## Validação mínima

Do root do repositório:

```powershell
Push-Location .\wydgo748
go test -count=1 ./...
Pop-Location
```

Do client:

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
