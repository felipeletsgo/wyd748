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
    ok: 'Operação confirmada pelo servidor.',
    stale_event: 'O evento ou o processo mudou. Confira o estado atual antes de enviar outra operação.',
    event_active: 'Já existe um evento ativo. Encerre-o antes de iniciar outro.',
    event_inactive: 'Este evento já está encerrado.',
    unknown_item: 'ID de item inexistente no catálogo carregado pelo servidor.',
    invalid_command: 'Configuração inválida. Confira item, quantidade, duração e motivo.',
    operation_conflict: 'Identificador já utilizado com outra configuração. Consulte o estado atual.',
    operation_capacity: 'Limite de operações deste processo atingido. Novos eventos estão bloqueados; o evento ativo ainda pode ser encerrado.',
    capability_required: 'A sessão não tem mais permissão para controlar este evento.',
    event_id_unavailable: 'Não foi possível gerar o identificador do evento. Tente uma nova operação.',
    server_maintenance: 'O servidor está em manutenção e não aceita alterações.',
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
    const ended = { time: 'prazo atingido', quantity: 'quantidade atingida', manual: 'encerrado pela equipe', '': '' };
    text('quiz-state', value.active ? 'Ativo' : value.eventId ? `Encerrado · ${ended[value.endedBy] ?? value.endedBy}` : 'Nenhum evento iniciado');
    text('quiz-item', value.itemId ? `${value.itemId} · ${value.itemName} · ${value.rewardQuantity} unidade(s) por acerto` : '—');
    text('quiz-count', `${value.rounds} perguntas · ${value.correct} acertos · ${value.rewarded} premiados · ${value.deliveryFailures} falhas de entrega`);
    text('quiz-end', value.endsAt ? new Date(value.endsAt).toLocaleString('pt-BR') : '—');
    text('quiz-next', value.active && value.nextRoundAt ? new Date(value.nextRoundAt).toLocaleTimeString('pt-BR') : '—');
    text('quiz-asof', `Leitura de ${new Date(value.asOf).toLocaleString('pt-BR')}${value.actor ? ` · iniciado por ${value.actor}` : ''}. Use Consultar evento para atualizar.`);
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
      if (!pending) text('quiz-message', 'Estado consultado.');
    } catch (error) {
      if (current !== generation) return;
      status = undefined;
      text('quiz-state', 'Dados do evento indisponíveis');
      text('quiz-message', explain(error));
    } finally { if (current === generation) { busy = false; buttons(); } }
  }
  async function send() {
    if (!pending || busy) return;
    const command = pending;
    const current = generation;
    busy = true; buttons();
    text('quiz-message', 'Aguardando confirmação do servidor…');
    try {
      const result = await api<Result>('/staff/events/quiz', 'POST', command);
      if (current !== generation) return;
      if (result.operationId !== command.operationId || !messages[result.code]) throw new Error('Invalid event receipt');
      pending = undefined;
      render(result.status);
      text('quiz-message', `${messages[result.code]}${result.replayed ? ' Resultado recuperado sem repetir a operação.' : ''}`);
    } catch (error) {
      if (current !== generation) return;
      text('quiz-message', `${explain(error)} O resultado não foi confirmado. Use o botão de repetir solicitação, que mantém o mesmo identificador; não recarregue a página.`);
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
    const detail = action === 'start' ? `Iniciar quiz por ${c.durationSeconds / 60} minutos?\nRecompensa por acerto: ${c.rewardQuantity} unidade(s) do item ${c.itemId}. Uma pergunta por minuto, 10 segundos para responder.` : `Encerrar o quiz e fechar a pergunta ativa?`;
    if (!window.confirm(`${detail}\nMotivo: ${c.reason}`)) return;
    pending = c; void send();
  }
  el<HTMLFormElement>('quiz-form').addEventListener('submit', event => { event.preventDefault(); prepare('start'); });
  el('quiz-stop').addEventListener('click', () => prepare('stop'));
  el('quiz-refresh').addEventListener('click', () => void refresh());
  el('quiz-retry').addEventListener('click', () => void send());
  return { refresh, reset() {
    generation++; status = undefined; pending = undefined; busy = false;
    text('quiz-state', 'Aguardando consulta');
    for (const id of ['quiz-item', 'quiz-count', 'quiz-end', 'quiz-next']) text(id, '—');
    text('quiz-asof', 'Sem atualização automática.'); text('quiz-message', '');
    buttons();
  } };
}
