# Port do client recompilavel para o WYD-Go

## Estado

O codigo importado tinha como alvo original o client Global/KR 7.69+. Nesta
árvore ele é adaptado como uma implementação de versão única, Win32/DirectX 9,
para o protocolo, ABI, UI e assets comprovados do WYD-Go 7.48. Não conservar
um caminho executável para a versão upstream.

O único candidato ativo é `tmproject/client748/project.exe`, instalado diretamente a
partir do build. Os executáveis e patchers históricos não são fallback, produto
ou gate de validação.

## Perfil de assets 7.48

O build recompilavel pode ler diretamente a arvore `tmproject/client748/` por meio de
`WYD_ASSET_ROOT`. Os loaders reconhecem e traduzem explicitamente:

- tabelas de textura 7.48 de 264 bytes por registro para a representacao de
  528 bytes usada em memoria pelo TMProject;
- `UITextureList.bin` como nome legado de `UITextureListN.bin`;
- `SkillData.bin` com 104 registros de 96 bytes;
- `strdef.bin` com 440 strings de 128 bytes, mantendo vazias as linhas modernas
  inexistentes.
- os 18 shaders DirectX 9 precompilados exigidos pelo renderer (`skinmesh`,
  `vseffect` e `pseffect`), importados do runtime oficial `WYDESTINY` do mesmo
  commit do TMProject.
- a tela de servidor exclusiva do 7.48 em `SelServerScene2.bin`, incluindo a
  composicao runtime dos seis paineis `NewUI_ServerList_*` e
  `NewUI_ChannelList_*` que nao estao materializados no arquivo RC;
- o parser opcional dos registros RC antigos com captions inline, mantido para
  recursos legados sem substituir a tela principal 7.48;
- `sn.bin` como tabela binaria fixa de 11 nomes e 11 ordens de grupo, em vez do
  formato texto esperado pelo TMProject recente.
- `config.txt` como configuracao nomeada do 7.48, preservando resolucao, modo
  de janela, variante da UI, camera, cursor, audio e animacao sem overrides.

`ItemList.bin`, `ValidIndex.bin`, `object.bin`, `serverlist.bin` e
`AttributeMap.dat` ja correspondem aos tamanhos consumidos pelo codigo ou
possuem apenas checksum adicional ignorado pelo loader. As verificações de
assets devem seguir os loaders, a skill `wyd-client748-assets` e o auditor
`tools/client-assets/Audit-ClientAssets.ps1`, que separa dívida declarada de
faltas ainda não classificadas e divergências de casing. O build atual
está descrito em [Build e integração](../build-and-integration.md).

Uma referência literal ausente na source não prova, isoladamente, um asset
7.48 faltante. Em especial, `TMSkinMesh::SetCostume` ainda contém caminhos da
source posterior e caminhos que se sobrepõem à coleção KR catalogada. O bucket
`SourceMissingUnclassified` é diagnóstico para priorizar investigação; não se
deve fabricar assets, remover `case`s ou promover esse bucket a gate de paridade
sem confirmar o fluxo ativo e a procedência do recurso.

Os shaders nao existiam no client 7.48 original porque aquele executavel
embutia outro caminho de renderizacao. Eles fazem parte da dependencia de
runtime do client recompilavel. Sua compatibilidade com as declaracoes de
vertices deve ser validada junto ao renderer; esta reorganizacao nao executou
validacao visual nem um gate automatico de hashes dos shaders.

O inicializador de render targets do TMProject tambem foi corrigido para usar
`D3DPOOL_DEFAULT`, combinacao exigida pelo DirectX 9 quando a textura possui
`D3DUSAGE_RENDERTARGET`. O codigo importado usava `D3DPOOL_MANAGED` e ainda
alocava a textura de origem duas vezes, causando falha de inicializacao e
vazamento antes da primeira cena.

## Trajes catalogados: seleção e carregamento

O fluxo `Equip[13] -> TMHuman::InitObject -> TMSkinMesh::RestoreDeviceObjects`
agora usa o ID completo do traje e a tabela de `Costumes-KR.json`. Dos 135
itens disponíveis, 130 selecionam os 129 renderers catalogados; os cinco
trajes-base delegam ao renderer existente. Os trajes importados não passam
pelo switch posterior de `SetCostume`. Itens fora da coleção mantêm o caminho
existente, sem remoção presumida por ausência no catálogo.

A seleção preserva o skeleton atual, inclusive suas variantes, e aplica a
escolha por paridade do skeleton para King e TopRanker. Uma marca interna
isola os renderers importados dos tipos usados por NPCs; ela não vai para o
protocolo. As seis partes são resolvidas por índice, sem cursor compartilhado:
parte zero vazia preserva a face nativa, demais partes vazias são omitidas e
partes fora da tabela, como armas, preservam o caminho original. Não há novo
parser JSON em runtime nem alteração de score, bônus ou autoridade do servidor.

Classificação: `MODERNIZACAO_COMPATIVEL` na source, reutilizando dados já
validados como `PARIDADE_NATIVA`. Procedência desta frente:

- Binário nativo/Ghidra e descompilação estudada: `UTILIZADA` por meio da
  evidência registrada em [Trajes KR](native-reference-patches.md), incluindo
  seleção, marca interna e preservação de skeleton/face; sem nova análise ou
  execução de patch histórico.
- Assets 7.48: `UTILIZADA`; manifesto, 774 partes e dependências conferidos.
- TMProject atual: `UTILIZADA` para integrar seleção e carregamento; o
  mapeamento hardcoded posterior é `CONTRADITÓRIA` para os trajes importados
  cujo tipo/caminho diverge do manifesto, não para todo o renderer.
- WYD-Go e testes: `UTILIZADA`; projeção atual de equipamento e consumo do
  slot 13 inspecionados, testes focados adicionados no client. Sem alteração
  de wire; não foi executada uma nova suíte do servidor neste lote.
- Guias adicionais: `NÃO APLICÁVEL`; nenhum contrato novo foi introduzido.

`tools/client-assets/Export-CostumeTable.ps1` emite a tabela C++ para revisão;
`-Check` compara o header compilado com o manifesto e verifica os arquivos
referenciados. Esse gate roda uma vez por invocação de `Build-Client.ps1`,
antes dos testes e de qualquer instalação do candidato.

Validação do lote: Release, tabela e dependências conferidas, 35.239
verificações automatizadas aprovadas. O candidato foi instalado com SHA-256
`9021B1FAAB444CBE85EE7141368C14890969A51499829848DE360752BC5B5E3F`.
Em 2026-09-12, o usuário confirmou que os trajes estão funcionais no jogo:
`CLIENT_TESTED` para o fluxo exercido, além de `BUILD_VERIFIED` e
`AUTOMATED_TESTED`. A confirmação não discrimina todos os itens/corpos,
observador remoto, logout/relogin ou transformações; não implica cobertura
exaustiva dessa matriz.

## Contratos

1. `model.Score` continua sendo a unica autoridade de atributos.
2. `STRUCT_SCORE` tem 140 bytes: 35 campos uint32 no contrato coordenado atual.
3. Campos de score recebidos do client nunca alimentam gameplay.
4. Os atributos adicionais integram o score diretamente; não há sidecar ativo.
5. Cada packet sera migrado com tamanho e offsets testados nos dois lados.
6. `tmproject/client748/project.exe` é o único candidato executável; o binário histórico
   7.48 serve apenas como referência Ghidra read-only.

### Regra de comparacao entre versoes

O TMProject 7.69 pode fornecer arquitetura e algoritmos candidatos. Para
fronteiras legadas, a evidência primária continua sendo o nativo 7.48/Ghidra.
As classificações e os gates estão no [AGENTS.md](../../AGENTS.md).

Uma modernização compatível preserva o contrato comprovado. Uma extensão
coordenada pode alterar o contrato quando ambas as pontas forem adaptadas e
testadas explicitamente. Nenhuma das duas deve ser apresentada como paridade
nativa. Assets posteriores exigem validação de formatos, recursos, loaders e
fluxo observável; sua ausência no nativo não justifica remoção automática.

## Layout ativo do score

O [contrato canônico](../SCORE.md) substitui o layout histórico de 48 bytes.
O tamanho atual é protegido por `static_assert` em
`tmproject/TMProject748/internal/core/WYD748Compat.cpp` e pelo encoder e testes
em `wydgo748/internal/wire/score.go` e `score_test.go`. Mudanças precisam cobrir
também todos os packets que embutem o score.

## Ordem de adaptacao

1. Confirmar o fluxo nativo e o SHA da referência histórica no Ghidra 7.48.
2. Localizar callers, callees, structs e assets correspondentes na source viva.
3. Adaptar um grupo pequeno de packets ou uma janela por vez, removendo o
   caminho incompatível somente quando houver evidência e contrato substituto.
4. Proteger wire/ABI com `static_assert` e testes byte-a-byte.
5. Validar assets, compilar e confirmar que o build instalou e conferiu
   automaticamente `tmproject/client748/project.exe`.
6. Testar owner, observer, falha e relogin antes de promover o comportamento.

Nao alterar varios packets estruturais de uma vez: `STRUCT_SCORE` esta embutido
em estruturas maiores, portanto cada mudanca deve possuir corpus e teste do
packet final. Nunca criar seleção de variante de protocolo na autenticação ou em
outro ponto; esta source tem apenas o contrato 7.48.
