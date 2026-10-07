import type { components } from '../lib/api.generated';
type Catalog = components['schemas']['PremiumCatalog'];
type CatalogItem = components['schemas']['PremiumCatalogItem'];
type CatalogCommand = components['schemas']['PremiumCatalogCommand'];
type CatalogResult = components['schemas']['PremiumCatalogResult'];
type Account = components['schemas']['DonateAccount'];
type CreditCommand = components['schemas']['DonateCreditCommand'];
type CreditResult = components['schemas']['DonateCreditResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

export function setupPremiumShop(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const text = (id: string, value: string) => { el(id).textContent = value; };
  let catalog: Catalog | undefined;
  let rows: CatalogItem[] = [];
  let account: Account | undefined;
  let pendingCredit: CreditCommand | undefined;
  let busy = false;
  let generation = 0;
  const catalogMessages: Record<string, string> = {
    ok: 'Catalog saved. Players see it the next time they open the shop.',
    stale_catalog: 'Another staff member changed the catalog. The current version was loaded; review and save again.',
    unknown_item: 'One of the item IDs does not exist in the server item list.',
    invalid_command: 'Invalid catalog. Check cells (0–26, no repeats), item IDs, prices and the reason.',
    capability_required: 'This session no longer has permission to edit the catalog.',
    unavailable: 'The premium shop requires the server with PostgreSQL.',
  };
  const creditMessages: Record<string, string> = {
    ok: 'Credit stored. The game server applies it once.',
    unknown_account: 'Account not found.',
    request_conflict: 'This request identifier was already used with different values. Start a new credit.',
    invalid_command: 'Invalid credit. Check the amount and the reason.',
    capability_required: 'This session no longer has permission to credit donate.',
    unavailable: 'Donate requires the server with PostgreSQL.',
  };
  function buttons() {
    el<HTMLButtonElement>('premium-add').disabled = busy || !catalog?.available || rows.length >= 27;
    el<HTMLButtonElement>('premium-save').disabled = busy || !catalog?.available;
    el<HTMLButtonElement>('premium-refresh').disabled = busy;
    el<HTMLFieldSetElement>('donate-fields').disabled = busy || !account || !!pendingCredit;
    el<HTMLButtonElement>('donate-retry').hidden = !pendingCredit;
    el<HTMLButtonElement>('donate-retry').disabled = busy;
  }
  function cell(content: HTMLElement) { const td = document.createElement('td'); td.append(content); return td; }
  function input(value: string | number, onChange: (value: string) => void, attrs: Record<string, string>) {
    const field = document.createElement('input');
    field.value = String(value);
    for (const [key, attr] of Object.entries(attrs)) field.setAttribute(key, attr);
    field.addEventListener('change', () => onChange(field.value));
    return field;
  }
  function renderRows() {
    const body = el('premium-rows');
    body.replaceChildren();
    if (!rows.length) {
      const tr = document.createElement('tr'); const td = document.createElement('td');
      td.colSpan = 6; td.className = 'empty'; td.textContent = catalog?.available ? 'Empty catalog. Add items to sell.' : 'Premium shop unavailable on this server.';
      tr.append(td); body.append(tr); return;
    }
    rows.forEach((row, index) => {
      const tr = document.createElement('tr');
      tr.append(cell(input(row.slot, v => { row.slot = Number(v); }, { type: 'number', min: '0', max: '26', 'aria-label': 'Cell' })));
      tr.append(cell(input(row.itemId, v => { row.itemId = Number(v); row.itemName = ''; }, { type: 'number', min: '1', max: '6499', 'aria-label': 'Item ID' })));
      const name = document.createElement('span'); name.textContent = row.itemName || '—'; tr.append(cell(name));
      tr.append(cell(input(row.effects.join(','), v => {
        const parts = v.split(',').map(part => Number(part.trim()));
        row.effects = parts.length === 6 && parts.every(n => Number.isInteger(n) && n >= 0 && n <= 255) ? parts : row.effects;
      }, { 'aria-label': 'Effects', pattern: '^\\s*\\d{1,3}(\\s*,\\s*\\d{1,3}){5}\\s*$' })));
      tr.append(cell(input(row.price, v => { row.price = Number(v); }, { type: 'number', min: '1', max: '2000000000', 'aria-label': 'Price' })));
      const remove = document.createElement('button'); remove.type = 'button'; remove.className = 'button subtle'; remove.textContent = 'Remove';
      remove.addEventListener('click', () => { rows.splice(index, 1); renderRows(); buttons(); });
      tr.append(cell(remove));
      body.append(tr);
    });
  }
  function renderCatalog(value: Catalog) {
    catalog = value;
    rows = value.items.map(item => ({ ...item, effects: [...item.effects] }));
    text('premium-state', value.available
      ? `Catalog version ${value.catalogVersion}${value.updatedBy ? ` · saved by ${value.updatedBy} on ${new Date(value.updatedAt).toLocaleString('en-US')}` : ''}.`
      : 'The premium shop requires the server with PostgreSQL.');
    renderRows(); buttons();
  }
  async function refresh() {
    if (busy) return;
    busy = true; buttons();
    const current = generation;
    try {
      const value = await api<Catalog>('/staff/premium-shop');
      if (current !== generation) return;
      if (value.version !== 1) throw new Error('Invalid catalog');
      renderCatalog(value);
      text('premium-message', '');
    } catch (error) {
      if (current !== generation) return;
      text('premium-message', explain(error));
    } finally { if (current === generation) { busy = false; buttons(); } }
  }
  async function save() {
    if (busy || !catalog) return;
    const reason = el<HTMLInputElement>('premium-reason');
    if (!reason.reportValidity()) return;
    const command: CatalogCommand = { version: 1, expectedCatalogVersion: catalog.catalogVersion, reason: reason.value.trim(),
      items: rows.map(({ slot, itemId, effects, price }) => ({ slot, itemId, effects, price })) };
    if (!window.confirm(`Save ${command.items.length} item(s) to the premium catalog?\nReason: ${command.reason}`)) return;
    busy = true; buttons();
    const current = generation;
    try {
      const result = await api<CatalogResult>('/staff/premium-shop', 'PUT', command);
      if (current !== generation) return;
      if (result.code === 'ok' || result.code === 'stale_catalog') renderCatalog(result.catalog);
      text('premium-message', catalogMessages[result.code] ?? 'Unexpected result.');
    } catch (error) {
      if (current !== generation) return;
      text('premium-message', `${explain(error)} Refresh the catalog to check whether it was saved.`);
    } finally { if (current === generation) { busy = false; buttons(); } }
  }
  function renderAccount(value: Account) {
    account = value;
    text('donate-name', value.account);
    text('donate-balance', value.balance.toLocaleString('en-US'));
    text('donate-pending', value.pending.length ? value.pending.map(p => `+${p.amount.toLocaleString('en-US')} (${p.actor || p.source})`).join(', ') : 'None');
    const body = el('donate-ledger');
    body.replaceChildren();
    if (!value.ledger.length) {
      const tr = document.createElement('tr'); const td = document.createElement('td');
      td.colSpan = 5; td.className = 'empty'; td.textContent = 'No balance changes yet.'; tr.append(td); body.append(tr);
    }
    for (const entry of value.ledger) {
      const tr = document.createElement('tr');
      const kind = { credit: 'Credit', purchase: 'Purchase', coin_use: 'Wyden coin' }[entry.kind] ?? entry.kind;
      for (const content of [new Date(entry.createdAt).toLocaleString('en-US'), kind,
        `${entry.delta > 0 ? '+' : ''}${entry.delta.toLocaleString('en-US')}`, entry.balanceAfter.toLocaleString('en-US'),
        [entry.itemId ? `${entry.itemId}${entry.itemName ? ` ${entry.itemName}` : ''}` : '', entry.character].filter(Boolean).join(' · ') || '—']) {
        const td = document.createElement('td'); td.textContent = content; tr.append(td);
      }
      body.append(tr);
    }
    buttons();
  }
  async function lookup(name: string) {
    if (busy) return;
    busy = true; buttons();
    const current = generation;
    try {
      const value = await api<Account>(`/staff/accounts/${encodeURIComponent(name)}/donate`);
      if (current !== generation) return;
      renderAccount(value);
      text('donate-message', '');
    } catch (error) {
      if (current !== generation) return;
      account = undefined;
      text('donate-message', explain(error));
    } finally { if (current === generation) { busy = false; buttons(); } }
  }
  async function sendCredit() {
    if (!pendingCredit || busy) return;
    const command = pendingCredit;
    busy = true; buttons();
    const current = generation;
    text('donate-message', 'Waiting for server confirmation…');
    try {
      const result = await api<CreditResult>(`/staff/accounts/${command.account}/donate`, 'POST', command);
      if (current !== generation) return;
      pendingCredit = undefined;
      text('donate-message', `${creditMessages[result.code] ?? 'Unexpected result.'}${result.replayed ? ' The credit was already stored; it was not repeated.' : ''}`);
      busy = false;
      if (result.code === 'ok') await lookup(command.account);
    } catch (error) {
      if (current !== generation) return;
      text('donate-message', `${explain(error)} Outcome not confirmed. Use the retry button, which keeps the same identifier.`);
    } finally { if (current === generation) { busy = false; buttons(); } }
  }
  el('premium-add').addEventListener('click', () => {
    const used = new Set(rows.map(row => row.slot));
    let slot = 0; while (used.has(slot) && slot < 26) slot++;
    rows.push({ slot, itemId: 3394, effects: [0, 0, 0, 0, 0, 0], price: 1000 });
    renderRows(); buttons();
  });
  el('premium-refresh').addEventListener('click', () => void refresh());
  el<HTMLFormElement>('premium-form').addEventListener('submit', event => { event.preventDefault(); void save(); });
  el<HTMLFormElement>('donate-lookup').addEventListener('submit', event => {
    event.preventDefault();
    const field = el<HTMLInputElement>('donate-account');
    if (field.reportValidity()) void lookup(field.value.trim().toLowerCase());
  });
  el<HTMLFormElement>('donate-form').addEventListener('submit', event => {
    event.preventDefault();
    if (!account || pendingCredit) return;
    const amount = el<HTMLInputElement>('donate-amount');
    const reason = el<HTMLInputElement>('donate-reason');
    if (!amount.reportValidity() || !reason.reportValidity()) return;
    const command: CreditCommand = { version: 1, requestId: crypto.randomUUID(), account: account.account,
      amount: Number(amount.value), reason: reason.value.trim() };
    if (!window.confirm(`Credit ${command.amount.toLocaleString('en-US')} donate to ${command.account}?\nReason: ${command.reason}`)) return;
    pendingCredit = command; void sendCredit();
  });
  el('donate-retry').addEventListener('click', () => void sendCredit());
  return { refresh, reset() {
    generation++; catalog = undefined; rows = []; account = undefined; pendingCredit = undefined; busy = false;
    text('premium-state', 'Waiting for a query.'); text('premium-message', ''); text('donate-message', '');
    for (const id of ['donate-name', 'donate-balance', 'donate-pending']) text(id, '—');
    renderRows(); buttons();
  } };
}
