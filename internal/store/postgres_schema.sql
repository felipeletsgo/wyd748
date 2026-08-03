CREATE TABLE IF NOT EXISTS schema_migrations (
    version      integer PRIMARY KEY,
    applied_at   timestamptz NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS accounts (
    name_key       text PRIMARY KEY,
    payload        jsonb NOT NULL,
    version        bigint NOT NULL DEFAULT 1,
    created_at     timestamptz NOT NULL DEFAULT now(),
    updated_at     timestamptz NOT NULL DEFAULT now(),
    CHECK (name_key = lower(name_key)),
    CHECK (name_key <> '')
);

CREATE TABLE IF NOT EXISTS character_names (
    name_key      text PRIMARY KEY,
    account_key   text NOT NULL REFERENCES accounts(name_key) ON DELETE CASCADE,
    CHECK (name_key = lower(name_key)),
    CHECK (name_key <> '')
);

CREATE INDEX IF NOT EXISTS character_names_account_idx
    ON character_names(account_key);

CREATE TABLE IF NOT EXISTS characters (
    character_uid uuid PRIMARY KEY,
    account_key   text NOT NULL REFERENCES accounts(name_key) ON DELETE CASCADE,
    slot          smallint NOT NULL CHECK (slot BETWEEN 0 AND 3),
    name_key      text NOT NULL,
    evolution     text NOT NULL DEFAULT '',
    mortal_uid    uuid REFERENCES characters(character_uid) ON DELETE SET NULL,
    UNIQUE (account_key, slot),
    CHECK (name_key = lower(name_key)),
    CHECK (name_key <> '')
);

CREATE INDEX IF NOT EXISTS characters_account_idx ON characters(account_key);
CREATE INDEX IF NOT EXISTS characters_name_idx ON characters(name_key);

CREATE TABLE IF NOT EXISTS item_instances (
    uid          uuid PRIMARY KEY,
    account_key  text NOT NULL REFERENCES accounts(name_key) ON DELETE CASCADE,
    location     text NOT NULL,
    item_index   integer NOT NULL CHECK (item_index BETWEEN 1 AND 65535),
    effects      bytea NOT NULL CHECK (octet_length(effects) = 6),
    UNIQUE (account_key, location)
);

CREATE INDEX IF NOT EXISTS item_instances_account_idx
    ON item_instances(account_key);

CREATE TABLE IF NOT EXISTS guild_state (
    singleton   boolean PRIMARY KEY DEFAULT true CHECK (singleton),
    payload     jsonb NOT NULL,
    version     bigint NOT NULL DEFAULT 1,
    updated_at  timestamptz NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS instance_state (
    singleton   boolean PRIMARY KEY DEFAULT true CHECK (singleton),
    payload     jsonb NOT NULL,
    version     bigint NOT NULL DEFAULT 1,
    updated_at  timestamptz NOT NULL DEFAULT now()
);

INSERT INTO schema_migrations(version) VALUES (1)
ON CONFLICT (version) DO NOTHING;

-- v2: identidade estavel por personagem. UIDs deterministas sao usados somente
-- na migracao inicial; novos personagens recebem UUIDv4 do servidor Go.
INSERT INTO characters(character_uid,account_key,slot,name_key,evolution)
SELECT
    md5(a.name_key || ':' || (entry.ordinality-1)::text || ':' ||
        coalesce(entry.value->>'name',''))::uuid,
    a.name_key,
    entry.ordinality-1,
    lower(entry.value->>'name'),
    coalesce(entry.value->>'evolution','')
FROM accounts a
CROSS JOIN LATERAL jsonb_array_elements(a.payload->'chars')
    WITH ORDINALITY AS entry(value, ordinality)
WHERE entry.value IS NOT NULL
  AND jsonb_typeof(entry.value)='object'
  AND coalesce(entry.value->>'name','') <> ''
ON CONFLICT (account_key,slot) DO NOTHING;

-- Injeta os UIDs no payload autoritativo e converte o antigo vinculo de Arch
-- por slot para UID. A operacao e idempotente.
UPDATE accounts a
SET payload = jsonb_set(a.payload, '{chars}', rebuilt.chars)
FROM (
    SELECT a2.name_key,
           jsonb_agg(
               CASE
                   WHEN entry.value IS NULL OR jsonb_typeof(entry.value) <> 'object'
                       THEN entry.value
                   ELSE
                       (entry.value - 'archMortalSlot') ||
                       jsonb_build_object(
                           'uid', replace(c.character_uid::text,'-','')
                       ) ||
                       CASE
                           WHEN origin.character_uid IS NULL THEN '{}'::jsonb
                           ELSE jsonb_build_object(
                               'archMortalUid',
                               replace(origin.character_uid::text,'-','')
                           )
                       END
               END
               ORDER BY entry.ordinality
           ) AS chars
    FROM accounts a2
    CROSS JOIN LATERAL jsonb_array_elements(a2.payload->'chars')
        WITH ORDINALITY AS entry(value, ordinality)
    LEFT JOIN characters c
        ON c.account_key=a2.name_key AND c.slot=entry.ordinality-1
    LEFT JOIN characters origin
        ON origin.account_key=a2.name_key
       AND origin.slot=CASE
           WHEN entry.value ? 'archMortalSlot'
               THEN (entry.value->>'archMortalSlot')::integer
           ELSE -1
       END
    GROUP BY a2.name_key
) rebuilt
WHERE rebuilt.name_key=a.name_key;

-- A tabela v1 era keyed pelo nickname. Migra o estado para o primeiro
-- personagem correspondente e remove a estrutura ambigua.
DO $migration$
BEGIN
    IF EXISTS (
        SELECT 1
        FROM information_schema.columns
        WHERE table_schema='public'
          AND table_name='character_states'
          AND column_name='name_key'
    ) THEN
        ALTER TABLE character_states RENAME TO character_states_v1;
    END IF;
END
$migration$;

CREATE TABLE IF NOT EXISTS character_states (
    character_uid uuid PRIMARY KEY
        REFERENCES characters(character_uid) ON DELETE CASCADE,
    payload       jsonb NOT NULL,
    updated_at    timestamptz NOT NULL DEFAULT now()
);

DO $migration$
BEGIN
    IF to_regclass('public.character_states_v1') IS NOT NULL THEN
        EXECUTE $sql$
            INSERT INTO character_states(character_uid,payload,updated_at)
            SELECT DISTINCT ON (legacy.name_key)
                   c.character_uid,legacy.payload,legacy.updated_at
            FROM character_states_v1 legacy
            JOIN characters c ON c.name_key=legacy.name_key
            ORDER BY legacy.name_key,c.slot
            ON CONFLICT(character_uid) DO NOTHING
        $sql$;
        DROP TABLE character_states_v1;
        IF EXISTS (
            SELECT 1 FROM pg_constraint
            WHERE conrelid='character_states'::regclass
              AND contype='p' AND conname<>'character_states_pkey'
        ) THEN
            EXECUTE format(
                'ALTER TABLE character_states RENAME CONSTRAINT %I TO character_states_pkey',
                (SELECT conname FROM pg_constraint
                 WHERE conrelid='character_states'::regclass AND contype='p')
            );
        END IF;
    END IF;
END
$migration$;

INSERT INTO schema_migrations(version) VALUES (2)
ON CONFLICT (version) DO NOTHING;

INSERT INTO schema_migrations(version) VALUES (3)
ON CONFLICT (version) DO NOTHING;
