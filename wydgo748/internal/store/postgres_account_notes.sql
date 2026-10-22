-- v11: staff case notes on the account page (DOCS/server/staff-operations.md).
-- Append-only: a correction is a new note. bug_refs holds BUG-NNN register IDs.
CREATE TABLE IF NOT EXISTS account_notes (
    id          bigserial PRIMARY KEY,
    account_key text NOT NULL REFERENCES accounts(name_key) ON DELETE CASCADE,
    actor       text NOT NULL CHECK (actor <> ''),
    body        text NOT NULL CHECK (length(body) BETWEEN 3 AND 2000),
    bug_refs    text[] NOT NULL DEFAULT '{}',
    created_at  timestamptz NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS account_notes_account_idx ON account_notes(account_key, id DESC);

INSERT INTO schema_migrations(version) VALUES (11) ON CONFLICT (version) DO NOTHING;
