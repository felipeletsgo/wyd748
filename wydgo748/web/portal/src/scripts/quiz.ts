import type { components } from '../lib/api.generated';
type Status = components['schemas']['QuizStatus'];
type Command = components['schemas']['QuizCommand'];
type Result = components['schemas']['QuizResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

export function setupQuiz(api: API, explain: (error: unknown) => string) {
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
    invalid_command: 'Invalid configuration. Check the item, quantity, duration, and reason.',
    operation_conflict: 'Identifier already used with a different configuration. Check the current state.',
    operation_capacity: 'Operation limit reached for this process. New events are blocked; the active event can still be ended.',
    capability_required: 'This session no longer has permission to control this event.',
    event_id_unavailable: 'Could not generate an event identifier. Try a new operation.',
    server_maintenance: 'The server is under maintenance and does not accept changes.',
  };
  function buttons() {
    el<HTMLFieldSetElement>('quiz-settings').disabled = busy || !!pending || !status || status.active;
    el<HTMLButtonElement>('quiz-start').disabled = busy || !!pending || !status || status.active;
    el<HTMLButtonElement>('quiz-stop').disabled = busy || !!pending || !status?.active;
    el<HTMLButtonElement>('quiz-refresh').disabled = busy;
    el<HTMLButtonElement>('quiz-retry').hidden = !pending;
    el<HTMLButtonElement>('quiz-retry').disabled = busy;
    el<HTMLInputElement>('quiz-reason').disabled = busy || !!pending;
  }
  function render(value: Status) {
    status = value;
    const ended = { time: 'time limit reached', quantity: 'quantity limit reached', manual: 'ended by staff', '': '' };
    text('quiz-state', value.active ? 'Active' : value.eventId ? `Ended · ${ended[value.endedBy] ?? value.endedBy}` : 'No event started');
    text('quiz-item', value.itemId ? `${value.itemId} · ${value.itemName} · ${value.rewardQuantity} unit(s) per correct answer` : '—');
    text('quiz-count', `${value.rounds} questions · ${value.correct} correct · ${value.rewarded} rewarded · ${value.deliveryFailures} delivery failures`);
    text('quiz-end', value.endsAt ? new Date(value.endsAt).toLocaleString('en-US') : '—');
    text('quiz-next', value.active && value.nextRoundAt ? new Date(value.nextRoundAt).toLocaleTimeString('en-US') : '—');
    text('quiz-asof', `Snapshot from ${new Date(value.asOf).toLocaleString('en-US')}${value.actor ? ` · started by ${value.actor}` : ''}. Use Check event to refresh.`);
    buttons();
  }
  async function refresh() {
    if (busy) return;
    busy = true; buttons();
    const current = generation;
    try {
      const value = await api<Status>('/staff/events/quiz');
      if (current !== generation) return;
      if (value.version !== 1 || value.epoch.length !== 32) throw new Error('Invalid event snapshot');
      render(value);
      if (!pending) text('quiz-message', 'State retrieved.');
    } catch (error) {
      if (current !== generation) return;
      status = undefined;
      text('quiz-state', 'Event data unavailable');
      text('quiz-message', explain(error));
    } finally { if (current === generation) { busy = false; buttons(); } }
  }
  async function send() {
    if (!pending || busy) return;
    const command = pending;
    const current = generation;
    busy = true; buttons();
    text('quiz-message', 'Waiting for server confirmation…');
    try {
      const result = await api<Result>('/staff/events/quiz', 'POST', command);
      if (current !== generation) return;
      if (result.operationId !== command.operationId || !messages[result.code]) throw new Error('Invalid event receipt');
      pending = undefined;
      render(result.status);
      text('quiz-message', `${messages[result.code]}${result.replayed ? ' Outcome recovered without repeating the operation.' : ''}`);
    } catch (error) {
      if (current !== generation) return;
      text('quiz-message', `${explain(error)} Outcome not confirmed. Use the retry button, which keeps the same identifier; do not reload the page.`);
    } finally { if (current === generation) { busy = false; buttons(); } }
  }
  function prepare(action: 'start' | 'stop') {
    if (busy || pending || !status) return;
    const reason = el<HTMLInputElement>('quiz-reason');
    if (!reason.reportValidity()) return;
    const number = (id: string) => Number(el<HTMLInputElement>(id).value);
    const c: Command = { version: 1, operationId: crypto.randomUUID(), epoch: status.epoch, expectedEventId: status.eventId,
      action, reason: reason.value.trim(), itemId: 0, rewardQuantity: 0, durationSeconds: 0 };
    if (action === 'start') {
      c.itemId = number('quiz-item-id'); c.rewardQuantity = number('quiz-quantity'); c.durationSeconds = number('quiz-minutes') * 60;
    }
    const detail = action === 'start' ? `Start a quiz for ${c.durationSeconds / 60} minutes?\nReward per correct answer: ${c.rewardQuantity} unit(s) of item ${c.itemId}. One question per minute, with 10 seconds to answer.` : `End the quiz and close the active question?`;
    if (!window.confirm(`${detail}\nReason: ${c.reason}`)) return;
    pending = c; void send();
  }
  el<HTMLFormElement>('quiz-form').addEventListener('submit', event => { event.preventDefault(); prepare('start'); });
  el('quiz-stop').addEventListener('click', () => prepare('stop'));
  el('quiz-refresh').addEventListener('click', () => void refresh());
  el('quiz-retry').addEventListener('click', () => void send());
  return { refresh, reset() {
    generation++; status = undefined; pending = undefined; busy = false;
    text('quiz-state', 'Waiting for a query');
    for (const id of ['quiz-item', 'quiz-count', 'quiz-end', 'quiz-next']) text(id, '—');
    text('quiz-asof', 'No automatic updates.'); text('quiz-message', '');
    buttons();
  } };
}
