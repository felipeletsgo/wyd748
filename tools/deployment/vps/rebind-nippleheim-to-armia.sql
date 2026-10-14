-- Offline maintenance only: stop both WYD services and take a fresh backup.
-- The character template stored hometown bits 3 (Nippleheim, Score.Merchant
-- bits 7:6) for every new character, so Archs and unbound characters were
-- born in the ice city. Rebind every character whose hometown is Nippleheim
-- to Armia (bits 0), preserving the remaining Merchant bits and all other
-- fields. A deliberate Nippleheim binding cannot be told apart and is reset
-- too, as the operator requested.
\set ON_ERROR_STOP on
BEGIN;
SET LOCAL lock_timeout = '5s';
LOCK TABLE public.accounts IN ACCESS EXCLUSIVE MODE;

CREATE FUNCTION pg_temp.rebind_armia(original jsonb) RETURNS jsonb
LANGUAGE plpgsql AS $$
DECLARE
    rebuilt jsonb;
BEGIN
    SELECT jsonb_agg(
        CASE WHEN jsonb_typeof(entry.value) = 'object'
            AND coalesce(entry.value->>'name', '') <> ''
            AND jsonb_typeof(entry.value#>'{score,merchant}') = 'number'
            AND ((entry.value#>>'{score,merchant}')::bigint & 192) = 192
        THEN jsonb_set(entry.value, '{score,merchant}',
            to_jsonb((entry.value#>>'{score,merchant}')::bigint & ~192::bigint))
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
        {"name":"Ice","score":{"level":5,"merchant":192},"inv":[null]},
        {"name":"IceWithLow","score":{"merchant":197}},
        {"name":"Armia","score":{"merchant":0}},
        {"name":"Erion","score":{"merchant":128}},
        {"name":"NoScore"},null]}';
    expected jsonb := '{"sentinel":42,"chars":[
        {"name":"Ice","score":{"level":5,"merchant":0},"inv":[null]},
        {"name":"IceWithLow","score":{"merchant":5}},
        {"name":"Armia","score":{"merchant":0}},
        {"name":"Erion","score":{"merchant":128}},
        {"name":"NoScore"},null]}';
BEGIN
    IF pg_temp.rebind_armia(original) <> expected THEN
        RAISE EXCEPTION 'Nippleheim rebind changed the wrong fields';
    END IF;
    IF pg_temp.rebind_armia(expected) <> expected THEN
        RAISE EXCEPTION 'Nippleheim rebind is not idempotent';
    END IF;
    IF pg_temp.rebind_armia('{"chars":[]}') <> '{"chars":[]}'::jsonb THEN
        RAISE EXCEPTION 'Nippleheim rebind changed an empty account';
    END IF;
END
$$;

CREATE TEMP TABLE armia_rebind_candidates ON COMMIT DROP AS
SELECT name_key, payload AS original, pg_temp.rebind_armia(payload) AS rebound
FROM public.accounts;

SELECT count(*) AS characters_rebound
FROM armia_rebind_candidates c
CROSS JOIN LATERAL jsonb_array_elements(c.rebound->'chars') WITH ORDINALITY AS e(value, ordinality)
WHERE e.value#>'{score,merchant}' IS DISTINCT FROM c.original#>ARRAY['chars',(e.ordinality-1)::text,'score','merchant'];

WITH updated AS (
    UPDATE public.accounts a
    SET payload = c.rebound, version = a.version + 1, updated_at = now()
    FROM armia_rebind_candidates c
    WHERE a.name_key = c.name_key AND c.rebound <> c.original
    RETURNING a.name_key
)
SELECT count(*) AS accounts_updated FROM updated;
COMMIT;
