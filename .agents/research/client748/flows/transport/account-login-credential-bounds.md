---
id: account-login-credential-bounds
title: Account login credential bounds before connection
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-22
---

# Account login credential bounds before connection

## Pergunta

Quais limites de conta e senha o client nativo WYD 7.48 aplica antes de abrir a
conexão de login, quais mensagens exibe e como esses limites se relacionam com
o campo fixo de senha do packet `0x20D`?

## Fronteira de evidência

- Executável/hash: `references/client748/WYD.exe`, SHA-256 do frontmatter.
- Função nativa: `FUN_004AC985`, já localizada no projeto Ghidra
  `WYD748Native_20260821.gpr` pela ficha
  `socket-disconnect-return-selectserver.md`; o projeto não está disponível
  neste checkout, então os branches de tamanho deste recorte foram
  reproduzidos diretamente no executável exato com o disassembler do MSVC.
- Asset: `tmproject/client748/UI/strdef.bin`, registros de 128 bytes ofuscados
  com XOR `0x5A`, conforme o loader `WYD748_LoadMessageStrings` em
  `tmproject/TMProject748/internal/core/WYD748Assets.cpp`.
- Source atual: `TMSelectServerScene.cpp`, `SharedStructs.h` e o teste
  `LoginCredentialContractTests.cpp` em `tmproject/TMProject748/`.
- Servidor e consumidores: `internal/account`, `internal/webadmin`,
  `internal/accountapi`, `internal/loadtest`, portal e OpenAPI em `wydgo748/`.

O recorte fecha os limites observáveis antes da primeira conexão e sua
validação autoritativa. Regras de composição além de tamanho/ASCII, recuperação
de contas antigas e credenciais da migração entre servidores ficam fora dele.

## Fluxo nativo 7.48

### Entrada observável

- Evento: confirmação da tela de login com conta e senha preenchidas.
- Precondição: controles de edição ativos e nenhuma conexão de login iniciada.
- Saída válida: o client abre a conexão e monta `MSG_AccountLogin`.
- Saída inválida: o client publica a mensagem correspondente e retorna antes
  da chamada de conexão.

### Callers

- O handler de confirmação da cena de seleção alcança `FUN_004AC985`, a mesma
  raiz já resolvida para o connect inicial na ficha de transporte existente.
- A função lê os dois controles de edição, valida seus comprimentos e somente
  depois chama o fluxo que abre a porta 8281 e envia o login.

### Função principal

No binário registrado, `FUN_004AC985` contém os seguintes branches:

- `0x004ADC22`: chama `strlen` para a conta; `cmp eax, 0x0C` em
  `0x004ADC2A` e `jbe 0x004ADC78` em `0x004ADC2D` aceitam no máximo 12 bytes;
- `0x004ADC95`: compara o comprimento da senha com 4 e segue apenas em
  `jae`, rejeitando valores menores;
- `0x004ADCF8`: chama `strlen` para a senha; `cmp eax, 0x0A` em
  `0x004ADD00` e `jbe 0x004ADD4E` em `0x004ADD03` aceitam no máximo 10 bytes.

O limite lógico de senha é portanto 4–10, mesmo que o campo wire tenha 12
bytes. Senhas de 11 ou 12 caracteres são rejeitadas localmente e nunca chegam
ao servidor pelo login normal do client 7.48.

### Callees

- O branch de senha curta seleciona a entrada 5 da tabela de mensagens.
- O branch de senha longa seleciona a entrada 6 da tabela de mensagens.
- Após todas as validações, o fluxo de conexão monta e envia o packet
  `0x20D/0x74` com versão 748.

### Saídas e erros

- Conta acima de 12 bytes não inicia conexão.
- Senha abaixo de 4 bytes mostra a mensagem de senha curta e não inicia
  conexão.
- Senha acima de 10 bytes mostra a mensagem de senha longa e não inicia
  conexão.
- Os registros decodificados do `strdef.bin` confirmam a entrada 5 como
  `Your password is not long enough.` e a entrada 6 como
  `Your password is too long.`.

## Estado e lifecycle

| Evento/estado | Precondição | Branch | Estado resultante | Side effect |
| --- | --- | --- | --- | --- |
| conta longa | conta > 12 bytes | `0x004ADC2A` | permanece na seleção | mensagem; sem conexão |
| senha curta | senha < 4 bytes | `0x004ADC95` | permanece na seleção | mensagem 5; sem conexão |
| senha longa | senha > 10 bytes | `0x004ADD00` | permanece na seleção | mensagem 6; sem conexão |
| credenciais válidas | conta <= 12 e senha 4–10 | `0x004ADD4E` em diante | conexão inicial | monta e envia `0x20D` |

Os controles continuam habilitados até as validações terminarem. O packet e o
socket só passam a existir no caminho válido; não há rollback de rede para os
três erros de comprimento.

## Wire, ABI e recursos

- Login client -> servidor: opcode `0x20D`, tamanho `0x74`/116 bytes.
- `AccountName` começa em `+0x0C` e ocupa 16 bytes.
- `AccountPassword` começa em `+0x1C` e ocupa 12 bytes.
- `ClientVersion` começa em `+0x28` e recebe 748.
- O campo wire de 12 bytes permanece inalterado. O limite de 10 deixa espaço
  para terminador e não requer mudança de struct, packing, framing ou ABI.
- O recurso de mensagens permanece o `strdef.bin` nativo; nenhum ID novo é
  introduzido.

## Mapeamento atual

### Source recompilável

`TMSelectServerScene.cpp` já rejeitava senha menor que quatro caracteres, mas
não rejeitava o limite superior antes de conectar. O envio posterior usava
`strncpy_s(..., _TRUNCATE)`, permitindo que uma entrada longa fosse aceita e
silenciosamente truncada. O source agora rejeita `strlen > 10`, mostra
`g_pMessageStringTable[6]` e retorna antes de desabilitar os controles, conectar
ou copiar para o packet.

`SharedStructs.h` conserva `AccountPassword[12]` e os `static_assert` do packet
`0x20D/0x74`. O teste de contrato fixa ambos os limites, a mensagem e a ordem
do guard em relação ao connect e à cópia limitada.

### WYD-Go

`internal/account` aceita criação com 4–10 caracteres e autenticação wire com
1–10, preservando a distinção histórica entre política de criação e pacote de
login. O endpoint administrativo, a API pública, o provisionador de carga, o
portal e o OpenAPI convergem para o máximo 10. A autenticação administrativa
usa a mesma conta autoritativa e também rejeita entradas acima do limite antes
de consultar a senha armazenada.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | WYD-Go anterior | Decisão |
| --- | --- | --- | --- | --- |
| conta no login | máximo 12 | máximo 12 | máximo 12 | preservar |
| senha mínima no client | 4 | 4 | criação 4; wire aceitava 1 | preservar |
| senha máxima no client | 10 | ausente; truncava para o campo | criação/auth aceitavam 12 | alinhar tudo a 10 |
| campo wire | 12 bytes | 12 bytes | decodifica 12 bytes | preservar ABI |
| mensagem de senha longa | tabela 6 | disponível, não usada nesse guard | N/A | restaurar branch nativo |

## Decisões

- Classificar o lote como `PARIDADE_NATIVA`: o limite 10 e o retorno anterior
  à conexão são comprovados no executável 7.48 do hash registrado.
- Manter o packet fixo de 12 bytes e a versão 748 sem qualquer extensão de
  protocolo.
- Aplicar o máximo 10 também no servidor e em todos os criadores/autenticadores
  para que o servidor continue autoritativo e não produza contas impossíveis de
  usar no client nativo.
- Não migrar nem reescrever automaticamente hashes existentes. Contas criadas
  anteriormente com 11 ou 12 caracteres precisam de recuperação/troca de senha
  operacional; elas já não eram utilizáveis pelo fluxo nativo 7.48.

## Lacunas

- Executar no client recompilado uma tentativa com 11 caracteres e confirmar
  a mensagem 6 sem conexão.
- Executar uma autenticação válida com exatamente 10 caracteres até a seleção
  de personagem.
- Definir, em tarefa operacional separada, a recuperação de eventuais contas
  legadas do emulador que possuam senha de 11 ou 12 caracteres.

## Validação

- Pesquisa: os branches e endereços acima foram reproduzidos no executável do
  hash registrado; as mensagens 5 e 6 foram decodificadas do asset usado pelo
  runtime; callers e envio foram vinculados à raiz `FUN_004AC985` já resolvida.
- Automação: `go test -count=1 ./...` e `go vet ./...` passaram no módulo do
  servidor. No portal, `npm run generate`, `npm run check` e `npm run build`
  passaram sem
  diagnósticos. `Build-Client.ps1` passou 51.711 checks, compilou Release/Win32
  e instalou `tmproject/client748/project.exe` com SHA-256
  `24B8A2B08E93BFF1F6B9AA9F72DF9C0283B751F12DAB44535AB3BA25A5EF77C3`.
  O validador de fichas aprovou o estado `CONTRACT`.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
