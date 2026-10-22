import type { components } from '../lib/api.generated';
type Page = components['schemas']['LogPage'];
type Backups = components['schemas']['BackupStatus'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

export const levels = ['emerg', 'alert', 'crit', 'error', 'warning', 'notice', 'info', 'debug'];

// Builds the request for one page; the window is fixed when the newest page
// loads so older pages stay inside it.
export function logQuery(source: string, since: Date, until: Date, priority: string, query: string, cursor = '') {
  const params = new URLSearchParams({ source, since: since.toISOString(), until: until.toISOString(), priority, limit: '200' });
  if (query) params.set('query', query);
  if (cursor) params.set('cursor', cursor);
  return params;
}

export function setupServerLogs(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  let scope: { source: string; since: Date; until: Date; priority: string; query: string } | undefined;
  let next = '';
  let busy = false;
  let generation = 0;

  function render(page: Page, append: boolean) {
    const rows = page.entries.map(entry => {
      const row = document.createElement('tr');
      const at = new Date(entry.time);
      const time = document.createElement('td'); time.textContent = at.toISOString().replace('T', ' ').slice(0, 19);
      const local = document.createElement('small'); local.textContent = at.toLocaleString('en-US'); time.append(local);
      const level = document.createElement('td'); level.textContent = levels[entry.priority] ?? String(entry.priority);
      if (entry.priority <= 3) level.className = 'log-error'; else if (entry.priority === 4) level.className = 'log-warning';
      const message = document.createElement('td'); message.className = 'log-message'; message.textContent = entry.message;
      row.append(time, level, message);
      return row;
    });
    if (append) el('logs-rows').append(...rows);
    else if (rows.length) el('logs-rows').replaceChildren(...rows);
    else {
      const cell = document.createElement('td'); cell.colSpan = 3; cell.className = 'empty'; cell.textContent = 'No entries in this window.';
      const row = document.createElement('tr'); row.append(cell); el('logs-rows').replaceChildren(row);
    }
    next = page.nextCursor;
    el<HTMLButtonElement>('logs-older').disabled = !next;
    el('logs-status').textContent = `${el('logs-rows').children.length} line(s) shown${page.truncated ? ' · page ended early by the scan limit; use Older to continue' : ''}${next ? '' : ' · start of the window reached'}.`;
  }

  async function load(append: boolean) {
    if (busy || !scope) return;
    const current = ++generation;
    busy = true; el<HTMLButtonElement>('logs-submit').disabled = true; el<HTMLButtonElement>('logs-older').disabled = true;
    el('logs-status').textContent = 'Reading logs…';
    try {
      const page = await api<Page>(`/staff/logs?${logQuery(scope.source, scope.since, scope.until, scope.priority, scope.query, append ? next : '')}`);
      if (current !== generation) return;
      if (page.version !== 1 || !Array.isArray(page.entries)) throw new Error('Invalid log page');
      render(page, append);
    } catch (error) {
      if (current !== generation) return;
      el('logs-status').textContent = explain(error);
    } finally {
      if (current === generation) { busy = false; el<HTMLButtonElement>('logs-submit').disabled = false; el<HTMLButtonElement>('logs-older').disabled = !next; }
    }
  }

  async function refreshBackups() {
    try {
      const value = await api<Backups>('/staff/backups');
      const backup = value.backup, restore = value.restoreTest;
      el('backup-last').textContent = backup ? `${new Date(backup.at).toLocaleString('en-US')} · ${backup.archive}` : 'No backup receipt recorded';
      el('backup-size').textContent = backup?.bytes ? `${(backup.bytes / 1048576).toFixed(1)} MiB · ${backup.sha256?.slice(0, 16) ?? ''}…` : '—';
      el('backup-restore').textContent = restore ? `${new Date(restore.at).toLocaleString('en-US')} · ${restore.restoredAccounts ?? 0} accounts restored` : 'No restore test recorded';
    } catch (error) {
      el('backup-last').textContent = explain(error);
    }
  }

  el<HTMLFormElement>('logs-form').addEventListener('submit', event => {
    event.preventDefault();
    const minutes = Number(el<HTMLSelectElement>('logs-window').value);
    const until = new Date();
    scope = { source: el<HTMLSelectElement>('logs-source').value, since: new Date(until.getTime() - minutes * 60000), until,
      priority: el<HTMLSelectElement>('logs-priority').value, query: el<HTMLInputElement>('logs-query').value.trim() };
    next = '';
    void load(false);
  });
  el('logs-older').addEventListener('click', () => void load(true));
  el('backup-refresh').addEventListener('click', () => void refreshBackups());
  return {
    refreshBackups,
    reset() {
      generation++; scope = undefined; next = ''; busy = false;
      el<HTMLButtonElement>('logs-submit').disabled = false; el<HTMLButtonElement>('logs-older').disabled = true;
      el('logs-status').textContent = 'Not queried. Times are shown in UTC and in your local time.';
      const cell = document.createElement('td'); cell.colSpan = 3; cell.className = 'empty'; cell.textContent = 'No query has been made.';
      const row = document.createElement('tr'); row.append(cell); el('logs-rows').replaceChildren(row);
      for (const id of ['backup-last', 'backup-size', 'backup-restore']) el(id).textContent = '—';
    },
  };
}
