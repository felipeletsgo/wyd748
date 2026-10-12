export {};
type Session = { version: number; authenticated: boolean; username: string; csrf: string };
type Character = { name: string; class: number; level: number; evolution: string };
const base = '/api/player/v1';
let session: Session | undefined;
function element<T extends HTMLElement>(id: string) { return document.getElementById(id) as T | null; }
function status(id: string, message: string, error = false) {
  const node = element(id); if (node) { node.textContent = message; node.dataset.error = String(error); }
}
async function api<T>(path: string, body?: Record<string, string>): Promise<T> {
  const response = await fetch(base + path, {
    method: body ? 'POST' : 'GET', credentials: 'same-origin', cache: 'no-store',
    headers: body ? { 'Content-Type': 'application/json', 'X-CSRF-Token': session?.csrf ?? '' } : {},
    body: body ? JSON.stringify(body) : undefined, signal: AbortSignal.timeout(8000),
  });
  const data = await response.json().catch(() => { throw new Error('Service unavailable. Please try again later.'); });
  if (!response.ok) throw new Error(typeof data.error === 'string' ? data.error : 'Service unavailable');
  if (data.version !== 1) throw new Error('Unsupported API version');
  return data as T;
}
function message(error: unknown): string {
  return error instanceof Error && error.name !== 'TimeoutError' && error.name !== 'TypeError' ? error.message : 'Connection unavailable. Please try again later.';
}
const classes = ['TransKnight', 'Foema', 'BeastMaster', 'Hunter'];
const evolutions: Record<string, string> = { mortal: 'Mortal', arch: 'Arch', celestial: 'Celestial', subcelestial: 'Subcelestial' };
function characters(id: string, rows: Character[], ranked = false) {
  const target = element(id); if (!target) return;
  const fragment = document.createDocumentFragment();
  rows.slice(0, ranked ? 100 : 4).forEach((character, index) => {
    const row = document.createElement('tr');
    // Native persisted levels are zero-based. Names are always text, never HTML.
    const values = [character.name, classes[character.class] ?? 'Unknown', String(character.level + 1), evolutions[character.evolution] ?? 'Unknown'];
    if (ranked) values.unshift(String(index + 1));
    values.forEach(value => { const cell = document.createElement('td'); cell.textContent = value; row.append(cell); });
    fragment.append(row);
  });
  if (rows.length === 0) { const row = document.createElement('tr'); const cell = document.createElement('td'); cell.colSpan = ranked ? 5 : 4; cell.className = 'empty-row'; cell.textContent = ranked ? 'No characters have entered the ranking yet.' : 'No characters yet. Create your first character in the game.'; row.append(cell); fragment.append(row); }
  target.replaceChildren(fragment);
}
async function profile() {
  const loggedIn = session?.authenticated === true;
  const loginPanel = element('login-panel'); const profilePanel = element('profile-panel');
  if (!loginPanel || !profilePanel) return;
  loginPanel.hidden = loggedIn; profilePanel.hidden = !loggedIn;
  if (!loggedIn) return;
  status('profile-status', 'Loading…');
  try {
    const data = await api<{profile: {username: string; donate: number; characters: Character[]}}>('/profile');
    const name = element('profile-username'); if (name) name.textContent = data.profile.username;
    const balance = element('profile-balance'); if (balance) balance.textContent = `Cash: ${data.profile.donate.toLocaleString('en-US')}`;
    characters('profile-characters', data.profile.characters);
    status('profile-status', '');
  } catch (error) { status('profile-status', message(error), true); }
}
function connectForm(id: string, path: string, statusID: string) {
  const form = element<HTMLFormElement>(id); if (!form) return;
  form.addEventListener('submit', async event => {
    event.preventDefault();
    const button = form.querySelector<HTMLButtonElement>('button[type="submit"]');
    if (!session || !button || button.disabled) return;
    const data = new FormData(form); const body: Record<string, string> = {};
    for (const key of ['username', 'password', 'passwordConfirmation']) { const value = data.get(key); if (typeof value === 'string') body[key] = value; }
    button.disabled = true; status(statusID, path === '/login' ? 'Signing in…' : 'Creating your account…');
    try {
      const result = await api<Session>(path, body);
      if (path === '/login') { session = result; form.reset(); status(statusID, 'Signed in.'); await profile(); }
      else { form.reset(); status(statusID, 'Account created.'); }
    } catch (error) { status(statusID, message(error), true); }
    finally { form.querySelectorAll<HTMLInputElement>('input[type="password"]').forEach(input => { input.value = ''; }); button.disabled = false; }
  });
}
async function initialize() {
  if (element('login-form') || element('registration-form')) {
    try {
      session = await api<Session>('/session');
      document.querySelectorAll<HTMLButtonElement>('#login-form button, #registration-form button').forEach(button => { button.disabled = false; });
      status('login-status', ''); status('registration-status', '');
      connectForm('login-form', '/login', 'login-status'); connectForm('registration-form', '/accounts', 'registration-status');
      await profile();
    } catch (error) { status('login-status', message(error), true); status('registration-status', message(error), true); }
  }
  element<HTMLButtonElement>('logout-button')?.addEventListener('click', async event => {
    const button = event.currentTarget as HTMLButtonElement; button.disabled = true;
    try { await api('/logout', {}); location.reload(); }
    catch (error) { status('profile-status', message(error), true); button.disabled = false; }
  });
  if (element('ranking-characters')) {
    try { const data = await api<{characters: Character[]; asOf: string}>('/ranking'); characters('ranking-characters', data.characters, true); status('ranking-status', `Updated ${new Date(data.asOf).toLocaleString('en-US')}`); }
    catch (error) { status('ranking-status', message(error), true); }
  }
}
void initialize();
