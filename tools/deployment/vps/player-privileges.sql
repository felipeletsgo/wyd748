-- Run as the database owner after the authoritative schema migration.
-- No game-state UPDATE, DELETE, DDL, or cross-table privileges are granted.
REVOKE CREATE ON SCHEMA public FROM PUBLIC;
GRANT USAGE ON SCHEMA public TO wyd_player;
GRANT SELECT, INSERT ON public.accounts TO wyd_player;

CREATE OR REPLACE FUNCTION public.wyd_check_player_registration()
RETURNS trigger LANGUAGE plpgsql SET search_path = pg_catalog AS $$
DECLARE empty_cargo jsonb;
BEGIN
    IF current_user <> 'wyd_player' THEN RETURN NEW; END IF;
    SELECT jsonb_agg(jsonb_build_object('index',0)) INTO empty_cargo
        FROM generate_series(1,128);
    IF NEW.name_key IS DISTINCT FROM lower(NEW.payload->>'name')
       OR (NEW.payload->>'name') IS NULL
       OR (NEW.payload->>'name') !~ '^[A-Za-z0-9]{4,12}$'
       OR coalesce(length(NEW.payload->>'passwordHash'),0) NOT BETWEEN 32 AND 512
       OR (NEW.payload->>'passwordHash') !~ '^pbkdf2-sha256[$]600000[$][A-Za-z0-9+/]{22}[$][A-Za-z0-9+/]{43}$'
       OR NEW.payload->'chars' IS DISTINCT FROM '[]'::jsonb
       OR NEW.payload->'cargo' IS DISTINCT FROM empty_cargo
       OR coalesce(NEW.payload->'donate','0'::jsonb) <> '0'::jsonb
       OR coalesce(NEW.payload->'cargoGold','0'::jsonb) <> '0'::jsonb
       OR coalesce(NEW.payload->'celestialCapsules','[]'::jsonb) <> '[]'::jsonb
       OR NEW.payload - ARRAY['name','passwordHash','chars','cargo','donate','cargoGold','celestialCapsules'] <> '{}'::jsonb
       OR NEW.version <> 1 THEN
        RAISE EXCEPTION 'Registration requires an empty account' USING ERRCODE='23514';
    END IF;
    RETURN NEW;
END $$;
CREATE TRIGGER wyd_player_registration BEFORE INSERT ON public.accounts
    FOR EACH ROW EXECUTE FUNCTION public.wyd_check_player_registration();
