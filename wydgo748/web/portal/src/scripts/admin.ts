import type { components } from '../lib/api.generated';
import { setupGlobalDrop } from './global-drop';
import { setupQuiz } from './quiz';
import { setupBosses } from './bosses';
import { setupPlayerModeration } from './player-moderation';
import { setupPlayerTeleport } from './player-teleport';

type Session = components['schemas']['Session'];
type AccountCreateResponse = components['schemas']['AccountCreateResponse'];
type AccountDirectory = components['schemas']['AccountDirectory'];
type AccountSummary = components['schemas']['AccountSummary'];
type Overview = components['schemas']['Overview'];
type Player = components['schemas']['Player'];
type PersistentPlayer = components['schemas']['PersistentPlayer'];
const el = <T extends HTMLElement = HTMLElement>(id: string) => {
  const element = document.getElementById(id);
  if (!element) throw new Error(`Missing UI element: ${id}`);
  return element as T;
};
const text = (id: string, value: string | number) => { el(id).textContent = String(value); };
const date = (value: string) => new Date(value).toLocaleString('en-US', { dateStyle: 'short', timeStyle: 'medium' });
let session: Session | undefined;
let snapshot: Overview | undefined;
let busy = false;
let offset = 0;
let search = '';
let inspectedUID: string | undefined;
let persistentRequest = 0;
let directorySnapshot: AccountDirectory | undefined;
let directorySearch = '';
let directoryCursor = '';
let directoryHistory: string[] = [];
let directoryBusy = false;

class APIError extends Error {
  constructor(public status: number, public code: string) { super(code); }
}
const messages: Record<string, string> = {
  invalid_credentials: 'Invalid account, password, or admin PIN, or access is not authorized.',
  invalid_admin_pin: 'Invalid admin PIN.',
  username_unavailable: 'This account name is already in use.',
  validation_error: 'Check the account name, password, and confirmation against the 7.48 client rules.',
  registration_unavailable: 'Account creation is available only in the panel integrated with tm.exe.',
  registration_busy: 'Other registrations are being processed. Please wait and try again.',
  registration_failed: 'Could not save the account to PostgreSQL. Please try again.',
  invalid_csrf: 'Your access session expired. Please sign in again.',
  authentication_required: 'Your session expired. Please sign in again.',
  staff_revoked: 'This account’s authorization has changed. Sign in again or contact an administrator.',
  capability_required: 'Your account does not have permission to perform this action.',
  moderation_unavailable: 'Moderation requires an updated panel integrated with tm.exe. The separate Web API remains read-only.',
  rate_limited: 'Too many attempts. Please wait one minute and try again.',
  control_busy: 'The server is processing another request. Please wait and try again.',
  invalid_query: 'Invalid search. Check the text and limits entered.',
  invalid_target: 'The selected player does not have a valid persistent identity.',
  persistent_not_found: 'Persistent state was not found for this character.',
  persistent_unavailable: 'Could not read persistent state from PostgreSQL.',
  server_unavailable: 'Could not query the server. This does not confirm that the game is offline.',
  invalid_snapshot: 'The server returned data incompatible with this panel. Update the server and panel to the same version.',
  staff_unavailable: 'Could not verify staff authorization.',
  authentication_unavailable: 'The account service is unavailable. Please try again later.',
  invalid_host: 'The panel address does not match the service’s configured origin.',
  invalid_origin: 'The panel origin does not match the service configuration.',
  events_unavailable: 'Events require an updated panel integrated with tm.exe. The separate Web API remains read-only.',
};
function explain(error: unknown) {
  return error instanceof APIError ? (messages[error.code] ?? 'The service could not complete this request. Please try again.') : 'Could not connect to the panel. Check your connection and try again.';
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
const bossesPanel = setupBosses(api, explain);
const moderationPanel = setupPlayerModeration(api, explain);
const teleportPanel = setupPlayerTeleport(api, explain);
function showSession() {
  const authenticated = session?.authenticated ?? false;
  el('login').hidden = authenticated;
  el('dashboard').hidden = !authenticated;
  text('access-badge', authenticated ? `Staff / ${session!.account}` : 'Restricted access');
  if (authenticated) {
    text('session-account', session!.account);
    text('session-expiry', `Expires at ${date(session!.expiresAt)} · 15 min of inactivity`);
  }
}
function resetSnapshot() {
  globalDropPanel.reset();
  quizPanel.reset();
  bossesPanel.reset();
  moderationPanel.reset();
  teleportPanel.reset();
  snapshot = undefined; offset = 0; search = '';
  closeInspector();
  el<HTMLInputElement>('search').value = '';
  for (const id of ['metric-online','metric-auth','metric-mobs','metric-queue','world-uptime','world-tick','world-lag','diagnostics-asof','postgres-queue','persistence-failures','world-panics','player-count','snapshot-time']) text(id,'—');
  text('world-state','Not queried');
  text('snapshot-state','Waiting for a query');
  text('snapshot-detail','Use Refresh data. Updates are not automatic.');
  el('snapshot-banner').classList.remove('error');
  emptyRows('No queries made yet.');
  updatePagination();
  resetDirectory();
}
async function init() {
  el<HTMLButtonElement>('login-submit').disabled = true;
  try {
    session = await api<Session>('/auth/session');
    showSession(); text('login-message','');
    if (session.authenticated) { void query(); void loadDirectory(); void globalDropPanel.refresh(); void quizPanel.refresh(); void bossesPanel.refresh(); }
  } catch (error) {
    session = undefined; showSession(); text('login-message',explain(error));
  } finally {
    el<HTMLButtonElement>('login-submit').disabled = false;
    el<HTMLButtonElement>('account-create-submit').disabled = false;
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
function clearPersistent(state = 'Waiting for a query') {
  text('persistent-state',state); text('persistent-asof','—');
  for (const id of ['persistent-class-level','persistent-evolution','persistent-hp','persistent-mp','persistent-attack','persistent-defense','persistent-attributes','persistent-points','persistent-position','persistent-gold','persistent-exp','persistent-cp']) text(id,'—');
}
function renderPersistent(player: PersistentPlayer) {
  text('persistent-state','Query complete'); text('persistent-asof',date(player.asOf));
  text('persistent-class-level',`ID ${player.class} · level ${player.level}`);
  text('persistent-evolution',player.evolution || 'mortal');
  text('persistent-hp',`${player.hp} / ${player.maxHp}`); text('persistent-mp',`${player.mp} / ${player.maxMp}`);
  text('persistent-attack',`${player.attack} / ${player.magicAttack}`); text('persistent-defense',player.defense);
  text('persistent-attributes',`${player.str} / ${player.int} / ${player.dex} / ${player.con}`);
  text('persistent-points',`${player.statusPoints} / ${player.masteryPoints} / ${player.skillPoints}`);
  text('persistent-position',`${player.x}, ${player.y}`); text('persistent-gold',player.gold.toLocaleString('en-US'));
  text('persistent-exp',`${player.exp.toLocaleString('en-US')} / ${player.hold.toLocaleString('en-US')}`); text('persistent-cp',player.cp);
}
async function loadPersistent(player: Player) {
  const requestId = ++persistentRequest;
  clearPersistent('Querying PostgreSQL…');
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
  text('inspector-class-level',`ID ${player.class} · level ${player.level}`);
  text('inspector-hp',`${player.hp} / ${player.maxHp}`);
  text('inspector-mp',`${player.mp} / ${player.maxMp}`);
  text('inspector-guild',player.guildId ? `ID ${player.guildId} · rank ${player.guildRank}` : 'No guild');
  text('inspector-party',player.partySize ? `${player.partySize} member${player.partySize === 1 ? '' : 's'}` : 'No party');
  text('inspector-position',`${player.x}, ${player.y}`);
  text('inspector-asof',asOf ? date(asOf) : '—');
  el('player-inspector').hidden = false;
  void loadPersistent(player);
}
function render(data: Overview) {
  text('metric-online',data.online); text('metric-auth',data.authenticated); text('metric-mobs',data.mobs); text('metric-queue',data.queueDepth);
  text('world-state',data.state === 'running' ? 'Running' : 'Under maintenance');
  text('world-uptime',`${Math.floor(data.uptimeSeconds / 3600)}h ${Math.floor(data.uptimeSeconds % 3600 / 60)}min`);
  text('world-tick',`${(data.tickMicros / 1000).toFixed(2)} ms`); text('world-lag',`${(data.lagMicros / 1000).toFixed(2)} ms`);
  text('diagnostics-asof',date(data.diagnostics.asOf)); text('postgres-queue',data.diagnostics.postgresQueueDepth);
  text('persistence-failures',data.diagnostics.persistenceFailures); text('world-panics',data.diagnostics.panics);
  text('snapshot-state','Query complete'); text('snapshot-detail','Snapshot data. Refresh to get a new reading.'); text('snapshot-time',date(data.asOf));
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
    const inspect = document.createElement('button'); inspect.type = 'button'; inspect.className = 'button subtle inspector-open'; inspect.textContent = 'Details';
    inspect.setAttribute('aria-controls','player-inspector'); inspect.addEventListener('click',() => showInspector(player,data.asOf)); action.append(inspect); row.append(action);
    const kick = document.createElement('button'); kick.type = 'button'; kick.className = 'button subtle'; kick.textContent = 'Disconnect';
    kick.setAttribute('aria-controls','moderation'); kick.addEventListener('click',() => moderationPanel.select(player,data.moderationEpoch)); action.append(kick);
    const teleport = document.createElement('button'); teleport.type = 'button'; teleport.className = 'button subtle'; teleport.textContent = 'Teleport';
    teleport.setAttribute('aria-controls','player-teleport'); teleport.addEventListener('click',() => teleportPanel.select(player,data.moderationEpoch)); action.append(teleport);
    return row;
  });
  if (rows.length) el('player-rows').replaceChildren(...rows);
  else emptyRows(data.matched ? 'This page is empty. Restart the search from the first page.' : 'No online players found for this search.');
  if (inspectedUID) {
    const current = data.players.find(player => player.uid === inspectedUID);
    if (current) showInspector(current,data.asOf); else closeInspector();
  }
  updatePagination();
}
function updatePagination() {
  el<HTMLButtonElement>('previous').disabled = busy || !snapshot || offset === 0;
  el<HTMLButtonElement>('next').disabled = busy || !snapshot?.hasMore;
  text('page-info',snapshot ? `${snapshot.players.length ? offset + 1 : 0}–${offset + snapshot.players.length} of ${snapshot.matched} · Each page requires a new query` : 'Up to 50 players per query · Sorted by UID');
}
function emptyDirectory(message: string) {
  const cell = document.createElement('td'); cell.colSpan = 4; cell.className = 'empty'; cell.textContent = message;
  const row = document.createElement('tr'); row.append(cell); el('account-directory-rows').replaceChildren(row);
}
function resetDirectory() {
  directorySnapshot = undefined;
  directorySearch = '';
  directoryCursor = '';
  directoryHistory = [];
  directoryBusy = false;
  el<HTMLInputElement>('account-directory-search').value = '';
  text('account-directory-status','Waiting for a query.');
  text('account-directory-page','Up to 20 accounts per query · Sorted by account');
  emptyDirectory('No queries made yet.');
  updateDirectoryPagination();
}
function updateDirectoryPagination() {
  el<HTMLButtonElement>('account-directory-previous').disabled = directoryBusy || directoryHistory.length === 0;
  el<HTMLButtonElement>('account-directory-next').disabled = directoryBusy || !directorySnapshot?.nextCursor;
}
function characterSummary(account: AccountSummary) {
  const cell = document.createElement('td');
  if (!account.characters.length) {
    cell.textContent = 'No characters';
    cell.className = 'account-characters-empty';
    return cell;
  }
  const list = document.createElement('div'); list.className = 'account-characters';
  for (const character of account.characters) {
    const item = document.createElement('div'); item.className = 'account-character';
    const name = document.createElement('strong'); name.textContent = character.name;
    const details = document.createElement('small');
    details.textContent = `slot ${character.slot + 1} · class ${character.class} · level ${character.level} · ${character.evolution || 'mortal'}`;
    item.append(name,details); list.append(item);
  }
  cell.append(list);
  return cell;
}
function renderDirectory(data: AccountDirectory) {
  const rows = data.accounts.map(account => {
    const row = document.createElement('tr');
    const username = document.createElement('td'); username.textContent = account.username;
    const created = document.createElement('small'); created.textContent = `Created ${date(account.createdAt)}`; username.append(created);
    const presence = document.createElement('td');
    const badge = document.createElement('span');
    badge.className = `presence-badge ${data.presenceAvailable ? (account.online ? 'online' : 'offline') : 'unknown'}`;
    badge.textContent = data.presenceAvailable ? (account.online ? 'Online' : 'Offline') : 'Unavailable'; presence.append(badge);
    const updated = document.createElement('td'); updated.textContent = date(account.updatedAt);
    row.append(username,presence,characterSummary(account),updated);
    return row;
  });
  if (rows.length) el('account-directory-rows').replaceChildren(...rows);
  else emptyDirectory('No accounts found for this search.');
  text('account-directory-status', data.presenceAvailable
    ? `Persistent snapshot from ${date(data.asOf)}. Presence was confirmed by the server.`
    : `Persistent snapshot from ${date(data.asOf)}. Online presence could not be confirmed.`);
  text('account-directory-page', `${data.accounts.length} account${data.accounts.length === 1 ? '' : 's'} on this page · Cursor pagination`);
  updateDirectoryPagination();
}
async function loadDirectory(nextCursor = directoryCursor, nextSearch = directorySearch, history = directoryHistory) {
  if (directoryBusy) return;
  directoryBusy = true; updateDirectoryPagination(); text('account-directory-status','Querying accounts in PostgreSQL…');
  try {
    const params = new URLSearchParams({search: nextSearch,cursor: nextCursor,limit: '20'});
    const value = await api<AccountDirectory>(`/staff/accounts?${params}`);
    if (value.version !== 1 || !value.asOf || !Array.isArray(value.accounts) || value.accounts.length > 20) throw new APIError(200,'invalid_snapshot');
    directorySnapshot = value; directorySearch = nextSearch; directoryCursor = nextCursor; directoryHistory = history; renderDirectory(value);
  } catch (error) {
    if (error instanceof APIError && (error.status === 401 || error.code === 'staff_revoked')) {
      session = undefined; resetSnapshot(); showSession(); text('login-message',explain(error));
    } else {
      text('account-directory-status',explain(error));
    }
  } finally { directoryBusy = false; updateDirectoryPagination(); }
}
async function query(nextOffset = 0, nextSearch = search) {
  if (busy) return;
  busy = true; el<HTMLButtonElement>('refresh').disabled = true; updatePagination();
  text('snapshot-state','Querying the server…');
  text('snapshot-detail',snapshot ? 'Previous data remains visible until the response arrives.' : 'Waiting for a world queue snapshot.');
  try {
    const value = await api<Overview>(`/staff/overview?${new URLSearchParams({search: nextSearch,offset: String(nextOffset)})}`);
    if (value.version !== 1 || value.diagnostics?.version !== 1 || !value.diagnostics.asOf || !Array.isArray(value.players) || value.players.length > 50 || !['running','maintenance'].includes(value.state)) throw new APIError(200, 'invalid_snapshot');
    snapshot = value; offset = value.offset; search = nextSearch; render(value);
  } catch (error) {
    if (error instanceof APIError && (error.status === 401 || error.code === 'staff_revoked')) {
      session = undefined; resetSnapshot(); showSession(); text('login-message',explain(error));
    } else {
      el('snapshot-banner').classList.add('error');
      text('snapshot-state',snapshot ? 'Query failed · previous data' : 'Data unavailable');
      text('snapshot-detail',explain(error));
    }
  } finally { busy = false; el<HTMLButtonElement>('refresh').disabled = false; updatePagination(); }
}
el<HTMLFormElement>('account-create-form').addEventListener('submit', async event => {
  event.preventDefault();
  const button = el<HTMLButtonElement>('account-create-submit');
  const username = el<HTMLInputElement>('new-account-username');
  const password = el<HTMLInputElement>('new-account-password');
  const confirmation = el<HTMLInputElement>('new-account-confirmation');
  const pin = el<HTMLInputElement>('new-account-pin');
  if (password.value !== confirmation.value) {
    confirmation.setCustomValidity('Passwords do not match.');
    confirmation.reportValidity();
    return;
  }
  confirmation.setCustomValidity('');
  button.disabled = true;
  text('account-create-message','Creating the account securely…');
  try {
    if (!session) session = await api<Session>('/auth/session');
    const result = await api<AccountCreateResponse>('/accounts','POST',{
      username:username.value,
      password:password.value,
      passwordConfirmation:confirmation.value,
      adminPin:pin.value,
    });
    el<HTMLInputElement>('username').value = result.username;
    el<HTMLFormElement>('account-create-form').reset();
    text('account-create-message',`Account ${result.username} created. It can now sign in to the game.`);
    if (session?.authenticated) void loadDirectory('',directorySearch,[]);
  } catch (error) {
    text('account-create-message',explain(error));
    if (error instanceof APIError && error.code === 'invalid_csrf') session = undefined;
  } finally {
    password.value = '';
    confirmation.value = '';
    pin.value = '';
    button.disabled = false;
  }
});
el<HTMLInputElement>('new-account-confirmation').addEventListener('input',event => {
  (event.currentTarget as HTMLInputElement).setCustomValidity('');
});
el<HTMLFormElement>('login-form').addEventListener('submit', async event => {
  event.preventDefault(); const button = el<HTMLButtonElement>('login-submit'); button.disabled = true;
  text('login-message','Checking credentials…');
  try {
    if (!session) session = await api<Session>('/auth/session');
    session = await api<Session>('/auth/login','POST',{username:el<HTMLInputElement>('username').value,password:el<HTMLInputElement>('password').value,adminPin:el<HTMLInputElement>('adminPin').value});
    resetSnapshot(); showSession(); el<HTMLButtonElement>('refresh').focus();
    void query(); void loadDirectory(); void globalDropPanel.refresh(); void quizPanel.refresh(); void bossesPanel.refresh();
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
    else {text('snapshot-state','Could not end the session');text('snapshot-detail',explain(error));el('snapshot-banner').classList.add('error');}
  } finally {button.disabled = false;}
});
el('refresh').addEventListener('click',() => { void query(0); void loadDirectory(directoryCursor); void globalDropPanel.refresh(); void quizPanel.refresh(); void bossesPanel.refresh(); });
el<HTMLFormElement>('search-form').addEventListener('submit',event => {event.preventDefault();void query(0,el<HTMLInputElement>('search').value.trim());});
el('previous').addEventListener('click',() => void query(Math.max(0,offset-50)));
el('next').addEventListener('click',() => void query(offset+50));
el('inspector-close').addEventListener('click',closeInspector);
el<HTMLFormElement>('account-directory-form').addEventListener('submit',event => {
  event.preventDefault();
  void loadDirectory('',el<HTMLInputElement>('account-directory-search').value.trim().toLowerCase(),[]);
});
el('account-directory-previous').addEventListener('click',() => {
  if (!directoryHistory.length) return;
  const history = directoryHistory.slice(0,-1);
  void loadDirectory(directoryHistory[directoryHistory.length-1] ?? '',directorySearch,history);
});
el('account-directory-next').addEventListener('click',() => {
  const next = directorySnapshot?.nextCursor;
  if (!next) return;
  void loadDirectory(next,directorySearch,[...directoryHistory,directoryCursor]);
});
document.querySelectorAll<HTMLAnchorElement>('.nav-link').forEach(link => link.addEventListener('click',() => {
  document.querySelectorAll('.nav-link').forEach(item => item.classList.remove('active'));link.classList.add('active');
}));
void init();
