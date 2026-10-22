import type { components } from '../lib/api.generated';
type Status = components['schemas']['OXEventStatus'];
type Command = components['schemas']['OXEventCommand'];
type Result = components['schemas']['OXEventResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

const labels: Record<Command['action'], string> = {
  start: 'Start OX event and open its gates',
  mode_on: 'Turn quiz mode on',
  mode_off: 'Turn quiz mode off',
  doors_lock: 'Lock the OX gates',
  doors_open: 'Open the OX gates',
  result_x: 'Send the X side out of the arena',
  result_o: 'Send the O side out of the arena',
};

export function setupOXEvent(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const text = (id: string, value: string) => { el(id).textContent = value; };
  const path = '/staff/events/ox';
  const buttons: Record<Command['action'], string> = {
    start: 'ox-event-start',
    mode_on: 'ox-event-mode-on', mode_off: 'ox-event-mode-off', doors_lock: 'ox-event-doors-lock',
    doors_open: 'ox-event-doors-open', result_x: 'ox-event-result-x', result_o: 'ox-event-result-o',
  };
  let status: Status | undefined;
  let busy = false;
  let generation = 0;
  const messages: Record<string, string> = {
    ok: 'The server applied the change.',
    unchanged: 'The arena was already in that state.',
    doors_unavailable: 'The OX gates are not loaded on this server.',
    invalid_command: 'Invalid operation. Check the reason and try again.',
    capability_required: 'This session no longer has permission to run the OX event.',
    server_maintenance: 'The server is under maintenance and does not accept changes.',
  };

  function setControls() {
    el<HTMLButtonElement>('ox-event-refresh').disabled = busy;
    el<HTMLInputElement>('ox-event-reason').disabled = busy;
    const ready = !busy && !!status;
    el<HTMLButtonElement>(buttons.start).disabled = !ready || !!status?.quizMode || status?.doors !== 6;
    el<HTMLButtonElement>(buttons.mode_on).disabled = !ready || !!status?.quizMode;
    el<HTMLButtonElement>(buttons.mode_off).disabled = !ready || !status?.quizMode;
    const doors = ready && (status?.doors ?? 0) > 0;
    el<HTMLButtonElement>(buttons.doors_lock).disabled = !doors || !!status?.doorsLocked;
    el<HTMLButtonElement>(buttons.doors_open).disabled = !doors || !status?.doorsLocked;
    el<HTMLButtonElement>(buttons.result_x).disabled = !ready;
    el<HTMLButtonElement>(buttons.result_o).disabled = !ready;
  }

  function render(value: Status) {
    status = value;
    text('ox-event-mode', value.quizMode ? 'On (ticket enters the arena)' : 'Off (ticket opens the Big Cube)');
    text('ox-event-doors', value.doors === 0 ? 'Unavailable' : value.doorsLocked ? 'Locked' : 'Open');
    text('ox-event-players', `X side ${value.playersX}, O side ${value.playersO}`);
    text('ox-event-asof', `Snapshot from ${new Date(value.asOf).toLocaleString('en-US')}. State is not updated automatically.`);
    setControls();
  }

  async function refresh() {
    if (busy) return;
    busy = true; setControls();
    const current = generation;
    try {
      const value = await api<Status>(path);
      if (current !== generation) return;
      if (value.version !== 1) throw new Error('Invalid OX arena snapshot');
      render(value);
      text('ox-event-message', 'Arena state retrieved.');
    } catch (error) {
      if (current !== generation) return;
      status = undefined;
      text('ox-event-message', explain(error));
    } finally { if (current === generation) { busy = false; setControls(); } }
  }

  async function send(action: Command['action']) {
    if (busy || !status || el<HTMLButtonElement>(buttons[action]).disabled) return;
    const reason = el<HTMLInputElement>('ox-event-reason');
    if (!reason.reportValidity() || reason.value.trim().length < 3) return;
    const command: Command = { version: 1, operationId: crypto.randomUUID(), action, reason: reason.value.trim() };
    if (!window.confirm(`${labels[action]}?\nReason: ${command.reason}`)) return;
    const current = generation;
    busy = true; setControls();
    text('ox-event-message', 'Waiting for server confirmation…');
    try {
      const result = await api<Result>(path, 'POST', command);
      if (current !== generation) return;
      if (result.operationId !== command.operationId || result.status.version !== 1 || !Object.hasOwn(messages, result.code)) throw new Error('Invalid OX arena receipt');
      render(result.status);
      const moved = action === 'result_x' || action === 'result_o' ? ` ${result.moved} player(s) moved out.` : '';
      text('ox-event-message', messages[result.code] + moved);
    } catch (error) {
      if (current !== generation) return;
      status = undefined;
      text('ox-event-message', `${explain(error)} Outcome not confirmed; refresh the arena before trying again.`);
    } finally { if (current === generation) { busy = false; setControls(); } }
  }

  el('ox-event-refresh').addEventListener('click', () => void refresh());
  for (const action of Object.keys(buttons) as Command['action'][]) {
    el(buttons[action]).addEventListener('click', () => void send(action));
  }

  return { refresh, reset() {
    generation++; status = undefined; busy = false;
    el<HTMLInputElement>('ox-event-reason').value = '';
    for (const id of ['ox-event-mode', 'ox-event-doors', 'ox-event-players']) text(id, 'Not queried');
    text('ox-event-message', ''); text('ox-event-asof', 'No automatic refresh.');
    setControls();
  } };
}
