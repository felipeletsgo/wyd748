import type { components } from '../lib/api.generated';
type Player = components['schemas']['Player'];
type Command = components['schemas']['TeleportCommand'];
type Result = components['schemas']['TeleportResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;
const outcomes: Record<Result['code'], string> = {
  ok: 'Jogador teleportado. Atualize a lista para consultar o estado atual.',
  persistence_failed: 'Não foi possível salvar. A posição anterior foi restaurada; consulte o log do servidor.',
  player_busy: 'Jogador morto, em troca, loja fantasma ou com persistência bloqueada. Nenhum teleporte realizado.',
  restricted_area: 'Jogador em instância ou arena de guerra. Use o fluxo de saída do evento.',
  destination_unavailable: 'Destino bloqueado ou sem espaço livre próximo. Nenhum teleporte realizado.',
  stale_target: 'Essa sessão não está mais disponível ou o servidor reiniciou. Atualize a lista e selecione novamente.',
  invalid_command: 'Comando inválido. Atualize o painel e selecione novamente.',
  capability_required: 'Permissão revogada ou sessão administrativa expirada. Entre novamente.',
  server_maintenance: 'Servidor em manutenção. Nenhum novo teleporte realizado.',
  operation_conflict: 'Identificador já vinculado a outro comando. Consulte os registros do servidor.',
  operation_capacity: 'Limite de recibos desta execução atingido. Nenhum teleporte realizado.',
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
    message.textContent = 'Aguardando confirmação do servidor…';
    try {
      const result = await api<Result>(`/staff/players/${encodeURIComponent(command.uid)}/teleport`, 'POST', command);
      if (ownGeneration !== generation) return;
      if (result.operationId !== command.operationId || !Object.hasOwn(outcomes, result.code) || typeof result.replayed !== 'boolean') throw new Error('Invalid receipt');
      if (!Number.isInteger(result.x) || !Number.isInteger(result.y) || (result.code === 'ok' ? result.x < 1 || result.x > 4095 || result.y < 1 || result.y > 4095 : result.x !== 0 || result.y !== 0)) throw new Error('Invalid coordinates');
      pending = undefined;
      selected = undefined;
      message.textContent = `${outcomes[result.code]}${result.code === 'ok' ? ` X: ${result.x} · Y: ${result.y}.` : ''}${result.replayed ? ' Recibo recuperado; a ação não foi repetida.' : ''} Operação: ${result.operationId}`;
    } catch (error) {
      if (ownGeneration !== generation) return;
      // Never replace an uncertain command with a new ID or a new target.
      message.textContent = `${explain(error)} Resultado não confirmado. Use “Consultar / repetir” mantendo o mesmo alvo e operação (${command.operationId}).`;
    } finally {
      if (ownGeneration === generation) { busy = false; controls(); }
    }
  }
  form.addEventListener('submit', event => {
    event.preventDefault();
    if (!selected || pending || busy || !form.reportValidity()) return;
    const justification = reason.value.trim();
    if ([...justification].length < 3 || /[\u0000-\u001f\u007f-\u009f]/u.test(justification)) { message.textContent = 'Informe um motivo válido com 3 a 200 caracteres, sem caracteres de controle.'; return; }
    const { player, epoch } = selected;
    const chosen = destination.value as Command['destination'];
    if (!Object.hasOwn(destinations, chosen)) { message.textContent = 'Selecione uma cidade válida.'; return; }
    if (!window.confirm(`Teleportar ${player.name} (${player.account}), sessão ${player.sessionId}, para ${destinations[chosen]}?\nMotivo: ${justification}\nO servidor recusará estados e destinos indisponíveis.`)) return;
    pending = { version: 1, operationId: crypto.randomUUID(), epoch, uid: player.uid, account: player.account.toLowerCase(), expectedSessionId: player.sessionId, reason: justification, destination: chosen };
    void send();
  });
  retry.addEventListener('click', () => { void send(); });
  return {
    select(player: Player, epoch?: string) {
      if (pending || busy) { message.textContent = 'Resolva a operação pendente antes de selecionar outro jogador.'; return; }
      if (!epoch || !/^[a-f0-9]{32}$/.test(epoch) || !Number.isSafeInteger(player.sessionId) || player.sessionId < 1) { message.textContent = 'Atualize os dados no servidor integrado para habilitar a moderação.'; return; }
      selected = { player: { ...player }, epoch };
      targetLabel.textContent = `${player.name} · ${player.account} · UID ${player.uid} · sessão ${player.sessionId}`;
      reason.value = ''; destination.value = 'armia'; message.textContent = ''; controls();
      document.getElementById('player-teleport')!.scrollIntoView({ block: 'center' });
      reason.focus();
    },
    reset() {
      generation++; pending = undefined; selected = undefined; busy = false;
      reason.value = ''; destination.value = 'armia'; targetLabel.textContent = 'Nenhum jogador selecionado.'; message.textContent = ''; controls();
    },
  };
}
