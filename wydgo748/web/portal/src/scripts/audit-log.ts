import type { components } from '../lib/api.generated';
type Page = components['schemas']['AuditPage'];
type Entry = components['schemas']['AuditEntry'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

// Bounded one-line summary; values are rendered as text, never as HTML.
export function auditSummary(entry: Entry) {
  const parts = Object.entries(entry.details ?? {}).map(([key, value]) => `${key}=${typeof value === 'string' ? value : JSON.stringify(value)}`);
  const summary = parts.join(' · ');
  return summary.length > 240 ? `${summary.slice(0, 239)}…` : summary;
}

export function setupAuditLog(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const text = (id: string, value: string) => { el(id).textContent = value; };
  const date = (value: string) => new Date(value).toLocaleString('en-US', { dateStyle: 'short', timeStyle: 'medium' });
  let page: Page | undefined;
  let filters = { actor: '', action: '' };
  let before = 0;
  let busy = false;
  let generation = 0;

  function controls() {
    el<HTMLButtonElement>('audit-newest').disabled = busy || before === 0;
    el<HTMLButtonElement>('audit-older').disabled = busy || !page?.nextBefore;
  }

  function empty(message: string) {
    const cell = document.createElement('td'); cell.colSpan = 5; cell.className = 'empty'; cell.textContent = message;
    const row = document.createElement('tr'); row.append(cell); el('audit-rows').replaceChildren(row);
  }

  function render(value: Page) {
    if (value.version !== 1 || !Array.isArray(value.entries) || value.entries.length > 100) throw new Error('Invalid audit page');
    page = value;
    const rows = value.entries.map(entry => {
      const row = document.createElement('tr');
      const time = document.createElement('td'); time.textContent = date(entry.at);
      const id = document.createElement('small'); id.textContent = `#${entry.id} · ${entry.clientIp}`; time.append(id);
      row.append(time);
      for (const content of [entry.actor || '—', entry.action, entry.result]) {
        const cell = document.createElement('td'); cell.textContent = content; row.append(cell);
      }
      const details = document.createElement('td'); details.className = 'audit-details'; details.textContent = auditSummary(entry) || '—';
      row.append(details);
      return row;
    });
    if (rows.length) el('audit-rows').replaceChildren(...rows); else empty('No audit entries match this search.');
    text('audit-status', `Read from PostgreSQL at ${date(value.asOf)}.`);
    text('audit-page', `${value.entries.length} entr${value.entries.length === 1 ? 'y' : 'ies'} · ${before ? 'older page' : 'newest first'}`);
    controls();
  }

  async function load(nextBefore = 0) {
    if (busy) return;
    const current = ++generation;
    busy = true; controls(); text('audit-status', 'Querying PostgreSQL…');
    try {
      const params = new URLSearchParams({ limit: '50' });
      if (filters.actor) params.set('actor', filters.actor);
      if (filters.action) params.set('action', filters.action);
      if (nextBefore) params.set('before', String(nextBefore));
      const value = await api<Page>(`/staff/audit?${params}`);
      if (current !== generation) return;
      before = nextBefore;
      render(value);
    } catch (error) {
      if (current !== generation) return;
      text('audit-status', explain(error));
    } finally { if (current === generation) { busy = false; controls(); } }
  }

  el<HTMLFormElement>('audit-form').addEventListener('submit', event => {
    event.preventDefault();
    const actor = el<HTMLInputElement>('audit-actor').value.trim().toLowerCase();
    const action = el<HTMLInputElement>('audit-action').value.trim();
    if (!/^[a-z0-9]{0,12}$/.test(actor) || !/^[a-z0-9._-]{0,64}$/.test(action)) { text('audit-status', 'Invalid filter.'); return; }
    filters = { actor, action };
    void load(0);
  });
  el('audit-older').addEventListener('click', () => { if (page?.nextBefore) void load(page.nextBefore); });
  el('audit-newest').addEventListener('click', () => void load(0));
  return {
    refresh() { void load(before); },
    reset() {
      generation++; page = undefined; before = 0; busy = false; filters = { actor: '', action: '' };
      el<HTMLInputElement>('audit-actor').value = ''; el<HTMLInputElement>('audit-action').value = '';
      text('audit-status', 'Waiting for a query.'); text('audit-page', 'Newest first · 50 entries per page');
      empty('No query has been made.'); controls();
    },
  };
}
