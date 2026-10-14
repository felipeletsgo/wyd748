-- Offline maintenance only: stop both WYD services and take a fresh backup.
-- Requires binaries that understand Char.capeEmblem before services restart.
-- Preserve existing symbols, real citizenship, every item and all other fields.
\set ON_ERROR_STOP on
BEGIN;
SET LOCAL lock_timeout = '5s';
LOCK TABLE public.accounts IN ACCESS EXCLUSIVE MODE;

CREATE FUNCTION pg_temp.assign_cape_emblems(original jsonb) RETURNS jsonb
LANGUAGE plpgsql AS $$
DECLARE
    rebuilt jsonb;
BEGIN
    SELECT jsonb_agg(
        CASE WHEN coalesce(entry.value->>'name', '') <> ''
            AND entry.value->'equip'->'capa'->>'index' IN (
                '543','545','734','736','1767','3191','3194','3197','3300','3303','3306',
                '544','546','735','737','1770','3192','3195','3198','3301','3304','3307')
            AND coalesce(entry.value->>'capeEmblem', '') NOT IN
                ('1','2','3','4','5','6','7','8','9','10')
        THEN jsonb_set(entry.value, '{capeEmblem}', to_jsonb(1 + floor(random()*10)::int))
        ELSE entry.value END ORDER BY entry.ordinality)
    INTO rebuilt
    FROM jsonb_array_elements(original->'chars') WITH ORDINALITY AS entry(value, ordinality);
    IF rebuilt IS NULL THEN RETURN original; END IF;
    RETURN jsonb_set(original, '{chars}', rebuilt);
END
$$;

-- Exercise the production transformation before touching any account rows.
DO $$
DECLARE
    original jsonb := '{"sentinel":42,"chars":[
        {"name":"Legacy","citizenship":1,"equip":{"capa":{"index":545,"uid":"keep"}},"inv":[null]},
        {"name":"Assigned","capeEmblem":7,"equip":{"capa":{"index":546}}},
        {"name":"Neutral","equip":{"capa":{"index":548}}},
        {"name":"NoCape","equip":{}},null]}';
    assigned jsonb;
    expected jsonb;
BEGIN
    assigned := pg_temp.assign_cape_emblems(original);
    IF (assigned#>>'{chars,0,capeEmblem}')::int NOT BETWEEN 1 AND 10 THEN
        RAISE EXCEPTION 'Cape symbol is outside the native range';
    END IF;
    expected := jsonb_set(original, '{chars,0,capeEmblem}', assigned#>'{chars,0,capeEmblem}');
    IF assigned <> expected OR pg_temp.assign_cape_emblems(assigned) <> assigned THEN
        RAISE EXCEPTION 'Cape backfill changed unrelated state or rerolled a symbol';
    END IF;
    IF pg_temp.assign_cape_emblems('{"chars":[]}') <> '{"chars":[]}'::jsonb THEN
        RAISE EXCEPTION 'Cape backfill changed an empty account';
    END IF;
END
$$;

CREATE TEMP TABLE cape_emblem_candidates ON COMMIT DROP AS
SELECT name_key, payload AS original, pg_temp.assign_cape_emblems(payload) AS assigned
FROM public.accounts;

WITH updated AS (
    UPDATE public.accounts a
    SET payload = c.assigned, version = a.version + 1, updated_at = now()
    FROM cape_emblem_candidates c
    WHERE a.name_key = c.name_key AND c.assigned <> c.original
    RETURNING a.name_key
)
SELECT count(*) AS accounts_updated FROM updated;

SELECT count(*) AS characters_assigned
FROM cape_emblem_candidates c
CROSS JOIN LATERAL jsonb_array_elements(c.assigned->'chars') WITH ORDINALITY AS e(value, ordinality)
WHERE e.value->'capeEmblem' IS DISTINCT FROM c.original#>ARRAY['chars',(e.ordinality-1)::text,'capeEmblem'];
COMMIT;
