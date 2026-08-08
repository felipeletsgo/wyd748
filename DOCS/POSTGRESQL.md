# PostgreSQL autoritativo

Atualizado em 26/07/2026.

O servidor de produção usa PostgreSQL. O JSON continua disponível somente com
`database_driver=json`, para desenvolvimento isolado e importadores. Não existe
fallback automático: se o PostgreSQL estiver configurado e indisponível, o
servidor interrompe o boot em vez de abrir com um estado diferente.

## Modelo

- `accounts`: ficha completa da conta em `jsonb`, hash da senha e versão.
- `character_names`: índice global case-insensitive dos nicknames.
- `characters`: identidade estável, conta, slot e vínculo Mortal→Arch por UID.
- `item_instances`: uma linha por item materializado.
- `character_states`: buffs e contadores, FK por UID com `ON DELETE CASCADE`.
- `guild_state`: registro canônico de guilds.
- `schema_migrations`: versão do schema aplicado.

`item_instances.uid` é uma chave `uuid` global. A restrição
`UNIQUE(account_key, location)` impede duas instâncias no mesmo slot. A conta,
os nomes, os itens e, quando aplicável, a guild são gravados na mesma transação
`SERIALIZABLE`. Uma colisão, nickname duplicado ou UID duplicado desfaz toda a
operação.

Os UIDs não vão para o client 7.48. O `STRUCT_ITEM` continua com seus oito bytes
nativos (`Index` e seis bytes de efeitos). O servidor valida esses bytes no
pacote e conserva a identidade da cópia autoritativa.

O autosave atualiza o JSONB da ficha, mas reconcilia `item_instances`
incrementalmente: itens inalterados não são apagados/reinseridos. Quando a fila
fica cheia, o produtor não bloqueia o World; snapshots pendentes são coalescidos
por conta e por UID de personagem. Crafts que gastam fama/contadores usam
`SavePlayerState`, que confirma conta e charstate na mesma transação.

Falha de um lote de autosave é contabilizada e logada; o snapshot autoritativo
seguinte, emitido pelo World em até três segundos, é o retry e substitui o
anterior. Saves anti-dupe continuam síncronos e devolvem erro ao handler.
`Guilds.txt` é derivado: falha de exportação usa retry com backoff, sem falsear
o commit PostgreSQL já concluído.

## Instalação no Ubuntu

```bash
sudo apt update
sudo apt install -y postgresql
sudo -u postgres psql
```

No console do PostgreSQL:

```sql
CREATE ROLE wydgo LOGIN PASSWORD 'troque-esta-senha';
CREATE DATABASE wydgo OWNER wydgo;
\q
```

Defina a URL somente no ambiente do processo:

```bash
export WYD_DATABASE_URL='postgres://wydgo:troque-esta-senha@127.0.0.1:5432/wydgo?sslmode=disable'
go build -o tm ./cmd/server
./tm
```

O servidor aplica o schema v2 no boot sob lock exclusivo. O PostgreSQL deve escutar apenas
localmente; não publique a porta 5432 na internet.

Para o criador de contas:

```bash
go build -o account-create ./cmd/account-create
./account-create
```

O mesmo `WYD_DATABASE_URL` é usado pelo servidor, pela API e pelo criador de
contas. A senha do jogador nunca deve ser passada como argumento de linha de
comando.

## Desenvolvimento e testes

Os testes comuns não exigem banco. Para executar também o teste de integração:

```bash
export WYD_TEST_POSTGRES_URL='postgres://wydgo:senha@127.0.0.1:5432/wydgo_test?sslmode=disable'
go test ./internal/store -run PostgresStoreIntegration -v
```

Use um banco separado para testes. Os testes criam nomes exclusivos, confirmam
round-trip, isolamento de Mortal/Arch homônimos, cascata do charstate,
persistência incremental e rollback de conta+charstate.

## Backup

```bash
pg_dump --format=custom --file=wydgo.dump "$WYD_DATABASE_URL"
pg_restore --clean --if-exists --dbname="$WYD_DATABASE_URL" wydgo.dump
```

Faça backup antes de alterações de schema. Restaurar o banco preserva os UIDs;
reemitir UIDs durante o restore destruiria a trilha de identidade dos itens.
