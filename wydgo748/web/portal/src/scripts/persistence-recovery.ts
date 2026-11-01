import type { components } from '../lib/api.generated';
type Status = components['schemas']['PersistenceRecoveryStatus'];
type Command = components['schemas']['PersistenceRecheckCommand'];
type Result = components['schemas']['PersistenceRecheckResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

export function setupPersistenceRecovery(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const text = (id: string, value: string) => { el(id).textContent = value; };
  const path = '/staff/persistence/quarantine';
  let status: Status | undefined;
  let busy = false;
  let generation = 0;
  const messages: Record<string, string> = {
    ok: 'Checks started. Refresh the list in a few seconds to see released accounts.',
    nothing_to_check: 'No quarantined write has a known transaction to check.',
    check_running: 'A check is already running. Refresh the list in a few seconds.',
    check_unavailable: 'The store cannot check transaction status.',
    invalid_command: 'Invalid operation. Check the reason and try again.',
    capability_required: 'This session no longer has permission to check quarantined writes.',
  };

  function setControls() {
    el<HTMLButtonElement>('persistence-recovery-refresh').disabled = busy;
    el<HTMLInputElement>('persistence-recovery-reason').disabled = busy;
    el<HTMLButtonElement>('persistence-recovery-check').disabled = busy || !status?.quarantined.some((op) => op.checkable);
  }

  function render(value: Status) {
    status = value;
    text('persistence-recovery-count', String(value.quarantined.length));
    text('persistence-recovery-released', String(value.released));
    el('persistence-recovery-list').replaceChildren(...value.quarantined.map((op) => {
      const item = document.createElement('li');
      const kind = op.shared ? 'shared objects: restart required'
        : op.checkable ? 'exact transaction known' : 'no transaction ID: restart required';
      const last = op.lastStatus ? `, last status ${op.lastStatus}` : '';
      item.textContent = `#${op.id} channel ${op.channel}, since ${new Date(op.since).toLocaleString('en-US')}: ` +
        `${op.accounts.join(', ') || 'no account'} (${kind}${last})`;
      return item;
    }));
    text('persistence-recovery-asof', `Snapshot from ${new Date(value.asOf).toLocaleString('en-US')}. The list is not updated automatically.`);
    setControls();
  }

  async function refresh() {
    if (busy) return;
    busy = true; setControls();
    const current = generation;
    try {
      const value = await api<Status>(path);
      if (current !== generation) return;
      if (value.version !== 1) throw new Error('Invalid quarantine snapshot');
      render(value);
      text('persistence-recovery-message', 'List retrieved.');
    } catch (error) {
      if (current !== generation) return;
      status = undefined;
      text('persistence-recovery-message', explain(error));
    } finally { if (current === generation) { busy = false; setControls(); } }
  }

  async function check() {
    if (busy || !status) return;
    const reason = el<HTMLInputElement>('persistence-recovery-reason');
    if (!reason.reportValidity()) return;
    const command: Command = { version: 1, operationId: crypto.randomUUID(), reason: reason.value.trim() };
    const current = generation;
    busy = true; setControls();
    text('persistence-recovery-message', 'Waiting for server confirmation…');
    try {
      const result = await api<Result>(path, 'POST', command);
      if (current !== generation) return;
      if (result.operationId !== command.operationId || !messages[result.code]) throw new Error('Invalid check receipt');
      render(result.status);
      text('persistence-recovery-message', messages[result.code]);
    } catch (error) {
      if (current !== generation) return;
      text('persistence-recovery-message', `${explain(error)} Refresh the list before trying again.`);
    } finally { if (current === generation) { busy = false; setControls(); } }
  }

  el('persistence-recovery-refresh').addEventListener('click', () => void refresh());
  el('persistence-recovery-check').addEventListener('click', () => void check());

  return { refresh, reset() {
    generation++; status = undefined; busy = false;
    text('persistence-recovery-count', 'Not queried'); text('persistence-recovery-released', '–');
    el('persistence-recovery-list').replaceChildren();
    text('persistence-recovery-message', ''); text('persistence-recovery-asof', 'No automatic refresh.');
    setControls();
  } };
}
