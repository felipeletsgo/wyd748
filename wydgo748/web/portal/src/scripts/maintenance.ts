import type { components } from '../lib/api.generated';
type Status = components['schemas']['ShutdownStatus'];
type Command = components['schemas']['ShutdownCommand'];
type Result = components['schemas']['ShutdownResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

const outcomes: Record<Result['code'], string> = {
  ok: 'The server confirmed the operation.',
  capability_required: 'Permission revoked or admin session expired. Sign in again.',
  server_maintenance: 'The server is already shutting down.',
  operation_conflict: 'This operation ID was already used for a different command.',
  operation_capacity: 'The process-local receipt limit has been reached.',
  shutdown_unavailable: 'This server process cannot schedule a shutdown.',
  already_scheduled: 'Maintenance is already scheduled. Cancel it before scheduling another.',
  not_scheduled: 'No maintenance is scheduled.',
  already_started: 'The shutdown has already started and can no longer be cancelled.',
};

export function setupMaintenance(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const text = (id: string, value: string) => { el(id).textContent = value; };
  const path = '/staff/server/shutdown';
  let status: Status | undefined;
  let busy = false;
  let generation = 0;

  function controls() {
    el<HTMLButtonElement>('maintenance-refresh').disabled = busy;
    const canSchedule = !busy && !!status?.available && !status.scheduled;
    el<HTMLFieldSetElement>('maintenance-settings').disabled = !canSchedule;
    el<HTMLButtonElement>('maintenance-schedule').disabled = !canSchedule;
    el<HTMLButtonElement>('maintenance-cancel').disabled = busy || !status?.scheduled;
    el<HTMLInputElement>('maintenance-reason').disabled = busy || !status?.available;
  }

  function render(value: Status) {
    if (value.version !== 1 || (value.scheduled && !value.at)) throw new Error('Invalid maintenance snapshot');
    status = value;
    const minutes = Math.floor(value.secondsRemaining / 60), seconds = value.secondsRemaining % 60;
    text('maintenance-state', !value.available ? 'Unavailable in this process' : value.scheduled
      ? `Scheduled · ${minutes} min ${seconds} s remaining${value.loginsBlocked ? ' · logins closed' : ''}` : 'Not scheduled');
    text('maintenance-mode', value.scheduled ? (value.mode === 'restart' ? 'Restart' : 'Shutdown') : '—');
    text('maintenance-at', value.at ? new Date(value.at).toLocaleString('en-US') : '—');
    text('maintenance-actor', value.scheduled ? `${value.actor} · ${value.reason}` : '—');
    text('maintenance-asof', `Snapshot from ${new Date(value.asOf).toLocaleString('en-US')}. No automatic refresh.`);
    controls();
  }

  async function refresh() {
    if (busy) return;
    const current = generation;
    busy = true; controls();
    try {
      const value = await api<Status>(path);
      if (current !== generation) return;
      render(value);
    } catch (error) {
      if (current !== generation) return;
      status = undefined; text('maintenance-message', explain(error));
    } finally { if (current === generation) { busy = false; controls(); } }
  }

  async function submit(command: Command) {
    const current = generation;
    busy = true; controls(); text('maintenance-message', 'Waiting for server confirmation…');
    try {
      const result = await api<Result>(path, 'POST', command);
      if (current !== generation) return;
      if (result.operationId !== command.operationId || !Object.hasOwn(outcomes, result.code)) throw new Error('Invalid maintenance receipt');
      render(result.status);
      text('maintenance-message', result.replayed ? 'Previously processed command; nothing was repeated. Review the current status.' : outcomes[result.code]);
      if (result.code === 'ok') el<HTMLInputElement>('maintenance-reason').value = '';
    } catch (error) {
      if (current !== generation) return;
      status = undefined;
      text('maintenance-message', `${explain(error)} Outcome not confirmed; refresh the status before trying again.`);
    } finally { if (current === generation) { busy = false; controls(); } }
  }

  function reason() {
    const input = el<HTMLInputElement>('maintenance-reason');
    const value = input.value.trim();
    if (!input.reportValidity() || [...value].length < 3 || /[\u0000-\u001f\u007f-\u009f]/u.test(value)) {
      text('maintenance-message', 'Enter a valid reason with 3 to 200 characters.'); return undefined;
    }
    return value;
  }

  el<HTMLFormElement>('maintenance-form').addEventListener('submit', event => {
    event.preventDefault();
    if (busy || !status?.available || status.scheduled) return;
    const minutes = Number(el<HTMLInputElement>('maintenance-minutes').value);
    const mode = el<HTMLSelectElement>('maintenance-kind').value as Command['mode'];
    const why = reason();
    if (!why) return;
    if (!Number.isInteger(minutes) || minutes < 1 || minutes > 1440) { text('maintenance-message', 'Choose 1 to 1440 minutes.'); return; }
    const effect = mode === 'restart' ? 'The service manager restarts the server afterwards.' : 'The server stays OFFLINE until someone starts it again.';
    if (!window.confirm(`${mode === 'restart' ? 'Restart' : 'Shut down'} the game server in ${minutes} minute${minutes === 1 ? '' : 's'}?\nEvery online player is warned and saved.\n${effect}\nReason: ${why}`)) return;
    void submit({ version: 1, operationId: crypto.randomUUID(), action: 'schedule', mode, delaySeconds: minutes * 60, reason: why });
  });
  el('maintenance-cancel').addEventListener('click', () => {
    if (busy || !status?.scheduled) return;
    const why = reason();
    if (!why || !window.confirm(`Cancel the scheduled ${status.mode}? Players are told it was cancelled.\nReason: ${why}`)) return;
    void submit({ version: 1, operationId: crypto.randomUUID(), action: 'cancel', mode: '', delaySeconds: 0, reason: why });
  });
  el('maintenance-refresh').addEventListener('click', () => void refresh());
  return { refresh, reset() {
    generation++; status = undefined; busy = false;
    el<HTMLInputElement>('maintenance-reason').value = '';
    for (const id of ['maintenance-mode', 'maintenance-at', 'maintenance-actor']) text(id, '—');
    text('maintenance-state', 'Waiting for a query'); text('maintenance-message', ''); text('maintenance-asof', 'No automatic refresh.');
    controls();
  } };
}
