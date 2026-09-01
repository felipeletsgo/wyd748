---
id: indexed-parameterized-message-extension
title: Aviso indexado e parametrizado da source
subsystem: ui
status: UNMAPPED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Aviso indexado e parametrizado da source

## Pergunta

Como preservar os avisos indexados `0x105/0x106` importados no TMProject sem
atribuir ao client nativo um fluxo inexistente, sem expor o painel a índice,
cópia ou format string não confiáveis e com um contrato explícito no WYD-Go?

## Fronteira de evidência

- Executável/hash: baseline histórica identificada pelo SHA-256 do frontmatter.
- Projeto/corpus Ghidra: `FUN_0049889A` e o dispatcher da Field foram
  consultados no projeto canônico; `FUN_0055890A` foi reutilizada somente para
  delimitar o que o gate nativo comprova.
- Integração legada: `0x101` continua sendo o aviso textual nativo;
  `0x333 -> FUN_00481DD6` e `0x334 -> FUN_00481FF3` continuam sendo chat e
  whisper, sem alias com esta extensão.
- Source atual: `Basedef.h`, `TMScene.cpp`, `WYD748Assets.cpp` e a tabela
  `g_pMessageStringTable` carregada de `UI/strdef.bin`.
- Servidor: builders em `internal/wire`, emissores funcionais em
  `internal/game/party.go` e `internal/game/ghost_shop.go`, e testes byte a
  byte/de integração.

## Fluxo nativo 7.48

### Entrada observável

Não existe entrada nativa `0x105/0x106` confirmada. Para `Header.ID == 0`,
`FUN_0049889A` trata `0x101`, `0x102` e `0x104`, mas não os dois opcodes da
source. O dispatcher da Field também não possui esses cases. A ausência é a
fronteira de integração confirmada, não uma implementação nativa a reconstruir.

### Callers

Não aplicável a uma transição nativa inexistente. O receptor vivo da extensão
é `TMScene::OnPacketEvent` na source recompilável.

### Função principal

`TMScene::OnPacketEvent` aceita o contrato novo somente com `Header.ID == 0`,
tamanho exato de 108 bytes e seletor `String[0] == 0`. Ele lê o índice assinado
com `memcpy`, resolve a linha e entrega uma cópia limitada ao painel da cena.

### Callees

- loader `WYD748_LoadMessageStrings`: materializa as linhas de
  `UI/strdef.bin`, valida tamanho/checksum e zera o restante da tabela;
- `SMessagePanel::SetMessage` e `SetVisible`: recebem somente o texto final já
  limitado; o packet e a tabela não são retidos;
- helpers locais de parse e substituição: não alocam estado persistente e não
  chamam `sprintf` com o asset como formato.

### Saídas e erros

- tamanho diferente de 108, ID diferente de zero ou seletor diferente de zero
  não exibe mensagem;
- índice efetivo fora de `0..1999`, linha ausente ou linha vazia exibe o índice
  relativo em decimal;
- painel ainda não materializado faz o packet terminar sem side effect;
- texto e resultado são truncados a 127 bytes mais NUL;
- parâmetro excedente ao sexto e bytes depois do limite do frame são ignorados.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| receber `0x105` | cena e painel ativos; frame válido | `TMScene::OnPacketEvent` | aviso visível | copia linha/fallback | frame/selector inválido: descarta |
| receber `0x106` | anterior + CSV opcional | parse limitado + substituição | aviso formatado visível | consome até seis parâmetros | diretiva não suportada fica literal |
| índice sem linha | tabela fora da faixa ou vazia | fallback decimal | aviso visível | nenhum acesso à tabela inválida | mostra índice relativo |
| convite de grupo | convite autoritativo aceito pelo servidor | `MessageParameterized(-938, nome)` | aviso localizado acompanha `PartyRequest` | parâmetro é o nome do líder | nenhuma mutação adicional |
| gold insuficiente | compra em loja fantasma rejeitada antes de persistir | `MessageIndexed(-845)` | aviso localizado | substitui apenas o texto genérico dessa rejeição | compra continua sem mutação |
| troca de cena | aviso ativo | teardown normal da cena | painel eliminado | nenhuma referência ao frame permanece | N/A |

### Vtables, vptrs e receptores

A extensão usa o receptor vivo `TMScene` e o `SMessagePanel` já pertencente à
árvore da cena. Não acrescenta vtable, callback, controle, ID de recurso nem
receiver por entidade. Os stubs de `TMHuman` e `TMFieldScene` para
`0x105/0x106` ficam fora do contrato até existir emissor com `Header.ID != 0`
e comportamento comprovado.

### Ownership

O transporte empresta o frame apenas durante `OnPacketEvent`. O parser copia o
template, os parâmetros e o texto final para buffers locais; `SetMessage`
recebe o resultado durante a chamada. A tabela global pertence ao loader e é
somente leitura durante o dispatch.

### Falha parcial

O aviso não altera gameplay, persistência nem estado autoritativo. Falha de
validação deixa a cena inalterada; não há ACK, mutação ou rollback a publicar.

### Cleanup e teardown

Não há buffer heap, timer próprio, inscrição ou callback novo. O painel e sua
cópia de apresentação seguem o cleanup normal da cena.

### Shutdown

O shutdown destrói a árvore de controles existente. Nenhum estado desta
extensão sobrevive ao receptor ou exige ordem adicional de destruição.

### Logout e relogin

Logout elimina o painel da Field atual; relogin materializa outro painel e
recarrega o lifecycle normal. Índices, parâmetros e texto não são armazenados
entre cenas, então não existe estado stale específico para limpar.

## Wire, ABI e recursos

Contrato `EXTENSAO_COORDENADA`, S para C, opcodes `0x105` e `0x106`:

| Offset absoluto | Corpo | Tamanho | Campo |
| --- | --- | ---: | --- |
| `0x00` | - | 12 | `MSG_STANDARD`, `ID=0`, `Size=108` após envio |
| `0x0C` | `String[0]` | 1 | seletor zero para aviso da cena |
| `0x0D` | `String[1]` | 1 | reservado zero |
| `0x0E` | `String[2..3]` | 2 | índice relativo `int16`, little-endian |
| `0x10` | `String[4..94]` | até 91 | CSV de `0x106`, máximo seis parâmetros |
| `0x6B` | `String[95]` | 1 | NUL final obrigatório |
| total | - | 108 | envelope de 12 + 96 bytes |

O índice efetivo é `relativeIndex + 1000` e precisa estar em `0..1999`. O
asset ativo possui 440 linhas materializadas; as demais são zeradas pelo
loader. Isso permite índices relativos negativos para o corpus existente e
produz fallback explícito quando o servidor referencia uma linha ausente.

No `0x106`, somente `%s` consome parâmetro e `%%` produz `%`. Qualquer outra
sequência `%x` permanece literal, e `%s` sem parâmetro vira vazio. O servidor
normaliza vírgula e NUL internos para espaço, limita o CSV a 91 bytes e ignora
parâmetros depois do sexto. Nenhum template do asset chega a uma função da
família `printf`.

Não foi criado capability bit porque este ecossistema possui um único client
ativo compilado da source. Os usos funcionais ficam restritos às linhas
materializadas: `-938` parametriza o convite com o nome do líder e `-845`
apresenta gold insuficiente na loja fantasma. `MessagePanel` continua sendo o
fallback para erros da loja sem índice coordenado.

## Mapeamento atual

### Source recompilável

Os opcodes receberam nomes explícitos de extensão. O parser passou a validar
tamanho, ID, seletor, índice e painel; lê o `int16` sem cast desalinhado, não
muta a tabela global, copia somente os 96 bytes realmente presentes no packet
e substitui apenas `%s`/`%%` em buffers limitados. As durações especiais dos
índices efetivos `465`, `466`, `484` e `485` continuam em 600 segundos; as
demais permanecem em quatro segundos.

### WYD-Go

`MessageIndexed` e `MessageParameterized` constroem frames de 108 bytes com
`ID=0`, índice assinado em `0x0E` e NUL final preservado. O builder
parametrizado limita e sanitiza o CSV. O convite de grupo envia
`MessageParameterized(-938, inviter.Char.Name)` junto do packet estrutural de
convite. A rejeição específica de gold insuficiente na loja fantasma envia
`MessageIndexed(-845)` sem alterar a validação, persistência ou rollback da
compra; os demais erros continuam no painel textual legado.

## Matriz de delta

| Claim | Nativo 7.48 | Source antes | Source/WYD-Go atual | Decisão |
| --- | --- | --- | --- | --- |
| opcodes `0x105/0x106` | ausentes dos dispatchers confirmados | cases importados | contrato explícito nos dois lados | `EXTENSAO_COORDENADA` |
| tamanho | não provado pelo gate nativo | usava `MSG_MessageChat` sem validar | 108 bytes exatos | adotar envelope comprovado da source |
| índice | N/A | cast desalinhado e sem bounds | `int16` LE + faixa | validar antes de acessar |
| tabela | N/A | escrevia NUL na global | cópia somente leitura de até 127 bytes | preservar ownership do loader |
| parâmetros | N/A | `strcpy`/`sscanf`/`sprintf` | seis campos, `%s`/`%%`, saída limitada | modernizar internamente |
| receptor por entidade | inexistente | stubs vazios | fora do contrato | não preencher por hipótese |
| emissores funcionais | inexistentes | builders dormentes | convite `-938`; gold insuficiente `-845` | extensão coordenada restrita a linhas existentes |

## Decisões

- Não apresentar `0x105/0x106` como paridade nativa nem promover a maturidade
  nativa; o frontmatter permanece `UNMAPPED` porque não existe transição nativa
  à qual atribuir callers/callees.
- Preservar a estrutura posterior como extensão coordenada porque o client e o
  servidor são controlados juntos e o envelope não colide com os dispatchers
  nativos confirmados.
- Restringir o contrato a `Header.ID == 0` e ao painel da cena. Não implementar
  handlers de humano/party sem emissor e sem semântica observável.
- Ligar somente usos funcionais cuja linha e parâmetros já foram escolhidos:
  convite de grupo `-938` e gold insuficiente `-845`; manter os demais fluxos
  no contrato anterior até haver semântica coordenada.

## Lacunas

- Executar no `client748/project.exe` um convite de grupo e uma compra em loja
  fantasma sem gold, confirmando os textos localizados e o nome do líder.
- Executar também fallback por índice vazio/inválido e `0x106` com zero/seis
  parâmetros.
- Repetir após troca de cena e logout/relogin antes de qualquer claim
  `CLIENT_TESTED`.

## Validação

- Pesquisa: ausência de `0x105/0x106` confirmada nos dispatchers nativos
  focalizados; não foi atribuído equivalente inexistente.
- Automação: `go test -count=1 ./internal/wire ./internal/game` passou; os
  testes byte a byte cobrem header, tamanho, `ID=0`, índice assinado, limite de
  seis parâmetros, normalização de vírgula/NUL, truncamento e NUL final. Testes
  de gameplay cobrem os emissores `-938` e `-845` sem mutação indevida.
- Validador: `validate_research.py --repo .` passou com esta ficha mantida em
  `UNMAPPED`, por ser extensão sem transição nativa correspondente.
- Build: `Build-Client.ps1` passou em `Release|Win32` com toolset v145, zero
  erros e 31 warnings preexistentes; instalou `client748/project.exe` com
  SHA-256 `9E225456063C5DC77917C007FDCA9ECD05DDC9312FD25D9AB28FE55F334B5BF4`.
- Client real: não executado; nenhuma alegação `CLIENT_TESTED` é feita.
