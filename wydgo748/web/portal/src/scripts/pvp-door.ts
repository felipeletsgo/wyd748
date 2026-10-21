import type { components } from '../lib/api.generated';
type Status = components['schemas']['PvPDoorStatus'];
type Command = components['schemas']['PvPDoorCommand'];
type Result = components['schemas']['PvPDoorResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

export function setupPvPDoor(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const text = (id: string, value: string) => { el(id).textContent = value; };
  const path = '/staff/world/pvp-door';
  let status: Status | undefined;
  let busy = false;
  let generation = 0;
  const messages: Record<string, string> = {
    ok: 'The server applied the change.',
    unchanged: 'The door was already in that state.',
    door_unavailable: 'The PvP door is not loaded on this server.',
    invalid_command: 'Invalid operation. Check the reason and try again.',
    capability_required: 'This session no longer has permission to control the PvP door.',
    server_maintenance: 'The server is under maintenance and does not accept changes.',
  };

  function setControls() {
    el<HTMLButtonElement>('pvp-door-refresh').disabled = busy;
    el<HTMLInputElement>('pvp-door-reason').disabled = busy;
    const ready = !busy && !!status?.available;
    el<HTMLButtonElement>('pvp-door-lock').disabled = !ready || !!status?.locked;
    el<HTMLButtonElement>('pvp-door-open').disabled = !ready || !status?.locked;
  }

  function render(value: Status) {
    status = value;
    text('pvp-door-state', !value.available ? 'Unavailable' : value.locked ? 'Locked (PvP area closed)' : 'Open');
    text('pvp-door-asof', `Snapshot from ${new Date(value.asOf).toLocaleString('en-US')}. State is not updated automatically.`);
    setControls();
  }

  async function refresh() {
    if (busy) return;
    busy = true; setControls();
    const current = generation;
    try {
      const value = await api<Status>(path);
      if (current !== generation) return;
      if (value.version !== 1) throw new Error('Invalid PvP door snapshot');
      render(value);
      text('pvp-door-message', 'Door state retrieved.');
    } catch (error) {
      if (current !== generation) return;
      status = undefined;
      text('pvp-door-message', explain(error));
    } finally { if (current === generation) { busy = false; setControls(); } }
  }

  async function send(action: Command['action']) {
    if (busy || !status) return;
    const reason = el<HTMLInputElement>('pvp-door-reason');
    if (!reason.reportValidity()) return;
    const command: Command = { version: 1, operationId: crypto.randomUUID(), action, reason: reason.value.trim() };
    if (!window.confirm(`${action === 'lock' ? 'Lock' : 'Open'} the Armia PvP door?\nReason: ${command.reason}`)) return;
    const current = generation;
    busy = true; setControls();
    text('pvp-door-message', 'Waiting for server confirmation…');
    try {
      const result = await api<Result>(path, 'POST', command);
      if (current !== generation) return;
      if (result.operationId !== command.operationId || !messages[result.code]) throw new Error('Invalid PvP door receipt');
      render(result.status);
      text('pvp-door-message', messages[result.code]);
    } catch (error) {
      if (current !== generation) return;
      text('pvp-door-message', `${explain(error)} Outcome not confirmed; refresh the door before trying again.`);
    } finally { if (current === generation) { busy = false; setControls(); } }
  }

  el('pvp-door-refresh').addEventListener('click', () => void refresh());
  el('pvp-door-lock').addEventListener('click', () => void send('lock'));
  el('pvp-door-open').addEventListener('click', () => void send('open'));

  return { refresh, reset() {
    generation++; status = undefined; busy = false;
    text('pvp-door-state', 'Not queried'); text('pvp-door-message', ''); text('pvp-door-asof', 'No automatic refresh.');
    setControls();
  } };
}
