-- Match PostgreSQL keys to the exact names used by the runtime catalog.
BEGIN;

DO $$
DECLARE legacy_check record;
BEGIN
    FOR legacy_check IN
        SELECT conrelid::regclass AS relation_name, conname
        FROM pg_constraint
        WHERE conrelid IN ('npc_definitions'::regclass, 'npc_definition_revisions'::regclass)
          AND contype = 'c'
          AND pg_get_constraintdef(oid) LIKE '%lower(npc_name)%'
    LOOP
        EXECUTE format('ALTER TABLE %s DROP CONSTRAINT %I', legacy_check.relation_name, legacy_check.conname);
    END LOOP;
END $$;

UPDATE npc_definitions SET npc_key = npc_name;
UPDATE npc_definition_revisions SET npc_key = npc_name;

ALTER TABLE npc_definitions
    ADD CONSTRAINT npc_definitions_exact_name_check CHECK (npc_key = npc_name);
ALTER TABLE npc_definition_revisions
    ADD CONSTRAINT npc_definition_revisions_exact_name_check CHECK (npc_key = npc_name);

INSERT INTO schema_migrations(version) VALUES (14);
COMMIT;
