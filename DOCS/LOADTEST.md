# Teste de carga do WYD-Go 7.48

O relatorio JSON tambem inclui `BytesSent` e `BytesReceived`, alem dos
contadores de conexao, entrada no mundo, combate, movimento e pacotes.

## Modelo em uma base separada

Inicie o binario de teste com o arquivo dedicado:

```powershell
.\tm.exe -config=data/server-loadtest.txt
```

Se a base de carga ainda nao contiver `felipe`, a conta modelo pode vir de
outra base PostgreSQL isolada (por exemplo, um dump restaurado da base de
staging):

```powershell
$env:WYD_SOURCE_DATABASE_URL = "postgres://.../wydgo_staging"
go run ./cmd/loadtest -provision -provision-only `
  -source-database-url-env=WYD_SOURCE_DATABASE_URL `
  -reset -database-url-env=WYD_LOADTEST_DATABASE_URL
```

Nesse fluxo `felipe` e materializado uma vez no destino e preservado nas
rodadas seguintes; a ferramenta nunca sobrescreve uma conta existente. A URL
da origem deve usar uma credencial de leitura e a base de origem nunca deve
ser usada para iniciar o servidor de carga.

O teste usa uma base PostgreSQL separada, `wydgo_loadtest`, e nunca deve ser
executado contra a base de produção. A conta `felipe` é somente o modelo e o
cliente de observação.

## Preparação

1. Crie a base `wydgo_loadtest` com o mesmo usuário PostgreSQL do ambiente de
   staging.
2. Defina `WYD_LOADTEST_DATABASE_URL` e `WYD_LOADTEST_BOT_PASSWORD` no ambiente.
3. Inicie o servidor com `data/server-loadtest.txt`.

O arquivo ativa `loadtest_spawn=1162,1700` e
`loadtest_account_prefix=bot`. Somente contas `bot...` recebem esse nascimento;
jogadores reais continuam nascendo em `(2100,2100)`. Os pontos `(1162,1700)` e
`(1163,1701)` pertencem aos geradores Tauron 3223/3224 do `NPCGener.txt`.

## Provisionamento

Na pasta `wyd-go`, o comando abaixo cria `bot0001` até `bot0990`:

```powershell
go run ./cmd/loadtest -provision -provision-only `
  -reset -database-url-env=WYD_LOADTEST_DATABASE_URL
```

`-reset` remove apenas contas com o formato `bot` seguido de quatro dígitos.
Por padrão, o modelo é o personagem do slot `1` da conta `felipe` (o segundo
`Felipe`, Arch). Isso evita escolher o Mortal quando os dois usam o mesmo
nickname. Use `-source-slot=0` para o primeiro personagem ou `-source-slot=-1`
para voltar à seleção legada; `-source-character` continua disponível como
validação adicional do nickname. O provisionador copia ExtendedScore,
equipamento, inventário, habilidades e mastery, mas gera novos CharacterUIDs
e ItemUIDs. Guild, party, cartas, loja e vínculos do Mortal original não são
clonados.

Nicknames são somente alfabéticos (`BOTAAAAA`, `BOTAAAAB`, ...), enquanto as
contas usam o formato curto `bot0001`, compatível com o limite do protocolo.

## Execução

Com `felipe` conectado para observar a percepção de lag:

```powershell
go run ./cmd/loadtest `
  -server=127.0.0.1:8281 `
  -bots=990 `
  -ramp=1m `
  -duration=10m `
  -database-url-env=WYD_LOADTEST_DATABASE_URL
```

O padrão usa ataque físico. Para testar uma skill, use `-skill=N`; o servidor
continua validando classe, aprendizagem, mastery, alcance, cooldown e mana.

Os bots entram no deserto de Noatum, observam os Taurons e, a cada
aproximadamente 900 ms, tentam se mover conforme `-move-percent` e também
enviam o ataque; movimento não substitui mais combate. O padrão é 50%, e
`-move-percent=0` deixa os bots estacionados sem desativar os ataques.
O modo distribuído usa os pontos Tauron próximos; para pior caso de visibilidade,
execute uma rodada concentrada com menos bots na mesma área.

## Métricas

Com `debug_address` habilitado, consultar por túnel local:

```text
http://127.0.0.1:6060/debug/vars
http://127.0.0.1:6060/debug/pprof/
```

Observar jogadores conectados/entrados, duração e atraso do tick, tamanho e
idade da fila de comandos, lotes que excederam o orçamento, latência do
PostgreSQL, CPU, memória, goroutines, desconexões e panics.

O resultado final do comando é JSON com `Started`, `Connected`, `Entered`,
`Errors`, `Attacks`, `Moves`, `Packets` e `Elapsed`.

## Critérios

- 990 bots e o jogador real entram sem sobrescrever IDs;
- a fila não cresce continuamente;
- movimento, combate, skills, HP/MP e visibilidade continuam respondendo;
- nenhum panic ou corrupção de estado ocorre;
- a base de teste pode ser removida sem afetar `felipe`.

## Perfil de 990 sessoes

O arquivo `data/server-loadtest.txt` usa `session_queue_capacity=4096` para
absorver a rajada de entidades enviada na entrada de centenas de bots. O
padrao de producao continua em 256; essa folga maior e exclusiva do staging.

Para medir somente combate, use `-move-percent=0`. Para uma carga de combate
com deslocamento, mantenha o padrão de 50% ou use qualquer valor entre 1 e
100 depois de validar a rota do mapa.
