---
id: war-info-contract
title: Snapshot de guerra recebido ao entrar no mundo
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Snapshot de guerra recebido ao entrar no mundo

## Pergunta

Qual forma de `0x3A8/WarInfo` o client ativo deve aceitar do WYD-Go e como
preservar os tres estados consumidos por selecao de alvo, PvP e render do mapa?

## Fronteira de evidência

- UTILIZADA: `WYD.exe` nativo 7.48, hash acima, e projeto/corpus Ghidra
  `WYD748Native_20260821`; a entrada `FUN_00489300 @ 0x00489300` e seu caller
  direto constam do inventario exportado do projeto.
- UTILIZADA: source atual `TMFieldScene::OnPacketWarInfo`, dispatcher central,
  inicializacao da cena/campos e consumidores de guerra.
- UTILIZADA: `internal/wire.WarInfo`, seu emissor durante enter-world e testes do
  WYD-Go.
- NAO APLICAVEL: assets 7.48; o fluxo nao materializa controles nem recursos.
- NAO APLICAVEL: guia WYD KR scraped; nao define o envelope deste packet.
- NAO CONSULTADAS: sources 7.54, W2PP, Secrets e Micronics.

## Fluxo nativo 7.48

### Entrada observável

- Evento: frame S->C com `Header.Type=0x3A8` chega ao dispatcher de campo.
- Precondicoes e estado inicial: cena de campo e ObjectManager ativos; os campos
  conservam o ultimo snapshot ate o handler escrever os componentes presentes.
- Saida observavel: guilda de guerra, cla de guerra e guilda aliada passam a
  influenciar selecao/agressao de alvos e a variante de objetos do mapa.

### Callers

`FUN_00492E7D @ 0x00492E7D` compara `Header.Type` em `packet+4` com `0x3A8` e
chama diretamente `FUN_00489300`. O inventario Ghidra registra esse caller como
o unico xref de fluxo da entrada.

### Função principal

`FUN_00489300` seleciona a mutacao por `Header.Size`:

- 16 bytes: copia o WORD baixo de `Parm1@12` para WarGuild; DWORD zero vira
  `0xFFFF`;
- 20 bytes: faz o anterior e copia o byte baixo de `Parm2@16` para WarClan;
- 24 bytes: faz os anteriores e copia o WORD baixo de `Parm3@20` para
  AllyGuild.

Tamanhos diferentes nao alteram estado e a funcao retorna `1`.

### Callees

N/A: o inventario Ghidra registra zero callees. O handler apenas compara o
header e escreve campos escalares no ObjectManager e na cena.

### Saídas e erros

Nao ha alocacao, envio de resposta ou retry. A forma reconhecida retorna `1`
apos as escritas; tamanho desconhecido tambem retorna `1`, sem mutacao.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| `0x3A8/16B` | cena ativa | `FUN_00489300` | WarGuild substituida | regras de alvo leem o novo valor | sem mutacao dos outros campos |
| `0x3A8/20B` | cena ativa | `FUN_00489300` | WarGuild e WarClan substituidos | alvo/render podem mudar | AllyGuild anterior permanece |
| `0x3A8/24B` | cena ativa | `FUN_00489300` | os tres campos substituidos | snapshot completo publicado | retorna `1` |
| outro tamanho | cena ativa | `FUN_00489300` | estado preservado | nenhum | retorna `1` |

### Vtables, vptrs e receptores

N/A para o contrato wire: o dispatch e a chamada sao diretos. O receptor
logico e a cena de campo; WarGuild/AllyGuild residem no ObjectManager global e
WarClan na instancia da cena.

### Ownership

O packet e emprestado somente durante o callback. Os tres valores sao copiados
para campos escalares; o handler nao assume ownership nem conserva ponteiro.

### Falha parcial

No nativo, 16B e 20B atualizam apenas o prefixo presente e deixam o restante
anterior. No par ativo, o gate exige 24B antes do callback, tornando a mutacao
dos tres componentes indivisivel na fronteira de transporte.

### Cleanup e teardown

Nao ha recurso proprio para liberar. Na source, a proxima cena de selecao
reinicializa WarGuild e AllyGuild para `0xFFFF`; a construcao de FieldScene
inicializa WarClan com `-1`.

### Shutdown

N/A: somente campos escalares pertencentes a objetos de lifecycle superior sao
alterados; o handler nao registra callback, timer ou alocacao.

### Logout e relogin

Ao voltar para selecao de personagem, os valores globais sao reinicializados.
O novo enter-world recebe outro snapshot completo antes das demais publicacoes
de score/inventario, impedindo herdar a guerra da sessao anterior.

## Wire, ABI e recursos

Direcao S->C, opcode `0x3A8`, header de 12 bytes. A forma completa tem 24 bytes:

| Campo | Offset | Largura wire | Consumo nativo |
| --- | ---: | ---: | --- |
| WarGuild/Parm1 | 12 | i32 | WORD baixo; zero completo vira `0xFFFF` |
| WarClan/Parm2 | 16 | i32 | byte baixo convertido para `char` |
| AllyGuild/Parm3 | 20 | i32 | WORD baixo |

O nativo aceita tambem os prefixos de 16B e 20B no proprio consumidor.
`FUN_0055890A` nao possui literal/case para `0x3A8`; portanto nao se atribui a
ele um tamanho unico para este opcode. O WYD-Go usa `Header.ID=SceneField
(0x7530)` e payload zerado, que representa ausencia de guerra/cla/alianca.

## Mapeamento atual

### Source recompilável

`TMFieldScene::OnPacketWarInfo` conserva as tres formas nativas e os mesmos
efeitos por tamanho. `ReceivedPacketDispatch` agora seleciona somente a forma
completa de 24B no transporte do peer ativo. `MSG_STANDARDPARM3` possui asserts
para tamanho e offsets.

### WYD-Go

`wire.WarInfo()` constroi 24B, `Type=0x3A8`, `ID=0x7530` e doze bytes de payload
zerados. O enter-world envia esse snapshot depois de materializar o proprio
personagem e antes de score, affects, Carry, equipamento e HP/MP.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| layout completo | 24B e campos em `+12/+16/+20` confirmados | mesma struct | procedencia exata nao alegada | builder 24B | manter como `PARIDADE_NATIVA` |
| formas parciais | 16B/20B aceitas pelo handler | handler ainda as descreve | N/A | nunca emitidas | rejeitar no gate como `MODERNIZACAO_COMPATIVEL` |
| mutacao de estado | prefixo recebido sobrescreve campos correspondentes | comportamento equivalente | N/A | payload completo zerado | manter consumidor |
| validacao central | `FUN_0055890A` nao lista `0x3A8` | gate incremental por opcode | N/A | tamanho unico conhecido | exigir 24B exatos |

## Decisões

- Usar a forma nativa completa de 24B como unico contrato entre os dois
  componentes ativos.
- Rejeitar 16B/20B antes do callback para nao combinar guilda nova com cla ou
  alianca antigos.
- Preservar o switch do handler como documentacao executavel das formas nativas;
  nao criar implementacoes de cena paralelas por versao.

## Lacunas

- Falta executar logout/relogin e regras de alvo/PvP no `project.exe` com dois
  personagens para promover a validacao a `CLIENT_TESTED`.
- A procedencia historica exata do handler na source atual nao foi atribuida;
  a equivalencia usada aqui vem da comparacao direta com o nativo.

## Validação

- Pesquisa: entrada, caller direto, ausencia de callees e tres branches de
  tamanho conferidos no projeto/corpus Ghidra; ausencia de `0x3A8` confirmada
  no export de `FUN_0055890A`.
- Automacao: testes C++ cobrem todo prefixo truncado, excedente, nulo,
  discriminantes divergentes, rejeicao explicita de 16B/20B, entrega unica de
  24B, offsets e imutabilidade. Teste Go cobre builder, destino e payload.
  `ArchitectureTests` passou com 26968 checks; `go test -count=1 ./...`, `go
  vet ./...`, XML, registro unico do header, `git diff --check` e o validador
  de pesquisa passaram. Candidato instalado:
  `023340D32C123AEB6AC54E33D605AE7762B92F884E998250BAE1B93A7F95B021`.
- Client real: nao executado; estado `AUTOMATED TESTED / STATICALLY VERIFIED`,
  nunca `CLIENT_TESTED`.
