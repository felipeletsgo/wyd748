-- v8: allowlisted public guild snapshot. Accounts, join dates, war internals,
-- credentials and character payloads never become privileges of the web role.
CREATE OR REPLACE VIEW public.player_public_guilds WITH (security_barrier=true) AS
SELECT CASE WHEN octet_length(payload::text) <= 8388608 THEN
    jsonb_build_object('version', payload->'version', 'guilds', COALESCE((
        SELECT jsonb_agg(jsonb_build_object(
            'id', g->'id', 'name', g->'name',
            'kingdom', COALESCE(g->'kingdom','0'::jsonb),
            'fame', COALESCE(g->'fame','0'::jsonb),
            'ally', COALESCE(g->'ally','0'::jsonb),
            'warTarget', COALESCE(g->'warTarget','0'::jsonb),
            'createdAt', g->'createdAt',
            'members', COALESCE((SELECT jsonb_agg(jsonb_build_object(
                'name', m->'character', 'rank', m->'rank'))
                FROM jsonb_array_elements(g->'members') m),'[]'::jsonb)))
        FROM jsonb_array_elements(COALESCE(NULLIF(payload->'guilds','null'::jsonb),'[]'::jsonb)) g
    ),'[]'::jsonb)) END AS projection, updated_at
FROM public.guild_state WHERE singleton=true;

INSERT INTO schema_migrations(version) VALUES (8) ON CONFLICT DO NOTHING;
