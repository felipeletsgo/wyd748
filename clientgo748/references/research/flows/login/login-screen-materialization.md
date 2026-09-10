---
id: login-screen-materialization
title: Materialização, input e teardown da tela de login 7.48
subsystem: login
status: LOCATED
mode: PARIDADE_NATIVA
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-09
---

# Materialização, input e teardown da tela de login 7.48

## Pergunta

Quais controles, recursos, estados e callbacks o client nativo materializa para
login, e quais invariantes precisam ser preservados na primeira tela visual do
client Go?

## Fronteira de evidência

Esta ficha cobre somente a transição observável da tela de login: criação,
entrada de credenciais, envio, falha, fechamento e reentrada. O hash do binário
analisado é o registrado no pacote Ghidra. Endereços, layouts e IDs ainda não
confirmados não são usados como contrato da implementação Go.

## Fluxo nativo 7.48

### Janela e superfície nativa

`FUN_0055BC0A` é a rotina de inicialização da janela principal e da superfície
de login. O binário fixa a resolução lógica em `800 x 600`, registra a classe
`With Your Destiny`, cria a janela principal e instala o procedimento nativo
`FUN_0055FA89`. Em seguida cria dois filhos Win32 que pertencem à superfície de
login: um controle `STATIC` de `800 x 0x230` (posicionado centralizado) e um
`BUTTON` com `HMENU/ID = 0x3E7`, também inicialmente oculto. O foco volta para a
janela principal depois da criação. Esses controles são evidência do bootstrap
da superfície, não prova de que os campos de credenciais sejam controles Win32
diretos.

A mesma rotina carrega `config.txt`, resolve a resolução escolhida e chama o
carregamento de dados antes de retornar sucesso. Falhas de dados mostram
`Initialize Data Failed` no parent window e abortam a inicialização. Isso fixa
uma ordem importante para o client Go: resolução/contexto e dados devem estar
prontos antes de materializar a cena, e a falha deve liberar a superfície
parcial.

`FUN_00494FA4` é o factory de recursos de cena chamado pelo caminho de carga de
UI. A descompilação enumera tipos `PANEL`, `BUTTON`, `TEXT` e `EDIT`, lê os
registros do recurso e cria objetos com parent, retângulo e textura. Portanto,
os campos de login devem ser rastreados no recurso consumido por essa factory;
os nomes/IDs ainda não foram encontrados e não podem ser inventados na source
Go.

### Entrada e envio

`FUN_004AC985` é uma raiz de interação que recebe um evento e chega ao caminho
de autenticação. O fluxo bloqueia a área de interação durante a operação e
encaminha a materialização do pedido para `FUN_00484D44`. A função de construção
produz o packet `0x20D` com 116 bytes; a ficha `login-session.md` registra os
campos e offsets do wire.

`FUN_00484D44` valida a fase/conexão antes de enviar, limpa a estrutura local,
preenche a versão `748` e os campos de credenciais e chama o envio do socket.
Falha na validação evita o envio e restaura o estado de espera.

### Falha e bootstrap

`FUN_0055F7F9` contém o bootstrap principal e materializa a mensagem nativa
`Login Failed` em pelo menos dois caminhos de falha de inicialização/credencial.
Isso confirma a semântica da mensagem, mas não identifica o painel visual nem
o controle que a recebe.

`FUN_0055890A` valida opcode e tamanho antes do dispatcher. Para login, o
contrato comprovado inclui `0x20D -> 0x74` bytes e os packets de resposta
documentados em `login-session.md`.

### Callers e callees conhecidos

- Caller observável: dispatcher/rotina de interação da cena de login que chega a
  `FUN_004AC985`; a cadeia completa de eventos até a criação dos controles
  ainda precisa ser aberta no projeto Ghidra.
- Bootstrap da superfície: `FUN_0055BC0A -> FUN_0055FA89`, com janela lógica
  `800 x 600`, filho `STATIC` e botão Win32 `ID 0x3E7` inicialmente ocultos.
- Factory de cena: `FUN_00494FA4`, que materializa registros `PANEL`,
  `BUTTON`, `TEXT` e `EDIT` a partir do recurso carregado.
- Callees confirmados: `FUN_00484D44` para construção/envio de `0x20D`,
  `FUN_0055890A` no caminho de validação de packets e rotinas de mensagem no
  bootstrap `FUN_0055F7F9`.
- Callbacks de criação, foco, teclado, mouse e destruição dos controles: ainda
  não resolvidos; não promover esta ficha para `TRACED` antes de localizá-los.

## Estado e lifecycle

O estado mínimo comprovado é:

```text
idle/login visible
-> credentials accepted
-> input/send in progress (interaction restricted)
-> success: response dispatcher advances session
-> failure: show Login Failed and return to a retryable state
-> disconnect/close: release scene resources and credentials
-> reentry: create a fresh scene and do not reuse password memory
```

O ownership dos widgets nativos, a prioridade modal, o foco inicial, os
atalhos de teclado, o comportamento de `Esc`, o callback do botão Login e a
ordem exata de destruição ainda são lacunas de pesquisa. A implementação Go
deve manter esses pontos encapsulados até que a evidência seja fechada.

## Wire, ABI e recursos

- Wire: `0x20D`, 116 bytes (`0x74`), direção client -> server; offsets e
  validações estão em `login-session.md` e nos testes byte a byte de
  `internal/login`.
- ABI nativa: não portar structs, vtables ou offsets para Go.
- Recursos visuais: a superfície Win32 e a factory de tipos foram confirmadas,
  mas o recurso efetivo da cena de login, seus nomes/IDs, texturas e callbacks
  ainda não foram reconciliados. Este continua sendo o gate para a
  implementação visual de paridade.
- Texto: as mensagens do novo client serão em inglês; a mensagem nativa
  observada é `Login Failed`.

## Mapeamento atual

- Native/Ghidra: `references/ghidra/corpus/0055bc0a_FUN_0055bc0a.c`,
  `0055fa89_FUN_0055fa89.c` (procedimento, quando disponível),
  `00494fa4_FUN_00494fa4.c`, `004ac985_FUN_004ac985.c`,
  `00484d44_FUN_00484d44.c`, `0055f7f9_FUN_0055f7f9.c` e
  `0055890a_FUN_0055890a.c`.
- Source Go: `internal/login/controller.go`, `dispatcher.go`, `state.go`,
  `internal/loginflow/coordinator.go` e `internal/app/application.go`.
- Contrato de sessão: `references/research/flows/login/login-session.md`.
- Renderer atual: `internal/graphics/`; ainda limitado à textura fullscreen.

## Matriz de fontes

| Fonte | Estado | Uso nesta ficha |
| --- | --- | --- |
| Binário nativo 7.48 | UTILIZADA | identidade e comportamento observável de bootstrap/envio/falha |
| Projeto/descompilação Ghidra | UTILIZADA | callers/callees parciais e construção de `0x20D` |
| Assets oficiais 7.48 | UTILIZADA | disponíveis no pacote; IDs/recursos da tela ainda não reconciliados |
| Source Go e testes | UTILIZADA | controller, dispatcher, estado e lifecycle ativos |
| WYD-Go | UTILIZADA | autoridade do servidor e contrato de sessão |
| TMProject | UTILIZADA secundariamente | comparação estrutural, sem copiar código ou IDs |
| Guias | NÃO APLICÁVEL | não fornecem evidência suficiente para esta fronteira |
| W2PP, Secrets, Micronics | EXCLUÍDA | fontes bugadas e proibidas pela campanha |

## Matriz de delta

| Claim | Nativo 7.48 | Source Go | Decisão |
| --- | --- | --- | --- |
| envio de credenciais | `FUN_004AC985 -> FUN_00484D44`, `0x20D/0x74` | controller tipado e testado | manter contrato; modernizar ownership |
| validação de entrada | `FUN_0055890A` e validações de fase | parser/dispatcher Go | manter e testar rejeição |
| mensagem de falha | `Login Failed` no bootstrap | estado de erro da cena | portar sem expor segredos |
| controles visuais | IDs/callbacks ainda não resolvidos | inexistentes visualmente | bloquear paridade visual até rastrear |
| superfície/bootstrap | `FUN_0055BC0A`, resolução lógica 800x600, `STATIC` e botão `0x3E7` | janela/contexto já existentes; cena ainda sem controles | reproduzir ordem e ownership, sem portar ABI |

## Decisões

- Classificação: `PARIDADE_NATIVA` para comportamento e ciclo da tela.
- Manter o contrato de packets já comprovado; não criar opcode novo.
- Implementar o renderer visual somente depois de identificar recurso, IDs,
  foco e callbacks no Ghidra/asset 7.48.
- A senha será mantida em memória somente durante a tentativa e limpa em
  sucesso, rejeição, desconexão e fechamento.

## Lacunas

### Evidência nova: framing do asset local (2026-09-10)

Teste reproduzível: `python -m unittest discover -s tools/assets -p
test_login_rc_evidence.py -v`, executado com sucesso a partir deste pacote.
`assets/current/UI/LoginScene2.bin` fecha em 1140 bytes com oito registros:
text 769 em 0, panel 4608 em 180, edits 5121/5122 em 220/408,
buttons 4609/4610/4611 em 596/764/932 e panel 305 em 1100.
Os payloads observados são panel=36, text=176, edit=184 e button=164 bytes.
O DWORD em 220 é o tipo EDIT (13), não `nPickable` nem padding.
A hipótese anterior de padding adicional está **REJECTED**.

Procedência: asset local UTILIZADA; TMProject copiado CONTRADITÓRIA para este
arquivo (`WalkRCRecords` exige panel=40 mesmo no modo inline); export Ghidra
UTILIZADA e CONTRADITÓRIA como prova direta desse formato: `FUN_00494FA4`
serializa panel=0x28, button=0x28 e text=0x34, com captions indexadas.
Essa função é um caminho de recurso textual/serialização; não deve ser chamada
de prova do leitor binário inline. Go/testes UTILIZADA somente como auditoria
offline. Servidor e guias NÃO APLICÁVEL ao framing local; identidade do binário
permanece a registrada acima, sem nova execução ou claim de validação runtime.

Decisão: manter a ficha LOCATED e não conectar um parser presumido ao renderer.
Investigação subsequente: `FUN_004974ec` é o leitor binário, com leituras
panel/button=0x28, text=0x34 e edit=0xb8. `FUN_004A8F14` seleciona
`UI/SelServerScene2.txt` no branch moderno e resolve nele também os IDs de
login 0x1200..0x1203. O TMProject atual confirma `LoadRC` desse recurso.
O asset `SelServerScene2.bin` fecha em 1352 bytes/22 controles com esse formato;
contém painel 4608, edits 5121/5122, botões 4609/4611/4610 e textos
5632/5633/5634. Logo, LoginScene2 não é o recurso ativo desse branch.

`internal/assets/scene.go` implementa a decodificação isolada indexada em Go,
sem copiar objetos/ownership C++. Testes focados passaram: 22 registros,
IDs de login, truncamento, cópia independente e rejeição do arquivo inline.
Modo PARIDADE_NATIVA restrito ao framing observado, sem claim de lifecycle
ou paridade visual. Ainda não conectado ao renderer. Próximo passo: resolver
strings/texturas e geometria final aplicada em runtime em SelServerScene2,
então substituir os controles hardcoded da cena Go.
Este teste não comprova captions, geometria final, callbacks ou paridade visual.

1. localizar todos os callers/callees que ligam `FUN_0055BC0A`,
   `FUN_00494FA4` e a cena de login;
2. confirmar painel, campos, botão e IDs no recurso efetivamente carregado;
3. rastrear foco, teclado, mouse, `Esc`, envio duplicado e callback de Login;
4. rastrear mensagem de erro, fechamento, destruição e reentrada;
5. comparar a materialização com screenshot/execução nativa antes de promover
   a ficha para `TRACED`/`CONTRACT`.

## Validação

- Evidência estática: `LOCATED`; callers e recursos ainda incompletos.
- Source Go: controller, dispatcher, estado e coordinator cobertos por testes
  automatizados na unidade anterior.
- Build/cliente real: a tela visual e o login do client Go ainda não foram
  executados contra o servidor real; `CLIENT_TESTED` permanece pendente.
