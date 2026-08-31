---
id: toto-list-loader
title: Load the Field TOTO match list from its CSV resource
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# Load the Field TOTO match list from its CSV resource

## Pergunta

Como o client nativo 7.48 carrega `UI\\TOTOGame.csv`, materializa a tabela
global usada pelo painel TOTO e se comporta diante de arquivo ausente, header
inválido, linhas incompletas e reconstrução da Field?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; exports
  `00558290_FUN_00558290.c`, `0054fc81_FUN_0054fc81.c`,
  `00435b13_FUN_00435b13.c`, `00472651_FUN_00472651.c`,
  `00418828_FUN_00418828.c` e `0054f0f2_FUN_0054f0f2.c`.
- Asset: `client748/UI/TOTOGame.csv`, header declarado `64` e 64 registros.
- Source: `Basedef.cpp/.h` e caller em `TMFieldScene.cpp`.
- Servidor: não participa deste loader; compra/aposta e seu packet ficam fora
  desta ficha.

## Fluxo nativo 7.48

### Entrada observável

- Evento: inicialização da Field em `FUN_00435B13` chama
  `FUN_00558290("UI\\TOTOGame.csv")`.
- Precondições: arquivo acessível pelo diretório de execução e primeira linha
  disponível.
- Saída: tabela global zerada e preenchida por índice; retorno `1` para header
  aceito, inclusive quando o arquivo termina antes da quantidade declarada.

### Callers

`FUN_00435B13` é o caller direto confirmado. Ele ignora o retorno do loader e
continua a inicialização da Field, portanto falha do recurso deixa o painel sem
dados mas não encerra a cena.

### Função principal

`FUN_00558290 @ 0x00558290` abre o arquivo em modo texto, zera
`DAT_00D42700` por `0x1E00` bytes e lê o número declarado na primeira linha.
Valores menores que 80 entram no loop; valores a partir de 80 fecham o arquivo
e retornam `0`.

Cada linha tem suas vírgulas trocadas por espaços e é lida como
`index time teamA teamB`. `FUN_0054FC81` troca underscores por espaços nos três
campos. Somente índices `1..78` copiam uma entrada inteira de `0x60` bytes para
`DAT_00D42700 + index * 0x60`.

### Callees

- CRT de arquivo: abertura, `fgets` de até 1024 bytes e fechamento.
- CRT de memória/parsing: `memset` e leitura formatada do header/registro.
- `FUN_0054FC81`: conversão in-place de `_` para espaço.

Os consumidores confirmados não são callees do loader. `FUN_00472651` valida
o número escolhido contra a quantidade declarada e publica `time/teamA/teamB`
nos três controles de texto. `FUN_00418828` também indexa a tabela ao montar
texto relacionado ao item TOTO.

### Saídas e erros

- Falha ao abrir ou ausência da primeira linha: fecha quando necessário e
  retorna `0`.
- Quantidade declarada `>= 80`: tabela permanece zerada e retorna `0`.
- EOF durante os registros: preserva o subconjunto já carregado e retorna `1`.
- Índice fora de `1..78`: ignora somente o registro.
- O parser recompilável exige os quatro campos e limita cada string a 31 bytes;
  isso torna linhas malformadas determinísticas e evita reproduzir o overflow
  possível do `%s` sem largura do executável, sem alterar o asset válido.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| entrada na Field | arquivo válido | `FUN_00558290` | tabela recarregada | 64 registros disponíveis | retorna `1` |
| arquivo ausente | tabela global inicial | open falha | tabela anterior/zero permanece | nenhum | retorna `0`; caller continua |
| header vazio | arquivo aberto | primeiro `fgets` falha | tabela zerada | fecha arquivo | retorna `0` |
| quantidade >= 80 | header lido | validação | tabela zerada | fecha arquivo | retorna `0` |
| linha válida | índice 1..78 | parse/cópia | slot de 96 bytes preenchido | underscores viram espaços | continua |
| linha inválida | arquivo/header aceitos | parse/range | slot não alterado | nenhum | continua |
| EOF antecipado | parte já lida | `fgets` falha | prefixo válido preservado | fecha arquivo | retorna `1` |

### Vtables, vptrs e receptores

Não há dispatch virtual no loader. O receptor observável é a Field em
`FUN_00435B13`; tabela e quantidade são globais. Os controles consumidores
usam seus próprios métodos virtuais de texto, fora do ownership do loader.

### Ownership

A tabela é armazenamento estático de 80 entradas; não possui heap, handles
persistentes ou ponteiros para o buffer de linha. O loader possui somente o
`FILE*` durante a chamada e sempre o fecha após abertura bem-sucedida.

### Falha parcial

Depois de abrir, a tabela é zerada antes do header e de qualquer registro. Uma
falha de header deixa estado vazio. EOF antecipado deixa apenas os registros
completos já copiados. A source ignora registros que não tenham os quatro
campos, em vez de publicar dados parcialmente inicializados.

### Cleanup e teardown

Cada chamada fecha o arquivo. Não há callback, thread, alocação dinâmica ou
cleanup adicional.

### Shutdown

`FUN_0054F0F2` também zera a tabela durante a inicialização global. Por ser
memória estática sem recursos associados, o shutdown não exige destrutor.

### Logout e relogin

A reconstrução da Field chama novamente o loader e substitui integralmente os
dados quando o arquivo abre. Nenhum estado de seleção/aposta é criado por esta
função e nada precisa atravessar logout/relogin.

## Wire, ABI e recursos

- Recurso: `UI\\TOTOGame.csv`, texto delimitado por vírgulas.
- Header: inteiro decimal; o asset atual declara 64 linhas.
- Registro: `index,time,teamA,teamB`; underscores codificam espaços.
- Tabela: 80 entradas estáticas de 96 bytes (`0x1E00` no total).
- Entrada: `szTime[32]`, `szTeamA[32]`, `szTeamB[32]`, sem padding.
- Índices materializados: `1..78`; slot zero e slot 79 não são escritos.
- Texto coreano é tratado como bytes do arquivo legado; o loader não converte
  encoding.
- Não há packet, opcode ou ABI server-side nesta transição.

## Mapeamento atual

### Source recompilável

`STRUCT_TOTOLIST` já possuía o layout correto e
`TMFieldScene::InitializeScene` já chamava `BASE_ReadTOTOList` com o recurso
correto. O stub foi substituído por uma implementação de largura limitada,
com `g_pTOTOList[80]`, quantidade declarada e `static_assert` de 96 bytes.

### WYD-Go

Não há consumo server-side do CSV. Métodos de seleção/compra e contrato de
aposta permanecem desativados até pesquisa própria de UI e wire.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject/W2PP | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| entrada | initializer da Field | caller já vivo | mesmo caller | N/A | manter |
| armazenamento | 80 x 96 bytes | layout e tabela materializados | struct presente, loader stub | N/A | portar |
| parsing válido | header + CSV + underscores | parser limitado equivalente | stub | N/A | portar com limites |
| malformado | `%s` sem largura e resultado parcial | exige quatro campos/31 bytes | stub | N/A | modernizar com segurança |
| aposta/packet | consumidores nativos separados | métodos ainda vazios | implementação incompleta | ausente/não confirmado | não implementar neste lote |

## Decisões

- Classificar o loader válido como `PARIDADE_NATIVA` e o endurecimento de
  linhas malformadas como `MODERNIZACAO_COMPATIVEL` do mesmo contrato externo.
- Preservar o layout nativo exato e a faixa incomum `1..78`.
- Expor a quantidade declarada para os consumidores futuros, sem ativar UI de
  aposta ou inventar packet a partir de versão posterior.
- Manter a ficha em `CONTRACT`; build não equivale a teste real da Field.

## Lacunas

- Executar entrada/reentrada na Field com asset presente, ausente e truncado.
- Confirmar visualmente os três textos depois que o fluxo de seleção for
  implementado em lote próprio.
- Investigar separadamente `TotoSelect`, `TotoClose`, `TotoBuy`, reel e wire
  antes de ativar apostas.

## Validação

- Pesquisa: caller, função principal, helpers, consumidores, layout, falhas,
  ownership e lifecycle fechados no corpus do hash registrado.
- Automação: o asset possui 64 registros para o header 64, índices 1..64 e
  nenhum campo acima de 31 bytes; `validate_research.py` e `git diff --check`
  passaram.
- Build: `Build-Client.ps1` passou em Release/Win32 e instalou um candidato
  byte a byte idêntico ao build. `client748/project.exe` tem SHA-256
  `CC317B6FB3EE4723DD0348AA32431ABA45D87AF7329936D5F15E3D0D8A06AFA3`.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
