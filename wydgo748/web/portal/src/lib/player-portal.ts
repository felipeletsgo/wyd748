import { RegistrationChallenge } from './registration-challenge';
import { AccountSessions, displayTime } from './account-sessions';
import { CharacterDetails } from './character-details';
import { ArchAdvisor } from './arch-advisor';
import { CelestialAdvisor } from './celestial-advisor';
import { SubCelestialAdvisor } from './subcelestial-advisor';
import { StorageView } from './storage-view';
import type { components } from './api.generated';
type Session = { version: number; authenticated: boolean; username: string; csrf: string };
type Character = { slot: number; name: string; class: number; level: number; evolution: string };
const base = '/api/player/v1';
let session: Session | undefined;
let accountSessions: AccountSessions | undefined;
let characterDetails: CharacterDetails | undefined;
let inventoryView: StorageView | undefined;
let cargoView: StorageView | undefined;
let registrationChallenge: RegistrationChallenge | undefined;
let registrationSubmitting = false;
let loginChallenge: RegistrationChallenge | undefined;
let loginSubmitting = false;
function updateLogin(error?: string) {
  const button = document.querySelector<HTMLButtonElement>('#login-form button[type="submit"]');
  if (button) button.disabled = !session || !loginChallenge?.token || loginSubmitting;
  if (error) status('login-status', error, true);
}
function updateRegistration(error?: string) {
  const button = document.querySelector<HTMLButtonElement>('#registration-form button[type="submit"]');
  if (button) button.disabled = !session || !registrationChallenge?.token || registrationSubmitting;
  if (error) status('registration-status', error, true);
}
function element<T extends HTMLElement>(id: string) { return document.getElementById(id) as T | null; }
function status(id: string, message: string, error = false) {
  const node = element(id); if (node) { node.textContent = message; node.dataset.error = String(error); }
}
async function api<T>(path: string, body?: Record<string, string>): Promise<T> {
  const requestSession = session;
  const response = await fetch(base + path, {
    method: body ? 'POST' : 'GET', credentials: 'same-origin', cache: 'no-store',
    headers: body ? { 'Content-Type': 'application/json', 'X-CSRF-Token': session?.csrf ?? '' } : {},
    body: body ? JSON.stringify(body) : undefined, signal: AbortSignal.timeout(8000),
  });
  const data = await response.json().catch(() => { throw new Error('Service unavailable. Please try again later.'); });
  if (response.status === 401 && session?.authenticated && requestSession === session) {
    session = undefined;
    clearProfile();
    status('login-status', 'Your website session ended. Reload this page to sign in.', true);
    updateLogin();
  }
  if (!response.ok) throw new Error(typeof data.error === 'string' ? data.error : 'Service unavailable');
  if (data.version !== 1) throw new Error('Unsupported API version');
  return data as T;
}
function message(error: unknown): string {
  return error instanceof Error && error.name !== 'TimeoutError' && error.name !== 'TypeError' ? error.message : 'Connection unavailable. Please try again later.';
}
const classes = ['TransKnight', 'Foema', 'BeastMaster', 'Hunter'];
const evolutions: Record<string, string> = { mortal: 'Mortal', arch: 'Arch', celestial: 'Celestial', subcelestial: 'Subcelestial' };
function characters(id: string, rows: Character[]) {
  const target = element(id); if (!target) return;
  const fragment = document.createDocumentFragment();
  rows.slice(0, 4).forEach(character => {
    const row = document.createElement('tr');
    // Native persisted levels are zero-based. Names are always text, never HTML.
    const values = [character.name, classes[character.class] ?? 'Unknown', String(character.level + 1), evolutions[character.evolution] ?? 'Unknown'];
    values.forEach((value, index) => {
      const cell = document.createElement('td');
      if (index === 0) {
        const button = document.createElement('button'); button.type = 'button'; button.className = 'character-link'; button.textContent = value;
        button.addEventListener('click', () => { void characterDetails?.load(character.slot); }); cell.append(button);
      } else cell.textContent = value;
      row.append(cell);
    });
    fragment.append(row);
  });
  if (rows.length === 0) { const row = document.createElement('tr'); const cell = document.createElement('td'); cell.colSpan = 4; cell.className = 'empty-row'; cell.textContent = 'No characters yet. Create your first character in the game.'; row.append(cell); fragment.append(row); }
  target.replaceChildren(fragment);
}
function clearProfile() {
  const loginPanel = element('login-panel'); const profilePanel = element('profile-panel');
  if (loginPanel) loginPanel.hidden = false;
  if (profilePanel) profilePanel.hidden = true;
  for (const id of ['profile-username', 'profile-balance', 'profile-created', 'profile-asof', 'profile-status', 'profile-characters']) element(id)?.replaceChildren();
  accountSessions?.clear();
  characterDetails?.clear();
  inventoryView?.clear(); cargoView?.clear();
}
async function profile() {
  const loggedIn = session?.authenticated === true;
  const loginPanel = element('login-panel'); const profilePanel = element('profile-panel');
  if (!loginPanel || !profilePanel) return;
  loginPanel.hidden = loggedIn; profilePanel.hidden = !loggedIn;
  if (!loggedIn) { clearProfile(); return; }
  const identity = session;
  status('profile-status', 'Loading…');
  try {
    const data = await api<components['schemas']['PlayerProfileResponse']>('/profile');
    if (identity !== session) return;
    const name = element('profile-username'); if (name) name.textContent = data.profile.username;
    const balance = element('profile-balance'); if (balance) balance.textContent = `Cash: ${data.profile.donate.toLocaleString('en-US')}`;
    characters('profile-characters', data.profile.characters);
    const created = element('profile-created'); if (created) created.textContent = `Account created: ${displayTime(data.profile.createdAt)}`;
    const asOf = element('profile-asof'); if (asOf) asOf.textContent = `Saved account snapshot: ${displayTime(data.profile.asOf)}`;
    status('profile-status', '');
    cargoView?.select(null);
  } catch (error) { if (identity === session) status('profile-status', message(error), true); }
  if (identity === session) await accountSessions?.load();
}
function connectForm(id: string, path: string, statusID: string) {
  const form = element<HTMLFormElement>(id); if (!form) return;
  form.addEventListener('submit', async event => {
    event.preventDefault();
    const button = form.querySelector<HTMLButtonElement>('button[type="submit"]');
    if (!session || !button || button.disabled) return;
    if (path === '/accounts' && !registrationChallenge?.token) {
      updateRegistration('Complete human verification before creating an account.'); return;
    }
    if (path === '/login' && !loginChallenge?.token) { updateLogin('Complete human verification before signing in.'); return; }
    const data = new FormData(form); const body: Record<string, string> = {};
    for (const key of ['username', 'password', 'passwordConfirmation']) { const value = data.get(key); if (typeof value === 'string') body[key] = value; }
    if (path === '/accounts') { body.turnstileToken = registrationChallenge!.token; registrationSubmitting = true; }
    if (path === '/login') { body.turnstileToken = loginChallenge!.token; loginSubmitting = true; }
    button.disabled = true; status(statusID, path === '/login' ? 'Signing in…' : 'Creating your account…');
    try {
      const result = await api<Session>(path, body);
      if (path === '/login') { session = result; form.reset(); status(statusID, 'Signed in.'); await profile(); }
      else { form.reset(); status(statusID, 'Account created.'); }
    } catch (error) { status(statusID, message(error), true); }
    finally {
      form.querySelectorAll<HTMLInputElement>('input[type="password"]').forEach(input => { input.value = ''; });
      if (path === '/accounts') { registrationSubmitting = false; registrationChallenge?.reset(); updateRegistration(); }
      else { loginSubmitting = false; loginChallenge?.reset(); updateLogin(); }
    }
  });
}
async function initialize() {
  function storageView(prefix: string) {
    const disclosure = element<HTMLDetailsElement>(`${prefix}-view`), items = element(`${prefix}-items`);
    const summary = element(`${prefix}-summary`), timestamp = element(`${prefix}-asof`), status = element(`${prefix}-status`);
    const refresh = element<HTMLButtonElement>(`${prefix}-refresh`);
    if (disclosure && items && summary && timestamp && status && refresh) {
      return new StorageView(disclosure, items, summary, timestamp, status, refresh, api,
        () => session?.authenticated ? session.csrf : undefined);
    }
  }
  inventoryView = storageView('inventory'); cargoView = storageView('cargo');
  const detailPanel = element('character-panel'), detailTitle = element('character-title'), detailStats = element('character-stats');
  const equipment = element('character-equipment'), detailTimestamp = element('character-asof'), detailStatus = element('character-status');
  const detailRefresh = element<HTMLButtonElement>('character-refresh');
  const progression = element<HTMLDetailsElement>('arch-advisor'), progressionChecks = element('arch-checks'), progressionStatus = element('arch-status');
  const advisor = progression && progressionChecks && progressionStatus ? new ArchAdvisor(progression, progressionChecks, progressionStatus) : undefined;
  const celestial = element<HTMLDetailsElement>('celestial-advisor'), celestialChecks = element('celestial-checks'), celestialStatus = element('celestial-status');
  const celestialAdvisor = celestial && celestialChecks && celestialStatus ? new CelestialAdvisor(celestial, celestialChecks, celestialStatus) : undefined;
  const subCelestial = element<HTMLDetailsElement>('subcelestial-advisor'), subChecks = element('subcelestial-checks'), subStatus = element('subcelestial-status');
  const subCelestialAdvisor = subCelestial && subChecks && subStatus ? new SubCelestialAdvisor(subCelestial, subChecks, subStatus) : undefined;
  if (detailPanel && detailTitle && detailStats && equipment && detailTimestamp && detailStatus && detailRefresh) {
    characterDetails = new CharacterDetails(detailPanel, detailTitle, detailStats, equipment, detailTimestamp, detailStatus, detailRefresh, api,
      () => session?.authenticated ? session.csrf : undefined, slot => inventoryView?.select(slot), advisor, celestialAdvisor, subCelestialAdvisor);
  }
  const sessionList = element('account-sessions'); const sessionStatus = element('sessions-status');
  const refresh = element<HTMLButtonElement>('sessions-refresh'); const revokeAll = element<HTMLButtonElement>('sessions-revoke-all');
  if (sessionList && sessionStatus && refresh && revokeAll) {
    accountSessions = new AccountSessions(sessionList, sessionStatus, refresh, revokeAll, api,
      () => session?.authenticated ? session.csrf : undefined,
      () => { session = undefined; clearProfile(); location.reload(); });
  }
  const loginContainer = element('login-challenge');
  if (loginContainer) {
    loginChallenge = new RegistrationChallenge(loginContainer, updateLogin);
    void loginChallenge.start().catch(error => updateLogin(message(error)));
  }
  const challenge = element('registration-challenge');
  if (challenge) {
    registrationChallenge = new RegistrationChallenge(challenge, updateRegistration);
    void registrationChallenge.start().catch(error => updateRegistration(message(error)));
  }
  if (element('login-form') || element('registration-form')) {
    try {
      session = await api<Session>('/session');
      updateLogin();
      updateRegistration();
      status('login-status', '');
      connectForm('login-form', '/login', 'login-status'); connectForm('registration-form', '/accounts', 'registration-status');
      await profile();
    } catch (error) { status('login-status', message(error), true); status('registration-status', message(error), true); }
  }
  element<HTMLButtonElement>('logout-button')?.addEventListener('click', async event => {
    const button = event.currentTarget as HTMLButtonElement; button.disabled = true;
    try { await api('/logout', {}); session = undefined; clearProfile(); location.reload(); }
    catch (error) { status('profile-status', message(error), true); button.disabled = false; }
  });
}
void initialize();
