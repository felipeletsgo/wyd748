-- Active NPC templates are keyed by channel; inactive source templates stay in files.
CREATE TABLE IF NOT EXISTS npc_definitions (
    channel_id      smallint NOT NULL CHECK (channel_id BETWEEN 1 AND 255),
    npc_key         text NOT NULL CHECK (length(npc_key) BETWEEN 1 AND 64),
    npc_name        text NOT NULL CHECK (length(npc_name) BETWEEN 1 AND 64),
    source_json     jsonb NOT NULL CHECK (jsonb_typeof(source_json) = 'object'),
    definition_json jsonb NOT NULL CHECK (jsonb_typeof(definition_json) = 'object'),
    source_hash     text NOT NULL CHECK (length(source_hash) = 64),
    active          boolean NOT NULL DEFAULT true,
    version         bigint NOT NULL CHECK (version BETWEEN 1 AND 4294967295),
    overridden      boolean NOT NULL DEFAULT false,
    updated_by      text NOT NULL DEFAULT '',
    updated_at      timestamptz NOT NULL DEFAULT now(),
    reason          text NOT NULL DEFAULT '',
    PRIMARY KEY (channel_id, npc_key),
    CHECK (npc_key = lower(npc_name)),
    CHECK (overridden OR definition_json = source_json)
);

CREATE INDEX IF NOT EXISTS npc_definitions_active_name_idx
    ON npc_definitions(channel_id, npc_name) WHERE active;

CREATE TABLE IF NOT EXISTS npc_definition_revisions (
    id              bigserial PRIMARY KEY,
    channel_id      smallint NOT NULL CHECK (channel_id BETWEEN 1 AND 255),
    npc_key         text NOT NULL,
    npc_name        text NOT NULL,
    version         bigint NOT NULL CHECK (version BETWEEN 2 AND 4294967295),
    definition_json jsonb NOT NULL CHECK (jsonb_typeof(definition_json) = 'object'),
    actor           text NOT NULL CHECK (actor <> ''),
    reason          text NOT NULL CHECK (reason <> ''),
    restored_source boolean NOT NULL DEFAULT false,
    created_at      timestamptz NOT NULL DEFAULT now(),
    CHECK (npc_key = lower(npc_name))
);

CREATE INDEX IF NOT EXISTS npc_definition_revisions_channel_id_idx
    ON npc_definition_revisions(channel_id, id);

INSERT INTO schema_migrations(version) VALUES (13)
ON CONFLICT (version) DO NOTHING;
