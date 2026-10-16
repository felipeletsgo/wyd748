# Website feature status

Reconciled against source and the deployment ledger on 2026-10-13. This is the
current implementation matrix for the dated [WYD Web Platform plan](../WYD-WEB-PLATAFORM.md),
not a claim that every proposed feature or browser acceptance test is complete.
Deployment identities and validation limits are in [current state](../current-state.md).

## Implemented surfaces

| Feature | Current implementation | Boundary or remaining work |
| --- | --- | --- |
| Public navigation/theme | Home, News, Account, Ranking, Guilds, Download; shared modern 7.48-inspired theme | Play and Database are hidden from navigation; their guide/catalogue URLs remain |
| Registration | Same-origin account creation with Turnstile and durable quotas | Five network attempts/hour is not a verified five-account-per-person limit |
| Player authentication | Secure session cookie, login/logout, profile, session listing and revoke/revoke-all | Session controls do not replace in-game authority |
| Account characters | Private saved character summaries, attributes, equipment appearance, kingdom, CP, Fame, Gold and experience | Owned slot selectors only; saved/cached state, not a live World mirror |
| Inventory and Cargo | Private read-only snapshots with item presentation | No website item mutation, withdrawal or trading |
| Progression advice | Saved-state Arch, Celestial and SubCelestial advice | Advisory only; no web ascension or guaranteed live eligibility |
| Ranking | Persisted character ranking and supported filters | No separate competitive/event ladder projections |
| Guilds | Searchable name directory and public saved profiles: kingdom, Fame, members/roles, allies and war target; signed-in guild section on `/player` where the leader uploads or removes the guild mark | No member administration or war-history UI; nonempty production rendering remains manual |
| Downloads and updates | External full-client link, launcher patches, changelog and manifest advertising 10005 | Full package size/hash receipt and actual launcher extraction remain acceptance gaps |
| News and roadmap | Versioned static content and detail pages | No editable CMS or staff publishing workflow |
| Game reference content | Read-only item, skill and quest catalogues with detail URLs | Catalogue presence is not proof that every listed game mechanic is implemented |
| Staff panel | Authenticated, capability-gated account lookup/actions, audit and existing game controls | Six-digit PIN supplements staff authorization; it is not an anonymous write credential |

The current public service is `cmd/player-api`, behind Caddy with a loopback
listener, and the integrated staff panel remains on the game process's private
HTTP listener. Do not deploy `cmd/account-api` as an equivalent replacement.
OpenAPI and generated types are the HTTP contract, not the historical product
plan. Current API version is `0.12.0`; response contracts use version `1` and
the database uses schema `8`.

## Security policy

Configured registration admission limits are 30 global attempts/minute,
300/hour and 1,000/day; per network, five/hour and ten/day. Network identity
uses IPv4 or IPv6 `/64`. Failed attempts also consume admission, and NAT users
share a network quota. Do not market this as enforcement of five accounts per
human; no verified human identity/grouping contract exists.

Player login hourly limits are 600 globally, 100 per network and 40 per account,
with atomic durable admission. Turnstile validates the configured hostname and
separate `register`, `login` and `staff_login` actions; unavailable verification
fails closed. Exact Host/Origin, CSRF, bounded password work and secure cookies
remain required. No secret key is reproduced here.

The game PostgreSQL role and player API role have different duties. Public
guild reads use an allowlisted security-barrier projection; private character
reads check account ownership and do not expose internal UIDs. Anonymous private
reads remain rejected. This describes implemented controls, not a penetration
test or a guarantee against SQL injection, credential stuffing or DDoS.

## Outstanding product-plan features

- Event calendar/registration/results and dedicated competitive rankings.
- Web marketplace, payments, cash/shop or website-authorized item transfers.
- Support-ticket lifecycle and staff assignment.
- Editable news/CMS publication and moderation workflow.
- War history, membership/leadership web actions.
- Live world/map views or guaranteed live presence snapshots.

These are not enabled by adding routes alone. Each needs ownership, permissions,
authoritative commands, failures, persistence and acceptance criteria before
implementation. Cosmetic changes must not create an alternate write path that
bypasses the game actor or S2 persistence boundary.

The [public-launch ledger](public-launch.md) retains build/deployment evidence
for individual batches. Later guild, storage and progression entries supersede
earlier statements that those features are absent. Do not rewrite old receipts
as if they described the latest release.

## Guild marks

The guild leader manages the mark in the signed-in player panel (`/player`,
section Guild). `POST /api/player/v1/guild/mark` accepts a ready-to-use 16x12,
24-bit BMP of 630/632 bytes unchanged, or converts a PNG, JPEG, GIF or BMP up
to 64 KB. The player API (`-guildmark-dir /var/lib/wyd-go-guildmark`) stores
`<guild:04>.bmp`; Caddy serves every client request
`/patch/guildmark/b<group:02><channel:02><guild:04>.bmp` from that file. The
client downloads marks off the render thread from
`http://wyd.forjago.com/patch/guildmark/`, creates textures on the render
thread, shows the mark next to the own character (score update) and other
characters (CreateMob), and retries a missing mark after five minutes.
Leadership is resolved from the account's characters in the saved guild
projection; only the leader (rank 9) may upload or remove.
