-- v10: account investigation (DOCS/server/staff-operations.md). One row per
-- admitted game login, kept 90 days by the staff panel writer.
CREATE TABLE IF NOT EXISTS account_logins (
    id          bigserial PRIMARY KEY,
    account_key text NOT NULL REFERENCES accounts(name_key) ON DELETE CASCADE,
    client_ip   text NOT NULL DEFAULT '',
    channel     smallint NOT NULL DEFAULT 1,
    created_at  timestamptz NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS account_logins_account_idx ON account_logins(account_key, id DESC);

-- Staff actions name their target account under one of two detail keys.
CREATE INDEX IF NOT EXISTS staff_audit_target_idx
    ON staff_audit ((COALESCE(details->>'target_account', details->>'account')), id DESC);

INSERT INTO schema_migrations(version) VALUES (10) ON CONFLICT (version) DO NOTHING;
