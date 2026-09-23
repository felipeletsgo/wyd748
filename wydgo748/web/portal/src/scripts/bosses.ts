import type { components } from '../lib/api.generated';
type Status = components['schemas']['BossesStatus'];
type Boss = components['schemas']['BossStatus'];
type Command = components['schemas']['BossesCommand'];
type Result = components['schemas']['BossesResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

export function setupBosses(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const text = (id: string, value: string) => { el(id).textContent = value; };
  let status: Status | undefined;
  let pending: Command | undefined;
  let busy = false;
  let generation = 0;
  const messages: Record<string, string> = {
    ok: 'Boss summoned by the server.',
    stale_boss: 'Boss state changed. Check it again before sending another operation.',
    boss_alive: 'The boss is already alive and was not duplicated.',
    unknown_boss: 'This boss is no longer in the loaded configuration.',
    spawn_failed: 'The server could not spawn the boss. The previous respawn cycle was preserved.',
    invalid_command: 'Invalid operation. Check the reason and query again.',
    operation_conflict: 'This identifier was already used for another operation. Check the current state.',
    operation_capacity: 'Operation limit reached for this run. Restart the server before further admin summons.',
    capability_required: 'This session no longer has permission to summon bosses.',
    server_maintenance: 'The server is under maintenance and does not accept changes.',
  };

  function setControls() {
    el<HTMLButtonElement>('bosses-refresh').disabled = busy;
    el<HTMLButtonElement>('bosses-retry').hidden = !pending;
    el<HTMLButtonElement>('bosses-retry').disabled = busy;
    el<HTMLInputElement>('bosses-reason').disabled = busy || !!pending;
    document.querySelectorAll<HTMLButtonElement>('[data-boss-summon]').forEach(button => {
      const boss = status?.bosses.find(item => item.id === button.dataset.bossSummon);
      button.disabled = busy || !!pending || !boss || boss.alive;
    });
  }

  function respawnLabel(boss: Boss) {
    if (boss.alive) return '—';
    if (!boss.respawnAt) return 'Not scheduled';
    const at = new Date(boss.respawnAt);
    const remaining = Math.max(0, at.getTime() - Date.now());
    const totalSeconds = Math.ceil(remaining / 1000);
    const minutes = Math.floor(totalSeconds / 60);
    const seconds = totalSeconds % 60;
    return `${at.toLocaleString('en-US')} · in ${minutes}m ${seconds}s`;
  }

  function appendCell(row: HTMLTableRowElement, value: string, small?: string) {
    const cell = document.createElement('td');
    cell.textContent = value;
    if (small) {
      const detail = document.createElement('small');
      detail.textContent = small;
      cell.appendChild(detail);
    }
    row.appendChild(cell);
    return cell;
  }

  function render(value: Status) {
    status = value;
    const body = el<HTMLTableSectionElement>('bosses-body');
    body.replaceChildren();
    if (!value.bosses.length) {
      const row = document.createElement('tr');
      const cell = document.createElement('td');
      cell.className = 'empty'; cell.colSpan = 7; cell.textContent = 'No bosses configured.';
      row.appendChild(cell); body.appendChild(row);
    }
    for (const boss of value.bosses) {
      const row = document.createElement('tr');
      appendCell(row, boss.name || boss.id, boss.id);
      appendCell(row, boss.alive ? 'Alive' : 'Dead', boss.alive ? `Mob ${boss.mobId}` : `Revision ${boss.revision}`);
      appendCell(row, boss.alive ? `${boss.hp.toLocaleString('en-US')} / ${boss.maxHp.toLocaleString('en-US')}` : `0 / ${boss.maxHp.toLocaleString('en-US')}`);
      appendCell(row, boss.alive ? `${boss.x}, ${boss.y}` : '—');
      appendCell(row, `${boss.spawnX}, ${boss.spawnY}`);
      appendCell(row, respawnLabel(boss));
      const action = appendCell(row, '');
      const button = document.createElement('button');
      button.type = 'button'; button.className = 'button subtle boss-summon';
      button.dataset.bossSummon = boss.id; button.textContent = boss.alive ? 'Already alive' : 'Summon boss';
      action.replaceChildren(button);
      body.appendChild(row);
    }
    text('bosses-asof', `Snapshot from ${new Date(value.asOf).toLocaleString('en-US')}. State is not updated automatically.`);
    setControls();
  }

  async function refresh() {
    if (busy) return;
    busy = true; setControls();
    const current = generation;
    try {
      const value = await api<Status>('/staff/bosses');
      if (current !== generation) return;
      if (value.version !== 1 || value.epoch.length !== 32 || !Array.isArray(value.bosses)) throw new Error('Invalid bosses snapshot');
      render(value);
      if (!pending) text('bosses-message', 'Boss state retrieved.');
    } catch (error) {
      if (current !== generation) return;
      status = undefined;
      text('bosses-message', explain(error));
    } finally { if (current === generation) { busy = false; setControls(); } }
  }

  async function send() {
    if (!pending || busy) return;
    const command = pending;
    const current = generation;
    busy = true; setControls();
    text('bosses-message', 'Waiting for server confirmation…');
    try {
      const result = await api<Result>('/staff/bosses', 'POST', command);
      if (current !== generation) return;
      if (result.operationId !== command.operationId || !messages[result.code]) throw new Error('Invalid bosses receipt');
      pending = undefined;
      render(result.status);
      text('bosses-message', `${messages[result.code]}${result.replayed ? ' Outcome recovered without repeating the summon.' : ''}`);
      if (result.code === 'stale_boss') void refresh();
    } catch (error) {
      if (current !== generation) return;
      text('bosses-message', `${explain(error)} Outcome not confirmed. Use the retry button; it preserves the same identifier and does not duplicate the summon.`);
    } finally { if (current === generation) { busy = false; setControls(); } }
  }

  function prepare(bossId: string) {
    if (busy || pending || !status) return;
    const boss = status.bosses.find(item => item.id === bossId);
    if (!boss || boss.alive) return;
    const reason = el<HTMLInputElement>('bosses-reason');
    if (!reason.reportValidity()) return;
    const command: Command = { version: 1, operationId: crypto.randomUUID(), epoch: status.epoch, bossId: boss.id,
      expectedRevision: boss.revision, action: 'summon', reason: reason.value.trim() };
    if (!window.confirm(`Summon ${boss.name || boss.id} at ${boss.spawnX}, ${boss.spawnY}?\nReason: ${command.reason}`)) return;
    pending = command;
    void send();
  }

  el('bosses-body').addEventListener('click', event => {
    const target = event.target;
    if (target instanceof HTMLButtonElement && target.dataset.bossSummon) prepare(target.dataset.bossSummon);
  });
  el('bosses-refresh').addEventListener('click', () => void refresh());
  el('bosses-retry').addEventListener('click', () => void send());

  return { refresh, reset() {
    generation++; status = undefined; pending = undefined; busy = false;
    el('bosses-body').innerHTML = '<tr><td class="empty" colspan="7"><span>◇</span>Waiting for a query<small>Use Check bosses to load the current state.</small></td></tr>';
    text('bosses-asof', 'No automatic updates.'); text('bosses-message', '');
    setControls();
  } };
}
