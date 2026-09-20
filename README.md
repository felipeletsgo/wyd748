# WYD 7.48 — client e servidor

Monorepo do WYD-Go 7.48: servidor autoritativo em Go e client C++ adaptado
para o runtime 7.48. O projeto ainda exige validação em jogo para fluxos que
estão marcados como pendentes; um build aprovado não equivale a teste no client.

| Local | Conteúdo |
| --- | --- |
| `wydgo748/` | Servidor Go, configuração e dados autoritativos |
| `tmproject/TMProject748/` | Source C++ e testes do client |
| `tmproject/client748/` | Runtime e assets do client 7.48 |
| `DOCS/` | Documentação de arquitetura, operação e contratos |
| `references/client748/` | Evidência histórica somente leitura |

## Início rápido no Windows

Este procedimento inicia PostgreSQL, compila o painel e o servidor, cria uma
conta e deixa o game server escutando na porta TCP `8281`.

### 1. Pré-requisitos

- PowerShell 7;
- Go na versão indicada em [`wydgo748/go.mod`](wydgo748/go.mod) (`1.26.5` atualmente);
- Node.js `22.12` ou superior, com `npm`;
- PostgreSQL 16 ou superior, com `psql` disponível no terminal.

Clone o repositório e abra o PowerShell na raiz dele. Confirme que o serviço do
PostgreSQL está ativo; no Windows, o nome inclui a versão instalada:

```powershell
Get-Service -Name 'postgresql*'
```

Se estiver parado, inicie-o em um PowerShell administrativo com
`Get-Service -Name 'postgresql*' | Start-Service`. Crie um usuário e um banco
exclusivos pelo SQL Shell (`psql`):

```powershell
psql -U postgres -d postgres
```

```sql
CREATE ROLE wydgo LOGIN PASSWORD 'wydgo-local';
CREATE DATABASE wydgo OWNER wydgo;
\q
```

Se eles já existirem, não repita os comandos. Use uma senha diferente em
produção e mantenha a porta `5432` acessível somente na rede privada/local.

### 2. Configure o banco e compile

A variável deve ser definida no mesmo terminal que iniciará o servidor. O
schema PostgreSQL é instalado e validado automaticamente no primeiro boot.

```powershell
$env:WYD_DATABASE_URL = 'postgres://wydgo:wydgo-local@127.0.0.1:5432/wydgo?sslmode=disable'

pwsh -NoProfile -File .\tools\web-admin\Start-WYDAdmin.ps1 -NoBrowser

Push-Location .\wydgo748
New-Item -ItemType Directory -Force .\bin | Out-Null
go build -o .\bin\tm.exe .\cmd\server
go build -o .\bin\account-create.exe .\cmd\account-create
Pop-Location
```

O script do painel só prepara os arquivos web; ele não inicia nem encerra o
servidor. Para trabalhar sem o painel, defina `web_admin_enabled=false` em
[`wydgo748/data/server.txt`](wydgo748/data/server.txt) e pule esse script.

### 3. Inicie o servidor

Execute sempre a partir de `wydgo748/`, pois `data/server.txt` usa caminhos
relativos a essa pasta.

```powershell
Push-Location .\wydgo748
.\bin\tm.exe
```

O boot correto informa PostgreSQL autoritativo e termina com o listener do jogo
em `0.0.0.0:8281`. Com o painel habilitado, acesse
`http://127.0.0.1:8082/admin/`. Troque o `admin_access_pin` de exemplo antes de
usar o painel fora de um ambiente local. A conta administrativa também precisa
estar autorizada em `wydgo748/data/staff.json`.

Não feche o processo à força. Use `Ctrl+C` para o servidor drenar persistência e
encerrar. Para os próximos boots, confirme que o serviço PostgreSQL está ativo
e execute o servidor novamente:

```powershell
Get-Service -Name 'postgresql*' | Start-Service
Push-Location .\wydgo748
.\bin\tm.exe
```

### 4. Crie uma conta

Em outro PowerShell, a partir da raiz do repositório:

```powershell
$env:WYD_DATABASE_URL = 'postgres://wydgo:wydgo-local@127.0.0.1:5432/wydgo?sslmode=disable'
Push-Location .\wydgo748
.\bin\account-create.exe
Pop-Location
```

O utilitário solicita usuário e senha no terminal. A senha não deve ser passada
na linha de comando.

### 5. Conecte o client

O client deve apontar para o IP ou domínio do host do servidor e para a porta
TCP `8281`. Em teste local, use `127.0.0.1`; em outro computador, libere somente
`8281/TCP` no firewall e use o endereço alcançável desse host. O endpoint do
PostgreSQL e as portas administrativas devem continuar privados.

Para recompilar e instalar o client no runtime local:

```powershell
pwsh -NoProfile -File .\tmproject\Build-Client.ps1 -Configuration Release
```

O script instala o resultado em `tmproject/client748/project.exe`. A configuração
do endpoint fica no `serverlist.bin` do runtime; não edite esse binário como
texto. O repositório ainda não oferece um editor para esse formato: use um
`serverlist.bin` compatível já gerado para o endpoint de destino.

## Solução rápida de problemas

- `PostgreSQL configurado, mas WYD_DATABASE_URL esta vazia`: defina a variável
  no mesmo processo/terminal que inicia o servidor.
- `connection refused` em `5432`: inicie o PostgreSQL e confira host, porta,
  usuário e banco na URL.
- o jogo não conecta: confirme que o servidor anunciou `0.0.0.0:8281`, que o
  client usa o endereço correto e que `8281/TCP` está liberada.
- o painel não iniciou: gere `wydgo748/web/portal/dist` com
  `tools/web-admin/Start-WYDAdmin.ps1` ou desabilite o painel na configuração.
- caminhos `data/...` não encontrados: o processo foi iniciado fora de
  `wydgo748/`.

O guia completo de banco, conta, painel, segurança, backup e encerramento está
em [Operação do servidor](DOCS/server/operations.md). Builds e gates estão em
[Build e integração](DOCS/build-and-integration.md).

## Documentação

- [Índice por tarefa](DOCS/README.md)
- [Inventário completo](DOCS/documentation-map.md)
- [Estado implementado](DOCS/IMPLEMENTED.md)
- [Pendências e critérios de aceite](DOCS/ROADMAP.md)
- [Regras do repositório](AGENTS.md)

O servidor valida intenções e mantém a verdade do jogo. Arquitetura posterior
só é aproveitada quando compatível com o contrato 7.48 ou por extensão explícita
e testada nos dois projetos. Evidência nativa e testes devem sustentar cada
afirmação de paridade.
