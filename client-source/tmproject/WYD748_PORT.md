# Port do client recompilavel para o WYD-Go

## Estado

O codigo importado tem como alvo original o client Global/KR 7.69+. Ele compila
como aplicacao Win32/DirectX 9, mas ainda nao e compativel com o protocolo vivo
do WYD-Go 7.48.

## Perfil de assets 7.48

O build recompilavel pode ler diretamente a arvore `client748/` por meio de
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
possuem apenas checksum adicional ignorado pelo loader. Execute
`Test-Client748Assets.ps1` antes do client para impedir leitura silenciosa de
um formato desconhecido.

Os shaders nao existiam no client 7.48 original porque aquele executavel
embutia outro caminho de renderizacao. Eles fazem parte da dependencia de
runtime do client recompilavel e seus hashes sao validados antes do boot para
manter o bytecode alinhado as declaracoes de vertices do TMProject.

O inicializador de render targets do TMProject tambem foi corrigido para usar
`D3DPOOL_DEFAULT`, combinacao exigida pelo DirectX 9 quando a textura possui
`D3DUSAGE_RENDERTARGET`. O codigo importado usava `D3DPOOL_MANAGED` e ainda
alocava a textura de origem duas vezes, causando falha de inicializacao e
vazamento antes da primeira cena.

## Contratos

1. `model.ExtendedScore` continua sendo a unica autoridade de atributos.
2. `STRUCT_SCORE` de 48 bytes e uma projecao do servidor para o client.
3. Campos de score recebidos do client nunca alimentam gameplay.
4. A extensao `XSC2` permanece enquanto houver atributos que nao cabem no
   `STRUCT_SCORE`: ataque magico, accuracy, evasion, parry, resistencias,
   pontos, regeneracao e outros campos wide.
5. Cada packet sera migrado com tamanho e offsets testados nos dois lados.
6. O executavel atual em `client748/` permanece como referencia do protocolo
  7.48 e como fallback ate a paridade in-game.

### Regra de comparacao entre versoes

O TMProject original e uma referencia mais nova (7.59/7.69, conforme o
subprojeto) e pode conter correcoes de comportamento, dados mais completos e
estruturas internas superiores. Isso nao autoriza copiar offsets, tamanhos de
struct ou opcodes diretamente para o 7.48. Para cada diferenca, a portabilidade
segue esta ordem:

1. confirmar a semantica no codigo mais novo;
2. localizar o campo equivalente no ABI que o client 7.48 realmente consome;
3. adaptar a representacao, mantendo os tamanhos e offsets do 7.48;
4. proteger a conversao com teste byte-a-byte e, quando possivel, corpus de
   pacote real.

Quando uma tabela mais nova for mais completa, ela pode ser usada como fonte
de dados somente se houver uma traducao segura para o formato 7.48. Por
exemplo, um `SkillData.bin` moderno pode fornecer definicoes adicionais, mas o
loader deve projetar cada registro para o layout 7.48 que o executavel conhece;
nao se deve enviar o registro moderno ou aumentar uma struct nativa embutida no
client. Se a traducao nao for comprovadamente consumivel pelo 7.48, a entrada
fica no formato legado e a feature nao e habilitada por suposicao.

Em resumo: a versao mais nova fornece o comportamento e os dados candidatos; o
ABI 7.48 continua sendo a fronteira de compatibilidade. Cada adapter deve
explicar no codigo qual lado e a fonte e qual lado e a representacao final.

## Layout inicial do score

| Offset | Tamanho | Campo |
| ---: | ---: | --- |
| 0 | 2 | Level |
| 2 | 2 | padding |
| 4 | 4 | Defense/Ac |
| 8 | 4 | Attack/Damage |
| 12 | 1 | Merchant/Reserved |
| 13 | 1 | AttackRun |
| 14 | 2 | padding |
| 16 | 4 | MaxHP |
| 20 | 4 | MaxMP |
| 24 | 4 | CurHP |
| 28 | 4 | CurMP |
| 32 | 2 | STR |
| 34 | 2 | INT |
| 36 | 2 | DEX |
| 38 | 2 | CON |
| 40 | 8 | Mastery[4] |

O tamanho e os offsets sao protegidos por `static_assert` no client e por
testes byte-a-byte em `internal/wire`.

## Ordem de migracao

1. Compilar e executar o baseline sem alterar o client 7.48 distribuido.
2. Adicionar identificacao explicita da variante de protocolo na autenticacao.
3. Migrar char-list e entrada no mundo.
4. Migrar `CreateMob`, `UpdateScore` e HP/MP.
5. Migrar equipamento, inventario, movimento, ataque e skills.
6. Portar a extensao wide, macro, trajes, montarias e faces ja validados.
7. Validar owner e observer antes de promover o novo executavel.

Nao alterar varios packets estruturais de uma vez: `STRUCT_SCORE` esta embutido
em estruturas maiores, portanto cada mudanca deve possuir corpus e teste do
packet final.
