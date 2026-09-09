# TMProject748

Este e o projeto ativo para a reestruturacao do client WYD 7.48. A migracao e
classificada como `MODERNIZACAO_COMPATIVEL`: a organizacao interna pode evoluir,
mas os contratos externos comprovados devem permanecer equivalentes.

## Referencia preservada

`../TMProject` contem a source anterior e deve permanecer intacta e disponivel
para consulta. Ela nao deve receber a reestruturacao nem ser removida quando um
arquivo for migrado para este projeto.

O script `../../Build-Client.ps1` compila `TMProject748` por padrao. Para uma
comparacao controlada, a referencia pode ser compilada com:

```powershell
.\Build-Client.ps1 -ProjectVariant TMProject
```

A compilacao da referencia possui diretorios de saida isolados e nao instala o
executavel sobre `client748/project.exe`.

## Limites desta migracao

- preservar packets, opcodes, tamanhos, packing, signedness e asserts de ABI;
- preservar IDs e formatos de recursos carregados pelo client;
- preservar ownership, nulabilidade, ordem de inicializacao e teardown;
- nao interpretar codigo do TMProject 7.69+ como prova do comportamento 7.48;
- registrar extensoes coordenadas separadamente da paridade nativa;
- validar cada lote antes de remover a implementacao equivalente deste clone;
- manter comentarios didaticos em portugues sobre responsabilidade, entradas,
  saidas, ownership e lifecycle das partes reorganizadas.

## Arquitetura de destino

A arvore acompanha o desenho do emulador Go: executaveis em `cmd/`, codigo nao
publico em `internal/` e separacao por `game`, `model`, `net`, `proto` e `wire`.
Dominios exclusivos do client permanecem dentro de `internal/`, sem misturar
renderizacao ou input aos contratos compartilhados.

```text
cmd/client/          ponto de entrada do executavel, como cmd/server no Go
internal/app/        composicao e lifecycle da aplicacao
internal/data/       loaders e dados materializados
internal/game/       regras, cenas, entidades e features observaveis
internal/model/      tipos de dominio sem comportamento de transporte
internal/net/        sessao e transporte
internal/proto/      dispatcher e semantica das mensagens
internal/wire/       layouts, codecs e invariantes de ABI
internal/platform/   Windows, DirectX, input, som e diagnosticos
internal/render/     dispositivo, malhas, texturas e efeitos
internal/ui/         controles, grids, HUD e janelas
docs/                decisoes e acompanhamento da migracao
legacy/              adaptadores temporarios explicitamente documentados
```

Esta primeira etapa e propositalmente mecanica: os mesmos fontes e recursos
foram copiados sem alterar comportamento. A movimentacao fisica e a divisao dos
arquivos grandes comecam somente depois de este clone compilar em Release.

## Progresso da reorganizacao

- `scenes/demo/TMDemoScene.*`: primeiro modulo movido apos o clone compilar.
  Continua sendo um stub documentado, sem recursos ou comportamento inventados
  para o 7.48.
- `features/macro/MacroFactory.*` e `MacroMsg.*`: entrada e ownership da macro
  de agua agrupados e documentados. `MacroLevel.*` permanece temporariamente na
  raiz porque seu envio de packets exige um lote de contrato separado.
- `TMFieldScene`, `TMHuman`, `TMScene` e `SGridControl`: permanecem como fachadas
  monoliticas ate seus lotes poderem ser separados e validados individualmente.
