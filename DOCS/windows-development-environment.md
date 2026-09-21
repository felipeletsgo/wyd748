# Ambiente Windows para continuar o WYD-Go

Este guia prepara um Windows recém-instalado para desenvolver, compilar e
executar o servidor Go, o client C++ 7.48, o painel web e a pesquisa nativa com
Ghidra. A referência usada para este levantamento foi o checkout de
20/09/2026 em um Dell G15 5520.

As versões mínimas devem ser conferidas nos arquivos do próprio repositório
antes de instalar. Este documento registra a configuração conhecida, mas
`wydgo748/go.mod`, `wydgo748/web/portal/package.json` e os projetos `.vcxproj`
são as fontes de verdade quando houver divergência.

## Antes de formatar: backup obrigatório

> **Não formate enquanto estes itens não existirem em outro disco e não tiverem
> sido verificados.** Um clone novo do Git não recupera alterações locais,
> arquivos ignorados, projeto Ghidra nem segredos.

Confira `git status --short` imediatamente antes do backup, mesmo que o estado
esteja limpo agora. A cópia completa protege alterações locais, arquivos não
rastreados e arquivos ignorados que um clone não traz. Não é necessário separar
ou conferir `tmproject/client748/project.exe` e `serverlist.bin`: o primeiro é
recompilado pela source e o segundo é regenerado pelo editor de serverlist.

### 1. Feche os processos

Feche o client, o servidor, Visual Studio, Ghidra e qualquer terminal que esteja
gravando no repositório. Encerre o servidor com `Ctrl+C`, não pelo Gerenciador
de Tarefas, para permitir que a persistência termine corretamente.

### 2. Copie o repositório inteiro

Conecte um SSD/HD externo com espaço livre. No exemplo, substitua `E:` pela
unidade real:

```powershell
$backupRoot = 'E:\WYD-Backup-2026-09-20'
$repo = 'C:\Users\felipe\Documents\CLAUDE\SERVER 7.54\wyd-go'

New-Item -ItemType Directory -Path $backupRoot -Force | Out-Null
robocopy $repo "$backupRoot\wyd-go" /E /COPY:DAT /DCOPY:DAT /R:2 /W:2 /XJ
if ($LASTEXITCODE -ge 8) { throw "Falha no backup do repositorio: $LASTEXITCODE" }
```

Essa cópia inclui `.git`, alterações rastreadas, arquivos não rastreados e
arquivos ignorados do runtime. Preserve uma cópia intacta mesmo depois de
restaurar o computador.

Registre também o estado do Git e um patch auxiliar. O patch não substitui a
cópia completa, pois não contém o conteúdo dos arquivos não rastreados:

```powershell
Push-Location $repo
git status --short | Out-File "$backupRoot\git-status.txt" -Encoding utf8
git rev-parse HEAD | Out-File "$backupRoot\git-head.txt" -Encoding ascii
git diff --binary HEAD | Out-File "$backupRoot\working-tree.patch" -Encoding utf8
git ls-files --others --exclude-standard |
    Out-File "$backupRoot\untracked-files.txt" -Encoding utf8
Pop-Location
```

### 3. Não preserve o banco desta reinstalação

Para esta formatação, o banco PostgreSQL atual é deliberadamente descartável.
Não restaure dump, diretório de dados ou tabelas antigas. O primeiro boot após
a reinstalação será usado para provar que o servidor cria corretamente seu
schema embutido em uma base vazia.

Não salve a URL do banco ou senhas dentro do Git. Se quiser preservar somente a
configuração, guarde as credenciais em um gerenciador de senhas ou arquivo
criptografado. Registre os nomes das variáveis que precisarão ser recriadas:

- `WYD_DATABASE_URL` para o servidor e ferramentas normais;
- `WYD_TEST_POSTGRES_URL` para testes integrados com banco;
- variáveis de load test somente se esse ambiente também for restaurado.

### 4. Copie o projeto e o corpus Ghidra

O arquivo `.gpr` é somente a entrada do projeto; a base real fica na pasta
`.rep` correspondente. Copie os dois, além do corpus de decompilação:

```powershell
$ghidraProjects = 'C:\Users\felipe\Tools\GhidraProjects'
$ghidraAnalysis = 'C:\Users\felipe\Tools\GhidraAnalysis'

robocopy $ghidraProjects "$backupRoot\GhidraProjects" /E /COPY:DAT /R:2 /W:2
if ($LASTEXITCODE -ge 8) { throw "Falha no backup GhidraProjects: $LASTEXITCODE" }

robocopy $ghidraAnalysis "$backupRoot\GhidraAnalysis" /E /COPY:DAT /R:2 /W:2
if ($LASTEXITCODE -ge 8) { throw "Falha no backup GhidraAnalysis: $LASTEXITCODE" }
```

Os caminhos de continuidade conhecidos são:

- `C:\Users\felipe\Tools\GhidraProjects\WYD748Native_20260821.gpr`;
- `C:\Users\felipe\Tools\GhidraProjects\WYD748Native_20260821.rep`;
- `C:\Users\felipe\Tools\GhidraAnalysis\20260821\decompiled`.

### 5. Preserve somente o que for necessário fora do projeto

Considere salvar em mídia criptografada:

- chave de recuperação do BitLocker;
- instalador do driver de rede do Dell G15 5520, para o caso de o Windows não
  reconhecer Wi-Fi ou Ethernet após a instalação;
- `%USERPROFILE%\.gitconfig`;
- `%USERPROFILE%\.ssh`, somente se souber proteger as chaves privadas;
- `%USERPROFILE%\.codex`, se quiser manter configurações e memória local do
  Codex; essa pasta pode conter dados sensíveis e não deve ir para Git nem para
  armazenamento público;
- `C:\Users\felipe\Documents\CLAUDE\SERVER 7.54\serverlist editor.exe`, pois o
  editor usado para regenerar `serverlist.bin` é uma ferramenta externa e não
  está versionado no repositório;
- screenshots, logs e dumps do client que tenham valor de diagnóstico.

Copie o editor atual para a mídia externa e registre seu hash:

```powershell
$serverListEditor =
    'C:\Users\felipe\Documents\CLAUDE\SERVER 7.54\serverlist editor.exe'
Copy-Item -LiteralPath $serverListEditor -Destination $backupRoot
Get-FileHash -LiteralPath "$backupRoot\serverlist editor.exe" -Algorithm SHA256 |
    Format-List |
    Out-File "$backupRoot\serverlist-editor-sha256.txt" -Encoding ascii
```

Um backup localizado em `%TEMP%` será apagado na formatação. Não dependa dele.
Para os drivers, prefira baixar novamente os pacotes oficiais.

### 6. Verifique a cópia antes de apagar o disco

Abra alguns arquivos diretamente no disco externo e confirme os registros:

```powershell
Get-Content "$backupRoot\git-head.txt"
Get-Content "$backupRoot\git-status.txt"
Get-Content "$backupRoot\serverlist-editor-sha256.txt"
Get-Item "$backupRoot\serverlist editor.exe"
Get-ChildItem "$backupRoot\GhidraProjects\WYD748Native_20260821.gpr"
Get-ChildItem "$backupRoot\GhidraProjects\WYD748Native_20260821.rep"
```

O backup mínimo aprovado contém a pasta completa `wyd-go`, o `.gpr`, a pasta
`.rep`, o corpus Ghidra, o editor de serverlist e as credenciais guardadas fora
do repositório. O banco PostgreSQL não faz parte deste backup por decisão
explícita desta reinstalação.

## Componentes necessários

| Componente | Requisito do projeto | Uso |
| --- | --- | --- |
| Windows 11 x64 | Atualizado e ativado | Sistema do client e ferramentas |
| Drivers Dell | Chipset, Intel e NVIDIA funcionando juntos | DirectX 9 e gráficos híbridos do notebook |
| Git for Windows | Versão estável atual | Histórico e sincronização do repositório |
| PowerShell | PowerShell 7 estável (`pwsh`) | Scripts de build e operação |
| Visual Studio Community | 2026 ou instalação que forneça `v145` | Compilar e depurar o client C++ |
| MSVC | Toolset `v145`, ferramentas x86 e x64 | Alvo Win32/x86 do client |
| Windows SDK | Windows 10/11 SDK | Headers e bibliotecas Win32 |
| Go | Versão indicada em `wydgo748/go.mod` | Servidor e utilitários |
| PostgreSQL | 16 ou superior, incluindo `psql`, `pg_dump` e `pg_restore` | Persistência autoritativa |
| Node.js | `22.12.0` ou superior, com `npm` | Painel Astro integrado |
| Python | Python 3 x64 atual, com `venv` e `pip` | Inventários e testes de pesquisa |
| JDK | Temurin/OpenJDK 21 x64 para o ambiente Ghidra 12.1.3 preservado | Execução do Ghidra usado no projeto |
| Ghidra | Restaurar a instalação 12.1.3 e o projeto existente | Evidência nativa 7.48 |
| Serverlist Editor | Cópia externa `serverlist editor.exe`, versão 1.0.1000.1000 | Gerar o endpoint do client em `serverlist.bin` |

O client já contém headers e bibliotecas DirectX em
`tmproject/TMProject748/Dependencies/Directx`. Não instale o antigo DirectX SDK
separadamente só para este repositório. Também não são requisitos atuais:

- Docker, Railway ou outro ambiente de container/deployment;
- CMake e Ninja;
- workload de desenvolvimento de jogos do Visual Studio;
- MFC ou ATL para o client;
- sources Micronics, W2PP ou Secrets;
- atualizadores de driver de terceiros.

VS Code e Codex são opcionais. O Visual Studio continua obrigatório para o
MSBuild e o toolset C++ do client.

## Ordem recomendada depois da formatação

### 1. Windows, firmware e drivers do Dell

1. Instale o Windows 11 x64 e conclua o Windows Update.
2. Acesse a página oficial de drivers do Dell G15 5520 usando a Service Tag.
3. Atualize BIOS/firmware somente com o carregador conectado. Se o BitLocker
   estiver ativo, suspenda a proteção e confirme que possui a chave de
   recuperação. Nunca interrompa a atualização de BIOS.
4. Instale chipset, Intel Management Engine, Serial IO e Dynamic Tuning.
5. Instale o driver gráfico Intel integrado.
6. Instale o driver NVIDIA do Dell para a RTX 3050 Laptop. Essa sequência é
   importante porque o notebook usa gráficos híbridos Intel/NVIDIA.
7. Instale áudio, rede Ethernet, Wi-Fi, Bluetooth e dispositivos de entrada.
8. Instale Alienware Command Center ou Dell Power Manager somente se precisar
   dos perfis térmicos/ventoinhas. SupportAssist não é necessário para o
   desenvolvimento.
9. Reinicie e volte ao Windows Update até não haver atualizações pendentes.

Na primeira configuração, use o driver NVIDIA homologado pela Dell. Uma versão
mais nova diretamente da NVIDIA pode ser testada depois que o ambiente estiver
estável, mantendo antes um ponto de restauração. Não use utilitários genéricos
de atualização de drivers.

Verifique:

```powershell
nvidia-smi
dxdiag
Get-PnpDevice | Where-Object Status -ne 'OK'
```

O Gerenciador de Dispositivos não deve mostrar dispositivo desconhecido ou com
erro. Abra também o NVIDIA Control Panel antes de instalar as ferramentas do
projeto; isso separa um defeito de driver de um defeito do client.

### 2. Ferramentas básicas

Instale Git for Windows e PowerShell 7 pelos canais oficiais. Abra um novo
PowerShell 7 e confirme:

```powershell
git --version
pwsh --version
```

Configure a identidade do Git se necessário:

```powershell
git config --global user.name 'Felipe'
git config --global user.email 'SEU_EMAIL'
```

### 3. Visual Studio para o client

No Visual Studio Installer, marque o workload **Desktop development with C++**
e confirme estes componentes individuais:

- MSVC `v145` C++ x64/x86 build tools;
- Windows 10 ou Windows 11 SDK;
- MSBuild.

O alvo é Win32/x86, usa C++17 e C17, e a solução é
`tmproject/WYDESTINY.sln`. Não troque o toolset no projeto para contornar uma
instalação incompleta; adicione o componente `v145` pelo instalador.

### 4. Go, Node.js, Python e Java

Instale a versão Go indicada no `go.mod`, Node.js LTS compatível com o requisito
do `package.json`, Python 3 x64 e Temurin/OpenJDK 21 x64. Confirme em um novo
terminal:

```powershell
go version
node --version
npm --version
python --version
java -version
```

Para os scripts Python, crie um ambiente virtual na raiz do repositório depois
da restauração:

```powershell
Set-Location 'C:\Users\felipe\Documents\CLAUDE\SERVER 7.54\wyd-go'
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install -r `
    .\.agents\skills\wyd-client748-catalog\scripts\requirements-source-inventory.txt
```

### 5. PostgreSQL

Instale PostgreSQL 16 ou superior com as ferramentas de linha de comando.
Durante a instalação, registre com segurança a senha do usuário `postgres`.
Se `psql` não for encontrado, adicione a pasta `bin` da versão instalada ao
`PATH` ou use o caminho completo, por exemplo:

```powershell
& 'C:\Program Files\PostgreSQL\18\bin\psql.exe' --version
```

Crie um papel e um banco totalmente vazios:

```powershell
psql -U postgres -d postgres
```

```sql
CREATE ROLE wydgo LOGIN PASSWORD 'ESCOLHA_UMA_SENHA_LOCAL';
CREATE DATABASE wydgo OWNER wydgo;
\q
```

Recrie a variável no escopo do usuário sem registrar a senha no Git:

```powershell
[Environment]::SetEnvironmentVariable(
    'WYD_DATABASE_URL',
    'postgres://wydgo:SENHA@127.0.0.1:5432/wydgo?sslmode=disable',
    'User'
)
```

Feche e abra o terminal depois de mudar uma variável persistente. Antes do
primeiro boot, comprove que não há relações no banco novo:

```powershell
$env:WYD_DATABASE_URL =
    [Environment]::GetEnvironmentVariable('WYD_DATABASE_URL', 'User')
psql $env:WYD_DATABASE_URL -c `
    "SELECT tablename FROM pg_tables WHERE schemaname = 'public';"
```

O resultado deve ter zero linhas. Não execute `postgres_schema.sql` à mão: o
servidor deve aplicar o schema embutido. A porta `5432` deve permanecer privada.
Não a exponha na Internet.

### 6. Ghidra

Restaure a instalação preservada do Ghidra 12.1.3 em
`C:\Users\felipe\Tools\ghidra_12.1.3_PUBLIC` e copie de volta
`GhidraProjects` e `GhidraAnalysis` para os caminhos originais. Abra
`WYD748Native_20260821.gpr` e confirme que o programa e as análises existentes
aparecem antes de apagar a cópia externa.

Não atualize o projeto existente automaticamente para uma versão nova do
Ghidra durante a reinstalação. Faça qualquer migração futura em uma cópia. A
versão mais recente do Ghidra pode exigir outro JDK; isso não muda o baseline
preservado deste projeto.

## Restaurar o repositório

Para manter todo o trabalho local, restaure a pasta completa no caminho
original. Não copie por cima de um clone novo e não aplique o patch auxiliar
antes de verificar o backup.

```powershell
$backupRepo = 'E:\WYD-Backup-2026-09-20\wyd-go'
$destination = 'C:\Users\felipe\Documents\CLAUDE\SERVER 7.54\wyd-go'

New-Item -ItemType Directory -Path (Split-Path $destination) -Force | Out-Null
robocopy $backupRepo $destination /E /COPY:DAT /DCOPY:DAT /R:2 /W:2 /XJ
if ($LASTEXITCODE -ge 8) { throw "Falha na restauracao: $LASTEXITCODE" }

Set-Location $destination
git status --short
git rev-parse HEAD
git remote -v
```

O `status` deve corresponder ao arquivo `git-status.txt` do backup e o `HEAD`
deve corresponder a `git-head.txt`. Só depois disso faça `git fetch` ou integre
mudanças remotas. Não execute `git reset --hard`, `git checkout --` nem limpeza
do diretório para tentar deixar a árvore limpa. Não há hash anterior de
`project.exe` ou `serverlist.bin` para restaurar: ambos serão gerados novamente.

## Primeiro build e validação

Execute os gates nesta ordem. Assim, uma falha identifica melhor qual camada da
reinstalação ainda está incompleta.

### 1. Layout e ferramentas

```powershell
Set-Location 'C:\Users\felipe\Documents\CLAUDE\SERVER 7.54\wyd-go'
pwsh -NoProfile -File .\tools\repository\Test-RepositoryLayout.ps1
go version
node --version
npm --version
python --version
psql --version
```

### 2. Painel web

O `node_modules` e o `dist` são gerados; não dependem do backup:

```powershell
pwsh -NoProfile -File .\tools\web-admin\Start-WYDAdmin.ps1 -NoBrowser
```

O script usa o lockfile e deve produzir `wydgo748/web/portal/dist/`.

### 3. Servidor Go

```powershell
Push-Location .\wydgo748
New-Item -ItemType Directory -Force .\bin | Out-Null
go test -count=1 ./...
go vet ./...
go build -o .\bin\tm.exe .\cmd\server
go build -o .\bin\account-create.exe .\cmd\account-create
go build -o .\bin\account-api.exe .\cmd\account-api
Pop-Location
```

Testes que dependem de PostgreSQL somente comprovam integração quando
`WYD_TEST_POSTGRES_URL` aponta para um banco de teste separado e descartável.

### 4. Compilar e instalar o client C++

O `project.exe` é um artefato reproduzível. Gere e instale um executável novo no
runtime local com o script do repositório:

```powershell
pwsh -NoProfile -File .\tmproject\Build-Client.ps1 -Configuration Release
```

O script encontra o MSBuild pelo Visual Studio Installer, compila o alvo x86,
executa os testes de arquitetura e instala o resultado em
`tmproject/client748/project.exe`. O endpoint não é configurado pelo build.

### 5. Execução integrada

Inicie o servidor sempre com o diretório atual em `wydgo748/`:

```powershell
$env:WYD_DATABASE_URL =
    [Environment]::GetEnvironmentVariable('WYD_DATABASE_URL', 'User')
Push-Location .\wydgo748
.\bin\tm.exe
```

O boot esperado informa `persistencia autoritativa: PostgreSQL` e termina com o
listener TCP `0.0.0.0:8281`. Depois desse primeiro boot, confirme em outro
terminal que o schema foi criado pelo servidor e chegou à versão esperada:

```powershell
$env:WYD_DATABASE_URL =
    [Environment]::GetEnvironmentVariable('WYD_DATABASE_URL', 'User')
psql $env:WYD_DATABASE_URL -c `
    'SELECT MAX(version) AS schema_version FROM schema_migrations;'
psql $env:WYD_DATABASE_URL -c `
    "SELECT tablename FROM pg_tables WHERE schemaname = 'public' ORDER BY tablename;"
```

O código atual exige `schema_version = 4`. A presença das tabelas e dessa versão
prova a criação inicial; em seguida, use `bin/account-create.exe` para criar uma
conta nova e valide que ela consegue entrar no jogo. Não importe contas antigas
neste teste.

### 6. Gerar o serverlist do ambiente

Restaure o `serverlist editor.exe` preservado com as ferramentas externas e
execute-o. Configure o IP que o client deve alcançar: `127.0.0.1` quando client
e servidor estiverem no mesmo computador, ou o IP local do notebook quando o
client estiver em outra máquina da rede. O contrato de conexão do projeto usa
`8281/TCP`.

Salve a saída diretamente como:

```text
C:\Users\felipe\Documents\CLAUDE\SERVER 7.54\wyd-go\tmproject\client748\serverlist.bin
```

Não restaure nem edite manualmente um `serverlist.bin` antigo. Esse arquivo é
uma saída do editor e pode ser regenerado sempre que o endereço mudar.

Inicie o client **a partir da pasta do runtime**:

```powershell
Set-Location 'C:\Users\felipe\Documents\CLAUDE\SERVER 7.54\wyd-go\tmproject\client748'
.\project.exe
```

Não copie o executável para a área de trabalho e não o inicie de outra pasta.
O client resolve arquivos como `SkillData.bin`, `ItemList.bin`, texturas e
meshes relativamente ao diretório do runtime. Executá-lo fora de
`tmproject/client748` causa erros de leitura mesmo quando os arquivos existem.

## Checklist de aceite

- [ ] O backup externo permanece intacto e contém repositório e Ghidra; o banco
      foi intencionalmente descartado.
- [ ] `git status --short` e `HEAD` correspondem ao snapshot anterior.
- [ ] O editor externo de serverlist foi restaurado e seu hash foi conferido.
- [ ] Gerenciador de Dispositivos não mostra erros ou dispositivos desconhecidos.
- [ ] Intel Graphics, NVIDIA RTX 3050 Laptop, `nvidia-smi` e NVIDIA Control
      Panel funcionam.
- [ ] `git`, `pwsh`, `go`, `node`, `npm`, `python`, `java` e `psql` respondem.
- [ ] O painel Astro foi gerado.
- [ ] Testes, vet e builds do servidor passaram.
- [ ] O banco começou sem tabelas e o primeiro boot gerou o schema versão 4.
- [ ] Uma conta nova foi criada e persistida sem importar dados antigos.
- [ ] O client x86 foi compilado e instalado pelo `Build-Client.ps1`.
- [ ] O `serverlist.bin` foi gerado pelo editor com o IP correto.
- [ ] O servidor escuta em `8281` e o client conecta pelo endpoint gerado.
- [ ] Login, seleção, entrada no mundo e logout/relogin foram testados.
- [ ] Armaduras com tintura, ícones/equipamentos e a montaria de rena foram
      inspecionados em jogo depois da reinstalação dos drivers.

Mantenha os estados separados:

- `STATICALLY VERIFIED`: configuração, caminhos e contratos conferidos;
- `AUTOMATED TESTED`: testes e builds executados com sucesso;
- `CLIENT-TESTED`: fluxo visual/interativo executado no client real.

Um build aprovado não prova que meshes, tinturas, conexão ou interação estejam
corretos em jogo.

## Solução de problemas comum

| Sintoma | Verificação/correção |
| --- | --- |
| `SkillData.bin` ou `ItemList.bin` não pode ser lido | Inicie `project.exe` dentro de `tmproject/client748`; não de uma cópia isolada |
| `MSBuild nao encontrado` | Modifique o Visual Studio e instale Desktop C++, MSBuild e `v145`; use `-MSBuildPath` apenas se necessário |
| `PlatformToolset v145` ausente | Instale o componente `v145`; não altere o `.vcxproj` para esconder a falta |
| `psql` não reconhecido | Adicione a pasta PostgreSQL `bin` ao `PATH` ou use o caminho completo |
| `WYD_DATABASE_URL esta vazia` | Reabra o terminal e carregue a variável do escopo `User` |
| `connection refused` em `5432` | Confirme serviço PostgreSQL, host, porta, papel e banco |
| Painel web não abre | Rode `Start-WYDAdmin.ps1 -NoBrowser` ou desabilite `web_admin_enabled` |
| Caminhos `data/...` ausentes | Inicie `tm.exe` com o diretório atual em `wydgo748/` |
| Client não conecta | Verifique listener `8281`, firewall e regenere `serverlist.bin` no editor com o IP alcançável pelo client |
| NVIDIA Control Panel não abre | Reinstale primeiro os drivers Dell de chipset/Intel e depois o NVIDIA; confirme ambas as GPUs antes de testar o client |
| Cores/meshes continuam incorretas | Compare em jogo com driver Dell estável, valide arquivos/assets do runtime e só então investigue o renderer |

## Downloads oficiais

- [Drivers do Dell G15 5520](https://www.dell.com/support/product-details/pt-pt/product/g-series-15-5520-laptop/drivers)
- [Git for Windows](https://git-scm.com/downloads/win)
- [PowerShell 7 no Windows](https://learn.microsoft.com/powershell/scripting/install/install-powershell-on-windows)
- [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/)
- [Instalação das ferramentas MSVC C++](https://learn.microsoft.com/cpp/overview/acquire-msvc)
- [Go para Windows](https://go.dev/dl/)
- [PostgreSQL para Windows](https://www.postgresql.org/download/windows/)
- [Node.js](https://nodejs.org/en/download)
- [Python para Windows](https://www.python.org/downloads/windows/)
- [Eclipse Temurin/OpenJDK](https://adoptium.net/temurin/releases/)
- [Ghidra oficial e releases](https://github.com/NationalSecurityAgency/ghidra)

Depois que o ambiente estiver aprovado, consulte [Build e integração](build-and-integration.md)
para os gates normais e [Operação do servidor](server/operations.md) para banco,
contas, painel, rede, segurança, encerramento e backups recorrentes.
