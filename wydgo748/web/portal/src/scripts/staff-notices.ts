import type { components } from '../lib/api.generated';
type View = components['schemas']['NoticesView'];
type Command = components['schemas']['NoticeCommand'];
type Result = components['schemas']['NoticeResult'];
type MessageCommand = components['schemas']['PlayerMessageCommand'];
type MessageResult = components['schemas']['PlayerMessageResult'];
type Player = components['schemas']['Player'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

const noticeOutcomes: Record<Result['code'], string> = {
  ok: 'Notice accepted.',
  capability_required: 'Permission revoked or admin session expired. Sign in again.',
  server_maintenance: 'The server is shutting down; no notice was scheduled.',
  operation_conflict: 'This operation ID was already used for a different notice.',
  operation_capacity: 'The process-local receipt limit has been reached.',
  notice_capacity: 'Too many scheduled notices. Cancel one before adding another.',
  unknown_notice: 'That notice was already delivered or cancelled.',
};
const messageOutcomes: Record<MessageResult['code'], string> = {
  ok: 'Message delivered.',
  stale_target: 'This session is no longer online or the server restarted. Refresh the player list.',
  capability_required: 'Permission revoked or admin session expired. Sign in again.',
  operation_conflict: 'This operation ID was already used for a different message.',
  operation_capacity: 'The process-local receipt limit has been reached.',
};
// Mirrors control.ValidNoticeText: the 7.48 client uses a legacy code page.
export const validNoticeText = (value: string) => value.length >= 1 && value.length <= 94 && value === value.trim() && /^[\x20-\x7e]+$/.test(value);

export function setupStaffNotices(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const text = (id: string, value: string) => { el(id).textContent = value; };
  const date = (value: string) => new Date(value).toLocaleString('en-US', { dateStyle: 'short', timeStyle: 'medium' });
  const path = '/staff/notices';
  let pending: Command | undefined;
  let busy = false;
  let generation = 0;
  let target: { player: Player; epoch: string } | undefined;
  let messageBusy = false;

  function controls() {
    el<HTMLButtonElement>('notice-refresh').disabled = busy;
    el<HTMLButtonElement>('notice-submit').disabled = busy || !!pending;
    el('notice-retry').hidden = !pending;
    el<HTMLButtonElement>('notice-retry').disabled = busy;
    document.querySelectorAll<HTMLButtonElement>('.notice-cancel').forEach(button => { button.disabled = busy; });
    el<HTMLFieldSetElement>('player-message-settings').disabled = messageBusy || !target;
    el<HTMLButtonElement>('player-message-submit').disabled = messageBusy || !target;
  }

  function render(value: View) {
    if (value.version !== 1 || !Array.isArray(value.notices)) throw new Error('Invalid notice snapshot');
    const rows = value.notices.map(notice => {
      const row = document.createElement('tr');
      const every = notice.repeatEverySeconds ? ` · every ${Math.round(notice.repeatEverySeconds / 60)} min` : '';
      for (const content of [date(notice.nextAt), notice.message, `${notice.remaining}${every}`, notice.actor]) {
        const cell = document.createElement('td'); cell.textContent = content; row.append(cell);
      }
      const action = document.createElement('td');
      const cancel = document.createElement('button'); cancel.type = 'button'; cancel.className = 'button subtle notice-cancel'; cancel.textContent = 'Cancel';
      cancel.addEventListener('click', () => void cancelNotice(notice.id)); action.append(cancel); row.append(action);
      return row;
    });
    if (rows.length) el('notice-rows').replaceChildren(...rows);
    else {
      const cell = document.createElement('td'); cell.colSpan = 5; cell.className = 'empty'; cell.textContent = 'No scheduled notices.';
      const row = document.createElement('tr'); row.append(cell); el('notice-rows').replaceChildren(row);
    }
    controls();
  }

  async function refresh() {
    if (busy) return;
    const current = generation;
    busy = true; controls();
    try {
      const value = await api<View>(path);
      if (current !== generation) return;
      render(value);
    } catch (error) {
      if (current !== generation) return;
      text('notice-message-status', explain(error));
    } finally { if (current === generation) { busy = false; controls(); } }
  }

  async function send() {
    if (!pending || busy) return;
    const command = pending;
    const current = generation;
    busy = true; controls(); text('notice-message-status', 'Waiting for server confirmation…');
    try {
      const result = await api<Result>(path, 'POST', command);
      if (current !== generation) return;
      if (result.operationId !== command.operationId || !Object.hasOwn(noticeOutcomes, result.code)) throw new Error('Invalid notice receipt');
      pending = undefined;
      render(result.view);
      const delivered = command.delaySeconds === 0 && result.code === 'ok' ? ` Shown to ${result.delivered} online player${result.delivered === 1 ? '' : 's'}.` : '';
      text('notice-message-status', `${noticeOutcomes[result.code]}${delivered}${result.replayed ? ' Receipt recovered; it was not sent twice.' : ''}`);
      if (result.code === 'ok') el<HTMLInputElement>('notice-message').value = '';
    } catch (error) {
      if (current !== generation) return;
      text('notice-message-status', `${explain(error)} Outcome not confirmed. Use “Check / retry” to resend the same notice (${command.operationId}).`);
    } finally { if (current === generation) { busy = false; controls(); } }
  }

  async function cancelNotice(id: number) {
    if (busy) return;
    const current = generation;
    busy = true; controls();
    try {
      const result = await api<Result>(`${path}/${id}/cancel`, 'POST', { version: 1, operationId: crypto.randomUUID() });
      if (current !== generation) return;
      if (!Object.hasOwn(noticeOutcomes, result.code)) throw new Error('Invalid notice receipt');
      render(result.view);
      text('notice-message-status', result.code === 'ok' ? 'Scheduled notice cancelled.' : noticeOutcomes[result.code]);
    } catch (error) {
      if (current !== generation) return;
      text('notice-message-status', `${explain(error)} Refresh the schedule to confirm.`);
    } finally { if (current === generation) { busy = false; controls(); } }
  }

  el<HTMLFormElement>('notice-form').addEventListener('submit', event => {
    event.preventDefault();
    if (pending || busy) return;
    const message = el<HTMLInputElement>('notice-message').value.trim();
    const delay = Number(el<HTMLInputElement>('notice-delay').value);
    const count = Number(el<HTMLInputElement>('notice-count').value);
    const every = Number(el<HTMLInputElement>('notice-every').value);
    if (!validNoticeText(message)) { text('notice-message-status', 'Use 1 to 94 printable ASCII characters (no accents or emoji).'); return; }
    if (!Number.isInteger(delay) || delay < 0 || delay > 1440 || !Number.isInteger(count) || count < 1 || count > 100 ||
      (count > 1 && (!Number.isInteger(every) || every < 1 || every > 1440))) { text('notice-message-status', 'Check the delay, deliveries and interval.'); return; }
    const when = delay ? `in ${delay} minute${delay === 1 ? '' : 's'}` : 'now';
    const repeat = count > 1 ? `, ${count} times every ${every} minute${every === 1 ? '' : 's'}` : '';
    if (!window.confirm(`Send this notice to every online player ${when}${repeat}?\n\n${message}`)) return;
    pending = { version: 1, operationId: crypto.randomUUID(), message, delaySeconds: delay * 60, repeatCount: count, repeatEverySeconds: count > 1 ? every * 60 : 0 };
    void send();
  });
  el('notice-retry').addEventListener('click', () => void send());
  el('notice-refresh').addEventListener('click', () => void refresh());

  el<HTMLFormElement>('player-message-form').addEventListener('submit', async event => {
    event.preventDefault();
    if (!target || messageBusy) return;
    const message = el<HTMLInputElement>('player-message-text').value.trim();
    if (!validNoticeText(message)) { text('player-message-status', 'Use 1 to 94 printable ASCII characters (no accents or emoji).'); return; }
    const { player, epoch } = target;
    const command: MessageCommand = { version: 1, operationId: crypto.randomUUID(), epoch, uid: player.uid,
      account: player.account.toLowerCase(), expectedSessionId: player.sessionId, message };
    const current = generation;
    messageBusy = true; controls(); text('player-message-status', 'Waiting for server confirmation…');
    try {
      const result = await api<MessageResult>(`/staff/players/${encodeURIComponent(player.uid)}/message`, 'POST', command);
      if (current !== generation) return;
      if (result.operationId !== command.operationId || !Object.hasOwn(messageOutcomes, result.code)) throw new Error('Invalid message receipt');
      text('player-message-status', messageOutcomes[result.code]);
      if (result.code === 'ok') el<HTMLInputElement>('player-message-text').value = '';
    } catch (error) {
      if (current !== generation) return;
      text('player-message-status', `${explain(error)} Delivery not confirmed.`);
    } finally { if (current === generation) { messageBusy = false; controls(); } }
  });

  return {
    refresh,
    selectPlayer(player: Player, epoch?: string) {
      if (!epoch || !/^[a-f0-9]{32}$/.test(epoch) || !Number.isSafeInteger(player.sessionId) || player.sessionId < 1) {
        text('player-message-status', 'Refresh data from the integrated server to message players.'); return;
      }
      target = { player: { ...player }, epoch };
      text('player-message-target', `${player.name} · ${player.account} · session ${player.sessionId}`);
      text('player-message-status', ''); controls();
      document.getElementById('player-message-form')?.scrollIntoView({ block: 'center' });
      el<HTMLInputElement>('player-message-text').focus();
    },
    reset() {
      generation++; pending = undefined; busy = false; target = undefined; messageBusy = false;
      el<HTMLInputElement>('notice-message').value = ''; el<HTMLInputElement>('player-message-text').value = '';
      text('notice-message-status', ''); text('player-message-status', '');
      text('player-message-target', 'Select “Message” from the online player list.');
      const cell = document.createElement('td'); cell.colSpan = 5; cell.className = 'empty'; cell.textContent = 'Schedule not queried.';
      const row = document.createElement('tr'); row.append(cell); el('notice-rows').replaceChildren(row);
      controls();
    },
  };
}
