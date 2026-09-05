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

```text
app/                 bootstrap e composicao da aplicacao
platform/            Windows, DirectX, entrada, som, rede e adaptadores
core/                tipos e utilitarios sem dependencia de cenas
protocol/wire/       layouts binarios e invariantes de ABI
protocol/dispatch/   roteamento de mensagens
data/                loaders e modelos de dados
render/              dispositivo, malhas, texturas e efeitos
world/               terreno, clima e objetos do mundo
entities/            humanos, itens, drops e entidades especializadas
ui/                  controles, grids, HUD e janelas
scenes/              ciclo de vida e comportamento das cenas
features/            fluxos verticais como trade, craft, party e guild
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
