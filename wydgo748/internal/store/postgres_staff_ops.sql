-- v9: staff operations (DOCS/server/staff-operations.md). Sanctions are an
-- append-only history: revocation stamps the row and expiry is computed from
-- expires_at, so the active state is never a separately mutable flag.
CREATE TABLE IF NOT EXISTS account_sanctions (
    id               bigserial PRIMARY KEY,
    account_key      text NOT NULL REFERENCES accounts(name_key) ON DELETE CASCADE,
    kind             text NOT NULL CHECK (kind IN ('ban', 'mute')),
    reason           text NOT NULL CHECK (reason <> ''),
    actor            text NOT NULL CHECK (actor <> ''),
    duration_minutes integer NOT NULL CHECK (duration_minutes BETWEEN 0 AND 525600),
    request_id       text NOT NULL UNIQUE,
    created_at       timestamptz NOT NULL DEFAULT now(),
    expires_at       timestamptz,
    revoked_at       timestamptz,
    revoked_by       text NOT NULL DEFAULT '',
    revoke_reason    text NOT NULL DEFAULT '',
    CHECK ((duration_minutes = 0) = (expires_at IS NULL)),
    CHECK ((revoked_at IS NULL) = (revoked_by = ''))
);

CREATE INDEX IF NOT EXISTS account_sanctions_account_idx
    ON account_sanctions(account_key, id DESC);
CREATE INDEX IF NOT EXISTS account_sanctions_live_idx
    ON account_sanctions(account_key, kind) WHERE revoked_at IS NULL;

-- Staff panel audit history. Credentials, PINs and tokens are never written.
CREATE TABLE IF NOT EXISTS staff_audit (
    id          bigserial PRIMARY KEY,
    created_at  timestamptz NOT NULL DEFAULT now(),
    actor       text NOT NULL DEFAULT '',
    action      text NOT NULL CHECK (action <> ''),
    result      text NOT NULL CHECK (result <> ''),
    request_id  text NOT NULL DEFAULT '',
    client_ip   text NOT NULL DEFAULT '',
    method      text NOT NULL DEFAULT '',
    path        text NOT NULL DEFAULT '',
    details     jsonb NOT NULL DEFAULT '{}'::jsonb
);

CREATE INDEX IF NOT EXISTS staff_audit_actor_idx ON staff_audit(actor, id DESC);
CREATE INDEX IF NOT EXISTS staff_audit_action_idx ON staff_audit(action, id DESC);

INSERT INTO schema_migrations(version) VALUES (9) ON CONFLICT (version) DO NOTHING;
