import type { components } from '../lib/api.generated';
type Status = components['schemas']['ColosseumStatus'];
type Command = components['schemas']['ColosseumCommand'];
type Result = components['schemas']['ColosseumResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

export function setupColosseum(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const text = (id: string, value: string) => { el(id).textContent = value; };
  const path = '/staff/events/colosseum';
  const messages: Record<Result['code'], string> = {
    ok: 'The server started the selected tier.',
    invalid_command: 'Invalid operation. Refresh and check the reason.',
    capability_required: 'This session cannot control the Colosseum.',
    server_maintenance: 'The server is under maintenance.',
    event_disabled: 'The Colosseum is disabled or unavailable.',
    stale_event: 'The event changed. Review the current state before trying again.',
    operation_conflict: 'This operation ID was already used for a different command.',
    operation_capacity: 'The process-local command receipt limit has been reached.',
    event_active: 'Another Colosseum tier is already running.',
    event_busy: 'Player exits are still being saved. Wait and refresh.',
    already_started: 'This tier has already started today.',
    schedule_passed: 'This tier can only be advanced before its scheduled start.',
  };
  let status: Status | undefined;
  let busy = false;
  let generation = 0;

  function setControls() {
    el<HTMLButtonElement>('colosseum-refresh').disabled = busy;
    el<HTMLInputElement>('colosseum-reason').disabled = busy;
    el<HTMLSelectElement>('colosseum-tier').disabled = busy;
    const selected = status?.schedule.find(entry => entry.tier === el<HTMLSelectElement>('colosseum-tier').value);
    el<HTMLButtonElement>('colosseum-start').disabled = busy || !status?.enabled || status.active || status.pendingExits > 0 ||
      !selected || selected.claimed || Date.parse(selected.startsAt) <= Date.parse(status.asOf);
  }

  function render(value: Status) {
    if (value.version !== 1 || !Array.isArray(value.schedule) || (value.enabled && (value.epoch.length !== 32 || value.schedule.length !== 3))) {
      throw new Error('Invalid Colosseum snapshot');
    }
    status = value;
    const display = (date: string) => new Date(date).toLocaleString('en-US', { timeZone: value.timezone });
    text('colosseum-schedule', value.enabled ? `${value.timezone}: ${value.schedule.map(entry => `${entry.tier} ${display(entry.startsAt)}${entry.claimed ? ' (already started)' : ''}`).join(' · ')}` : 'Colosseum unavailable.');
    text('colosseum-state', value.active ? `Tier ${value.tier} running · stage ${value.stage} · ends ${display(value.endsAt!)}` : `Idle · ${value.pendingExits} pending player exit(s).`);
    text('colosseum-asof', `Snapshot from ${new Date(value.asOf).toLocaleString('en-US')}. No automatic refresh.`);
    setControls();
  }

  async function refresh() {
    if (busy) return;
    const current = generation;
    busy = true; setControls();
    try {
      const value = await api<Status>(path);
      if (current !== generation) return;
      render(value); text('colosseum-message', 'Schedule retrieved.');
    } catch (error) {
      if (current !== generation) return;
      status = undefined; text('colosseum-message', explain(error));
    } finally { if (current === generation) { busy = false; setControls(); } }
  }

  async function start() {
    if (busy || !status || el<HTMLButtonElement>('colosseum-start').disabled) return;
    const reason = el<HTMLInputElement>('colosseum-reason');
    if (!reason.reportValidity() || reason.value.trim().length < 3) return;
    const tier = el<HTMLSelectElement>('colosseum-tier').value as Command['tier'];
    const command: Command = { version: 1, operationId: crypto.randomUUID(), epoch: status.epoch,
      expectedRevision: status.revision, tier, reason: reason.value.trim() };
    if (!window.confirm(`Start Colosseum ${tier} now for 12 minutes, replacing today's scheduled session until restart?\nReason: ${command.reason}`)) return;
    const current = generation;
    busy = true; setControls(); text('colosseum-message', 'Waiting for server confirmation…');
    try {
      const result = await api<Result>(path, 'POST', command);
      if (current !== generation) return;
      if (result.operationId !== command.operationId || !Object.hasOwn(messages, result.code)) throw new Error('Invalid Colosseum receipt');
      render(result.status);
      text('colosseum-message', result.replayed ? 'Previously processed command; no new event was started. Review the current state.' : messages[result.code]);
    } catch (error) {
      if (current !== generation) return;
      status = undefined;
      text('colosseum-message', `${explain(error)} Outcome not confirmed; refresh the schedule before trying again.`);
    } finally { if (current === generation) { busy = false; setControls(); } }
  }

  el('colosseum-refresh').addEventListener('click', () => void refresh());
  el('colosseum-start').addEventListener('click', () => void start());
  el('colosseum-tier').addEventListener('change', setControls);
  return { refresh, reset() {
    generation++; status = undefined; busy = false;
    el<HTMLInputElement>('colosseum-reason').value = '';
    text('colosseum-schedule', 'Schedule not queried.'); text('colosseum-state', 'State not queried.');
    text('colosseum-message', ''); text('colosseum-asof', 'No automatic refresh.'); setControls();
  } };
}
