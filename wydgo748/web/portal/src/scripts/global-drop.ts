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
    ok: 'Operação confirmada pelo servidor.',
    stale_event: 'O evento ou o processo mudou. Confira o estado atual antes de enviar outra operação.',
    event_active: 'Já existe um evento ativo. Encerre-o antes de iniciar outro.',
    event_inactive: 'Este evento já está encerrado.',
    unknown_item: 'ID de item inexistente no catálogo carregado pelo servidor.',
    invalid_command: 'Configuração inválida. Confira item, chance, limites e motivo.',
    operation_conflict: 'Identificador já utilizado com outra configuração. Consulte o estado atual.',
    operation_capacity: 'Limite de operações atingido. Novos eventos estão bloqueados; o evento ativo ainda pode ser encerrado.',
    event_id_unavailable: 'Não foi possível gerar o identificador do evento. Tente uma nova operação.',
    capability_required: 'A sessão não tem mais permissão para controlar este evento.',
    server_maintenance: 'O servidor está em manutenção e não aceita alterações.',
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
    const ended = { time: 'prazo atingido', quantity: 'quantidade atingida', manual: 'encerrado pela equipe', '': '' };
    text('event-state', value.active ? 'Ativo' : value.eventId ? `Encerrado · ${ended[value.endedBy] ?? value.endedBy}` : 'Nenhum evento iniciado');
    text('event-item', value.itemId ? `${value.itemId} · ${value.itemName} · ${value.ratePercent}%` : '—');
    text('event-count', `${value.dropped.toLocaleString('pt-BR')} / ${value.maxDrops ? value.maxDrops.toLocaleString('pt-BR') : 'sem limite de quantidade'}`);
    text('event-end', value.endsAt ? new Date(value.endsAt).toLocaleString('pt-BR') : 'Sem limite de tempo');
    text('event-asof', `Leitura de ${new Date(value.asOf).toLocaleString('pt-BR')}${value.actor ? ` · iniciado por ${value.actor}` : ''}. Use Consultar evento para atualizar.`);
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
      if (!pending) text('event-message', 'Estado consultado.');
    } catch (error) {
      if (current !== generation) return;
      status = undefined;
      text('event-state', 'Dados do evento indisponíveis');
      text('event-message', explain(error));
    } finally { if (current === generation) { busy = false; buttons(); } }
  }
  async function send() {
    if (!pending || busy) return;
    const command = pending;
    const current = generation;
    busy = true; buttons();
    text('event-message', 'Aguardando confirmação do servidor…');
    try {
      const result = await api<Result>('/staff/events/global-drop', 'POST', command);
      if (current !== generation) return;
      if (result.operationId !== command.operationId || !messages[result.code]) throw new Error('Invalid event receipt');
      pending = undefined;
      render(result.status);
      text('event-message', `${messages[result.code]}${result.replayed ? ' Resultado recuperado sem repetir a operação.' : ''}`);
    } catch (error) {
      if (current !== generation) return;
      text('event-message', `${explain(error)} O resultado não foi confirmado. Use o botão de repetir solicitação, que mantém o mesmo identificador; não recarregue a página.`);
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
      if (!c.maxDrops && !c.durationSeconds) { text('event-message', 'Informe um limite de quantidade ou de tempo.'); return; }
    }
    const detail = action === 'start' ? `Iniciar drop global do item ${c.itemId} com ${c.ratePercent}% de chance?\nQuantidade: ${c.maxDrops || 'sem limite'} · Minutos: ${c.durationSeconds / 60 || 'sem limite'}` : `Encerrar o drop global do item ${status.itemId} (${status.itemName})?`;
    if (!window.confirm(`${detail}\nMotivo: ${c.reason}`)) return;
    pending = c; void send();
  }
  el<HTMLFormElement>('event-form').addEventListener('submit', event => { event.preventDefault(); prepare('start'); });
  el('event-stop').addEventListener('click', () => prepare('stop'));
  el('event-refresh').addEventListener('click', () => void refresh());
  el('event-retry').addEventListener('click', () => void send());
  return { refresh, reset() {
    generation++; status = undefined; pending = undefined; busy = false;
    text('event-state', 'Aguardando consulta');
    for (const id of ['event-item', 'event-count', 'event-end']) text(id, '—');
    text('event-asof', 'Sem atualização automática.'); text('event-message', '');
    buttons();
  } };
}
