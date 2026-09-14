import type { components } from '../lib/api.generated';
type Player = components['schemas']['Player'];
type Command = components['schemas']['KickCommand'];
type Result = components['schemas']['KickResult'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;
const outcomes: Record<Result['code'], string> = {
  ok: 'Jogador desconectado. Atualize a lista para consultar o estado atual.',
  disconnected_persistence_failed: 'Jogador desconectado, mas houve falha ao salvar. Consulte o log do servidor antes de outras ações.',
  stale_target: 'Essa sessão não está mais disponível ou o servidor reiniciou. Atualize a lista e selecione novamente.',
  invalid_command: 'Comando inválido. Atualize o painel e selecione novamente.',
  capability_required: 'Permissão revogada ou sessão administrativa expirada. Entre novamente.',
  server_maintenance: 'Servidor em manutenção. Nenhuma nova desconexão realizada.',
  operation_conflict: 'Identificador já vinculado a outro comando. Consulte os registros do servidor.',
  operation_capacity: 'Limite de recibos desta execução atingido. Nenhuma desconexão realizada.',
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
    message.textContent = 'Aguardando confirmação do servidor…';
    try {
      const result = await api<Result>(`/staff/players/${encodeURIComponent(command.uid)}/kick`, 'POST', command);
      if (ownGeneration !== generation) return;
      if (result.operationId !== command.operationId || !Object.hasOwn(outcomes, result.code) || typeof result.replayed !== 'boolean') throw new Error('Invalid receipt');
      pending = undefined;
      selected = undefined;
      message.textContent = `${outcomes[result.code]}${result.replayed ? ' Recibo recuperado; a ação não foi repetida.' : ''} Operação: ${result.operationId}`;
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
    if (!window.confirm(`Desconectar ${player.name} (${player.account}), sessão ${player.sessionId}?\nMotivo: ${justification}\nIsso não é um banimento.`)) return;
    pending = { version: 1, operationId: crypto.randomUUID(), epoch, uid: player.uid, account: player.account.toLowerCase(), expectedSessionId: player.sessionId, reason: justification };
    void send();
  });
  retry.addEventListener('click', () => { void send(); });
  return {
    select(player: Player, epoch?: string) {
      if (pending || busy) { message.textContent = 'Resolva a operação pendente antes de selecionar outro jogador.'; return; }
      if (!epoch || !/^[a-f0-9]{32}$/.test(epoch) || !Number.isSafeInteger(player.sessionId) || player.sessionId < 1) { message.textContent = 'Atualize os dados no servidor integrado para habilitar a moderação.'; return; }
      selected = { player: { ...player }, epoch };
      targetLabel.textContent = `${player.name} · ${player.account} · UID ${player.uid} · sessão ${player.sessionId}`;
      reason.value = ''; message.textContent = ''; controls();
      document.getElementById('moderation')!.scrollIntoView({ block: 'center' });
      reason.focus();
    },
    reset() {
      generation++; pending = undefined; selected = undefined; busy = false;
      reason.value = ''; targetLabel.textContent = 'Nenhum jogador selecionado.'; message.textContent = ''; controls();
    },
  };
}
