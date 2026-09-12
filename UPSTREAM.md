# Upstream provenance

Esta arvore foi importada de:

- repositorio: <https://github.com/lorransouza/TMProject-Free>
- branch: `main`
- commit: `14937eda00d462c06dfd3a1ac5905f910d70a804`
- data da importacao: 2026-08-20
- escopo importado: `Source cliente/`

Nao foram importados o historico Git, releases, client pronto ou a source de
servidor do repositorio externo.

O upstream declara o codigo sob GNU GPL v3 no seu README. O WYD-Go tambem e
distribuido sob GNU GPL v3; o arquivo `LICENSE` na raiz do WYD-Go permanece
aplicavel a esta copia modificada. Os creditos originais permanecem no
`README.md` deste diretorio.

Esta copia e o único produto source-built do client WYD 7.48 no WYD-Go.
Atualizacoes futuras do upstream devem ser revisadas campo a campo e somente a
semantica comprovada no 7.48 pode ser portada por commit. Nunca importar em
massa ABI, packets, controles, loaders ou branches 7.59/7.69, nem manter um
segundo alvo de versão. Este diretorio nao e um submodulo e nao deve receber um
`.git` proprio.

Executaveis e patchers historicos nao participam da sincronizacao com upstream,
do build ou da validacao. Toda mudanca ativa termina nesta source ou nos assets
consumidos diretamente por `client748/project.exe`.
