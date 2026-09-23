# Ambiente Windows para o WYD-Go

Este guia cobre a preparação do checkout atual no Windows. Para versões de
dependências, consulte primeiro `wydgo748/go.mod`,
`wydgo748/web/portal/package.json` e os projetos `.vcxproj`. Não reutilize
caminhos, executáveis ou credenciais de instalações anteriores.

## Ferramentas

- Git e PowerShell 7 (`pwsh`);
- Go na versão declarada em `wydgo748/go.mod`;
- Node.js compatível com `wydgo748/web/portal/package.json` e `npm`;
- Visual Studio com MSBuild, ferramentas C++ x86 e o toolset indicado em
  `tmproject/TMProject748/TMProject748.vcxproj`;
- PostgreSQL e `psql` para a persistência do servidor;
- Python e Java somente quando a pesquisa nativa/Ghidra exigir.

O client usa as dependências DirectX já presentes em
`tmproject/TMProject748/Dependencies/Directx`. Docker, CMake e Ninja não são
requisitos da arquitetura atual. O projeto Ghidra e seus dados devem ser
mantidos fora do repositório; `references/client748/` contém evidências
históricas somente leitura.

## Preparar o checkout

Execute a partir da raiz do repositório:

```powershell
git status --short
git rev-parse HEAD
pwsh -NoProfile -File .\tools\repository\Test-RepositoryLayout.ps1
go version
node --version
npm --version
psql --version
```

Preserve alterações locais antes de atualizar ou mover o checkout. Não use um
clone novo como substituto de arquivos não rastreados, ignorados, projetos
Ghidra ou segredos. Guarde credenciais fora do Git. Consulte
[Operação do servidor](server/operations.md) para a configuração de
`WYD_DATABASE_URL`, banco e backups.

## Compilar e validar

O procedimento e os gates de integração ficam em
[Build e integração](build-and-integration.md). Para compilar o client sem
substituir o executável do runtime:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File .\tmproject\Build-Client.ps1 -NoDeploy
```

Sem `-NoDeploy`, o script instala o candidato em
`tmproject/client748/project.exe`; só use esse modo quando a tarefa incluir a
troca do runtime. Build e testes automatizados não equivalem a teste visual do
client (`CLIENT-TESTED`).

O servidor deve ser executado com o diretório de trabalho em `wydgo748/`.
Para conectar o client, gere `tmproject/client748/serverlist.bin` com o editor
externo de serverlist e o endereço alcançável do servidor. Execute o client a
partir de `tmproject/client748/`, pois ele resolve assets relativamente ao
diretório do runtime. O contrato de conexão e os passos de operação estão em
[Build e integração](build-and-integration.md) e
[Operação do servidor](server/operations.md).
