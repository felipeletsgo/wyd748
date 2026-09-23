import type { components } from '../lib/api.generated';
type Player = components['schemas']['Player'];
type Command = components['schemas']['TeleportCommand'];
type Result = components['schemas']['TeleportResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;
const outcomes: Record<Result['code'], string> = {
  ok: 'Player teleported. Refresh the list to see the current state.',
  persistence_failed: 'Could not save. The previous position was restored; check the server log.',
  player_busy: 'Player is dead, trading, using a ghost shop, or persistence is blocked. No teleport was performed.',
  restricted_area: 'Player is in an instance or war arena. Use the event exit flow.',
  destination_unavailable: 'Destination is blocked or has no nearby free space. No teleport was performed.',
  stale_target: 'This session is no longer available or the server restarted. Refresh the list and select the player again.',
  invalid_command: 'Invalid command. Refresh the panel and select the player again.',
  capability_required: 'Permission revoked or admin session expired. Sign in again.',
  server_maintenance: 'Server under maintenance. No new teleport was performed.',
  operation_conflict: 'This identifier is already linked to another command. Check the server logs.',
  operation_capacity: 'Receipt limit reached for this run. No teleport was performed.',
};

export function setupPlayerTeleport(api: API, explain: (error: unknown) => string) {
  const form = document.getElementById('teleport-form') as HTMLFormElement;
  const destination = document.getElementById('teleport-destination') as HTMLSelectElement;
  const destinations: Record<Command['destination'], string> = { armia: 'Armia', azran: 'Azran', erion: 'Erion', nippleheim: 'Nippleheim' };
  const reason = document.getElementById('teleport-reason') as HTMLInputElement;
  const submit = document.getElementById('teleport-submit') as HTMLButtonElement;
  const retry = document.getElementById('teleport-retry') as HTMLButtonElement;
  const targetLabel = document.getElementById('teleport-target')!;
  const message = document.getElementById('teleport-message')!;
  let selected: { player: Player; epoch: string } | undefined;
  let pending: Command | undefined;
  let busy = false;
  let generation = 0;
  function controls() {
    reason.disabled = busy || !!pending || !selected;
    submit.disabled = reason.disabled;
    destination.disabled = reason.disabled;
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
      const result = await api<Result>(`/staff/players/${encodeURIComponent(command.uid)}/teleport`, 'POST', command);
      if (ownGeneration !== generation) return;
      if (result.operationId !== command.operationId || !Object.hasOwn(outcomes, result.code) || typeof result.replayed !== 'boolean') throw new Error('Invalid receipt');
      if (!Number.isInteger(result.x) || !Number.isInteger(result.y) || (result.code === 'ok' ? result.x < 1 || result.x > 4095 || result.y < 1 || result.y > 4095 : result.x !== 0 || result.y !== 0)) throw new Error('Invalid coordinates');
      pending = undefined;
      selected = undefined;
      message.textContent = `${outcomes[result.code]}${result.code === 'ok' ? ` X: ${result.x} · Y: ${result.y}.` : ''}${result.replayed ? ' Receipt recovered; the action was not repeated.' : ''} Operation: ${result.operationId}`;
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
    const chosen = destination.value as Command['destination'];
    if (!Object.hasOwn(destinations, chosen)) { message.textContent = 'Select a valid city.'; return; }
    if (!window.confirm(`Teleport ${player.name} (${player.account}), session ${player.sessionId}, to ${destinations[chosen]}?\nReason: ${justification}\nThe server will reject unavailable states and destinations.`)) return;
    pending = { version: 1, operationId: crypto.randomUUID(), epoch, uid: player.uid, account: player.account.toLowerCase(), expectedSessionId: player.sessionId, reason: justification, destination: chosen };
    void send();
  });
  retry.addEventListener('click', () => { void send(); });
  return {
    select(player: Player, epoch?: string) {
      if (pending || busy) { message.textContent = 'Resolve the pending operation before selecting another player.'; return; }
      if (!epoch || !/^[a-f0-9]{32}$/.test(epoch) || !Number.isSafeInteger(player.sessionId) || player.sessionId < 1) { message.textContent = 'Refresh data from the integrated server to enable teleportation.'; return; }
      selected = { player: { ...player }, epoch };
      targetLabel.textContent = `${player.name} · ${player.account} · UID ${player.uid} · session ${player.sessionId}`;
      reason.value = ''; destination.value = 'armia'; message.textContent = ''; controls();
      document.getElementById('player-teleport')!.scrollIntoView({ block: 'center' });
      reason.focus();
    },
    reset() {
      generation++; pending = undefined; selected = undefined; busy = false;
      reason.value = ''; destination.value = 'armia'; targetLabel.textContent = 'No player selected.'; message.textContent = ''; controls();
    },
  };
}
