import type { components } from '../lib/api.generated';
import { auditSummary } from './audit-log';
type Logins = components['schemas']['AccountLogins'];
type Actions = components['schemas']['AccountActions'];
type Summary = components['schemas']['AccountSummary'];
type Notes = components['schemas']['AccountNotes'];
type Note = components['schemas']['AccountNote'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

// The account page joins the directory row with login history and staff
// actions. Each part loads independently so one missing permission does not
// hide the others.
// Parses "BUG-004, BUG-011" into distinct register IDs; undefined when invalid.
export function bugRefs(raw: string): string[] | undefined {
  const refs = raw.split(/[\s,]+/).map(ref => ref.trim().toUpperCase()).filter(Boolean);
  if (refs.length > 10 || refs.some(ref => !/^BUG-[0-9]{3,6}$/.test(ref)) || new Set(refs).size !== refs.length) return undefined;
  return refs;
}

export function setupAccountPage(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const date = (value: string) => new Date(value).toLocaleString('en-US', { dateStyle: 'short', timeStyle: 'medium' });
  let account = '';
  let summary: Summary | undefined;
  let generation = 0;

  function rows(id: string, columns: number, values: string[][], empty: string) {
    if (!values.length) {
      const cell = document.createElement('td'); cell.colSpan = columns; cell.className = 'empty'; cell.textContent = empty;
      const row = document.createElement('tr'); row.append(cell); el(id).replaceChildren(row);
      return;
    }
    el(id).replaceChildren(...values.map(cells => {
      const row = document.createElement('tr');
      for (const value of cells) { const cell = document.createElement('td'); cell.textContent = value; row.append(cell); }
      return row;
    }));
  }

  function renderCharacters() {
    rows('account-characters', 4, (summary?.characters ?? []).map(character => [String(character.slot + 1), character.name,
      `class ${character.class} · level ${character.level}`, character.evolution || 'mortal']),
      summary ? 'No characters.' : 'Character list available when opened from the account directory.');
  }

  async function load() {
    if (!account) return;
    const current = ++generation;
    el<HTMLButtonElement>('account-refresh').disabled = true;
    el('account-status').textContent = 'Querying PostgreSQL…';
    const path = `/staff/accounts/${encodeURIComponent(account)}`;
    const [logins, actions, notes] = await Promise.allSettled([api<Logins>(`${path}/logins`), api<Actions>(`${path}/actions`), api<Notes>(`${path}/notes`)]);
    if (current !== generation) return;
    if (notes.status === 'fulfilled') renderNotes(notes.value.notes); else noteList([explain(notes.reason)]);
    el<HTMLButtonElement>('account-note-submit').disabled = false;
    if (logins.status === 'fulfilled') {
      el('account-logins-note').textContent = logins.value.ipVisible
        ? 'Last 50 admitted game logins, kept 90 days.'
        : 'Last 50 admitted game logins, kept 90 days. IP addresses need the moderation.account.ip permission.';
      rows('account-logins', 3, logins.value.logins.map(login => [date(login.at), String(login.channel), login.clientIp || 'hidden']), 'No logins recorded.');
    } else rows('account-logins', 3, [], explain(logins.reason));
    if (actions.status === 'fulfilled') {
      rows('account-actions', 5, actions.value.entries.map(entry => [date(entry.at), entry.actor || '—', entry.action, entry.result,
        auditSummary(entry) || '—']), 'No staff actions recorded for this account.');
    } else rows('account-actions', 5, [], explain(actions.reason));
    el('account-status').textContent = logins.status === 'fulfilled' ? `Read ${date(logins.value.asOf)}.` : explain(logins.reason);
    el<HTMLButtonElement>('account-refresh').disabled = false;
  }

  function noteList(lines: string[]) {
    el('account-notes').replaceChildren(...lines.map(line => { const li = document.createElement('li'); li.textContent = line; return li; }));
  }
  function renderNotes(notes: Note[]) {
    noteList(notes.length ? notes.map(note => `${date(note.at)} · ${note.actor}${note.bugRefs.length ? ` · ${note.bugRefs.join(', ')}` : ''} — ${note.body}`) : ['No case notes.']);
  }

  el<HTMLFormElement>('account-note-form').addEventListener('submit', async event => {
    event.preventDefault();
    if (!account) return;
    const body = el<HTMLTextAreaElement>('account-note-body').value.trim();
    const refs = bugRefs(el<HTMLInputElement>('account-note-bugs').value);
    if ([...body].length < 3 || [...body].length > 2000) { el('account-note-status').textContent = 'Write 3 to 2,000 characters.'; return; }
    if (!refs) { el('account-note-status').textContent = 'Bug references must look like BUG-004, at most 10.'; return; }
    const current = generation;
    el<HTMLButtonElement>('account-note-submit').disabled = true;
    try {
      await api<Note>(`/staff/accounts/${encodeURIComponent(account)}/notes`, 'POST', { version: 1, body, bugRefs: refs });
      if (current !== generation) return;
      el<HTMLTextAreaElement>('account-note-body').value = ''; el<HTMLInputElement>('account-note-bugs').value = '';
      el('account-note-status').textContent = 'Note added.';
      void load();
    } catch (error) {
      if (current === generation) el('account-note-status').textContent = explain(error);
    } finally { if (current === generation) el<HTMLButtonElement>('account-note-submit').disabled = false; }
  });
  el('account-refresh').addEventListener('click', () => void load());
  return {
    open(name: string, row?: Summary) {
      account = name.toLowerCase(); summary = row;
      el('account-name').textContent = account;
      renderCharacters();
      document.getElementById('account')?.scrollIntoView({ block: 'start' });
      void load();
    },
    reset() {
      generation++; account = ''; summary = undefined;
      el('account-name').textContent = '—';
      el('account-status').textContent = 'Open an account from the directory or the online player list.';
      el<HTMLButtonElement>('account-refresh').disabled = true;
      for (const [id, columns] of [['account-characters', 4], ['account-logins', 3], ['account-actions', 5]] as const) rows(id, columns, [], 'No account open.');
      el('account-notes').replaceChildren(); el('account-note-status').textContent = '';
      el<HTMLButtonElement>('account-note-submit').disabled = true;
    },
  };
}
