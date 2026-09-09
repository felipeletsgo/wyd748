# Mapeamento do client WYD 7.48

Este diretório é o pacote autocontido de pesquisa e preparação do novo client
Go. Ele reúne o executável nativo usado como evidência, o cliente oficial e
seus assets, a descompilação exportada, o censo das funções, as fichas de
fluxo, os contratos do servidor e a documentação secundária necessária para
implementar cada unidade sem depender de caminhos externos.

## Resultado atual

O corpus nativo contém **4.146 funções**. O inventário local correlaciona:

- 4.146 entradas no censo (`references/catalog/functions.tsv`);
- 350 funções citadas pelas pesquisas existentes;
- 143 entradas `LOCATED`, 37 com evidência estática adicional e 5 com
  evidência de paridade/fluxo;
- 76 arquivos de fluxo (75 fichas + 1 template) em
  `references/research/flows/` e 47 exportações TSV;
- 3.961 entradas ainda `UNMAPPED` semanticamente.

Portanto, o pacote já possui o **mapeamento completo do corpus e das fontes**,
mas não declara que 4.146 funções foram semanticamente rastreadas. A maturidade
de cada entrada permanece no TSV e não é promovida por compilação ou por
semelhança com o TMProject.

O mapa operacional de assets é uma camada independente desse censo. O snapshot
`assets/current` contém **7.370 arquivos** e **347.172.280 bytes canônicos**, distribuídos
nas famílias ANI, BIN, BON, CSV, CUR, DAT, JSON, MSA, MSH, TRN, TXT, WYS e WYT.
`references/assets/asset-map.tsv` registra contagem e tamanho por família; as
relações ficam em `asset-dependencies.tsv`. A presença de uma extensão não
promove seu estado nem cria dependência implícita. WYT possui um caminho Go
testado para textura e TRN possui agora um parser de framing testado em
`Env/Character.trn`; somente `UI/logo1.wyt` possui relação de materialização
confirmada. Os semânticos de altura, textura e colisão do TRN permanecem
pendentes de evidência nativa.

O validador das fichas reproduz, no snapshot atual, `CLIENT_TESTED=1`,
`CONTRACT=51`, `TRACED=19`, `LOCATED=9` e `UNMAPPED=4`. Esses números são das
fichas de transição e não devem ser somados aos estados por função do censo.

## Primeira unidade executável do client Go

A primeira fatia vertical foi fechada no fluxo de render/assets:

```text
WT10 oficial -> parser TGA seguro -> RGBA8 próprio -> OpenGL/WGL -> logo -> teardown
```

O contrato está documentado em
`references/research/flows/render-assets/initial-server-logo-texture.md`.
`internal/assets.ParseWYT` cobre os três formatos observados no corpus oficial
(16, 24 e 32 bpp), normaliza a origem da imagem e rejeita assinatura inválida,
truncamento, rodadas não suportadas, dimensões inseguras e trailers inválidos.
O bootstrap Windows carrega `logo1.wyt`, faz o upload para OpenGL e destrói a
textura antes do contexto. Os testes automatizados cobrem pixels sintéticos e
o asset oficial; o smoke test real cobre janela, resize, `WM_CLOSE` e Alt+F4.
Isso não declara ainda uma validação visual manual do logo nem paridade de
outras cenas.

## Segunda unidade: formato canônico e `assetc`

`cmd/assetc` converte uma textura WYT por execução sem modificar o arquivo de
origem. `internal/assets.EncodeTexture` e `DecodeTexture` usam o formato
`WYDASSET`, versão `1`, com cabeçalho fixo, metadados da textura, payload RGBA8
compacto e SHA-256 do payload. A decodificação verifica magic, versão, tipo,
dimensões, tamanho, campos reservados e integridade antes de devolver uma
cópia própria.

Essa unidade ainda não é o pacote de distribuição: assinatura, criptografia,
compressão e manifesto de arquivos serão adicionados separadamente. O arquivo
canônico é uma representação intermediária determinística e não substitui os
assets oficiais.

O contrato está documentado em
`references/research/flows/render-assets/assetc-canonical-format.md`.

## Terceira unidade: pacote protegido de assets

`internal/assets.BuildProtectedPackage` cria um pacote determinístico de
chunks comprimidos, autenticados com AES-256-GCM e acompanhado de manifesto
canônico assinado com Ed25519. `OpenProtectedPackage` verifica o manifesto e a
assinatura antes de descriptografar, limita tamanhos, rejeita paths inseguros e
nonces repetidos, confere o hash SHA-256 do payload original e só então entrega
cópias independentes ao chamador.

Esta é uma extensão interna de distribuição (`MODERNIZACAO_COMPATIVEL`): não
altera opcode, ABI, recurso nativo ou regra server-authoritative. A chave
privada permanece fora do client; a chave pública valida autenticidade e a
chave de conteúdo deve vir do fluxo autorizado de sessão. A maturidade nativa
permanece `UNMAPPED`, pois não existe formato equivalente comprovado no WYD
7.48. A entrega está `IMPLEMENTED / CLIENT TESTED`: o `ProtectedAssetCache`
alimenta a primeira textura do renderer, fecha no teardown e foi exercitado no
executável com um pacote protegido materializado a partir do logo oficial.

O contrato está documentado em
`references/research/flows/render-assets/protected-asset-package.md`.

## Lifecycle de cenas do client Go

`internal/scene.Manager` agora controla a cena ativa, transições na fronteira
de frame, falhas parciais e teardown idempotente. `internal/input.Event` remove
a dependência de cenas em mensagens Win32, e `Application` mantém o renderer e
a janela como owners externos às cenas. Essa é uma `MODERNIZACAO_COMPATIVEL`
interna; não promove a ficha nativa ampla de cenas, que continua `LOCATED`.

A unidade está `IMPLEMENTED / AUTOMATED TESTED`, com build Windows aprovado e
bootstrap integrado executado nos ambientes default e protegido. Isso marca o
bootstrap/ciclo de fechamento como `CLIENT_TESTED`; a troca de cenas reais
continua pendente. A evidência e a matriz de fontes estão em
`references/research/flows/lifecycle/go-scene-manager.md`.

## Ordem de autoridade

1. código/testes atuais do WYD-Go e dados autoritativos;
2. binário nativo 7.48 e descompilação estudada no Ghidra;
3. assets oficiais 7.48 e comportamento observável;
4. fichas e exportações de pesquisa deste pacote;
5. TMProject e guias, apenas como comparação estrutural secundária.

W2PP, Secrets e Micronics foram explicitamente excluídos por serem fontes
bugadas. Não orientam, validam nem contradizem contratos.

## Artefatos principais

| Caminho | Conteúdo | Uso |
| --- | --- | --- |
| `CLIENT OFICIAL 7.48/` | executável e assets oficiais | baseline materializada e somente leitura |
| `DECOMP/` | exports de descompilação/disassembly | busca rápida e correlação |
| `references/catalog/functions.tsv` | 4.146 funções com estado e callers/callees | censo e fila de cobertura |
| `references/catalog/ghidra-functions.tsv` | catálogo Ghidra | xrefs, vtables e callgraph |
| `references/catalog/research-queue.tsv` | fila priorizada | próxima raiz de investigação |
| `references/research/flows/` | fichas por transição observável | evidência `TRACED`/`CONTRACT` |
| `references/research/exports/` | tabelas reproduzíveis de xrefs/fluxos | conferência sem reabrir toda a análise |
| `references/tmproject/` | snapshot de referência estrutural | comparação; não copiar código para o client Go |
| `references/server-source/` | wire/game/model relevantes | conferência do contrato server-authoritative |
| `references/server-data/` | skills, itens e NPCs autoritativos | dados consumidos pelos contratos |
| `references/skills/` | instruções e critérios de pesquisa | reprodução do método e dos gates |

## Mapa por subsistema

O mapa detalhado está em [`references/skills/subsystem-map.md`](references/skills/subsystem-map.md).
Ele separa bootstrap, transporte, login, cenas, movimento, entidades, combate,
score, inventário, NPC/trade, UI, render/assets e shutdown. Cada linha aponta
para fichas existentes ou registra explicitamente a lacuna como `UNMAPPED`.

O caminho crítico atual é:

```text
bootstrap -> transporte/login -> cenas -> dispatcher -> mundo
          -> logout/relogin -> shutdown/reconexão
```

A unidade de transporte está implementada na source Go e possui contrato nativo
`CONTRACT`: `internal/protocol` cobre handshake, framing, cifra, checksum,
fragmentação, escrita parcial, recepção assíncrona por fila limitada e
fechamento. A goroutine do socket não chama gameplay ou UI;
`internal/app.Application` drena a fila na thread principal antes do update da
cena e fecha a sessão antes das cenas e do renderer. A conexão com o servidor
real, autenticação e relogin continuam pendentes e não são promovidos a
`CLIENT_TESTED`.

O contrato nativo de login está documentado em
`references/research/flows/login/login-session.md`. `internal/login` agora
materializa os packets `0x20D`, `0x10A`, `0x213`, `0x114`, `0x215` e `0x116`,
os agregados de seleção/mundo e a máquina de estados que rejeita truncamento,
replay, resposta fora de ordem e snapshot reutilizado após logout ou
desconexão. `internal/login.Dispatcher` recebe os eventos na thread principal,
trata `0x10A`, `0x114` e `0x116`, rejeita packets client→server recebidos e
deixa opcodes de outros domínios para dispatchers futuros. A unidade está
`IMPLEMENTED / AUTOMATED TESTED`; controller de envios, cenas e fluxo contra o
servidor real continuam pendentes, portanto login ainda não é `CLIENT_TESTED`.

As fichas existentes fecham apenas transições estreitas. Movimento completo,
combate/skills, score/equipamento, inventário geral, render e vários lifecycles
continuam como frentes de pesquisa, mesmo quando há código candidato no
TMProject.

## Procedência e fingerprint

O binário analisado está autocontido em `references/ghidra/input/WYD.exe`:

```text
SHA-256 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
Tamanho 2,060,288 bytes
```

O executável stock histórico é `CLIENT OFICIAL 7.48/WYD.exe`, SHA-256
`B545EA104DE50641E820F00B6BC54E4B2B14583ED75C7DCEC06F50BA5042619C`.
O antigo `client748/project.exe` era somente o candidato de validação da source
TMProject ativa; ele não é dependência nem executável do client Go. O snapshot
de comparação necessário está em `references/tmproject/`.

## Estados e claims

- `UNMAPPED`: função presente no censo, sem fluxo nativo completo;
- `LOCATED`: entrada ou relação encontrada, mas callers/callees/lifecycle ainda
  incompletos;
- `TRACED`: transição observável, estado, erros e lifecycle rastreados;
- `CONTRACT`: além de `TRACED`, wire/ABI/recurso testável;
- `CLIENT_TESTED`: fluxo executado no executável candidato.

`IMPLEMENTED`, `STATICALLY VERIFIED`, `AUTOMATED TESTED` e `CLIENT-TESTED` são
estados de entrega separados. Nenhum build verde substitui o teste real do
client.

## Ciclo proporcional de verificação

O tooling desta unidade é uma `MODERNIZACAO_COMPATIVEL`: não altera runtime,
wire, ABI, asset ou comportamento legado. `Verify-Fast.ps1` cobre o ciclo de
código ativo; `Verify-Contract.ps1` acrescenta manifesto, autocontenção e
census determinístico de assets; `Verify-Mapping.ps1` acrescenta fingerprint,
catálogo, corpus, fichas e testes das ferramentas de pesquisa.

Na medição local deste lote, os níveis levaram aproximadamente 3,5 s, 13,9 s e
15,3 s, respectivamente. O ganho cotidiano vem de não reler 18.766 registros
do manifesto nem consultar as 4.146 funções quando apenas código Go comum foi
alterado. O nível rápido não promove `LOCATED`, `TRACED`, `CONTRACT` ou
`CLIENT_TESTED`.

Procedência desta mudança de tooling:

- source Go, testes, scripts e workflows atuais: `UTILIZADA`;
- assets 7.48: `UTILIZADA` somente para comprovar o census do gate de contrato;
- binário nativo e projeto Ghidra: `NÃO APLICÁVEL`, pois nenhuma fronteira
  legada foi alterada;
- TMProject: `NÃO APLICÁVEL`, pois não orienta o pipeline de verificação;
- W2PP, Secrets e Micronics: excluídos e não utilizados.

## Coordenador de login e cenas lógicas

`internal/loginflow` compõe o `SessionState`, `Controller`, `Dispatcher` e o
navegador de cenas sem criar um novo opcode ou alterar ABI. As fases
`Disconnected/Connecting/Authenticating`, `CharacterSelect`, `EnteringWorld` e
`InWorld/LoggingOut` mapeiam respectivamente para `Login`, `CharacterSelect`,
`Loading` e `World`. As factories atuais são cenas lógicas sem renderer e
validam a fase em `Enter` e `Update`; a implementação é
`MODERNIZACAO_COMPATIVEL` e está `AUTOMATED TESTED`.

`Application.Options.SceneSynchronizer` executa depois do dispatch de sessão e
input e antes de `scene.Manager.Update`, garantindo que vários packets do mesmo
frame produzam uma única transição baseada no estado final.

O cliente real ainda não foi executado e o lote não é `CLIENT_TESTED`.

## Como continuar

1. Escolher a raiz nativa correspondente em
   `references/catalog/research-queue.tsv` e fechar seu fluxo observável antes
   de adaptar comportamento legado.
2. Partir da função/feature viva da source, localizar a candidata nativa e
   abrir callers, callees, vtables, callbacks e teardown no Ghidra.
3. Atualizar uma ficha em `references/research/flows/` com a matriz de fontes,
   decisão (`portar`, `manter`, `modernizar`, `estender` etc.) e lacunas.
4. Só então implementar a unidade Go e validar parser, lifecycle e contrato.
5. Executar os comandos reproduzíveis de `references/REPRODUCE.md` e manter o
   manifesto atualizado após cada unidade.

Os snapshots de auditoria e a implementação vivem neste diretório. O módulo
`wydclient748` é independente do módulo Go do servidor. Os arquivos sob
`references/tmproject/` são leitura de comparação, não código ativo.

No fluxo de login, layouts, parsers, estado, dispatcher, controller,
coordenador e cenas lógicas estão implementados e testados automaticamente. O
próximo lote liga as factories às telas visuais e executa o teste real de login,
logout e relogin contra o WYD-Go.
