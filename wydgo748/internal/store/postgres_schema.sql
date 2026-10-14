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
    uid            uuid PRIMARY KEY,
    account_key    text NOT NULL REFERENCES accounts(name_key) ON DELETE CASCADE,
    location       text NOT NULL,
    item_index     integer NOT NULL CHECK (item_index BETWEEN 1 AND 65535),
    effects        bytea NOT NULL CHECK (octet_length(effects) = 6),
    activated_unix bigint NOT NULL DEFAULT 0 CHECK (activated_unix >= 0),
    expires_unix   bigint NOT NULL DEFAULT 0 CHECK (expires_unix >= 0),
    CHECK (
        (activated_unix = 0 AND expires_unix = 0) OR
        (activated_unix > 0 AND expires_unix >= activated_unix)
    ),
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

-- v2: stable identity per character. Deterministic UIDs are used only by the
-- initial migration; new characters get a UUIDv4 from the Go server.
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

-- Injects the UIDs into the authoritative payload and converts the old
-- slot-based Arch link to a UID. The operation is idempotent.
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

-- The v1 table was keyed by nickname. Moves the state to the first matching
-- character and removes the ambiguous structure.
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

ALTER TABLE item_instances
    ADD COLUMN IF NOT EXISTS activated_unix bigint NOT NULL DEFAULT 0,
    ADD COLUMN IF NOT EXISTS expires_unix bigint NOT NULL DEFAULT 0;

DO $migration$
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM pg_constraint
        WHERE conrelid='item_instances'::regclass
          AND conname='item_instances_activated_unix_check'
    ) THEN
        ALTER TABLE item_instances
            ADD CONSTRAINT item_instances_activated_unix_check CHECK (activated_unix >= 0);
    END IF;
    IF NOT EXISTS (
        SELECT 1 FROM pg_constraint
        WHERE conrelid='item_instances'::regclass
          AND conname='item_instances_expires_unix_check'
    ) THEN
        ALTER TABLE item_instances
            ADD CONSTRAINT item_instances_expires_unix_check CHECK (expires_unix >= 0);
    END IF;
    IF NOT EXISTS (
        SELECT 1 FROM pg_constraint
        WHERE conrelid='item_instances'::regclass
          AND conname='item_instances_timed_window_check'
    ) THEN
        ALTER TABLE item_instances
            ADD CONSTRAINT item_instances_timed_window_check CHECK (
                (activated_unix = 0 AND expires_unix = 0) OR
                (activated_unix > 0 AND expires_unix >= activated_unix)
            );
    END IF;
END
$migration$;

INSERT INTO schema_migrations(version) VALUES (4)
ON CONFLICT (version) DO NOTHING;

-- v5: premium shop (DOCS/protocol/premium-shop.md). The donate balance lives in
-- the account payload; these tables hold the staff catalog, the credit inbox
-- written by the staff panel or the web platform, and the balance ledger.
CREATE TABLE IF NOT EXISTS premium_shop (
    singleton   boolean PRIMARY KEY DEFAULT true CHECK (singleton),
    version     bigint NOT NULL CHECK (version > 0),
    entries     jsonb NOT NULL,
    updated_by  text NOT NULL DEFAULT '',
    updated_at  timestamptz NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS donate_credits (
    id          bigserial PRIMARY KEY,
    account_key text NOT NULL REFERENCES accounts(name_key) ON DELETE CASCADE,
    amount      integer NOT NULL CHECK (amount > 0),
    reason      text NOT NULL CHECK (reason <> ''),
    source      text NOT NULL CHECK (source IN ('staff', 'web')),
    actor       text NOT NULL DEFAULT '',
    request_id  text UNIQUE,
    created_at  timestamptz NOT NULL DEFAULT now(),
    applied_at  timestamptz
);

CREATE INDEX IF NOT EXISTS donate_credits_pending_idx
    ON donate_credits(account_key) WHERE applied_at IS NULL;

CREATE TABLE IF NOT EXISTS donate_ledger (
    id            bigserial PRIMARY KEY,
    account_key   text NOT NULL REFERENCES accounts(name_key) ON DELETE CASCADE,
    delta         bigint NOT NULL CHECK (delta <> 0),
    balance_after bigint NOT NULL CHECK (balance_after >= 0),
    kind          text NOT NULL CHECK (kind IN ('credit', 'purchase', 'coin_use')),
    item_index    integer NOT NULL DEFAULT 0,
    item_uid      text NOT NULL DEFAULT '',
    credit_id     bigint REFERENCES donate_credits(id),
    character     text NOT NULL DEFAULT '',
    created_at    timestamptz NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS donate_ledger_account_idx
    ON donate_ledger(account_key, id DESC);

INSERT INTO schema_migrations(version) VALUES (5)
ON CONFLICT (version) DO NOTHING;

-- Security-only counters. No passwords, tokens, raw addresses or account names.
CREATE TABLE IF NOT EXISTS web_auth_limits (
 key text PRIMARY KEY CHECK (length(key) BETWEEN 1 AND 160),
 attempts integer NOT NULL CHECK (attempts > 0),
 expires_at timestamptz NOT NULL
);
CREATE INDEX IF NOT EXISTS web_auth_limits_expiry_idx ON web_auth_limits(expires_at);
INSERT INTO schema_migrations(version) VALUES (6) ON CONFLICT (version) DO NOTHING;
