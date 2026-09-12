# Build e integração

Os caminhos abaixo partem da raiz do repositório. Dados relativos do servidor
são resolvidos a partir de `wydgo748/`.

## Servidor

Requisito: versão Go indicada em `wydgo748/go.mod`.

```powershell
Push-Location wydgo748
go test -count=1 ./...
go vet ./...
go build -o bin/tm.exe ./cmd/server
Pop-Location
```

Consulte [operação](server/operations.md) para configuração e inicialização.
Testes PostgreSQL usam `WYD_TEST_POSTGRES_URL`; sem essa configuração, os
testes condicionais não comprovam integração com o banco.

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
pwsh -NoProfile -File tools/repository/Test-RepositoryLayout.ps1 -UpdateMap
pwsh -NoProfile -File tools/repository/Test-RepositoryLayout.ps1
```

O primeiro comando atualiza o inventário; o segundo confere sua atualidade,
links Markdown locais, regras únicas e documentação fora das sources.
O acervo `references/client748/` é evidência; seus patchers são históricos,
não ferramentas do desenvolvimento ativo.
