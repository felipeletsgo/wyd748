# WYD Client Go 7.48

Este diretório contém a preparação e o mapeamento autocontido do novo client
escrito em Go para o ecossistema WYD 7.48. A fundação compilável está em
`cmd/` e `internal/`; a documentação, o censo nativo, a descompilação
exportada, as fichas e os contratos necessários ficam em
[`MAPPING.md`](MAPPING.md) e `references/`.

O diretório possui seu próprio `go.mod`, ferramentas, corpus, projeto Ghidra,
fontes de consulta, dados e assets. Ele pode ser copiado para outro local e
validado sem o módulo Go da raiz nem arquivos em `.agents/`.
Registros históricos podem mencionar seus caminhos de origem; a equivalência
interna completa está documentada em
[`references/LOCAL_PATHS.md`](references/LOCAL_PATHS.md).

O `TMProject748` continua sendo o client ativo e a referência executável
durante a migração, mas uma cópia read-only da source usada para comparação já
está em `references/tmproject/`. O novo client Go não depende dela.

## Regra de reimplementação

O TMProject é usado somente como referência de organização de subsistemas,
dependências e nomes históricos. Nenhum código C++ dele deve ser copiado,
traduzido mecanicamente ou tratado como implementação confiável. Os bugs,
ownership implícito, estado global e lifecycle do TMProject não são contratos
do client Go.

Para cada componente novo, a ordem é:

1. confirmar a fronteira observável no código/testes atuais, nos assets 7.48 e
   na descompilação/binário nativo quando a fronteira for legada;
2. definir uma API Go pequena, tipada e testável;
3. implementar validação de entrada, ownership explícito, erros e teardown;
4. comparar o resultado com o contrato, e não com a forma do código C++;
5. manter o comportamento problemático como hipótese até haver evidência.

O design Go deve melhorar, quando compatível, o tratamento de nulabilidade,
limites de buffers, concorrência, cancelamento, lifecycle de recursos,
telemetria e testes. Uma melhoria interna não pode alterar opcode, packing,
asset, timing ou efeito observável sem um contrato coordenado e uma validação
específica.

## Escopo desta unidade

- separar protocolo, assets, renderer e plataforma;
- usar o TMProject apenas como mapa estrutural, nunca como fonte de código;
- manter o servidor autoritativo;
- não alterar opcodes, layouts ou assets nesta etapa;
- executar uma janela Win32 e um contexto OpenGL/WGL sem CGO;
- manter criação, frames e teardown na mesma thread do sistema;
- registrar a futura cadeia de conversão e proteção de assets.

Os assets ativos da preparação estão em `assets/current/`; o client oficial
intacto permanece em `CLIENT OFICIAL 7.48/` como evidência somente leitura.

## Fontes e classificação

Esta unidade é uma `MODERNIZACAO_COMPATIVEL` interna: ainda não altera a
fronteira observável do client. O código atual do WYD-Go e a organização do
TMProject foram utilizados como referência estrutural. A descompilação e o
binário nativo 7.48 permanecem obrigatórios antes de implementar qualquer
fluxo observável, wire, UI, input, renderização ou loader legado.

W2PP, Secrets e Micronics não são fontes desta unidade.

## Próximas unidades

O plano de cobertura e as lacunas reais estão em [`MAPPING.md`](MAPPING.md).
As próximas unidades devem seguir a fila em
`references/catalog/research-queue.tsv`, fechando uma transição nativa por vez.

1. validar no executável o `SceneManager`, input/eventos e recursos por cena;
2. fechar o lifecycle de criação, troca, falha parcial e teardown;
3. só depois rastrear e implementar transporte, login, seleção de personagem
   e entrada no mundo.

Cada unidade deve compilar e ser validada antes da seguinte. Os arquivos
originais continuam sendo a fonte e não serão sobrescritos pelo conversor.

## Validação local

O ciclo de desenvolvimento possui três níveis. Execute o menor nível que cobre
a alteração:

```powershell
pwsh -NoProfile -File .\Verify-Fast.ps1
pwsh -NoProfile -File .\Verify-Contract.ps1
pwsh -NoProfile -File .\Verify-Mapping.ps1
```

- `Verify-Fast.ps1`: testes Go, `go vet`, build temporário e `git diff --check`
  quando houver worktree. É o ciclo normal de código ativo.
- `Verify-Contract.ps1`: inclui o nível rápido, manifesto integral, pacote
  autocontido e census determinístico de assets. Use para protocolo, loaders,
  assets e empacotamento.
- `Verify-Mapping.ps1`: inclui o nível de contrato e audita fingerprint nativo,
  catálogo de 4.146 funções, corpus, fichas e ferramentas de pesquisa. Use ao
  alterar evidência, catálogo, fichas ou o próprio pipeline de auditoria.

O smoke test do executável permanece separado porque depende de Windows e de
uma janela real:

```powershell
pwsh -NoProfile -File .\Build-ClientGo.ps1 -Configuration Debug
pwsh -NoProfile -File .\tools\Test-ClientBootstrap.ps1
pwsh -NoProfile -File .\tools\Test-ClientBootstrap.ps1 -EnvironmentFile .\bin\protected-test.env
```

O nível rápido não substitui contrato, Ghidra nem teste manual. O CI executa o
nível rápido em toda mudança, o nível de contrato quando protocolo/assets são
afetados e a auditoria completa quando a base de pesquisa muda, além de uma
execução semanal e acionamento manual.
