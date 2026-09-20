---
id: application-bootstrap-message-loop
title: Application bootstrap and message loop
subsystem: lifecycle
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-20
---

# Application bootstrap and message loop

## Pergunta

Como o executável nativo WYD 7.48 sai do entrypoint CRT, constrói o singleton
da aplicação, impede uma segunda instância, inicializa a janela e seus owners e
entra no message loop que despacha `WndProc`?

## Fronteira de evidência

- Executável/hash: `references/client748/WYD.exe`, SHA-256 do frontmatter.
- Corpus Ghidra imutável: `005927ac_entry.c`,
  `0055f7f9_FUN_0055f7f9.c`, `0055b18f_FUN_0055b18f.c`,
  `0055bc0a_FUN_0055bc0a.c`, `0055ca18_FUN_0055ca18.c`,
  `0055d345_FUN_0055d345.c` e `0055fa89_FUN_0055fa89.c` em
  `C:/Users/felipe/Tools/GhidraAnalysis/20260821/decompiled/`.
- Source recompilável: `internal/app/scenes/NewApp.cpp`, `NewApp.h`,
  `internal/platform/windows/TMPaths.h`, `DirShow.cpp` e
  `internal/platform/windows/media/TMVideoWnd.cpp`, além do adaptador
  `internal/render/effects/core/JBlur.cpp`.
- IDs estáveis do inventário de source: `acbeb4d7c73897744a3d` (`wWinMain`),
  `a4554b50ec02ebf74ba5` (`NewApp::Initialize`),
  `86d5b7f9d1ed79540b47` (`NewApp::InitDevice`),
  `ce9a04f5c259609b08a0` (`NewApp::Run`) e
  `00b0bdb56f6afaa96e47` (`WndProc`).

Esta ficha fecha o bootstrap até o message loop. O fechamento disparado por
`WM_CLOSE` e o teardown de owners permanecem em
`application-close-global-shutdown.md`. Login por argumento do launcher e
reprodução de AVI não são promovidos por esta investigação.

## Fluxo nativo 7.48

### Entrada observável

`entry` em `0x005927AC` completa o startup CRT, obtém a command line ANSI e
chama `FUN_0055F7F9`. Esse wrapper inicia a camada de diagnóstico/processo,
aloca `0x114` bytes e chama `FUN_0055B18F`.

### Callers

O startup CRT é o caller direto de `FUN_0055F7F9`. Dentro do wrapper, o objeto
publicado por `FUN_0055B18F` é o receptor dos slots virtuais `+0x04` e `+0x0C`.
No extremo Win32, `DispatchMessageA` chama `FUN_0055FA89`, que encaminha a
mensagem ao mesmo singleton em `DAT_013B71E0`. Não há outro receptor de
aplicação nessa cadeia.

### Função principal

O construtor `FUN_0055B18F` instala a vtable `0x005A6104`, publica o singleton
em `DAT_013B71E0`, posiciona o current directory no diretório do executável,
inicializa a tabela de hit rate, carrega o arquivo de mensagens, abre o log e
define o título da janela.

Depois do parsing opcional do argumento de launcher, `FUN_0055F7F9` chama
`FindWindowA(NULL, titulo_da_janela)`. Se uma janela com esse título já existe,
o wrapper retorna sem inicializar uma segunda instância. O segundo argumento é
o título, não o nome da classe Win32.

### Bootstrap, hook e loop

Sem instância anterior, o wrapper consulta o keyboard layout, chama
`CoInitialize(NULL)` e `OleInitialize(NULL)` e encerra o processo se qualquer
resultado for negativo. Em seguida resolve o modo de janela pela command line
e chama o slot virtual `+0x04`, `FUN_0055BC0A`.

`FUN_0055BC0A` lê a configuração, registra `FUN_0055FA89` como WndProc, cria a
janela principal e as janelas auxiliares, carrega os dados globais e inicializa
vídeo/dispositivo conforme a presença do AVI. O retorno zero impede hook e
message loop.

O parser nativo consome as mesmas quinze chaves nomeadas de `config.txt`. Sua
tabela oferece modos de 16 e 32 bits, enquanto a source preserva onze modos de
32 bits e resoluções widescreen já adotadas pelo runtime atual. O valor
`CLASSIC` é lido no nativo, mas depois sobrescrito por UI version `2`; a source
mantém o valor configurado para não reintroduzir o desalinhamento conhecido da
UI 7.48. Essas diferenças são adaptações explícitas, não prova de erro.

Depois dos dados base e de item, o nativo lê `itemhelp.dat` em um buffer cujo
tamanho e stride comportam 6.500 registros. O limite de 6.500 usado pela
source, embora a condição decompilada aparente 65.000, coincide com a alocação
real e evita ultrapassar o array recompilável. As janelas filhas `static` e
`Button` criadas pelo nativo não possuem consumidor demonstrado na source e
permanecem fora da adaptação.

Sem AVI ativo, `FUN_0055BC0A` chama `FUN_0055CA18`. Essa função cria e
inicializa, na mesma sequência essencial da source, RenderDevice, áudio/BGM,
EventTranslator/IME, TimerManager, ObjectManager e CPSock; publica o socket,
zera o tempo de servidor, ativa a aplicação e ajusta mipmap pela memória de
textura disponível. `NewApp::InitDevice` acrescenta somente o modo fullscreen
adaptado e um `JBlur` inerte. O owner desse adaptador agora é inicializado no
construtor e liberado antes do RenderDevice no shutdown.

No sucesso, `FUN_0055F7F9` instala o hook de teclado por `FUN_00423C1F` e chama
o slot `+0x0C`, `FUN_0055D345`. Esse loop alterna `PeekMessageA` no estado ativo
e espera bloqueante no estado inativo, chama `TranslateMessage` e
`DispatchMessageA`, e executa tick/frame quando não há mensagem terminal.

`FUN_0055FA89` é o thunk do WndProc: preserva os quatro argumentos Win32,
carrega `DAT_013B71E0` e encaminha para `FUN_0055DAB8`. Assim, o caminho
observável fica:

```text
CRT entry -> FUN_0055F7F9 -> FUN_0055B18F
-> app vslot +0x04/FUN_0055BC0A -> FUN_00423C1F
-> app vslot +0x0C/FUN_0055D345 -> DispatchMessageA
-> FUN_0055FA89 -> DAT_013B71E0/FUN_0055DAB8
```

Depois que o loop retorna pelo shutdown documentado separadamente, o wrapper
executa `OleUninitialize`, `CoUninitialize`, o deleting destructor no slot
`+0x00/FUN_0055F3E0` e o teardown da camada de processo `FUN_004B428E`.

No loop normal, `FUN_0055D345` trata retorno zero como frame ocioso e retorno
não zero como mensagem. A source usa a mesma alternância, mas agora zera a
estrutura `MSG` antes do primeiro `PeekMessage` e encerra com log quando
`GetMessage` retorna `-1`. Isso não muda mensagens válidas nem frames ativos e
evita reutilizar estado indefinido em uma falha da API. O ramo nativo que exibe
a janela auxiliar de navegador continua ausente porque essa janela não possui
consumer ativo na source atual.

### Callees

- `FUN_0055B18F`: constrói e publica o singleton da aplicação.
- `FindWindowA`: aplica a unicidade pelo título da janela.
- `CoInitialize` e `OleInitialize`: preparam o apartment do thread principal.
- `FUN_0055BC0A`: cria janela, recursos e owners globais.
- `FUN_00423C1F`: instala o hook de teclado depois do bootstrap.
- `FUN_0055D345`: mantém o message loop e os frames.
- `FUN_0055FA89` e `FUN_0055DAB8`: encaminham e despacham mensagens Win32.
- `FUN_0055F3E0` e `FUN_004B428E`: encerram objeto e camada de processo após o
  loop; o teardown de managers já ocorreu no WndProc.

## Estado e lifecycle

### Matriz de transições

| Estado/evento | Precondição | Ação | Estado resultante | Falha/saída |
| --- | --- | --- | --- | --- |
| entrada CRT | runtime pronto | chama wrapper WinMain nativo | bootstrap iniciado | CRT encerra com retorno |
| argumento presente | command line não vazia | decodifica dados do launcher | credenciais globais preparadas | message box e retorno em erro |
| teste de unicidade | título já encontrado | não chama `Initialize` | segunda instância encerrada | retorno `0` |
| bootstrap | nenhuma janela anterior | COM/OLE, config, janela e owners | aplicação inicializada | retorno zero interrompe o fluxo |
| execução | bootstrap bem-sucedido | instala hook e entra no loop | mensagens e frames ativos | termina por `WM_QUIT` |
| retorno do loop | shutdown já executado no WndProc | COM/OLE e wrapper são desmontados | processo finaliza | sem relogin local |

### Vtables, vptrs e receptores

A vtable `0x005A6104` resolve `+0x04` para `FUN_0055BC0A` e `+0x0C` para
`FUN_0055D345`. O singleton publicado pelo construtor é o receptor usado pelo
thunk de janela. O deleting destructor ocupa `+0x00`; o shutdown global ocupa
`+0x08` e é chamado pelo WndProc, não pelo wrapper.

### Ownership

O wrapper possui o bloco da aplicação até o deleting destructor. A aplicação
possui janela e managers criados por `Initialize`; o loop apenas os consome.
O singleton global é uma referência publicada, não um segundo owner. O teardown
completo ocorre no ramo `WM_CLOSE` antes de `Run` retornar.

### Falha parcial

O CFG decompilado não mostra cleanup explícito do objeto da aplicação quando
`Initialize` retorna zero. Isso registra somente a ausência local da chamada;
não é usado para concluir leak nem para importar esse comportamento inseguro.

O nativo termina o processo quando a inicialização COM/OLE falha. Na source,
o player `TMVideoWnd` está explicitamente inativo e retorna valores neutros;
`DS_SOUND_CHANNEL` mantém um ciclo próprio `CoInitialize`/`CoUninitialize`.
Por isso esta ficha não adiciona um owner COM global enquanto não houver um
consumidor ativo que exija esse lifecycle. A diferença permanece explícita.

### Cleanup e teardown

O loop não possui cleanup autônomo dos managers. `WM_CLOSE` chama o slot
`+0x08/Finalize`, remove o hook, destrói a janela e publica `WM_QUIT`; só então
o wrapper desmonta COM/OLE e o objeto da aplicação. Esse contrato está fechado
em `application-close-global-shutdown.md`.

### Shutdown

Shutdown é terminal. Logout de personagem e relogin dentro da mesma conexão
não passam novamente por este bootstrap de processo. O caminho terminal e a
ordem dos owners estão fechados em `application-close-global-shutdown.md`.

### Logout e relogin

Logout de personagem e relogin dentro da mesma conexão pertencem a
`character-logout-selectchar-relogin.md`; eles trocam a cena mantendo o
processo e não repetem construtor, COM/OLE, registro da janela ou message loop.

## Wire, ABI e recursos

Este fluxo não introduz packet nem contrato wire. Os slots, offsets e globais
acima descrevem somente o ABI interno do binário nativo e não devem ser
copiados para a source.

Recursos e APIs observados:

- classe Win32 registrada pelo bootstrap e título usado na criação da janela;
- `config.txt`, dados globais e arquivo de mensagens carregados antes do loop;
- COM/OLE no thread principal do nativo;
- hook global de teclado instalado somente após bootstrap bem-sucedido;
- `GetMessage`/`PeekMessage`, `TranslateMessage` e `DispatchMessage`.

## Mapeamento atual

| Nativo 7.48 | Source atual | Estado |
| --- | --- | --- |
| `entry -> FUN_0055F7F9` | `wWinMain`, ID `acbeb4d7c73897744a3d` | adaptado |
| `FUN_0055B18F` | `NewApp::NewApp` | adaptado; asset root modernizado |
| `FindWindowA(NULL, título)` | `FindWindowA(nullptr, GameWindow_Title)` | corrigido neste lote |
| vslot `+0x04/FUN_0055BC0A` | `NewApp::Initialize`, ID `a4554b50ec02ebf74ba5` | localizado |
| `FUN_0055CA18` | `NewApp::InitDevice`, ID `86d5b7f9d1ed79540b47` | traçado; sequência essencial preservada |
| vslot `+0x0C/FUN_0055D345` | `NewApp::Run`, ID `ce9a04f5c259609b08a0` | localizado |
| `FUN_0055FA89 -> FUN_0055DAB8` | `WndProc -> g_pApp->MsgProc`, ID `00b0bdb56f6afaa96e47` | localizado |
| COM/OLE do processo | não existe no wrapper atual | delta registrado; consumidor global inativo |
| parser de launcher | filtros atuais são stubs | fora deste lote; exige contrato próprio |

## Matriz de delta

| Delta | Evidência | Impacto | Decisão |
| --- | --- | --- | --- |
| unicidade consultava `ClassName` como título | `CreateWindowEx` usa classe `"WYD"` e título `GameWindow_Title`; API recebe título no segundo argumento | segunda instância não encontrava a própria janela | corrigir para `GameWindow_Title` |
| resolução e `CLASSIC` divergem | o nativo oferece modos 16/32-bit e força UI version `2` | runtime atual usa modos 32-bit/widescreen e honra o valor configurado | preservar adaptação já intencional |
| janelas filhas auxiliares não existem na source | `static` e `Button` são criadas após a janela principal | nenhum consumidor atual foi comprovado | não importar sem contrato observável |
| limite visual do loop de `itemhelp` | condição decompilada sugere 65.000, mas buffer/stride comportam 6.500 | array e limite são 6.500 | manter normalização segura |
| `JBlur` não existe no nativo | source cria adaptador inerte após os owners nativos | objeto ficava sem teardown | preservar stub e completar ownership local |
| erro de `GetMessage` | qualquer retorno não zero segue para dispatch | a comparação anterior tratava apenas `1` e podia repetir após `-1` | inicializar `MSG`, registrar erro e encerrar o loop |
| source não possui COM/OLE global | chamadas explícitas no wrapper nativo | diferença de lifecycle, hoje sem consumidor global ativo | documentar e reavaliar com vídeo/COM ativo |
| `TMVideoWnd` é adaptador inativo | métodos retornam `E_NOTIMPL`/zero | AVI nativo não está implementado | não fingir paridade nesta ficha |
| parser de launcher não corresponde ao nativo | helpers nativos precedem unicidade/COM | login automático pode divergir | separar investigação de autenticação |

## Decisões

- Classificação da correção: `PARIDADE_NATIVA`. A source continua livre para
  usar o título atual do produto; o contrato preservado é procurar o mesmo
  título usado ao criar a janela.
- Usar `FindWindowA` explicitamente porque classe e título atuais são ANSI.
- Não importar offsets, strings históricas ou o título literal do executável
  nativo; `GameWindow_Title` é a única fonte da verdade na source atual.
- Não adicionar COM/OLE global antes de ativar um consumidor que dependa dele.
- Preservar a lista de resoluções, a leitura efetiva de `CLASSIC` e o limite de
  6.500 itens; a comparação nativa não demonstrou incompatibilidade nesses
  pontos.
- Completar apenas o ownership do `JBlur` exclusivo da source, sem ativar seu
  pipeline gráfico.
- Tratar `GetMessage == -1` como falha terminal local. Essa modernização não
  altera os retornos normais `0/1` nem importa o comportamento inseguro do
  pseudocódigo nativo.
- Não alterar nesta ficha o parser de launcher, AVI, updater ou failure cleanup
  parcial.

## Lacunas

- Resolver o contrato do argumento de launcher antes de substituir os filtros
  stubs ou afirmar login automático compatível.
- Se `TMVideoWnd` for ativado, fechar criação/release das interfaces DirectShow
  e então restaurar um owner COM/OLE balanceado no thread principal.
- O parser de launcher e o player de AVI continuam sem validação runtime; eles
  não são cobertos pelo teste de unicidade deste lote.

## Validação

- `STATICALLY VERIFIED`: callgraph nativo fechado do entrypoint até WndProc;
  classe e título comparados na source; o segundo argumento de `FindWindowA`
  agora coincide com o título passado à janela.
- `AUTOMATED TESTED`: `Release|Win32` e `Debug|Win32` compilaram com
  `-NoDeploy`; os 41.231 checks de arquitetura, os testes dos inventários, o
  gate de staleness e o validador das fichas passaram. O inventário regenerado
  contém 547 arquivos, 445.407 linhas, 3.977 símbolos e zero erro de parse.
- `CLIENT-TESTED`: o `WYD.exe` de `Release|Win32`, executado com os assets de
  `client748`, permaneceu ativo; uma segunda execução encerrou em até três
  segundos com código `0`, sem encerrar a primeira. Os dois processos usados
  no teste foram fechados ao final.
- `CLIENT-TESTED` limitado ao shutdown fora de Field: os candidatos Debug e
  Release abriram a janela `WYDESTINY MMORPG`, receberam `WM_CLOSE` e
  encerraram normalmente com código `0`. O `project.exe` instalado não foi
  substituído; seu SHA-256
  permaneceu `1F6D5FBD870B782FBAC74AE30A493B95CD1E1ECA53C1010EA84F4B391B141E3F`.
