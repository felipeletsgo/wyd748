import type { components } from '../lib/api.generated';
type View = components['schemas']['SanctionsView'];
type Sanction = components['schemas']['Sanction'];
type Command = components['schemas']['SanctionCommand'];
type Revoke = components['schemas']['SanctionRevokeCommand'];
type Result = components['schemas']['SanctionResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

const live: Record<Result['live'], string> = {
  '': '',
  applied: 'Online sessions were updated immediately.',
  offline: 'The account was offline; the sanction applies at its next login.',
  pending: 'The server did not confirm the live effect. The stored sanction still applies at the next login; retry to refresh it.',
};
const outcomes: Record<Result['code'], string> = {
  ok: 'Sanction saved.',
  capability_required: 'Permission revoked or admin session expired. Sign in again.',
  operation_conflict: 'This request ID was already used for a different sanction. Reload the history.',
  unknown_sanction: 'That sanction no longer exists for this account. Reload the history.',
};

export function setupSanctions(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const text = (id: string, value: string) => { el(id).textContent = value; };
  const date = (value: string) => new Date(value).toLocaleString('en-US', { dateStyle: 'short', timeStyle: 'short' });
  let view: View | undefined;
  let pending: Command | undefined;
  let busy = false;
  let generation = 0;

  function controls() {
    el<HTMLButtonElement>('sanction-load').disabled = busy;
    el<HTMLFieldSetElement>('sanction-settings').disabled = busy || !view || !!pending;
    el<HTMLButtonElement>('sanction-submit').disabled = busy || !view || !!pending;
    el('sanction-retry').hidden = !pending;
    el<HTMLButtonElement>('sanction-retry').disabled = busy;
    document.querySelectorAll<HTMLButtonElement>('.sanction-revoke').forEach(button => { button.disabled = busy || !!pending; });
  }

  function status(sanction: Sanction) {
    if (sanction.revokedAt) return `Revoked by ${sanction.revokedBy}`;
    return sanction.active ? 'Active' : 'Expired';
  }

  function render(value: View) {
    if (value.version !== 1 || !Array.isArray(value.sanctions)) throw new Error('Invalid sanctions snapshot');
    view = value;
    text('sanction-target', value.account);
    const active = value.sanctions.filter(sanction => sanction.active);
    text('sanction-status', `${value.account}: ${active.length ? active.map(sanction => `${sanction.kind} active`).join(' · ') : 'no active sanctions'}. Read ${date(value.asOf)}.`);
    const rows = value.sanctions.map(sanction => {
      const row = document.createElement('tr');
      const cells = [sanction.kind === 'ban' ? 'Ban' : 'Mute', status(sanction), date(sanction.createdAt),
        sanction.expiresAt ? date(sanction.expiresAt) : 'Permanent'];
      for (const value of cells) { const cell = document.createElement('td'); cell.textContent = value; row.append(cell); }
      const who = document.createElement('td'); who.textContent = sanction.actor;
      const reason = document.createElement('small'); reason.textContent = sanction.revokedAt ? `${sanction.reason} · revoked: ${sanction.revokeReason}` : sanction.reason;
      who.append(reason); row.append(who);
      const action = document.createElement('td');
      if (sanction.active) {
        const revoke = document.createElement('button'); revoke.type = 'button'; revoke.className = 'button subtle sanction-revoke'; revoke.textContent = 'Revoke';
        revoke.addEventListener('click', () => void revokeSanction(sanction)); action.append(revoke);
      }
      row.append(action);
      return row;
    });
    if (rows.length) el('sanction-rows').replaceChildren(...rows);
    else {
      const cell = document.createElement('td'); cell.colSpan = 6; cell.className = 'empty'; cell.textContent = 'This account has no sanctions.';
      const row = document.createElement('tr'); row.append(cell); el('sanction-rows').replaceChildren(row);
    }
    controls();
  }

  async function load(account = view?.account) {
    if (busy || !account) return;
    const current = ++generation;
    busy = true; controls(); text('sanction-status', 'Querying PostgreSQL…');
    try {
      const value = await api<View>(`/staff/accounts/${encodeURIComponent(account)}/sanctions`);
      if (current !== generation) return;
      render(value);
    } catch (error) {
      if (current !== generation) return;
      view = undefined; text('sanction-status', explain(error)); text('sanction-target', '—');
    } finally { if (current === generation) { busy = false; controls(); } }
  }

  async function send() {
    if (!pending || busy) return;
    const command = pending;
    const current = generation;
    busy = true; controls(); text('sanction-message', 'Waiting for server confirmation…');
    try {
      const result = await api<Result>(`/staff/accounts/${encodeURIComponent(command.account)}/sanctions`, 'POST', command);
      if (current !== generation) return;
      if (!Object.hasOwn(outcomes, result.code) || !Object.hasOwn(live, result.live)) throw new Error('Invalid sanction receipt');
      pending = undefined;
      text('sanction-message', `${outcomes[result.code]}${result.replayed ? ' Receipt recovered; nothing was applied twice.' : ''} ${live[result.live]}`.trim());
      el<HTMLInputElement>('sanction-reason').value = '';
    } catch (error) {
      if (current !== generation) return;
      text('sanction-message', `${explain(error)} Outcome not confirmed. Use “Check / retry” to resend the same request (${command.requestId}).`);
    } finally {
      if (current === generation) { busy = false; controls(); if (!pending) void load(); }
    }
  }

  async function revokeSanction(sanction: Sanction) {
    if (busy || pending || !view) return;
    const reason = window.prompt(`Revoke the ${sanction.kind} of ${view.account}? Enter the reason:`)?.trim() ?? '';
    if ([...reason].length < 3 || [...reason].length > 200 || /[\u0000-\u001f\u007f-\u009f]/u.test(reason)) {
      if (reason) text('sanction-message', 'Enter a valid reason with 3 to 200 characters.');
      return;
    }
    const command: Revoke = { version: 1, account: view.account, kind: sanction.kind, reason };
    const current = generation;
    busy = true; controls(); text('sanction-message', 'Waiting for server confirmation…');
    try {
      const result = await api<Result>(`/staff/accounts/${encodeURIComponent(view.account)}/sanctions/${sanction.id}/revoke`, 'POST', command);
      if (current !== generation) return;
      if (!Object.hasOwn(outcomes, result.code) || !Object.hasOwn(live, result.live)) throw new Error('Invalid revoke receipt');
      text('sanction-message', result.code === 'ok' ? `${result.replayed ? 'This sanction was already revoked.' : 'Sanction revoked.'} ${live[result.live]}`.trim() : outcomes[result.code]);
    } catch (error) {
      if (current !== generation) return;
      text('sanction-message', `${explain(error)} Revocation not confirmed; reload the history before trying again.`);
    } finally { if (current === generation) { busy = false; controls(); void load(); } }
  }

  el<HTMLFormElement>('sanction-lookup').addEventListener('submit', event => {
    event.preventDefault();
    const account = el<HTMLInputElement>('sanction-account').value.trim().toLowerCase();
    if (!/^[a-z0-9]{1,12}$/.test(account)) { text('sanction-status', 'Enter a valid account name.'); return; }
    if (pending) { text('sanction-message', 'Resolve the pending sanction before loading another account.'); return; }
    text('sanction-message', ''); void load(account);
  });
  el<HTMLFormElement>('sanction-form').addEventListener('submit', event => {
    event.preventDefault();
    if (!view || pending || busy) return;
    const reasonInput = el<HTMLInputElement>('sanction-reason');
    const reason = reasonInput.value.trim();
    if (!reasonInput.reportValidity() || [...reason].length < 3 || /[\u0000-\u001f\u007f-\u009f]/u.test(reason)) { text('sanction-message', 'Enter a valid reason with 3 to 200 characters.'); return; }
    const kind = el<HTMLSelectElement>('sanction-kind').value as Command['kind'];
    const durationMinutes = Number(el<HTMLSelectElement>('sanction-duration').value);
    const duration = el<HTMLSelectElement>('sanction-duration').selectedOptions[0]?.textContent ?? '';
    const consequence = kind === 'ban' ? 'This disconnects the account now and blocks login.' : 'This blocks the account’s chat.';
    if (!window.confirm(`${kind === 'ban' ? 'Ban' : 'Mute'} ${view.account} for ${duration.toLowerCase()}?\nReason: ${reason}\n${consequence}`)) return;
    pending = { version: 1, requestId: crypto.randomUUID(), account: view.account, kind, durationMinutes, reason };
    void send();
  });
  el('sanction-retry').addEventListener('click', () => void send());
  return {
    refresh() { void load(); },
    open(account: string) {
      if (pending || busy) { text('sanction-message', 'Resolve the pending sanction before loading another account.'); return; }
      el<HTMLInputElement>('sanction-account').value = account;
      text('sanction-message', '');
      document.getElementById('sanctions')?.scrollIntoView({ block: 'start' });
      void load(account.toLowerCase());
    },
    reset() {
      generation++; view = undefined; pending = undefined; busy = false;
      el<HTMLInputElement>('sanction-account').value = ''; el<HTMLInputElement>('sanction-reason').value = '';
      text('sanction-status', 'No account loaded.'); text('sanction-target', '—'); text('sanction-message', '');
      const cell = document.createElement('td'); cell.colSpan = 6; cell.className = 'empty'; cell.textContent = 'No account loaded.';
      const row = document.createElement('tr'); row.append(cell); el('sanction-rows').replaceChildren(row);
      controls();
    },
  };
}
