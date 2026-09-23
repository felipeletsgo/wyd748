# Build e integração

Os caminhos abaixo partem da raiz do repositório. Dados relativos do servidor
são resolvidos a partir de `wydgo748/`.

## Servidor

Requisitos:

- Go na versão exata ou compatível indicada em `wydgo748/go.mod`;
- Node.js `22.12` ou superior para gerar o painel Astro;
- PostgreSQL para o boot e para os testes de integração que usam banco.

O painel está habilitado no `data/server.txt` versionado, mas o diretório
`web/portal/dist/` é gerado e ignorado pelo Git. Em um clone novo, gere o painel
antes do primeiro boot ou desabilite `web_admin_enabled`.

```powershell
pwsh -NoProfile -File tools/web-admin/Start-WYDAdmin.ps1 -NoBrowser

Push-Location wydgo748
New-Item -ItemType Directory -Force bin | Out-Null
go test -count=1 ./...
go vet ./...
go build -o bin/tm.exe ./cmd/server
go build -o bin/account-create.exe ./cmd/account-create
go build -o bin/account-api.exe ./cmd/account-api
Pop-Location
```

| Saída | Uso |
| --- | --- |
| `wydgo748/bin/tm.exe` | servidor do jogo e painel administrativo integrado |
| `wydgo748/bin/account-create.exe` | cadastro local interativo |
| `wydgo748/bin/account-api.exe` | API de cadastro separada; publicar somente por proxy HTTPS |
| `wydgo748/web/portal/dist/` | assets compilados do painel integrado |

Inicie `tm.exe` com o diretório atual em `wydgo748/`; iniciar pela raiz faz os
caminhos relativos `data/...` apontarem para o local errado. Consulte
[operação](server/operations.md) para banco, configuração, conta, portas,
encerramento e backup.

Testes PostgreSQL usam `WYD_TEST_POSTGRES_URL`; sem essa configuração, os
testes condicionais não comprovam integração com o banco.

## CI e execução

O workflow versionado valida o layout do repositório, pesquisa, assets, servidor
Go com PostgreSQL e client C++ no Windows. Ele não publica nem configura um
servidor externo. O runtime suportado é o processo nativo `bin/tm.exe`, iniciado
a partir de `wydgo748/`; o repositório não possui arquitetura de container ou
integração com provedor de deployment.

O conversor histórico `cmd/npcconvert` não participa do runtime. Quando seu
formato de entrada for necessário, exige `-in` e `-out` explícitos; não busca
projetos externos nem grava em dados ativos por padrão. Resultados de conversão
devem ser revisados antes de qualquer incorporação em `data/`.

## Client

Requisitos: Windows, Visual Studio com C++ x86 e toolset v145, SDK Windows.
As dependências DirectX ficam na própria árvore do client.

```powershell
pwsh -NoProfile -File tmproject/Build-Client.ps1 -Configuration Release
```

O script compila e executa os testes de arquitetura, compila a solução e
copia `tmproject/build/TMProject748/Release/WYD.exe` para
`tmproject/client748/project.exe`, conferindo SHA-256. Não inicia nem encerra
o jogo. O executável instalado é um produto, não a referência nativa.

Para validar sem substituir o executável instalado, acrescente `-NoDeploy`.
Nesse modo, o artefato e seu SHA-256 ficam em `tmproject/build/`; nenhum
resultado deve ser descrito como teste visual ou `CLIENT-TESTED`. Na máquina
usada nesta campanha, o teste de tela está bloqueado e não deve ser repetido;
registre o gate de execução real como pendente.

## Fronteira entre os projetos

O client envia intenções; o servidor valida e responde com estado autoritativo.
Para mudanças de contrato, registrar direção, opcode, tamanho, offsets,
packing, rejeições e compatibilidade antes de implementar nos dois lados.
A arquitetura 7.69 pode permanecer quando não impõe recursos ou formatos
incompatíveis; não presumir equivalência entre versões.

Build e testes não comprovam conexão, login, entrada no mundo ou logout/relogin.
Esses fluxos exigem validação integrada com o client construído e o servidor
atual. O estado de cada frente fica nas fichas de
[pesquisa](../.agents/research/client748/README.md) e no roadmap.

## Organização

```powershell
pwsh -NoProfile -File tools/repository/Test-RepositoryLayout.ps1
```

O comando confere inventário, links Markdown locais, regras únicas e
documentação fora das sources. Use `-UpdateMap` somente quando documentos forem
adicionados, removidos ou movidos; alterações apenas de conteúdo não exigem
regenerar o mapa.

O acervo `references/client748/` é evidência; seus patchers são históricos,
não ferramentas do desenvolvimento ativo.
