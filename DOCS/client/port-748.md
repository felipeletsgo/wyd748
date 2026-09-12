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
assets devem seguir os loaders e a skill `wyd-client748-assets`; o antigo
validador separado de assets não está disponível nesta árvore. O build atual
está descrito em [Build e integração](../build-and-integration.md).

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
