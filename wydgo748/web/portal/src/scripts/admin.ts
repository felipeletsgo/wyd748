import type { components } from '../lib/api.generated';
import { setupGlobalDrop } from './global-drop';
import { setupQuiz } from './quiz';

type Session = components['schemas']['Session'];
type Overview = components['schemas']['Overview'];
type Player = components['schemas']['Player'];
type PersistentPlayer = components['schemas']['PersistentPlayer'];
const el = <T extends HTMLElement = HTMLElement>(id: string) => {
  const element = document.getElementById(id);
  if (!element) throw new Error(`Missing UI element: ${id}`);
  return element as T;
};
const text = (id: string, value: string | number) => { el(id).textContent = String(value); };
const date = (value: string) => new Date(value).toLocaleString('pt-BR', { dateStyle: 'short', timeStyle: 'medium' });
let session: Session | undefined;
let snapshot: Overview | undefined;
let busy = false;
let offset = 0;
let search = '';
let inspectedUID: string | undefined;
let persistentRequest = 0;

class APIError extends Error {
  constructor(public status: number, public code: string) { super(code); }
}
const messages: Record<string, string> = {
  invalid_credentials: 'Conta, senha ou PIN administrativo inválido, ou acesso não autorizado.',
  invalid_csrf: 'A sessão de acesso expirou. Tente entrar novamente.',
  authentication_required: 'Sua sessão expirou. Entre novamente.',
  staff_revoked: 'A autorização desta conta mudou. Entre novamente ou fale com o responsável.',
  capability_required: 'Sua conta não tem as duas permissões de leitura exigidas por este painel.',
  rate_limited: 'Limite de consultas atingido. Aguarde um minuto e tente novamente.',
  control_busy: 'O servidor está atendendo outra consulta. Aguarde um instante e tente novamente.',
  invalid_query: 'Busca inválida. Use até 48 caracteres Unicode.',
  invalid_target: 'O jogador selecionado não possui uma identidade persistente válida.',
  persistent_not_found: 'O estado persistido desse personagem não foi encontrado.',
  persistent_unavailable: 'Não foi possível ler o estado persistido no PostgreSQL.',
  server_unavailable: 'Não foi possível consultar o servidor. Isso não confirma que o jogo esteja offline.',
  invalid_snapshot: 'O servidor respondeu com dados incompatíveis com o painel. Atualize o servidor e o painel para a mesma versão.',
  staff_unavailable: 'Não foi possível verificar a autorização da equipe.',
  authentication_unavailable: 'O serviço de contas está indisponível. Tente novamente mais tarde.',
  invalid_host: 'O endereço do painel não corresponde à origem configurada no serviço.',
  invalid_origin: 'A origem do painel não corresponde à configuração do serviço.',
  events_unavailable: 'Eventos exigem o painel integrado ao tm.exe atualizado. A Web API separada continua somente leitura.',
};
function explain(error: unknown) {
  return error instanceof APIError ? (messages[error.code] ?? 'O serviço não pôde atender esta solicitação. Tente novamente.') : 'Não foi possível conectar ao painel. Verifique a conexão e tente novamente.';
}
async function api<T>(path: string, method = 'GET', body?: unknown): Promise<T> {
  const response = await fetch(`/api/v1${path}`, {
    method, credentials: 'same-origin', cache: 'no-store', signal: AbortSignal.timeout(12000),
    headers: method === 'GET' ? {} : { 'Content-Type': 'application/json', 'X-CSRF-Token': session?.csrf ?? '' },
    body: body === undefined ? undefined : JSON.stringify(body),
  });
  if (!response.headers.get('Content-Type')?.includes('application/json')) throw new Error('Unexpected service response');
  const value = await response.json();
  if (!response.ok) throw new APIError(response.status, value.error ?? 'request_failed');
  return value as T;
}
const globalDropPanel = setupGlobalDrop(api, explain);
const quizPanel = setupQuiz(api, explain);
function showSession() {
  const authenticated = session?.authenticated ?? false;
  el('login').hidden = authenticated;
  el('dashboard').hidden = !authenticated;
  text('access-badge', authenticated ? `Equipe / ${session!.account}` : 'Acesso restrito');
  if (authenticated) {
    text('session-account', session!.account);
    text('session-expiry', `Expira em ${date(session!.expiresAt)} · 15 min de inatividade`);
  }
}
function resetSnapshot() {
  globalDropPanel.reset();
  quizPanel.reset();
  snapshot = undefined; offset = 0; search = '';
  closeInspector();
  el<HTMLInputElement>('search').value = '';
  for (const id of ['metric-online','metric-auth','metric-mobs','metric-queue','world-uptime','world-tick','world-lag','diagnostics-asof','postgres-queue','persistence-failures','world-panics','player-count','snapshot-time']) text(id,'—');
  text('world-state','Não consultado');
  text('snapshot-state','Aguardando consulta');
  text('snapshot-detail','Use Atualizar dados. Não há atualização automática.');
  el('snapshot-banner').classList.remove('error');
  emptyRows('Nenhuma consulta realizada.');
  updatePagination();
}
async function init() {
  el<HTMLButtonElement>('login-submit').disabled = true;
  try {
    session = await api<Session>('/auth/session');
    showSession(); text('login-message','');
    if (session.authenticated) { void query(); void globalDropPanel.refresh(); void quizPanel.refresh(); }
  } catch (error) {
    session = undefined; showSession(); text('login-message',explain(error));
  } finally {
    el<HTMLButtonElement>('login-submit').disabled = false;
  }
}
function emptyRows(message: string) {
  const cell = document.createElement('td'); cell.colSpan = 6; cell.className = 'empty'; cell.textContent = message;
  const row = document.createElement('tr'); row.append(cell); el('player-rows').replaceChildren(row);
}
function closeInspector() {
  inspectedUID = undefined;
  persistentRequest++;
  el('player-inspector').hidden = true;
}
function clearPersistent(state = 'Aguardando consulta') {
  text('persistent-state',state); text('persistent-asof','—');
  for (const id of ['persistent-class-level','persistent-evolution','persistent-hp','persistent-mp','persistent-attack','persistent-defense','persistent-attributes','persistent-points','persistent-position','persistent-gold','persistent-exp','persistent-cp']) text(id,'—');
}
function renderPersistent(player: PersistentPlayer) {
  text('persistent-state','Consulta concluída'); text('persistent-asof',date(player.asOf));
  text('persistent-class-level',`ID ${player.class} · nível ${player.level}`);
  text('persistent-evolution',player.evolution || 'mortal');
  text('persistent-hp',`${player.hp} / ${player.maxHp}`); text('persistent-mp',`${player.mp} / ${player.maxMp}`);
  text('persistent-attack',`${player.attack} / ${player.magicAttack}`); text('persistent-defense',player.defense);
  text('persistent-attributes',`${player.str} / ${player.int} / ${player.dex} / ${player.con}`);
  text('persistent-points',`${player.statusPoints} / ${player.masteryPoints} / ${player.skillPoints}`);
  text('persistent-position',`${player.x}, ${player.y}`); text('persistent-gold',player.gold.toLocaleString('pt-BR'));
  text('persistent-exp',`${player.exp.toLocaleString('pt-BR')} / ${player.hold.toLocaleString('pt-BR')}`); text('persistent-cp',player.cp);
}
async function loadPersistent(player: Player) {
  const requestId = ++persistentRequest;
  clearPersistent('Consultando PostgreSQL…');
  try {
    const params = new URLSearchParams({account: player.account});
    const value = await api<PersistentPlayer>(`/staff/players/${encodeURIComponent(player.uid)}/persistent?${params}`);
    if (requestId !== persistentRequest || inspectedUID !== player.uid) return;
    if (value.version !== 1 || value.uid !== player.uid || value.account.toLowerCase() !== player.account.toLowerCase()) throw new Error('Invalid persistent state');
    renderPersistent(value);
  } catch (error) {
    if (requestId !== persistentRequest || inspectedUID !== player.uid) return;
    if (error instanceof APIError && (error.status === 401 || error.code === 'staff_revoked')) {
      session = undefined; resetSnapshot(); showSession(); text('login-message',explain(error));
      return;
    }
    clearPersistent(explain(error));
  }
}
function showInspector(player: Player, asOf = snapshot?.asOf) {
  inspectedUID = player.uid;
  text('inspector-name',player.name);
  text('inspector-account',player.account);
  text('inspector-uid',player.uid);
  text('inspector-session',player.sessionId || '—');
  text('inspector-class-level',`ID ${player.class} · nível ${player.level}`);
  text('inspector-hp',`${player.hp} / ${player.maxHp}`);
  text('inspector-mp',`${player.mp} / ${player.maxMp}`);
  text('inspector-guild',player.guildId ? `ID ${player.guildId} · rank ${player.guildRank}` : 'Sem guild');
  text('inspector-party',player.partySize ? `${player.partySize} membro${player.partySize === 1 ? '' : 's'}` : 'Sem grupo');
  text('inspector-position',`${player.x}, ${player.y}`);
  text('inspector-asof',asOf ? date(asOf) : '—');
  el('player-inspector').hidden = false;
  void loadPersistent(player);
}
function render(data: Overview) {
  text('metric-online',data.online); text('metric-auth',data.authenticated); text('metric-mobs',data.mobs); text('metric-queue',data.queueDepth);
  text('world-state',data.state === 'running' ? 'Em execução' : 'Em manutenção');
  text('world-uptime',`${Math.floor(data.uptimeSeconds / 3600)}h ${Math.floor(data.uptimeSeconds % 3600 / 60)}min`);
  text('world-tick',`${(data.tickMicros / 1000).toFixed(2)} ms`); text('world-lag',`${(data.lagMicros / 1000).toFixed(2)} ms`);
  text('diagnostics-asof',date(data.diagnostics.asOf)); text('postgres-queue',data.diagnostics.postgresQueueDepth);
  text('persistence-failures',data.diagnostics.persistenceFailures); text('world-panics',data.diagnostics.panics);
  text('snapshot-state','Consulta concluída'); text('snapshot-detail','Dados deste instante. Atualize para obter uma nova leitura.'); text('snapshot-time',date(data.asOf));
  el('snapshot-banner').classList.remove('error'); text('player-count',data.matched);
  const rows = data.players.map(player => {
    const row = document.createElement('tr');
    const name = document.createElement('td'); name.textContent = player.name;
    const uid = document.createElement('small'); uid.textContent = player.uid; name.append(uid); row.append(name);
    // Unknown class IDs remain visible rather than guessing a legacy class mapping.
    for (const value of [player.account,`ID ${player.class}`,String(player.level),`${player.x}, ${player.y}`]) {
      const cell = document.createElement('td'); cell.textContent = value; row.append(cell);
    }
    const action = document.createElement('td');
    const inspect = document.createElement('button'); inspect.type = 'button'; inspect.className = 'button subtle inspector-open'; inspect.textContent = 'Detalhes';
    inspect.setAttribute('aria-controls','player-inspector'); inspect.addEventListener('click',() => showInspector(player,data.asOf)); action.append(inspect); row.append(action);
    return row;
  });
  if (rows.length) el('player-rows').replaceChildren(...rows);
  else emptyRows(data.matched ? 'Esta página ficou vazia. Atualize a busca desde a primeira página.' : 'Nenhum jogador online encontrado para esta busca.');
  if (inspectedUID) {
    const current = data.players.find(player => player.uid === inspectedUID);
    if (current) showInspector(current,data.asOf); else closeInspector();
  }
  updatePagination();
}
function updatePagination() {
  el<HTMLButtonElement>('previous').disabled = busy || !snapshot || offset === 0;
  el<HTMLButtonElement>('next').disabled = busy || !snapshot?.hasMore;
  text('page-info',snapshot ? `${snapshot.players.length ? offset + 1 : 0}–${offset + snapshot.players.length} de ${snapshot.matched} · Cada página é uma nova consulta` : 'Até 50 jogadores por consulta · Ordenados por UID');
}
async function query(nextOffset = 0, nextSearch = search) {
  if (busy) return;
  busy = true; el<HTMLButtonElement>('refresh').disabled = true; updatePagination();
  text('snapshot-state','Consultando o servidor…');
  text('snapshot-detail',snapshot ? 'Os dados anteriores permanecem visíveis até a resposta.' : 'Aguardando uma leitura pela fila do mundo.');
  try {
    const value = await api<Overview>(`/staff/overview?${new URLSearchParams({search: nextSearch,offset: String(nextOffset)})}`);
    if (value.version !== 1 || value.diagnostics?.version !== 1 || !value.diagnostics.asOf || !Array.isArray(value.players) || value.players.length > 50 || !['running','maintenance'].includes(value.state)) throw new APIError(200, 'invalid_snapshot');
    snapshot = value; offset = value.offset; search = nextSearch; render(value);
  } catch (error) {
    if (error instanceof APIError && (error.status === 401 || error.code === 'staff_revoked')) {
      session = undefined; resetSnapshot(); showSession(); text('login-message',explain(error));
    } else {
      el('snapshot-banner').classList.add('error');
      text('snapshot-state',snapshot ? 'Consulta falhou · dados anteriores' : 'Dados indisponíveis');
      text('snapshot-detail',explain(error));
    }
  } finally { busy = false; el<HTMLButtonElement>('refresh').disabled = false; updatePagination(); }
}
el<HTMLFormElement>('login-form').addEventListener('submit', async event => {
  event.preventDefault(); const button = el<HTMLButtonElement>('login-submit'); button.disabled = true;
  text('login-message','Verificando credenciais…');
  try {
    if (!session) session = await api<Session>('/auth/session');
    session = await api<Session>('/auth/login','POST',{username:el<HTMLInputElement>('username').value,password:el<HTMLInputElement>('password').value,adminPin:el<HTMLInputElement>('adminPin').value});
    resetSnapshot(); showSession(); el<HTMLButtonElement>('refresh').focus();
    void query(); void globalDropPanel.refresh(); void quizPanel.refresh();
  } catch (error) {
    text('login-message',explain(error)); if (error instanceof APIError && error.code === 'invalid_csrf') session = undefined;
  } finally {
    el<HTMLInputElement>('password').value = ''; el<HTMLInputElement>('adminPin').value = ''; button.disabled = false;
  }
});
el('logout').addEventListener('click',async () => {
  const button = el<HTMLButtonElement>('logout'); button.disabled = true;
  try { await api('/auth/logout','POST',{}); session = undefined; resetSnapshot(); showSession(); await init(); }
  catch(error) {
    if (error instanceof APIError && error.code === 'invalid_csrf') {session = undefined;resetSnapshot();showSession();await init();}
    else {text('snapshot-state','Não foi possível encerrar a sessão');text('snapshot-detail',explain(error));el('snapshot-banner').classList.add('error');}
  } finally {button.disabled = false;}
});
el('refresh').addEventListener('click',() => { void query(0); void globalDropPanel.refresh(); void quizPanel.refresh(); });
el<HTMLFormElement>('search-form').addEventListener('submit',event => {event.preventDefault();void query(0,el<HTMLInputElement>('search').value.trim());});
el('previous').addEventListener('click',() => void query(Math.max(0,offset-50)));
el('next').addEventListener('click',() => void query(offset+50));
el('inspector-close').addEventListener('click',closeInspector);
document.querySelectorAll<HTMLAnchorElement>('.nav-link').forEach(link => link.addEventListener('click',() => {
  document.querySelectorAll('.nav-link').forEach(item => item.classList.remove('active'));link.classList.add('active');
}));
void init();
