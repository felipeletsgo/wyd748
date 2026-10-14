-- v7: narrow read model for the website. Never grant the private sidecar table.
-- Unsupported versions or malformed counters are NULL, not invented zeroes.
CREATE OR REPLACE VIEW public.player_character_progression WITH (security_barrier=true) AS
SELECT s.character_uid, c.account_key, s.updated_at,
 CASE WHEN payload->'version'='1'::jsonb AND payload->>'version'='1'
       AND (payload->'specialCoins' IS NULL OR payload->'specialCoins'='null'::jsonb
            OR jsonb_typeof(payload->'specialCoins')='object')
 THEN CASE
   WHEN payload->'specialCoins'->'fame' IS NULL OR payload->'specialCoins'->'fame'='null'::jsonb THEN 0::bigint
   WHEN jsonb_typeof(payload->'specialCoins'->'fame')='number'
        AND payload->'specialCoins'->>'fame' ~ '^[0-9]{1,10}$'
   THEN CASE WHEN (payload->'specialCoins'->>'fame')::numeric <= 4294967295
             THEN (payload->'specialCoins'->>'fame')::bigint END
 END END AS fame
FROM public.character_states s JOIN public.characters c ON c.character_uid=s.character_uid;
INSERT INTO schema_migrations(version) VALUES (7) ON CONFLICT (version) DO NOTHING;
