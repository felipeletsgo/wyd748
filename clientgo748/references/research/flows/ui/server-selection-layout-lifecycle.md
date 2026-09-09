---
id: ui-server-selection-layout-lifecycle
title: Posicionamento e lifecycle da selecao de servidor 7.48
subsystem: ui-layout
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-02
---

# Posicionamento e lifecycle da selecao de servidor 7.48

## Pergunta

Por que o painel Server/Channel da tela inicial ficava abaixo da posicao do
client nativo e qual composicao deve ser preservada durante entrada, retorno do
login e relogin?

## Fronteira de evidência

- Executavel: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 acima.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr` e
  `%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled`.
- Inicializador nativo: `FUN_004A8F14`, cena do estado `7`.
- Vtable: `0x005A4544`; o slot `+0x4C`, armazenado em `0x005A4590`, aponta
  para `FUN_004A8F14`.
- Source ativa: `TMSelectServerScene::InitializeScene` e
  `TMSelectServerScene::InitializeUI`.
- Recurso: `UI\SelServerScene2.txt`, materializado por
  `client748/UI/SelServerScene2.bin`.
- Captura fornecida em 2026-09-02: o conjunto Server/Channel aparecia
  deslocado para baixo em relacao ao client original.

## Fluxo nativo 7.48

### Entrada observável

Ao entrar no estado de selecao de servidor, `FUN_004B3500` instancia a cena
`7` e chama indiretamente seu initializer pelo slot virtual `+0x4C`.
`FUN_004A8F14` seleciona `SelServerScene2` quando `g_UIVer == 2`, resolve a
arvore de controles e compoe o painel Server/Channel antes de exibi-lo.

O root nativo e o controle `4622` (`0x120E`). A traducao de recursos da source
o entrega como `P_SERVER_SEL` (`65537`); listas `4618/4619` tornam-se
`65542/65543`, e os titulos `5635/5636` tornam-se `65540/65541`.

### Callers

`CONFIRMED`: a vtable `0x005A4544` da cena `7` contem `FUN_004A8F14` no slot
`+0x4C`. O caller generico esta em `FUN_004B3500:0x004B370F`, por
`CALL [EDX+0x4C]`; retorno zero ativa destruicao parcial, diagnostico e
`WM_CLOSE`.

### Função principal

`FUN_004A8F14` busca o root `0x120E` e chama seu slot de posicionamento com:

```text
x = viewportWidth*0.5 - rootWidth*0.5
y = viewportHeight*0.5 - rootHeight*0.5
```

Nao existe uma segunda soma vertical de 75 pixels nesse receptor. O mesmo
initializer define `g_nChannelWidth = 140`, cria os tres trechos do cabecalho,
ajusta o titulo Server em `y += 5` e o titulo Channel em `x += 8`, `y += 5`.
Os tratamentos de logo para larguras 1024, 1280 e 1600 pertencem a receptores
separados e nao alteram a formula do root.

`SControl` ja materializa posicao e tamanho conforme as razoes do viewport.
Assim, `m_nWidth` e `m_nHeight` usados pela source sao dimensoes renderizadas;
aplicar outra escala ou offset apos `SetPos` duplicaria a transformacao.

### Callees

- carregamento do RC pelo slot virtual `+0x3C`;
- busca de controles pelo slot `+0x48`;
- posicionamento pelo slot `+0x70`;
- visibilidade pelo slot `+0x60`;
- anexacao dos paines dinamicos a arvore por `FUN_0054AC09`.

Os tres cabecalhos e os backgrounds de linhas/canais sao filhos do root. Uma
unica mudanca na posicao do root desloca desenho, textos, listas e hitboxes em
conjunto; nao ha justificativa para corrigir filhos individualmente.

### Saídas e erros

Falha de `FUN_00541065` faz o initializer registrar erro, mostrar
`MessageBoxA`, postar `WM_CLOSE` e retornar zero. O caller generico destrói a
cena parcial e repete o diagnostico. O caminho normal retorna um e entrega a
cena composta ao ObjectManager.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondicao | Funcao/call | Estado resultante | Side effects | Erro/saida |
| --- | --- | --- | --- | --- | --- |
| entrada na tela inicial | estado `7` solicitado | `FUN_004B3500 -> +0x4C -> FUN_004A8F14` | root centralizado e visivel | carrega Scene2 e compoe filhos | falha do RC fecha a janela |
| selecionar canal | grupo/canal validos | event handler da cena | root oculto; login visivel | foco vai ao ID | endpoint ainda e validado pelo servidor |
| voltar do login | cancelar/retornar | event handler da cena | root reaparece na mesma posicao | login perde foco | nao recria controles |
| Game Grade | demonstracao ativa | `FrameMoveGameGrade` | alterna root e logo | preserva coordenadas | sem novo ownership |
| logout/relogin | ObjectManager troca cenas | recriacao da cena `7` | nova arvore reaplica a formula | ponteiros antigos nao sobrevivem | segue falha padrao de init |
| teardown/shutdown | cena marcada para delecao | `FUN_004AE5C0 -> FUN_004A8E29 -> FUN_00494C00` | filhos e container liberados | detach por `FUN_0054AA45` | sem alocacao propria do layout |

### Vtables, vptrs e receptores

O vptr da cena `7` aponta para `0x005A4544`. O ObjectManager anexa a cena e sua
arvore por `FUN_0054AC09`. O slot `+0x4C` resolve o initializer; os receptores
de layout sao o root e os controles filhos obtidos do container.

### Ownership

O container e owner dos controles carregados e dos filhos adicionados ao root.
O helper de posicionamento nao aloca nem retém ponteiros fora da cena.

### Falha parcial

Se `FUN_00541065` falhar, `FUN_004A8F14` diagnostica, solicita `WM_CLOSE` e
retorna zero. `FUN_004B3500` destroi a cena parcialmente inicializada pelo
mesmo caminho de ownership; remover o offset nao acrescenta recursos parciais.

### Cleanup e teardown

`FUN_004B16C0` consome a marca de delecao e converge no deleting destructor
`FUN_004AE5C0`, cleanup especifico `FUN_004A8E29`, base `FUN_00494C00` e
detach `FUN_0054AA45`.

### Shutdown

O shutdown global usa a mesma coleta e teardown da cena. A geometria nao deixa
timer, captura de input, alocacao ou callback proprio a ser cancelado.

### Logout e relogin

Posicionamento nao cria estado persistente. Retorno do login usa a mesma
instancia; logout, relogin e shutdown descartam a arvore inteira e a proxima
cena executa novamente o initializer.

## Wire, ABI e recursos

Nao ha mudanca de wire, ABI, opcode, asset ou servidor. A adaptacao conserva o
recurso Scene2, os IDs traduzidos, callbacks, visibilidade, foco e ownership.

## Mapeamento atual

Classificacao: `PARIDADE_NATIVA`.

`TMSelectServerScene::InitializeUI` ja possuia a formula nativa de
centralizacao, mas somava `75.0f` a `m_nPosY` em seguida. A adaptacao remove
somente essa divergencia. Os filhos continuam ancorados ao root e os ajustes
nativos de titulo, lista, largura de canal e logo permanecem inalterados.

WYD-Go nao participa deste layout local e nao requer funcao correspondente.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Source adaptada | Decisao |
| --- | --- | --- | --- | --- |
| root Server/Channel | centro exato em X/Y | centro em X/Y seguido de `y += 75` | centro exato em X/Y | remover apenas o offset extra |
| filhos e hitboxes | filhos do root | filhos do root | preservados | nao deslocar individualmente |
| titulos e largura | `+5`, `+8/+5`, largura 140 | ja equivalentes | preservados | sem alteracao |
| logos por resolucao | branches 1024/1280/1600 | ja equivalentes | preservados | sem alteracao |
| servidor/wire | nao participa da geometria | inalterado | inalterado | nenhuma mudanca |

## Decisões

- Remover somente o deslocamento vertical extra de 75 pixels.
- Manter Scene2, traducoes de ID e composicao dinamica existente.
- Nao alterar logos, listas, callbacks, disponibilidade de canal ou servidor.
- Nao aplicar escala adicional a dimensoes ja materializadas.

## Lacunas

A correlacao estatica do layout e do lifecycle esta fechada. Falta validar no
client recompilado a aparencia e os hitboxes em resolucoes de referencia,
incluindo retorno do login e logout/relogin.

## Validação

- Pesquisa: `TRACED`; initializer, caller virtual, vtable, receptores,
  geometria, falha parcial, ownership e teardown correlacionados.
- Implementacao: `IMPLEMENTED`; removido apenas o offset vertical ausente no
  nativo.
- Automacao: `validate_research.py` e `git diff --check` passaram; o build
  oficial `Release|Win32` v145 terminou com zero erros e zero warnings e
  instalou `client748/project.exe` com SHA-256
  `6E6AF9A88CC81729E866277F2B81618E9FA9A0B07C26C54D9C552CE719A380BC`.
- Client real: ainda nao e `CLIENT_TESTED`.

Teste em jogo obrigatorio:

1. abrir a tela inicial em `800x600`, `1024x768` e `1280x960`;
2. confirmar que Server/Channel fica centralizado e que texto, overlay e
   hitboxes acompanham o frame;
3. selecionar canal, voltar do login e confirmar que a posicao nao muda;
4. entrar no mundo, executar logout/relogin e confirmar a nova instancia.
