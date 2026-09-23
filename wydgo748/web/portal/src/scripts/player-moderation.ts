import type { components } from '../lib/api.generated';
type Player = components['schemas']['Player'];
type Command = components['schemas']['KickCommand'];
type Result = components['schemas']['KickResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;
const outcomes: Record<Result['code'], string> = {
  ok: 'Player disconnected. Refresh the list to see the current state.',
  disconnected_persistence_failed: 'Player disconnected, but saving failed. Check the server log before taking further action.',
  stale_target: 'This session is no longer available or the server restarted. Refresh the list and select the player again.',
  invalid_command: 'Invalid command. Refresh the panel and select the player again.',
  capability_required: 'Permission revoked or admin session expired. Sign in again.',
  server_maintenance: 'Server under maintenance. No new disconnection was performed.',
  operation_conflict: 'This identifier is already linked to another command. Check the server logs.',
  operation_capacity: 'Receipt limit reached for this run. No disconnection was performed.',
};

export function setupPlayerModeration(api: API, explain: (error: unknown) => string) {
  const form = document.getElementById('kick-form') as HTMLFormElement;
  const reason = document.getElementById('kick-reason') as HTMLInputElement;
  const submit = document.getElementById('kick-submit') as HTMLButtonElement;
  const retry = document.getElementById('kick-retry') as HTMLButtonElement;
  const targetLabel = document.getElementById('kick-target')!;
  const message = document.getElementById('kick-message')!;
  let selected: { player: Player; epoch: string } | undefined;
  let pending: Command | undefined;
  let busy = false;
  let generation = 0;
  function controls() {
    reason.disabled = busy || !!pending || !selected;
    submit.disabled = reason.disabled;
    retry.hidden = !pending;
    retry.disabled = busy;
  }
  async function send() {
    if (!pending || busy) return;
    const command = pending;
    const ownGeneration = generation;
    busy = true; controls();
    message.textContent = 'Waiting for server confirmation…';
    try {
      const result = await api<Result>(`/staff/players/${encodeURIComponent(command.uid)}/kick`, 'POST', command);
      if (ownGeneration !== generation) return;
      if (result.operationId !== command.operationId || !Object.hasOwn(outcomes, result.code) || typeof result.replayed !== 'boolean') throw new Error('Invalid receipt');
      pending = undefined;
      selected = undefined;
      message.textContent = `${outcomes[result.code]}${result.replayed ? ' Receipt recovered; the action was not repeated.' : ''} Operation: ${result.operationId}`;
    } catch (error) {
      if (ownGeneration !== generation) return;
      // Never replace an uncertain command with a new ID or a new target.
      message.textContent = `${explain(error)} Outcome not confirmed. Use “Check / retry” with the same target and operation (${command.operationId}).`;
    } finally {
      if (ownGeneration === generation) { busy = false; controls(); }
    }
  }
  form.addEventListener('submit', event => {
    event.preventDefault();
    if (!selected || pending || busy || !form.reportValidity()) return;
    const justification = reason.value.trim();
    if ([...justification].length < 3 || /[\u0000-\u001f\u007f-\u009f]/u.test(justification)) { message.textContent = 'Enter a valid reason with 3 to 200 characters and no control characters.'; return; }
    const { player, epoch } = selected;
    if (!window.confirm(`Disconnect ${player.name} (${player.account}), session ${player.sessionId}?\nReason: ${justification}\nThis is not a ban.`)) return;
    pending = { version: 1, operationId: crypto.randomUUID(), epoch, uid: player.uid, account: player.account.toLowerCase(), expectedSessionId: player.sessionId, reason: justification };
    void send();
  });
  retry.addEventListener('click', () => { void send(); });
  return {
    select(player: Player, epoch?: string) {
      if (pending || busy) { message.textContent = 'Resolve the pending operation before selecting another player.'; return; }
      if (!epoch || !/^[a-f0-9]{32}$/.test(epoch) || !Number.isSafeInteger(player.sessionId) || player.sessionId < 1) { message.textContent = 'Refresh data from the integrated server to enable moderation.'; return; }
      selected = { player: { ...player }, epoch };
      targetLabel.textContent = `${player.name} · ${player.account} · UID ${player.uid} · session ${player.sessionId}`;
      reason.value = ''; message.textContent = ''; controls();
      document.getElementById('moderation')!.scrollIntoView({ block: 'center' });
      reason.focus();
    },
    reset() {
      generation++; pending = undefined; selected = undefined; busy = false;
      reason.value = ''; targetLabel.textContent = 'No player selected.'; message.textContent = ''; controls();
    },
  };
}
