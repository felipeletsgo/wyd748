import type { components } from '../lib/api.generated';
type Status = components['schemas']['GlobalDropStatus'];
type Command = components['schemas']['GlobalDropCommand'];
type Result = components['schemas']['GlobalDropResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

export function setupGlobalDrop(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const text = (id: string, value: string) => { el(id).textContent = value; };
  let status: Status | undefined;
  let pending: Command | undefined;
  let busy = false;
  let generation = 0;
  const messages: Record<string, string> = {
    ok: 'Operation confirmed by the server.',
    stale_event: 'The event or server process changed. Check the current state before sending another operation.',
    event_active: 'An event is already active. End it before starting another.',
    event_inactive: 'This event has already ended.',
    unknown_item: 'Item ID not found in the server catalog.',
    invalid_command: 'Invalid configuration. Check the item, chance, limits, and reason.',
    operation_conflict: 'Identifier already used with a different configuration. Check the current state.',
    operation_capacity: 'Operation limit reached. New events are blocked; the active event can still be ended.',
    event_id_unavailable: 'Could not generate an event identifier. Try a new operation.',
    capability_required: 'This session no longer has permission to control this event.',
    server_maintenance: 'The server is under maintenance and does not accept changes.',
  };
  function buttons() {
    el<HTMLFieldSetElement>('event-settings').disabled = busy || !!pending || !status || status.active;
    el<HTMLButtonElement>('event-start').disabled = busy || !!pending || !status || status.active;
    el<HTMLButtonElement>('event-stop').disabled = busy || !!pending || !status?.active;
    el<HTMLButtonElement>('event-refresh').disabled = busy;
    el<HTMLButtonElement>('event-retry').hidden = !pending;
    el<HTMLButtonElement>('event-retry').disabled = busy;
    el<HTMLInputElement>('event-reason').disabled = busy || !!pending;
  }
  function render(value: Status) {
    status = value;
    const ended = { time: 'time limit reached', quantity: 'quantity limit reached', manual: 'ended by staff', '': '' };
    text('event-state', value.active ? 'Active' : value.eventId ? `Ended · ${ended[value.endedBy] ?? value.endedBy}` : 'No event started');
    text('event-item', value.itemId ? `${value.itemId} · ${value.itemName} · ${value.ratePercent}%` : '—');
    text('event-count', `${value.dropped.toLocaleString('en-US')} / ${value.maxDrops ? value.maxDrops.toLocaleString('en-US') : 'no quantity limit'}`);
    text('event-end', value.endsAt ? new Date(value.endsAt).toLocaleString('en-US') : 'No time limit');
    text('event-asof', `Snapshot from ${new Date(value.asOf).toLocaleString('en-US')}${value.actor ? ` · started by ${value.actor}` : ''}. Use Check event to refresh.`);
    buttons();
  }
  async function refresh() {
    if (busy) return;
    busy = true; buttons();
    const current = generation;
    try {
      const value = await api<Status>('/staff/events/global-drop');
      if (current !== generation) return;
      if (value.version !== 1 || value.epoch.length !== 32) throw new Error('Invalid event snapshot');
      render(value);
      if (!pending) text('event-message', 'State retrieved.');
    } catch (error) {
      if (current !== generation) return;
      status = undefined;
      text('event-state', 'Event data unavailable');
      text('event-message', explain(error));
    } finally { if (current === generation) { busy = false; buttons(); } }
  }
  async function send() {
    if (!pending || busy) return;
    const command = pending;
    const current = generation;
    busy = true; buttons();
    text('event-message', 'Waiting for server confirmation…');
    try {
      const result = await api<Result>('/staff/events/global-drop', 'POST', command);
      if (current !== generation) return;
      if (result.operationId !== command.operationId || !messages[result.code]) throw new Error('Invalid event receipt');
      pending = undefined;
      render(result.status);
      text('event-message', `${messages[result.code]}${result.replayed ? ' Outcome recovered without repeating the operation.' : ''}`);
    } catch (error) {
      if (current !== generation) return;
      text('event-message', `${explain(error)} Outcome not confirmed. Use the retry button, which keeps the same identifier; do not reload the page.`);
    } finally { if (current === generation) { busy = false; buttons(); } }
  }
  function prepare(action: 'start' | 'stop') {
    if (busy || pending || !status) return;
    const reason = el<HTMLInputElement>('event-reason');
    if (!reason.reportValidity()) return;
    const number = (id: string) => Number(el<HTMLInputElement>(id).value);
    const c: Command = { version: 1, operationId: crypto.randomUUID(), epoch: status.epoch, expectedEventId: status.eventId,
      action, reason: reason.value.trim(), itemId: 0, ratePercent: 0, maxDrops: 0, durationSeconds: 0 };
    if (action === 'start') {
      c.itemId = number('event-item-id'); c.ratePercent = number('event-rate'); c.maxDrops = number('event-max'); c.durationSeconds = number('event-minutes') * 60;
      if (!c.maxDrops && !c.durationSeconds) { text('event-message', 'Enter a quantity or time limit.'); return; }
    }
    const detail = action === 'start' ? `Start a global drop for item ${c.itemId} with a ${c.ratePercent}% chance?\nQuantity: ${c.maxDrops || 'unlimited'} · Minutes: ${c.durationSeconds / 60 || 'unlimited'}` : `End the global drop for item ${status.itemId} (${status.itemName})?`;
    if (!window.confirm(`${detail}\nReason: ${c.reason}`)) return;
    pending = c; void send();
  }
  el<HTMLFormElement>('event-form').addEventListener('submit', event => { event.preventDefault(); prepare('start'); });
  el('event-stop').addEventListener('click', () => prepare('stop'));
  el('event-refresh').addEventListener('click', () => void refresh());
  el('event-retry').addEventListener('click', () => void send());
  return { refresh, reset() {
    generation++; status = undefined; pending = undefined; busy = false;
    text('event-state', 'Waiting for a query');
    for (const id of ['event-item', 'event-count', 'event-end']) text(id, '—');
    text('event-asof', 'No automatic updates.'); text('event-message', '');
    buttons();
  } };
}
