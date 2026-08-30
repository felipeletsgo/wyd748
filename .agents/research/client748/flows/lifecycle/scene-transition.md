---
id: scene-transition
title: Criar e substituir a cena ativa no lifecycle 7.48
subsystem: lifecycle
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-30
---

# Criar e substituir a cena ativa no lifecycle 7.48

## Pergunta

Como o client nativo WYD 7.48 converte o estado solicitado em uma cena,
seleciona e inicializa a cena nova, preserva a mensagem entre cenas e agenda a
cena anterior para destruição? Quais partes da implementação do TMProject
7.69+ podem ser aproveitadas sem importar contrato ou layout de versão nova?

## Fronteira de evidência

- Executável: `client748/wyd.exe nativo+patches/WYD.exe`.
- SHA-256: `8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593`.
- Projeto Ghidra: `WYD748Native_20260821.gpr`; corpus auxiliar com 4.146
  funções em `GhidraAnalysis/20260821/decompiled`.
- Exports focados versionados:
  `.agents/research/client748/exports/scene-transition-focused-next.tsv`,
  `scene-lifecycle-network-focused.tsv`, `scene-lifecycle-helpers.tsv` e
  `scene-lifecycle-missing-helpers.tsv`.
- Ledger da rodada posterior:
  `inventory/scene-transition-evidence-log.md`. Ele inventaria os 54
  exports TSV gerados em `%TEMP%\codex-wyd748-lifecycle-149205b7`, separa o
  que foi interpretado do que ainda é somente pista e preserva a pergunta de
  cada export sem versionar aproximadamente 34,87 MiB de recortes regeneráveis.
- Source recompilável consultada: `client-source/tmproject/Projects/TMProject/`
  (`ObjectManager`, `TMScene`, `TreeNode` e `NewApp`).
- Servidor: não há regra server-side direta para troca de cena local; qualquer
  intenção posterior continua sujeita à autoridade do WYD-Go.

Os exports são aceleradores de busca. A ordem local entre coletor, deleting
destructor, cleanup específico/base e detach, o caller de shutdown da aplicação
e o lifecycle do hook global de teclado já foram fechados; callers de estado,
ownership dos demais campos da aplicação, convergência integral do teardown e
transições de logout/relogin ainda precisam ser fechados no projeto Ghidra
antes de transformar esta ficha em contrato de implementação.

## Fluxo nativo 7.48

### Entrada observável

`LOCATED`: o lifecycle da aplicação passa pela vtable em `0x005A6104` e pelo
loop de mensagens/input/rede em `FUN_0055DAB8`. A inicialização geral de
subsistemas está localizada em `FUN_0055CA18`, o bootstrap/configuração em
`FUN_0055BC0A` e o shutdown em `FUN_0055D066`. A criação/troca de cena converge
para `FUN_004B3500` e a substituição da cena ativa para `FUN_004B37C9`.

`CONFIRMED`: `FUN_0055B18F` instala a vtable `0x005A6104` no objeto da
aplicação em `0x0055B229` e publica o mesmo receptor em `DAT_013B71E0` em
`0x0055B232`. `FUN_0055FA89` é um thunk de janela de quatro argumentos: copia
os quatro valores da pilha, carrega `ECX = DAT_013B71E0` em `0x0055FA9C`,
chama `FUN_0055DAB8` em `0x0055FAA2` e retorna com `RET 0x10`. Isso fecha a
origem do receptor do dispatcher como o singleton da aplicação; não comprova,
por si só, o caller virtual do slot `+0x08/FUN_0055D066`.

`CONFIRMED`: o slot `+0x18` da vtable da aplicação, `FUN_0055EE1E`, grava seu
argumento em `app+0xF8` e também em `DAT_013B71E8`. No branch `0x464` de
`FUN_0055DAB8`, o dispatch em `0x0055E80D` usa o receptor em `app+0xF8`,
enquanto o dispatch em `0x0055E8CA` usa `app+0xF4`. O segundo receptor, seu
ownership e o significado observável desse evento permanecem `UNRESOLVED`;
esses acessos localizam a cadeia da aplicação, mas ainda não a conectam por
completo à transição de cena.

### Callers

`LOCATED`: `FUN_004B3500` é exposta pelo slot `+0x54` da vtable efetiva do
`ObjectManager` em `0x005A45FC`; `FUN_004B37C9` ocupa o slot `+0x58`. A
instância instala essa vtable por `FUN_004AFAC0` e `FUN_004AFBA0`. O endereço
`0x005A45F0` é um preâmbulo adjacente, não a base da vtable.

`CONFIRMED`: o consumidor de packets do `ObjectManager` é `FUN_004B263E`, no
slot `+0x08` de `0x005A45FC`. Ele captura a raiz ativa em `DAT_0067CF38`, chama
primeiro o slot `+0x04` dessa raiz e percorre em profundidade os filhos ainda
ativos; o primeiro handler que retorna `1` encerra o dispatch. Portanto o
receptor virtual nasce da cena ativa, não de um dispatcher global fixo.

Para a resposta `0x114`, os dois caminhos de mudança para a cena `0` são
alternativas por estado. `FUN_00492E7D` é o override `+0x04` da cena `0`,
enquanto `FUN_004A626E` ocupa o mesmo slot na cena `5`. O primeiro chama
`FUN_0048529B` e solicita estado `9`, normalizado por `FUN_004B3500`; o segundo
atualiza o personagem selecionado e solicita diretamente estado `0`.

### Função principal: `FUN_004B3500`

`CONFIRMED`: o estado solicitado `9` é convertido para o estado interno `0`.
Depois da conversão, somente os estados internos `0`, `5`, `7` e `8` entram na
seleção de construtor observada. Os quatro construtores nativos localizados são:

| Estado interno | Construtor observado | Claim |
| --- | --- | --- |
| `0` | `FUN_004343A4` | `CONFIRMED` |
| `5` | `FUN_0049EE30` | `CONFIRMED` |
| `7` | `FUN_004A8CCF` | `CONFIRMED` |
| `8` | `FUN_00431D00` | `CONFIRMED` |

Cada construtor instala seu próprio vptr/vtable. A cena nova é selecionada por
`FUN_004B37C9` e inicializada por uma chamada virtual no slot `+0x4C` do
receptor. Em sucesso, a cena é anexada à árvore pelo helper `FUN_0054AC09`,
usando a raiz localizada em `manager + 0x1B07C`.

`CONFIRMED`: quando a criação não produz uma cena, o nativo mostra uma mensagem
de erro e agenda o fechamento da janela por `PostMessageA`. Quando a cena existe,
mas sua inicialização falha, a cena parcialmente criada é destruída quando
aplicável, uma mensagem de erro é exibida e o mesmo fechamento é agendado.

`CONFIRMED`: para a cena do estado `0`, o slot `0x005A4294 +0x4C`, armazenado
em `0x005A42E0`, resolve para `FUN_00435B13`. O caller real é a chamada
indireta `CALL [EDX+0x4C]` em `0x004B370F`; `FUN_004B3500` testa `EAX` em
`0x004B3712` e salta ao attach em `0x004B3783` quando o resultado é diferente
de zero. O único retorno normal de `FUN_00435B13` passa pelo log
`">> Init Field Scene::End"`, chama `FUN_00431159`, grava `EAX=1` em
`0x00441810` e retorna em `0x00441822`. Portanto, esse override sempre informa
sucesso quando conclui normalmente: o ramo `"Initialize Scene Fail."` de
`0x004B3716..0x004B3781` não é alcançável por retorno falso da cena `0`.
Exceções, access violations ou outras falhas não locais não são convertidas em
retorno zero por essa função; esta evidência não prova que toda alocação parcial
seja tolerada.

`CONFIRMED`: para a cena do estado `5`, o slot `0x005A44B4 +0x4C`, armazenado
em `0x005A4500`, resolve para `FUN_0049F0E7` e usa o mesmo caller indireto em
`FUN_004B3500:0x004B370F`. O initializer possui 2.509 instruções exportadas e
um único `RET`, em `0x004A218B`. Quando `FUN_00541065`, chamada em
`0x0049F318`, retorna zero, o branch registra `"DataFile Not Found  "`, mostra
`MessageBoxA("DataFile Not Found.", "File Lost", 0)`, agenda
`PostMessageA(hwnd, WM_CLOSE, 0, 0)`, grava `EAX=0` em `0x0049F35F` e salta
diretamente ao epílogo em `0x004A217E`. Esse é o único salto normal que alcança
o epílogo sem passar por `MOV EAX,1` em `0x004A2179`; os demais `XOR EAX,EAX`
do corpo são valores transitórios. Portanto, os demais caminhos normais que
concluem a função retornam `1`. O papel mais específico de `FUN_00541065`
permanece não confirmado.

Quando recebe esse zero, `FUN_004B3500` também destrói a cena parcial quando
aplicável, mostra `"Initialize Scene Fail."` e agenda outro `WM_CLOSE`. Assim,
essa falha da cena `5` produz diagnóstico e fechamento tanto no initializer
quanto na camada genérica de criação da cena.

`CONFIRMED`: para a cena do estado `7`, `FUN_004A8CCF` instala o vptr
`0x005A4544` em `0x004A8D14`; o slot `+0x4C`, armazenado em `0x005A4590`,
resolve para `FUN_004A8F14` e usa o mesmo caller indireto em
`FUN_004B3500:0x004B370F`. O initializer possui 1.626 instruções e um único
`RET`, em `0x004AAD83`. Quando `FUN_00541065`, chamada em `0x004AA982`,
retorna zero, o branch registra `"DataFile Not Found  "`, mostra
`MessageBoxA("DataFile Not Found.", "File Lost", 0)`, agenda
`PostMessageA(hwnd, WM_CLOSE, 0, 0)`, executa `XOR EAX,EAX` em `0x004AA9C9`
e salta de `0x004AA9CB` para o epílogo em `0x004AAD75`. Esse salto evita o
`MOV EAX,1` de `0x004AAD70`; os demais caminhos normais passam por essa
atribuição. Portanto, a falha retorna `0` e todos os demais caminhos normais
retornam `1`. O papel mais específico de `FUN_00541065` permanece não
confirmado.

Ao receber esse zero, `FUN_004B3500` aplica o mesmo tratamento genérico:
destrói a cena parcial quando aplicável, mostra `"Initialize Scene Fail."` e
agenda outro `WM_CLOSE`. Assim, as cenas `5` e `7` compartilham o mesmo padrão
de falha dupla observável, sem que isso autorize inferir a semântica do helper.

`CONFIRMED`: para a cena do estado `8`, `FUN_00431D00` instala o vptr
`0x005A421C` em `0x00431D4A`; o slot `+0x4C`, armazenado em `0x005A4268`,
resolve para `FUN_00432181` e usa o mesmo caller indireto em
`FUN_004B3500:0x004B370F`. O initializer possui 431 instruções e um único
`RET`, em `0x00432958`. Quando `FUN_00541065`, chamada em `0x00432665`,
retorna zero, o branch de `0x0043266C` registra `"DataFile Not Found  "`,
mostra `MessageBoxA("DataFile Not Found.", "File Lost", 0)`, agenda
`PostMessageA(hwnd, WM_CLOSE, 0, 0)`, executa `XOR EAX,EAX` em `0x004326AC` e
salta de `0x004326AE` para `0x0043294B`. Esse salto evita o `MOV EAX,1` de
`0x00432946`; todos os demais caminhos normais passam por essa atribuição.
Portanto, a falha retorna `0` e os demais caminhos normais retornam `1`. O
papel mais específico de `FUN_00541065` permanece não confirmado.

Ao receber esse zero, `FUN_004B3500` destrói a cena `8` parcialmente criada,
mostra `"Initialize Scene Fail."` e agenda outro `WM_CLOSE`, repetindo o padrão
de falha dupla das cenas `5` e `7`. Com isso, os initializers das cenas
`0/5/7/8` estão fechados no CFG normal.

### Cenas dos estados 0 e 5

`CONFIRMED`: o ramo `case 0` de `FUN_004B3500` chama `FUN_004343A4`, que
instala explicitamente o vptr `0x005A4294`. O slot `+0x04` dessa tabela aponta
para `FUN_00492E7D`. Essa é a raiz de cena consultada por `FUN_004B263E` quando
o estado ativo é `0`.

`CONFIRMED`: o ramo `case 5` de `FUN_004B3500` aloca `0x26EFC` bytes e chama
`FUN_0049EE30`. Esse construtor instala explicitamente o vptr `0x005A44B4`,
resolvendo essa vtable como a cena do estado `5`:

```text
0x005A44B4 +0x00 FUN_004A8910  deleting destructor
0x005A44B4 +0x04 FUN_004A626E  packet handler
0x005A44B4 +0x4C FUN_0049F0E7  initialize
0x005A44B4 +0x58 FUN_004A32DD  control/event handler
0x005A44B4 +0x64 FUN_0049AD57  método ainda sem semântica fechada
```

O deleting destructor `FUN_004A8910` chama `FUN_0049F053` e libera o bloco
somente quando solicitado. `FUN_0049F053` restaura o vptr `0x005A44B4`, destrói
o membro em `scene[0x9B89]` quando presente e converge em `FUN_00494C00`.

### Evento de seleção e packet `0x213`

`CONFIRMED`: em `FUN_004A32DD`, o evento `0x1204` passa por debounce superior
a 2.000 ms. O byte global em `+0xEF4` fornece um índice assinado, que precisa
estar entre `0` e `3`; o personagem correspondente precisa existir e estar
habilitado. A função então zera um packet de `0x24` bytes, grava opcode `0x213`
em `+0x04`, grava o índice em `+0x0C` e chama
`FUN_0042550E(packet, 0x24)` no callsite `0x004A3422`. Após o envio, grava o
timestamp e desabilita três controles por chamadas no slot virtual `+0x64`.

`CONFIRMED`: `FUN_00493E70` aloca o container de controles com `0x154` bytes e
chama `FUN_0040C2CD` em `0x004940B1`, passando a própria cena como owner. O
objeto retornado fica em `scene[10]` (`scene+0x28`). O construtor instala o vptr
principal `0x005A3F34`, constrói em `container+0x24` um receptor embutido com
vptr `0x005A3F30` e guarda a cena em `container+0x2C`.

No registro de tipo `2`, `FUN_004974EC` lê `0x28` bytes, aloca um `SButton` de
`0x208` bytes e chama `FUN_00402F01`. O ID do controle fica em `SButton+0x44`;
`FUN_0040C030` guarda em `SButton+0x5C` o receptor
`scene[10]+0x24`. No release `0x202`, `FUN_004032E8` chama o slot zero desse
receptor com o ID do botão e ação `0`.

O slot zero de `0x005A3F30` é `FUN_0040CDA4`. Com
`this=container+0x24`, ela lê a cena em `[this+8]` (`container+0x2C`), retorna
`0` quando o owner é nulo e, caso contrário, encaminha os dois argumentos ao
slot virtual `+0x58` da cena no callsite `0x0040CDCA`. Na cena 5, a vtable
`0x005A44B4` resolve esse slot para `FUN_004A32DD`. Assim, a origem do evento
está fechada como `SButton release -> receptor embutido -> FUN_0040CDA4 ->
scene vtable +0x58 -> FUN_004A32DD -> 0x1204`.

`FUN_0049F0E7` carrega `UI_SelCharScene.txt` ou `UI_SelCharScene2.txt`, procura
o controle `0x1204` em `scene[10]` e o guarda em `scene[0x9B94]`, ligando o ID
do recurso carregado ao botão usado pelo handler.

### Transporte do `0x213`

`CONFIRMED`: `FUN_0042550E` é método do objeto de rede em `DAT_013B71E4` e
recebe `(buffer, tamanho)`. Ela enfileira por `FUN_00424C2C(buffer, size)`,
ignora o retorno desse enqueue, chama `FUN_00425266()` e retorna somente o
resultado do flush.

`FUN_00424C2C` escolhe chave/seed e chama `FUN_00424DFE`. Esta última exige
socket não nulo e `queued + size < 0x20000`, grava tamanho em `packet+0x00`,
seed em `+0x02`, checksum em `+0x03` e `DAT_0092E658` em `+0x08`, cifra os
bytes `+0x04..size-1`, preserva os quatro bytes iniciais e acrescenta o
resultado ao buffer de saída. Retorna `1` em sucesso e `0` para socket ou
limite de buffer inválido.

`FUN_00425266` zera posições e retorna `0` se o socket for nulo. Com socket
válido, compacta pendências por `FUN_00425200`, valida os limites e realiza uma
tentativa de `send`. `send == -1` consulta `WSAGetLastError`; esse caminho e um
envio parcial mantêm os dados pendentes e podem retornar `1`. O envio completo
zera `queued/sent` e retorna `1`. Limites inválidos registram erro, zeram o
estado e retornam `0`.

### Resposta `0x114`

`CONFIRMED` dentro da ficha ainda `LOCATED`: `FUN_004B263E` despacha o packet
para o slot `+0x04` da raiz em `DAT_0067CF38`. Na cena `5`, essa chamada resolve
para `FUN_004A626E`; o handler processa `0x114`, atualiza o estado global do
personagem, solicita sincronamente estado `0` por `ObjectManager+0x54` e retorna
`1`. Esse retorno faz `FUN_004B263E` encerrar, sem reenviar o mesmo packet à
nova raiz. Na cena `0`, o slot resolve para `FUN_00492E7D`, que chama primeiro a
base `FUN_0049889A` e, se ela não consumir o packet, trata `0x114` por
`FUN_0048529B`. `FUN_004A626E` também chama essa base antes de seus opcodes.
Logo, os dois handlers são overrides mutuamente exclusivos por cena, não dois
consumidores sequenciais. A ligação observável da seleção permanece
`0x1204 -> 0x213 -> 0x114 -> cena 0`.

### Substituição: `FUN_004B37C9`

`CONFIRMED`: a função salva a cena atual, grava a cena nova em
`DAT_0067CF38`, marca a cena anterior para deleção e transfere para a cena nova
o estado de controles da mensagem em `scene + 0x84`. Ela também copia estado e
tempo relacionados à mensagem e encaminha a cena anterior ao slot
`ObjectManager + 0x64`. A identidade do receptor final foi resolvida pelas
instruções como o próprio `ObjectManager`.

`CONFIRMED` dentro da ficha ainda `LOCATED`: `FUN_004B16C0` consome a marca de
deleção, mas limita a varredura à subárvore da cena que estava ativa ao entrar.
Ele captura `DAT_0067CF38` como raiz/sentinela em
`0x004B16C9..0x004B16D4` e, ao subir novamente até essa sentinela, deixa de
seguir irmãos da raiz em `0x004B1763..0x004B1777`. Só depois processa
separadamente a cena anterior guardada em `manager+0x1B088`, em
`0x004B1783..0x004B17C2`, e zera `+0x1B088/+0x1B08C`.

`CONFIRMED`: quando o deleting destructor da cena anterior alcança
`FUN_00494C00`, a função recarrega o `ObjectManager` de `DAT_013B71E8`, zera
`manager+0x1B088` em `0x00494D31` e só então chama novamente
`FUN_004B16C0`, em `0x00494D41`. Portanto, a reentrada percorre somente a
subárvore da cena nova e não possui mais o único ponteiro explícito que poderia
selecionar a cena antiga uma segunda vez. `FUN_0054AA45`, chamado em
`0x00494D70`, desanexa a cena antiga depois dessa coleta. A ordem local e a
prevenção de dupla destruição estão fechadas; shutdown global e relogin ainda
não estão.

### Callees e helpers

- `FUN_004343A4`, `FUN_0049EE30`, `FUN_004A8CCF` e `FUN_00431D00`: construtores
  das cenas dos estados `0`, `5`, `7` e `8`, respectivamente.
- Virtual `+0x4C`: inicialização da cena nova; retorno e falha estão fechados
  no CFG normal para as cenas `0`, `5`, `7` e `8`.
- `FUN_0054AC09`: anexa um objeto/cena à árvore de ownership.
- `FUN_004B21C9`, slot `ObjectManager+0x64`: marca `scene+0x14 = 1` e
  `manager+0x1B08C = 1` quando a cena existe.
- `FUN_004B16C0`: percorre a árvore a partir de `DAT_0067CF38`, consome
  `node+0x14`, chama o deleting destructor pelo slot `+0x00`, interrompe a
  subida ao reencontrar a raiz/sentinela e finaliza separadamente o objeto em
  `manager+0x1B088` antes de zerar `+0x1B088/+0x1B08C`.
- `FUN_00494C00`: cleanup base comum das quatro cenas; instala vptr-base
  `0x005A443C`, ajusta timer, destrói três membros opcionais, zera
  `ObjectManager+0x1B088` antes da reentrada em `FUN_004B16C0`, limpa estado
  adicional e converge em `FUN_0054AA45`.
- `FUN_0054AA45`: destrói filhos, reconecta pai/irmãos e desanexa o nó da
  árvore; seu uso em handlers de unwind confirma o papel de cleanup base.
- `FUN_0054A9E0`: inicializa os links/flags do nó raiz e instala o vptr
  `0x005A5CFC`; o slot zero dessa tabela é o deleting destructor
  `FUN_0054AE10`, que converge em `FUN_0054AA45` e só libera o bloco quando
  `flag & 1`.
- `FUN_00401152`: instala `0x005A3420` e chama `FUN_0054AA45`; não é o
  deleting destructor da raiz. O slot zero de `0x005A3420` é
  `FUN_0040B980`, que chama `FUN_00401152` e libera o bloco quando `flag & 1`.
- `FUN_00423CA0`, `FUN_00423DD8`, `FUN_004BAC70` e `FUN_004BACFF`: helpers de
  transporte/socket e timer localizados no lifecycle adjacente; sua interação
  completa com logout, relogin e troca de cena permanece pendente.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| Estado solicitado `9` | dispatcher/bootstrap fornece o estado | `FUN_004B3500` | estado interno `0` | câmera e cena são preparados | cadeia de entrada ainda `LOCATED` |
| Estado interno `0`, `5`, `7` ou `8` | estado aceito | construtor específico; virtual `+0x4C` | cena nova criada/inicializada | vptr próprio, seleção da cena | falha parcial destrói cena e fecha janela |
| Initializer da cena `5` | `FUN_00541065` retorna zero | `FUN_0049F0E7` | retorno `0` ao caller genérico | log, `MessageBoxA` e primeiro `WM_CLOSE` | `FUN_004B3500` destrói a cena parcial, exibe o segundo diagnóstico e agenda outro `WM_CLOSE` |
| Initializer da cena `7` | `FUN_00541065` retorna zero | `FUN_004A8F14` | retorno `0` ao caller genérico | log, `MessageBoxA` e primeiro `WM_CLOSE` | `FUN_004B3500` destrói a cena parcial, exibe o segundo diagnóstico e agenda outro `WM_CLOSE` |
| Initializer da cena `8` | `FUN_00541065` retorna zero | `FUN_00432181` | retorno `0` ao caller genérico | log, `MessageBoxA` e primeiro `WM_CLOSE` | `FUN_004B3500` destrói a cena parcial, exibe o segundo diagnóstico e agenda outro `WM_CLOSE` |
| Cena criada com sucesso | cena inicializada | `FUN_0054AC09` | cena sob a raiz `manager+0x1B07C` | inserida no início da lista de filhos | ordem global de shutdown pendente |
| Cena anterior existente | cena atual não nula | `FUN_004B37C9` | nova cena global; anterior marcada | mensagem/tempo são copiados | teardown posterior não fechado |
| Release `0x202` do `SButton` ID `0x1204` | receptor `scene[10]+0x24`, owner não nulo, índice `0..3`, personagem existente/habilitado e debounce > 2 s | `FUN_004032E8 -> FUN_0040CDA4 -> scene vtable +0x58 -> FUN_004A32DD` | evento `0x1204`; packet `0x213` de `0x24` bytes | timestamp atualizado; três controles desabilitados | receptor retorna `0` se a cena for nula |
| Packet `0x213` | socket e espaço no buffer | `FUN_0042550E -> FUN_00424C2C -> FUN_00424DFE -> FUN_00425266` | dados cifrados/enfileirados; flush tentado | seed, checksum e tempo gravados | enqueue falho pode ser mascarado pelo retorno do flush |
| Resposta `0x114` | raiz ativa em `DAT_0067CF38`; slot de packet `+0x04` | `FUN_004B263E -> FUN_004A626E` na cena 5 ou `FUN_004B263E -> FUN_00492E7D` na cena 0 | estado `0` solicitado; `9` é normalizado para `0` no caminho da cena 0 | personagem selecionado atualizado na cena 5; retorno `1` encerra o dispatch | handlers são overrides alternativos, sem segundo consumo do mesmo packet |
| Cena marcada | `scene+0x14 != 0` | `FUN_0055D345 -> FUN_004B16C0` | subárvore ativa coletada; previous scene finalizada | sentinela impede sair da raiz; `+0x1B088` zerado antes da reentrada | ordem local de detach fechada |
| Loop de aplicação | janela e subsistemas ativos | `FUN_0055D345` | tick concluído e coleta condicional executada | testa `manager+0x1B08C`; chama `FUN_004B16C0` com `app+0xF8` | origem completa do loop ainda parcialmente resolvida |
| Shutdown | `WM_CLOSE`; retorno de `app+0xF4` alcança o baseline `DAT_013B7220 + 0xBB8` | `FUN_0055DAB8:0x0055EB9C -> app vslot +0x08 -> FUN_0055D066` | recursos e owners desmontados; janela destruída e quit postado | limpa `DAT_013B71F0`, grava `DAT_013B7228=1`, chama `FUN_00423C61`, `DestroyWindow` e `PostQuitMessage(0)` | caller fechado; unidade do gate, `FUN_00423C61` e classes dos campos ainda não confirmados |
| Logout/relogin | transição de sessão | não resolvido | não resolvido | não resolvido | bloqueia `TRACED`/`CONTRACT` |

### Vtables, vptrs e receptores

Vtable da aplicação em `0x005A6104`:

```text
+0x00 FUN_0055F3E0
+0x04 FUN_0055BC0A
+0x08 FUN_0055D066
+0x0C FUN_0055D345
+0x10 FUN_0055EDF7
+0x14 FUN_0055D6E6
+0x18 FUN_0055EE1E
+0x1C FUN_0055EE45
```

`CONFIRMED`: `FUN_0055D345:0x0055D5BC` usa a própria aplicação como receptor
do slot `+0x14/FUN_0055D6E6` quando `app+0xE8 != 0` e
`[app+0xE8]+0x10C != 2`. A função consome `app+0xE0`, `app+0xF8` e
`DAT_0067CF38`, chama `FUN_0040C0F0`, `FUN_00494DCF`, `FUN_004AFFF7`,
`FUN_004B7F0D` e `__ftol`, e em `0x0055D8F7` despacha o slot `+0x48` do
objeto em `scene+0x28`. Esse conjunto a fecha como etapa de frame/render da
aplicação; ela não chama estruturalmente os slots de cena `+0x14/+0x20`.

Vtable efetiva do `ObjectManager` em `0x005A45FC`:

```text
+0x08 FUN_004B263E  dispatch de packet pela árvore da cena ativa
+0x54 FUN_004B3500
+0x58 FUN_004B37C9
+0x5C FUN_004B3952
+0x60 FUN_004B21F0
+0x64 FUN_004B21C9
+0x68 FUN_004B27DA
```

`FUN_004AFAC0` e `FUN_004AFBA0` instalam o vptr efetivo do `ObjectManager`.
`FUN_004B1EA9` também instala `0x005A45FC`, inicializa
`DAT_0067CF38 = 0`, grava estado atual `-1` e cria os objetos raiz/ownership.
`FUN_004B3A20 -> FUN_004B2155` destrói `manager+0x1B07C`, zera esse campo e
`DAT_0067CF38`, e opcionalmente libera o próprio manager.

Vtable confirmada da raiz da árvore em `0x005A5CFC`:

```text
+0x00 FUN_0054AE10  deleting destructor da raiz
```

`FUN_0054A9E0` instala esse vptr. `FUN_0054AE10` chama `FUN_0054AA45` e
condiciona `FUN_0058EA3A(this)` a `flag & 1`. A tabela `0x005A3420` pertence a
outra classe: seu slot zero é `FUN_0040B980`, que chama `FUN_00401152`; por
isso `FUN_00401152` não deve mais ser descrita como deleting destructor da raiz.

Vtable confirmada da cena do estado `0` em `0x005A4294`:

```text
+0x00 FUN_00493BC0  deleting destructor
+0x04 FUN_00492E7D  packet handler
+0x14 FUN_00453C59
+0x20 FUN_004776C3
+0x4C FUN_00435B13  initialize
+0x58 FUN_004662C5  control/event handler
+0x64 FUN_0049AD57
```

Vtable confirmada da cena do estado `5` em `0x005A44B4`:

```text
+0x00 FUN_004A8910  deleting destructor
+0x04 FUN_004A626E  packet handler
+0x14 FUN_004A2F7A
+0x20 FUN_004A2AEF
+0x4C FUN_0049F0E7  initialize
+0x58 FUN_004A32DD  control/event handler
+0x64 FUN_0049AD57
```

Vtable confirmada da cena do estado `7` em `0x005A4544`:

```text
+0x14 FUN_004AAD84
+0x20 FUN_004AAEDC
+0x4C FUN_004A8F14  initialize
```

`FUN_004A8CCF` instala esse vptr; os demais slots não recebem semântica nova
nesta unidade documental.

Vtable confirmada da cena do estado `8` em `0x005A421C`:

```text
+0x14 FUN_00433961
+0x20 FUN_00432959
+0x4C FUN_00432181  initialize
```

`FUN_00431D00` instala esse vptr em `0x00431D4A`; os demais slots não recebem
semântica nova nesta unidade documental.

`LOCATED`: a varredura global de chamadas virtuais encontrou 34 hits para o
offset `+0x14` e 53 para `+0x20`; cada scan examinou 8.173 candidatos e
441.614 instruções. Entre os callsites próximos do `ObjectManager`, estão
`FUN_004B29B9:0x004B29F3` no slot `+0x14` e
`FUN_004B2D35:0x004B2D86` no slot `+0x20`. O offset prova somente a forma da
chamada: receptor, vptr, classe e semântica continuam não atribuídos.

O container de controles da cena 5 fica em `scene+0x28`. Seu vptr principal é
`0x005A3F34`; o receptor embutido em `container+0x24` usa a tabela unitária
`0x005A3F30`:

```text
0x005A3F30 +0x00 FUN_0040CDA4  encaminha (control_id, action) à cena +0x58
```

`FUN_0040C2CD` associa ambos os objetos e grava a cena em `container+0x2C`, que
é `[receiver+8]` para `FUN_0040CDA4`.

Vtable do timer em `0x005A4688`:

```text
+0x00 FUN_004BAEB0  deleting destructor
+0x04 FUN_004BADA5  inicializa base/timestamp
+0x08 FUN_004BADDF  calcula tempo e atualiza DAT_0092E658
+0x0C FUN_004BAE89  setter
+0x10 FUN_004BAE9F  getter
```

`FUN_004BAC70` instala essa vtable e publica o timer em `DAT_0092E654`.

### Ownership

`CONFIRMED`: após inicialização bem-sucedida, `FUN_0054AC09` anexa a cena à
árvore cujo root é derivado de `manager + 0x1B07C`. A cena anterior não é
liberada imediatamente durante a substituição: ela é marcada para deleção e
encaminhada ao caminho do `ObjectManager`. `FUN_004B16C0` é o consumidor
localizado dessa marca, e `FUN_0054AA45` desanexa o nó da árvore após destruir
filhos e religar os vínculos intrusivos. A inserção por `FUN_0054AC09` ocorre
no início da lista: `child+0x04 = parent`, `child+0x0C = antigo primeiro
filho`, `parent+0x10 = child` e, quando havia primeiro filho, seu `+0x08` passa
a apontar para o novo nó. A ordenação local entre iteração, deleting destructor,
cleanup base, previous scene e detach está fechada; o ownership dos campos da
aplicação e a ordem global de shutdown permanecem parciais.

### Falha parcial

`CONFIRMED`: criação nula e falha de inicialização têm diagnóstico visível e
solicitam fechamento da janela. Falha de inicialização também contempla a
destruição da cena parcial quando aplicável. Ainda não foi fechado se efeitos,
controles ou filhos alocados antes da falha exigem uma limpeza adicional por
tipo de cena. Para a cena `0`, `FUN_00435B13` não produz retorno falso em
conclusão normal. Para a cena `5`, `FUN_0049F0E7` retorna zero quando
`FUN_00541065` falha, depois de emitir seu próprio diagnóstico e `WM_CLOSE`; a
camada genérica repete ambos os side effects e destrói a cena parcial. O
initializer da cena `7`, `FUN_004A8F14`, repete esse contrato de retorno e
falha observável. O initializer da cena `8`, `FUN_00432181`, também retorna
zero após a mesma falha de `FUN_00541065` e retorna um nos demais caminhos
normais; a camada genérica repete destruição parcial, diagnóstico e
`WM_CLOSE`. A semântica específica do helper permanece não confirmada.

### Cleanup e teardown

`CONFIRMED`: `FUN_004B21C9` grava as marcas em `scene+0x14` e
`manager+0x1B08C`; `FUN_004B16C0` consome a marca da cena e chama seu deleting
destructor. Os cleanups específicos convergem no mesmo cleanup base:

```text
estado 0: FUN_00493BC0 -> FUN_004358DA -> FUN_00494C00
estado 5: FUN_004A8910 -> FUN_0049F053 -> FUN_00494C00
estado 7: FUN_004AE5C0 -> FUN_004A8E29 -> FUN_00494C00
estado 8: FUN_00433A10 -> FUN_00431F13 -> FUN_00494C00
```

Os quatro construtores chamam `FUN_00493E70`. No teardown,
`FUN_00494C00` instala o vptr-base `0x005A443C`, limpa membros e estado, chama
`FUN_004B16C0` e termina em `FUN_0054AA45`. Antes da reentrada no coletor,
zera `manager+0x1B088`; junto da sentinela de `FUN_004B16C0`, isso impede que a
cena antiga seja selecionada uma segunda vez. A coleta fica:

```text
tick marca/coleta -> subárvore da cena nova -> previous scene
-> deleting destructor específico -> FUN_00494C00
-> zera previous scene -> reentrada limitada à cena nova -> FUN_0054AA45
```

Continuam `LOCATED` a desativação de input/foco e o teardown após erro,
shutdown, logout e relogin; por isso a ficha não é promovida a `TRACED`.

### Shutdown

`CONFIRMED` dentro da ficha ainda `LOCATED`: `FUN_0055D345` é o loop principal
da vtable da aplicação. Depois do tick, testa `manager+0x1B08C` em
`0x0055D6B3`, carrega `ECX = app+0xF8` em `0x0055D6BF` e chama
`FUN_004B16C0` em `0x0055D6C5`. Isso fecha o caller periódico do coletor de
cenas marcadas.

`LOCATED`: `FUN_0055D066` é o slot `+0x08` de shutdown. A ordem observada no
corpo é:

1. libera três recursos por entrada em 100 registros;
2. chama o deleting destructor virtual de `DAT_013B71E8`;
3. finaliza `app+0xF4`;
4. finaliza `app+0xE0`;
5. finaliza `app+0xE4` por `FUN_0055F410`;
6. finaliza `app+0xFC` por `FUN_0055F440`;
7. finaliza `app+0xEC` por `FUN_00493CF0`;
8. finaliza `app+0xE8`;
9. finaliza `app+0xF0`;
10. zera `DAT_013B71E4`;
11. chama `DeleteObject(app+0x110)`;
12. opcionalmente chama `FUN_0059DFE0(DAT_013B7364)`.

`CONFIRMED`: `FUN_0055F7F9` aloca `0x114` bytes e chama `FUN_0055B18F`, que
constrói e publica a aplicação. Em `0x0055F9F9`, o wrapper chama o slot
`+0x04/FUN_0055BC0A` para o bootstrap; após sucesso, chama o slot
`+0x0C/FUN_0055D345` para o loop principal e, quando esse loop retorna, chama
o slot `+0x00/FUN_0055F3E0` com flag `1`. O caminho normal decompilado não
chama o slot `+0x08/FUN_0055D066`. Os caminhos anteriores ao bootstrap e a
falha retornada pelo bootstrap também não exibem cleanup explícito no CFG
decompilado; isso registra a ausência local da chamada, sem concluir leak.

`CONFIRMED`: o slot `+0x00/FUN_0055F3E0` chama `FUN_0055B26F`, que reinstala
a vtable `0x005A6104` e chama apenas `FUN_004312DD`; depois, se `flag & 1`, o
deleting destructor libera o bloco da aplicação. `FUN_004312DD` fecha e zera
`DAT_005CCFAC`, criado por `FUN_004310E0` no fluxo que imprime
`"Logging System turn on"`. Não há chamada a `FUN_0055D066` nessa cadeia.
Após o deleting destructor, `FUN_0055F7F9` chama `FUN_004B428E`, que desmonta
uma DLL/callback de processo, buffers associados, critical section e filtro de
exceção. Logo, deleting destructor, logging finalizer e teardown da camada de
DLL/exceção são posteriores e distintos do slot de shutdown ainda sem caller.

`CONFIRMED` como exclusões de candidatos: em `FUN_0055FBB9:0x0055FC85`, o
slot `+0x08` recebe `local_8`, obtido do slot `+0x18` de `param_1`, sem fluxo
de dados até `DAT_013B71E0`. Os hits de `FUN_0055D345` em `0x0055D415` e
`0x0055D4DF` usam respectivamente `app+0xF4` e `app+0xF8`, não a própria
aplicação. Em `FUN_0055EE59:0x0055EFF7`, o receptor é `DAT_0092E654`, o timer
de vtable `0x005A4688`. Portanto, nenhum desses callsites fecha o caller de
`FUN_0055D066`; igualdade do offset virtual não identifica a classe do
receptor.

`CONFIRMED`: no ramo `WM_CLOSE` de `FUN_0055DAB8`, o callsite
`0x0055EB9C` carrega o vptr diretamente de `param_1` e usa o próprio
`param_1` como receptor:

```asm
0055EB8E MOV EDX,[EBP-0x178]
0055EB94 MOV EAX,[EDX]
0055EB96 MOV ECX,[EBP-0x178]
0055EB9C CALL [EAX+0x08]
```

`CONFIRMED`: o shutdown não é executado imediatamente ao entrar nesse ramo.
`0x0055EB5A` obtém o objeto em `app+0xF4` e `0x0055EB74` chama seu slot
virtual `+0x08`. Em seguida, `0x0055EB77` lê `DAT_013B7220`, soma `0xBB8`
(3000 unidades) em `0x0055EB7D` e compara esse limite com o retorno do objeto
em `0x0055EB83`; o fluxo só alcança `0x0055EB9C` quando o limite foi atingido.
A unidade e a semântica do provedor em `app+0xF4` ainda não estão resolvidas,
portanto essas 3000 unidades não são classificadas como milissegundos.

Depois de `FUN_0055D066`, `0x0055EB9F` testa `DAT_013B71F0`; quando positivo,
`0x0055EBAF` chama `FUN_0058F75F`, e `0x0055EBB7` zera o global em ambos os
caminhos convergentes. O ramo então grava `DAT_013B7228 = 1` em `0x0055EBC1`,
chama `FUN_00423C61` em `0x0055EBCB`, `DestroyWindow` em `0x0055EBD4`,
`PostQuitMessage(0)` em `0x0055EBDC` e finalmente zera `DAT_013B7220` em
`0x0055EBE2`.

`FUN_0055FA89` fornece `DAT_013B71E0` como esse `param_1`; a aplicação usa a
vtable `0x005A6104`, cujo slot `+0x08` é `FUN_0055D066`. Portanto, o caller
efetivo está fechado como `WM_CLOSE -> 0x0055EB9C -> app vslot +0x08 ->
FUN_0055D066`. Após o retorno do shutdown, o mesmo ramo chama `FUN_00423C61`
em `0x0055EBCB`, `DestroyWindow` em `0x0055EBD4`, `PostQuitMessage(0)` em
`0x0055EBDC` e zera `DAT_013B7220` em `0x0055EBE2`. Isso explica por que
`FUN_0055F7F9` não contém uma chamada direta ao slot: o shutdown ocorre dentro
do window procedure, antes de o loop retornar ao wrapper e executar o deleting
destructor.

`CONFIRMED`: `FUN_00423C61` é o finalizador do hook global de teclado e possui
um único caller, `FUN_0055DAB8:0x0055EBCB`. Se `DAT_005CCF84 == 0`, retorna
sem efeitos. Caso contrário, chama `FUN_00423B25`; quando esse helper retorna
zero, passa `DAT_005CCF80` a `UnhookWindowsHookEx`. Independentemente da
plataforma, termina gravando `DAT_005CCF84 = 0`. Assim, a cadeia posterior ao
shutdown fica fechada até a destruição da janela:

```text
WM_CLOSE -> FUN_0055D066 -> FUN_00423C61
-> UnhookWindowsHookEx, quando aplicável -> DestroyWindow
-> PostQuitMessage(0) -> DAT_013B7220 = 0
```

`CONFIRMED`: o outro extremo do lifecycle está em
`FUN_0055F7F9:0x0055FA08 -> FUN_00423C1F`, após bootstrap bem-sucedido e antes
do loop principal. O instalador é idempotente quando `DAT_005CCF84 == 1`.
Nos demais casos, consulta `FUN_00423B25` e, se o retorno for zero, chama
`SetWindowsHookExA(WH_KEYBOARD_LL=0x0D, FUN_00423B74, HINSTANCE, 0)`, grava o
handle em `DAT_005CCF80` e depois marca `DAT_005CCF84 = 1`. O retorno da API
não é validado: um handle nulo ainda deixa o flag lógico ativo. Esse é o
comportamento nativo de falha parcial, não uma recomendação de implementação.

`FUN_00423B25` usa `GetVersionExA` e retorna se `dwPlatformId == 1`; nessa
plataforma, instalador e finalizador pulam respectivamente `SetWindowsHookExA`
e `UnhookWindowsHookEx`, mas mantêm a transição do flag. O callback
`FUN_00423B74` é comprovadamente indireto pelo ponteiro imediato passado em
`0x00423C46`. Para hook code não negativo, ele bloqueia `Ctrl+Esc`, `Alt+Esc`,
Windows esquerda (`0x5B`), Windows direita (`0x5C`) e Apps (`0x5D`); nos
demais casos chama `CallNextHookEx(DAT_005CCF80, ...)`. Os xrefs globais são
fechados: somente `FUN_00423C1F` escreve `DAT_005CCF80`, lido pelo callback e
pelo finalizador, e somente instalador/finalizador leem ou escrevem
`DAT_005CCF84`.

Essa sequência prova que o `ObjectManager` global é destruído antes dos demais
campos listados, fecha o caller do slot de shutdown e o teardown do hook até
`PostQuitMessage(0)`, mas ainda não identifica as classes/ownership de todos os
campos nem a unidade do gate. Socket, timer, foco, fontes de `WM_CLOSE`,
logout/relogin e a convergência integral com `FUN_0055D066` permanecem abertos.

### Logout e relogin

`UNRESOLVED`: não foi fechada uma transição observável de logout/relogin que
prove a limpeza e a reconstrução da árvore, do estado global da cena, dos
controles e dos recursos de transporte. Não portar comportamento moderno para
preencher essa lacuna.

## Wire, ABI e recursos

O fluxo localiza a intenção `0x213` com tamanho `0x24`, opcode em `+0x04` e
índice em `+0x0C`, além da resposta `0x114`. Isso ainda não é uma ficha
`CONTRACT`: direção completa, campos restantes, packing, signedness wire,
validação server-side e ordenação dos consumidores da resposta não foram
fechados. Os endereços, vptrs, slots e offsets de objeto registrados são
layout/ABI interno do executável nativo 7.48 e não devem ser copiados para
structs da source ou para o servidor. Nenhum ID de UI, loader ou recurso novo
foi inferido a partir do TMProject.

## Mapeamento atual

### Source recompilável

O equivalente semântico atual está em:

- `client-source/tmproject/Projects/TMProject/ObjectManager.cpp:701-764`:
  `SetCurrentState` converte `TM_FIELD2_STATE` para `TM_NONE_STATE`, cria
  `TMFieldScene`, `TMSelectCharScene`, `TMSelectServerScene` ou `TMDemoScene`,
  inicializa, anexa ao root e reporta falhas.
- `client-source/tmproject/Projects/TMProject/ObjectManager.cpp:766-783`:
  `SetCurrentScene` troca a cena global, marca a anterior, copia o estado do
  painel de mensagens e chama `DeleteObject`.
- `client-source/tmproject/Projects/TMProject/ObjectManager.cpp:924-963`:
  `CleanUp` percorre a árvore, consome `m_cDeleted` e libera
  `m_pPreviousScene`.
- `TMScene.cpp` e `TreeNode.cpp`: constroem containers/filhos e mantêm a lista
  intrusiva de ownership. Esses arquivos são a implementação atual, não prova
  do layout nativo 7.48.

### WYD-Go

Não existe equivalente server-side direto para a troca de cena local. O Go é
autoritativo apenas para as intenções de gameplay que surgem depois da entrada
no mundo; conexão, login, sessão, permissões, estado de personagem,
inventário, observers e persistência continuam sendo validados no servidor.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject 7.69+ | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| Conversão de estado | `9 -> 0` | `TM_FIELD2_STATE -> TM_NONE_STATE` | semântica homônima sugerida | não aplicável | confirmar enum/entrada 7.48 antes de portar |
| Estados que criam cena | `0`, `5`, `7`, `8` | quatro classes de cena | referência semântica posterior | não aplicável | manter no estudo; não alterar enquanto `LOCATED` |
| Inicialização | virtual `+0x4C`; cena `0` retorna `1` em toda conclusão normal; cenas `5/7/8` retornam `0` após falha de `FUN_00541065` e `1` nos demais caminhos normais | `InitializeScene()` | pode ter outra ABI | não aplicável | initializers fechados no CFG normal; fechar teardown antes de adaptar |
| Ownership | root `manager+0x1B07C`; `FUN_0054AC09` insere no início da lista intrusiva | `m_pRoot->AddChild` | árvore moderna não decide offset | não aplicável | ordem local comprovada; não portar layout |
| Troca/limpeza | marca anterior, sentinela de subárvore, quatro cadeias específicas, zero preventivo e detach | `m_cDeleted`, `DeleteObject`, `CleanUp` | lifecycle posterior é pista | não aplicável | ordem local fechada; concluir shutdown/relogin antes de editar |
| Seleção de personagem | `SButton` ID `0x1204`; release `0x202`; receptor embutido; cena `+0x58`; packet `0x213`, `0x24` bytes, índice `+0x0C` | sem comparação autorizada nesta etapa | nomes modernos são apenas pista | contrato server-side ainda não correlacionado | origem UI fechada; fechar ordem do `0x114` e ficha wire antes de adaptar |
| Transporte | enqueue cifra `+0x04..fim`; flush preserva pendências parciais | sem comparação autorizada nesta etapa | implementação posterior não decide ABI | transporte server-side não analisado nesta ficha | registrar como comportamento nativo, sem portar layout |
| Resposta de seleção | `0x114`; dois consumidores solicitam cena 0 | sem comparação autorizada nesta etapa | semântica posterior não decide ordem | handler server-side não correlacionado | fechar ordem global/cena antes de promover |
| Falha | mensagem, destruição parcial e `PostMessageA` | `MessageBox`/`PostMessage` equivalentes | não decide erro 7.48 | não aplicável | manter como claim nativa; testar somente após contrato |

## Decisões

- Manter esta ficha em `LOCATED`. A criação, origem UI da seleção, packet
  `0x213`, resposta `0x114`, consumidor da marca e destrutores foram
  localizados; os initializers das cenas `0/5/7/8` e os consumidores mutuamente
  exclusivos da resposta estão fechados. A ordem local de coleta, deleting
  destructor, prevenção de dupla destruição, detach e caller do shutdown
  também está fechada, mas callers de estado restantes, ownership, convergência
  integral do teardown e logout/relogin ainda não estão.
- Não alterar `ObjectManager`, `TMScene`, `TreeNode` ou `NewApp` por causa da
  semelhança observada. A implementação do TMProject 7.69+ é referência
  semântica secundária, não contrato 7.48.
- Não importar endereço, offset, vtable, enum, recurso ou ABI do TMProject para
  o client 7.48. O delta `9 -> 0` só poderá ser adaptado depois que o teardown
  integral e as demais lacunas obrigatórias estiverem comprovados.
- Não declarar `TRACED`, `CONTRACT`, `IMPLEMENTED` ou `CLIENT_TESTED` por causa
  de export, build ou correspondência estrutural.

## Lacunas

- callers restantes e transições que fornecem outros estados a `FUN_004B3500`;
- callers, receptores e vptrs dos traversals `FUN_004B29B9` e `FUN_004B2D35`,
  candidatos aos slots estruturais `+0x14/+0x20` próximos do manager;
- atribuição, consulta, invalidação e destruição de `app+0xF4`, `app+0xF8` e
  `app+0xFC` no loop de mensagens;
- fontes normais, de bootstrap e de falha que conduzem ao `WM_CLOSE`;
- classe e ownership de `app+0xE0/+0xE4/+0xE8/+0xEC/+0xF0/+0xF4/+0xFC/+0x110`
  e dos globals desmontados;
- convergência do shutdown observado com janela, socket, timer, cenas,
  controles e foco;
- logout/relogin e reconstrução completa da sessão/cena;
- comparação campo a campo com o recurso e a source 7.48, após o fluxo nativo
  estar fechado.

## Validação

- Pesquisa: SHA-256 da referência reconferido em 2026-08-30; funções, vtables,
  instruções e helpers foram exportados em modo headless/read-only pelo Ghidra.
- Automação: `validate_research.py --repo .` foi reexecutado em 2026-08-30,
  após mover o ledger para `inventory/`, e passou com três fichas válidas e
  `LOCATED=3`; `triage_catalog.py --repo . --format summary` passou com
  `functions=4146`, `UNMAPPED=4084`, `LOCATED=23` e
  `STATICALLY_EVIDENCED=39`. O corpus permanece com 4.146 funções; a triagem
  não é contagem de funções compreendidas.
- Cobertura documental: as 54 linhas do ledger correspondem aos 54 TSVs ainda
  presentes no diretório regenerável, sem ausências de nenhum lado: 29
  conclusões confirmadas, 17 pistas localizadas, 1 export ainda não
  interpretado e 7 lacunas seguintes; volume aproximado 34,87 MiB.
- Recortes mais recentes interpretados: `fun-00423c61-shutdown-focused.tsv`,
  304.785 bytes, SHA-256
  `5799FB5705F68DBB188B53D2989E635DBD1C146AA559161696F844B438676991`,
  com 18 instruções, 10 referências e único caller em `0x0055EBCB`; e
  `windows-hook-lifecycle-focused.tsv`, 37.306 bytes, SHA-256
  `ACC5288A10463BABD0FA2DD83058F78BB2B65130A52D1AA90169B0397EA97D43`,
  que fecha instalador, callback, globals e finalizador. Ambos contêm o hash
  nativo esperado, foram gerados com `-noanalysis` e não têm `SCRIPT ERROR`.
- Novos recortes: `application-frame-0055d6e6-focused.tsv`, 259.951 bytes,
  SHA-256 `DD7762EA1384BC1A21E1DF4B58F2F94DF486B912A0C212E9B587B3349A61ED09`,
  fecha o caller e o papel de frame/render de `FUN_0055D6E6`; e
  `virtual-slots-14-20-all.tsv`, 30.094 bytes, SHA-256
  `BC4CF4ECDBACD7412DE9F42045B58DCDD2185E919C4E0277E7FC0B2AD4E1FDD6`,
  registra 34/53 hits estruturais. Ambos embutem o hash nativo esperado e não
  têm `SCRIPT ERROR`.
- O Ghidra foi usado sem gravação no projeto. O recorte mais recente usou
  `-noanalysis`; uma execução focada anterior iniciou autoanálise temporária
  porque essa opção não foi informada, mas suas mudanças foram descartadas ao
  final e não alteram o corpus versionado.
- Integridade: `git diff --check` passou nesta documentação; os avisos exibidos
  referem-se somente à conversão normal de LF/CRLF pelo Git no Windows.
- Source: inspeção estática de `ObjectManager.cpp`, `TMScene.cpp` e
  `TreeNode.cpp`; nenhum arquivo ativo foi alterado nesta etapa.
- Client real: não executado; não há build, startup, screenshot, dump ou fluxo
  real em `client748/project.exe`. Portanto, nenhuma alegação `CLIENT_TESTED` é
  permitida.
